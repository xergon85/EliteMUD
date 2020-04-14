/* ************************************************************************
*   File: magic.c                                       Part of EliteMUD  *
*  Usage: actual effects of magical spells                                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "db.h"
#include "functions.h"

/* extern structures */
extern struct char_data *mob_proto;
extern struct room_data **world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct clan_data *clan_list;
extern int	pk_allowed;	/* see config.c */
extern int      pkok_allowed;   /* see config.c */
extern sh_int r_mortal_start_room;
extern char	*pc_class_types[];
extern struct obj_data *obj_proto;

/* extern functions */
void	add_follower(struct char_data *ch, struct char_data *leader);
bool    elite_follow(struct char_data *ch, struct char_data * victim);
void	stop_follower(struct char_data *ch);
int	roomflag_check(struct char_data *ch);


extern  char  *skills[];
extern struct message_list fight_messages[MAX_MESSAGES];


/* New function to handle group spells */
int perform_mag_groups(int level, struct char_data * ch,
			struct char_data * victim, int spellnum, int casttype)
{
/*  check cost per vs. initial cost   first  */
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
      if ((GET_MANA(ch) - 40) < 25 ) return 1;
      GET_MANA(ch) -= 40;
      mag_points(level, ch, victim, SPELL_HEAL, casttype);
      mag_unaffects(level, ch, victim, SPELL_HEAL, casttype);
    break;
  case SPELL_GROUP_RECALL:
      if ((GET_MANA(ch) - 10) < 15 ) return 1;
      GET_MANA(ch) -= 10;
      spell_word_of_recall(level, ch, NULL, victim, NULL);
    break;
  default:
       sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_groups", spellnum);
       log(buf);
       return(1);
       break;
  }
  return 0;
}

/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, struct char_data * ch, int spellnum, int casttype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!IS_AFFECTED(ch, AFF_GROUP))
    return;

  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!IS_AFFECTED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    if (perform_mag_groups(level, ch, tch, spellnum, casttype)) {
      send_to_char("You don't have enough mana to complete the spell\r\n", ch);
      return; }
  }

  if ((k != ch) && IS_AFFECTED(k, AFF_GROUP))
    if (perform_mag_groups(level, ch, k, spellnum, casttype))  {
      send_to_char("You don't have enough mana to complete the spell\r\n", ch);
      return; }
    if (perform_mag_groups(level, ch, ch, spellnum, casttype))  {
      send_to_char("You don't have enough mana to complete the spell\r\n", ch);
      return; }
}


/* New function to handle damage spells -Petrus */
void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		int spellnum, int casttype)
{
    int savetype = 0;
    int dam = 0;
    int type = SAVE_HALF;

    if (victim == NULL || ch == NULL)
	return;

    if (!pkok_check(ch, victim))
      return;

    if (magic_resist(ch, victim)) return;

    switch (spellnum) {

    case SPELL_MAGIC_MISSILE:
      /* dam = dice(1,4) + MIN(level/2, 20); - Helm */
      dam = dice(4, 6) + (2 * MIN(level, 15));
	savetype = SAVING_MAGIC;
	break;
    case SPELL_ARC_FIRE:
	dam = dice(1,6) + MIN(level/2, 20);
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_WARSTRIKE:
	dam = dice(4,6) + MIN(level/4, 10);
	savetype = SAVING_MAGIC;
	break;
    case SPELL_SHOCKING_SPHERE:
	dam = dice(4,8) + MIN(level/4, 10);
	savetype = SAVING_MAGIC;
	break;
    case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
	dam = (dice(1,6) + MIN(level/2, 20));
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_BURNING_HANDS:
	dam = dice(3, 8) + 3;
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_SHOCKING_GRASP:
	dam = dice(5, 8) + 5;
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_LIGHTNING_BOLT:
      /*dam = dice(7, 8) + 7; - Helm*/
      dam = (dice(2, 8) + 4) * (MIN(level, 60) / 4);
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_COLOUR_SPRAY:
	dam = dice(2,12) + MIN(level/2, 20);
	savetype = SAVING_MAGIC;
	break;
    case SPELL_FIREBALL:
      /*dam = dice(6, 10); -  Helm */
      dam = (dice(1, 20) + 10) * (MIN(level, 90) / 5);
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_DISPEL_EVIL:
	dam = dice(6, 8) + 6;
	if (IS_EVIL(ch)) {
	    victim = ch;
	    dam = GET_HIT(ch) - 1;
	} else if (IS_GOOD(victim)) {
	    act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
	    dam = 0;
	    return;
	}
	break;
    case SPELL_CALL_LIGHTNING:
	dam = dice(7, 8) + 7;
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_HARM:
	dam = dice(8, 8) + 8;
	savetype = SAVING_MAGIC;
	break;
    case SPELL_ENERGY_DRAIN:
	if (GET_LEVEL(victim) <= 2)
	    dam = 100;
	else
	    dam = dice(1, 10);
	savetype = SAVING_MAGIC;
	/* Area spells */
    case SPELL_EARTHQUAKE:
	dam = dice(2, 8) + MIN(level/4, 30);
	break;
    case SPELL_LIGHTNING_BREATH:
	dam = number(0, MAX(10, GET_HIT(ch) >> 2));
	savetype = SAVING_MAGIC;
	break;
    case SPELL_GAS_BREATH:
    case SPELL_ACID_BREATH:
	dam = number(0, MAX(10, GET_HIT(ch) >> 2));
	savetype = SAVING_POISON;
	break;
    case SPELL_FIRE_BREATH:
	dam = number(0, MAX(10, GET_HIT(ch) >> 2));
	savetype = SAVING_MAGIC;
	break;
    case SPELL_FROST_BREATH:
	dam = number(0, MAX(10, GET_HIT(ch) >> 2));
	savetype = SAVING_MAGIC;
	break;
    case SPELL_STAR_FLARE:
      /* dam = dice(4,10) + MIN(level/4, 10); - Helm */
      dam = dice(5, 8) + (2 * MIN(level, 50));
	savetype = SAVING_MAGIC;
	break;
    case SPELL_SPECTRE_TOUCH:	/* spectre touch also has an affect */
      /*dam = (dice(4,6) + MIN(level/2, 20));*/
      dam = dice(4, 10) + MIN(level, 40);
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_DEATH_STRIKE:
	if (number(1, GET_LEVEL(ch)*12/10) > GET_LEVEL(victim))
	    dam = GET_HIT(victim) + 11;
	else
	    dam = 0;
	savetype = SAVING_MAGIC;
	break;
    case SPELL_ICE_STORM:
	dam = dice(6, 10) + 10;
	savetype = SAVING_PHYSICAL;
	break;
    case SPELL_MIND_BLADE:
	dam = dice(4, 10) + MIN(level/4, 25);
	savetype = SAVING_MENTAL;
	break;
    case SPELL_RIMEFANG:
      dam = (dice(5, 6) + 10) * (MIN(level, 90) / 6);
        savetype = SAVING_MAGIC;
	break;
    case SPELL_MAELSTROM:
	dam = dice(5, 20);
	savetype = SAVING_MAGIC;
	break;
    case SPELL_GRAVITY_FOCUS:
	dam = dice(1,100) + 100;
	break;
	/* Helm - New Damage Spells */
    case SPELL_MIND_JAB:
      dam = dice(3, 6) + (MIN(level, 20));
      savetype = SAVING_MENTAL;
      break;
    case SPELL_PHASE_KNIFE:
      dam = dice(3, 8) + (MIN(level, 25));
      savetype = SAVING_MENTAL;
      break;
    case SPELL_FLAME_RAY:
      dam = dice(4, 10) + (MIN(level, 25) * 2);
      savetype = SAVING_PHYSICAL;
      break;
    case SPELL_PSYCHIC_SCREAM:
      dam = dice(6, 8) + (MIN(level, 40) * 3/2);
      savetype = SAVING_MENTAL;
      break;
    case SPELL_PROJECT_FORCE:
      dam = dice(6, 10) + (15 * (MIN(level, 70) / 5));
      savetype = SAVING_PHYSICAL;
      break;
    case SPELL_DETONATE:
      dam = (dice(2, 10) + 10) * (MIN(level, 75) / 6);
      savetype = SAVING_PHYSICAL;
      break;
    case SPELL_PHASEFIRE:
      dam = dice(1, 30) * (MIN(level, 75) / 6);
      savetype = SAVING_MENTAL;
      break;
    case SPELL_DRAIN_LIFE:
      dam = (dice(1, 8) + 8) * (MIN(level, 109) / 5);
      savetype = SAVING_MAGIC;
      GET_HIT(ch) += MIN(dam / 4, 75);
      break;
    case SPELL_SHADOW_KNIFE:
      dam = (dice(1, 20) + 15) * (MIN(level, 100) / 6);
      savetype = SAVING_MENTAL;
      break;
    case SPELL_DISINTEGRATE:
      dam = (dice(1, 20) + 20) * (MIN(level, 109) / 6);
      savetype = SAVING_MAGIC;
      break;
    case SPELL_TERRORWEAVE:
      dam = (dice(1, 30) + 30) * (MIN(level, 109) / 5);
      savetype = SAVING_MENTAL;
      break;
    case SPELL_DRAGON_BREATH:
      /*      dam = (dice(1, 20) + 20) * (MIN(level, 109) / 5); */
      dam = (dice(1, 20) + 10) * (MIN(level, 109) / 5);
      savetype = SAVING_MAGIC;
      break;
    case SPELL_ELEMENTAL_CANNON:
      dam = (dice(1, 20) + 30) * (MIN(level, 109) / 5);
      savetype = SAVING_MAGIC;
      break;
    case SPELL_UNLEASH_MIND:
      dam = (dice(1, 20) + 20) * (MIN(level, 109) / 5);
      savetype = SAVING_MENTAL;
      break;
    case SPELL_PHANTASMAL_KILLER:
      dam = (dice(1, 20) + 20) * (MIN(level, 109) / 5);
      savetype = SAVING_MENTAL;
      break;
    default:
	 sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_damage", spellnum);
	 log(buf);
	 return;
	 break;
    }				/* switch(spellnum) */

    if (savetype) saves_spell(victim, savetype, &dam, type);

    damage(ch, victim, dam, spellnum);
}

/* Spells that affect a greater area than one room */
void mag_areas(byte level, struct char_data * ch, int spellnum, int savetype)
{
    struct char_data *tch, *next_tch, *m;
    int skip;

    if (ch == NULL)
	return;

    if (ch->master != NULL)
	m = ch->master;
    else
	m = ch;

    for (tch = world[ch->in_room]->people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	/*
         * Modified for PKOK system and pk_allowed check
         *
	 * 1) immortals && self
	 * 2) mobs only hit charmed mobs
	 * 3) players can only hit players in CRIMEOK rooms
         *    or if PKOK flagged if pkok_allowed
         *    players can hit players regardless if pk_allowed
	 * 4) players can only hit charmed mobs in CRIMEOK rooms
         *    or if PKOK flagged if pkok_allowed
         *    players can hit players regardless if pk_allowed
	 */


	skip = 0;
	if (tch == ch)
	    skip = 1;
	if (IS_NPC(ch) && IS_NPC(tch) && !IS_AFFECTED(tch, AFF_CHARM))
	    skip = 1;
	if (!IS_NPC(tch) && GET_LEVEL(tch) >= LEVEL_DEITY)
	    skip = 1;
        if (!pk_allowed) {
          if (!pkok_allowed || !PLR_FLAGGED(ch, PLR_PKOK) || !PLR_FLAGGED(tch, PLR_PKOK)) {
	    if (!IS_NPC(ch) && !IS_NPC(tch) && !ROOM_FLAGGED(IN_ROOM(ch), PKOK))
	      skip = 1;
	    if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) &&
                !ROOM_FLAGGED(IN_ROOM(ch), PKOK))
	      skip = 1;
          }
        }

	if (skip)
	    continue;

	switch (spellnum) {

	case SPELL_STAR_FLARE:
	case SPELL_DRAGON_BREATH:
	case SPELL_LIGHTNING_BREATH:
	case SPELL_FIRE_BREATH:
	case SPELL_GAS_BREATH:
	case SPELL_ACID_BREATH:
	case SPELL_FROST_BREATH:
	case SPELL_EARTHQUAKE:
	case SPELL_ICE_STORM:
	case SPELL_MAELSTROM:
	    mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
	    break;
	default:
	    return;
	}
    }
}



/* New function to handle heal- restore- refresh- spells -Petrus */
void mag_points(int level, struct char_data * ch, struct char_data * victim,
		int spellnum, int casttype)
{
    int nr, j;
    int hit = 0;
    int move = 0;
    int messnum;
    struct message_type *messages;

    if (victim == NULL)
	return;

    messnum = spellnum;

    switch (spellnum) {
    case SPELL_CURE_LIGHT:
	hit = dice(1, 8) + 1 + (level >> 4);
	break;
    case SPELL_CURE_CRITIC:
	hit = dice(3, 8) + 3;
    break;
    case SPELL_HEAL:
	hit = 100 + dice(3, 8);
	break;
    case SPELL_WORD_OF_HEALING:
	hit = dice(2, 8);
	break;
    case SPELL_FLESH_RESTORE:
	hit = dice(2, 8) + 1 + (level >> 4);
	messnum = SPELL_CURE_LIGHT;
	break;
    case SPELL_FLESH_ANEW:
	hit = dice(4, 8) + 1 + (level >> 4);
	messnum = SPELL_HEAL;
	break;
    case SPELL_QUICK_FIX:
	hit = dice(1, 6);
	messnum = SPELL_CURE_LIGHT;
	break;
    default:
	 sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_points", spellnum);
	 log(buf);
	 return;
	 break;
    }

    GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
    GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);

    update_pos(victim);

    assert(messnum > 0 && messnum < MAX_MESSAGES);

    if (fight_messages[messnum].number_of_attacks &&
	fight_messages[messnum].msg)
    {
	nr = dice(1, fight_messages[messnum].number_of_attacks);
	for (j = 1, messages = fight_messages[messnum].msg; (j < nr) && (messages); j++)
	    messages = messages->next;


	if (ch != victim) {
	    /* Here the messages are the opposite - die = completely healed*/
	    if (GET_HIT(victim) == GET_MAX_HIT(victim)) {
		act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
		act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
		act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	    } else { /* Partially healed */
		act(messages->hit_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
		act(messages->hit_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
		act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	    }
	} else { /* Heal oneself */
	    if (GET_HIT(victim) == GET_MAX_HIT(victim))
		act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	    else
		act(messages->miss_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	}
    }
}


/* New function to handle affect spells -P */
void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		 int spellnum, int casttype)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  af.bitvector = 0;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.type = spellnum;

  switch (spellnum) {
  case SPELL_CHILL_TOUCH:
  case SPELL_SPECTRE_TOUCH:
    if (saves_spell(victim, SAVING_MAGIC, NULL, SAVE_NEGATE))
      af.duration = 1;
    else
      af.duration = 4;
    af.modifier = -1;
    af.location = APPLY_STR;
    if (!magic_resist(ch, victim)) {
      affect_join(victim, &af, TRUE, FALSE);
      send_to_char("You feel your strength wither!\r\n", victim);
    }
    break;
  case SPELL_ARMOR:
    af.duration = 24;
    af.modifier = -20;
    af.location = APPLY_AC;

    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("You feel someone protecting you.\r\n", victim);
    break;
  case SPELL_BLESS:
    af.modifier = 2;
    af.duration = 6;
    af.location = APPLY_HITROLL;
    affect_join(victim, &af, TRUE, TRUE);

    af.location = APPLY_SAVING_MAGIC;
    af.modifier = 5;
    affect_join(victim, &af, TRUE, TRUE);

    send_to_char("You feel righteous.\r\n", victim);
    break;
    /*  case SPELL_IMPROVED_BLESS:
        af.modifier = 2;
        af.duration = 6;
        if (GET_LEVEL(ch) < LEVEL_WORSHIP)
          for (d = descriptor_list; d; d = d->next)
            if ((CAN_SEE(ch, d->character)) && (WORSHIPS(ch) == GET_IDNUM(d->character))) {
	      af.location = APPLY_DAMROLL;
	      affect_join(victim, &af, TRUE, TRUE);
	    }
        af.location = APPLY_HITROLL;
        affect_join(victim, &af, TRUE, TRUE);

        af.location = APPLY_SAVING_MAGIC;
        af.modifier = 5;
        affect_join(victim, &af, TRUE, TRUE);

        send_to_char("You feel righteous.\r\n", victim);
        break;*/
  case SPELL_BLINDNESS:
    if (IS_AFFECTED(victim, AFF_BLIND)) {
      send_to_char("Nothing seems to happen.\r\n", ch);
      return;
    }
    if (magic_resist(ch, victim)) return;
    if (GET_LEVEL(victim) > GET_LEVEL(ch) ||
	saves_spell(victim, SAVING_MAGIC, NULL, SAVE_NEGATE)) {
      send_to_char("You fail.\r\n", ch);
      return;
    }
    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\r\n", victim);

    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 2;
    af.bitvector = AFF_BLIND;
    affect_join(victim, &af, TRUE, TRUE);

    af.location = APPLY_AC;
    af.modifier = 40;
    affect_join(victim, &af, TRUE, TRUE);
    break;

  case SPELL_CURSE:
    if (magic_resist(ch, victim) ||
	saves_spell(victim, SAVING_MAGIC, NULL, SAVE_NEGATE))
      return;
    if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
	 send_to_char("Pick on someone your own size!\r\n", ch);
	 return;
    }
    af.duration = 1 + (GET_LEVEL(ch) >> 1);
    af.modifier = -1;
    af.location = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_join(victim, &af, TRUE, TRUE);

    af.modifier = -1;
    af.location = APPLY_HITROLL;
    affect_join(victim, &af, TRUE, TRUE);

    act("$n briefly glows red!", FALSE, victim, 0, 0, TO_ROOM);
    act("You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);
    break;

  case SPELL_DETECT_INVISIBLE:
    af.duration = 12 + level;
    af.bitvector = AFF_DETECT_INVIS;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("Your eyes tingle.\r\n", victim);
    break;

  case SPELL_DETECT_ALIGN:
    af.duration = 12 + level;
    af.bitvector = AFF_DETECT_ALIGN;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("Your eyes tingle.\r\n", victim);
    break;

  case SPELL_DETECT_MAGIC:
    af.duration = 12 + level;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("Your eyes tingle.\r\n", victim);
    break;

  case SPELL_DETECT_POISON:
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
	send_to_char("You can sense poison in your blood.\r\n", ch);
      else
	send_to_char("You feel healthy.\r\n", ch);
    else if (IS_AFFECTED(victim, AFF_POISON))
      act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
    else
      act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    break;
  case SPELL_INFRAVISION:
    af.duration = 12 + level;
    af.bitvector = AFF_INFRARED;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("Your eyes glow red.\r\n", victim);
    act("$n's eyes glow red.", TRUE, victim, 0, 0, TO_ROOM);
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;
    act("$n slowly fades out of existence.", TRUE, victim, 0, 0, TO_NOTVICT);
    send_to_char("You vanish.\r\n", victim);

    af.duration = 12 + (GET_LEVEL(ch) >> 2);
    af.modifier = -40;
    af.location = APPLY_AC;
    af.bitvector = AFF_INVISIBLE;
    affect_join(victim, &af, TRUE, TRUE);
    break;

  case SPELL_POISON:
    if (magic_resist(ch, victim) ||
	saves_spell(victim, SAVING_POISON, NULL, SAVE_NEGATE))
      return;

    af.duration = GET_LEVEL(ch);
    af.modifier = -2;
    af.location = APPLY_STR;
    af.bitvector = AFF_POISON;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("You feel very sick.\r\n", victim);
    act("$N gets violently ill!", TRUE, ch, NULL, victim, TO_NOTVICT);
    break;

  case SPELL_PROTECT_FROM_EVIL:
    af.duration = 24;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("You feel invulnerable!\r\n", victim);
    break;

  case SPELL_SANCTUARY:
    act("$n is surrounded by a white aura.", TRUE, victim, 0, 0, TO_ROOM);
    act("You start glowing.", TRUE, victim, 0, 0, TO_CHAR);

    af.duration = 4;
    af.bitvector = AFF_SANCTUARY;
    affect_join(victim, &af, TRUE, TRUE);
    break;

  case SPELL_SLEEP:
    /* Modified to be allowed under PKOK system or pk_allowed - Bod */
    if (!IS_NPC(ch) && !IS_NPC(victim)) {
      if (!pk_allowed) {
        if (!pkok_allowed)
          return;
        else if (!(PLR_FLAGGED(ch, PLR_PKOK) && PLR_FLAGGED(victim, PLR_PKOK)))
          return;
      }
    }
    if (GET_LEVEL(victim) > level ||
	saves_spell(victim, SAVING_MAGIC, NULL, SAVE_NEGATE) ||
	saves_spell(victim, SPELL_SLEEP, NULL, SAVE_NEGATE) ||
	magic_resist(ch, victim))
      return;

    af.duration = 4 + (GET_LEVEL(ch) >> 2);
    af.location = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, TRUE, TRUE);
    if (GET_POS(victim) > POS_SLEEPING) {
      act("You feel very sleepy...zzzzzz", FALSE, victim, 0, 0, TO_CHAR);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    send_to_char("You feel stronger!\r\n", victim);
    af.duration = (GET_LEVEL(ch) >> 1) + 4;
    af.modifier = 1 + (level > 18);
    af.location = APPLY_STR;
    affect_join(victim, &af, TRUE, FALSE);
    break;

  case SPELL_SENSE_LIFE:
    send_to_char("Your feel your awareness improve.\r\n", victim);
    af.duration = GET_LEVEL(ch);
    af.bitvector = AFF_SENSE_LIFE;
    affect_join(victim, &af, TRUE, TRUE);
    break;
    /*
       case SPELL_WATERWALK:
       af.type = SPELL_WATERWALK;
       af.duration = 24;
       af.bitvector = AFF_WATERWALK;
       affect_join(victim, &af, TRUE, TRUE);
       send_to_char("You feel webbing between your toes.\r\n", victim);
       break;
       */
  case SPELL_LEVITATION:
    send_to_char("You start to float in the air.\r\n", victim);
    act("$n starts to float in the air.", FALSE, victim, 0, 0, TO_ROOM);
    af.duration  = 20;
    af.bitvector = AFF_HOVER;
    affect_join(victim, &af, TRUE, TRUE);
    break;
  case SPELL_FLY:
    send_to_char("You start to fly.\r\n", victim);
    act("$n glows bright blue for a second and then starts flying.", TRUE, victim, 0, 0, TO_ROOM);
    af.duration  = 6;
    af.bitvector = AFF_FLY;
    affect_join(victim, &af, TRUE, TRUE);
    break;
  case SPELL_REGENERATION:
    send_to_char("You feel a growing health sensation.\r\n", victim);
    af.duration  = 6;
    af.bitvector = AFF_REGENERATION;
    affect_join(victim, &af, TRUE, TRUE);
    break;
  case SPELL_CAT_EYES:
    send_to_char("You get perfect nightvision.\r\n", ch);
    af.duration  = 15;
    af.bitvector = AFF_LIGHT;
    affect_join(victim, &af, TRUE, TRUE);
    break;
  case SPELL_VORPAL_PLATING:
    af.duration = 12;
    af.modifier = -30;
    af.location = APPLY_AC;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("You feel an invisible barrier protecting you.\r\n", victim);
    break;
  case SPELL_MAGE_GAUNTLETS:
    af.duration = 6;
    af.modifier = 2;
    af.location = APPLY_HITROLL;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("You feel an magical force guiding your hands.\r\n", victim);
    break;
  case SPELL_MYSTIC_SHIELD:
    af.duration = 24;
    af.modifier = -40;
    af.location = APPLY_AC;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("A magical shield of grey mist appears in front of you.\r\n", victim);
    break;
  case SPELL_MYSTICAL_COAT:
    af.duration = 48;
    af.modifier = -50;
    af.location = APPLY_AC;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("A magical coat of grey mist surrounds you.\r\n", victim);
    break;
  case SPELL_PHASE_BLUR:
    af.duration = 3;
    af.modifier = -40;
    af.location = APPLY_AC;
    affect_join(victim, &af, TRUE, TRUE);
    send_to_char("You start to blur.\r\n", victim);
    break;
  case SPELL_WATER_BREATHING:
       send_to_char("Gills magically appear on your neck.\r\n", victim);
       af.duration = 10 + level / 10;
       af.bitvector = AFF_BREATH_WATER;
       affect_join(victim, &af, TRUE, TRUE);
    break;
  case SPELL_HOLY_WRATH:
       send_to_char("You feel the strength of the gods.\r\n", victim);
       af.duration = 4;
       af.modifier = 1 + MIN(GET_LEVEL(ch) - 20, 80) / 10;
       af.location = APPLY_HITROLL;
       affect_join(victim, &af, TRUE, TRUE);
       /* removed -Helm
       af.modifier = 1 + MIN(GET_LEVEL(ch) - 20, 80) / 30;
       af.location = APPLY_DAMROLL;
       affect_join(victim, &af, TRUE, TRUE);*/
    break;

  default:
	sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_affects", spellnum);
	log(buf);
	return;
	break;
  }
}


/* Spells to remove affects from chars */
void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		   int spellnum, int type)
{
    int spell = 0;
    char *to_vict = '\0', *to_room = '\0';

    if (victim == NULL)
	return;

    switch (spellnum) {

    case SPELL_CURE_BLIND:
	spell = SPELL_BLINDNESS;
	to_vict = "Your vision returns!";
	break;
    case SPELL_HEAL:
	spell = SPELL_BLINDNESS;
	break;
    case SPELL_REMOVE_POISON:
	spell = SPELL_POISON;
	to_vict = "A warm feeling runs through your body!";
	to_room = "$n looks better.";
	break;
    case SPELL_REMOVE_CURSE:
	spell = SPELL_CURSE;
	to_vict = "You don't feel so cursed anymore.";
	break;
    default:
	sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects", spellnum);
	log(buf);
	return;
	break;
    }

    if (affected_by_spell(victim, spell)) {
	affect_from_char(victim, spell);
	if (to_vict != NULL) {
	    send_to_char(to_vict, victim);
	    send_to_char("\r\n", victim);
	}
	if (to_room != NULL)
	    act(to_room, TRUE, victim, NULL, NULL, TO_ROOM);
    } else if (to_vict != NULL)
	send_to_char("Nothing seems to happen.\r\n", ch);
}

/* Spells to create objects */
void mag_creations(int level, struct char_data * ch, int spellnum)
{
    struct obj_data *tobj;
    int z;

    if (ch == NULL)
	return;
    level = MAX(MIN(level, LEVEL_IMPL), 1);

    switch (spellnum) {

    case SPELL_CREATE_FOOD:
	z = 7090;
	break;
    default:
	send_to_char("Spell unimplemented, it would seem.\r\n", ch);
	sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_creations", spellnum);
	log(buf);
	return;
	break;
    }

    if (!(tobj = read_object(z, VIRTUAL))) {
	send_to_char("I seem to have goofed.\r\n", ch);
	sprintf(buf, "SYSERR: spell_creations, spell %d, obj %d: obj not found", spellnum, z);
	log(buf);
	return;
    }
    obj_to_char(tobj, ch);
    act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
}


/* Summon/Gate SPELLS here */
void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		 int spellnum, int savetype)
{
    struct char_data *mob = NULL;
    struct obj_data *tobj, *next_obj;
    struct message_type *messages = NULL;
    int nr, i, messnum;
    int pfail = 0;
    int num = 1;
    int mob_num = 0;
    int handle_corpse = 0;

    if (ch == NULL)
	return;

    /* Messages when you try to summon more pets than you can handle - Charlene*/
    const char *fail_control_mess[] =
    {
      "You feel so drained! You no longer have the strength to "
      "control your pets! Oh no! Look out!\r\n",
      "You have more pets than you can control! *FLEE*\r\n",
      "Oh god, what have you done! It's revenge of the pets!\r\n",
      "Before your very eyes %s turns into a ferocious monster out "
      "for mortal blood. Run for your lives!\r\n",
      "Acck! It's a mutant pet escapee from the Immortal realm! "
      "Somebody help!\r\n"
    };

    messnum = spellnum;

    switch (spellnum) {
    case SPELL_ANIMATE_DEAD:
	if ((obj == NULL) || (GET_ITEM_TYPE(obj) != ITEM_CONTAINER) ||
	    (!(GET_ITEM_VALUE(obj, 3) == 1))) {
	    act("You failed the summoning.", FALSE, ch, ch->equipment[WIELD], NULL, TO_CHAR);
	    return;
	}
	handle_corpse = TRUE;
	mob_num = 3703;
	pfail = 8;
	break;

    case SPELL_INSTANT_WOLF:
	mob_num = 3094;
	num = level / 12 + 1;
	break;

    case SPELL_INSTANT_SLAYER:
	mob_num = 17010;
	messnum = SPELL_INSTANT_WOLF;
	break;
    default:
	sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_summons", spellnum);
	log(buf);
	return;
	break;
    }

    assert(messnum > 0 && messnum < MAX_MESSAGES);

    if (fight_messages[messnum].number_of_attacks &&
	fight_messages[messnum].msg) {
      nr = dice(1, fight_messages[messnum].number_of_attacks);
      for (i = 1, messages = fight_messages[messnum].msg; (i < nr) && (messages); i++)
	messages = messages->next;
    }


    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("You are too giddy to have any followers!\r\n", ch);
	return;
    }

    if (number(0, 101) < pfail) {
	act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], NULL, TO_CHAR);
	return;
    }

    for (i = 0; i < num; i++) {
	if (!(mob = read_mobile(mob_num, VIRTUAL)))
	    return;

	act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], mob, TO_CHAR);
	act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], mob, TO_VICT);
	act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], mob, TO_ROOM);

	char_to_room(mob, ch->in_room);
	IS_CARRYING_W(mob) = 0;
	IS_CARRYING_N(mob) = 0;
	GET_GOLD(mob) = 0;

	/* Make sure that you can have more followers,
	/  if not you will lose control over them  - Charlene*/
	if (allow_follower(ch))
	{
	  SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
      add_follower(mob, ch);
	}
	else
	{
	  sprintf(buf, fail_control_mess[number(0,4)], GET_NAME(mob));
	  send_to_char(buf, ch);
	  sprintf(buf, "$n seems to be unable to control %s pets.", HSHR(ch));
	  act(buf, TRUE, ch, 0, 0, TO_ROOM);
	  hit(mob, ch, TYPE_UNDEFINED);
    }

	if (spellnum == SPELL_CLONE) {
	    strcpy(GET_NAME(mob), GET_NAME(ch));
	    strcpy(mob->player.short_descr, GET_NAME(ch));
	}
    }

    if (handle_corpse) {
	for (tobj = obj->contains; tobj; tobj = next_obj) {
	    next_obj = tobj->next_content;
	    obj_from_obj(tobj);
	    obj_to_char(tobj, mob);
	}
	extract_obj(obj);
    }

}


ASPELL(spell_teleport)
{
    int	to_room;
    extern int	top_of_world;      /* ref to the top element of world */

    ACMD(do_look);
    void	death_cry(struct char_data *ch);

    if (!ch)
	return;

    if (IS_SET(world[ch->in_room]->room_flags, GODROOM | ARENA)) {
      send_to_char("You can't teleport from here!\r\n", ch);
      return;
    }

    do {
	to_room = number(0, top_of_world);
    } while (ROOM_FLAGGED(to_room, NO_TELEPORT | PRIVATE | GODROOM | DEATH | ARENA | PKOK ));

    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);

    do_look(ch, "", 15, 0);
    roomflag_check(ch);
}


/* function: clone_char: Clones a char and return a pointer to the clone
 * Used in spells as clone, animate dead etc   -Petrus
 */
struct char_data *
clone_char(struct char_data *ch)
{
    struct char_data *clone;
    struct affected_type *af;
    int	i;
    char buf[100];

    if (IS_NPC(ch)) {
	clone = read_mobile(ch->nr, REAL);
    } else {

	CREATE(clone, struct char_data, 1);

	clear_char(clone);       /* Clear EVERYTHING! (ASSUMES CORRECT) */

	clone->player       = ch->player;
	clone->abilities    = ch->abilities;
	clone->tmpabilities = ch->tmpabilities;
	clone->points       = ch->points;

	/* specials can't be copied - only some values -Petrus */
	/* clear the whole specials */
	memset((char *)(&clone->specials), 0, sizeof(struct char_special_data));

	clone->specials.affected_by  = ch->specials.affected_by;
	clone->specials.position     = ch->specials.position;
	clone->mob_specials.default_pos  = ch->mob_specials.default_pos;
	/* end */

	clone->specials2    = ch->specials2;
	clone->skills = 0;

	/* SKILLS */
	CREATE(clone->mobskills, byte, MOB_SKILLS);
	for (i = 0; i < MOB_SKILLS && *skills[i] != '\n'; i++)
	    clone->mobskills[i] = ch->skills[i + SKILL_START];
	for (; i < MOB_SKILLS; i++)
	    clone->mobskills[i] = 0;

	/* Affects */
	for (af = ch->affected; af; af = af->next)
	    affect_to_char(clone, af);

	/* Equipment */
	for (i = 0; i < MAX_WEAR; i++)
	    clone->equipment[i] = 0;

	clone->desc = 0;

	GET_CLASS(clone) = 0;

	for (i = 0; i < 5; i++)
	    clone->specials2.resistances[i] = ch->specials2.resistances[i];

	for (i = 0; i < 3; i++)
	    GET_COND(clone, i) = GET_COND(ch, i);

	strcpy(buf, ch->player.name);
	strcat(buf, " clone");

	clone->player.name = strdup(buf);

	sprintf(buf, "%s the clone", GET_NAME(ch));
	clone->player.short_descr = strdup(buf);

	if (ch->player.long_descr)
	    clone->player.long_descr = strdup(ch->player.long_descr);

	clone->player.description = 0;
	/* REMEMBER EXTRA DESCRIPTIONS */

	clone->nr = -1;
	SET_BIT(clone->specials2.act, MOB_ISNPC);

	/* ATTACKS */
	CREATE(clone->mob_specials.attacks, struct attack_type, 2);
	clone->mob_specials.attacks[0].type = 400;
	clone->mob_specials.attacks[0].percent_of_use = 100;
	clone->mob_specials.attacks[0].damodice = 1;
	clone->mob_specials.attacks[0].damsizedice = 2;
	clone->mob_specials.attacks[0].damadd = 0;
	clone->mob_specials.attacks[0].damtype = 500;
	clone->mob_specials.attacks[1].type = 0;

	GET_HITROLL(clone) = (sbyte)(20 - get_thaco(ch));

	clone->followers = 0;
	clone->master = 0;
    }

    return clone;
}


ASPELL(spell_clone)
{
    struct char_data *clone;

    if (!victim)
	return;

    clone = clone_char(victim);

    insert_to_char_list(clone);

    char_to_room(clone, victim->in_room);
}


ASPELL(spell_control_weather)
{
   /* Control Weather is not possible here!!! */
   /* Better/Worse can not be transferred     */
}



ASPELL(spell_create_food)
{
    struct obj_data *tmp_obj;

    if (!ch)
	return;

    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);

    tmp_obj->name = strdup("mushroom");
    tmp_obj->short_description = strdup("a Magic Mushroom");
    tmp_obj->description = strdup("A really delicious looking magic mushroom lies here.");

    tmp_obj->obj_flags.type_flag = ITEM_FOOD;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
    tmp_obj->obj_flags.value[0] = 5 + level;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 10;
    tmp_obj->obj_flags.cost_per_day = 1;

    tmp_obj->next = object_list;
    object_list = tmp_obj;

    obj_to_room(tmp_obj, ch->in_room);

    tmp_obj->item_number = -1;

    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}



ASPELL(spell_create_water)
{
  int	water;

  void	name_to_drinkcon(struct obj_data *obj, int type);
  void	name_from_drinkcon(struct obj_data *obj);

  if (!ch || !obj)
    return;

  if ((GET_ITEM_TYPE(obj) == ITEM_DRINKCON) ||
      (GET_ITEM_TYPE(obj) == ITEM_FOUNTAIN)) {
    if ((obj->obj_flags.value[2] != LIQ_WATER)
	&& (obj->obj_flags.value[1] != 0)) {
      name_from_drinkcon(obj);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);

    } else {
      water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

      /* Calculate water it can contain, or water created */
      water = MIN(obj->obj_flags.value[0] - obj->obj_flags.value[1], water);

      if (water > 0) {
	obj->obj_flags.value[2] = LIQ_WATER;
	obj->obj_flags.value[1] += water;

	weight_change_object(obj);

	name_from_drinkcon(obj);
	name_to_drinkcon(obj, LIQ_WATER);
	act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  } /* if itemtype == DRINKCON */ else
    act("You try, but are unable to fill $p!", FALSE, ch, obj, 0, TO_CHAR);
}


ASPELL(spell_enchant_weapon)
{
   int	i;

   if (!ch || !obj || MAX_OBJ_AFFECT < 2)
       return;

   if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON ||
        GET_ITEM_TYPE(obj) == ITEM_FIREWEAPON) &&
       !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

      for (i = 0; i < MAX_OBJ_AFFECT; i++)
	 if (obj->affected[i].location != APPLY_NONE)
	    return;

      SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

      obj->affected[0].location = APPLY_HITROLL;
      obj->affected[0].modifier = 1 + (level >= 18);

      obj->affected[1].location = APPLY_DAMROLL;
      obj->affected[1].modifier = 1 + (level >= 20);

      if (IS_GOOD(ch)) {
	 SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
	 act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
      } else if (IS_EVIL(ch)) {
	 SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
	 act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
      } else {
	 act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
      }
   }
}


ASPELL(spell_locate_object)
{
    struct obj_data *i;
    int	j;

    if (!ch)
	return;

    j = level >> 1;

    for (i = object_list; i && (j > 0); i = i->next)
	if (isname(arg, i->name) &&
            !(IS_SET(i->obj_flags.extra_flags, ITEM_NOLOCATE) &&
             (GET_LEVEL(ch) < LEVEL_GREATER))) {
            if (i->carried_by) {
              sprintf(buf, "%s #Ncarried by %s.\r\n",
  		i->short_description, PERS(i->carried_by, ch));
              send_to_char(buf, ch);
            } else if (i->in_obj) {
              sprintf(buf, "%s #Nin %s.#N\r\n", i->short_description,
                i->in_obj->short_description);
              send_to_char(buf, ch);
            } else {
              sprintf(buf, "%s #Nin %s.#N\r\n", i->short_description,
                (i->in_room == NOWHERE ? "Used but uncertain" : world[i->in_room]->name));
              send_to_char(buf, ch);
          }
	    j--;
	}

    if (j == 0)
	send_to_char("You are very confused.\r\n", ch);
    if (j == level >> 1)
	send_to_char("No such object.\r\n", ch);
}


ASPELL(spell_locate_person)
{
  struct char_data *target;

  if(!(target = get_player_vis(ch, arg))) {
    send_to_char("Nobody by that name in the game.\r\n", ch);
    return;
  }

  if (IN_ROOM(target) != NOWHERE) {
    sprintf(buf, "%s is in %s.\r\n", GET_NAME(target), world[IN_ROOM(target)]->name);
    send_to_char(buf, ch);
  }

}

ASPELL(spell_word_of_recall)
{
    extern int	top_of_world;
    int	loc_nr, location;
    bool found = FALSE;

    ACMD(do_look);

    if (!victim || (IS_NPC(victim) && !victim->desc))
	return;

    if (GET_LEVEL(victim) >= LEVEL_DEITY)
      return;

    /*  loc_nr = GET_HOME(ch); */

    if (PLR_FLAGGED(victim, PLR_ARENA))
        return;

    loc_nr = 3001;
    for (location = 0; location <= top_of_world; location++)
	if (world[location]->number == loc_nr) {
	    found = TRUE;
	    break;
	}

    if ((location == top_of_world) || !found) {
	send_to_char("You are completely lost.\r\n", victim);
	return;
    }

    /* a location has been found. */

    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    do_look(victim, "", 0, 0);
}

ASPELL(spell_clan_recall)
{
    extern int	top_of_world;
    int	loc_nr = 0, location;
    bool found = FALSE;

    ACMD(do_look);

    if (!victim || (IS_NPC(victim) && !victim->desc))
	return;

    if ((GET_LEVEL(victim) >= LEVEL_DEITY) && (ch != victim))
      return;

    if (PLR_FLAGGED(victim, PLR_ARENA))
        return;

    if (CLAN(victim) < 0) {
	 send_to_char("You aren't in a clan!!!\r\n", victim);
	 return;
    }

    if (CLAN_LEVEL(victim) < 2) {
	 send_to_char("You aren't in the clan yet!!!\r\n", victim);
	 return;
    }

    loc_nr = clan_list[CLAN(victim)].recall;

    for (location = 0; location <= top_of_world; location++)
	if (world[location]->number == loc_nr) {
	    found = TRUE;
	    break;
	}

    if ((location == top_of_world) || !found) {
	send_to_char("You can't seem to find your clan...\r\n", victim);
	return;
    }

    /* a location has been found. */

    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    do_look(victim, "", 0, 0);
}


ASPELL(spell_summon)
{
  sh_int target;

  ACMD(do_look);

  if (!ch || !victim)
    return;

  if (GET_LEVEL(victim) >= LEVEL_DEITY) {
    send_to_char("Ok.\r\n", ch);
    send_to_char("You failed.\r\n", ch);
    return;
  }

  if (!pk_allowed) {
    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE) ||
	IS_AFFECTED(victim, AFF_CHARM) ||
	(IS_NPC(victim) && FIGHTING(victim))) {
      send_to_char("Nobody playing by that name.\r\n", ch);
      GET_MANA(ch) += 45; /* Match this with spell_parser.c */
      return;
    }

    if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)) {
      send_to_char("Ok.\r\n", ch);

      sprintf(buf, "%s just tried to summon you to: %s.\r\n"
	      "%s failed because you have summon protection on.\r\n"
	      "Type TOGGLE SUMMON to allow other players to summon you.\r\n",
	      GET_NAME(ch), world[ch->in_room]->name,
	      (ch->player.sex == SEX_MALE) ? "He" : "She");
      send_to_char(buf, victim);

      sprintf(buf, "You failed because %s has summon protection on.\r\n",
	      GET_NAME(victim));
      send_to_char(buf, ch);

      sprintf(buf, "%s failed summoning %s to %s.",
	      GET_NAME(ch), GET_NAME(victim), world[ch->in_room]->name);
      mudlog(buf, BRF, LEVEL_DEITY, TRUE);
      return;
    }
    if (!IS_NPC(victim) && ROOM_FLAGGED(IN_ROOM(ch), PKOK | ARENA)) {
      send_to_char("Ok.\r\n", ch);
      send_to_char("You can't summon a player to a PKOK/ARENA room.\r\n", ch);
      return;
    }
  }

  if (ROOM_FLAGGED(IN_ROOM(victim), LAWFULL)) {
    send_to_char("Ok.\r\n", ch);
    send_to_char("You can't summon a player from a lawful room.\r\n",ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), GODROOM)) {
    send_to_char("Ok.\r\n", ch);
    send_to_char("You can't summon a player to a GODROOM.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), NO_SUMMON)) {
    send_to_char("Ok.\r\n", ch);
    send_to_char("You can't summon a player to a NOSUMMON room.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(victim), NO_SUMMON)) {
    send_to_char("Ok.\r\n", ch);
    send_to_char("You can't summon a player from a NOSUMMON room.\r\n", ch);
    return;
  }
  if (PLR_FLAGGED(victim, PLR_KILLER)) {
    send_to_char("Ok.\r\n", ch);
    send_to_char("You can't summon a KILLER.\r\n", ch);
    return;
  }


  if (IS_NPC(victim) && saves_spell(victim, SPELL_SUMMON, NULL, SAVE_NEGATE)) {
    send_to_char("Nobody playing by that name.\r\n", ch);
    GET_MANA(ch) += 45; /* Match this with spell_parser.c */
    return;
  }


  send_to_char("Ok.\r\n", ch);

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  target = ch->in_room;
  char_from_room(victim);
  char_to_room(victim, target);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  do_look(victim, "", 15, 0);
  roomflag_check(victim);
}


ASPELL(spell_charm_person)
{
    struct affected_type af;
    int found = 0;

    if (!ch || !victim)
	return;

    /* By testing for IS_AFFECTED we avoid ie. Mordenkainens sword to be */
    /* able to be "recharmed" with duration                              */

    if (victim == ch) {
	send_to_char("You like yourself even better!\r\n", ch);
	return;
    }

    /* Disabled from EVER working on players as messes up their name! - Bod */
    if (!IS_NPC(ch) && !IS_NPC(victim)) /* a player charming another player */
        return; /* there's no legal reason to do this.  */

    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM) &&
	(level > GET_LEVEL(victim))) {
	if (elite_follow(victim, ch)) {
	    send_to_char("Sorry, following in loops is not allowed.\r\n", ch);
	    return;
	}

	if (!allow_follower(ch)) {
	  send_to_char("You can't have another follower.\r\n", ch);
	  return;
	}

	if (saves_spell(victim, SPELL_CHARM_PERSON, NULL, SAVE_NEGATE))
	    return;

	if (victim->master)
	    stop_follower(victim);

	add_follower(victim, ch);

	af.type      = SPELL_CHARM_PERSON;

	if (GET_INT(victim))
	    af.duration  = 6 * 18 / GET_INT(victim);
	else
	    af.duration  = 6 * 18;

	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char(victim, &af);

	act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
	if (!IS_NPC(victim))
	    REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);


	strcpy(buf, victim->player.name);
	if (strstr(buf, GET_NAME(ch))) found = 1;

	if (!found) {
	  sprintf(buf, "%s _%s_ _charm_", victim->player.name, GET_NAME(ch));

	  if ((victim->player.name) && (victim->player.name == mob_proto[victim->nr].player.name))
	    victim->player.name = NULL;

	  change_string(&victim->player.name, buf);
	}


    }
}


ASPELL(spell_identify)
{
    int	i, itmtype;
    bool found;

    char *good_or_bad[] = {
	"lousy",
	"bad",
	"normal",
	"high",
	"super",
	"n/a",
	"\n"
	};

    /* Spell Names */
    extern char	*spells[];

    /* For Objects */
    extern char	*item_types[];
    extern char	*extra_bits[];
    extern char	*apply_types[];
    extern char	*affected_bits[];

    if (!ch || (!obj && !victim))
	return;

    if (obj) {

	itmtype = GET_ITEM_TYPE(obj);

	send_to_char("You feel informed:\r\n", ch);
	sprintf(buf, "Object -='%s'=-, Item type: ", obj->name);
	sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
	strcat(buf, buf2);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	if (obj->obj_flags.bitvector) {
	    send_to_char("Item will give you following abilities:  ", ch);
	    sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
	    strcat(buf, "\r\n");
	    send_to_char(buf, ch);
	}

	send_to_char("Item is: ", ch);
	sprintbit(obj->obj_flags.extra_flags, extra_bits, buf);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	if (GET_ITEM_ANTICLASS(obj)) {
	    send_to_char("Anti Class: ", ch);
	    sprintbit(GET_ITEM_ANTICLASS(obj), &pc_class_types[1], buf);
	    strcat(buf, "\r\n");
	    send_to_char(buf, ch);
	}

	sprintf(buf, "Weight: [%d], Value: [%d] Min Level: [%d]\r\n",
		obj->obj_flags.weight, obj->obj_flags.cost, GET_ITEM_LEVEL(obj));
	send_to_char(buf, ch);

	switch (GET_ITEM_TYPE(obj)) {

	case ITEM_SCROLL :
	case ITEM_POTION :
	    sprintf(buf, "Level %d spells of:\r\n", 	obj->obj_flags.value[0]);
	    send_to_char(buf, ch);
	    if (obj->obj_flags.value[1] >= 1) {
		sprinttype(obj->obj_flags.value[1] - 1, spells, buf);
		strcat(buf, "\r\n");
		send_to_char(buf, ch);
	    }
	    if (obj->obj_flags.value[2] >= 1) {
		sprinttype(obj->obj_flags.value[2] - 1, spells, buf);
		strcat(buf, "\r\n");
		send_to_char(buf, ch);
	    }
	    if (obj->obj_flags.value[3] >= 1) {
		sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
		strcat(buf, "\r\n");
		send_to_char(buf, ch);
	    }
	    break;

	case ITEM_WAND :
	case ITEM_STAFF :
	    sprintf(buf, "Has %d charges, with %d charges left.\r\n",
		    obj->obj_flags.value[1],
		    obj->obj_flags.value[2]);
	    send_to_char(buf, ch);

	    sprintf(buf, "Level %d spell of:\r\n", 	obj->obj_flags.value[0]);
	    send_to_char(buf, ch);

	    if (obj->obj_flags.value[3] >= 1) {
		sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
		strcat(buf, "\r\n");
		send_to_char(buf, ch);
	    }
	    break;

	case ITEM_WEAPON :
        case ITEM_FIREWEAPON :
	    sprintf(buf, "Damage Dice is '%dD%d'\r\n",
		    obj->obj_flags.value[1],
		    obj->obj_flags.value[2]);
	    send_to_char(buf, ch);
	    break;

	case ITEM_ARMOR :
	    sprintf(buf, "AC-apply is %d\r\n",
		    obj->obj_flags.value[0]);
	    send_to_char(buf, ch);
	    break;

	}

	found = FALSE;

	for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	    if ((obj->affected[i].location != APPLY_NONE) &&
		(obj->affected[i].modifier != 0)) {
		if (!found) {
		    send_to_char("Can affect you as :\r\n", ch);
		    found = TRUE;
		}

		sprinttype(obj->affected[i].location, apply_types, buf2);
		sprintf(buf, "    Affects : %s By %d\r\n", buf2, obj->affected[i].modifier);
		send_to_char(buf, ch);
	    }
	}

    } else {       /* victim */

	if (!IS_NPC(victim)) {
	    sprintf(buf, "%d Years,  %d Months,  %d Days,  %d Hours old.\r\n",
		    age(victim).year, age(victim).month,
		    age(victim).day, age(victim).hours);
	    send_to_char(buf, ch);

	    sprintf(buf, "Height %din  Weight %dpounds \r\n",
		    GET_HEIGHT(victim), GET_WEIGHT(victim));
	    send_to_char(buf, ch);


	    sprintf(buf, "Str [%s] Int [%s] Wis [%s] Dex [%s] Con [%s] Cha [%s]\r\n",
		    good_or_bad[GET_STR(victim)/5],
		    good_or_bad[GET_INT(victim)/5],
		    good_or_bad[GET_WIS(victim)/5],
		    good_or_bad[GET_DEX(victim)/5],
		    good_or_bad[GET_CON(victim)/5],
		    good_or_bad[GET_CHA(victim)/5]);
	    send_to_char(buf, ch);

	} else {
	    send_to_char("You learn nothing new.\r\n", ch);
	}
    }
}

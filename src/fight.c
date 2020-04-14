/*************************************************************************
*   File: fight.c                                       Part of EliteMUD  *
*  Usage: Combat system                                                   *
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
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "screen.h"
#include "functions.h"

/* Structures */

struct char_data *combat_list = 0;	/* head of l-list of fighting chars */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */


/* External structures */

extern struct room_data **world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int	pk_allowed;	/* see config.c */
extern int	pkok_allowed;   /* see config.c */
extern int	auto_save;	/* see config.c */
extern struct ama_info_type ama_info[78];
extern void info_line(struct char_data *ch, struct char_data *victim, char *message);

/* External procedures */

ACMD(do_flee);

struct ama_info_type *ama_msg;

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
   { "hit",   "hits" },
   { "pound", "pounds" },
   { "pierce", "pierces" },
   { "slash", "slashes" },
   { "blast", "blasts" },
   { "whip", "whips" },
   { "pierce", "pierces" },
   { "claw", "claws" },
   { "bite", "bites" },
   { "sting", "stings" },
   { "crush", "crushes" }           /* TYPE_CRUSH    */

/* { "stab", "stabs" },
   { "chop", "chops" } */
};



/* The Fight related routines */

void	appear(struct char_data *ch)
{
   act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);

   if (affected_by_spell(ch, SPELL_INVISIBLE))
      affect_from_char(ch, SPELL_INVISIBLE);

   REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

void   fall_of_mount(struct char_data *ch)
{
     if (ch->specials.mounting) {
	  act("You fall off $N", FALSE, ch, 0, ch->specials.mounting, TO_CHAR);
	  act("$n falls off from you.", TRUE, ch, 0, ch->specials.mounting, TO_VICT);
	  act("$n falls off from $N.", TRUE, ch, 0, ch->specials.mounting, TO_NOTVICT);
	  (ch->specials.mounting)->specials.mounted_by = 0;
	ch->specials.mounting = 0;
     }

     if (ch->specials.mounted_by) {
	  act("$N falls off from you.", TRUE, ch, 0, ch->specials.mounted_by, TO_CHAR);
	  act("You fall off $n.", TRUE, ch, 0, ch->specials.mounted_by, TO_VICT);
	  act("$N falls off $n.", TRUE, ch, 0, ch->specials.mounted_by, TO_NOTVICT);
	  (ch->specials.mounted_by)->specials.mounting = 0;
	  ch->specials.mounted_by = 0;
     }
}


void	load_messages(void)
{
    FILE * f1;
    int	i, type;
    struct message_type *messages;
    char	chk[128];

    if (!(f1 = fopen(MESS_FILE, "r"))) {
	sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
	perror(buf2);
	exit(0);
    }

    for (i = 0; i < MAX_MESSAGES; i++) {
	fight_messages[i].number_of_attacks = 0;
	fight_messages[i].msg = 0;
    }

    fgets(chk, 128, f1);
    while (!feof(f1) && (*chk == '\n' || *chk == '*'))
	fgets(chk, 128, f1);

    while (*chk == 'M') {
	fgets(chk, 128, f1);
	sscanf(chk, " %d\n", &type);

	if (type < 1 || type >= MAX_MESSAGES) {
	    sprintf(buf2, "Error reading combat messages. Nr %d out of bounds.", type);
	    perror(buf2);
	    exit(0);
	}

	CREATE(messages, struct message_type, 1);
	fight_messages[type].number_of_attacks++;
	messages->next = fight_messages[type].msg;
	fight_messages[type].msg = messages;

	sprintf(buf2, "combat message #%d in file '%s'", type, MESS_FILE);

	messages->die_msg.attacker_msg      = fread_string(f1, buf2);
	messages->die_msg.victim_msg        = fread_string(f1, buf2);
	messages->die_msg.room_msg          = fread_string(f1, buf2);
	messages->miss_msg.attacker_msg     = fread_string(f1, buf2);
	messages->miss_msg.victim_msg       = fread_string(f1, buf2);
	messages->miss_msg.room_msg         = fread_string(f1, buf2);
	messages->hit_msg.attacker_msg      = fread_string(f1, buf2);
	messages->hit_msg.victim_msg        = fread_string(f1, buf2);
	messages->hit_msg.room_msg          = fread_string(f1, buf2);
	fgets(chk, 128, f1);
	while (!feof(f1) && (*chk == '\n' || *chk == '*'))
	    fgets(chk, 128, f1);
    }

    fclose(f1);
}



void	update_pos( struct char_data *victim )
{
     if (IS_NPC(victim)) {
	  if (GET_MOB_WAIT(victim) > 0) {
	       if (GET_HIT(victim) <= -11)
		    GET_POS(victim) = POS_DEAD;
	       else if (GET_HIT(victim) <= -6)
		    GET_POS(victim) = POS_MORTALLYW;
	       else if (GET_HIT(victim) <= -3)
		    GET_POS(victim) = POS_INCAP;
	       else if (GET_HIT(victim) <= 0)
		    GET_POS(victim) = POS_STUNNED;

	       return;
	  }
     } /* Mobs with wait states do not stand automatically */

     /* Players or Mobs without wait states */
     if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
	  return;
     else if ((GET_HIT(victim) > 0))
	  GET_POS(victim) = POS_STANDING;
     else if (GET_HIT(victim) <= -11)
	  GET_POS(victim) = POS_DEAD;
     else if (GET_HIT(victim) <= -6)
	  GET_POS(victim) = POS_MORTALLYW;
     else if (GET_HIT(victim) <= -3)
	  GET_POS(victim) = POS_INCAP;
     else
	  GET_POS(victim) = POS_STUNNED;

     fall_of_mount(victim);
}

/* Small change by Petrus */
/* Modified for PKOK System by Bod */
void	check_killer(struct char_data *ch, struct char_data *vict)
{
    if (!ROOM_FLAGGED(IN_ROOM(vict), PKOK) &&
	!PLR_FLAGGED(vict, PLR_KILLER | PLR_THIEF)) {
      if (!pkok_allowed) {
  	  char buf[200];

	  SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
	  sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
	  	  GET_NAME(ch), GET_NAME(vict), world[vict->in_room]->name);
	  mudlog(buf, BRF, LEVEL_DEITY, TRUE);
	  send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);
      }
      else if (!PLR_FLAGGED(ch, PLR_PKOK) || !PLR_FLAGGED(vict, PLR_PKOK)) {
  	  char buf[200];

	  SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
	  sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
	  	  GET_NAME(ch), GET_NAME(vict), world[vict->in_room]->name);
	  mudlog(buf, BRF, LEVEL_DEITY, TRUE);
	  send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);
      }
    }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void	set_fighting(struct char_data *ch, struct char_data *vict)
{

   if (ch == vict)
      return;

   assert(!ch->specials.fighting);

   ch->next_fighting = combat_list;
   combat_list = ch;

   if (IS_AFFECTED(ch, AFF_SLEEP))
      affect_from_char(ch, SPELL_SLEEP);

   ch->specials.fighting = vict;
   if (!IS_NPC(ch))
	GET_POS(ch) = POS_FIGHTING;
   /*   if (!IS_NPC(ch->specials.fighting))
	GET_POS(ch->specials.fighting) = POS_FIGHTING;
   */
   ch->specials.timer = 0;

/*  Not necessary _Petrus
 *   if (!pk_allowed)
 *     check_killer(ch, vict);
 */
}



/* remove a char from the list of fighting chars. Changed by Petrus
 */
void  stop_fighting(struct char_data *ch)
{
    struct char_data *tmp;

    assert(ch->specials.fighting);

    if (ch == combat_next_dude)
	combat_next_dude = ch->next_fighting;

    if (combat_list == ch)
	combat_list = ch->next_fighting;
    else {
	for (tmp = combat_list; tmp && (tmp->next_fighting != ch);
	     tmp = tmp->next_fighting)
	    ;
	if (!tmp) {
	    log("SYSERR: Char fighting not found Error (fight.c, stop_fighting)");
	    abort();
	}
	tmp->next_fighting = ch->next_fighting;
    }

    ch->next_fighting = 0;
    ch->specials.fighting = 0;
    GET_POS(ch) = POS_STANDING;

    if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_BERZERK)) {
	send_to_char("You become humble again.\r\n", ch);
	act("$n becomes humble again.", TRUE, ch, 0, 0, TO_ROOM);
        affect_from_char(ch, SKILL_BERZERK);
        /* Stop players from using berzerk as a heal or for
           accumulating fake hp / move */
        if (GET_HIT(ch) > MIN(GET_LEVEL(ch), 100))
          GET_HIT(ch) -= MIN(GET_LEVEL(ch), 100);
        else if (GET_HIT(ch) > -3) { /* Changed so dead players don't die a 2nd time */
            GET_HIT(ch) = -3;
            GET_POS(ch) = POS_INCAP;
            send_to_char("Your wounds are too numerous and you fall to the ground.\r\n", ch);
            act("$n wounds are too numerous and they fall to the ground in a bloody heap.",
                 TRUE, ch, 0, 0, TO_ROOM);
        }
        if (GET_MOVE(ch) > MIN(GET_LEVEL(ch)/2, 50))
          GET_MOVE(ch) -= MIN(GET_LEVEL(ch)/2, 50);
        else
          GET_MOVE(ch) = 0;
    }

    update_pos(ch);

    if (ch->desc)
	line_to_screen(ch->desc);
}




void	make_corpse(struct char_data *ch)
{
    struct obj_data *corpse, *o;
    struct obj_data *money;
    int	i;
    extern int max_npc_corpse_time, max_pc_corpse_time, max_recall_level;
    extern sh_int r_newbie_corpse_room;

    struct obj_data *create_money(int amount);

    CREATE(corpse, struct obj_data, 1);
    clear_object(corpse);

    corpse->item_number = NOWHERE;
    corpse->in_room = NOWHERE;

    sprintf(buf2, "The corpse of %s is lying here", GET_NAME(ch));
    if (GET_HIT(ch) > -20)
	strcat(buf2, ".");
    else if (GET_HIT(ch) > -40)
	strcat(buf2, ", looking mutilated.");
    else if (GET_HIT(ch) > -80)
	strcat(buf2, ", or rather parts of it.");
    else
	strcpy(buf2, "The remains of something or someone is lying here.");
    corpse->description = strdup(buf2);

    sprintf(buf2, "the corpse of %s", GET_NAME(ch));
    corpse->short_description = strdup(buf2);

    corpse->contains = ch->carrying;
    if (GET_GOLD(ch) > 0) {
	/* following 'if' clause added to fix gold duplication loophole */
	if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
	    money = create_money(GET_GOLD(ch));
	    obj_to_obj(money, corpse);
	}
	GET_GOLD(ch) = 0;
    }

    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags = ITEM_TAKE;
    corpse->obj_flags.extra_flags = ITEM_NODONATE | ITEM_NOSWEEP;
    corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
    corpse->obj_flags.weight = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
    corpse->obj_flags.cost_per_day = 100000;
    corpse->obj_flags.cost = GET_LEVEL(ch) * 50;

    corpse->obj_flags.value[4] = number(1, 6) + 3;  /* phobos blood */
    corpse->obj_flags.value[5] = MAX(1, GET_LEVEL(ch));  /* phobos ripe */

    if (IS_NPC(ch)) {
        corpse->name = strdup("corpse");
	corpse->obj_flags.timer = max_npc_corpse_time;
	corpse->obj_flags.value[3] = 1; /* corpse identifyer */
    } else {
        corpse->name = strdup("corpse pcorpse");
	corpse->obj_flags.timer = max_pc_corpse_time;
	corpse->obj_flags.value[3] = 2; /* plr corpse identifyer */
    }

    for (i = 0; i < MAX_WEAR; i++)
	if (ch->equipment[i])
	    obj_to_obj(unequip_char(ch, i), corpse);

    ch->carrying = 0;
    IS_CARRYING_N(ch) = 0;
    IS_CARRYING_W(ch) = 0;

    corpse->next = object_list;
    object_list = corpse;

    for (o = corpse->contains; o; o->in_obj = corpse, o = o->next_content)
	 ;

    if(IS_NPC(ch)) set_key_timer(corpse);

    object_list_new_owner(corpse, 0);

    if (!IS_NPC(ch) && (REMORT(ch) < 1 && GET_LEVEL(ch) <= max_recall_level))
      obj_to_room(corpse, r_newbie_corpse_room);
    else
      obj_to_room(corpse, ch->in_room);
}


/* New corpse routine to create corpses for PK chars but not take equipment */
void    make_pk_corpse(struct char_data *ch)
{
    struct obj_data *corpse;
    extern int max_pc_corpse_time;

    CREATE(corpse, struct obj_data, 1);
    clear_object(corpse);

    corpse->item_number = NOWHERE;
    corpse->in_room = NOWHERE;

    sprintf(buf2, "The corpse of %s is lying here", GET_NAME(ch));
    if (GET_HIT(ch) > -20)
        strcat(buf2, ".");
    else if (GET_HIT(ch) > -40)
        strcat(buf2, ", looking mutilated.");
    else if (GET_HIT(ch) > -80)
        strcat(buf2, ", or rather parts of it.");
    else
        strcpy(buf2, "The remains of something or someone is lying here.");
    corpse->description = strdup(buf2);

    sprintf(buf2, "the corpse of %s", GET_NAME(ch));
    corpse->short_description = strdup(buf2);

    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags = ITEM_TAKE;
    corpse->obj_flags.extra_flags = ITEM_NODONATE | ITEM_NOSWEEP;
    corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
    corpse->obj_flags.weight = GET_WEIGHT(ch);
    corpse->obj_flags.cost_per_day = 100000;
    corpse->obj_flags.cost = GET_LEVEL(ch) * 50;

    corpse->obj_flags.value[4] = number(1, 6) + 3;  /* phobos blood */
    corpse->obj_flags.value[5] = MAX(1, GET_LEVEL(ch));  /* phobos ripe */

    corpse->name = strdup("corpse pcorpse");
    corpse->obj_flags.timer = max_pc_corpse_time;
    corpse->obj_flags.value[3] = 2; /* plr corpse identifyer */

    corpse->next = object_list;
    object_list = corpse;

    obj_to_room(corpse, ch->in_room);
}


/* When ch kills victim */
void change_alignment(struct char_data * ch, struct char_data * victim)
{
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 4;

  /* Special for Demons */
  if (GET_RACE(ch) == RACE_DEMON)
    stat_check(ch);
  /* Special for Angels */
  if (GET_RACE(ch) == RACE_ANGEL) {
    stat_check(ch);
    innate_check(ch);
  }
}


void	death_cry(struct char_data *ch)
{
   int	door, was_in;

   act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
   was_in = ch->in_room;

   if (was_in != NOWHERE) {
	for (door = 0; door < NUM_OF_DIRS; door++) {
	     if (CAN_GO(ch, door)) {
		  ch->in_room = world[was_in]->dir_option[door]->to_room;
		  act("Your bones shiver as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
		  ch->in_room = was_in;
	     }
	}
   }
}



void raw_kill(struct char_data * ch, struct char_data *killer)
{
  extern sh_int r_mortal_start_room;

  if (ch->specials.fighting)
    stop_fighting(ch);

  while (ch->affected  && ch->affected->duration != DURATION_INNATE)
    affect_remove(ch, ch->affected);

  death_cry(ch);

  /* if (killer)
    mprog_death_trigger(ch, killer);
  */

  if (IS_NPC(ch)) {
    make_corpse(ch);
    extract_char(ch);
  } else {
    Crash_crashsave(ch, BACKUP_SAVE);

    if (IN_ROOM(ch) != NOWHERE)
      if (ROOM_FLAGGED(IN_ROOM(ch), PKOK) ||
	  PLR_FLAGGED(ch, PLR_KILLER)) {

	REMOVE_BIT(PLR_FLAGS(ch), PLR_KILLER);
        make_pk_corpse(ch);
	char_from_room(ch);
	char_to_room(ch, r_mortal_start_room);
	Crash_quitsave(ch);
	extract_char(ch);
	return;
      }

    if (killer) {
      if (PLR_FLAGGED(killer, PLR_KILLER)) {
        make_pk_corpse(ch);
	char_from_room(ch);
	char_to_room(ch, r_mortal_start_room);
	Crash_quitsave(ch);
	extract_char(ch);
	return;
      }
    }

    make_corpse(ch);
    Crash_crashsave(ch, DELETE_SAVE);
    extract_char(ch);
  }
}




void die(struct char_data * ch, struct char_data *killer)
{
  gain_exp(ch, -(exp_needed(ch) / 2));
  raw_kill(ch, killer);
}


int group_xp_value(struct char_data *ch)
{
    int value;

    value = (int)GET_LEVEL(ch) * (int)GET_LEVEL(ch) + 25;

    if (GET_LEVEL(ch) >= LEVEL_DEITY)
	value *= (int)GET_LEVEL(ch);

    return value;
}


void	group_gain(struct char_data *ch, struct char_data *victim)

{
  int no_members, maxlevel;
  int share, maxneed, gain;
  struct char_data *k;
  struct follow_type *f;

  k = (ch->master ? ch->master : ch);

  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room)) {
    maxlevel = GET_LEVEL(k);
    maxneed = group_xp_value(k);
    no_members = 1;
  } else {
    maxneed = group_xp_value(ch);
    maxlevel = GET_LEVEL(ch);
    no_members = 0;
  }

  for (f = k->followers; f; f = f->next) {
    if(IS_AFFECTED(f->follower,AFF_GROUP) &&
       (f->follower)->in_room == ch->in_room) {
      no_members++;
      if (group_xp_value(f->follower) > maxneed) {
	maxneed = group_xp_value(f->follower);
	maxlevel = GET_LEVEL(f->follower);
      }
    }
  }

  if (no_members > 1)
    share = GET_EXP(victim) / (2 + no_members);	/* Group Penalty */
  else if (no_members == 1)
    share = GET_EXP(victim) / 4; /* single member */
  else
    share = 0;

  /* Implement level diff. here  -Petrus */


  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room) && share > 0) {
    gain = (int)((float)share * (float)group_xp_value(k) / (float)maxneed);
    gain = MAX(1, gain);
    gain_exp(k, gain);
    change_alignment(k, victim);

    SET_BIT(PLR_FLAGS(k), PLR_SAVECHR);
  }

  for (f = k->followers; f; f = f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room)) {
      if (share > 1) {
	gain = (int)((float)share * (float)group_xp_value(f->follower) / (float)maxneed);
	gain = MAX(1, gain);
	gain_exp(f->follower, (int)gain);
	change_alignment(f->follower, victim);

	SET_BIT(PLR_FLAGS(f->follower), PLR_SAVECHR);
      }
    }
  }
}

char *
replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
  static char	buf[256];
  char	*cp;

  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W' :
	for (; *weapon_plural; *(cp++) = *(weapon_plural++)) ;
	break;
      case 'w' :
	for (; *weapon_singular; *(cp++) = *(weapon_singular++)) ;
	break;
      default :
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  } /* For */

  return(buf);
}



void	dam_message(int dam, struct char_data *ch, struct char_data *victim,
int w_type)
{
  struct obj_data *wield;
  char	*buf;
  int	msgnum, gag = 0, procent;

  static struct dam_weapon_type {
    char	*to_room;
    char	*to_char;
    char	*to_victim;
  } dam_weapons[] = {

    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

    { "$n misses $N with $s #w.", /* 0: 0     */
	"§1GYou miss $N with your #w.##N",
	"§1g$n misses you with $s #w.##N" },

    { "$n barely #W $N.",	/* 1: 1..2  */
	"§1RYou barely #w $N.##N",
	"§1r$n barely #W you.##N" },

    { "$n scratches $N with $s #w.", /* 2: 3..4  */
	"§1RYou scratch $N as you #w $M.##N",
	"§1r$n scratches you as $e #W you.##N" },

    { "$n #W $N.",		/* 3: 5..6  */
	"§1RYou #w $N.##N",
	"§1r$n #W you.##N" },

    { "$n #W $N hard.",		/* 4: 7..10  */
	"§1RYou #w $N hard.##N",
	"§1r$n #W you hard.##N" },

    { "$n #W $N very hard.",	/* 5: 11..14  */
	"§1RYou #w $N very hard.##N",
	"§1r$n #W you very hard.##N" },

    { "$n #W $N extremely hard.", /* 6: 15..19  */
	"§1RYou #w $N extremely hard.##N",
	"§1r$n #W you extremely hard.##N" },

    { "$n massacres $N to small fragments with $s #w.",	/* 7: 19..23 */
	"§1RYou massacre $N to small fragments with your #w.##N",
	"§1r$n massacres you to small fragments with $s #w.##N" },

    { "$n obliterates $N with $s deadly #w!", /* 8: 23..28  */
	"§1RYou obliterate $N with your deadly #w!##N",
	"§1r$n obliterates you with $s deadly #w!##N" },

    { "$n ANNIHILATES $N with $s wicked #w!!", /* 9: 29..33  */
	"§1RYou ANNIHILATE $N with your wicked #w!!##N",
	"§1r$n ANNIHILATES you with $s wicked #w!!##N" },

    { "$n ATOMIZES $N with $s cruel #w!!!", /* 10: > 34   */
	"§1RYou ATOMIZE $N with your cruel #w!!!##N",
	"$n ATOMIZES you with $s cruel #w!!!##N" },

    { "$n PAINTS THE WALLS WITH $N's head with $s mindblowing #w!!!",
	"§1RYou PAINT THE WALLS with $N's head with your mindblowing #W!!!##N",
	"§1r$n PAINTS THE WALLS with your head with $s mindblowing #w!!!##N"} /*>40*/

  };



  w_type -= TYPE_HIT;		/* Change to base of table with text */
  procent = dam*100/(MAX(1, GET_MAX_HIT(victim)));
  wield = ch->equipment[WIELD];

  if (dam == 0) {
    msgnum = 0;
    gag = ACT_GAG;
  }
  else if (procent < 1)	msgnum = 1;
  else if (procent < 2)	msgnum = 2;
  else if (procent < 3)	msgnum = 3;
  else if (procent < 5)	msgnum = 4;
  else if (procent < 8)	msgnum = 5;
  else if (procent < 13)	msgnum = 6;
  else if (procent < 21)	msgnum = 7;
  else if (procent < 34)	msgnum = 8;
  else if (procent < 55)       msgnum = 9;
  else if (procent < 89)       msgnum = 10;
  else                 msgnum = 11;

  if (w_type == TYPE_AMA - TYPE_HIT) {
    /* damage message to onlookers */
    buf = replace_string(dam_weapons[msgnum].to_room,
			 ama_msg->sin, ama_msg->plu);
    act(buf, FALSE, ch, wield, victim, TO_NOTVICT | gag);

    /* damage message to damager */
    buf = replace_string(dam_weapons[msgnum].to_char,
			 ama_msg->sin, ama_msg->plu);
    act(buf, FALSE, ch, wield, victim, TO_CHAR | gag);

    /* damage message to damagee */
    buf = replace_string(dam_weapons[msgnum].to_victim,
			 ama_msg->sin, ama_msg->plu);
    act(buf, FALSE, ch, wield, victim, TO_VICT | gag);
  } else {

    /* damage message to onlookers */
    buf = replace_string(dam_weapons[msgnum].to_room,
			 attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
    act(buf, FALSE, ch, wield, victim, TO_NOTVICT | gag);

    /* damage message to damager */
    buf = replace_string(dam_weapons[msgnum].to_char,
			 attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
    act(buf, FALSE, ch, wield, victim, TO_CHAR | gag);

    /* damage message to damagee */
    buf = replace_string(dam_weapons[msgnum].to_victim,
			 attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
    act(buf, FALSE, ch, wield, victim, TO_VICT | gag);
  }
}

void
damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype)
{
  struct message_type *messages;
  int	i, j, nr, exp, origdam;

  ACMD(do_protect); ACMD(do_rescue);

  if (GET_POS(victim) <= POS_DEAD) {
    sprintf(buf, "SYSERR: Attempt to damage a corpse. ATT: %s  VICT: %s",
	    GET_NAME(ch), GET_NAME(victim));
    log(buf);
    /* die(victim, ch); */ /* This Crashes the MUD -Helm */
    return;			/* -Petrus 941009 */
  }

  /* You can't damage an immortal! */
  if ((GET_LEVEL(victim) >= LEVEL_DEITY) && !IS_NPC(victim) &&
      GET_LEVEL(ch) < LEVEL_DEITY)
    dam = 0;

  if(IS_SET(world[ch->in_room]->room_flags, LAWFULL) && ch != victim) {
    send_to_char("No.  You can't bring yourself to hurt someone here, in this place.\r\n", ch);
    return;
  }

  if (victim != ch) {
    if (GET_POS(ch) > POS_STUNNED) {
      if (!(ch->specials.fighting))
	set_fighting(ch, victim);

      /* This looks a little weird  -Petrus */
      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	  !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	if (ch->specials.fighting)
	  stop_fighting(ch);
	hit(ch, victim->master, TYPE_UNDEFINED);
	return;
      }
    }

    if (GET_POS(victim) >= POS_STUNNED) {
      if (!(victim->specials.fighting))
	set_fighting(victim, ch);
      if (IS_NPC(victim) && IS_SET(victim->specials2.act, MOB_MEMORY) &&
	  !IS_NPC(ch) && (GET_LEVEL(ch) < LEVEL_DEITY))
	remember(victim, ch);

      /* Mob Switching by Petrus */
      if (IS_NPC(victim) && ch != victim->specials.fighting) {
	i = GET_LEVEL(ch) - GET_LEVEL(victim->specials.fighting);
	if (!number(0, 20 - MAX(10, MIN(15, i))))
	  victim->specials.fighting = ch;
      }
    }
    /* Small change by Petrus */
    if (!pk_allowed && !IS_NPC(ch) &&
	!IS_NPC(victim) && !PLR_FLAGGED(ch, PLR_KILLER)) {
      check_killer(ch, victim);

    }
  }


  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    appear(ch);

  origdam = dam; /* For Damage Display report: PLR_DETAIL) - Bod */

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam = dam / 2;		/* 1/2 damage when sanctuary */

  dam = MIN(dam, 500);		/* Maxdam changed to 500 - Petrus */
  dam = MAX(dam, 0);

  if ((i = saves_spell(victim, attacktype, NULL, SAVE_SPECIAL)))
    dam = (dam * (100 - i)/100);

  GET_HIT(victim) -= dam;

  /* -Bod (Modified Helm's basic display dam value) */
  if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_TEST)) {
    if (PLR_FLAGGED(ch, PLR_DETAIL)) {
      if (dam || !PRF_FLAGGED(ch, PRF_GAG)) {
        sprintf(buf, "(Orig: #G%d#N) #w<<#R%d#w>>#N ]\r\n", origdam, dam);
        send_to_char(buf, ch);
      }
    } else {
      if (dam || !PRF_FLAGGED(ch, PRF_GAG)) {
        sprintf(buf, "#w<<#R%d#w>>#N ", dam);
        send_to_char(buf, ch);
      }
    }
  }

  if (!IS_NPC(victim) && PLR_FLAGGED(victim, PLR_TEST)) {
    if (PLR_FLAGGED(victim, PLR_DETAIL)) {
      if (dam || !PRF_FLAGGED(victim, PRF_GAG)) {
        sprintf(buf, "(Orig: #w%d#N) >>#r%d#N<<\r\n", origdam, dam);
        send_to_char(buf, victim);
      }
    } else {
      if (dam || !PRF_FLAGGED(victim, PRF_GAG)) {
        sprintf(buf, ">>#r%d#N<< ", dam);
        send_to_char(buf, victim);
      }
    }
  }


  if ((ch != victim) && (GET_LEVEL(ch) < LEVEL_DEITY) && !(!IS_NPC(ch) && !IS_NPC(victim)))
    GET_EXP(ch) += GET_LEVEL(victim) * dam / 2;


  update_pos(victim);


  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_AMA) &&
      dam > 0 && GET_POS(victim) != POS_DEAD) {
    dam_message(dam, ch, victim, attacktype);
  } else {
    assert(attacktype > 0 && attacktype < MAX_MESSAGES);

    if(attacktype == TYPE_AMA) {
      if (dam != 0) {
        sprintf(buf, "$n %s $N into oblivion.", ama_msg->plu);
        act(buf, FALSE, ch, 0, victim, TO_NOTVICT);
	sprintf(buf, "#yYou %s $N into oblivion.#N", ama_msg->sin);
	act(buf, FALSE, ch, 0, victim, TO_CHAR);
	sprintf(buf, "#r$n %s you into oblivion.#N", ama_msg->plu);
	act(buf, FALSE, ch, 0, victim, TO_VICT);
      } else {/* Dam == 0 */
        sprintf(buf, "$n misses $N with $s %s.", ama_msg->sin);
        act(buf, FALSE, ch, 0, victim, TO_NOTVICT | ACT_GAG);
	sprintf(buf, "#RYour %s misses $N by miles.#N", ama_msg->sin);
	act(buf, FALSE, ch, 0, victim, TO_CHAR);
	sprintf(buf, "#r$n misses you with $s %s.#N", ama_msg->sin);
	act(buf, FALSE, ch, 0, victim, TO_VICT);
      }
    }

    if (fight_messages[attacktype].number_of_attacks && fight_messages[attacktype].msg) {
      nr = dice(1, fight_messages[attacktype].number_of_attacks);
      for (j = 1, messages = fight_messages[attacktype].msg; (j < nr) && (messages); j++)
	messages = messages->next;

      if (dam != 0) {
	if (GET_POS(victim) == POS_DEAD) {
	  sprintf(buf, "§1y%s#N", messages->die_msg.attacker_msg);
	  act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	  sprintf(buf, "§1r%s#N", messages->die_msg.victim_msg);
	  act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	  act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	} else {
	  sprintf(buf, "§1R%s#N", messages->hit_msg.attacker_msg);
	  act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	  sprintf(buf, "§1r%s#N", messages->hit_msg.victim_msg);
	  act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	  act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	}
      } else {		/* Dam == 0 */
	act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR | ACT_GAG);
	sprintf(buf, "§1g%s#N", messages->miss_msg.victim_msg);
	act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT | ACT_GAG);
	act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT | ACT_GAG);
      }
    }
  }

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are incapacitated and will slowly die, if not aided.\r\n", victim);
    break;
  case POS_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
    break;
  case POS_DEAD:
    act("$n is dead!  R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are dead!  Sorry...\r\n", victim);
    break;

  default:			/* >= POS SLEEPING */
    if (dam > (GET_MAX_HIT(victim) / 5))
      act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

    if (IS_AFFECTED(victim, AFF_SLEEP)) {
	 affect_from_char(victim, SPELL_SLEEP);
         send_to_char("You wake up a little groggy...\r\n", victim);
	 GET_POS(victim) = POS_FIGHTING;
    }

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 5)) {
      act("You wish that your wounds would stop BLEEDING so much!", FALSE, victim, 0, 0, TO_CHAR);
      if (MOB_FLAGGED(victim, MOB_WIMPY) && (GET_MOB_WAIT(victim) == 0))
	do_flee(victim, "", 0, 0);
    }

    if (!IS_NPC(victim) && victim != ch &&
	!IS_AFFECTED(victim, AFF_BERZERK) &&
	GET_HIT(victim) < WIMP_LEVEL(victim)) {
      send_to_char("You wimp out, and attempt to flee!\r\n", victim);
      do_flee(victim, "", 0, 0);
    }

    /* Double death possible w/ do_simple_move() and damage() -Helm */
    if (victim->in_room == NOWHERE) {
      return;
    }

    break;
  }

  if (victim->specials.protected_by &&
      victim->specials.protected_by != ch &&
      victim->in_room == (victim->specials.protected_by)->in_room &&
      ch->specials.fighting == victim &&
      number(1,60)<GET_LEVEL(victim->specials.protected_by) &&
      number(1,100)<GET_SKILL(victim->specials.protected_by, SKILL_RESCUE))
    {
      if (!pkok_check(victim->specials.protected_by, ch)) {
	do_protect(victim->specials.protected_by, GET_NAME(victim->specials.protected_by), 240, 0);
      } else {
	do_rescue(victim->specials.protected_by, GET_NAME(victim), 158, 0);
	if (!((victim->specials.protected_by)->specials.fighting))
	  set_fighting(victim->specials.protected_by, ch);
      }
    }

  if (!IS_NPC(victim) && !(victim->desc)) {
    if (!victim->specials.fighting) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      act("$N has been rescued by divine forces.", FALSE, ch, 0, victim, TO_ROOM);
      victim->specials.was_in_room = victim->in_room;
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }

  /* stop someone from fighting if they're incap or worse */
  if ((GET_POS(victim) < POS_STUNNED) &&
       (victim->specials.fighting != NULL))
    stop_fighting(victim);

  if (GET_POS(victim) == POS_DEAD) {
    if ((ch != victim) && (IS_NPC(victim) || victim->desc)) {
      if (IS_AFFECTED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
      } else {
	/* level diff bonus */
	exp = GET_EXP(victim) / 4;

	/* Change here to set maxXP -Petrus */
	exp = MAX(exp, 1);

	change_alignment(ch, victim);
        gain_exp(ch, exp);

	SET_BIT(PLR_FLAGS(ch), PLR_SAVECHR);
      }
    }
    if (!IS_NPC(victim)) {
      sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
	      (IN_ROOM(victim) != NOWHERE) ? world[IN_ROOM(victim)]->name : "NOWHERE");
      mudlog(buf2, BRF, LEVEL_DEITY, TRUE);
      /* Broadcast to all mortals on pksay channel -Bod */
      if (!IS_NPC(ch))
        info_line(ch, victim, buf2);
    }

    if (!IS_NPC(ch) && GET_LEVEL(ch) >= LEVEL_DEITY) {
      sprintf(buf2, "(GC) %s killed %s at %s", GET_NAME(ch), GET_NAME(victim),
	      (IN_ROOM(victim) != NOWHERE) ? world[IN_ROOM(victim)]->name : "NOWHERE");
      mudlog(buf2, BRF, MIN(GET_LEVEL(ch)+1, LEVEL_IMPL), TRUE);
    }

    if (IS_NPC(ch) && !IS_NPC(victim) && IS_SET(ch->specials2.act, MOB_MEMORY))
      forget(ch, victim);

    die(victim, ch);
  }
}

/*  Function get_thaco:  Calculates the thaco instead of using array.
 *  -Petrus
 */
int get_thaco(struct char_data *ch)
{
    int i, class[3], level[3], thaco[3];

    if (IS_NPC(ch))
	return (20 - GET_HITROLL(ch));

    class[0] = GET_CLASS(ch);
    class[1] = CLASS_MAGIC_USER;
    class[2] = CLASS_MAGIC_USER;
    level[0] = GET_LEVEL(ch);
    level[1] = 0;
    level[2] = 0;

    if (IS_DUAL(ch) || IS_MULTI(ch)) {
	class[0] = GET_1CLASS(ch);
	level[0] = GET_1LEVEL(ch);
	class[1] = GET_2CLASS(ch);
	level[1] = GET_2LEVEL(ch);
    }

    if (IS_3MULTI(ch)) {
	class[2] = GET_3CLASS(ch);
	level[2] = GET_3LEVEL(ch);
    }

    for (i = 0;i < 3;i++)
	switch (class[i])
	{
	case CLASS_ILLUSIONIST:
	case CLASS_WIZARD:
	case CLASS_MAGIC_USER:
	case CLASS_PSIONICIST:
	    thaco[i] = (20 - level[i] / 4);
	    break;
	case CLASS_DRUID:
	case CLASS_THIEF:
	case CLASS_CLERIC:
	case CLASS_BARD:
	case CLASS_NINJA:
	    thaco[i] = (20 - level[i] / 4);
	    break;
	case CLASS_PALADIN:
	case CLASS_MARINER:
	case CLASS_MONK:
	case CLASS_ASSASSIN:
	    thaco[i] = (20 - level[i] / 4);
	    break;
	case CLASS_CAVALIER:
	case CLASS_WARRIOR:
	case CLASS_KNIGHT:
	case CLASS_RANGER:
	    thaco[i] = (20 - level[i] / 3);
	    break;
	default:
	    thaco[i] = 20;
	}

    return MIN(thaco[0], MIN(thaco[1], thaco[2]));
}


void	hit(struct char_data *ch, struct char_data *victim, int type)
{
    struct obj_data *wielded = 0;
    int	w_type, skillbonus = 0, wield_pos = 0;
    int	victim_ac, calc_thaco;
    int	dam, percent, i;
    byte diceroll;
    struct affected_type af;
    int special = -1;

    extern struct str_app_type str_app[];

    if (ch->in_room != victim->in_room) {
	sprintf(buf, "SYSERR: !SAME ROOM %s[%d] hit %s[%d] (%d)",
		GET_NAME(ch),
		(IN_ROOM(ch) != NOWHERE) ? world[IN_ROOM(ch)]->number : -1,
		GET_NAME(victim),
		(IN_ROOM(victim) != NOWHERE) ? world[IN_ROOM(victim)]->number : -1,
		type);
	log(buf);
	if (FIGHTING(ch) && FIGHTING(ch) == victim)
	  stop_fighting(ch);
	return;
    }

    if (GET_MOVE(ch) < 1) {
	if (!number(0, 3)) {
	    send_to_char("You pant for breath...\r\n", ch);
	    act("$n pants for breath.", TRUE, ch, 0, 0, TO_ROOM | ACT_GAG);
	}
	return;
    }

    mprog_hitprcnt_trigger(ch, FIGHTING(ch));
    mprog_fight_trigger(ch, FIGHTING(ch));

    /* Skills that replace attacks: Unfair fight, Racial Skills */
    if (type != SKILL_BACKSTAB) {
      if (number(1, 1000) < GET_SKILL(ch, SKILL_UNFAIR_FIGHT)) {
        if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL) && \
            !PRF_FLAGGED(ch, PRF_GAG)) /* Damage Detail - GAG Check Necessary */
          send_to_char("[#GUnfair Fight Attack#N  (#rWAIT:#w2#N) (To Dam: 0) ", ch);
	damage(ch, victim, 0, SKILL_UNFAIR_FIGHT);
	WAIT_STATE(victim, PULSE_VIOLENCE * 2);
	return;
      }

      /* Racial attacks */
      if (!IS_NPC(ch)) {
	if (GET_RACE(ch) == RACE_FELINE) {
	  if (number(1, 1000) < GET_SKILL(ch, SKILL_CLAW)) {
            if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) { /* Damage Detail */
              sprintf(buf, "[#GClaw Attack#N  (#rBLIND#N) (To Dam: %d) ", GET_LEVEL(ch));
              send_to_char(buf, ch);
            }
	    damage(ch, victim, GET_LEVEL(ch), SKILL_CLAW);
	    if (GET_SKILL(ch, SKILL_CLAW) < get_skill_max(ch, SKILL_CLAW))
	      SET_SKILL(ch, SKILL_CLAW, GET_SKILL(ch, SKILL_CLAW) + 1);
	    if (!saves_spell(victim, SAVING_PHYSICAL, NULL, SAVE_NEGATE)) {
	      af.location = APPLY_HITROLL;
	      af.modifier = -4;
	      af.duration = 1;
	      af.bitvector = AFF_BLIND;
	      af.type = SPELL_BLINDNESS;
	      affect_join(victim, &af, TRUE, TRUE);

	      af.location = APPLY_AC;
	      af.modifier = 40;
	      affect_join(victim, &af, TRUE, TRUE);
	      send_to_char("You can't see!!!.\r\n", victim);
	      act("$N flails around blindly!", TRUE, ch, NULL, victim, TO_NOTVICT);
	    }
	    return;
	  }
	  if (number(1, 2000) < GET_SKILL(ch, SKILL_POUNCE)) {
            if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) { /* Damage Detail */
              sprintf(buf, "[#GPounce Attack#N  (#rWAIT:#w2 #ySTUN#N ) (To Dam: %d) ",
                           GET_LEVEL(ch));
              send_to_char(buf, ch);
            }
	    damage(ch, victim, GET_LEVEL(ch), SKILL_POUNCE);
	    WAIT_STATE(victim, PULSE_VIOLENCE * 2);
	    GET_POS(victim) = POS_STUNNED;
	    if (GET_SKILL(ch, SKILL_POUNCE) < get_skill_max(ch, SKILL_POUNCE))
	      SET_SKILL(ch, SKILL_POUNCE, GET_SKILL(ch, SKILL_POUNCE) +1);
	    return;
	  }
	} /* end Felines */

	if (GET_RACE(ch) == RACE_DRAGON) {
	  if (number(1, 1000) < GET_SKILL(ch, SKILL_TAIL_LASH)) {
            if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) { /* Damage Detail */
              sprintf(buf, "[#GTail Lash Attack#N  (#rWAIT:#w1 #ySTUN#N ) (To Dam: %d) ",
                            GET_LEVEL(ch));
              send_to_char(buf, ch);
            }
	    damage(ch, victim, GET_LEVEL(ch), SKILL_TAIL_LASH);
	    GET_POS(victim) = POS_STUNNED;
	    WAIT_STATE(victim, PULSE_VIOLENCE);
	    if (GET_SKILL(ch, SKILL_TAIL_LASH < get_skill_max(ch, SKILL_TAIL_LASH)))
	      SET_SKILL(ch, SKILL_TAIL_LASH, GET_SKILL(ch, SKILL_TAIL_LASH)+1);
	    return;
	  }
	} /* end Dragons */
      } /* end Racial Attacks */
    } /* end Skills that replace attacks */

    if (type == SKILL_DUAL && ch->equipment[HOLD] &&
	(ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON ||
         ch->equipment[HOLD]->obj_flags.type_flag == ITEM_FIREWEAPON))
      wielded = ch->equipment[(wield_pos = HOLD)];
    else
      if (ch->equipment[WIELD])
	wielded = ch->equipment[(wield_pos = WIELD)];

    if (wielded && (wielded->obj_flags.type_flag == ITEM_WEAPON ||
        wielded->obj_flags.type_flag == ITEM_FIREWEAPON)) {
	switch (wielded->obj_flags.value[3]) {
	case 0  :
	case 1  :
	case 2  :
	    w_type = TYPE_WHIP;
	    break;
	case 3  :
	    w_type = TYPE_SLASH;
	    if (number(1,110) < GET_SKILL(ch,SKILL_SLASH))
		skillbonus = 2;
	    break;
	case 4  :
	case 5  :
	case 6  :
	    w_type = TYPE_CRUSH;
	    break;
	case 7  :
	    w_type = TYPE_BLUDGEON;
	    if (number(1,110) < GET_SKILL(ch,SKILL_BLUDGEON))
		skillbonus = 2;
	    break;
	case 8  :
	case 9  :
	case 10 :
	    w_type = TYPE_BLAST;
	    break;
	case 11 :
	    w_type = TYPE_PIERCE;
	    if (number(1,110) < GET_SKILL(ch,SKILL_STAB))
		skillbonus = 2;
	    break;
	case 12 :
	    w_type = TYPE_NO_BS_PIERCE;
            if (number(1,110) < GET_SKILL(ch,SKILL_PIERCE))
                skillbonus = 2;
	    break;

	    default :
		w_type = TYPE_HIT;
	    break;
	}

	if (CAN_WEAR(wielded, ITEM_WIELD_2H))
	  if (number(1,110) < GET_SKILL(ch, SKILL_TWO_HANDED)) skillbonus +=2;

	GET_MOVE(ch) -= (GET_OBJ_WEIGHT(wielded)/8);
    } else {
      w_type = TYPE_HIT;
    }

    if (IS_NPC(ch)) {
      i = 0;
      if (type != SKILL_BACKSTAB) {
	percent = number(1, 100);
	while (ch->mob_specials.attacks[i].type &&
	       percent > ch->mob_specials.attacks[i].percent_of_use) {
	  percent -= ch->mob_specials.attacks[i].percent_of_use;
	  i++;
	}
      }

      if (ch->mob_specials.attacks[i].type)
	w_type = ch->mob_specials.attacks[i].type;
      else {
	w_type = TYPE_HIT;
	i = 0;
      }

      if (ch->mob_specials.attacks[i].type < SKILL_START) {
	if ((ch->mob_specials.attacks[i].damadd >= 100) ||
	    (type == TYPE_UNDEFINED)){/* Only cast if high % or 1st attack */
	  if (mob_cast_spell(ch, victim,
			     ch->mob_specials.attacks[i].type,
			     ch->mob_specials.attacks[i].damadd,
			     ch->mob_specials.attacks[i].damodice,
			     ch->mob_specials.attacks[i].damtype,
			     ch->mob_specials.attacks[i].damsizedice)) {

	    if (IN_ROOM(ch) == IN_ROOM(victim)) {
	      if (!FIGHTING(ch))
		set_fighting(ch, victim); /* In Case of Wimpy */
	    }
	    return;
	  }
	} else w_type = ch->mob_specials.attacks[0].type;
      } else if (ch->mob_specials.attacks[i].type < TYPE_START) {
	if (mob_do_action(ch->mob_specials.attacks[i].type, ch, victim)) {
	  if (!FIGHTING(ch))
	    set_fighting(ch, victim);
	  return;
	}
      } else
	w_type = ch->mob_specials.attacks[0].type;
    }

    /* Battle tactics */
    if (ch->master && AWAKE(ch->master) &&
	IS_AFFECTED(ch, AFF_GROUP) &&
	GET_POS(ch->master) > POS_STUNNED &&
	IN_ROOM(ch->master) == IN_ROOM(ch) &&
	number(1, 1500) < GET_SKILL(ch->master, SKILL_BATTLE_TACTICS))
    {
	act("You yell some strategic commands to $N.", TRUE, ch->master, 0, ch, TO_CHAR);
	act("$n yells some strategic commands to $N.", TRUE, ch->master, 0, ch, TO_NOTVICT);
	act("$n yells some strategic commands to you.", TRUE, ch->master, 0, ch, TO_VICT);
	skillbonus += 1;
    } /* end */

    /* Mounted Battle */
    if (ch->specials.mounting) {
	if (AWAKE(ch->specials.mounting)) {
	    if (number(1, 110) < GET_SKILL(ch, SKILL_MOUNTED_BATTLE)) {
		skillbonus += 1;
		improve_skill(ch, SKILL_MOUNTED_BATTLE);
	    }
	} else {
	    skillbonus -= 5;
	    if (!number(0, 5))
		send_to_char("Your mount seems asleep.\r\n", ch);
	    act("$n have problems with $s sleeping mount.", TRUE, ch, 0, ch->specials.mounting, TO_NOTVICT);
	}
    } /* end */

    /* Calculate the raw armor including magic armor */
    /* The lower AC, the better                      */

    if (!IS_NPC(ch))
	calc_thaco = get_thaco(ch);
    else
	/* THAC0 for monsters is set in the HitRoll */
	calc_thaco = 20;

    calc_thaco -= ( str_app[STRENGTH_APPLY_INDEX(ch)].tohit +
		   GET_HITROLL(ch) +
		   ((GET_INT(ch) - 13) / 3 ) +       /* Smartness helps! */
		   ((GET_WIS(ch) - 13) / 3 ) +       /* So does wisdom   */
		   skillbonus -                      /* SKILLBONUS       */
		   GET_COND(ch, DRUNK) );            /* Be sober         */

    diceroll = number(1, 20);

    victim_ac = GET_AC(victim);

    if (IS_AFFECTED(victim, AFF_BERZERK))
	victim_ac += (GET_LEVEL(ch)) / 2;
    else if (AWAKE(victim))
	victim_ac -= (GET_DEX(victim)) * 10 / 6;

    if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
      victim_ac -= 10;

    if (IS_AFFECTED(ch, AFF_BLIND)) {
	if(number(1, 200) < GET_SKILL(ch, SKILL_BLINDFIGHT)) {
	    victim_ac += 30;
	    improve_skill(ch, SKILL_BLINDFIGHT);
	}
    }

    if (GET_MOVE(victim) < 1)
	victim_ac += 50;

    victim_ac = MAX(-200, victim_ac / 10);  /* I was here -Petrus */

    if (((diceroll < 20) && AWAKE(victim)) &&       /* MISS */
	((diceroll == 1) || ((calc_thaco - diceroll) > victim_ac))) {

        /* Damage Display: Hit */
        if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL) && \
            !PRF_FLAGGED(ch, PRF_GAG)) { /* Check for GAG - Missed! */
          sprintf(buf, "[To Hit: #w%d#N - Roll: #r%d#N > Vict AC: #G%d/10#N"
                       " = To Dam: #c0#N]\r\n",
                       calc_thaco, diceroll, (victim_ac * 10));
          send_to_char(buf, ch);
        }

	if (number(1,100)<GET_COND(ch, DRUNK)) {
	    send_to_char("You feel a little dizzy...\r\n", ch);
	    act("$n looks little drunk...", TRUE, ch, 0, victim, TO_NOTVICT);
	}
	if (type == SKILL_BACKSTAB)
	    damage(ch, victim, 0, SKILL_BACKSTAB);
	else
	    damage(ch, victim, 0, w_type);
    } else {

	dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
	dam += GET_DAMROLL(ch);

	if (!wielded) {
	    if (IS_NPC(ch)) {
		dam += dice(ch->mob_specials.attacks->damodice, ch->mob_specials.attacks->damsizedice);
	    } else if (number(1, 120) < GET_SKILL(ch, SKILL_NINJITSU)) {
	      dam = GET_LEVEL(ch);
	      w_type = SKILL_PUGILISM;
	      if (number(1, 120) < GET_SKILL(ch, SKILL_ADV_MARTIAL_ARTS)) {
		special = MAX(0, MIN(number(0, GET_LEVEL(ch) - 30), 77));
		dam += ama_info[special].dam;
		ama_msg = &ama_info[special];
		w_type = TYPE_AMA;
	      }
	    } else if (number(1, 120) < GET_SKILL(ch, SKILL_PUGILISM)) {
		dam = GET_LEVEL(ch);
		w_type = SKILL_PUGILISM;
	    } else
	      dam += number(0, 2);  /* Max. 2 dam with bare hands */
	} else {
	  dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
                 /* Bless/Evil/Flaming extra damage (3d6) */
                 if (IS_OBJ_STAT(wielded, ITEM_BLESS) && IS_EVIL(victim))
                    dam += dice(3,6);
                 if (IS_OBJ_STAT(wielded, ITEM_EVIL)  && IS_GOOD(victim))
                    dam += dice(3,6);
                 if (IS_OBJ_STAT(wielded, ITEM_FLAME) && !saves_spell(victim, SAVING_PHYSICAL, NULL, SAVE_NEGATE))
                    dam += dice(3,6);


	  if (IS_NPC(ch))
	    dam += dice(ch->mob_specials.attacks->damodice, ch->mob_specials.attacks->damsizedice);
	}

        /* Damage Display: Hit */
        if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) {/* No GAG check as we have hit */
          sprintf(buf, "[To Hit: #w%d#N - Roll: #r%d#N < Vict AC: #G%d/10#N = To Dam: #c%d#N ",
                       calc_thaco, diceroll, (victim_ac * 10), dam);
          send_to_char(buf, ch);
        }


	if (GET_POS(victim) < POS_FIGHTING)
	    dam *= (3 + POS_FIGHTING - GET_POS(victim)) / 3;
	/* Position  sitting  x 1.33 */
	/* Position  resting  x 1.66 */
	/* Position  sleeping x 2.00 */
	/* Position  stunned  x 2.33 */
	/* Position  incap    x 2.66 */
	/* Position  mortally x 3.00 */

	dam = MAX(1, dam);  /* Not less than 0 damage */
	if (IS_AFFECTED(ch, AFF_BERZERK))
	    dam += number(1, dam);

	/* Extra damage */
	if (number(1, 200) < GET_SKILL(ch, SKILL_EXTRA_DAMAGE))
	    dam += GET_LEVEL(ch) / 2;

	/* Poison Blade */
	if (( (w_type == TYPE_SLASH) ||
	     (w_type == TYPE_PIERCE) ||
	     (w_type == TYPE_NO_BS_PIERCE) ) &&
	    (number(1,5000) < GET_SKILL(ch, SKILL_POISON_BLADE) )) {

	  if (!saves_spell(victim, SAVING_POISON, NULL, SAVE_NEGATE)) {
	    af.type = SPELL_POISON;
	    af.duration = GET_LEVEL(ch);
	    af.modifier = -2;
	    af.location = APPLY_STR;
	    af.bitvector = AFF_POISON;
	    affect_join(victim, &af, TRUE, TRUE);
	    send_to_char("You feel very sick.\r\n", victim);
	    act("$N gets violently ill!", TRUE, ch, NULL, victim, TO_NOTVICT);
	  }
	  if (IS_AFFECTED(victim, AFF_POISON))
	      send_to_char("#gYou poison your victim!#N\r\n", ch);
	}

	if (type == SKILL_BACKSTAB) {
	    dam *= MIN(GET_LEVEL(ch) / 10 + 1, 5);
	    damage(ch, victim, dam, SKILL_BACKSTAB);
	} else {
	    if ((victim->equipment[WEAR_SHIELD]) &&
		(number(1,300)+dam)<GET_SKILL(victim,SKILL_PARRY))
	    {
		damage(ch, victim, dam - GET_LEVEL(victim), SKILL_PARRY);
		improve_skill(victim, SKILL_PARRY);
		if (wielded && damage_object(wielded, dam/10)) {
		    send_to_char("Your weapon breaks from the impact.\r\n", ch);
		    obj_to_char(unequip_char(ch, wield_pos), ch);
		}
		if (damage_object(victim->equipment[WEAR_SHIELD], dam/10)) {
		    send_to_char("Your shield shatters from the impact.\r\n", victim);
		    obj_to_char(unequip_char(victim, WEAR_SHIELD), victim);
		}
		return;
	    } else if ((number(1,250)+dam)<GET_SKILL(victim,SKILL_DODGE)) {
		damage(ch, victim, dam - 2*GET_LEVEL(victim), SKILL_DODGE);
		improve_skill(victim, SKILL_DODGE);
		return;
	    } else if ((number(1,250)+dam)<GET_SKILL(victim,SKILL_TUMBLE)) {
		damage(ch, victim, dam-3*GET_LEVEL(victim), SKILL_TUMBLE);
		improve_skill(victim, SKILL_TUMBLE);
	    } else {
		if (number(1, 5000) < GET_SKILL(ch, SKILL_CRITICAL_HIT)) {
		    dam += GET_LEVEL(ch)*4;
		    w_type = SKILL_CRITICAL_HIT;
		} else if ((w_type != TYPE_AMA) &&
			   (number(1, 250) < GET_SKILL(ch, SKILL_MARTIAL_ARTS))) {
		    dam += 2;
		    w_type = SKILL_MARTIAL_ARTS;
		}
		/* Put in disbelieve here */
		damage(ch, victim, dam, w_type);

		if (type == SKILL_2ATTACK ||
		    type == SKILL_3ATTACK ||
		    type == SKILL_4ATTACK ||
		    type == SKILL_DUAL)
		    improve_skill(ch, type);
	    }
	}
    }
}

void autoassist(struct char_data *ch)
{
     struct follow_type *f, *f_next;
     struct char_data *k;

     ACMD(do_assist);

     if (!ch || !FIGHTING(ch)) return;

     if (IS_NPC(ch)) { /* MOB_HELPER */

	  for (k = world[ch->in_room]->people ; k ; k = k->next_in_room) {
	       if (!FIGHTING(ch)) return; /* if target disappears */

	       if (!IS_NPC(k))      /* check only mobs in the room */
		    continue;

	       if ((k != ch) &&                /* ch is already fighting */
		   MOB_FLAGGED(k, MOB_HELPER) &&     /* Helper Flag */
		   !FIGHTING(k) &&                /* not already fighting */
		   (GET_MOB_WAIT(k) < 1))         /* no wait state */
	       {
		    if (k->master) {
			 if (FIGHTING(k->master) &&
			     (IN_ROOM(k) == IN_ROOM(FIGHTING(k->master))))
			      hit(k, FIGHTING(k->master), TYPE_UNDEFINED);
		    } else {
			 if (abs(GET_ALIGNMENT(ch) - GET_ALIGNMENT(k))<=750 &&
			      (IN_ROOM(k) == IN_ROOM(FIGHTING(ch))))
			      hit (k, FIGHTING(ch), TYPE_UNDEFINED) ;
		    }
	       }
	  }

	                      /* END MOB_HELPER */
     } else {
                              /* Player Autoassist */

	  if (ch->master != NULL)
	       k = ch->master;
	  else
	       k = ch;

	  if (IS_NPC(k)) return;

	  if (k != ch) {
	       if ((k->desc) &&                       /* connected */
		   PLR_FLAGGED(k, PLR_AUTOASSIST) &&  /* autoassist */
		   IS_AFFECTED(k, AFF_GROUP) &&       /* grouped */
		   !FIGHTING(k) &&                 /* not already fighting */
		   (IN_ROOM(FIGHTING(ch)) == IN_ROOM(k)) &&   /* same room */
		   AWAKE(k) &&                        /* proper position */
		   CAN_SEE(k, FIGHTING(ch)) &&        /* can see target */
		   (CHECK_WAIT(k) < 1) &&             /* check wait state */
		   (WIMP_LEVEL(k) < GET_HIT(k)) &&         /* wimpie */
		   !PLR_FLAGGED(k, PLR_WRITING | PLR_MAILING))
		    /* not mailing or writing */
	       {
		    do_assist(k, "", 0, 0);
		    WAIT_STATE(k, PULSE_VIOLENCE);
	       }
	  }

	  for (f = k->followers; f; f = f_next) {
	       f_next = f->next;
	       if (!FIGHTING(ch)) return;

	       if (IS_NPC(f->follower) && !FIGHTING(f->follower) &&
                   (IN_ROOM(FIGHTING(ch)) == IN_ROOM(f->follower))) {
		    hit(f->follower, FIGHTING(ch), TYPE_UNDEFINED);
		    continue;
	       }

	       if ((!(ch == (f->follower))) &&   /* ch is already fighting */
		   (f->follower->desc) &&             /* connected? */
		   PLR_FLAGGED(f->follower, PLR_AUTOASSIST) && /*autoassist*/
		   IS_AFFECTED(f->follower, AFF_GROUP) && /* grouped */
		   !FIGHTING(f->follower) &&          /* not fighting */
		   (IN_ROOM(FIGHTING(ch)) == IN_ROOM(f->follower)) &&
                                                        /* same room */
		   AWAKE(f->follower) &&               /* correct position */
		   CAN_SEE(f->follower, FIGHTING(ch)) && /* can see target */
		   (CHECK_WAIT(f->follower) < 1) &&    /* wait state */
		   WIMP_LEVEL(f->follower) < GET_HIT(f->follower) && /*wimp*/
		   !PLR_FLAGGED(f->follower, PLR_WRITING | PLR_MAILING))
		    /* not mailing or writing */
	       {
		    do_assist(f->follower, "", 0, 0);
		    WAIT_STATE(f->follower, PULSE_VIOLENCE);
	       }
	  }
     }                /* END Player Autoassist */

     return;
}


#define NUMBER_OF_ATTACKS  5
void	perform_violence(void)
{
     int i, attacktype = TYPE_UNDEFINED, percent = 100;
     struct char_data *ch;

     for (i = 1; i <= NUMBER_OF_ATTACKS; i++) {

	  switch(i) {
	       case 1: attacktype = TYPE_UNDEFINED; percent = 0; break;
	       case 2: attacktype = SKILL_2ATTACK; percent = 120; break;
	       case 3: attacktype = SKILL_3ATTACK; percent = 140; break;
	       case 4: attacktype = SKILL_4ATTACK; percent = 160; break;
	       case 5: attacktype = SKILL_DUAL; percent = 160; break;
	       default: attacktype = TYPE_UNDEFINED; percent = 0; break;
	  }

	  for (ch = combat_list; ch; ch = combat_next_dude) {
	       combat_next_dude = ch->next_fighting;

	       if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
		    stop_fighting(ch);
		    continue;
	       }

	       if (IS_NPC(ch)) {
		    if (GET_MOB_WAIT(ch) > 0) {
			 GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
			 continue;
		    }
		    GET_MOB_WAIT(ch) = 0;
		    if (GET_POS(ch) != POS_FIGHTING) {
			 if (GET_POS(ch) < POS_STANDING)
			      act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
                         GET_POS(ch) = POS_FIGHTING;
			 continue;
		    }
	       } else {
		 if (GET_POS(ch) == POS_SITTING) {
		   act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
		   act("You regain your senses and scramble to your feet.", TRUE, ch, 0, 0, TO_CHAR);
		   GET_POS(ch) = POS_FIGHTING;
		   continue;
		 }
	       }

	       if (!AWAKE(ch)) continue;

	       if (attacktype == SKILL_DUAL) {
		 if (!ch->equipment[HOLD])
		   continue;
		 else if (!(ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON ||
                            ch->equipment[HOLD]->obj_flags.type_flag == ITEM_FIREWEAPON))
		   continue;
	       }

	       if (!percent || number(1,percent) < GET_SKILL(ch,attacktype)) {
		    hit(ch, ch->specials.fighting, attacktype);
		    if (IS_AFFECTED(ch, AFF_HIDE))
			 REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
	       }

	       autoassist(ch); /* autoassist flag and MOB_HELPER */

	  }
     }
}



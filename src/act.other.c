/*************************************************************************
 *   File: act.other.c                                   Part of EliteMUD  *
 *  Usage: Miscellaneous player-level commands                             *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
 *  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
 ************************************************************************ */

#define __ACT_OTHER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "limits.h"
#include "functions.h"
#include "ignore.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data **world;
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern struct player_index_element *player_table;
extern char	*class_abbrevs[];
extern char     *class_dual[4][4];
extern char     *class_multi[];
extern char     *dirs[];
extern char     *skills[];
extern char     *spells[];
extern char     *COLOR_TEST;
extern int      ident;
extern int	nameserver_is_slow;
extern int      save_last_command;
extern int      is_quest;

/* extern procedures */
SPECIAL(shop_keeper);


/* function to check for and improve a skill  _Petrus*/
void  improve_skill(struct char_data *ch, int skillnr)
{
  int percent;
  char tmp[MAX_INPUT_LENGTH];

  if (!ch || IS_NPC(ch))
    return;

  percent = GET_SKILL(ch, skillnr);

  if (percent > 1 &&
      know_skill(ch, skillnr) &&
      percent < get_skill_max(ch, skillnr) &&
      number(0, 99) > percent && number(0, 100) < ch->specials.skillgain)
    {
      SET_SKILL(ch, skillnr, percent + 1);
      sprintf(tmp, "Your %s - %s - just improved.\r\n",
	      (skillnr<SKILL_START?"spell":"skill"),
	      (skillnr<SKILL_START?spells[skillnr-1]:skills[skillnr-SKILL_START]));
      send_to_char(tmp, ch);
      ch->specials.skillgain = 0;
    }
}



ACMD(do_quit)
{
  void count_objs(struct obj_data *obj, int *num);
  extern int max_obj_save;
  extern sh_int r_mortal_start_room;
  int i, j;

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (subcmd != SCMD_QUIT) {
    send_to_char("You have to type quit - no less, to quit!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), NO_QUIT) &&
      (GET_LEVEL(ch) < LEVEL_DEITY)) {
    send_to_char("You can't quit in this room.\r\n", ch);
    return;
  }

  if (GET_POS(ch) == POS_FIGHTING) {
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
    return;
  }

  /* Dont allow quit when over 100 items */
  for (j = 0,i = 0; j < MAX_WEAR; j++)
 	if (ch->equipment[j])
      count_objs(ch->equipment[j], &i);

  count_objs(ch->carrying, &i);
  if (i > max_obj_save)
  {
    send_to_char("Quit with that many items? You will never be able to keep track of them all...\r\n", ch);
    return;
  }

  if (GET_POS(ch) < POS_STUNNED) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch, NULL);
    return;
  }

  REMOVE_BIT(PLR_FLAGS(ch), PLR_ARENA);

  if (ROOM_FLAGGED(ch->in_room, ARENA)) {
    char_from_room(ch);
    char_to_room(ch, r_mortal_start_room);
  }


  act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
  if (!GET_INVIS_LEV(ch))
    act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "%s has quit the game.", GET_NAME(ch));
  mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_LEVEL(ch)), TRUE);


  if (ch->specials.aliases)
    alias_save(ch);

  if (ch->specials.ignore_list)
    write_ignorefile(ch);

  stringdata_save(ch);

  Crash_quitsave(ch);

  extract_char(ch);		/* Char is saved in extract char */
}



ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  if (cmd != 0) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }
  if (PLR_FLAGGED(ch, PLR_SAVECHR))
    save_char(ch, IN_VROOM(ch), 2);
  if (PLR_FLAGGED(ch, PLR_SAVEOBJS))
    Crash_crashsave(ch, MANUAL_SAVE);
  if (PLR_FLAGGED(ch, PLR_SAVEALS))
    alias_save(ch);
  if (PLR_FLAGGED(ch, PLR_SAVESTR))
    stringdata_save(ch);
  if (PLR_FLAGGED(ch, PLR_SAVEIGN))
        write_ignorefile(ch);
}


ACMD(do_not_here)
{
  send_to_char("Sorry, but you can't do that here!\r\n", ch);
}

ACMD(do_flip)
{
  struct obj_data *obj;
  int rannum;
  if (--GET_GOLD(ch) < 0) {
    send_to_char("You don't have a coin to flip :(\r\n", ch);
    GET_GOLD(ch) = 0;
    return;
  }



  obj = create_money(1);
  obj_to_room(obj, ch->in_room);

  rannum = number(1,101);
  if( GET_COND(ch,DRUNK) > 20)
    rannum = 101;
  if (rannum <= 50) {
    send_to_char("You flip a coin and win!\r\n", ch);
    act("$n flips a coin and wins.", TRUE, ch, 0, 0, TO_NOTVICT);
  } else if (rannum <= 100){
    send_to_char("You flip a coin and lose.\r\n", ch);
    act("$n flips a coin and loses.", TRUE, ch, 0, 0, TO_NOTVICT);
  }
  else
    {
      rannum = number(1,100);
      rannum = MIN(rannum,GET_GOLD(ch));
      GET_GOLD(ch) -= rannum;
      if(rannum > 1)
	{

	  send_to_char("Oops! Instead of one coin you flip many coins.\r\n",ch);
	  sprintf(buf2,"$n tries to flip a coin, but accidently throws %s coins up in the air instead.",rannum < 10 ? "some" : rannum < 50 ?
		  "a couple" : "a lot");
	  act(buf2,TRUE,ch,0,0,TO_ROOM);
	  obj_to_room(create_money(rannum - 1),ch->in_room);
	}
      else
	{
	  send_to_char("Up up up in the air your coin goes. Perhaps it will return to earth someday.\r\n",ch);
	  act("$n flips a coin... way up in the air...perhaps it will return someday.",TRUE,ch,0,0,TO_ROOM);
	  obj_from_room(obj);
	}
    }
  return;
}


ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    send_to_char("You stop trying to sneak.\r\n", ch);
    affect_from_char(ch, SKILL_SNEAK);
    return;
  }

  if(GET_SKILL(ch, SKILL_SNEAK) < 1) {
	send_to_char("You aren't very good at sneaking.\r\n", ch);
	return;
  }

  send_to_char("Ok, you'll try to move silently for a while.\r\n", ch);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + GET_DEX(ch))
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}


ACMD(do_aid)
{
  struct char_data *victim;
  char victim_name[MAX_INPUT_LENGTH];

  one_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Can't aid someone not here!\r\n", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Lie down and shut up!\r\n", ch);
    return;
  }

  if (GET_POS(victim) >= POS_STUNNED) {
    send_to_char("Nay!  No need! (yet)\r\n", ch);
    return;
  }

  if (number(1, 110) < GET_SKILL(ch, SKILL_FIRST_AID)) {
    GET_HIT(victim) += number (1, 4);
    update_pos(victim);
    send_to_char("You succeed in stopping some of the bleeding.\r\n", ch);
    act("$n bandages your wounds.", TRUE, ch, 0, victim, TO_VICT);
    improve_skill(ch, SKILL_FIRST_AID);
  } else {
    send_to_char("You can't stop the bleeding!  Panic!\r\n", ch);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);
}




ACMD(do_hide)
{
  byte percent;

  send_to_char("You attempt to hide yourself.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_HIDE) + GET_DEX(ch))
    return;

  improve_skill(ch, SKILL_HIDE);
  SET_BIT(ch->specials.affected_by, AFF_HIDE);
}




ACMD(do_steal)
{
  struct char_data *victim;
  struct obj_data *obj;
  char	victim_name[240];
  char	obj_name[240];
  int	percent, gold, eq_pos, pcsteal = 0;
  extern int	pt_allowed;
  bool ohoh = FALSE;

  ACMD(do_gen_com);

  argument = one_argument(argument, obj_name);
  one_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Steal what from who?\r\n", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Come on now, that's rather stupid!\r\n", ch);
    return;
  }

  if(IS_SET(world[ch->in_room]->room_flags, LAWFULL))
    {
      act("No you cannot bring yourself to steal here, in this place.",
	  FALSE,ch,0,victim,TO_CHAR);
      act("$n seems to want something from you but changes $e mind.",TRUE,ch,0,victim,
	  TO_VICT);
      act("$n seems to want something from $N but changes $e mind.",TRUE,ch,0,victim, TO_NOTVICT);
      return;
    }

  if (!pt_allowed) {
    if (!IS_NPC(victim) && !PLR_FLAGGED(victim, PLR_THIEF) &&
	!PLR_FLAGGED(victim, PLR_KILLER) && !PLR_FLAGGED(ch, PLR_THIEF)) {
      /* SET_BIT(ch->specials.act, PLR_THIEF);
	 send_to_char("OK, you're the boss... you're now a THIEF!\r\n",ch);
	 sprintf(buf, "PC Thief bit set on %s", GET_NAME(ch));
	 log(buf);
	 */
      pcsteal = 1;
    }

    if (PLR_FLAGGED(ch, PLR_THIEF))
      pcsteal = 1;

    /* We'll try something different... instead of having a thief flag,
       just have PC Steals fail all the time.
       */
    if (pcsteal) {
      send_to_char("You failed.\r\n", ch);
      return;
    }

  }

  /* 101% is a complete failure */
  percent = number(1, 101);

  if (GET_POS(victim) < POS_SLEEPING)
    percent = -1;		/* ALWAYS SUCCESS */

  /* NO NO With Imp's and Shopkeepers! */
  if ((GET_LEVEL(victim) >= LEVEL_DEITY) || pcsteal ||
      (IS_MOB(victim) && mob_index[victim->nr].func == shop_keeper))
    percent = 101;		/* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {

      for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	if (victim->equipment[eq_pos] &&
	    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
	    CAN_SEE_OBJ(ch, victim->equipment[eq_pos])) {
	  obj = victim->equipment[eq_pos];
	  break;
	}

      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, victim, TO_CHAR);
	return;
      } else {			/* It is equipment */
	if ((GET_POS(victim) > POS_STUNNED)) {
	  send_to_char("Steal the equipment now?  Impossible!\r\n", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.", FALSE, ch, obj , 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, victim, TO_NOTVICT);
	  obj_to_char(unequip_char(victim, eq_pos), ch);
	  improve_skill(ch, SKILL_STEAL);
	}
      }
    } else {			/* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

      if (AWAKE(victim) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, victim, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else {			/* Steal the item */
	if (GET_ITEM_LEVEL(obj) > GET_LEVEL(ch)) {
	  send_to_char("You can't touch that item.\r\n", ch);
	} else if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);

	    set_key_timer(obj);  /* Activate Key Timer */

	    send_to_char("Got it!\r\n", ch);
	  }
	} else
	  send_to_char("You cannot carry that much.\r\n", ch);
      }
    }
  } else {			/* Steal some coins */
    if (AWAKE(victim) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(victim) -= gold;
	sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
	send_to_char(buf, ch);
      } else {
	send_to_char("You couldn't get any gold...\r\n", ch);
      }
    }
  }

  if (ohoh && IS_NPC(victim) && AWAKE(victim)) {
    if (IS_SET(MOB_FLAGS(victim), MOB_NICE_THIEF)) {
      sprintf(buf, "%s is a bloody thief!", GET_NAME(ch));
      do_gen_com(victim, buf, 0, SCMD_YELL);
      log(buf);
      send_to_char("Don't you ever do that again!\r\n", ch);
    }
    else
      hit(victim, ch, TYPE_UNDEFINED);
  }
}



ACMD(do_practice)
{
  void practices(struct char_data *ch, int mode);

  one_argument(argument, arg);

  if (!subcmd)
    send_to_char("You can only practice with your guildmaster.\r\n", ch);
  else if (subcmd == SCMD_SKILLS) {
    if (*arg == 'i')
      practices(ch, 3);
    else
      practices(ch, 1);
  } else if (subcmd == SCMD_SPELLS) {
    if (*arg == 'i')
      practices(ch, 4);
    else
      practices(ch, 2);
  }
}


ACMD(do_visible)
{
  void	appear(struct char_data *ch);

  if IS_AFFECTED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  } else
    send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if (!*argument) {
    send_to_char("If that is what you want.\r\n", ch);
    set_title(ch);
  }
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > 60)
    send_to_char("Sorry, titles can't be longer than 60 characters.\r\n", ch);
  else {
    strcat(argument, "#N");  /* Add end of col */
/*
    if(!(argument[0] == '\''))   {
        strcpy(temparg, argument);
        argument[0] = ' ';
        for(i = 1;i < strlen(argument) + 1;i++) {
                argument[i] = temparg[i-1];
        }
    }
 */
    if (GET_TITLE(ch))
      RECREATE(GET_TITLE(ch), char, strlen(argument) + 1);
    else
      CREATE(GET_TITLE(ch), char, strlen(argument) + 1);
    strcpy(GET_TITLE(ch), argument);

    sprintf(buf, "OK, you're now %s %s.\r\n", GET_NAME(ch),
GET_TITLE(ch));
    send_to_char(buf, ch);

    SET_BIT(PLR_FLAGS(ch), PLR_SAVESTR);
  }
}


ACMD(do_group)
{
  struct char_data *victim, *k;
  struct follow_type *f;
  bool found;
  int maxneed;
  char wiz[30];

  one_argument(argument, buf);

  if (!*buf) {
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are not the member of a group!\r\n", ch);
    } else {
      k = (ch->master ? ch->master : ch);
      if (k->in_room == ch->in_room && IS_AFFECTED(k, AFF_GROUP))
	maxneed = group_xp_value(k);
      else
	maxneed = group_xp_value(ch);

      for (f = k->followers; f; f = f->next)
	if (group_xp_value(f->follower) > maxneed &&
	    f->follower->in_room == ch->in_room &&
	    IS_AFFECTED(f->follower, AFF_GROUP))
	  maxneed = group_xp_value(f->follower);
      send_to_char("Your group consists of:\r\n", ch);

      if (IS_AFFECTED(k, AFF_GROUP)) {
	sprintf(buf2, "    (%6.2f%%)",
		group_xp_value(k)/(float)maxneed*100.0);
        CLASS_ABBR(k, wiz);
	sprintf(buf, "%s [%3d %5s] <§g%5dHp§m%5dMn§c%5dMv§N > §C$N the leader§N",
		(k->in_room == ch->in_room) ? buf2:
		"(not present)",
		GET_LEVEL(k), wiz,
		GET_HIT(k), GET_MANA(k), GET_MOVE(k));
	act(buf, FALSE, ch, 0, k, TO_CHAR|TO_SLEEP);
      }
      for (f = k->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	  sprintf(buf2, "    (%6.2f%%)",
		  group_xp_value(f->follower)/(float)maxneed*100.0);
          CLASS_ABBR(f->follower, wiz);
	  sprintf(buf, "%s [%3d %s] <§g%5dHp§m%5dMn§c%5dMv§N > §C$N§N",
		  (f->follower->in_room == ch->in_room) ? buf2:
		  "(not present)",
		  GET_LEVEL(f->follower), wiz,
		  GET_HIT(f->follower),
		  GET_MANA(f->follower),
		  GET_MOVE(f->follower));
	  act(buf, FALSE, ch, 0, f->follower, TO_CHAR|TO_SLEEP);
	}
    }

    return;
  }

  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    SET_BIT(ch->specials.affected_by, AFF_GROUP);
    for (f = ch->followers; f; f = f->next) {
      victim = f->follower;
      if (!IS_AFFECTED(victim, AFF_GROUP)) {
	act("$N is now a member of your group.", FALSE, ch, 0, victim, TO_CHAR);
	act("You are now a member of $n's group.", FALSE, ch, 0, victim, TO_VICT);
	act("$N is now a member of $n's group.", FALSE, ch, 0, victim, TO_NOTVICT);
	SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    }
    return;
  }

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("No one here by that name.\r\n", ch);
  } else {
    found = FALSE;

    if (victim == ch)
      found = TRUE;
    else {
      for (f = ch->followers; f; f = f->next) {
	if (f->follower == victim) {
	  found = TRUE;
	  break;
	}
      }
    }

    if (found) {
      if (IS_AFFECTED(victim, AFF_GROUP)) {
	act("$N is no longer a member of your group.", FALSE, ch, 0, victim, TO_CHAR);
	act("You have been kicked out of $n's group!", FALSE, ch, 0, victim, TO_VICT);
	act("$N has been kicked out of $n's group!", FALSE, ch, 0, victim, TO_NOTVICT);
	REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
      } else {
	act("$N is now a member of your group.", FALSE, ch, 0, victim, TO_CHAR);
	act("You are now a member of $n's group.", FALSE, ch, 0, victim, TO_VICT);
	act("$N is now a member of $n's group.", FALSE, ch, 0, victim, TO_NOTVICT);
	SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    } else
      act("$N must follow you to enter your group.", FALSE, ch, 0, victim, TO_CHAR);
  }
}


ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  struct char_data *tch;
  void	stop_follower(struct char_data *ch);

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }

    if (IS_AFFECTED(ch, AFF_GROUP))
      REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);

    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	REMOVE_BIT(f->follower->specials.affected_by, AFF_GROUP);
	send_to_char(buf2, f->follower);
	stop_follower(f->follower);
      }
    }

    send_to_char("You have disbanded the group.\r\n", ch);
    return;
  }

  if (!(tch = get_char_room_vis(ch, buf))) {
    send_to_char("There is no such person!\r\n", ch);
    return;
  }

  if (tch->master != ch && tch != ch)
  {
    send_to_char("That person is not following you!\r\n", ch);
    return;
  }

  /* If target is yourself then disban the group */
  if (tch == ch)
  {
    if (IS_AFFECTED(tch, AFF_GROUP))
      REMOVE_BIT(tch->specials.affected_by, AFF_GROUP);

    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol)
    {
      next_fol = f->next;
      if (IS_AFFECTED(f->follower, AFF_GROUP))
      {
       REMOVE_BIT(f->follower->specials.affected_by, AFF_GROUP);
       send_to_char(buf2, f->follower);
       stop_follower(f->follower);
      }
    }

    send_to_char("You have disbanded the group.\r\n", ch);
    return;
  }
  else
  {
    if (IS_AFFECTED(tch, AFF_GROUP))
      REMOVE_BIT(tch->specials.affected_by, AFF_GROUP);

    act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
    act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
    act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
    stop_follower(tch);
  }
}

ACMD(do_split)
{
  int	amount, num, share;
  struct char_data *k;
  struct follow_type *f;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }

    if (IS_NPC(ch)) return; /* Fixes a cheat */

    if (amount > GET_GOLD(ch)) {
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
      return;
    }

    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room))
	num++;

    if (num && IS_AFFECTED(ch, AFF_GROUP))
      share = amount / num;
    else {
      send_to_char("There's no one here to share your gold :(\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k)) &&  k != ch) {
      GET_GOLD(k) += share + amount % num;
      sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	      amount, share + amount % num);
      send_to_char(buf, k);
    }

    for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch) {
	GET_GOLD(f->follower) += share;
	sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	        amount, share);
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members and keep %d coins.\r\n", amount, num, share);
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}


ACMD(do_use)
{
  struct obj_data *mag_item;
  int equipped = 1;

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = ch->equipment[HOLD];

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      equipped = 0;
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_use");
      return;
      break;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_ITEM_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.\r\n", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_ITEM_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char("You can only recite scrolls.\r\n", ch);
      return;
    }
    break;
  case SCMD_USE:
    if ((GET_ITEM_TYPE(mag_item) != ITEM_WAND) &&
	(GET_ITEM_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char("You can't seem to figure out how to use it.\r\n", ch);
      return;
    }
    break;
  }

  if (CLANEQ(mag_item))
       if ((CLANEQ_CLAN(mag_item) != CLAN(ch) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) ||
           (CLAN_LEVEL(ch) <= 1)) {
	    send_to_char("You aren't in the correct clan to use that.\r\n", ch);
    return;
       }

  mag_objectmagic(ch, mag_item, buf);
}


ACMD(do_wimpy)
{
  int	wimp_lev;

  one_argument(argument, arg);

  if (!*arg) {
    if (WIMP_LEVEL(ch)) {
      sprintf(buf, "Your current wimp level is %d hit points.\r\n",
	      WIMP_LEVEL(ch));
      send_to_char(buf, ch);
      return;
    } else {
      send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
      return;
    }
  }

  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg))) {
      if (wimp_lev < 0) {
	send_to_char("He he he! OK you will be a dead wimp.\r\n", ch);
	return;
      }
      if (wimp_lev > GET_MAX_HIT(ch)/4) {
	send_to_char("I know that you are a constant wimp, but set it a little lower will you?\r\n", ch);
	return;
      }
      sprintf(buf, "OK, you'll chicken out if you drop below %d hp.\r\n",
	      wimp_lev);
      send_to_char(buf, ch);
      WIMP_LEVEL(ch) = wimp_lev;
    } else {
      send_to_char("OK, you are now as tough (and stupid) as a knight.\r\n", ch);
      WIMP_LEVEL(ch) = 0;
    }
  } else
    send_to_char("Specify at how many hp you want to chicken out at.  (0 to disable)\r\n", ch);

  return;

}


void	chlist_to_char(struct char_data *list, struct char_data *ch,
		       int range)
{
  struct char_data *i;
  char buf[80], buf2[80];
  int num;

  for (i = list; i; i = i->next_in_room) {
    if (ch != i) {
      num = 1;
      while (CAN_SEE(ch, i) && i->next_in_room && i->nr == i->next_in_room->nr)
	{
	  num++;
	  i = i->next_in_room;
	}
      if (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE) &&
	  number(0, GET_LEVEL(i)) < GET_SKILL(ch, SKILL_SCOUT))
	{
	  if (num > 1)
	    sprintf(buf2, "[%2d] ", num);
	  else
	    *buf2 = 0;

	  strcpy(buf, GET_NAME(i));

	  strcat(buf2, CAP(buf));

	  sprintf(buf, "   %2d - %s\r\n", range, buf2);

	  send_to_char(buf, ch);
	}
    }
  }
}


ACMD(do_scout)
{
  int dir, range, roomnr;

  if (!ch->desc)
    return;

  if (GET_SKILL(ch, SKILL_SCOUT) < 1) {
    send_to_char("Your ability to scout is severly lacking...\r\n", ch);
    return;
  }

  if (IS_SET(world[ch->in_room]->room_flags, NO_SCOUT)) {
    send_to_char("You can't seem to scout in this area.\r\n", ch);
    return;
  }

  send_to_char("You scout the surrounding area...\r\n", ch);
  act("$n scouts the surrounding area.", TRUE, ch, 0, 0, TO_ROOM);
  for (dir = 0; dir < 4; dir++) {
    range = 1;
    roomnr = ch->in_room;

    if ((world[roomnr]->dir_option[dir]) &&
	(world[roomnr]->dir_option[dir]->to_room != NOWHERE) &&
	!IS_SET(world[roomnr]->dir_option[dir]->exit_info, EX_CLOSED))
      {
	sprintf(buf2, " To the %s:\r\n", dirs[dir]);
	send_to_char(buf2, ch);
	GET_MOVE(ch) -= 1;
      }

    while(GET_SKILL(ch, SKILL_SCOUT) > range*15) {
      if ((world[roomnr]->dir_option[dir]) &&
	  (world[roomnr]->dir_option[dir]->to_room != NOWHERE) &&
	  !IS_SET(world[roomnr]->dir_option[dir]->exit_info, EX_CLOSED))
	{
	  roomnr = world[roomnr]->dir_option[dir]->to_room;
	  if (!IS_SET(world[roomnr]->room_flags, NO_SCOUT)) {
	    chlist_to_char(world[roomnr]->people, ch, range);
	    range++;
	  } else {
	    sprintf(buf, "    %d - You can't seem to scout through here...\r\n", range);
	    send_to_char(buf, ch);
	    break;
	  }
	} else
	  break;
    }
  }
  improve_skill(ch, SKILL_SCOUT);
}


ACMD(do_mount)
{
  int i;
  struct char_data *mount;

  for (i = 0; argument[i] == ' '; i++);

  if (subcmd == SCMD_DISMOUNT) {
    if (!ch->specials.mounting)
      send_to_char("Try to find a horse first.\r\n", ch);
    else {
      send_to_char("You dismount.\r\n", ch);
      act("$n dismounts from $N.", TRUE, ch, 0, ch->specials.mounting, TO_ROOM);
      (ch->specials.mounting)->specials.mounted_by = 0;
      ch->specials.mounting = 0;
    }

    return;
  }

  if (!*(argument + i)) {
    send_to_char("Mount what?\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  if (!(mount = get_char_room_vis(ch, buf))) {
    send_to_char("Can't see that here.\r\n", ch);
    return;
  }

  if (!IS_NPC(mount) || mount == ch) {
    send_to_char("Not a good idea...\r\n", ch);
    return;
  }

  if (ch->specials.mounting) {
    send_to_char("Try to dismount first.\r\n", ch);
    return;
  }

  if (mount->specials.mounted_by || ch->specials.mounted_by) {
    send_to_char("Already carrying someone.\r\n", ch);
    return;
  }

  if (!IS_NPC(mount)) {
    send_to_char("Hmmm... Not such a good idea.\r\n", ch);
    return;
  }

  if (GET_RACE(mount) != MOB_MOUNT && GET_RACE(mount) != MOB_MOUNT_FLY) {
    sprintf(buf, "%s doesn't seem too willingly...\r\n", GET_NAME(mount));
    CAP(buf);
    send_to_char(buf, ch);
    return;
  }

  sprintf(buf, "You mount %s.\r\n", GET_NAME(mount));
  send_to_char(buf, ch);
  act("$n mounts $N.", TRUE, ch, 0, mount, TO_ROOM);

  ch->specials.mounting = mount;
  mount->specials.mounted_by = ch;
}


ACMD(do_protect)
{
  int i;
  struct char_data *victim;

  for (i = 0; argument[i] == ' '; i++);

  if (!*(argument + i)) {
    send_to_char("Protect whom?\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("Nobody around here with that name.\r\n", ch);
    return;
  }

  die_protector(ch);

  if (ch == victim)
    return;

  if (IS_NPC(victim)) {
    send_to_char("You cannot protect a mob.\r\n", ch);
    return;
  }

  if (victim->specials.protected_by)
    (victim->specials.protected_by)->specials.protecting = 0;

  ch->specials.protecting = victim;
  victim->specials.protected_by = ch;

  act("You now protect $N.", FALSE, ch, 0, victim, TO_CHAR);
  act("$n is now protecting $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  act("$n is now protecting you.", TRUE, ch, 0, victim, TO_VICT);
}


ACMD(do_display)
{
  int	i;
  bool changed = FALSE;
  long old;

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: display <H|M|V|T|norm|all|none|vt|ansi>\r\n", ch);
    return;
  }

  old = PRF_FLAGS(ch);

  if(!str_cmp(argument, "norm")) {
    if (PRF_FLAGGED(ch, PRF_DISPVT))
      changed = TRUE;
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPVIC | PRF_DISPVT | PRF_DISPANSI);
    SET_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE);
  } else if (!str_cmp(argument, "vt")) {
    if (!PRF_FLAGGED(ch, PRF_DISPVT))
      changed = TRUE;
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISPVIC | PRF_DISPANSI);
    SET_BIT(PRF_FLAGS(ch), PRF_DISPVT);
  } else if (!str_cmp(argument, "all")) {
    if (PRF_FLAGGED(ch, PRF_DISPVT))
      changed = TRUE;
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPVT | PRF_DISPANSI);
    SET_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISPVIC);
  } else if (!str_cmp(argument, "none")) {
    if (PRF_FLAGGED(ch, PRF_DISPVT))
      changed = TRUE;
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISPVIC | PRF_DISPVT | PRF_DISPANSI);
  } else if (!str_cmp(argument, "ansi")) {
    if (!PRF_FLAGGED(ch, PRF_DISPVT))
      changed = TRUE;
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISPVIC);
    SET_BIT(PRF_FLAGS(ch), PRF_DISPVT | PRF_DISPANSI);
  } else if (!PRF_FLAGGED(ch, PRF_DISPVT)) {
    for (i = 0; i < strlen(argument); i++) {
      switch (LOWER(argument[i])) {
      case 'h': SET_BIT(PRF_FLAGS(ch), PRF_DISPHP); break;
      case 'm': SET_BIT(PRF_FLAGS(ch), PRF_DISPMANA); break;
      case 'v': SET_BIT(PRF_FLAGS(ch), PRF_DISPMOVE); break;
      case 't': SET_BIT(PRF_FLAGS(ch), PRF_DISPVIC); break;
      }
    }
  }

  if (ch->desc && changed && PRF_FLAGGED(ch, PRF_DISPVT)) {
    redraw_screen(ch->desc);
    sprintf(buf, VTCURPOS VTDELEOS,
	    GET_SCRLEN(ch->desc->character), 1);
    write_to_descriptor(ch->desc->descriptor, buf);
  } else if (ch->desc && changed)
    write_to_descriptor(ch->desc->descriptor, VTCLS);

  if (old != PRF_FLAGS(ch))
    send_to_char("Ok.\r\n", ch);
  else
    send_to_char("No change made to display.\r\n", ch);
}


ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, *filename;
  struct stat fbuf;
  time_t ct;
  int max_filesize = 50000;

  if (IS_NPC(ch)) {
    send_to_char("Yeah, I've got an idea - leave me alone.\r\n", ch);
    return;
  }

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(buf, CMP, LEVEL_DEITY, FALSE);

  if (stat(filename, &fbuf) < 0) {
    perror("Error statting file");
    return;
  }
  if (fbuf.st_size >= max_filesize) {
    send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
          world[ch->in_room]->number, argument);
  fclose(fl);
  send_to_char("Okay.  Thanks!\r\n", ch);
}



/*
ACMD(do_gen_write)
{
  FILE * fl;
  char	*tmp, *filename;
  long	ct;
  char	str[MAX_STRING_LENGTH];

  switch (subcmd) {
  case SCMD_BUG:	filename = BUG_FILE; break;
  case SCMD_TYPO:	filename = TYPO_FILE; break;
  case SCMD_IDEA:	filename = IDEA_FILE; break;
  default: return;
  }

  ct  = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
    return;
  }

  for (; isspace(*argument); argument++)
    ;

  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }

  if (!(fl = fopen(filename, "a"))) {
    perror ("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  sprintf(str, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
	  world[ch->in_room]->number, argument);
  fputs(str, fl);
  fclose(fl);
  send_to_char("Ok.  Thanks.  :)\r\n", ch);
}
*/

static char	*ctypes[] = {
  "off", "sparse", "normal", "complete", "test", "\n" };

ACMD(do_color)
{
  int	tp;

  one_argument (argument, arg);

  if (!*arg) {
    sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
    send_to_char(buf, ch);
    return;
  }

  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char ("Usage: color { Off | Sparse | Normal | Complete | Test }\r\n", ch);
    return;
  } else if (tp == 4) {
    send_to_char(COLOR_TEST, ch);
    return;
  }

  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
  SET_BIT(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

  sprintf (buf, "Your §rc§Yo§yl§go§Gr§bm§co§Cd§me§N is now %s.\r\n", ctypes[tp]);
  send_to_char(buf, ch);
}


static char	*logtypes[] = {
  "off", "brief", "normal", "complete", "\n" };

ACMD(do_syslog)
{
  int	tp;

  if (IS_NPC(ch))
    return;

  one_argument (argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }

  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char ("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }

  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}


#define TOG_OFF 0
#define TOG_ON  1

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_tog)
{
  long	result;
  extern int level_can_chat;

  char	*tog_messages[][2] = {
    { "You are now safe from summoning by other players.\r\n",
	"You may now be summoned by other players.\r\n" },
{ "Nohassle disabled.\r\n",
    "Nohassle enabled.\r\n" },
{ "Brief mode off.\r\n",
    "Brief mode on.\r\n" },
{ "Compact mode off.\r\n",
    "Compact mode on.\r\n" },
{ "You can now hear tells.\r\n",
    "You are now deaf to tells.\r\n" },
{ "You can now hear auctions.\r\n",
    "You are now deaf to auctions.\r\n" },
{ "You can now hear the newbie channel.\r\n",
    "You are now deaf to the newbie channel.\r\n" },
{ "You can now hear gossip.\r\n",
    "You are now deaf to gossip.\r\n" },
{ "You can now hear the chats.\r\n",
    "You are now deaf to the chats.\r\n" },
{ "You can now hear the Wiz-channel.\r\n",
    "You are now deaf to the Wiz-channel.\r\n" },
{ "You are no longer part of the Quest.\r\n",
    "Okay, you are part of the Quest!\r\n" },
{ "You will no longer see the room flags.\r\n",
    "You will now see the room flags.\r\n" },
{ "You will now have your communication repeated.\r\n",
    "You will no longer have your communication repeated.\r\n" },
{ "HolyLight mode off.\r\n",
    "HolyLight mode on.\r\n" },
{ "Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
    "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n" },
{ "You can now use the speedwalk.\r\n",
    "Speedwalking is now disabled.\r\n"},
{ "Input will not be processed for alias.\r\n",
    "Alias processor on.\r\n" },
{ "Multi-command-line and numerical input enabled.\r\n",
    "Multi-command-line and numerical input disabled.\r\n" },
{ "Will display short-exits.\r\n",
    "Short-exits won't be displayed now.\r\n" },
{ "Removing gag. All messages will be displayed.\r\n",
    "Unnecessary messages will be gagged.\r\n"},
{ "PC/IBM-mode disabled.\r\n",
    "PC/IBM-mode enabled.\r\n" },
{ "You can now hear the grats.\r\n",
    "You are now deaf to the grats.\r\n" },
{ "Last commands will no longer be saved.\r\n",
    "Last commands will be saved.\r\n" },
{ "Your clan & clanlevel will show in title.\r\n",
    "Your clan & clanlevel will NOT show in title.\r\n"},
{ "You can now hear clan tells.\r\n",
    "You have turned off clan tells.\r\n" },
{ "Linewrap mode is now off.\r\n",
    "Linewrap mode is now on.\r\n" },
{ "Autoassist mode is now off.\r\n",
    "Autoassist mode is now on.\r\n" },
{ "You will now show up on the who list.\r\n",
    "You will no longer show up on the who list.\r\n" },
{ "Ident is changed to NO; Ident will no longer be resolved.\r\n",
    "Ident changed to YES; Ident will be resolved.\r\n" },
{ "Damage Display set to brief mode.\r\n",
   "Damage Display set to detailed mode.\r\n" },
{ "You will now hear the pkok channel.\r\n",
   "You are now deaf to the pkok channel.\r\n" }
};


  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_NOSUMMON	: result = PRF_TOG_CHK(ch, PRF_SUMMONABLE); break;
  case SCMD_NOHASSLE	: result = PRF_TOG_CHK(ch, PRF_NOHASSLE); break;
  case SCMD_BRIEF	: result = PRF_TOG_CHK(ch, PRF_BRIEF); break;
  case SCMD_COMPACT	: result = PRF_TOG_CHK(ch, PRF_COMPACT); break;
  case SCMD_NOTELL	: result = PRF_TOG_CHK(ch, PRF_NOTELL); break;
  case SCMD_NOGRAT	: result = PRF_TOG_CHK(ch, PRF_NOGRAT); break;
  case SCMD_NOAUCTION	: result = PRF_TOG_CHK(ch, PRF_NOAUCT); break;
  case SCMD_NONEWBIE	: result = PRF_TOG_CHK(ch, PRF_NONEWBIE); break;
  case SCMD_NOGOSSIP	: result = PRF_TOG_CHK(ch, PRF_NOGOSS); break;
  case SCMD_NOCHAT 	: result = PRF_TOG_CHK(ch, PRF_NOCHAT); break;
  case SCMD_NOWIZ	: result = PRF_TOG_CHK(ch, PRF_NOWIZ); break;
  case SCMD_QUEST	:
            if (is_quest || GET_LEVEL(ch) >= LEVEL_DEITY)
                          result = PRF_TOG_CHK(ch, PRF_QUEST);
            else {
              send_to_char("The Quest channel is currently closed.\r\n", ch);
              return;
            }
	    break;
  case SCMD_ROOMFLAGS	: result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS); break;
  case SCMD_NOREPEAT	: result = PRF_TOG_CHK(ch, PRF_NOREPEAT); break;
  case SCMD_HOLYLIGHT   : result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT); break;
  case SCMD_SLOWNS	: result = (nameserver_is_slow = !nameserver_is_slow);
    break;
  case SCMD_LASTCMD	: result = (save_last_command = !save_last_command);
    break;
  case SCMD_NOSPDWLK   : result = PRF_TOG_CHK(ch, PRF_NOSPDWLK); break;
  case SCMD_NOALIAS    : result = PRF_TOG_CHK(ch, PRF_NOALIAS); break;
  case SCMD_VERBATIM   : result = PRF_TOG_CHK(ch, PRF_VERBATIM); break;
  case SCMD_EXITS      : result = PRF_TOG_CHK(ch, PRF_NOEXITS); break;
  case SCMD_GAG        : result = PRF_TOG_CHK(ch, PRF_GAG); break;
  case SCMD_PC         : result = PRF_TOG_CHK(ch, PRF_IBM_PC); break;
  case SCMD_NOCLANTIT  : result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_NOCLANTITLE) & PLR_NOCLANTITLE; break;
  case SCMD_NOCLANTELL : result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_NOCLANTELL) & PLR_NOCLANTELL; break;
  case SCMD_LINEWRAP   : result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_LINEWRAP) & PLR_LINEWRAP; break;
  case SCMD_AUTOASSIST : result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_AUTOASSIST) & PLR_AUTOASSIST; break;
  case SCMD_NOWHO      :
    if (GET_LEVEL(ch) >= LEVEL_IMMORT)
      result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_NOWHO) & PLR_NOWHO;
    else {
      send_to_char("Foolish mortal, that is beyond your ability...\r\n", ch);
      return;
    }
    break;
  case SCMD_IDENT :
    result = (ident = !ident); break;
  case SCMD_DETAIL:
    if (PLR_FLAGGED(ch, PLR_TEST))
      result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_DETAIL) & PLR_DETAIL;
    else {
      send_to_char("Foolish mortal, that is beyond your ability...\r\n", ch);
      return;
    }
    break;
  case SCMD_NOPKSAY  : 
    if (GET_LEVEL(ch) >= level_can_chat || REMORT(ch) > 0)
      result = TOGGLE_BIT(PLR_FLAGS(ch), PLR_NOPKSAY) & PLR_NOPKSAY; 
    else {
      sprintf(buf, "You can't enable this channel until you are level %d.\r\n",
                   level_can_chat);
      send_to_char(buf, ch);
    }
    break;
  default :
    log("SYSERR: Unknown subcmd in do_gen_toggle");
    return;
    break;
  }

  if (result)
    send_to_char(tog_messages[subcmd-SCMD_TOG_BASE][TOG_ON], ch);
  else
    send_to_char(tog_messages[subcmd-SCMD_TOG_BASE][TOG_OFF], ch);

  return;
}


ACMD(do_worship)
{
  int i;

  if (subcmd == SCMD_WORSHI) {
    send_to_char("You must type the complete command for this important change.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < 5) {
    send_to_char("You are too inexperienced to decide whom to worship.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) >= LEVEL_WORSHIP) {
    send_to_char("You are proud enough only to worship yourself.\r\n", ch);
    return;
    WORSHIPS(ch) = GET_IDNUM(ch);
  }

  if (!argument) {
    send_to_char("Worship which deity or hero?\r\n", ch);
    return;
  }

  skip_spaces(&argument);

  for (i = 0; *(argument + i); i++)
    *(argument + i) = LOWER(*(argument + i));


  if ((i = find_name(argument)) < 0) {
    send_to_char("There is no such deity or player.\r\n", ch);
    return;
  }

  if (player_table[i].level < LEVEL_WORSHIP) {
    send_to_char("Nay.  Forget that one.  Not high level enough for your attention.\r\n", ch);
    return;
  }

  send_to_char("Ok.  You start a new religious life.\r\n", ch);

  if (WORSHIPS(ch) && WORSHIPS(ch) != player_table[i].worships) {
    send_to_char("All experience lost from earlier life.\r\n", ch);
    GET_EXP(ch) = 0;
  }

  WORSHIPS(ch) = player_table[i].worships;
}


ACMD(do_setinnate)
{
  struct char_data *vict;
  char name[80];

  if (IS_NPC(ch))
    return;

  one_argument(argument, name);

  if (!(vict = get_char_vis(ch, name))) {
    send_to_char("No such person.\r\n", ch);
    return;
  }

  if (vict->affected) {
    vict->affected->duration = DURATION_INNATE;
  }

  sprintf(buf, "(GC) %s: setinnate %s", GET_NAME(ch), GET_NAME(vict));
  mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
}


void afs_force_save(void)
{
  extern FILE *player_fl;

  mudlog("(GC) Forcing physical save of pfile.", BRF, LEVEL_DEITY, TRUE);

  fclose(player_fl);

  if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
    perror("Error opening playerfile");
    exit(1);
  }

  rewind(player_fl);

}



ACMD(do_temp)
{
  if (IS_NPC(ch))
    return;

  afs_force_save();
}


void	count_objs(struct obj_data *obj, int *num)
{
   if (obj) {
      count_objs(obj->contains, num);
      count_objs(obj->next_content, num);
      ++(*num);
   }
}


ACMD(do_count)
{
     int i = 0, j;

     for (j = 0; j < MAX_WEAR; j++)
	  if (ch->equipment[j])
	       count_objs(ch->equipment[j], &i);

     count_objs(ch->carrying, &i);

     sprintf(buf, "You have #C%d#N objects.\r\n", i);
     send_to_char(buf, ch);
}


ACMD(do_ignore)
{
  extern int get_player_lvl(char *name);

  int victim_level;

  one_argument(argument, arg);

  if (IS_NPC(ch))
  {
        send_to_char("You're a mob! If they are annoying kill them!\r\n", ch);
        return;
  }

  /* Ignore for imms is a big NONO! */
  if (GET_LEVEL(ch) >= LEVEL_DEITY)
  {
    send_to_char("Ignore? Shame on you. That's not a very divine act now is it.\r\n", ch);
    return;
  }

  if (!*arg)
  {
    show_ignore(ch);
    return;
  }

  if (*arg == '*')
  {
    if (ch->specials.ignore_list)
    {
      free_ignore_list(ch->specials.ignore_list);
      ch->specials.ignore_list = NULL;
      send_to_char("Clearing ignore list.\r\n", ch);
      SET_BIT(PLR_FLAGS(ch), PLR_SAVEIGN);
    }
    else
      send_to_char("Ignore list already empty.\r\n", ch);
    return;
  }

  if (!str_cmp(GET_NAME(ch), arg))
  {
    send_to_char("Don't be silly! You can't ignore yourself!\r\n", ch);
    return;
  }

  if (!(victim_level = get_player_lvl(arg)))
  {
    /* No player found or level is 0 */
    send_to_char("If you want the voices to go away, might I suggest a good shrink?\r\n", ch);
    return;
  }
  else
  {
    if (victim_level>= LEVEL_DEITY)
    {
      /* No ignoring Immortals *sigh* */
      send_to_char("Foolish mortal! You can't ignore a God!\r\n", ch);
      return;
    }
    else
    {
      if (!is_ignoring(ch, arg))
      {
        /* We aren't ignoring them lets add them if allowed */
        if (!allow_ignore(ch))
        {
          /* Already ignoring MAX_IGNORE players*/
          send_to_char("If you want to ignore everyone just toggle all channels.\r\n", ch);
          return;
        }
        else
        {
          /* Ok, lets add them */
          add_ignore(ch, arg);
          sprintf(buf, "Ok, %s probably doesn't have anything good to say anyway.\r\n", CAP(arg));
          send_to_char(buf, ch);
        }
      }
      else
      {
        /* Stop ignoring the player */
        remove_ignore(ch, arg);
        sprintf(buf, "Ok, lets hear what %s has to say.\r\n", CAP(arg));
        send_to_char(buf, ch);
      }
    }
  }
}



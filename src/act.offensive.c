/* ************************************************************************
*   File: act.offensive.c                               Part of EliteMUD  *
*  Usage: player-level commands of an offensive nature                    *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "functions.h"

/* extern variables */
extern struct room_data **world;
extern struct descriptor_data *descriptor_list;
extern int    pk_allowed;
extern int    pkok_allowed;
extern const struct str_app_type str_app[];



ACMD(do_assist)
{
  struct char_data *helpee, *opponent;
  struct follow_type *f;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);
  helpee = NULL;

  if (!*arg) {
    if (PROTECTING(ch) && FIGHTING(PROTECTING(ch)))
      helpee = PROTECTING(ch);
    else if (IS_AFFECTED(ch, AFF_GROUP)) {
      helpee = (ch->master?ch->master:ch);
      f = helpee->followers;
      while (f && helpee && !(FIGHTING(helpee) && helpee != ch && IN_ROOM(helpee) == IN_ROOM(ch)))
	{
	  helpee = f->follower;
	  f = f->next;
	}
    }
    if (!helpee || helpee == ch) {
      send_to_char("Whom do you wish to assist?\r\n", ch);
      return;
    }
  }    
    
  if (!helpee && !(helpee = get_char_room_vis(ch, arg)))
    send_to_char("No such person around.\r\n", ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else {
    if (FIGHTING(helpee))
      opponent = FIGHTING(helpee);
    else
      for (opponent = world[ch->in_room]->people; opponent &&
	   (FIGHTING(opponent) != helpee); 
	   opponent = opponent->next_in_room);
    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!pkok_check(ch, opponent));
    else if (opponent != ch) {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}


/* Modified to incorporate PKOK system - Bod */
int pkok_check(struct char_data *ch, struct char_data *victim)
{

  if(IS_SET(world[ch->in_room]->room_flags, LAWFULL)) {
       send_to_char("No.  You can't bring yourself to hurt someone here, in this place.\r\n", ch);
       return 0;
  }

  if (!pkok_allowed) {
    if (!pk_allowed && !ROOM_FLAGGED(IN_ROOM(victim), PKOK) &&
        FIGHTING(ch) != victim && FIGHTING(victim) != ch) {
      if (!IS_NPC(victim) && !IS_NPC(ch)) {
        send_to_char("Player killing is not allowed.\r\n", ch);
        return 0;
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(victim))
        return 0;  /* you can't order a charmed pet to attack a player */
    }
  }
  else if (!PLR_FLAGGED(ch, PLR_PKOK) || !PLR_FLAGGED(victim, PLR_PKOK)) {
    if (!pk_allowed && !ROOM_FLAGGED(IN_ROOM(victim), PKOK) &&
        FIGHTING(ch) != victim && FIGHTING(victim) != ch) {
      if (!IS_NPC(victim) && !IS_NPC(ch)) {
        send_to_char("Player killing is not allowed.\r\n", ch);
        return 0;
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(victim))
        return 0;  /* you can't order a charmed pet to attack a player */
    }
  }

  return 1;
}


ACMD(do_hit)
{
  struct char_data *vict;
    
  one_argument(argument, arg);
    
  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (subcmd != SCMD_MURDER || IS_NPC(ch))
      if (!pkok_check(ch, vict))
	return;

    if (subcmd == SCMD_MURDER) {
      if (!PLR_FLAGGED(vict, PLR_KILLER | PLR_THIEF)) {
        send_to_char("You can only murder a murderer!!!\r\n", ch);
        return;
      }
    }
    

    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}


ACMD(do_kill)
{
  struct char_data *victim;
    
  if ((GET_LEVEL(ch) < LEVEL_IMMORT) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
    
  one_argument(argument, arg);
    
  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(victim = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == victim)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
	 if (GET_LEVEL(ch) >= GET_LEVEL(victim)) {
	      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, victim, TO_CHAR);
	      act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
	      act("$n brutally slays $N!", FALSE, ch, 0, victim, TO_NOTVICT);
	      
	      if (GET_LEVEL(victim) >= LEVEL_DEITY && !IS_NPC(victim))
	      {
		   send_to_char("You are dead! Heh heh...  Damn! You are immortal.\r\n", victim);
		   char_from_room(victim);
		   char_to_room(victim, 0);
	      }
	      else
		   raw_kill(victim, ch);
	 } else {
	      send_to_char("Pick on someone your own size!!!\r\n", ch);
	 }
    }
  }
}



ACMD(do_backstab)
{
   struct char_data *victim;
   byte percent, prob;

   one_argument(argument, buf);

   if (!(victim = get_char_room_vis(ch, buf))) {
      send_to_char("Backstab who?\r\n", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("How can you sneak up on yourself?\r\n", ch);
      return;
   }

   if (!ch->equipment[WIELD]) {
      send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
      return;
   }

   if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
      send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
      return;
   }

   if (!pkok_check(ch, victim))
       return;

   if(IS_SET(world[ch->in_room]->room_flags,LAWFULL))
	 {
	     act("Naah..You feels to...Humble dude.. GO AWAY",FALSE,ch,0,0,
		 TO_CHAR);
	     act("$n tries to start a fight but lights a joint instead.",TRUE,
		 ch,0,0,TO_ROOM);
	     return;
	 }

   if (victim->specials.fighting) {
      send_to_char("You can't backstab a fighting person, too alert!\r\n", ch);
      return;
   }

   percent = number(1, 101); /* 101% is a complete failure */

   prob = GET_SKILL(ch, SKILL_BACKSTAB);

   if (AWAKE(victim) && (percent > prob))
     damage(ch, victim, 0, SKILL_BACKSTAB);
   else {
     hit(ch, victim, SKILL_BACKSTAB);
     improve_skill(ch, SKILL_BACKSTAB);
   }

   WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_circle)
{
  struct char_data *victim;
  byte percent, prob;
    
  one_argument(argument, buf);
    
  if (!(victim = get_char_room_vis(ch, buf)) && !(victim = ch->specials.fighting)) {
    send_to_char("Circle around whom?\r\n", ch);
    return;
  }
    
  if (victim == ch) {
    send_to_char("You circle around yourself easily, looking like a fool.\r\n", ch);
    return;
  }
    
  if (!ch->equipment[WIELD] || ch->equipment[WIELD]->obj_flags.value[3] != 11) {
    send_to_char("You have to wield a piercing weapon.\r\n", ch);
    return;
  }

  if (!pkok_check(ch, victim))
    return;

  percent = number(1, 101);	/* 101% is a complete failure */
    
  prob = GET_SKILL(ch, SKILL_CIRCLE_AROUND);
    
  if (AWAKE(victim) && (percent > prob)) {
    act("You try to circle around $N but fail.",TRUE,ch,0,victim,TO_CHAR);
    act("$n tries to circle around $N but fails.",TRUE,ch,0,victim,TO_NOTVICT);
    act("$n tries to circle around you but fails.",TRUE,ch,0,victim,TO_VICT);
  } else {
    act("You easily circle around $N.",TRUE,ch,0,victim, TO_CHAR);
    act("$n easily circles around $N.",TRUE,ch,0,victim, TO_NOTVICT);
    improve_skill(ch, SKILL_CIRCLE_AROUND);

    percent = number(1, 101);	/* 101% is a complete failure */

    prob = GET_SKILL(ch, SKILL_BACKSTAB);
	
    if (AWAKE(victim) && (percent > prob))
      damage(ch, victim, 0, SKILL_BACKSTAB);
    else {
      hit(ch, victim, SKILL_BACKSTAB);
      improve_skill(ch, SKILL_BACKSTAB);
    }
  }
    
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_order)
{
  char name[MAX_INPUT_LENGTH], message[256];
  char buf[256];
  bool found = FALSE;
  int	org_room;
  struct char_data *victim;
  struct follow_type *k, *next;
    
  half_chop(argument, name, message);
    
  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even issue orders.\r\n", ch);
    return;
  }

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(victim = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == victim)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);
    
  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
	
    if (victim) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
	    
      if ( (victim->master != ch) || !IS_AFFECTED(victim, AFF_CHARM) )
	act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
      else {
	send_to_char("Ok.\r\n", ch);
	command_interpreter(victim, message);
      }
    } else {			/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);
	    
      org_room = ch->in_room;
	    
      for (k = ch->followers; k; k = next) {
	next = k->next;
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char("Ok.\r\n", ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}


ACMD(do_flee)
{
  int i, escape, attempt, loose, die;
  struct char_data *k;
  struct room_data *rm;

  void gain_exp(struct char_data *ch, int gain);
  int special(struct char_data *ch, int cmd, char *arg);

  if (GET_POS(ch) < POS_FIGHTING)
  {
    send_to_char("You are in pretty bad shape, unable to flee!\r\n", ch);
    return;
  }

  if (!(FIGHTING(ch)))
  {
    for (i = 0; i < 6; i++)
    {
      attempt = number(0, NUM_OF_DIRS-1); /* Select a random direction */
      if (CAN_GO(ch, attempt) &&
         !IS_SET(world[EXIT(ch, attempt)->to_room]->room_flags, DEATH) &&
        (!IS_SET(world[EXIT(ch, attempt)->to_room]->room_flags, IN_AIR) ||
          IS_AFFECTED(ch, AFF_FLY)) &&
         !IS_SET(world[EXIT(ch, attempt)->to_room]->room_flags, GODROOM))
      {
        act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
        fall_of_mount(ch);
        if ((die = do_simple_move(ch, attempt, FALSE)) == 1)
        {
          /* The escape has succeded */
          send_to_char("MOMMY!!! You flee head over heels.\r\n", ch);
        }
        else
        {
          if (!die)
            act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
        }
        return;
      }
    } /* for */
    /* No exits was found */
    send_to_char("PANIC!  You couldn't escape!\r\n", ch);
    return;
  }

  loose = GET_MAX_HIT(FIGHTING(ch)) - GET_HIT(FIGHTING(ch));
  loose *= GET_LEVEL(FIGHTING(ch));

  // The room we try to flee from...
  rm = world[IN_ROOM(ch)];

  for (i = 0; i < 6; i++)
  {
    attempt = number(0, NUM_OF_DIRS-1);/* Select a random direction */
    if (CAN_GO(ch, attempt) &&
       !IS_SET(world[EXIT(ch, attempt)->to_room]->room_flags, DEATH))
    {
      if ((escape = (number(1, 160) < GET_SKILL(ch, SKILL_ESCAPE))))
      {
        act("$n starts to back away and escape.", TRUE, ch, 0, 0, TO_ROOM);
        improve_skill(ch, SKILL_ESCAPE);
      }
      else
        act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);

      fall_of_mount(ch);

      if ((die = do_simple_move(ch, attempt, FALSE)) == 1)
      {
        /* The escape has succeded */
        if (!escape)
        {
          if (!IS_NPC(ch))
            gain_exp(ch, -loose);
          send_to_char("You flee head over heels.\r\n", ch);
        }
        else
          send_to_char("You escape from your foes.\r\n", ch);

        /* Insert later when using huntiing system        */
        /* ch->specials.fighting->specials.hunting = ch */

        /* Make sure everyone in that room that we fleed from and
           was fighting us stops fighting us. */
        for (k=rm->people;k;k=k->next_in_room)
          if (FIGHTING(k) == ch)
            stop_fighting(k);

        return;
      }
      else
      {
        if (!die)
          act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
        return;
      }
    }
  } /* for */

  /* No exits was found */
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_bash)
{
     struct char_data *victim;
     struct room_direction_data *back;
     byte percent, prob;
     char dir[MAX_INPUT_LENGTH], buf[SMALL_BUFSIZE];
     int door, other_room;
     extern int rev_dir[];


     two_arguments(argument, buf, dir);
     
     if (!ch->equipment[WEAR_SHIELD]) {
	  send_to_char("You need to have a shield, to bash something.\r\n", ch);
	  return;
     }

     /* Check for Door First if not fighting and victim is not in room */
     
     if (!(victim = get_char_room_vis(ch, buf)) && !(ch->specials.fighting)) { 
       if ((door = find_door(ch, buf, dir)) >= 0) {
	 if (GET_HIT(ch) <= 25) {
	   send_to_char("You feel too weak to bash the door down\r\n", ch);
	   return;
	 }
	 else if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	   send_to_char("That's absurd.\r\n", ch);
         else if (IS_SET(EXIT(ch, door)->exit_info, EX_BROKEN))
           send_to_char("Heck.. it's already broken!\r\n", ch); 
	 else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	   send_to_char("Heck.. it ain't even closed!\r\n", ch);
	 else {
	   if ((number(1, 1000) > (GET_SKILL(ch, SKILL_BASH) + str_app[STRENGTH_APPLY_INDEX(ch)].bash)) || 
	       IS_SET(EXIT(ch, door)->exit_info, EX_BASHPROOF)) {
	     send_to_char("You bounce off the door.\r\n", ch);
	     GET_HIT(ch) = MAX(1, GET_HIT(ch) - number(1, 25)); 
	     /* 1-25 hp loss per failed attempt  */
	     return;
	   } else {
	     REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	     REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
             SET_BIT(EXIT(ch, door)->exit_info, EX_BROKEN); // Make it Break - Charlene
	     if (EXIT(ch, door)->keyword)
	       act("$n bashes down the $F.", 0, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
	     else
	       act("$n bashes down the door.", 0, ch, 0, 0, TO_ROOM);
	     send_to_char("*smash*\r\n", ch);
	     /* now for unlocking the other side, too */
	     if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	       if ((back = world[other_room]->dir_option[rev_dir[door]]))
		 if (back->to_room == ch->in_room) {
		   if (back->keyword) {
		     sprintf(buf, "The %s is bashed down from the other side.\r\n", fname(back->keyword));
		     send_to_room(buf, EXIT(ch, door)->to_room);
		   } else
		     send_to_room("A door is bashed down.\r\n", EXIT(ch, door)->to_room);
		   REMOVE_BIT(back->exit_info, EX_LOCKED);
		   REMOVE_BIT(back->exit_info, EX_CLOSED);
                   SET_BIT(back->exit_info, EX_BROKEN); // And the other side as well - Charlene
		 }
	   }
	 }
       } 
       return;
     } 
     
     if (ch->specials.fighting) 
       victim = ch->specials.fighting;
     
     if (victim == ch) {
	  send_to_char("Aren't we funny today...\r\n", ch);
	  return;
     }
          
     if (!pkok_check(ch, victim))
	  return;
     
     percent = number(1, 101);	/* 101% is a complete failure */
     prob = GET_SKILL(ch, SKILL_BASH);
     
     if (percent > prob) {
	  act("You try to bash $N but kiss the ground instead.",TRUE,ch,0,victim,TO_CHAR);
	  act("$n tries to bash $N but ends up eating dirt.",TRUE,ch,0,victim,TO_NOTVICT);
	  act("$n tries to bash you but fails.",TRUE,ch,0,victim,TO_VICT);
          if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL) && \
              !PRF_FLAGGED(ch, PRF_GAG)) /* Damage Detail - GAG Check as MISS */
            send_to_char("[#GBash Skill#N  (#rWAIT:#w0 (ch:2) #yFAILURE#N ) ", ch);
	  damage(ch, victim, 0, SKILL_BASH);
	  GET_POS(ch) = POS_SITTING;
     } else {
	  act("You easily bash $N.",TRUE,ch,0,victim, TO_CHAR);
	  act("$n easily bashes $N.",TRUE,ch,0,victim, TO_NOTVICT);
	  GET_POS(victim) = POS_SITTING;
	  WAIT_STATE(victim, PULSE_VIOLENCE);
          if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) /* Damage Detail */
            send_to_char("[#GBash Skill#N  (#rWAIT:#w1 #ySIT#w (ch:2)#N ) (To Dam: 10) ", ch);
	  damage(ch, victim, 10, SKILL_BASH);
     }
     WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}



ACMD(do_rescue)
{
  struct char_data *victim, *tmp_ch;
  byte	percent, prob;

  one_argument(argument, arg);

  if (!(victim = get_char_room_vis(ch, arg))) {
    send_to_char("Who do you want to rescue?\r\n", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }

  if (ch->specials.fighting == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }

  for (tmp_ch = world[ch->in_room]->people; tmp_ch && 
       (tmp_ch->specials.fighting != victim); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if (!pkok_check(ch, tmp_ch))
    return;

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_RESCUE);
   
  if (percent > prob) {
    send_to_char("You fail the rescue!\r\n", ch);
    return;
  }
   
  send_to_char("Banzai!  To the rescue...\r\n", ch);
  act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
  act("$n heroically rescues $N!", FALSE, ch, 0, victim, TO_NOTVICT);

  if (victim->specials.fighting == tmp_ch)
    stop_fighting(victim);
  if (tmp_ch->specials.fighting)
    stop_fighting(tmp_ch);
  if (ch->specials.fighting)
    stop_fighting(ch);
   
  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);
   
  improve_skill(ch, SKILL_RESCUE);

  WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
}


ACMD(do_kick)
{
  struct char_data *victim;
  byte percent, prob;

  one_argument(argument, arg);

  if (!(victim = get_char_room_vis(ch, arg))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pkok_check(ch, victim))
    return;

  percent = ((10 - (GET_AC(victim) / 10)) << 1) + number(1, 101); /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_KICK);

  if (percent > prob) {
          if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL) && \
              !PRF_FLAGGED(ch, PRF_GAG)) /* Damage Detail - GAG Check as MISS */
            send_to_char("[#GKick Skill#N  (#rWAIT:#w0 (ch:3) #yFAILURE#N ) ", ch);
    damage(ch, victim, 0, SKILL_KICK);
  } else {
      if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) { /* Damage Detail */
        sprintf(buf, "[#GKick Skill#N  (#rWAIT:#w0 (ch:3)#N ) (To Dam: %d) ",
                GET_LEVEL(ch) >> 1);
        send_to_char(buf, ch);
    }
    damage(ch, victim, GET_LEVEL(ch) >> 1, SKILL_KICK);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_throw)
{
  struct char_data *victim;
  byte percent, prob;
  int  dam, mult;
    
  one_argument(argument, arg);
    
  if (!(victim = get_char_room_vis(ch, arg))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Throw at whom?\r\n", ch);
      return;
    }
  }
    
  if (victim == ch) {
    send_to_char("Aren't we jolly funny today...\r\n", ch);
    return;
  }
    
  if (!ch->equipment[WIELD]) {
    send_to_char("Try wielding something to throw.\r\n", ch);
    return;
  }

  if (!pkok_check(ch, victim))
    return;

  percent = (GET_AC(victim) + GET_SKILL(ch, SKILL_THROW))/2; 
  prob = number(1,101); 
  if ((ch->equipment[WIELD])->obj_flags.value[3] == 11)
    dam = GET_OBJ_WEIGHT(ch->equipment[WIELD]) * GET_LEVEL(ch);
  else
    dam = GET_LEVEL(ch)/2;
  
  mult = 1;
  if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_BLESS) && IS_EVIL(victim))
     mult += 1;
  if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_EVIL)  && IS_GOOD(victim))
     mult += 1;
  if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_FLAME))
     if (!saves_spell(victim,SAVING_PHYSICAL, NULL, SAVE_NEGATE))
        dam += dice(3,6);
  dam *= mult;
  
  if (prob > percent) {
    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL) && \
        !PRF_FLAGGED(ch, PRF_GAG)) /* Damage Detail - GAG Check as MISS */
      send_to_char("[#GThrow Skill#N  (#rWAIT:#w0 (ch:2) #yFAILURE#N ) ", ch);
    damage(ch, victim, 0, SKILL_THROW);
  } else
      if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DETAIL)) { /* Damage Detail */
        sprintf(buf, "[#GThrow Skill#N  (#rWAIT:#w0 (ch:2)#N ) (To Dam: %d) ",
                MIN(GET_LEVEL(ch), dam));
        send_to_char(buf, ch);
    }
    damage(ch, victim, MIN(GET_LEVEL(ch), dam), SKILL_THROW);
    
  obj_to_room(unequip_char(ch, WIELD), ch->in_room);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_berzerk)
{
  struct affected_type af; 

  if (IS_NPC(ch)) {
    send_to_char("Too stupid.  Go away!\r\n", ch);
    return;
  }

  if (ch->specials.fighting && 
      number(1, 101) < GET_SKILL(ch, SKILL_BERZERK)) {
    send_to_char("You work up a frenzy and will fight to death!\r\n", ch);
    act("$n works up a frenzy and charges $N.", 
	TRUE, ch, 0, ch->specials.fighting, TO_ROOM);
    SET_BIT(ch->specials.affected_by, AFF_BERZERK);
    af.location = APPLY_HIT;
    af.modifier = MIN(GET_LEVEL(ch), 100);
    af.duration = -1;
    af.bitvector = AFF_BERZERK;
    af.type = SKILL_BERZERK;
    affect_join(ch, &af, FALSE, FALSE);
    GET_HIT(ch) += MIN(GET_LEVEL(ch), 100);
    af.location = APPLY_MOVE;
    af.modifier = MIN(GET_LEVEL(ch)/2, 50);
    af.duration = -1;
    af.bitvector = AFF_BERZERK;
    af.type = SKILL_BERZERK;
    affect_join(ch, &af, FALSE, FALSE);
    GET_MOVE(ch) += MIN(GET_LEVEL(ch)/2, 50);
  } else {
    send_to_char("You can't find the energy.\r\n", ch);
    act("$n hops around like a frenzied pig...", TRUE, ch, 0, 0, TO_ROOM);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);
}
	


ACMD(do_disarm)
{
  struct char_data *victim;
  byte percent, prob;
    
  victim = ch->specials.fighting;
    
  if (!victim) {
    send_to_char("You are not fighting anyone.\r\n", ch);
    return;
  }
    
  if (!victim->equipment[WIELD] ||
      (victim->equipment[WIELD] && 
       !CAN_SEE_OBJ(ch, victim->equipment[WIELD]))) {
    act("$N isn't wielding anything.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  percent = (GET_SKILL(ch, SKILL_DISARM) - GET_LEVEL(victim)); 
  prob = number(1,101); 
  if (prob < percent) {
    act("$n makes a quick manouver, disarming you.", 
	TRUE, ch, 0, victim, TO_VICT);
    act("$n easily disarms $N, sending $p flying.", 
	TRUE, ch, victim->equipment[WIELD], victim, TO_NOTVICT);
    act("You send $S weapon flying.", TRUE, ch, 0, victim, TO_CHAR);
    obj_to_room(unequip_char(victim, WIELD), victim->in_room);
    improve_skill(ch, SKILL_DISARM);
  } else
    send_to_char("You try to disarm your opponent but fail.\r\n", ch);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}



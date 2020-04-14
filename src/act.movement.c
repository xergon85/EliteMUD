/* ************************************************************************
*   File: act.movement.c                                Part of EliteMUD  *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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
#include "functions.h"

/* external vars  */
extern struct room_data **world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern int	rev_dir[];
extern char	*dirs[];
extern char     *from_dir[];
extern int	movement_loss[];
extern sh_int r_mortal_start_room;
extern int max_recall_level;

/* external functs */
int   special(struct char_data *ch, int cmd, char *arg);
void  death_cry(struct char_data *ch);

ACMD(do_stand);
ACMD(do_look);
int roomflag_check(struct char_data *ch);
int exit_rprog_check(struct char_data *ch, int cmd);

char * leave_enter_string(struct char_data *ch)
{
     if ((world[ch->in_room]->sector_type == SECT_UNDERWATER) ||
	 (world[ch->in_room]->sector_type == SECT_WATER_SWIM &&
	  !IS_AFFECTED(ch, AFF_FLY | AFF_HOVER)))
	  return("swims");
     if (ch->specials.mounting)
	  return ("rides");
     else if (IS_AFFECTED(ch, AFF_FLY))
	  return ("flies");
     else if (IS_AFFECTED(ch, AFF_HOVER))
	  return ("hovers");
     else
	  return ("walks");    
}


int	do_simple_move(struct char_data *ch, int cmd, int following)
/* Assumes, 
	1. That there is no master and no followers.
	2. That the direction exists. 

   Returns :
   1 : If succes.
   0 : If fail
  -1 : If dead.
*/
{
   int	was_in;
   int	need_movement;
   struct   obj_data *obj;
   bool     has_boat;
   int i;

   if (special(ch, cmd + 1, ""))  /* Check for special routines (North is 1) */
      return(FALSE);

   need_movement = (movement_loss[atoi(&world[ch->in_room]->sector_type)] + 
       movement_loss[atoi(&world[world[ch->in_room]->dir_option[cmd]->to_room]->sector_type)]) / 2;

   if ((world[ch->in_room]->sector_type == SECT_WATER_NOSWIM) || 
       (world[world[ch->in_room]->dir_option[cmd]->to_room]->sector_type == SECT_WATER_NOSWIM)) {
       has_boat = FALSE;
       /* See if char is carrying a boat */
       for (obj = ch->carrying; obj; obj = obj->next_content)
	   if (obj->obj_flags.type_flag == ITEM_BOAT)
	       has_boat = TRUE;
       if (IS_AFFECTED(ch, AFF_FLY))
	   has_boat = TRUE;
       if (!has_boat) {
	   if (GET_SKILL(ch, SKILL_SWIM) == 0) {
	       send_to_char("You need a boat to go there.\r\n", ch);
	       return(FALSE);
	   } else if (number(1, 80) < GET_SKILL(ch, SKILL_SWIM)) {
	       improve_skill(ch, SKILL_SWIM);
	  } else {
	      send_to_char("The current is too strong for your feeble swimming skills.\r\n", ch);
	      return FALSE;
	  }
       }
   }
  
   /* Add in the Desert Movement */ 
   if ((world[ch->in_room]->sector_type == SECT_DESERT) || 
       (world[world[ch->in_room]->dir_option[cmd]->to_room]->sector_type == SECT_DESERT)) {
      if (GET_COND (ch, THIRST) > 0)
          GET_COND (ch, THIRST) -= 1;
	send_to_char("Hot drying winds blow around you.\r\n", ch);
   }

   /* Add in the Underwater Movement */ 
   if ((world[ch->in_room]->sector_type == SECT_UNDERWATER) || 
       (world[world[ch->in_room]->dir_option[cmd]->to_room]->sector_type == SECT_UNDERWATER)) {
      if (!IS_AFFECTED(ch, AFF_BREATH_WATER) &&
          (GET_LEVEL(ch) < LEVEL_DEITY)) {         
	   damage(ch, ch, number(1, 20), TYPE_SUFFOCATE);
         if (number(1, 101) >= GET_SKILL(ch, SKILL_SWIM)) {
	     send_to_char("Your diving skills need work!!!\r\n", ch);
	     return FALSE;
         } else {
           improve_skill(ch, SKILL_SWIM);
         }
      }
      if (GET_POS(ch) == POS_DEAD) return FALSE;
   } 

   if (!IS_NPC(ch) && world[world[ch->in_room]->dir_option[cmd]->to_room]->sector_type == SECT_MOUNTAIN && !IS_AFFECTED(ch, AFF_FLY))
   {
       if (GET_SKILL(ch, SKILL_CLIMB) == 0) {
	   send_to_char("This mountain looks impossible to climb to you.\r\n", ch);
	   return(FALSE);
       } else if (number(1, 101) < GET_SKILL(ch, SKILL_CLIMB)) {
	   improve_skill(ch, SKILL_CLIMB);
       } else {
	   damage(ch, ch, number(1, GET_MAX_HIT(ch)/10), TYPE_FALLING);
	   return FALSE;
       }
   }

   if ((ch->specials.mounting) || IS_AFFECTED(ch, AFF_FLY))
     need_movement = 0;
   else if (IS_AFFECTED(ch, AFF_HOVER))
     need_movement = 1;

   if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) {
     if (following && !ch->master)
       send_to_char("You are too exhausted to follow.\r\n", ch);
     else
       send_to_char("You are too exhausted.\r\n", ch);
     
     return(FALSE);
   }
   
   if (GET_LEVEL(ch) < LEVEL_DEITY && !IS_NPC(ch))
     GET_MOVE(ch) -= need_movement;

   if (IS_SET(world[ch->in_room]->room_flags,CHAOTIC)  && number(1,5) > 3
       && !IS_AFFECTED(ch, AFF_SNEAK))
   {
       act("$n leaves ... somewhere. You are a bit disoriented so you missed where...", TRUE, ch, 0, 0, TO_ROOM);
   }
   else if (!ch->specials.mounted_by &&
	    !IS_AFFECTED(ch, AFF_SNEAK))
   {
       sprintf(buf2, "$n %s %s.",leave_enter_string(ch), dirs[cmd]);
       act(buf2,TRUE,ch,0,ch->specials.mounted_by,TO_NOTVICT);
   }
   
   /* Add in the Exit being trapped */
   if (IS_SET(EXIT(ch, cmd)->exit_info, EX_TRAP) && IS_SET(EXIT(ch, cmd)->exit_info, EX_TRAPSET)) {
     if (number(1, 101) < GET_SKILL(ch, SKILL_DISARM_TRAP)) {
       REMOVE_BIT(EXIT(ch, cmd)->exit_info, EX_TRAPSET);  /* disarm trap */
       improve_skill(ch, SKILL_DISARM_TRAP);
     } else {
       if (number (1,5) > 3) {  /* 40% chance of tripping trap */
	 damage(ch, ch, number(1, GET_MAX_HIT(ch)/10), TYPE_TRAP);
	 if (GET_POS(ch) == POS_DEAD) return FALSE;
	 send_to_char("The Exit was trapped!!\r\n", ch);
       }
     }
   }

   if (world[ch->in_room]->rprogs) {
     if (exit_rprog_check(ch, cmd))
       return FALSE;
   }
   
   was_in = ch->in_room;

   char_from_room(ch);

   char_to_room(ch, world[was_in]->dir_option[cmd]->to_room);
  
   if (!ch->specials.mounted_by &&
       !IS_AFFECTED(ch, AFF_SNEAK)) 
   {
       sprintf(buf2, "$n %s in from %s.",leave_enter_string(ch),from_dir[rev_dir[cmd]]);
       act(buf2,TRUE,ch,0,0,TO_ROOM);
   }
   
   do_look(ch, "", 0, 0);

   if ((i = roomflag_check(ch)) != 1)
     return i;

   mprog_entry_trigger(ch);
   mprog_greet_trigger(ch);

   return(1);
}

/* check for effects from roomflags */ 
int roomflag_check(struct char_data *ch)
{
   struct affected_type *aff;

   if (IS_SET(world[ch->in_room]->room_flags, ZERO_MANA) &&
       !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
     send_to_char(" KABOOM! Your head spins around a couple of times as you lose all your mana.\r\n",ch);
     send_to_char("You faint!\r\n",ch);
     act("$n's eyeballs grow big as melons, and then he faints.",TRUE,ch,
	 0,0,TO_ROOM);
     GET_POS(ch) = POS_SLEEPING;
     GET_MANA(ch) = 0;
   }

   if (IS_SET(world[ch->in_room]->room_flags, DISPELL) &&
       !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
     if (ch->affected) {
       for (aff = ch->affected; aff; aff = aff->next) {
	 if (aff->duration != DURATION_INNATE)
	   affect_remove(ch, aff);
       }
       send_to_char("You feel your magical auras fade away...\r\n", ch);
       act("$n's magical auras fade away.", TRUE, ch, 0, 0, TO_ROOM);
     }
   }

   if (IS_SET(world[ch->in_room]->room_flags, DEATH) && 
       GET_LEVEL(ch) < LEVEL_DEITY) {
       log_death_trap(ch);
       death_cry(ch);
       char_from_room(ch);
       char_to_room(ch, r_mortal_start_room);
       send_to_char("Death traps are no fun.\r\n", ch);
       gain_exp(ch, 0 -  GET_EXP(ch)/4);
       GET_HIT(ch) = 1;
       return(-1);
   }

   /* Add in the DAMAGE Room flag */
   if (IS_SET(world[ch->in_room]->room_flags, DAMAGE) &&
	GET_LEVEL(ch) < LEVEL_DEITY) {
      damage(ch, ch, number(1, 20), TYPE_DAMAGE);
      if (GET_POS(ch) == POS_DEAD) 
	return (-1);
   } 
   return (1);
}


/* Procedure made by Rickard for CHAOTIC rooms */
/*   is also used for DRUNK people he he he he */
/* guess if we had fun making this             */

int
chaotic_move(struct char_data *ch,int direction)
{
    if (IS_NPC(ch)) return direction;

    if((IS_SET(world[ch->in_room]->room_flags,CHAOTIC) && number(1,5) < 3)
       || (number(1,24) < GET_COND(ch,DRUNK))) {
	 direction = number(0,5);
	 send_to_char("You feel a bit disoriented....\r\n",ch);
	 act("$n looks somewhat disoriented...",TRUE,ch,0,0,TO_ROOM);
    }
    return direction;
}
	    
#define MOUNT_MOVE      100

ACMD(do_move)
{
    int	was_in;
    struct follow_type *k, *next_dude;

    cmd = chaotic_move(ch,--cmd);
    
    if (!world[ch->in_room]->dir_option[cmd] ||
	world[ch->in_room]->dir_option[cmd]->to_room < 0) {
      send_to_char("Alas, you cannot go that way...\r\n", ch);
    } else if(IS_SET(world[world[ch->in_room]->dir_option[cmd]->to_room]->room_flags, GODROOM) && (GET_LEVEL(ch) < LEVEL_DEITY))
      send_to_char("You feel an barrier impeding your progress\r\n", ch);
    else if (IS_SET(world[world[ch->in_room]->dir_option[cmd]->to_room]->room_flags,IN_AIR) && !IS_AFFECTED(ch, AFF_FLY)){
      send_to_char("Naah! Try to grow wings first, will you!", ch);
      act("$n tries to leave, but suddenly realizes that $e's no bird..",
	  TRUE, ch, 0, 0, TO_ROOM);
    } else {          /* Direction is possible */
      if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)) {
	if (IS_SET(EXIT(ch, cmd)->exit_info, EX_SECRET))
	  send_to_char("Alas, you cannot go that way...\r\n", ch);
	else {
	  if (EXIT(ch, cmd)->keyword) {
	    sprintf(buf2, "The %s seems to be closed.\r\n",
		    fname(EXIT(ch, cmd)->keyword));
	    send_to_char(buf2, ch);
	  } else
	    send_to_char("It seems to be closed.\r\n", ch);
	}
      } else if (EXIT(ch, cmd)->to_room == NOWHERE)
	send_to_char("Alas, you cannot go that way...\r\n", ch);
      else if (ch->specials.mounting && !subcmd) {
	if (!AWAKE(MOUNTING(ch))) {
	  send_to_char("Your mount seems to be sleeping.\r\n", ch);
	} else if (GET_POS(MOUNTING(ch)) == POS_FIGHTING) {
	  send_to_char("Your mount is currently in mortal combat!\r\n", ch);
	} else {
	  if (GET_POS(MOUNTING(ch)) <= POS_SITTING) {
	    do_stand(MOUNTING(ch), "", 42, 0);
	  }
	  if (GET_RACE(ch->specials.mounting) == MOB_MOUNT &&
	      number(1, 100) < GET_SKILL(ch, SKILL_RIDING_LAND)) 
	    {
	      do_move(ch->specials.mounting, argument, cmd + 1, MOUNT_MOVE);
	      improve_skill(ch, SKILL_RIDING_LAND);
	    } else if (GET_RACE(ch->specials.mounting) == MOB_MOUNT_FLY &&
		       number(1, 110) < GET_SKILL(ch, SKILL_RIDING_AIR)) 
	      {
		do_move(ch->specials.mounting, argument, cmd + 1, MOUNT_MOVE);
		improve_skill(ch, SKILL_RIDING_AIR);
	      } else 
		send_to_char("Your mount doesn't seem to want to move.\r\n", ch);
	}
	return;
      } else if (!ch->followers && !ch->master && !ch->specials.mounted_by)
	do_simple_move(ch, cmd, FALSE);
      else {
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && 
	    (ch->in_room == ch->master->in_room) && subcmd != MOUNT_MOVE) {
	  send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
	  act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
	} else {
	  was_in = ch->in_room;
	  if (do_simple_move(ch, cmd, TRUE) == 1) { /* Move the character */
	    if (ch->specials.mounted_by)
	      /* set subcmd to MOUNT_MOVE to avoid endless loop */
	      do_move(ch->specials.mounted_by,
		      argument, cmd + 1, MOUNT_MOVE);
	    if (ch->followers) { /* If succes move followers */
	      for (k = ch->followers; k; k = next_dude) {
		next_dude = k->next;
		if ((was_in == k->follower->in_room) && 
		    (GET_POS(k->follower) >= POS_STANDING) &&
		    !PLR_FLAGGED(k->follower, PLR_WRITING | PLR_MAILING)) {
		  act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
		  do_move(k->follower, argument, cmd + 1, 0);
		}
		
	      }
	    }
	  } else if (ch->specials.mounted_by && subcmd == MOUNT_MOVE)
	    send_to_char("Your mount can't go there.\r\n", ch->specials.mounted_by);
	}
      }
    }
}

int	find_door(struct char_data *ch, char *type, char *dir)
{
   int	door;
   char	*dirs[] = 
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
   };

   if (*dir) /* a direction was specified */ {
      if ((door = search_block(dir, dirs, FALSE)) == -1) /* Partial Match */ {
	 send_to_char("That's not a direction.\r\n", ch);
	 return(-1);
      }

      if (EXIT(ch, door))
	 if (EXIT(ch, door)->keyword)
	    if (isname(type, EXIT(ch, door)->keyword))
	       return(door);
	    else {
	       sprintf(buf2, "I see no %s there.\r\n", type);
	       send_to_char(buf2, ch);
	       return(-1);
	    }
	 else
	    return(door);
      else {
	 send_to_char("I really don't see how you can close anything there.\r\n", ch);
	 return(-1);
      }
   } else /* try to locate the keyword */	 {
      for (door = 0; door < NUM_OF_DIRS; door++)
	 if (EXIT(ch, door))
	    if (EXIT(ch, door)->keyword)
	       if (isname(type, EXIT(ch, door)->keyword))
		  return(door);

      sprintf(buf2, "I see no %s here.\r\n", type);
      send_to_char(buf2, ch);
      return(-1);
   }
}


ACMD(do_open)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;

   two_arguments(argument, type, dir);

   if (!*type)
      send_to_char("Open what?\r\n", ch);
   else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			 ch, &victim, &obj)) {
     
     /* this is an object */
     
     if (obj->obj_flags.type_flag != ITEM_CONTAINER)
       send_to_char("That's not a container.\r\n", ch);
     else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
       send_to_char("But it's already open!\r\n", ch);
     else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
       send_to_char("You can't do that.\r\n", ch);
     else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
       send_to_char("It seems to be locked.\r\n", ch);
     else
       {
	 REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	 send_to_char("Ok.\r\n", ch);
	 act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
       }
   } else if ((door = find_door(ch, type, dir)) >= 0) {

      /* perhaps it is a door */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's impossible, I'm afraid.\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_BROKEN))
         send_to_char("You can't it's broken!\r\n", ch); 
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("It's already open!\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	 send_to_char("It seems to be locked.\r\n", ch);
      else {
	 REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	 if (EXIT(ch, door)->keyword)
	    act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword,
	        TO_ROOM);
	 else
	    act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
	 send_to_char("Ok.\r\n", ch);
	 /* now for opening the OTHER side of the door! */
	 if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ((back = world[other_room]->dir_option[rev_dir[door]]))
	       if (back->to_room == ch->in_room) {
		  REMOVE_BIT(back->exit_info, EX_CLOSED);
		  if (back->keyword) {
		     sprintf(buf, "The %s is opened from the other side.\r\n",
		         fname(back->keyword));
		     send_to_room(buf, EXIT(ch, door)->to_room);
		  } else
		     send_to_room("The door is opened from the other side.\r\n",
		         EXIT(ch, door)->to_room);
	       }
      }
   }
}


ACMD(do_close)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;


   two_arguments(argument, type, dir);

   if (!*type)
      send_to_char("Close what?\r\n", ch);
   else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			 ch, &victim, &obj)) {

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("But it's already closed!\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	 send_to_char("That's impossible.\r\n", ch);
      else {
	 SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	 send_to_char("Ok.\r\n", ch);
	 act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
   } else if ((door = find_door(ch, type, dir)) >= 0) {

      /* Or a door */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's absurd.\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_BROKEN))
         send_to_char("You can't it's broken!\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("It's already closed!\r\n", ch);
      else {
	 SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	 if (EXIT(ch, door)->keyword)
	    act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
	        TO_ROOM);
	 else
	    act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
	 send_to_char("Ok.\r\n", ch);
	 /* now for closing the other side, too */
	 if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ((back = world[other_room]->dir_option[rev_dir[door]]))
	       if (back->to_room == ch->in_room) {
		  SET_BIT(back->exit_info, EX_CLOSED);
		  if (back->keyword) {
		     sprintf(buf, "The %s closes quietly.\r\n", 
                             fname(back->keyword));
		     send_to_room(buf, EXIT(ch, door)->to_room);
		  } else
		     send_to_room("The door closes quietly.\r\n", EXIT(ch, door)->to_room);
	       }
      }
   }
}


int	has_key(struct char_data *ch, int key)
{
   struct obj_data *o;

   for (o = ch->carrying; o; o = o->next_content)
      if (obj_index[o->item_number].virtual == key)
	 return(1);

   if (ch->equipment[HOLD])
      if (obj_index[ch->equipment[HOLD]->item_number].virtual == key)
	 return(1);

   return(0);
}


ACMD(do_lock)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;


   two_arguments(argument, type, dir);

   if (!*type)
      send_to_char("Lock what?\r\n", ch);
   else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			 ch, &victim, &obj)) {

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("Maybe you should close it first...\r\n", ch);
      else if (obj->obj_flags.value[2] < 0)
	 send_to_char("That thing can't be locked.\r\n", ch);
      else if (!has_key(ch, obj->obj_flags.value[2]))
	 send_to_char("You don't seem to have the proper key.\r\n", ch);
      else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	 send_to_char("It is locked already.\r\n", ch);
      else {
	 SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	 send_to_char("*Cluck*\r\n", ch);
	 act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
      }
   } else if ((door = find_door(ch, type, dir)) >= 0) {

      /* a door, perhaps */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's absurd.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("You have to close it first, I'm afraid.\r\n", ch);
      else if (EXIT(ch, door)->key < 0)
	 send_to_char("There does not seem to be any keyholes.\r\n", ch);
      else if (!has_key(ch, EXIT(ch, door)->key))
	 send_to_char("You don't have the proper key.\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	 send_to_char("It's already locked!\r\n", ch);
      else {
	 SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	 if (EXIT(ch, door)->keyword)
	    act("$n locks the $F.", 0, ch, 0,  EXIT(ch, door)->keyword,
	        TO_ROOM);
	 else
	    act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	 send_to_char("*Click*\r\n", ch);
	 /* now for locking the other side, too */
	 if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ((back = world[other_room]->dir_option[rev_dir[door]]))
	       if (back->to_room == ch->in_room)
		  SET_BIT(back->exit_info, EX_LOCKED);
      }
   }
}

ACMD(do_unlock)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;
        
  two_arguments(argument, type, dir);
    
  if (!*type)
    send_to_char("Unlock what?\r\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    /* this is an object */
    if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER)
      send_to_char("That's not a container.\r\n", ch);
    else if (!IS_SET(GET_ITEM_VALUE(obj, 1), CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\r\n", ch);
    else if (GET_ITEM_VALUE(obj, 2) < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (!has_key(ch, GET_ITEM_VALUE(obj, 2)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (!IS_SET(GET_ITEM_VALUE(obj, 1), CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\r\n", ch);
    else {
      REMOVE_BIT(GET_ITEM_VALUE(obj, 1), CONT_LOCKED);
      send_to_char("*Click*\r\n", ch);
      act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    /* it is a door */
	
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("Heck.. it ain't even closed!\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't seem to spot any keyholes.\r\n", ch);
    else if (!has_key(ch, EXIT(ch, door)->key) && GET_LEVEL(ch) < LEVEL_IMMORT)
      send_to_char("You do not have the proper key for that.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already unlocked, it seems.\r\n", ch);
    else {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
	act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
	    TO_ROOM);
      else
	act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\r\n", ch);
	
      /* Key disappears after used to unlock door
      if(GET_LEVEL(ch) < LEVEL_IMMORT) {
	for (obj = ch->carrying; obj; obj = obj->next_content)
	  if (obj_index[obj->item_number].virtual == EXIT(ch, door)->key) break;
	  
	if (ch->equipment[HOLD])
	  if (obj_index[ch->equipment[HOLD]->item_number].virtual == EXIT(ch, door)->key){
	    obj=ch->equipment[HOLD];
	    perform_remove(ch, HOLD);
	  }
	obj_from_char(obj);
	extract_obj(obj);
      }*/
      
      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	if ((back = world[other_room]->dir_option[rev_dir[door]]))
	  if (back->to_room == ch->in_room)
	    REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
  }
}

ACMD(do_pick)
{
   byte percent;
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;

   two_arguments(argument, type, dir);

   percent = number(1, 101); /* 101% is a complete failure */

   if (percent > GET_SKILL(ch, SKILL_PICK_LOCK)) {
      send_to_char("You failed to pick the lock.\r\n", ch);
      return;
   }

   if (!*type)
      send_to_char("Pick what?\r\n", ch);
   else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			 ch, &victim, &obj)) {

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("Silly - it isn't even closed!\r\n", ch);
      else if (obj->obj_flags.value[2] < 0)
	 send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	 send_to_char("Oho! This thing is NOT locked!\r\n", ch);
      else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
	 send_to_char("It resists your attempts at picking it.\r\n", ch);
      else {
	 REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	 send_to_char("*Click*\r\n", ch);
	 act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
   } else if ((door = find_door(ch, type, dir)) >= 0) {
      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's absurd.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("You realize that the door is already open.\r\n", ch);
      else if (EXIT(ch, door)->key < 0)
	 send_to_char("You can't seem to spot any lock to pick.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	 send_to_char("Oh.. it wasn't locked at all.\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
	 send_to_char("You seem to be unable to pick this lock.\r\n", ch);
      else {
	 REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	 if (EXIT(ch, door)->keyword)
	    act("$n skillfully picks the lock of the $F.", 0, ch, 0,
	        EXIT(ch, door)->keyword, TO_ROOM);
	 else
	    act("$n picks the lock of the door.", TRUE, ch, 0, 0, TO_ROOM);
	 send_to_char("The lock quickly yields to your skills.\r\n", ch);
	 /* now for unlocking the other side, too */
	 if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ((back = world[other_room]->dir_option[rev_dir[door]]))
	       if (back->to_room == ch->in_room)
		  REMOVE_BIT(back->exit_info, EX_LOCKED);
      }
   }
}


ACMD(do_activate)
{
  struct obj_data *o, *portal;
  int found;

    one_argument(argument, buf);

  if (!*buf)
    send_to_char("Activate what???\r\n", ch);
  
  /* Check for Portals */
  found = 0;

  for (portal = world[ch->in_room]->contents; portal; portal = portal->next_content) {
    if (isname(buf, portal->name)) {
      found = 1;  /* name match */
      if ((GET_ITEM_TYPE(portal) == ITEM_PORTAL) && CAN_SEE_OBJ(ch, portal))
	{
	  if (IS_SET(GET_ITEM_VALUE(portal, 1), PORTAL_CLOSED)) {
	    for (o = ch->carrying; o; o = o->next_content)
	      if ((GET_LEVEL(ch) >= LEVEL_IMMORT) || (obj_index[o->item_number].virtual == GET_ITEM_VALUE(portal, 2))) {
		REMOVE_BIT(GET_ITEM_VALUE(portal, 1), PORTAL_CLOSED);
		act("You activate the $p.", TRUE, ch, portal, 0, TO_CHAR);
                act("$n activates the $p.", TRUE, ch, portal, 0, TO_ROOM);
		return;
	      }
	    send_to_char("You don't seem to have the proper item.\r\n", ch);
	    return;
	  } else {
	    send_to_char("It is already active.\r\n", ch);
	    return;
	  }
	}
    }
  }
  
  if (found == 1) 
    sprintf(buf2, "You can't seem to activate the %s...\r\n", buf);
  else
    sprintf(buf2, "There is no %s here.\r\n", buf);
  send_to_char(buf2, ch);
}



void enter_portal(struct char_data *ch, struct obj_data *portal)
{
  ACMD(do_look);
  int destination;

  /* 
     Portal Values
     [0] - destination
     [1] - portal info (closed, locked, ....)
     [2] - lock object, -1 = no locking 
     [3] - min level
     [4] - max level
     [5] - duration before portal closes, -1 = unlimited
  */

  if (IS_SET(GET_ITEM_VALUE(portal, 1), PORTAL_CLOSED)) {
    act("The $p appears to be closed.", TRUE, ch, portal, 0, TO_CHAR);
    return;
  }

  if (IS_SET(GET_ITEM_VALUE(portal, 1), PORTAL_LOCKED)) {
    act("The $p appears to be locked.", TRUE, ch, portal, 0, TO_CHAR);
    return;
  }

  if ((GET_LEVEL(ch) <= LEVEL_IMMORT) &&
      (GET_ITEM_VALUE(portal, 3) != -1) &&
      (GET_ITEM_VALUE(portal, 3) > GET_LEVEL(ch))) {
       act("You are too weak to enter the $p.", TRUE, ch, portal, 0, TO_CHAR);
       return;
  }

  if ((GET_LEVEL(ch) <= LEVEL_IMMORT) &&
      (GET_ITEM_VALUE(portal, 4) != -1) &&
      (GET_ITEM_VALUE(portal, 4) < GET_LEVEL(ch))) {
       act("You are too powerful to enter the $p.", TRUE, ch, portal, 0, TO_CHAR);
       return;
  }


  if ((destination = real_room(GET_ITEM_VALUE(portal, 0))) < 0)
    send_to_char("It appears to be broken...\r\n", ch);
  else if ((IS_SET(world[destination]->room_flags, GODROOM) 
	      && (GET_LEVEL(ch) < LEVEL_DEITY)) 
	    || (IS_SET(world[destination]->room_flags, PRIVATE)))
    {
      act("The $p repels your entry.", FALSE, ch, portal, 0, TO_CHAR);
      return;
    } else {
      act("You disappear into $a $p.", FALSE, ch, portal, 0, TO_CHAR);
      act("$n disappears into $a $p.", TRUE, ch, portal, 0, TO_ROOM);

     /* 
      if (GET_ITEM_VALUE(portal, 5) >= 1)
	   GET_ITEM_VALUE(portal, 5) -= 1;

      if (GET_ITEM_VALUE(portal, 5) == 0)
	   SET_BIT(GET_ITEM_VALUE(portal, 1), PORTAL_CLOSED);

      act("The $p closes behind $n.", FALSE, ch, portal, 0, TO_ROOM);
*/
      char_from_room(ch);
      char_to_room(ch, destination);
      act("$n appears from $a $p.", TRUE, ch, portal, 0, TO_ROOM);
      do_look(ch, "", 0, 0);

      roomflag_check(ch);
    }

  return;

}


ACMD(do_enter)
{
   struct obj_data *cont;

   ACMD(do_move);

   one_argument(argument, buf);

   if (*buf)  /* an argument was supplied, search for portal keyword */ {
     
     /* Check for Portals */
     for (cont = world[ch->in_room]->contents; cont; cont = cont->next_content)
       if (isname(buf, cont->name))
	 if ((GET_ITEM_TYPE(cont) == ITEM_PORTAL) && CAN_SEE_OBJ(ch, cont))
	   {
	     enter_portal(ch, cont);
	     return;
	   }
     
     sprintf(buf2, "There is no %s here that you can enter.\r\n", buf);
     send_to_char(buf2, ch);
     
   } else 
     if (IS_SET(world[ch->in_room]->room_flags, INDOORS))
       send_to_char("You are already indoors.\r\n", ch);
   
     else {
       
       send_to_char("You can't seem to find anything to enter.\r\n", ch);
     }
}


ACMD(do_leave)
{
   int	door;

   ACMD(do_move);

   if (!IS_SET(world[ch->in_room]->room_flags, INDOORS))
      send_to_char("You are outside.. where do you want to go?\r\n", ch);
   else {
      for (door = 0; door < NUM_OF_DIRS; door++)
	 if (EXIT(ch, door))
	    if (EXIT(ch, door)->to_room != NOWHERE)
	       if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && 
	           !IS_SET(world[EXIT(ch, door)->to_room]->room_flags, INDOORS)) {
		  do_move(ch, "", ++door, 0);
		  return;
	       }
      send_to_char("I see no obvious exits to the outside.\r\n", ch);
   }
}


ACMD(do_stand)
{
   switch (GET_POS(ch)) {
   case POS_STANDING :
      act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
      break;
   case POS_SITTING	:
      act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_STANDING;
      break;
   case POS_RESTING	:
      act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_STANDING;
      break;
   case POS_SLEEPING :
      act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
      break;
   case POS_FIGHTING :
      act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
      break;
   default :
      act("You stop floating around, and put your feet on the ground.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.",
          TRUE, ch, 0, 0, TO_ROOM);
      break;
   }
}


ACMD(do_sit)
{
   switch (GET_POS(ch)) {
   case POS_STANDING :
      act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
      break;
   case POS_SITTING	:
      send_to_char("You are sitting already.\r\n", ch);
      break;
   case POS_RESTING	:
      act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
      break;
   case POS_SLEEPING :
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
      break;
   case POS_FIGHTING :
      act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
      break;
   default :
      act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
      break;
   }
}




ACMD(do_rest)
{
   switch (GET_POS(ch)) {
   case POS_STANDING :
      act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_RESTING;
      break;
   case POS_SITTING :
      act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_RESTING;
      break;
   case POS_RESTING :
      act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
      break;
   case POS_SLEEPING :
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
      break;
   case POS_FIGHTING :
      act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
      break;
   default :
      act("You stop floating around, and stop to rest your tired bones.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
      break;
   }
}


ACMD(do_sleep)
{

  if (IS_SET(world[ch->in_room]->room_flags, NO_SLEEP)) {
    send_to_char("You don't feel safe enough to sleep in here...\r\n", ch);
    return;
  }

  switch (GET_POS(ch)) {
  case POS_STANDING :
  case POS_SITTING  :
  case POS_RESTING  :
    if (subcmd == 1) {
      if (GET_RACE(ch) == RACE_FELINE) {
	send_to_char("You curl into a tiny ball and go to sleep.", ch);
	act("$n curls $mself into a tiny ball and goes to sleep.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SLEEPING;
      } else {
	send_to_char("You aren't a feline!\r\n", ch);
	return ;
      } 
    } else {
      send_to_char("You go to sleep.\r\n", ch);
      act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SLEEPING;
    }
    break;
  case POS_SLEEPING :
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING :
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default :
    act("You stop floating around, and lie down to sleep.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
   struct char_data *tmp_char;

   one_argument(argument, arg);
   if (*arg) {
      if (GET_POS(ch) == POS_SLEEPING) {
	 act("You can't wake people up if you are asleep yourself!",
	     FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      } else {
	 tmp_char = get_char_room_vis(ch, arg);
	 if (tmp_char) {
	    if (tmp_char == ch) {
	       act("If you want to wake yourself up, just type 'wake'",
	           FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	       if (GET_POS(tmp_char) == POS_SLEEPING) {
		  if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
		     act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
		  } else {
		     act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
		     GET_POS(tmp_char) = POS_SITTING;
		     act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
		  }
	       } else {
		  act("$N is already awake.", FALSE, ch, 0, tmp_char, TO_CHAR);
	       }
	    }
	 } else {
	    send_to_char("You do not see that person here.\r\n", ch);
	 }
      }
   } else {
      if (IS_AFFECTED(ch, AFF_SLEEP)) {
	 send_to_char("You can't wake up!\r\n", ch);
      } else {
	 if (GET_POS(ch) > POS_SLEEPING)
	    send_to_char("You are already awake...\r\n", ch);
	 else {
	    send_to_char("You wake, and sit up.\r\n", ch);
	    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POS_SITTING;
	 }
      }
   }
}


ACMD(do_follow)
{
    struct char_data *leader;
    
    void	stop_follower(struct char_data *ch);
    void	add_follower(struct char_data *ch, struct char_data *leader);
    void return_all_in_hand(struct char_data *ch, struct char_data *victim);
    
    one_argument(argument, buf);
    
    if (*buf) {
	if (!str_cmp(buf, "self"))
	    leader = ch;
	else if (!(leader = get_char_room_vis(ch, buf))) {
	    send_to_char("I see no person by that name here!\r\n", ch);
	    return;
	}
    } else {
	send_to_char("Whom do you wish to follow?\r\n", ch);
	return;
    }
    
    if (ch->master == leader) {
	sprintf(buf, "You are already following %s.\r\n", HMHR(leader));
	send_to_char(buf, ch);
	return;
    }

    if (PLR_FLAGGED(leader, PLR_KILLER)) {
      send_to_char("You sure you want to follow a killer???\r\n", ch);
      return;
    }
    
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
	act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
    } else { /* Not Charmed follow person */
	if (leader == ch) {
	    if (!ch->master) {
		send_to_char("You are already following yourself.\r\n", ch);
		return;
	    }
	    stop_follower(ch);
	} else {
	    if (elite_follow(ch, leader)) {
		act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
		return;
	    }
	    
	    if (!allow_follower(leader)) {
	      act("$N can't have anymore followers.", FALSE, ch, 0, leader, TO_CHAR);
	      return;
	    }

	    if (ch->master) 
 		stop_follower(ch);

	    REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
	    
	    add_follower(ch, leader);
	}
    }
}


ACMD(do_hunt)
{
     struct obj_data *tobj;
     int z;

     if (GET_MOVE(ch) < 5) {
	  send_to_char("You are too exausted.\r\n", ch);
	  return;
     }
     if (number(1, 101) < GET_SKILL(ch, SKILL_HUNT)) {
	  switch(world[ch->in_room]->sector_type) {
	  case SECT_INSIDE   : send_to_char("You can't hunt indoors.\r\n", ch);
	                       return;
	  case SECT_CITY     : z = 3015; break; /* fido meat */
	  case SECT_FIELD    : z = 6023; break; /* rabbit */
	  case SECT_FOREST   : z = 6024; break; /* deer */
          case SECT_HILLS    : z = 6023; break; /* rabbit */
	  case SECT_MOUNTAIN : z = 6024; break; /* deer */
          case SECT_WATER_SWIM :      
          case SECT_WATER_NOSWIM : z = 6607; break; /* fish */   
	  default : z = 3015;
	  }

	  if (!(tobj = read_object(z, VIRTUAL))) {
	       send_to_char("I seem to have goofed.\r\n", ch);
	       sprintf(buf, "SYSERR: Hunt skill obj error: %d", z);
	       log(buf);
	       return;
	  }
          if (can_take_obj(ch,tobj)) {
  	    GET_MOVE(ch) -= 5;
	    obj_to_char(tobj, ch);
	    act("$n gets some $p.", FALSE, ch, tobj, 0, TO_ROOM);
	    act("You manage to get $p.", FALSE, ch, tobj, 0, TO_CHAR);
          }
          else {
            GET_MOVE(ch) -= 5;
            send_to_char("You try to hunt but are too overburdened!\r\n", ch);
          }
     }
}

ACMD(do_recall)
{
  ACMD(do_look);

  if (IS_SET(world[ch->in_room]->room_flags, GODROOM | ARENA)) {
    send_to_char("You can't recall from here!\r\n", ch);
    return;
  }

  if((GET_LEVEL(ch) <= max_recall_level) && (REMORT(ch)<1) && (!IS_NPC(ch))) {
    act("$n seeks deep into prayer.", FALSE, ch, 0, 0, TO_ROOM);
    act("The gods reach down and escort $n to safety!", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, r_mortal_start_room);
    if(GET_POS(ch) == POS_FIGHTING)
      GET_POS(ch) = POS_STANDING;
    send_to_char("You sink deep into prayer.\r\n", ch);
    send_to_char("The gods reach down and escort you to safety!\r\n", ch); 
    do_look(ch, "", 15, 0);
    act("The gods drop $n in the middle of the room!", FALSE, ch, 0, 0, TO_NOTVICT);
    if(GET_LEVEL(ch) == max_recall_level)
    send_to_char("Beware!  Soon the gods will not watch over you.  Buy recall scrolls!\r\n", ch);
  }
  else
  send_to_char("You have reached a level where the gods no longer protect you!\r\n", ch); 
}

ACMD(do_fly)
{
  struct affected_type af;
  
  if (IS_AFFECTED(ch, AFF_FLY)) {
    send_to_char("You are already flying.\r\n", ch);
    return;
  } else 

    if (GET_RACE(ch) == RACE_DRAGON ||
        GET_RACE(ch) == RACE_VAMPIRE ||
        GET_RACE(ch) == RACE_FAIRY ||
	GET_RACE(ch) == RACE_ANGEL) {
      
      switch(GET_RACE(ch)) {
	
      case RACE_DRAGON:
	if (GET_MOVE(ch) < 75) {
	  send_to_char("You are too exausted to take off.\r\n", ch);
	  return;
	}

	GET_MOVE(ch) -= 50;
	act("$n spreads $s mighty wings and lifts off the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
	break;
      case RACE_FAIRY:
	if (GET_MOVE(ch) < 35) {
	  send_to_char("You are too exausted to take off.\r\n", ch);
	  return;
	}

	GET_MOVE(ch) -= 15;
	act("You hear a buzzing sound as $n flutters $s wings and takes off.\r\n", FALSE, ch, 0, 0, TO_ROOM); 
	break;
      case RACE_VAMPIRE:
	if (GET_MOVE(ch) < 50) {
	  send_to_char("You are too exausted to take off.\r\n", ch);
	  return;
	}

	GET_MOVE(ch) -= 25;
	act("$n bears $s fangs and silently rises above the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
	break;
      case RACE_ANGEL:
	if (GET_MOVE(ch) < 50) {
	  send_to_char("You are too exausted to take off.\r\n", ch);
	  return;
	}
	GET_MOVE(ch) -= 25;
	act("$n unfurls $s wings and takes to the skies above.\r\n", FALSE, ch, 0, 0, TO_ROOM);
	break;
	
      default:
	return;
      }
      
      af.type = SPELL_FLY;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.duration = DURATION_INNATE;
      af.bitvector = AFF_FLY;
      affect_to_char(ch, &af);
      
      send_to_char("You take flight.\r\n", ch);
      
     
  } else {
    send_to_char("You flap your arms and attempt to take flight.\r\n", ch);
    act("$n flaps $s arms looking quite silly.\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  
  return;
}

ACMD(do_land)
{
  if (IS_AFFECTED(ch, AFF_FLY)) {

    send_to_char("You land on the ground below.\r\n", ch);
    affect_from_char(ch, SPELL_FLY);

    switch(GET_RACE(ch)) {

      case RACE_DRAGON:
	act("$n glides to the ground and furls $s giant wings.\r\n", FALSE, ch, 0, 0, TO_ROOM);
	break;
      case RACE_FAIRY:
	act("$n stops fluttering $s wings and lands.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        break;
      case RACE_VAMPIRE:
        act("$n quietly floats to the ground before you.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        break;
      case RACE_ANGEL:
        act("$n majestically descends to the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
	break;

      default:
        act("$n stops flying and lands on the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        break;
        return;
    }
  } else {
    send_to_char("You already seem to be on the ground.\r\n", ch);
    return;
  }

  return;
}

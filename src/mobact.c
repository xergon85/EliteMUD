/* ************************************************************************
*   File: mobact.c                                      Part of EliteMUD  *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
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
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "functions.h"

/* external structs */
extern struct char_data *character_list;
extern struct char_data **mob_list;
extern struct index_data *mob_index;
extern struct room_data **world;
extern struct str_app_type str_app[];

extern int top_of_world;

void
parse_add_mob_to_errorbuf(struct char_data *mob, char *str);


/* get a cmd-line from string, ended with either ';' or NULL */
char * get_command_line(char *from, char *to)
{
    int begin = 0;
    char *to_ptr, *from_ptr;
    
    for ( ; isspace(*(from + begin)); begin++);

    to_ptr = to;
    from_ptr = from + begin;
    
    while (*from_ptr != '\0') {
	if (*from_ptr == ';') {
	  from_ptr++;
	  break;
	} else if (*from_ptr == '\n' || *from_ptr == '\r') {
	  from_ptr++;
	} else {
	  *to_ptr = *from_ptr;
	  to_ptr++;
	  from_ptr++;
	} 
      }
    
    *to_ptr = '\0';
    return from_ptr;
  }

void  get_next_mob_command(struct char_data *mob, char *cmd)
{
  while (*mob->nextact == '\n' || *mob->nextact == '\r' || 
	 *mob->nextact == ' ')
    mob->nextact++;
  if (!*mob->nextact)
    mob->nextact = mob->mobaction;
		
  mob->nextact = get_command_line(mob->nextact, cmd);
}

void exec_mob_command(struct char_data *mob, char *cmd, long flags)
{
  int percent, i;
    
  switch(*cmd) {
	
  case '#':
    /* special mob command interpretor */
    break;
  case '%':
    percent = atoi((cmd + 1));
    if (percent > number(1, 100)) {
      i = 1;
      while ((*(cmd + i) >= '0' && *(cmd + i) <= '9') || 
	     *(cmd + i) == ' ')
	i++;
      exec_mob_command(mob, (cmd + i), flags);
    }
    break;
  default:
    command_interpreter(mob, cmd);
    break;
  }
  return;
}


void	mobile_activity(void)
{
  register struct char_data *ch;
  struct char_data *tmp_ch, *vict;
  struct obj_data *obj, *best_obj;
  int	door, found, max;
  memory_rec * names;
  static char cmd[MAX_INPUT_LENGTH];
  
  extern int	no_specials;
  
  ACMD(do_move);
  ACMD(do_get);
  ACMD(do_stand);
  ACMD(do_wake);
  ACMD(do_follow);
  
  for (ch = *mob_list; ch; ch = ch->next)
    if (IS_MOB(ch)) {
      if (IN_ROOM(ch) == NOWHERE) continue;
      if (IS_SET(ch->specials2.act, MOB_SWITCHED)) continue;
      /* Check for action string NEW -Petrus */
      if (ch->mobaction) {
	get_next_mob_command(ch, cmd);
	exec_mob_command(ch, cmd, 0);
      }
      
      /* Examine call for special procedure */
      if (IS_SET(ch->specials2.act, MOB_SPEC) && !no_specials) {
	if (!mob_index[ch->nr].func) {
	  sprintf(buf, "%s (#%d): Attempting to call non-existing mob func",
		  GET_NAME(ch), (int)mob_index[ch->nr].virtual);
	  log(buf);
	  REMOVE_BIT(ch->specials2.act, MOB_SPEC);
	} else {
	  if ((*mob_index[ch->nr].func)(ch, ch, NULL, 0, ""))
	    continue;		/* go to next char */
	}
      }
      
      if(!is_empty(world[ch->in_room]->zone))
	mprog_random_trigger(ch);
      
      if (AWAKE(ch) && !(ch->specials.fighting)) {
	if (IS_SET(ch->specials2.act, MOB_SCAVENGER)) { /* if scavenger */
	  if (world[ch->in_room]->contents && !number(0, 10)) {
	    for (max = 1, best_obj = 0, obj = world[ch->in_room]->contents; 
		 obj; obj = obj->next_content) {
	      if (MOB_CAN_GET_OBJ(ch, obj)) {
		if (obj->obj_flags.cost > max) {
		  best_obj = obj;
		  max = obj->obj_flags.cost;
		}
	      }
	    } /* for */
			
	    if (best_obj) {
	      obj_from_room(best_obj);
	      obj_to_char(best_obj, ch);
	      act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
	    }
	  }
	} /* Scavenger */
	

	if (ch->trackdir && !FIGHTING(ch)) {
	  if (!AWAKE(ch))
	    do_wake(ch, "", 0, 0);
	  if (GET_POS(ch) < POS_STANDING)
	    do_stand(ch, "", 0, 0);
	  if ((ch->trackdir->room == IN_ROOM(ch)) && !IS_SET(world[EXIT(ch, (int)(ch->trackdir->dir))->to_room]->room_flags, NO_MOB)) {
	    do_move(ch, "", stack_pop(&ch->trackdir) + 1, 0);
	  } else {
	    free_stack(ch->trackdir);
	    ch->trackdir = NULL;
	  }
	} else if (!IS_SET(ch->specials2.act, MOB_SENTINEL) && 
		   (GET_POS(ch) == POS_STANDING) && 
		   ((door = number(0, 45)) < NUM_OF_DIRS) && CAN_GO(ch, door) && 
		   !IS_SET(world[EXIT(ch, door)->to_room]->room_flags, NO_MOB) && 
		   !IS_SET(world[EXIT(ch, door)->to_room]->room_flags, DEATH)) {
	  if (ch->mob_specials.last_direction == door) {
	    ch->mob_specials.last_direction = -1;
	  } else {
	    if (!IS_SET(ch->specials2.act, MOB_STAY_ZONE)) {
	      ch->mob_specials.last_direction = door;
	      do_move(ch, "", ++door, 0);
	    } else {
	      if (world[EXIT(ch, door)->to_room]->zone == world[ch->in_room]->zone) {
		ch->mob_specials.last_direction = door;
		do_move(ch, "", ++door, 0);
	      }
	    }
	  }
	} else if (IS_SET(ch->specials2.act, MOB_SENTINEL) &&
		   ch->player.hometown != IN_ROOM(ch) &&
		   !FIGHTING(ch)) {
	  perform_track(ch, ch->player.hometown, 100);
	}
	
	/* MOB Prog foo */
	if(IS_NPC(ch) && ch->mpactnum > 0 && !is_empty(world[ch->in_room]->zone)) {
	  MPROG_ACT_LIST *tmp_act, *tmp2_act;
	  for(tmp_act = ch->mpact; tmp_act != NULL; tmp_act=tmp_act->next) {
	    parse_add_mob_to_errorbuf(ch, "ActTrigger");
	    mprog_wordlist_check(tmp_act->buf, ch, tmp_act->ch,
				 tmp_act->obj, tmp_act->vo, ACT_PROG);
	    free(tmp_act->buf);
	  }
	  for(tmp_act = ch->mpact; tmp_act != NULL; tmp_act=tmp2_act) {
	    tmp2_act = tmp_act->next;
	    free(tmp_act);
	  }
	  ch->mpactnum = 0;
	  ch->mpact = NULL;
	} /* IF MOB PROG */

	if (IS_SET(ch->specials2.act, MOB_AGGRESSIVE)) {
	  found = FALSE;
         if (IN_ROOM(ch) < 0 || IN_ROOM(ch) > top_of_world)
          log("SYSERR: Calling AGGRESSIVE Mob Act with mob not in a valid room");
         else
	  for (tmp_ch = world[ch->in_room]->people; tmp_ch && !found; 
	       tmp_ch = tmp_ch->next_in_room) {
	    if (!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) && !PRF_FLAGGED(tmp_ch, PRF_NOHASSLE)) {
	      if (!IS_SET(ch->specials2.act, MOB_WIMPY) || !AWAKE(tmp_ch)) {
		if ((IS_SET(ch->specials2.act, MOB_AGGRESSIVE_EVIL) && 
		     IS_EVIL(tmp_ch)) || 
		    (IS_SET(ch->specials2.act, MOB_AGGRESSIVE_GOOD) && 
		     IS_GOOD(tmp_ch)) || 
		    (IS_SET(ch->specials2.act, MOB_AGGRESSIVE_NEUTRAL) && 
		     IS_NEUTRAL(tmp_ch)) || 
		    (!IS_SET(ch->specials2.act, MOB_AGGRESSIVE_EVIL) && 
		     !IS_SET(ch->specials2.act, MOB_AGGRESSIVE_NEUTRAL) && 
		     !IS_SET(ch->specials2.act, MOB_AGGRESSIVE_GOOD) ) ) {
		  hit(ch, tmp_ch, 0);
		  found = TRUE;
		}
	      }
	    }
	  }
	} /* if aggressive */
	
	if (IS_SET(ch->specials2.act, MOB_MEMORY) && ch->mob_specials.memory) {
	  for (vict = 0, tmp_ch = character_list, max = 1; 
	       tmp_ch && !IS_NPC(tmp_ch);
	       tmp_ch = tmp_ch->next, max++)
	    if (CAN_SEE(ch, tmp_ch)) {
	      for (names = ch->mob_specials.memory; names; names = names->next)
		if (names->id == GET_IDNUM(tmp_ch))
		  vict = tmp_ch;
	      if (vict && IN_ROOM(vict) == IN_ROOM(ch)) {
		if (!IS_SET(world[ch->in_room]->room_flags, LAWFULL)) {
		  act("'Hey!  You're the fiend that attacked me!!!', exclaims $n.", FALSE, ch, 0, 0, TO_ROOM);
		  hit(ch, vict, 0);
		  return;
		} else {
		  do_follow(ch, GET_NAME(vict), 0, 0);
		}
	      }
	    }
	  
	  if (vict && IN_ROOM(vict) != IN_ROOM(ch) &&
	      !IS_SET(ch->specials2.act, MOB_SENTINEL) &&
	      GET_HIT(ch) + GET_LEVEL(ch) > GET_MAX_HIT(ch)) {
	    annoy_hunted_victim(ch, vict, max);
	    perform_track(ch, IN_ROOM(vict), GET_LEVEL(ch));
	  }
	  
	} /* mob memory */
      }
    } /* If IS_MOB(ch)  */
}



/* Mob Memory Routines */

/* make ch remember victim */
void	remember (struct char_data *ch, struct char_data *victim)
{
   memory_rec * tmp;
   bool present = FALSE;

   if (!IS_NPC(ch) || IS_NPC(victim)) 
      return;

   for (tmp = ch->mob_specials.memory; tmp && !present; tmp = tmp->next)
      if (tmp->id == GET_IDNUM(victim))
	 present = TRUE;

   if (!present) {
      CREATE(tmp, memory_rec, 1);
      tmp->next = ch->mob_specials.memory;
      tmp->id = GET_IDNUM(victim);
      ch->mob_specials.memory = tmp;
   }
}


/* make ch forget victim */
void	forget (struct char_data *ch, struct char_data *victim)
{
   memory_rec *curr, *prev = NULL;

   if (!(curr = ch->mob_specials.memory))
      return;

   while (curr && curr->id != GET_IDNUM(victim)) {
      prev = curr;
      curr = curr->next;
   }

   if (!curr) 
      return; /* person wasn't there at all. */

   if (curr == ch->mob_specials.memory)
      ch->mob_specials.memory = curr->next;
   else
      prev->next = curr->next;

   free(curr);
}


/* erase ch's memory */
void	clearMemory(struct char_data *ch)
{
   memory_rec *curr, *next;

   curr = ch->mob_specials.memory;

   while (curr) {
      next = curr->next;
      free(curr);
      curr = next;
   }

   ch->mob_specials.memory = NULL;
}


void annoy_hunted_victim(struct char_data *ch, struct char_data *vict, int chance)
{
  char text[MAX_INPUT_LENGTH];

  ACMD(do_gen_com);

  if (!ch || !vict)
    return;

  if (number(0,1 + chance/10))
    return;

  *text = '\0'; 
  
  switch (number(0,5)) {
  case 0:
    sprintf(text, "Where are you, little %s?", GET_NAME(vict));
    break;
  case 1:
    sprintf(text, "Come out from hiding, %s, you little weasel.", GET_NAME(vict));
    break;
  case 2:
    sprintf(text, "You cannot run forever, %s.", GET_NAME(vict));
    break;
  case 3:
    sprintf(text, "Come out here and play, %s.  You'll enjoy it.", GET_NAME(vict));
    break;
  case 4:
    sprintf(text, "Does it feel good to be hunted down like a pig, %s?", GET_NAME(vict));
    break;
  default:
    sprintf(text, "I am coming for you, %s.", GET_NAME(vict));
    break;
  }
  
  if (*text)
    do_gen_com(ch, text, 18, SCMD_YELL);
}

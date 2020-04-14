/* ****************************************************************************
*  File: ptriggers.c                                        Part of EliteMUD  *
*  Usage: Triggers and utils for program parser                               *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) Mr Wang 1995 EliteMud RIT                                    *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
*  This file is based on MOBPROGS by Merc Industries - with thanks -P         *
**************************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "parser.h"
#include "functions.h"

extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data **world;

void mprog_driver(char *com_list, struct char_data *mob, struct char_data *actor, struct obj_data *obj, void *vo);
bool str_prefix(const char *astr, const char *bstr);


/* we need str_infix here because strstr is not case insensitive */
bool
str_infix(const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;
  
  if ((c0 = LOWER(astr[0])) == '\0')
    return FALSE;
  
  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  
  for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
    if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
      return FALSE;
  }

  return TRUE;
}


/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void
mprog_wordlist_check(char *arg, struct char_data *mob, struct char_data *actor, struct obj_data *obj, void *vo, int type)
{
  char        temp1[MAX_STRING_LENGTH];
  char        temp2[MAX_INPUT_LENGTH];
  char        word[MAX_INPUT_LENGTH];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  int         i;

  for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
    if (mprg->type & type) {
      strcpy(temp1, mprg->arglist);
      list = temp1;
      while(isspace(*list)) list++;
      for (i = 0; i < strlen(list); i++)
	list[i] = LOWER(list[i]);
      strcpy(temp2, arg);
      dupl = temp2;
      for (i = 0; i < strlen(dupl); i++)
	dupl[i] = LOWER(dupl[i]);
	if ((list[0] == 'p') && (list[1] == ' ')) {
	  list += 2;
	  while ((start = strstr(dupl, list)))
	    if ((start == dupl || *(start-1) == ' ')
		&& (*(end = start + strlen(list)) == ' '
		    || *end == '\n'
		    || *end == '\r'
		    || *end == '\0'))
	      {
		mprog_driver(mprg->comlist, mob, actor, obj, vo);
		break;
	      } else
		dupl = start+1;
	} else {
	  list = one_argument(list, word);
	  for(; word[0] != '\0'; list = one_argument(list, word))
	    while ((start = strstr(dupl, word)))
	      if ((start == dupl || *(start-1) == ' ')
		  && (*(end = start + strlen(word)) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0'))
		{
		  mprog_driver(mprg->comlist, mob, actor, obj, vo);
		  break;
		} else
		  dupl = start+1;
	}
    }
  return;
}


void mprog_percent_check(struct char_data *mob, struct char_data *actor, struct obj_data *obj, void *vo, int type)
{
  MPROG_DATA * mprg;
  
  for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
    if ((mprg->type & type)
	&& (number(0,100) < atoi(mprg->arglist)))
      {
	mprog_driver(mprg->comlist, mob, actor, obj, vo);
	if (type != GREET_PROG && type != ALL_GREET_PROG)
	  break;
      }
  
  return;
}


/* Function to add a string to the parse_errorbuf, in case
 * the program doesn't it's good to know where and what and who
 */
void
parse_add_mob_to_errorbuf(struct char_data *mob, char *str)
{
  strcpy(parse_errorbuf, "(SYSERR): ");

  if (str && *str)
    strcat(parse_errorbuf, str);

  sprintf(parse_errorbuf, "%s:(%d) %s: ",
	  parse_errorbuf, mob_index[mob->nr].virtual,
	  GET_NAME(mob));
}


/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
/* I don't think this is quite bugfree ... -P */
void mprog_act_trigger(char *buf, struct char_data *mob, struct char_data *ch,
		       struct obj_data *obj, void *vo)
{
  MPROG_ACT_LIST * tmp_act;
  
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & ACT_PROG)
      && (mob != ch))
    {
      tmp_act = malloc(sizeof(MPROG_ACT_LIST));
      if (mob->mpactnum > 0)
	tmp_act->next = mob->mpact;
      else
	tmp_act->next = NULL;
      
      mob->mpact      = tmp_act;
      mob->mpact->buf = strdup(buf);
      mob->mpact->ch  = ch; 
      mob->mpact->obj = obj; 
      mob->mpact->vo  = vo; 
      mob->mpactnum++;
      
    }
  return;
}

void mprog_bribe_trigger(struct char_data *mob, struct char_data *ch, int amount)
{

  MPROG_DATA *mprg;
  struct obj_data   *obj;

  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & BRIBE_PROG))
    {
      obj = create_money(amount);
      obj_to_char(obj, mob);
      mob->points.gold -= amount;
      
      for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
	if ((mprg->type & BRIBE_PROG)
	    && (amount >= atoi(mprg->arglist)))
	  {
	    parse_add_mob_to_errorbuf(mob, "BribeTrigger");
	    mprog_driver(mprg->comlist, mob, ch, obj, NULL);
	    break;
	  }
    }
  
  return;
}

void mprog_death_trigger(struct char_data *mob, struct char_data *killer)
{

 if (IS_NPC(mob)
     && (mob_index[mob->nr].progtypes & DEATH_PROG))
   {
     parse_add_mob_to_errorbuf(mob, "DeathTrigger");
     mprog_percent_check(mob, killer, NULL, NULL, DEATH_PROG);
   }

 death_cry(mob);
 return;

}

void mprog_entry_trigger(struct char_data *mob)
{

 if (IS_NPC(mob)
     && (mob_index[mob->nr].progtypes & ENTRY_PROG)) {
   parse_add_mob_to_errorbuf(mob, "EntryTrigger");
   mprog_percent_check(mob, NULL, NULL, NULL, ENTRY_PROG);
 }
 
 return;

}

void mprog_fight_trigger(struct char_data *mob, struct char_data *ch)
{

 if (IS_NPC(mob)
     && (mob_index[mob->nr].progtypes & FIGHT_PROG)) {
   parse_add_mob_to_errorbuf(mob, "FightTrigger");
   mprog_percent_check(mob, ch, NULL, NULL, FIGHT_PROG);
 }
 
 return;

}

void mprog_give_trigger(struct char_data *mob, struct char_data *ch, struct obj_data *obj)
{
  char        buf[MAX_INPUT_LENGTH];
  MPROG_DATA *mprg;
  int virtual = (obj->item_number>=0)?obj_index[obj->item_number].virtual:-1;
  
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & GIVE_PROG))
    for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next) {
      one_argument(mprg->arglist, buf);
      if ((mprg->type & GIVE_PROG) &&
	  ((virtual == atoi(mprg->arglist) || 
	   !str_infix(obj->name, mprg->arglist)) ||
	  (!str_cmp("all", buf)))) {
	parse_add_mob_to_errorbuf(mob, "GiveTrigger");
	mprog_driver(mprg->comlist, mob, ch, obj, NULL);
	break;
      }
    }
  return;
}

void mprog_greet_trigger(struct char_data *ch)
{

 struct char_data *vmob;

 for (vmob = world[ch->in_room]->people; vmob != NULL; vmob = vmob->next_in_room)
   if (IS_NPC(vmob)
       && ch != vmob
       && CAN_SEE(vmob, ch)
       && (vmob->specials.fighting == NULL)
       && AWAKE(vmob)
       && (mob_index[vmob->nr].progtypes & GREET_PROG)) {
     parse_add_mob_to_errorbuf(vmob, "GreetTrigger");
     mprog_percent_check(vmob, ch, NULL, NULL, GREET_PROG);
   } else
     if (IS_NPC(vmob)
	 && (vmob->specials.fighting == NULL)
	 && AWAKE(vmob)
	 && (mob_index[vmob->nr].progtypes & ALL_GREET_PROG)) {
       parse_add_mob_to_errorbuf(vmob, "GreetAllTrigger");
       mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
     }
 
 return;

}

void mprog_hitprcnt_trigger(struct char_data *mob, struct char_data *ch)
{

 MPROG_DATA *mprg;

 if (IS_NPC(mob)
     && (mob_index[mob->nr].progtypes & HITPRCNT_PROG))
   for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
     if ((mprg->type & HITPRCNT_PROG)
	 && ((100*mob->points.hit / mob->points.max_hit) < atoi(mprg->arglist)))
       {
	 parse_add_mob_to_errorbuf(mob, "HitPrcntTrigger");
	 mprog_driver(mprg->comlist, mob, ch, NULL, NULL);
	 break;
       }
 
 return;

}

void mprog_random_trigger(struct char_data *mob)
{
  if (mob_index[mob->nr].progtypes & RAND_PROG) {
    parse_add_mob_to_errorbuf(mob, "RandomTrigger");
    mprog_percent_check(mob,NULL,NULL,NULL,RAND_PROG);
  }
  
  return;

}

void mprog_speech_trigger(char *txt, struct char_data *mob)
{

  struct char_data *vmob;

  for (vmob = world[mob->in_room]->people; vmob != NULL; vmob = vmob->next_in_room)
    if ((mob != vmob) && IS_NPC(vmob) && (mob_index[vmob->nr].progtypes & SPEECH_PROG)) {
      parse_add_mob_to_errorbuf(vmob, "SpeechTrigger");
      mprog_wordlist_check(txt, vmob, mob, NULL, NULL, SPEECH_PROG);
    }
  
  return;
}

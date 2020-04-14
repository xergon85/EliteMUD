/* ************************************************************************
*  File: history.c (Version 1.0)                        Part of EliteMUD  *
*  Usage: Channel History Procedures                                      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  (C) 1998 Petya Vachranukunkiet                                         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "history.h"
#include "interpreter.h"

extern struct descriptor_data *descriptor_list;
extern struct room_data **world;

/* for global history channels */
struct history_list *global_history[CHAN_GLOBAL_MAX];

void log(char *str);
void send_to_char(char *messg, struct char_data *ch);
char *delete_doubledollar(char *string);


void clear_history(struct char_data *ch)
{
     int i;

     if (IS_NPC(ch)) return;

     for (i = CHAN_GLOBAL_MAX; i < CHAN_MAX; i++) {
	  free_history(&GET_HISTORY(ch, i));
	  GET_HISTORY(ch, i) = NULL;
     }

     return;
}


/* Frees history and all subsequent histories if any */
void free_history(struct history_list **history)
{
     struct history_list *tmp, *next;

     for (tmp = *history; tmp; tmp = next) {
	  next = tmp->next;
	  if (tmp->text) free(tmp->text);
	  if (tmp->name) free(tmp->name);
	  free(tmp);
     }

     return;
}


/* Add message, and data to history, cap if necessary */
void add_history(char *message, struct char_data *ch, struct history_list **history)
{
     struct history_list *new, *tmp;
     int depth = 1;

     if (!ch) {
	  log("SYSERR: No target ch for add_history.");
	  return;
     }

     CREATE(new, struct history_list, 1);
     new->text = strdup(message);
     delete_doubledollar(new->text);
     new->name = strdup(GET_NAME(ch));
     new->invis_level = GET_INVIS_LEV(ch);
     new->next = NULL;

     if (*history) {
	  for (tmp = *history; tmp->next; depth++)
               tmp = tmp->next;
          tmp->next = new;
     } else {
	  *history = new;
     }

     if (depth >= MAX_HISTORY_DEPTH) {
	  tmp = *history;
	  *history = tmp->next;
	  if (tmp->text) free(tmp->text);
	  if (tmp->name) free(tmp->name);
	  free(tmp);
     }

}

/* Gives each character the appropriate history */
void chan_history(char *message, struct char_data *from, struct char_data *to, int level, int channel)
{
     struct char_data *k;
     struct follow_type *f;
     struct descriptor_data *d;
     char buf[MAX_STRING_LENGTH];

     switch (channel) {
     case CHAN_NONE :
          return;
/* global channels */
     case CHAN_GOSSIP :
     case CHAN_CHAT :
     case CHAN_NEWBIE :
     case CHAN_AUCTION :
     case CHAN_PKSAY :
     case CHAN_QUEST :
	  add_history(message, from, &global_history[channel]);
	  break;
/* local channels */
     case CHAN_SAY :
	  for (k = world[from->in_room]->people; k; k = k->next_in_room)
	       if (!IS_NPC(k))
		    add_history(message, from, &GET_HISTORY(k, CHAN_SAY));
	  break;

     case CHAN_TELL :
	  if(!IS_NPC(from) && !IS_NPC(to)) {
	       sprintf(buf, "-> #w[#C%s#w]#N %s", GET_NAME(to), message);
	       add_history(buf, from, &GET_HISTORY(from, CHAN_TELL));
	       add_history(buf, from, &GET_HISTORY(to, CHAN_TELL));
	  }
          break;

     case CHAN_GROUP :
	  if (from->master != NULL)
	       k = from->master;
	  else
	       k = from;

	  if (!IS_NPC(k)) {
	       add_history(message, from, &GET_HISTORY(k, CHAN_GROUP));
	  }

	  for (f = k->followers; f; f = f->next) {
	       if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower))
		    add_history(message, from, &GET_HISTORY(f->follower, CHAN_GROUP));
	  }
	  break;

     case CHAN_WIZLINE :
	  sprintf(buf, "#w<#C%d#w>#N %s", level, message);
	  for (d = descriptor_list; d; d = d->next)
	       if (d->character && STATE(d) == CON_PLYNG)
		    if (!IS_NPC(d->character) &&
			GET_LEVEL(d->character) >= level)
			 add_history(buf, from, &GET_HISTORY(d->character, CHAN_WIZLINE));

	  break;

     case CHAN_CLAN :
	  sprintf(buf, "#w<#C%d#w>#N %s", level, message);
	    for (d = descriptor_list; d ; d = d->next)
		 if (d->character && STATE(d) == CON_PLYNG)
		      if (!IS_NPC(d->character) &&
			  (CLAN(d->character) == CLAN(from)) &&
			  (CLAN_LEVEL(d->character) >= level))
			  add_history(buf, from, &GET_HISTORY(d->character, CHAN_CLAN));

	  break;

     case CHAN_WHISPER :
	  if(!IS_NPC(from) && !IS_NPC(to)) {
	       sprintf(buf, "-> #w[#C%s#w]#N %s", GET_NAME(to), message);
	       add_history(buf, from, &GET_HISTORY(from, CHAN_WHISPER));
	       add_history(buf, from, &GET_HISTORY(to, CHAN_WHISPER));
	  }
	  break;

     case CHAN_YELL :
          for (d = descriptor_list; d ; d = d->next)
            if (d->character && STATE(d) == CON_PLYNG)
              if (!IS_NPC(d->character) &&
                  (world[d->character->in_room]->zone == world[from->in_room]->zone))
                  add_history(message, from, &GET_HISTORY(d->character, CHAN_YELL));
          break;

     default :
	  log("SYSERR: chan_history, non-exisitent channel");
	  return;
	  break;
     }

     return;
}


/* Returns char history */
void print_history(struct char_data *ch, int channel)
{
     char buf[MAX_STRING_LENGTH];
     struct history_list *tmp, *history;

     if (IS_NPC(ch)) {
	  send_to_char("Mobs don't get a history!\r\n", ch);
	  return;
     }

     switch(channel) {
     case CHAN_NONE : return;
     case CHAN_GOSSIP : /* Gossip 0 */
	  sprintf(buf, "#YGossip History#N\r\n");
	  history = global_history[channel];
	  break;
     case CHAN_CHAT : /* Chat 1 */
	  sprintf(buf, "#GChat History#N\r\n");
	  history = global_history[channel];
	  break;
     case CHAN_NEWBIE : /* Newbie 2 */
	  sprintf(buf, "#wNewbie History#N\r\n");
	  history = global_history[channel];
	  break;
     case CHAN_AUCTION : /* Auction  3 */
	  sprintf(buf, "#MAuction History#N\r\n");
	  history = global_history[channel];
	  break;
	 case CHAN_PKSAY : /* PKOK channel 4*/
	  sprintf(buf, "#rPK Say History#N\r\n");
	  history = global_history[channel];
      break;
     case CHAN_QUEST : /* Quest Say 5 */
	  sprintf(buf, "#MQuest-say History#N\r\n");
	  history = global_history[channel];
	  break;
     case CHAN_SAY : /* Say 6 */
	  sprintf(buf, "#ySay History#N\r\n");
	  history = GET_HISTORY(ch, channel);
	  break;
     case CHAN_TELL : /* Tell 7 */
	  sprintf(buf, "#bTell History#N\r\n");
	  history = GET_HISTORY(ch, channel);
	  break;
     case CHAN_GROUP : /* Group Tell 8 */
	  sprintf(buf, "#gGroup Tell History#N\r\n");
	  history = GET_HISTORY(ch, channel);
	  break;
     case CHAN_WIZLINE : /* Wizline 9 */
	  sprintf(buf, "#CWizline History#N\r\n");
	  history = GET_HISTORY(ch, channel);
	  break;
     case CHAN_CLAN : /* Clantell 10 */
	  sprintf(buf, "#mClan Tell History#N\r\n");
	  history = GET_HISTORY(ch, channel);
	  break;
     case CHAN_WHISPER : /* Whisper 11 */
      sprintf(buf, "#cWhisper History#N\r\n");
	  history = GET_HISTORY(ch, channel);
	  break;
     case CHAN_YELL : /* Yell 12 */
      sprintf(buf, "#cYell History#N\r\n");
      history = GET_HISTORY(ch, channel);
      break;
     default:
	  log("SYSERR: return_history, Channel non-existent");
	  return;
     }
     send_to_char(buf, ch);

     for (tmp = history; tmp; tmp = tmp->next) {
	  sprintf(buf, "#w[#C%s#w]#N %s\r\n", (tmp->invis_level <= GET_LEVEL(ch)) ? tmp->name : "Someone", tmp->text);
	  send_to_char(buf, ch);
     }

     return;
}


/* ************************************************************************
*  File: history.h                                      Part of EliteMUD  *
*  Usage: header file for Channel History                                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  (C) 1998 Petya Vachranukunkiet                                         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
************************************************************************ */

struct history_list {
     char *text;
     char *name;
     int invis_level;
     struct history_list *next;
};

/* Max depth of linked list */
#define MAX_HISTORY_DEPTH 10

/* Channel defines */
#define CHAN_NONE         -1
#define CHAN_GOSSIP        0
#define CHAN_CHAT          1
#define CHAN_NEWBIE        2
#define CHAN_AUCTION       3
#define CHAN_PKSAY         4
#define CHAN_QUEST         5
/* Be sure to match the global channels to subcmd in interpreter.h */

#define CHAN_SAY           6
#define CHAN_TELL          7
#define CHAN_GROUP         8
#define CHAN_WIZLINE       9
#define CHAN_CLAN          10
#define CHAN_WHISPER       11
#define CHAN_YELL          12
#define CHAN_MAX           13

/* automatically calculates global channel -> local player channel */
#define GET_HISTORY(ch, channel) ((ch)->specials.chan_hist[(channel)-CHAN_GLOBAL_MAX])

/* History related procedures */

void free_history(struct history_list **history);
void add_history(char *message, struct char_data *ch, struct history_list **history);
void print_history(struct char_data *ch, int channel);
void clear_history(struct char_data *ch);
void chan_history(char *message, struct char_data *from, struct char_data *to, int level, int channel);

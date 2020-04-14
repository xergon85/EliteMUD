/* ****************************************************************************
*  File: cstore.h                                           Part of EliteMUD  *
*  Based upon shop.h                                                          *
*  Usage: spec-procs and other funcs for clan storing.                        *
*                                                                             *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#ifndef __CSTORE_H__
#define __CSTORE_H__

#define CSTORE_MAX_TRADE 5
/* #define MAX_TRADE 5   - Since this may be a global define, alawys specify
 * more clearly where it comes from, and what it deals with, to avoid
 * ambiguities -Petrus 
 */

struct cstore_data {
  byte   type[CSTORE_MAX_TRADE+1];   /* Which item to trade.                 */
  char  *no_such_item1;       /* Message if keeper hasn't got an item */
  char  *no_such_item2;       /* Message if player hasn't got an item */
  char  *message_buy;         /* Message when player buys item        */
  char  *message_sell;        /* Message when player sells item       */
  int    temper2;             /* How does keeper react when attacked  */
  int	 keeper;              /* The mobil who owns the shop (virtual)*/
  int	 with_who;	      /* Who does the shop trade with?	      */
  int	 in_room;	      /* Where is the shop?		      */
};

extern struct cstore_data *cstore_index;
extern int number_of_stores;

#endif  /* __CSTORE_H__ */ 

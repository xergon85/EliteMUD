/* ************************************************************************
*   File: shop.h                                        Part of EliteMUD  *
*  Usage: spec-procs and other funcs for shops and shopkeepers            *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#ifndef SHOP_H
#define SHOP_H

#define MAX_TRADE 5
#define MAX_PROD 10

struct shop_data {
   int	 producing[MAX_PROD+1];/* Which item to produce (virtual)      */
   float profit_buy;         /* Factor to multiply cost with.        */
   float profit_sell;        /* Factor to multiply cost with.        */
   byte  type[MAX_TRADE+1];    /* Which item to trade.                 */
   char	 *no_such_item1;     /* Message if keeper hasn't got an item */
   char	 *no_such_item2;     /* Message if player hasn't got an item */
   char	 *missing_cash1;     /* Message if keeper hasn't got cash    */
   char	 *missing_cash2;     /* Message if player hasn't got cash    */
   char	 *do_not_buy;	     /* If keeper dosn't buy such things.    */
   char	*message_buy;        /* Message when player buys item        */
   char	*message_sell;       /* Message when player sells item       */
   int	temper1;             /* How does keeper react if no money    */
   int	temper2;             /* How does keeper react when attacked  */
   int	keeper;              /* The mobil who owns the shop (virtual)*/
   int	with_who;	     /* Who does the shop trade with?	     */
   int	in_room;	     /* Where is the shop?		     */
   int	open1, open2;	     /* When does the shop open?	     */
   int	close1, close2;	     /* When does the shop close?	     */
   int	bankAccount;  	     /* Store all gold over 15000            */
};


extern struct shop_data *shop_index;
extern int number_of_shops;

#endif 

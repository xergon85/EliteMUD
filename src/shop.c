/* ************************************************************************
*   File: shop.c                                        Part of EliteMUD  *
*  Usage: spec-procs and other funcs for shops and shopkeepers            *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "functions.h"
#include "shop.h"

struct shop_data *shop_index;
int number_of_shops = 0;

extern struct room_data **world;
extern struct time_info_data time_info;
extern struct str_app_type str_app[];
extern struct index_data *mob_index;
extern int highest_obj_level(struct obj_data *obj);


ACMD(do_tell);
ACMD(do_action);
ACMD(do_emote);
ACMD(do_say);

int	is_ok(struct char_data *keeper, struct char_data *ch, int shop_nr)
{
   if (shop_index[shop_nr].open1 > time_info.hours) {
      do_say(keeper, "Come back later!", 17, 0);
      return(FALSE);
   } else if (shop_index[shop_nr].close1 < time_info.hours) {
      if (shop_index[shop_nr].open2 > time_info.hours) {
	 do_say(keeper, "Sorry, we have closed, but come back later.", 17, 0);
	 return(FALSE);
      } 
      else if (shop_index[shop_nr].close2 < time_info.hours) {
	 do_say(keeper, "Sorry, come back tomorrow.", 17, 0);
	 return(FALSE);
      }
   }

   if (!(CAN_SEE(keeper, ch))) {
      do_say(keeper, "I don't trade with someone I can't see!", 17, 0);
      return(FALSE);
   };

   switch (shop_index[shop_nr].with_who) {
   case 0  : 
      return(TRUE);
   case 1  : 
      return(TRUE);
   default : 
      return(TRUE);
   };
}


int	trade_with(struct obj_data *item, int shop_nr)
{
  int	counter = 0;

  if (item->obj_flags.cost < 1)
    return(FALSE);

  while (shop_index[shop_nr].type[counter] != -1)
    if (shop_index[shop_nr].type[counter++] == item->obj_flags.type_flag)
      return(TRUE);
  return(FALSE);
}


int	shop_producing(struct obj_data *item, int shop_nr)
{
  int	counter = 0;

  if (item->item_number < 0)
    return(FALSE);

  while (shop_index[shop_nr].producing[counter] != -1)
    if (shop_index[shop_nr].producing[counter++] == item->item_number)
      return(TRUE);
  return(FALSE);
}


void	shopping_buy( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	argm[100], buf[MAX_STRING_LENGTH];
   struct obj_data *temp1;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   one_argument(arg, argm);
   if (!(*argm)) {
      sprintf(buf, "%s what do you want to buy??", GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   };
   if (!( temp1 =  get_obj_in_list_vis(ch, argm, keeper->carrying))) {
      sprintf(buf, shop_index[shop_nr].no_such_item1 , GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   if (temp1->obj_flags.cost <= 0) {
      sprintf(buf, shop_index[shop_nr].no_such_item1 , GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      extract_obj(temp1);
      return;
   }

   if (GET_GOLD(ch) < (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_buy) && GET_LEVEL(ch) < LEVEL_GREATER) {
      sprintf(buf, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);

      switch (shop_index[shop_nr].temper1) {
      case 0:
	 do_action(keeper, GET_NAME(ch), 30, 0);
	 return;
      case 1:
	 do_emote(keeper, "smokes a joint.", 36, 0);
	 return;
      default:
	 return;
      }
   }

   if (GET_ITEM_LEVEL(temp1) > GET_LEVEL(ch)) {
     sprintf(buf, "%s : This item surpasses your might.\r\n", fname(temp1->name));
     send_to_char(buf, ch);
     return;
   }

   /*ADDED possible misuse to get items higher level inside a low level char.- Daniel*/
    if (highest_obj_level(temp1->contains) > GET_LEVEL(ch)) {
     act("$p: Contains something too mighty for you.",FALSE, ch, temp1, 0, TO_CHAR);
     return;
    }

   if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
      sprintf(buf, "%s : You can't carry that many items.\r\n",  fname(temp1->name));
      send_to_char(buf, ch);
      return;
   }

   if ((IS_CARRYING_W(ch) + temp1->obj_flags.weight) > CAN_CARRY_W(ch)) {
      sprintf(buf, "%s : You can't carry that much weight.\r\n",  fname(temp1->name));
      send_to_char(buf, ch);
      return;
   }

   if (CLANEQ(temp1))
	if ((CLANEQ_CLAN(temp1) != CLAN(ch)) ||
            (CLAN_LEVEL(ch) <= 1)) {
	     sprintf(buf, "%s : You can't buy that clan's items.\r\n", GET_NAME(ch));
	     send_to_char(buf, ch);
	     return;
	}

   act("$n buys $p.", FALSE, ch, temp1, 0, TO_ROOM);

   sprintf(buf, shop_index[shop_nr].message_buy, GET_NAME(ch), (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_buy));
   do_tell(keeper, buf, 19, 0);
   sprintf(buf, "You now have %s.\r\n", temp1->short_description);
   send_to_char(buf, ch);
   if (GET_LEVEL(ch) < LEVEL_GREATER)
      GET_GOLD(ch) -= (int)(temp1->obj_flags.cost *  shop_index[shop_nr].profit_buy);

   GET_GOLD(keeper) += (int)(temp1->obj_flags.cost *  shop_index[shop_nr].profit_buy);

   /* If the shopkeeper has more than 150000 coins, put it in the bank! */

   if (GET_GOLD(keeper) > 150000) {
     shop_index[shop_nr].bankAccount += (GET_GOLD(keeper) - 50000);
     GET_GOLD(keeper) = 50000;
     act("$n deposits some gold.", TRUE, keeper, 0, 0, TO_ROOM);
   } 

   /* Test if producing shop ! */
   if (shop_producing(temp1, shop_nr))
      temp1 = read_object(temp1->item_number, REAL);
   else
      obj_from_char(temp1);

   obj_to_char(temp1, ch);
   set_key_timer(temp1);

   return;
}


void	shopping_sell( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	argm[100], buf[MAX_STRING_LENGTH];
   struct obj_data *temp1;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   one_argument(arg, argm);

   if (!(*argm)) {
      sprintf(buf, "%s What do you want to sell??" , GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   if (!( temp1 = get_obj_in_list_vis(ch, argm, ch->carrying))) {
      sprintf(buf, shop_index[shop_nr].no_such_item2 , GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   if (!(trade_with(temp1, shop_nr)) || (temp1->obj_flags.cost < 1)) {
      sprintf(buf, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   if (IS_OBJ_STAT(temp1, ITEM_DONATED)) {
     sprintf(buf, "%s Trying to sell donated items is frowned upon", GET_NAME(ch));
     do_tell(keeper, buf, 19, 0);
     return;
   }

   if (CLANEQ(temp1)) {
	sprintf(buf, "%s I don't buy those clan items", GET_NAME(ch));
	do_tell(keeper, buf, 19, 0);
	return;
   }

   if ((GET_GOLD(keeper) + shop_index[shop_nr].bankAccount) <  (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell)) {
      sprintf(buf, shop_index[shop_nr].missing_cash1 , GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);

   sprintf(buf, shop_index[shop_nr].message_sell, GET_NAME(ch), (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell));
   do_tell(keeper, buf, 19, 0);
   sprintf(buf, "The shopkeeper now has %s.\r\n", temp1->short_description);
   send_to_char(buf, ch);
   GET_GOLD(ch) += (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell);

   /* Get money from the bank, buy the obj, then put money back. */
   GET_GOLD(keeper) += shop_index[shop_nr].bankAccount;
   shop_index[shop_nr].bankAccount = 0;

   GET_GOLD(keeper) -= (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell);

   /* If the shopkeeper has more than 15000 coins, put it in the bank! */
   /* disabled since keepers have so many HP now
        if (GET_GOLD(keeper) > 15000) {
           shop_index[shop_nr].bankAccount += (GET_GOLD(keeper) - 15000);
           GET_GOLD(keeper) = 15000;
        } 
*/
   if ((get_obj_in_list(argm, keeper->carrying)) ||  (GET_ITEM_TYPE(temp1) == ITEM_TRASH))
      extract_obj(temp1);
   else {
      obj_from_char(temp1);
      obj_to_char(temp1, keeper);
   }

   return;
}


void	shopping_value( char *arg, struct char_data *ch, 
struct char_data *keeper, int shop_nr)
{
   char	argm[100], buf[MAX_STRING_LENGTH];
   struct obj_data *temp1;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   one_argument(arg, argm);

   if (!(*argm)) {
      sprintf(buf, "%s What do you want me to valuate??", GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   if (!( temp1 = get_obj_in_list_vis(ch, argm, ch->carrying))) {
      sprintf(buf, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   if (!(trade_with(temp1, shop_nr))) {
      sprintf(buf, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;
   }

   sprintf(buf, "%s I'll give you %d gold coins for that!", GET_NAME(ch), (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell));
   do_tell(keeper, buf, 19, 0);

   return;
}


void	shopping_list( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	buf[LARGE_BUFSIZE], buf2[SMALL_BUFSIZE], buf3[SMALL_BUFSIZE];
   struct obj_data *temp1;
   extern char	*drinks[];
   int	found_obj;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   sprintf(buf, "§w%-36s %9s %s§N\r\n", "You can buy:", "Price", "Level");
   found_obj = FALSE;
   if (keeper->carrying)
      for (temp1 = keeper->carrying; temp1; temp1 = temp1->next_content)
	 if ((CAN_SEE_OBJ(ch, temp1)) && (temp1->obj_flags.cost > 0)) {
	    found_obj = TRUE;
	    if (temp1->obj_flags.type_flag != ITEM_DRINKCON)
		sprintf(buf2, "§w%-36s §y%9d§N [§r%3d§N]\r\n" , (temp1->short_description), (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy), GET_ITEM_LEVEL(temp1));
	    else {
	       if (temp1->obj_flags.value[1])
		  sprintf(buf3, "%s of %s", (temp1->short_description), drinks[temp1->obj_flags.value[2]]);
	       else
		  sprintf(buf3, "%s", (temp1->short_description));
	       sprintf(buf2, "§w%-36s §y%9d§N [§r%3d§N]\r\n", buf3, (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy), GET_ITEM_LEVEL(temp1));
	    }
	    /* ALWAYS CHECK THAT THIS CAP IS RIGHT*/
	    strcat(buf, CAP(buf2 +2));
	 };

   if (!found_obj)
      strcat(buf, "Nothing!\r\n");

   page_string(ch->desc, buf, TRUE);
   return;
}


void	shopping_kill( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	buf[100];

   switch (shop_index[shop_nr].temper2) {
   case 0:
      sprintf(buf, "%s Don't ever try that again!", GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;

   case 1:
      sprintf(buf, "%s Scram - midget!", GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;

   default :
      return;
   }
}


SPECIAL(shop_keeper)
{
  /*   char	argm[100]; */
   int	shop_nr;

   for (shop_nr = 0 ; shop_index[shop_nr].keeper != mob->nr; shop_nr++)
      ;

   if (CMD_IS("buy") && (ch->in_room ==  real_room(shop_index[shop_nr].in_room))) {/* Buy */

      shopping_buy(arg, ch, mob, shop_nr);
      return(TRUE);
   }

   if (CMD_IS("sell") && (ch->in_room ==  real_room(shop_index[shop_nr].in_room))) {/* Sell */

      shopping_sell(arg, ch, mob, shop_nr);
      return(TRUE);
   }

   if (CMD_IS("value") && (ch->in_room ==  real_room(shop_index[shop_nr].in_room))) {/* value */

      shopping_value(arg, ch, mob, shop_nr);
      return(TRUE);
   }

   if (CMD_IS("list") && (ch->in_room ==  real_room(shop_index[shop_nr].in_room))) {/* List */

      shopping_list(arg, ch, mob, shop_nr);
      return(TRUE);
   }

   /* Removing Shopkeepers superiority (test) 

   if ((cmd == 25) || (cmd == 70)) {
       one_argument(arg, argm);
       
       if (mob == get_char_room(argm, ch->in_room)) {
	   shopping_kill(arg, ch, mob, shop_nr);
	   return(TRUE);
       }
   } else if ((cmd == 84) || (cmd == 207) || (cmd == 172)) {
       act("$N tells you 'No magic here - kid!'.", FALSE, ch, 0, mob, TO_CHAR);
       return TRUE;
   }
   */
   return(FALSE);
}


void	boot_the_shops(FILE *shop_f, char *filename)
{
  char	buf2[150];
  int	temp, count, nr;

  sprintf(buf2, "beginning of shop file %s", filename);
  
  if (!fscanf(shop_f, "%s\n", buf2)) {
    perror("load_shops");
    exit(1);
  }

  for (; ; ) {
    if (*buf2 == '#') { /* a new shop */ 
      
      sscanf(buf2, "#%d\n", &nr);
      if (nr >= 99999) 
	break;

      sprintf(buf2, "shop #%d in shop file %s", nr, filename);
      
      for (count = 0; count <= MAX_PROD; count++) {
	temp = fread_number(shop_f, buf2);
	if (temp == -1) {
	  shop_index[number_of_shops].producing[count] = -1;
	  break;
	}
	if ((temp = real_object(temp)) < 0) {
	  log("Invalid producing item vnum.");
	  log(buf2);
	} else
	  shop_index[number_of_shops].producing[count] = temp;
      }
      
      fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_buy);
      fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_sell);
      
      for (count = 0; count <= MAX_TRADE; count++) {
	temp = fread_number(shop_f, buf2);
	shop_index[number_of_shops].type[count] =  (byte) temp;
	if (temp == -1)
	  break;
      }
      shop_index[number_of_shops].no_such_item1 =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].no_such_item2 =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].do_not_buy    =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].missing_cash1 =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].missing_cash2 =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].message_buy   =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].message_sell  =  fread_string(shop_f, buf2);
      shop_index[number_of_shops].temper1       =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].temper2       =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].keeper        =
	real_mobile(fread_number(shop_f, buf2));

      shop_index[number_of_shops].with_who      =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].in_room       =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].open1         =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].close1        =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].open2         =  fread_number(shop_f, buf2);
      shop_index[number_of_shops].close2        =  fread_number(shop_f, buf2);

      shop_index[number_of_shops].bankAccount = 1000000;
      number_of_shops++;
    } else if (*buf2 == '$') /* EOF */
      break;
    else {
      sprintf(buf2, "SYSERR: Format error in shop file near shop #%d", nr);
      log(buf2);
      exit(1);
    }
    
    if (!fscanf(shop_f, "%s\n", buf2)) {
      sprintf(buf2, "SYSERR: Format error in mob file near shop #%d", nr);
      log(buf2);
      exit(1);
    }
  }
}


void	assign_the_shopkeepers(void)
{
   int	temp1;

   for (temp1 = 0 ; temp1 < number_of_shops ; temp1++)
      mob_index[shop_index[temp1].keeper].func = shop_keeper;
}

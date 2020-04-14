/* ****************************************************************************
*  File: cstore.c                                           Part of EliteMUD  *
*  Based upon shop.h                                                          *
*  Usage: spec-procs and other funcs for clan storing.                        *
*                                                                             *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "functions.h"
#include "cstore.h"


struct cstore_data *cstore_index;
int number_of_stores = 0;

extern struct room_data **world;
extern struct time_info_data time_info;
extern struct str_app_type str_app[];
extern struct index_data *mob_index;


ACMD(do_tell);
ACMD(do_action);
ACMD(do_emote);
ACMD(do_say);


/* For functions not intended to be used outside this module, always make them
 * STATIC - this also to avoid ambiguities -Petrus
 */
static int
c_is_ok(struct char_data *keeper, struct char_data *ch, int cstore_nr)
{
  if (!(CAN_SEE(keeper, ch))) {
    do_say(keeper, "I can't help you if I can't see you!", 17, 0);
    return(FALSE);
  };
  
  switch (cstore_index[cstore_nr].with_who) {
  case 0  : 
    return(TRUE);
  default : 
    if (CLAN(ch) == real_clan(cstore_index[cstore_nr].with_who) )
      return(TRUE);
    do_say(keeper, "You are not permitted to be here, Leave NOW!", 17, 0);
    return(FALSE);
  };
}

static void
Cash_from_store(char *arg, struct char_data *ch, struct char_data *keeper, int cstore_nr)
{
  int amount;
  char buf[MAX_STRING_LENGTH];
  if (!(c_is_ok(keeper,ch,cstore_nr)) || CLAN_LEVEL(ch)!=8) {
    sprintf(buf, "%s You are not authorized to withdraw gold!\r\n", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }
  
  if ((amount = atoi(arg)) <= 0) {
    sprintf(buf, "%s How much do you want to withdraw?\r\n", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }
  if (GET_GOLD(keeper) < amount) {
    sprintf(buf, "%s I don't have that many coins on me\r\n", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }
  GET_GOLD(keeper) -= amount;
  GET_GOLD(ch) += amount;
  sprintf(buf, "%s You withdraw %d coins.\r\n", GET_NAME(ch), amount);
  do_tell(keeper, buf, 19, 0);

/*  act("The clerk gives some money to $n.", TRUE, ch, 0, FALSE, TO_ROOM); */
  act("$n gives some money to $n.", TRUE, ch, 0, FALSE, TO_ROOM);

  return;
}

static void
Cash_to_store(char *arg, struct char_data *ch, struct char_data *keeper, int cstore_nr)
{
  int amount;
  char buf[MAX_STRING_LENGTH];
  
  if (!(c_is_ok(keeper,ch,cstore_nr))) return;
  
  if ((amount = atoi(arg)) <= 0) {
    sprintf(buf, "%s How much do you want to give me?\r\n", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }
  if (GET_GOLD(ch) < amount) {
    sprintf(buf, "%s You don't have that many coins on you!\r\n", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
    }
  GET_GOLD(ch) -= amount;
  GET_GOLD(keeper) += amount;
  sprintf(buf, "%s You give %d coins.\r\n", GET_NAME(ch), amount);
  do_tell(keeper, buf, 19, 0);
  act("$n gives some money to the clerk.", TRUE, ch, 0, FALSE, TO_ROOM);
  return;
}


static void
Take_from_store( char *arg, struct char_data *ch, struct char_data *keeper, int cstore_nr)
{
  char	argm[100], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1;
  
  if (!(c_is_ok(keeper, ch, cstore_nr)))
    return;
  
  one_argument(arg, argm);
  if (!(*argm)) {
    sprintf(buf, "%s what do you want to take??", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  };
  if (!( temp1 =  get_obj_in_list_vis(ch, argm, keeper->carrying))) {
    sprintf(buf, cstore_index[cstore_nr].no_such_item1 , GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }
  
  if (temp1->obj_flags.cost <= 0) {
    sprintf(buf, cstore_index[cstore_nr].no_such_item1 , GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    extract_obj(temp1);
    return;
  }
  
  
  if (GET_ITEM_LEVEL(temp1) > GET_LEVEL(ch)) {
    sprintf(buf, "%s This item surpasses your might.\r\n", fname(temp1->name));
    send_to_char(buf, ch);
    return;
  }
  
  if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
    sprintf(buf, "%s You can't carry that many items.\r\n",  fname(temp1->name));
    send_to_char(buf, ch);
    return;
  }
  
  if ((IS_CARRYING_W(ch) + temp1->obj_flags.weight) > CAN_CARRY_W(ch)) {
    sprintf(buf, "%s You can't carry that much weight.\r\n",  fname(temp1->name));
    send_to_char(buf, ch);
    return;
  }
  
  
  act("$n takes $p.", FALSE, ch, temp1, 0, TO_ROOM);
  
  sprintf(buf, cstore_index[cstore_nr].message_buy, GET_NAME(ch) );
  do_tell(keeper, buf, 19, 0);
  sprintf(buf, "You now have %s.\r\n", temp1->short_description);
  send_to_char(buf, ch);
  obj_from_char(temp1);
  
  obj_to_char(temp1, ch);
  
  return;
}


static void
Give_to_store( char *arg, struct char_data *ch, struct char_data *keeper, int cstore_nr)
{
  char	argm[100], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1;
  
  if (!(c_is_ok(keeper, ch, cstore_nr)))
    return;
  
  one_argument(arg, argm);
  
  if (!(*argm)) {
    sprintf(buf, "%s What do you want to give me??" , GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }

  if (!( temp1 = get_obj_in_list_vis(ch, argm, ch->carrying))) {
    sprintf(buf, cstore_index[cstore_nr].no_such_item2 , GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
  }
  
  
  act("$n stores $p.", FALSE, ch, temp1, 0, TO_ROOM);
  
  sprintf(buf, cstore_index[cstore_nr].message_sell, GET_NAME(ch));
  do_tell(keeper, buf, 19, 0);
  sprintf(buf, "The clerk now has %s.\r\n", temp1->short_description);
  send_to_char(buf, ch);
  
  obj_from_char(temp1);
  obj_to_char(temp1, keeper);
  
  return;
}


static void
store_list( char *arg, struct char_data *ch, struct char_data *keeper, int cstore_nr)
{
  char	buf[LARGE_BUFSIZE], buf2[100], buf3[100];
  struct obj_data *temp1;
  extern char	*drinks[];
  int	found_obj;
  
  if (!(c_is_ok(keeper, ch, cstore_nr)))
    return;
  
  sprintf(buf, "#w%-36s %s#N\r\n", "I can fetch for you:", "Level");
  found_obj = FALSE;
  if (keeper->carrying)
    for (temp1 = keeper->carrying; temp1; temp1 = temp1->next_content) {
      found_obj = TRUE;
      if (temp1->obj_flags.type_flag != ITEM_DRINKCON)
	sprintf(buf2, "#w%-36s [#r%3d#N]\r\n" , (temp1->short_description), GET_ITEM_LEVEL(temp1));
      else {
	sprintf(buf3, "%s", (temp1->short_description));
	sprintf(buf2, "#w%-36s [#r%3d#N]\r\n", buf3, GET_ITEM_LEVEL(temp1));
      }
      CAP(buf2 + 2);  /* ALWAYS CHECK THAT THIS CAP IS RIGHT*/
      strcat(buf, buf2);
    };
  if (!found_obj)
    strcat(buf, "Nothing!\r\n");
  sprintf(buf2, "I have %d cash\r\n", GET_GOLD(keeper));
  strcat(buf, buf2);
  page_string(ch->desc, buf, TRUE);
  return;
}


static void
cstore_kill( char *arg, struct char_data *ch, struct char_data *keeper, int cstore_nr)
{
  char	buf[100];
  
  switch (cstore_index[cstore_nr].temper2) {
  case 0:
    sprintf(buf, "%s Don't ever try that again!", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
    
  case 1:
    sprintf(buf, "%s Scram - Rookie!", GET_NAME(ch));
    do_tell(keeper, buf, 19, 0);
    return;
    
  default :
    return;
  }
}


SPECIAL(cstore_keeper)
{
  char	argm[100];
  struct char_data *temp_char;
  int	cstore_nr;
  
  for (cstore_nr = 0 ; cstore_index[cstore_nr].keeper != mob->nr; cstore_nr++)
    ;
  
  if (CMD_IS("fetch") && (ch->in_room ==  real_room(cstore_index[cstore_nr].in_room))) {/* fetch */
    
    Take_from_store(arg, ch, mob, cstore_nr);
    return(TRUE);
  }
  
  if (CMD_IS("store") && (ch->in_room ==  real_room(cstore_index[cstore_nr].in_room))) {/* Deposit */
    
    Give_to_store(arg, ch, mob, cstore_nr);
    return(TRUE);
  }
  
  if (CMD_IS("deposit") && (ch->in_room ==  real_room(cstore_index[cstore_nr].in_room))) {/* deposit */
    
    Cash_to_store(arg, ch, mob, cstore_nr);
    return(TRUE);
  }
  
  if (CMD_IS("withdraw") && (ch->in_room ==  real_room(cstore_index[cstore_nr].in_room))) {/* withdraw */
    
    Cash_from_store(arg, ch, mob, cstore_nr);
    return(TRUE);
  }
  
  if (CMD_IS("list") && (ch->in_room ==  real_room(cstore_index[cstore_nr].in_room))) {/* List */
    
    store_list(arg, ch, mob, cstore_nr);
    return(TRUE);
  }

  /* Removing Shopkeepers superiority (test) */
  
  if ((cmd == 25) || (cmd == 70)) {
    one_argument(arg, argm);
    
    if (mob == get_char_room(argm, ch->in_room)) {
      cstore_kill(arg, ch, mob, cstore_nr);
      return(TRUE);
    }
  } else if ((cmd == 84) || (cmd == 207) || (cmd == 172)) {
    act("$N tells you 'No magic here - friend!'.", FALSE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }
  
  return(FALSE);
}


void
boot_the_cstores(FILE *cstore_f, char *filename)
{
  char	*buf, buf2[150];
  int	temp, count, nr;
  
  sprintf(buf2, "beginning of cstore file %s", filename);
  
  if (!fscanf(cstore_f, "%s\n", buf2)) {
    perror("load cstores");
    exit(1);
  }
  
  for (; ; ) {
    if (*buf2 == '#') { /* a new cstore */ 
      
      sscanf(buf2, "#%d\n", &nr);
      if (nr >= 99999) 
	break;
      
      sprintf(buf2, "cstore #%d in cstore file %s", nr, filename);
      
      cstore_index[number_of_stores].no_such_item1 =  fread_string(cstore_f, buf2);
      cstore_index[number_of_stores].no_such_item2 =  fread_string(cstore_f, buf2);
      cstore_index[number_of_stores].message_buy   =  fread_string(cstore_f, buf2);
      cstore_index[number_of_stores].message_sell  =  fread_string(cstore_f, buf2);
      cstore_index[number_of_stores].temper2       =  fread_number(cstore_f, buf2);
      cstore_index[number_of_stores].keeper        =
	real_mobile(fread_number(cstore_f, buf2));
      
      cstore_index[number_of_stores].with_who      =  fread_number(cstore_f, buf2);
      cstore_index[number_of_stores].in_room       =  fread_number(cstore_f, buf2);
      
      number_of_stores++;
    } else if (*buf2 == '$') /* EOF */
      break;
    else {
      sprintf(buf2, "SYSERR: Format error in cstore file near store #%d", nr);
      log(buf2);
      exit(1);
    }
    
    if (!fscanf(cstore_f, "%s\n", buf2)) {
      sprintf(buf2, "SYSERR: Format error in mob file near cstore #%d", nr);
      log(buf2);
      exit(1);
    }
  }
}


void
assign_the_cstorekeepers(void)
{
  int	temp1;
  
  for (temp1 = 0 ; temp1 < number_of_stores ; temp1++)
    mob_index[cstore_index[temp1].keeper].func = cstore_keeper;
}


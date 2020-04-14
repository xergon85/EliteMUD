/* *********************************************************************** *
*   File: casino.c                                       Part of EliteMUD  *
*  Usage: Special procedures for casino code                               *
*                                                                          *
*  (C) 1995 Petrus Wang f93-pwa Royal Institute of Technology              *
************************************************************************* */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "gen_cards.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "functions.h"

extern struct room_data **world;

struct bet_data {
  long idnum;             /* IDNUM of the better   */
  int  amount;            /* The amount of the bet */
  struct bet_data *next;
};


#define ROULETTE_BLUE 37
#define ROULETTE_RED   38
#define ROULETTE_LOW   39
#define ROULETTE_MID   40
#define ROULETTE_HIGH  41

const char* roulette_names[] = {
  "BLUE",
  "RED",
  "LOW",
  "MID",
  "HIGH",
  ""
};

struct bet_data * roulette_index[42];

char roulette_table[] =
"              _____________________________________\r\n"
"             /                 0                  /\r\n"
"            /--/--/--/----/--/--/--/----/--/--/--/\r\n"
"           / 1/ 2/ 3/    /13/14/15/    /25/26/27/\r\n"
"          /--/--/--/    /--/--/--/ E  /--/--/--/\r\n"
"         / 4/ 5/ 6/ D  /16/17/18/    /28/29/30/\r\n"
"        /--/--/--/    /--/--/--/ U  /--/--/--/\r\n"
"       / 7/ 8/ 9/ E  /19/20/21/    /31/32/33/\r\n"
"      /--/--/--/    /--/--/--/ L  /--/--/--/\r\n"
"     /10/11/12/ R  /22/23/24/    /34/35/36/\r\n"
"    /--/--/--/    /--/--/--/ B  /--/--/--/\r\n"
"   /  LOW   /    /  MID   /    /  HIGH  /\r\n"
"  /________/____/________/____/________/\r\n";


static void
init_roulette(void)
{
  int i;
  
  for (i = 0; i <= ROULETTE_HIGH; i++)
    roulette_index[i] = 0;
} 


void
show_bets_on_slot(struct char_data *ch, int slot)
{
  struct bet_data *ptr;
  int amount = 0;

  if (slot <= 36)
    sprintf(buf2, "%6d:", slot);
  else
    sprintf(buf2, "%6s:", roulette_names[slot-ROULETTE_BLUE]);

  ptr = roulette_index[slot];
  
  while (ptr) {
    if (GET_IDNUM(ch) == ptr->idnum)
      amount += ptr->amount;
    ptr = ptr->next;
  }

  if (amount) {
    sprintf(buf2, "%s %d gold\r\n", buf2, amount);
    send_to_char(buf2, ch);
  }
}

void
show_bets(struct char_data *ch)
{
  int i;
  
  send_to_char("Your roulette bets:\r\n", ch);
  for (i = 0; i <= ROULETTE_HIGH; show_bets_on_slot(ch, i++));
}


void
add_bet(struct char_data *ch, int amount, int slot)
{
  struct bet_data *ptr;

  CREATE(ptr, struct bet_data, 1);
  ptr->idnum = GET_IDNUM(ch);
  ptr->amount = amount;

  ptr->next = roulette_index[slot];
  roulette_index[slot] = ptr;

  sprintf(buf2, "OK - %d gold placed.\r\n", amount);
  send_to_char(buf2, ch);
}
  

void
remove_bet_list(struct bet_data *ls)
{
  if (ls) {
    remove_bet_list(ls->next);
    free(ls);
  }
}

void
remove_all_bets(void)
{
  int i;

  for (i = 0; i <= ROULETTE_HIGH; i++) {
    remove_bet_list(roulette_index[i]);
    roulette_index[i] = 0;
  }
}


struct char_data *
get_player_in_room_using_id(long id, int room)
{
  struct char_data *tmp;

  tmp = world[room]->people;

  while (tmp) {
    if (GET_IDNUM(tmp) == id)
      return tmp;
    tmp = tmp->next_in_room;
  }
  return NULL;
}


void
check_for_winner(int slot, float mult, int room)
{
  struct bet_data *ptr;
  struct char_data *winner;
  int gold_won = 0;

  ptr = roulette_index[slot];

  while (ptr) {
    if ((winner = get_player_in_room_using_id(ptr->idnum, room))) {
      sprintf(buf2, "You've won %2.2f * %d = %d gold!\r\n",
	     mult, ptr->amount, (gold_won = mult * ptr->amount));
      send_to_char(buf2, winner);
      act("$n has won some gold.", TRUE, winner, 0, 0, TO_ROOM);
      GET_GOLD(winner) += gold_won;
    }
    
    ptr = ptr->next;
  }
}



ACMD(do_say);

SPECIAL(roulette)
{
  static int round = 0;
  unsigned int amount, slot;

  if (IS_NPC(ch) && cmd)
    return FALSE;

  arg = one_argument(arg, buf);

  if (!round) {
    init_roulette();
    round++;
  }

  if (!cmd) {
    switch (round++) {
    case 1: case 2: case 3: 
      do_say(mob, "Place your bets please!", 0, 0);
      break;
    case 4:
      do_say(mob, "No more bets please!", 0, 0);
      break;
    case 5:
      act("$n starts the roulette wheel spinning ...",
          FALSE, mob, 0, 0, TO_ROOM);
      break;
    case 6:
      act("$n throws the ball into the roulette wheel ...",
          FALSE, mob, 0, 0, TO_ROOM);      
      break;
    case 7:
      act("The ball skitters around the wheel at high speed ...",
          FALSE, mob, 0, 0, TO_ROOM);
      break;
    case 8:
      act("The roulette wheel starts to slow down and it stops on ...",
	  FALSE, mob, 0, 0, TO_ROOM);
      slot = number(0, 36);

      if (slot % 2)
	sprintf(buf, "... BLUE %d.", slot);
      else
	sprintf(buf, "... RED %d.", slot);

      act(buf, FALSE, mob, 0, 0, TO_ROOM);

      check_for_winner(slot, 20.0, IN_ROOM(mob));
      if (slot % 2)
	check_for_winner(ROULETTE_BLUE, 1.5, IN_ROOM(mob));
      else
	check_for_winner(ROULETTE_RED, 1.5, IN_ROOM(mob));
      if (slot < 13)
	check_for_winner(ROULETTE_LOW, 2.4, IN_ROOM(mob));
      else if (slot < 25)
	check_for_winner(ROULETTE_MID, 2.4, IN_ROOM(mob));
      else
	check_for_winner(ROULETTE_HIGH, 2.4, IN_ROOM(mob));

      remove_all_bets();

      break;
    case 9: 
      break;
    case 10:
      if (number(1,6) == 1) 
        act("$n looks around for some more punters to profit from.",
            FALSE, mob, 0, 0, TO_ROOM);
      else if (number(1,4) == 1)
        act("$n wonders when his next break is due.",
            FALSE, mob, 0, 0, TO_ROOM);
      break;
    case 11:
      break;
    };
    
    if (round > 11)
      round = 1;
    
    return TRUE;
  }

  if (CMD_IS("emote") || CMD_IS(":")) {     /* ban emote to prevent spoofing */
    send_to_char("You can't emote in the roulette room!\r\n", ch);
    return TRUE;
  }

  if (CMD_IS("look") && is_abbrev(buf, "table")) {
    send_to_char(roulette_table, ch);
    return TRUE;
  }

  if (CMD_IS("bet")) {
    if (is_abbrev(buf, "list")) {
      show_bets(ch);
      return TRUE;
    } else if (round > 4) {
      send_to_char("No more bets now.\r\n", ch);
      return TRUE;
    } else if ((amount = atoi(buf)) <= 0) {
      send_to_char("Usage: bet AMOUNT SLOT - try HELP ROULETTE\r\n", ch);
      return TRUE;
    } else {
      if (amount < 1000 || amount > 100000) {
	send_to_char("Your bet is not valid - read the rules!\r\n", ch);
	return TRUE;
      }
      if (GET_GOLD(ch) < amount) {
	act("$N tells you, 'You don't have that much gold'", FALSE, ch, 0, mob, TO_CHAR);
	return(TRUE);
      }

      GET_GOLD(ch) -= amount;

      arg = one_argument(arg, buf);
      if (is_abbrev(buf, "low"))
	slot = ROULETTE_LOW;
      else if (is_abbrev(buf, "mid"))
	slot = ROULETTE_MID;
      else if (is_abbrev(buf, "high"))
	slot = ROULETTE_HIGH;
      else if (is_abbrev(buf, "red"))
	slot = ROULETTE_RED;
      else if (is_abbrev(buf, "blue"))
	slot = ROULETTE_BLUE;
      else if ((slot = atoi(buf)) > 36) {
	send_to_char("Valid slots are 0-36, low, mid, high, red and blue.\r\n", ch);
	return TRUE;
      }
      
      add_bet(ch, amount, slot);
    }
    
    return TRUE;
  }

  return FALSE;
}

/* ************************************************************************
*   File: gen_cards.c                                    Part of EliteMUD  *
*  Usage: A generic card game system for ELiteMud                          *
*                                                                          *
*  (C) 1994 Petrus Wang f93-pwa Royal Institute of Technology              *
************************************************************************* */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "gen_cards.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "functions.h"


char   *cards[] = {
    "2  §bs",
    "2  §bc",
    "2  §rd",
    "2  §rh",
    "3  §bs",
    "3  §bc",
    "3  §rd",
    "3  §rh",
    "4  §bs",
    "4  §bc",
    "4  §rd",
    "4  §rh",
    "5  §bs",
    "5  §bc",
    "5  §rd",
    "5  §rh",
    "6  §bs",
    "6  §bc",
    "6  §rd",
    "6  §rh",
    "7  §bs",
    "7  §bc",
    "7  §rd",
    "7  §rh",
    "8  §bs",
    "8  §bc",
    "8  §rd",
    "8  §rh",
    "9  §bs",
    "9  §bc",
    "9  §rd",
    "9  §rh",
    "10 §bs",
    "10 §bc",
    "10 §rd",
    "10 §rh",
    "J  §bs",
    "J  §bc",
    "J  §rd",
    "J  §rh",
    "Q  §bs",
    "Q  §bc",
    "Q  §rd",
    "Q  §rh",
    "K  §bs",
    "K  §bc",
    "K  §rd",
    "K  §rh",
    "A  §bs",
    "A  §bc",
    "A  §rd",
    "A  §rh",
    " JJ ",
    " JJ "
    };


/* Free a complete list of cards.  ls should be the first in line */
void   free_card_list(struct card_list *ls)
{
    if (ls->next)
	free_card_list(ls->next);
    free(ls);
};


/* CREATES a complete deck of cards and 'gives' them to ch */
void   init_card_deck(struct char_data *ch)
{
    int i;
    struct card_list *ls;

    CREATE(ch->specials.deck_head, struct card_list, 1);
    ls = ch->specials.deck_head;
    ls->value = 0;
    ls->prev = 0;

    for (i = 1;i < 52;i++) {
	CREATE(ls->next, struct card_list, 1);
	ls->next->prev = ls;
	ls = ls->next;
	ls->value = i;
    }
    ls->next = 0;
    ch->specials.deck_tail = ls;
}

/* COUNT a card_list. I e how many cards after ls */
int   count_deck(struct card_list *ls)
{
    int i;

    for (i = 0;ls;ls = ls->next, i++);
    return i;
}


/* SHUFFLES a complete or incomplete deck of cards on a ch  */
void   shuffle_deck(struct char_data *ch)
{
    struct card_list *head, *tail, *point;
    int temp;

    if (!ch->specials.deck_head || 
	!ch->specials.deck_tail)
    {
	log("Serious fuckup in shuffle_deck!");
	return;
    }
    
    head = ch->specials.deck_head;
    tail = head;
    temp = count_deck(head);
    temp = temp/2+1;
    while (--temp)
	tail = tail->next;
    point = tail;
    
    while (head != point && tail && head != tail) {
	if (number(0,1)) {
	    temp = head->value;
	    head->value = tail->value;
	    tail->value = temp;
	}
	
	if (number(0,1)) 
	    head = head->next;
	else
	    tail = tail->next;
    }
}


/* Diplays a hand of cards to a char  */
void   display_cards(struct char_data *ch, int show)
{
    char buf[256];
    char buf2[256];
    struct card_list *ls;
    int i = 0;
    
    if (!ch->specials.cards_in_hand)
	return;
    
    *buf = '\0';
    *buf2 = '\0';
    ls = ch->specials.cards_in_hand;
    while(ls) {
	sprintf(buf2, "%s  §w_____§N", buf2);
	sprintf(buf, "%s §w/%s§w/§N", buf, cards[ls->value]);
	i++;
	if (i == 8) {
	    i = 0;
	    strcat(buf2, "\r\n");
	    strcat(buf2, buf);
	    if (show)
		act(buf2, FALSE, ch, 0, 0, TO_ROOM);
	    strcat(buf2, "\r\n");
	    send_to_char(buf2, ch);
	    
	    
	    *buf = '\0';
	    *buf2 = '\0';
	}
	ls = ls->next;
    }
    strcat(buf2, "\r\n");
    strcat(buf2, buf);
    if (show)
	act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    strcat(buf2, "\r\n");
    send_to_char(buf2, ch);
}


void   deal_cards_to_char(struct char_data *ch, 
			  struct char_data *victim,
			  int num)
{
    char buf[256]; 
    struct card_list *ls;
    
    if (!ch->specials.deck_head && !ch->specials.deck_tail) {
	log("Serious fuckup in deal_cards_to_char!");
	return;
    }
	
    if (num > count_deck(ch->specials.deck_head)) {
	send_to_char("You don't have that many cards!\r\n", ch);
	return;
    }
    
    if (num < 1) {
	send_to_char("Negative cards!  Get lost!\r\n", ch);
	return;
    }
    
    *buf = '\0';

    sprintf(buf,"Ok, you deal %d card%s to %s.\r\n", num,
	    (num == 1)?"":"s", 
	    (ch == victim)?"yourself":GET_NAME(victim));
    send_to_char(buf, ch);

    if (ch != victim) {
	sprintf(buf,"%s deals you %d card%s.\r\n", GET_NAME(ch), num,
		(num == 1)?"":"s");
	send_to_char(buf, victim);
    }
    
    sprintf(buf,"$n deals %d card%s to %s.", num, (num == 1)?"":"s",
	    (ch == victim)?"$mself":"$N");
    act(buf, TRUE, ch, 0, victim, TO_NOTVICT);

    ls = victim->specials.cards_in_hand;

    while(ls && ls->next)
	ls = ls->next;

    if (ls) {
	ls->next = ch->specials.deck_head; 
	ls->next->prev = ls;
    } else 
	victim->specials.cards_in_hand = ch->specials.deck_head;

    ls = ch->specials.deck_head;
    while(--num) 
	ls = ls->next;

    ch->specials.deck_head = ls->next;
    if (ls->next)
	ls->next->prev = 0;
    ls->next = 0;
}
    

/* Return all cards from a list to the deck */
void  return_all_cards(struct char_data *ch, struct card_list *ls)
{

    if (!ls)
	return;

    if (!ch->specials.deck_head && !ch->specials.deck_tail)
	return;
    
    if (ch->specials.deck_head) {
	ch->specials.deck_tail->next = ls; 	
	ls->prev = ch->specials.deck_tail; 
    } else { 	
	ch->specials.deck_head = ls;
	ls->prev = 0; 
    }

    while(ls->next)
	ls = ls->next;

    ch->specials.deck_tail = ls;
}

void  return_all_in_hand(struct char_data *ch, struct char_data *victim)
{
    if (!victim->specials.cards_in_hand)
	return;

    return_all_cards(ch, victim->specials.cards_in_hand);
    victim->specials.cards_in_hand = 0;
}


void  return_played_cards(struct char_data *ch)
{
    if(ch->specials.cards_played) {
	return_all_cards(ch, ch->specials.cards_played);
	ch->specials.cards_played = 0;
    }
}


/* Returns specific cards to the deck */
struct card_list *  drop_cards(struct char_data *ch,
			       char *arg)
{
    struct card_list *temp;
    struct card_list *cards[53];
    char buf[256];
    int i = 0, num = 0, in_deck;
    
    
    if (!ch->specials.cards_in_hand)
	return 0;
    
    for (num = 0; num < 53; num++)
	cards[num] = 0;
    
    temp = ch->specials.cards_in_hand;
    in_deck = count_deck(temp);

    arg = one_argument(arg, buf);
    
    while ((num = atoi(buf))) {
	arg = one_argument(arg, buf);
	if (num > 0 && num <= in_deck) {
	    cards[i] = temp;
	    	    
	    while(--num) 
		cards[i] = cards[i]->next;
	   	    
	    i++;
	    
	}
    }
    
    for (num = 0; num < i;num++) {
	if (cards[num]->prev)
	    cards[num]->prev->next = cards[num]->next;
	else
	    ch->specials.cards_in_hand = cards[num]->next;
	if (cards[num]->next)
	    cards[num]->next->prev = cards[num]->prev;
	cards[num]->prev = 0;
	cards[num]->next = 0;
    }

    for (num = 0; num < i;num++) {
	cards[num]->next = cards[num + 1];
	if (num > 0)
	    cards[num]->prev = cards[num - 1];
	else
	    cards[num]->prev = 0;
    }

    if (i) 
	return cards[0];
    else
	return 0;
}
	

int   switch_cards(struct char_data *ch, int first, int second)
{
    struct card_list *ls1, *ls2;
    int num;

    if (!ch->specials.cards_in_hand)
	return 0;

    num = count_deck(ch->specials.cards_in_hand);
    if (first > num || second > num || first < 1 || second < 1) 
	return 0;
    
    ls1 = ch->specials.cards_in_hand;
    ls2 = ch->specials.cards_in_hand;

    while (--first)
	ls1 = ls1->next;
    while (--second)
	ls2 = ls2->next;

    num = ls1->value;
    ls1->value = ls2->value;
    ls2->value = num;

    return TRUE;
}
	
/* Sorts a list of cards using bubble sort */
void  sort_cards(struct  card_list *ls)
{
    struct card_list *temp;
    int len, i, val;

    if (!ls)
	return;

    len = count_deck(ls);
    
    if (len > 52 || len < 1) {
	log("Serious Error in cardgame - more than 52 cards");
	exit(1);
    }

    while (--len) {
	temp = ls;
	i = len;
	while (i) {
	    if (temp->value < temp->next->value) {
		val = temp->next->value;
		temp->next->value = temp->value;
		temp->value = val;
	    }
	    temp = temp->next;
	    --i;
	}
    }
}

void sort_cards_in_hand(struct char_data *ch)
{
    sort_cards(ch->specials.cards_in_hand);
}
	    



ACMD(do_cards)
{
    send_to_char("No cards around!\r\n", ch);
    return;
}


SPECIAL(poker)
{
    char buf[MAX_STRING_LENGTH];
    struct char_data *victim;
    struct card_list *cards;
    struct follow_type *f;
    int num, to_all = 0;
    
    arg = one_argument(arg, buf);
    
    if (CMD_IS("emote") || CMD_IS(":")) {     /* ban emote to prevent cheating */
      send_to_char("You can't emote in the poker room!\r\n", ch);
      return TRUE;
    }

    if (CMD_IS("get") && is_abbrev(buf, "cards")) {      /* get cards */
	if (ch->specials.deck_head ||
	    ch->specials.deck_tail) 
	{
	    if (ch->master) {
		send_to_char("You have to be the leader to do this!\r\n", ch);
		return TRUE;
	    }
	    
	    return_all_in_hand(ch, ch);
	    for (f = ch->followers; f; f = f->next)
		return_all_in_hand(ch, f->follower);
	    act("$n gets all $s cards back.", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char("You get all the cards back.\r\n", ch);
	} else if (ch->master) {
	    send_to_char("Only the leader can get cards.\r\n", ch);
	} else {
	    init_card_deck(ch);
	    send_to_char("Ok, you get a deck of cards.\r\n", ch);
	    act("$n gets a deck of cards.", TRUE, ch, 0, 0, TO_ROOM);
	}
	return TRUE;
    }
    
    if (CMD_IS("look") && is_abbrev(buf, "cards")) {     /* look cards */
	if (!ch->specials.cards_in_hand) {
	    send_to_char("You don't have any cards!\r\n", ch);
	} else {
	    act("$n looks at $s cards.", TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("Your hand:\r\n", ch);
	    display_cards(ch, 0);
	}
	return TRUE;
    }
    
    if (CMD_IS("shuffle") && is_abbrev(buf, "cards")) {     /* shuffle  */
	if (ch->specials.deck_head &&
	    ch->specials.deck_tail) {
	    shuffle_deck(ch);
	    send_to_char("Ok, you shuffle the deck.\r\n", ch);
	    act("$n shuffles the deck.", TRUE, ch, 0, 0, TO_ROOM);
	} else {
	    send_to_char("You don't have any cards to shuffle.\r\n", ch);
	}
	return TRUE;
    }

    if (CMD_IS("display") && is_abbrev(buf, "cards")) {      /* show */
	if (!ch->specials.cards_in_hand) {
	    send_to_char("You don't have any cards!\r\n", ch);
	} else {
	    act("$n shows $s cards:", TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("Your hand:\r\n", ch);
	    display_cards(ch, 1);
	}
	return TRUE;
    }

    if (CMD_IS("deal")) {                   /* deal cards  */
	
	if (!str_cmp(buf, "all")) {
	    victim = ch;
	    to_all = TRUE;
	} else if (!(victim = get_char_room_vis(ch, buf))) {
	    send_to_char("No such person around!\r\n", ch);
	    return TRUE;
	}
	
	if (ch->master ||
	    (victim->master != ch &&
	     ch != victim))
	{
	    send_to_char("Have to be the leader to deal!\r\n", ch);
	    return TRUE;
	}
	
	if (!str_cmp(buf, "all")) {
	    victim = 0;
	} else if (!(victim = get_char_room_vis(ch, buf))) {
	    send_to_char("No such person around!\r\n", ch);
	    return TRUE;
	}
	
	arg = one_argument(arg, buf);
	num = atoi(buf);
	
	if (num == 0) {
	    send_to_char("Have to specify a number!\r\n", ch);
	} else if (!to_all) {
	    deal_cards_to_char(ch, victim, num);
	} else {
	    for (f = ch->followers; f; f = f->next)
		if (f->follower->in_room == ch->in_room)
		    deal_cards_to_char(ch, f->follower, num);
	    deal_cards_to_char(ch, ch, num);
	}
	
	return TRUE;
    }
    
    if (CMD_IS("switch") && is_abbrev(buf, "cards")) {    /* switch */
	arg = one_argument(arg, buf);
	if (switch_cards(ch, atoi(buf), atoi(arg)))
	    send_to_char("Ok.\r\n", ch);
	else
	    send_to_char("Not done.\r\n", ch);
	
	return TRUE;
    }


    if (CMD_IS("drop") && is_abbrev(buf, "cards")) {     /* drop */
	
	if (!arg) {
	    send_to_char("Have to specify a number!\r\n", ch);
	} else {
	    if (ch->master)
		victim = ch->master;
	    else
		victim = ch;
	    
	    one_argument(arg, buf);

	    if (!str_cmp(buf, "all")) {
		cards = ch->specials.cards_in_hand;
		num = count_deck(cards);
		ch->specials.cards_in_hand = 0;
	    } else { 
		cards = drop_cards(ch, arg);
		num = count_deck(cards);
	    }
	    
	    if (num > 0) {
		return_all_cards(victim, cards);
		sprintf(buf, "$n drops %d card%s.", num, (num == 1)?"":"s");
		act(buf, FALSE, ch, 0, 0, TO_ROOM);
		sprintf(buf,"You drop %d card%s.\r\n", num, (num == 1)?"":"s");
		send_to_char(buf, ch);
	    } else {
		send_to_char("No cards dropped.\r\n", ch);
		
	    }
	}
	return TRUE;
    }

    if (CMD_IS("sort") && is_abbrev(buf, "cards")) {    /* sort */
	if (!ch->specials.cards_in_hand) {
	    send_to_char("You don't have any cards in hand!\r\n", ch);
	} else {
	    send_to_char("You sort your hand.\r\n", ch);
	    act("$n sorts $s cards.", FALSE, ch, 0, 0, TO_ROOM);
	    sort_cards_in_hand(ch);
	}

	return TRUE;
    }

    return FALSE;
}
    
    






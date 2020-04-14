/*
************************************************************************
 *   File: act.comm.c                                    Part of EliteMUD  *
 *  Usage: Player-level communication commands                             *
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
#include "history.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "functions.h"
#include "ignore.h"

/* extern variables */
extern struct room_data **world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;


/* Function to include racial speaking 'sludder' to a sentence string */
void add_racial_sentence(struct char_data *ch, char *sent)
{
  char *ptr, *tmp, new[MAX_INPUT_LENGTH], add[16];
  int i = 0, len;

  if (IS_NPC(ch) || ch->specials.invis_level)
    return;

  if (GET_RACE(ch) == RACE_MINOTAUR)
    strcpy(add, " (grunt) ");
  else if (GET_RACE(ch) == RACE_DUCK)
    strcpy(add, " (kvack) ");
  else if (GET_RACE(ch) == RACE_RATMAN)
    strcpy(add, " (squeek) ");
  else if (GET_RACE(ch) == RACE_LIZARDMAN)
    strcpy(add, " (hiss) ");
  else if (GET_RACE(ch) == RACE_DRAGON)    /* Added new race sounds-DT */
    strcpy(add, " (hiss) ");
  else if (GET_RACE(ch) == RACE_DRACONIAN)
    strcpy(add, " (hiss) ");
  else if (GET_RACE(ch) == RACE_TROLL)
    strcpy(add, " (grunt) ");
  else if (GET_RACE(ch) == RACE_WEREWOLF)
    strcpy(add, " (howl) ");
  else if (GET_RACE(ch) == RACE_FELINE)
    strcpy(add, " (purr) ");
  else
    return;

  *new = '\0';
  ptr = sent;
  tmp = new;

  while (*ptr != '\0' && i < 240) {
    if (*ptr == ' ' && !number(0, 5)) {
      strcpy(tmp, add);
      len = strlen(add)-1;
      tmp += len;
      i += len;
    } else
      *tmp = *ptr;

    ptr++;
    tmp++;
    i++;
  }
  *tmp = '\0';

  strcpy(sent, new);
}


ACMD(do_say)
{
  int i;
  char *last = NULL, say[16];
  struct char_data *vict;
  struct room_data *rm;

  skip_spaces(&argument);

  for (i = 0; *(argument + i) != '\0'; i++)
    if (*(argument + i) != ' ')
      last = (argument + i);

  if (!*argument)  {
	print_history(ch, CHAN_SAY);
  }
  else if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even speak.\r\n", ch);
    return;
  }
  else {
    switch (*last) {
    case '?': strcpy(say, "ask"); break;
    case '!': strcpy(say, "exclaim"); break;
    case '.': strcpy(say, "state"); break;
    case '"': strcpy(say, "quote"); break;
    default: strcpy(say, "say"); break;
    }

    if (!PRF_FLAGGED(ch, PRF_NOREPEAT)) {
      sprintf(buf, "#yYou %s '%s#y'#N", say, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    } else
      send_to_char("Ok.\r\n", ch);

    chan_history(argument, ch, NULL, 0, CHAN_SAY);
    add_racial_sentence(ch, argument);

    sprintf(buf, "#y$n %ss '%s#y'#N", say, argument);

    MOBTrigger = FALSE;

    rm = world[IN_ROOM(ch)];

    for (vict=rm->people;vict;vict = vict->next_in_room)
	  if (!PLR_FLAGGED(vict, PLR_WRITING) &&
	      !is_ignoring(vict, GET_NAME(ch)))
        act(buf, FALSE, ch, 0, vict, TO_VICT);

    mprog_speech_trigger(argument, ch);
  }
}


ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("YEAH, sure. Try to be in a group first\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even talk to your group.\r\n", ch);
    return;
  }

  if (!*argument)
    print_history(ch, CHAN_GROUP);
  else if (IS_SET(world[ch->in_room]->room_flags, SILENT)) {
    send_to_char("The walls seem to absorb your words.\r\n", ch);
    return;
  }
  else {
    if (!PRF_FLAGGED(ch, PRF_NOREPEAT)) {
      sprintf(buf, "#gYou tell your group '%s#g'#N\r\n", argument);
      send_to_char(buf, ch);
    } else
      send_to_char("Ok.\r\n", ch);

    if (ch->master)
      k = ch->master;
    else
      k = ch;

    chan_history(argument, ch, NULL, 0, CHAN_GROUP);

    add_racial_sentence(ch, argument);
    sprintf(buf, "#g%s tells your group '%s#g'#N\r\n", GET_NAME(ch), argument);
    if (IS_AFFECTED(k, AFF_GROUP) && (k != ch))
      if (!IS_SET(world[k->in_room]->room_flags, SILENT) &&
          !is_ignoring(k, GET_NAME(ch)) &&
          !PLR_FLAGGED(k, PLR_WRITING))
        send_to_char(buf, k);
    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch))
        if (!IS_SET(world[f->follower->in_room]->room_flags, SILENT) &&
            !is_ignoring(f->follower, GET_NAME(ch)) &&
            !PLR_FLAGGED(f->follower, PLR_WRITING))
	  send_to_char(buf, f->follower);
  }
}

ACMD(do_pstatus)
{
  sprintf(argument, "< #g%d#G(%d)Hp #m%d#M(%d)Mn #b%d#B(%d)Mv#g >",
	  GET_HIT(ch), GET_MAX_HIT(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch));
  do_gsay(ch, argument, 0, 0);
}

void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  if (vict->specials.timer > 5)
    act("$N has been idle for more than 5 ticks and may not respond.", FALSE,
	ch, 0, vict, TO_CHAR);

  sprintf(buf, "§2b%s tells you '%s#b'#N", GET_NAME(ch), arg);

  chan_history(arg, ch, vict, 0, CHAN_TELL);

  CAP(buf+3);
  if (!is_ignoring(vict, GET_NAME(ch)))
  {
    act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
    GET_LAST_TELL(vict) = ch;
  }

  if (!PRF_FLAGGED(ch, PRF_NOREPEAT)) {
    sprintf(buf, "§2bYou tell %s '%s#b'#N", GET_NAME(vict), arg);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  } else
    send_to_char("Ok.\r\n", ch);
}


ACMD(do_tell)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
     print_history(ch, CHAN_TELL);
  else if(IS_NPC(ch) && !*buf)
     send_to_char("Mobs don't get a channel history.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even talk to other players.\r\n", ch);
    return;
  }
  else if (!(vict = get_player_vis(ch, buf)))
    send_to_char("No such player around.\r\n", ch);
  else if (ch == vict)
    send_to_char("A nasty smell strikes you as you tell yourself to shut up.\r\n", ch);
  else if (PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (IS_SET(world[ch->in_room]->room_flags, SILENT))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)	/* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now. Try again later.",
	FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PRF_FLAGGED(vict, PRF_NOTELL) || IS_SET(world[vict->in_room]->room_flags, SILENT))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else
    perform_tell(ch, vict, buf2);

}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  skip_spaces(&argument);

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even reply.\r\n", ch);
    return;
  }

  if (GET_LAST_TELL(ch) == NULL)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else {
    /* Make sure the person you're replying to is still playing by searching
     * for them.  Note, this will break in a big way if I ever implement some
     * scheme where it keeps a pool of char_data structures for reuse.
     */

    while (tch != NULL && tch != GET_LAST_TELL(ch))
      tch = tch->next;
    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else if (!IS_NPC(tch) && !tch->desc)        /* linkless */
      act("$E's linkless at the moment.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else if (PRF_FLAGGED(ch, PRF_NOTELL))
      send_to_char("You can't reply to other people while you have notell on.\r\n", ch);
    else if (IS_SET(world[ch->in_room]->room_flags, SILENT))
      send_to_char("The walls seem to absorb your words.\r\n", ch);
    else if (PLR_FLAGGED(tch, PLR_WRITING))
      act("$E's writing a message right now. Try again later.",
	     FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else if (PRF_FLAGGED(tch, PRF_NOTELL) || IS_SET(world[tch->in_room]->room_flags, SILENT))
      act("$E can't hear you.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else
      perform_tell(ch, GET_LAST_TELL(ch), argument);
  }
}


ACMD(do_whisper)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even whisper.\r\n", ch);
    return;
  }

  if (!*buf || !*buf2)
    print_history(ch, CHAN_WHISPER);
  else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char("No-one by that name here..\r\n", ch);
  else if (vict == ch) {
    act("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You can't seem to get your mouth close enough to your ear...\r\n", ch);
  }
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now. Try again later.",
	 FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else {
	if (!is_ignoring(vict, GET_NAME(ch)))
	{
	  sprintf(buf, "#c$n whispers to you, $-'%s#c'#N", buf2);
	  act(buf, FALSE, ch, 0, vict, TO_VICT);
    }
    send_to_char("Ok.\r\n", ch);
    act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    chan_history(buf2, ch, vict, 0, CHAN_WHISPER);
  }
}

ACMD(do_ask)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even ask.\r\n", ch);
    return;
  }

  if (!*buf || !*buf2)
    send_to_char("Who do you want to ask something.. and what??\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char("No-one by that name here..\r\n", ch);
  else if (vict == ch) {
    act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You think about it for a while...\r\n", ch);
  }
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now. Try again later.",
	 FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else {
    if (!is_ignoring(vict, GET_NAME(ch)))
	{
	  sprintf(buf, "#C$n asks you '%s#C'#N", buf2);
	  act(buf, FALSE, ch, 0, vict, TO_VICT);
    }
    send_to_char("Ok.\r\n", ch);
    act("$n asks $N a question.", FALSE, ch, 0, vict, TO_NOTVICT);
  }
}


#define MAX_NOTE_LENGTH 1000      /* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper = 0, *pen = 0;
  char	*papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername)  /* nothing was delivered */ {
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) /* there were two arguments */ {
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
  } else /* there was one arg.let's see what we can find */	 {
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */ {
      pen = paper;
      paper = 0;
    } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }

    /* one object was found. Now for the other one. */
    if (!ch->equipment[HOLD]) {
      sprintf(buf, "You can't write with a %s alone.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD])) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }

    if (pen)
      paper = ch->equipment[HOLD];
    else
      pen = ch->equipment[HOLD];
  }

  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  } else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
    /* we can write - hooray! */
    ch->desc->backstr = NULL;
    send_to_char("Write your note.  (/s saves /h for help)\r\n", ch);
    if (paper->action_description) {
      ch->desc->backstr = strdup(paper->action_description);
      send_to_char(paper->action_description, ch);
    }
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;

  if (!*argument) {
    send_to_char("Whom do you wish to page?\r\n", ch);
    return;
  }
  half_chop(argument, buf, buf2);
  if (!str_cmp(buf, "all")) {
    if (GET_LEVEL(ch) > LEVEL_IMMORT) {
      sprintf(buf, "\007\007#r*%s* %s#N\r\n", GET_NAME(ch), buf2);
      for (d = descriptor_list; d; d = d->next)
	if (!d->connected)
	  SEND_TO_Q(buf, d);
    } else
      send_to_char("You will never be godly enough to do that!\r\n", ch);
    return;
  }

  if ((vict = get_char_vis(ch, buf))) {
    sprintf(buf, "\007\007#r*%s* %s#N\r\n", GET_NAME(ch), buf2);
    send_to_char(buf, vict);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Ok.\r\n", ch);
    else
      send_to_char(buf, ch);
    return;
  } else
    send_to_char("There is no such person in the game!\r\n", ch);
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

void cut_colors(char *buf)
{
  char tmp, buf2[SMALL_BUFSIZE];
  int i, j, color_code;

  for (i=0,j=0;i<strlen(buf);i++)
  {
    if (buf[i] == '#')
    {
      tmp = buf[i+1];
      color_code = 0;

      switch (tmp)
      {
        case 'r' : color_code = 1; break;
        case 'R' : color_code = 1; break;
        case 'b' : color_code = 1; break;
        case 'B' : color_code = 1; break;
        case 'c' : color_code = 1; break;
        case 'C' : color_code = 1; break;
        case 'y' : color_code = 1; break;
        case 'Y' : color_code = 1; break;
        case 'w' : color_code = 1; break;
        case 'W' : color_code = 1; break;
        case 'g' : color_code = 1; break;
        case 'G' : color_code = 1; break;
        case 'm' : color_code = 1; break;
        case 'M' : color_code = 1; break;
        case 'e' : color_code = 1; break;
        case 'N' : color_code = 1; break;
        case ':' : color_code = 2; break;
        case '#' : color_code =-1; break;
        default  : color_code = 0; break;
      }

      if (color_code > 0)
        i += color_code;
      else
        if (color_code < 0)
        {
          buf2[(j++)] = buf[(i++)];
          buf2[(j++)] = buf[i];
        }
        else
          buf2[(j++)] = buf[i];
    }
    else
      buf2[(j++)] = buf[i];
  }

  buf2[j] = '\0';

  strcpy(buf, buf2);
}

extern int	level_can_chat;
extern int	holler_move_cost;
extern sbyte    channel_allowed[];

ACMD(do_gen_com)
{
  struct descriptor_data *i;
  char	name[SMALL_BUFSIZE];
  char	colon[24];

  static int	channels[] = {
    PRF_NONEWBIE,
    PRF_NOCHAT, /* yell */
    PRF_NOGOSS,
    PRF_NOAUCT,
    PRF_NOCHAT,
    PLR_NOPKSAY,
    PRF_NOGRAT
  };

  static int find_chan[] = {
      CHAN_NEWBIE,
      CHAN_YELL,
      CHAN_GOSSIP,
      CHAN_AUCTION,
      CHAN_CHAT,
      CHAN_PKSAY,
      CHAN_NONE
  };

  static char	*com_msgs[][4] =  {
    { "You cannot newbie!!\r\n",
	"newbie",
	"You aren't on the newbie channel!\r\n",
	"§2w" },
    { "You cannot yell!!\r\n",
	"yell",
	"Toggle chat channel ON if you want to yell!\r\n",
	"§2c" },
    { "You cannot gossip!!\r\n",
	"gossip",
	"You aren't even on the channel!\r\n",
	"§2Y" },
    { "You cannot auction!!\r\n",
	"auction",
	"You aren't even on the channel!\r\n",
	"§2M" },
    { "You cannot chat!\r\n",
	"chat",
	"You aren't even on the chat channel!\r\n",
	"§2G" },
	{"You cannot use the pkok channel!\r\n",
	"pksay",
	"You aren't even on the pkok channel!\r\n",
	"§2r" },
	{ "You cannot congrat!\r\n",
    "grat",
    "You aren't on the grat channel!\r\n",
    "§2C" }
  };

  /* Check if channel is disabled and if so block use - Bod */
  if (!channel_allowed[subcmd]) {
    sprintf(buf1, " No use! You can't %s the channel has been disabled.\r\n",
            com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }

  /* Block Pets from abusing the Comm channels - from Circle 3.0 code */
  if (!CMD_IS("yell") && !ch->desc) return;

  if (IS_SET(world[ch->in_room]->room_flags, SILENT)){
    sprintf(buf1, " No use! You are in a silent room so you can't %s.\r\n",
	    com_msgs[subcmd][1]);
    send_to_char(buf1,ch);
    return;
  }

  if ((*argument) && PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }

  if ((GET_LEVEL(ch) < level_can_chat) && (REMORT(ch)<1) &&
      (subcmd != SCMD_NEWBIEC)) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
	    level_can_chat, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }

  if (subcmd == SCMD_PKSAY) {
    if (PLR_FLAGGED(ch, channels[subcmd])) {
      send_to_char(com_msgs[subcmd][2], ch);
      return;
    }
  }
  else if (PRF_FLAGGED(ch, channels[subcmd])) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }

  skip_spaces(&argument);

  if (!(*argument)) {
    if(IS_NPC(ch)) {
	send_to_char("Mobs don't get a channel history.\r\n", ch);
	return;
    }

    print_history(ch, find_chan[subcmd]);
    return;
  }

  if (subcmd == SCMD_YELL) { /* Added to allow move point cost for yelling */
    if (GET_MOVE(ch) < holler_move_cost) {
      send_to_char("You're too exhausted to yell.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= holler_move_cost;
  }

  /* Strip Colour Codes from Chat and Auction Channels */
  if (subcmd == SCMD_CHAT || subcmd == SCMD_AUCTION)
    cut_colors(argument);
 

  strcpy(colon, com_msgs[subcmd][3]);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Ok.\r\n", ch);
  else {
    sprintf(buf1, "%sYou %s, '%s%s'#N\r\n", colon, com_msgs[subcmd][1],
	      argument, colon);
   act(buf1, FALSE, ch, 0, argument, TO_CHAR | TO_SLEEP);
  }

  chan_history(argument, ch, NULL, 0, find_chan[subcmd]);

  strcpy(name, GET_NAME(ch));
  *name = UPPER(*name);


  add_racial_sentence(ch, argument);
  sprintf(buf1, "%s%s %ss, '%s%s'#N", colon, name, com_msgs[subcmd][1], argument, colon);
  sprintf(buf2, "%sSomeone %ss, '%s%s'#N", colon, com_msgs[subcmd][1], argument, colon);

  for (i = descriptor_list; i; i = i->next) {
    if (subcmd == SCMD_PKSAY) {
      if (!i->connected && i != ch->desc &&
        !PLR_FLAGGED(i->character, channels[subcmd]) &&
        !is_ignoring(i->character, GET_NAME(ch)) &&
        !PLR_FLAGGED(i->character, PLR_WRITING) && !IS_SET(world[i->character->in_room]->room_flags, SILENT)) {

      if (CAN_SEE(i->character, ch))
        act(buf1, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      else
        act(buf2, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      }
    }
    else {
      if (!i->connected && i != ch->desc &&
	!PRF_FLAGGED(i->character, channels[subcmd]) &&
	!is_ignoring(i->character, GET_NAME(ch)) &&
	!PLR_FLAGGED(i->character, PLR_WRITING) && !IS_SET(world[i->character->in_room]->room_flags, SILENT)) {

	 if (subcmd == SCMD_YELL &&
	     ((world[ch->in_room]->zone != world[i->character->in_room]->zone) ||
	      GET_POS(i->character) < POS_RESTING))
	      continue;

      if (CAN_SEE(i->character, ch))
	act(buf1, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      else
	act(buf2, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      }
    }
  }
}


ACMD(do_qcomm)
{
  struct descriptor_data *i;

  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char("You aren't even part of the quest!\r\n", ch);
    return;
  }

  if ((*argument) && PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You can't Quest say!\r\n", ch);
    return;
  }

  if ((GET_LEVEL(ch) < level_can_chat) && (REMORT(ch)<1)) {
    sprintf(buf1, "You must be at least level %d before you can quest say.\r\n",
	    level_can_chat);
    send_to_char(buf1, ch);
    return;
  }

  if (IS_SET(world[IN_ROOM(ch)]->room_flags, SILENT)){
    send_to_char( "You are in a silent room so you can't quest say.\r\n" ,ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument) {
    print_history(ch, CHAN_QUEST);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Ok.\r\n", ch);
    else {
      if (subcmd == SCMD_QSAY)
	sprintf(buf, "#MYou quest-say, '%s#M'#N", argument);
      else
	strcpy(buf, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR | TO_SLEEP);
    }

    if (subcmd == SCMD_QSAY)
      sprintf(buf, "#M$n quest-says, '%s#M'#N", argument);
    else
      strcpy(buf, argument);

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i != ch->desc &&
	  PRF_FLAGGED(i->character, PRF_QUEST) &&
	  !PLR_FLAGGED(i->character, PLR_WRITING) &&
      !is_ignoring(i->character, GET_NAME(ch)) &&
	  !IS_SET(world[IN_ROOM(i->character)!=NOWHERE?IN_ROOM(i->character):0]->room_flags, SILENT))
	act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
    chan_history(argument, ch, NULL, 0, CHAN_QUEST);
  }
}


ACMD(do_pray)
{

  struct char_data *tch = character_list;

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even pray.\r\n", ch);
    return;
  }

  if ((GET_LEVEL(ch) < level_can_chat) && (REMORT(ch)<1)) {
    sprintf(buf1, "You must be at least level %d before you can pray.\r\n",
	    level_can_chat);
    send_to_char(buf1, ch);
    return;
  }

  if (IS_SET(world[ch->in_room]->room_flags, SILENT)){
    send_to_char( "You are in a silent room so you can't pray.\r\n" ,ch);
    return;
  }

  skip_spaces(&argument);

  sprintf(buf, "%s prays: %s#G", GET_NAME(ch), argument);

  mudlog(buf, NRM, LEVEL_DEITY, FALSE);

  act("$n prays to the powers that be.", TRUE, ch, NULL, NULL, TO_ROOM);

  /* If the player's god is on, send them a copy of the prayer -- DT */

  while (tch != NULL && WORSHIPS (ch) != GET_IDNUM (tch))
     tch = tch->next;
  if (tch != NULL) {
     strcat (buf, "\r\n");
     send_to_char (buf, tch);
  }
}


void info_line(struct char_data *ch, struct char_data *victim, char *message)
{
  struct descriptor_data *i;


  /* Check if PK Say channel is disabled and if so return */
  if (!channel_allowed[SCMD_PKSAY])
    return;

  sprintf(buf, "#NInfo: %s\r\n", message);

  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && i != ch->desc && i != victim->desc &&
      !PLR_FLAGGED(i->character, PLR_NOPKSAY) &&
      !PLR_FLAGGED(i->character, PLR_WRITING) && 
      !IS_SET(world[i->character->in_room]->room_flags, SILENT)) {

        act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
    }
  }
}



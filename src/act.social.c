/* ************************************************************************
*   File: act.social.c                                  Part of EliteMUD  *
*  Usage: Functions to handle socials                                     *
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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "history.h"
#include "functions.h"
#include "ignore.h"

/* extern variables */
extern struct room_data **world;
extern struct descriptor_data *descriptor_list;
extern struct clan_data *clan_list;
extern int      top_of_clan;
extern char *clan_ranks[][3];

/* extern functions */
void	parse_string(char *input, char *output, struct char_data *ch1,
struct char_data *ch2, struct char_data *to);
int	action(int cmd);
char	*fread_action(FILE *fl, int nr);

/* local globals */
static int	list_top = -1;

struct social_messg {
   int	act_nr;
   int	hide;
   int	min_victim_position; /* Position of victim */

   /* No argument was supplied */
   char	*char_no_arg;
   char	*others_no_arg;

   /* An argument was there, and a victim was found */
   char	*char_found;		/* if NULL, read no further, ignore args */
   char	*others_found;
   char	*vict_found;

   /* An argument was there, but no victim was found */
   char	*not_found;

   /* The victim turned out to be the character */
   char	*char_auto;
   char	*others_auto;
} *soc_mess_list = 0;


char	*fread_action(FILE *fl, int nr)
{
   char	buf[MAX_STRING_LENGTH], *rslt;

   fgets(buf, MAX_STRING_LENGTH, fl);
   if (feof(fl)) {
      sprintf(buf, "SYSERR: fread_action - unexpected EOF near action #%d", nr);
      log(buf);
      exit(0);
   }

   if (*buf == '#')
      return(0);
   else {
      *(buf + strlen(buf) - 1) = '\0';
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
      return(rslt);
   }
}


int	find_action(int cmd)
{
   int	bot, top, mid;

   bot = 0;
   top = list_top;

   if (top < 0)
      return(-1);

   for (; ; ) {
      mid = (bot + top) / 2;

      if (soc_mess_list[mid].act_nr == cmd)
	 return(mid);
      if (bot >= top)
	 return(-1);

      if (soc_mess_list[mid].act_nr > cmd)
	 top = --mid;
      else
	 bot = ++mid;
   }
}





ACMD(do_action)
{
   int	act_nr;
   struct social_messg *action;
   struct char_data *vict;

   if ((act_nr = find_action(cmd)) < 0) {
      send_to_char("That action is not supported.\r\n", ch);
      return;
   }

   if (PLR_FLAGGED(ch, PLR_MUTE)) {
     send_to_char("You cannot even use socials.\r\n", ch);
     return;
   }

   action = &soc_mess_list[act_nr];

   if (action->char_found)
      one_argument(argument, buf);
   else
      *buf = '\0';

   if (!*buf) {
      act(action->char_no_arg, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
      return;
   }


   if (!(vict = get_char_room_vis(ch, buf))) {
       act(action->not_found, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
   } else if (vict == ch) {
       act(action->char_auto, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
       act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
   } else {
      if (GET_POS(vict) < action->min_victim_position) {
	 act("$N is not in a proper position for that.", FALSE, ch, 0, vict, TO_CHAR);
      } else {
	 act(action->char_found, 0, ch, 0, vict, TO_CHAR);
	 act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);
	 act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
      }
   }
}


void boot_social_messages(void)
{
  FILE *fl;
  int nr, i, hide, min_pos, curr_soc = -1;
  char next_soc[100];
  struct social_messg temp;
  extern struct command_info cmd_info[];

  /* open social file */
  if (!(fl = fopen(SOCMESS_FILE, "r"))) {
    sprintf(buf, "Can't open socials file '%s'", SOCMESS_FILE);
    perror(buf);
    exit(1);
  }
  /* count socials & allocate space */
  for (nr = 0; *cmd_info[nr].command != '\n'; nr++)
    if (cmd_info[nr].command_pointer == do_action)
      list_top++;

  CREATE(soc_mess_list, struct social_messg, list_top + 1);

  /* now read 'em */
  for (;;) {
    fscanf(fl, " %s ", next_soc);
    if (*next_soc == '$')
      break;
    if ((nr = find_command(next_soc)) < 0) {
      sprintf(buf, "Unknown social '%s' in social file", next_soc);
      log(buf);
    }
    if (fscanf(fl, " %d %d \n", &hide, &min_pos) != 2) {
      fprintf(stderr, "Format error in social file near social '%s'\n",
	      next_soc);
      exit(1);
    }
    /* read the stuff */
    curr_soc++;
    soc_mess_list[curr_soc].act_nr = nr;
    soc_mess_list[curr_soc].hide = hide;
    soc_mess_list[curr_soc].min_victim_position = min_pos;

    soc_mess_list[curr_soc].char_no_arg = fread_action(fl, nr);
    soc_mess_list[curr_soc].others_no_arg = fread_action(fl, nr);
    soc_mess_list[curr_soc].char_found = fread_action(fl, nr);

    /* if no char_found, the rest is to be ignored */
    if (!soc_mess_list[curr_soc].char_found)
      continue;

    soc_mess_list[curr_soc].others_found = fread_action(fl, nr);
    soc_mess_list[curr_soc].vict_found = fread_action(fl, nr);
    soc_mess_list[curr_soc].not_found = fread_action(fl, nr);
    soc_mess_list[curr_soc].char_auto = fread_action(fl, nr);
    soc_mess_list[curr_soc].others_auto = fread_action(fl, nr);
  }

  /* close file & set top */
  fclose(fl);
  list_top = curr_soc;

  /* now, sort 'em */
  for (curr_soc = 0; curr_soc < list_top; curr_soc++) {
    min_pos = curr_soc;
    for (i = curr_soc + 1; i <= list_top; i++)
      if (soc_mess_list[i].act_nr < soc_mess_list[min_pos].act_nr)
	min_pos = i;
    if (curr_soc != min_pos) {
      temp = soc_mess_list[curr_soc];
      soc_mess_list[curr_soc] = soc_mess_list[min_pos];
      soc_mess_list[min_pos] = temp;
    }
  }
}




ACMD(do_insult)
{
   struct char_data *victim;

   one_argument(argument, arg);

   if (PLR_FLAGGED(ch, PLR_MUTE)) {
     send_to_char("You cannot even insult anyone.\r\n", ch);
     return;
   }

   if (*arg) {
      if (!(victim = get_char_room_vis(ch, arg))) {
	 send_to_char("Can't hear you!\r\n", ch);
      } else {
	 if (victim != ch) {
	    sprintf(buf, "You insult %s.\r\n", GET_NAME(victim) );
	    send_to_char(buf, ch);

	    switch (circle_random() % 4) {
	    case 0 :
	        {
		  if (GET_SEX(ch) == SEX_MALE) {
		     if (GET_SEX(victim) == SEX_MALE)
			act(
			    "$n accuses you of fighting like a woman!", FALSE,
			    ch, 0, victim, TO_VICT);
		     else
			act("$n says that women can't fight.",
			    FALSE, ch, 0, victim, TO_VICT);
		  } else { /* Ch == Woman */
		     if (GET_SEX(victim) == SEX_MALE)
			act("$n accuses you of having the smallest.... (brain?)",
			     			     								FALSE, ch, 0, victim, TO_VICT );
		     else
			act("$n tells you that you'd loose a beautycontest against a troll.",
			     			     								FALSE, ch, 0, victim, TO_VICT );
		  }
	       }
	       break;
	    case 1 :
	        {
		  act("$n calls your mother a bitch!",
		      FALSE, ch, 0, victim, TO_VICT );
	       }
	       break;
	    case 2 :
	    {
		if(GET_SEX(ch) == SEX_FEMALE)
		    act("&n thinks your should start behaving as the man you obviously are", FALSE, ch, 0, victim, TO_VICT);
	    }
	    default :
	        {
		  act("$n tells you to get lost!", FALSE, ch, 0, victim, TO_VICT);
	       }
	       break;
	    } /* end switch */

	    act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
	 } else { /* ch == victim */
	    send_to_char("You feel insulted.\r\n", ch);
	 }
      }
   } else
      send_to_char("I'm sure you don't want to insult *everybody*...\r\n", ch);
}


/* ****************************************************************************
*  Elite Clan System - ECS (C) 1994 Mr.Wang at RIT - This is unique for Elite *
**************************************************************************** */

void
show_clan_info(struct char_data *ch, int vnum, int info_check)
{
  int j, i;

  if (vnum >= 0) {
    for (j = vnum, i = 0; clan_list[i].vnum != j && i <= top_of_clan; i++);
    if (i <= top_of_clan) {
      *buf2 = 0;
      for (j = 10; j >= 6; j--) {
	if(info_check)
	if (clan_list[i].roster[j]) {
	  sprintf(buf2, "%s%8s§r: %s%s§N\r\n", buf2,
		  clan_list[i].ranknames[j][0],
		  (j>=8?"§G":"§C"),
		  clan_list[i].roster[j]);
	}
      }
      if(info_check)	 {
      sprintf(buf, "§y%3d.§N %s\r\n§r Gods: §w%d   §rRemorts: §w%d   §rMortals: §w%d   §rPower: §w%d\r\n§r Average level: §w%d   §rAverage power: §w%d   §rTreasure: §w%ldM§N\r\n%s§r    Info:§N\r\n%s§r\r\n",
	      clan_list[i].vnum,
	      clan_list[i].name,
	      clan_list[i].gods,
	      clan_list[i].remorts,
              clan_list[i].mortals,
	      clan_list[i].power,
              clan_list[i].members ? clan_list[i].level / clan_list[i].members : 0,
              clan_list[i].members ? clan_list[i].power / clan_list[i].members : 0,
	      clan_list[i].wealth/1000,
	      buf2,
	      clan_list[i].info);
      } else {
      sprintf(buf, "%s\r\n", clan_list[i].symbol);
      }
      page_string(ch->desc, buf, TRUE);
    } else {
      send_to_char("No clan with that number.\r\n", ch);
      return;
    }
  } else {
    for (*buf = '\0', i = 0; i <= top_of_clan; i++)
     if(info_check) {
     sprintf(buf, "%s %3d. %s\r\n",
	      buf, clan_list[i].vnum, clan_list[i].name);
    page_string(ch->desc, buf, TRUE);
    }
  }
}


void show_clan_roster(struct char_data *ch, int vnum)
{
  int i, j;
  *buf = 0;

  if (vnum == 100){
    send_to_char("Roster unavailable for that clan.\r\n", ch);
    return;
  }


  for (j = vnum, i = 0; clan_list[i].vnum != j && i <= top_of_clan; i++);
  if (i <= top_of_clan) {
    for (j = 10; j >= 0; j--) {
      if (clan_list[i].roster[j]) {
	sprintf(buf, "%s%8s#r: %s%s#N\r\r\n\n", buf,
		clan_list[i].ranknames[j][0],
		(j>=8?"#G":"#C"),
		clan_list[i].roster[j]);
      }
    }
  } else {
    send_to_char("That clan is a figment of your imagination.\r\n", ch);
    return;
  }

  send_to_char(buf, ch);
}

void clan_update(void)
{
  FILE *clan_f;
  int i = 0;
  struct descriptor_data *d;

  for(d = descriptor_list; d; d = d->next) {
    if((!d->connected) &&
       (d->character->specials.timer < 2) &&
       (CLAN_LEVEL(d->character) > 1) &&
       ((GET_LEVEL(d->character) < LEVEL_DEITY) ||
	GET_INVIS_LEV(d->character) == 0))  {

      clan_list[CLAN(d->character)].on_power += 1;
      clan_list[CLAN(d->character)].on_power_rec += 1;
    }
  }

  /* Update File Records */

  if (!(clan_f = fopen(CLAN_FILE, "w+"))) {
    log("SYSERR: Unable to open/create Clan Power File");
    return;
  }

  for (i = 0; i <= top_of_clan; i++) {
      fprintf(clan_f, "#%d A%ld\n", clan_list[i].vnum,
	      clan_list[i].on_power_rec);
  }

  fprintf(clan_f, "#99999\n");

  fclose(clan_f);
}

void show_clan_power(struct char_data *ch)
{
     extern int top_of_clan;
     extern int newbie_clan;
     struct descriptor_data *d;
     int top = (top_of_clan + 1);
     int mortal[top];
     int immortal[top];
     int remort[top];
     int i = 0;
     long j = 0, k = 0;;

     *buf = '\0';
     *buf2 = '\0';
     *buf1 = '\0';
     for (i = 0; i <= top_of_clan; i++) {
	  mortal[i] = 0;
	  immortal[i] = 0;
	  remort[i] = 0;
	  if (clan_list[i].vnum != newbie_clan) {
	       j += clan_list[i].on_power;
	       k += clan_list[i].on_level;
	  }
     }

     sprintf(buf, "Elite Mud Online Clan Statistics:\r\n");
     strcat(buf, "#y(M)  #g(R)  #r(I)       #bActivity  #cLevels Gained  #wClan#N\r\n");
     strcat(buf, "-=-  =-=  -=-  =-=-=-=-=-=-=  -=-=-=-=-=-=-  =-=-\r\n");

     for (d = descriptor_list; d; d = d->next) {
	  if (d->connected == CON_PLYNG) {
	       if ((CLAN(d->character) >= 0) &&
		   (CLAN_LEVEL(d->character) > 1)) {

		    if (GET_LEVEL(ch) < GET_INVIS_LEV(d->character))
			continue;

                    if (GET_LEVEL(ch) < LEVEL_DEITY && PLR_FLAGGED(d->character, PLR_NOWHO))
                        continue;

		    if (GET_LEVEL(d->character) >= LEVEL_DEITY)
			 immortal[CLAN(d->character)]++;
		    else if (REMORT(d->character) > 0)
			 remort[CLAN(d->character)]++;
		    else
			 mortal[CLAN(d->character)]++;
	       }
	  }
     }

     for (i = 0; i <= top_of_clan; i++) {
	  sprintf(buf2, "%3d  %3d  %3d %7ld (%3ld%%) %7ld (%3ld%%)  %3d. %s\r\n",
		  mortal[i],
		  remort[i],
		  immortal[i],
		  clan_list[i].on_power,
		  j ? ( 100 * clan_list[i].on_power / j) : 0,
		  clan_list[i].on_level,
		  k ? (100 * clan_list[i].on_level / k) : 0,
		  clan_list[i].vnum,
		  clan_list[i].name);

	  if (clan_list[i].vnum != newbie_clan) {
	       strcat(buf, buf2);
	  } else {
	       sprintf(buf1, "%3d  %3d  %3d %7ld ( N/A) %7ld ( N/A)  %3d. %s\r\n",
		    mortal[i],
		    remort[i],
		    immortal[i],
		    clan_list[i].on_power,
		    clan_list[i].on_level,
		    clan_list[i].vnum,
		    clan_list[i].name);
	  }
     }

     strcat(buf, "\r\n");
     strcat(buf, buf1);
     page_string(ch->desc, buf, TRUE);
}

#define CLANLEVEL_APPLYING  1
#define CLANLEVEL_NEW       2
#define CLANLEVEL_MEMBER    3
#define CLANLEVEL_SOLDIER   4
#define CLANLEVEL_ADEPT     5
#define CLANLEVEL_ADVISOR   6
#define CLANLEVEL_RULER     7
#define CLANLEVEL_QUEEN     8
#define CLANLEVEL_DEITY     9
#define CLANLEVEL_OVERGOD  10

#define NONE    0
#define VICTIM  1
#define NUMBER  2
#define STRING  3

void send_to_clan(struct char_data *ch, char *arg)
{
  struct descriptor_data *d;

  if (CLAN(ch) < 0) return;

  for (d = descriptor_list; d ; d = d->next) {
    if (!d->connected)
      if ((CLAN(d->character) == CLAN(ch))  && (IN_ROOM(d->character) >= 0) &&
         (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING)) && !IS_NPC(d->character))
        act(arg, FALSE, ch, 0, d->character, TO_VICT | TO_SLEEP);
  }

  return;
}


ACMD(do_clantell);

int clan_power_check(struct char_data *ch, int cmd)
{

     switch (cmd)
     {
     case  5 : /* enlist */
	  if (IS_SET(clan_list[CLAN(ch)].pwr_enlist, (1 << CLAN_LEVEL(ch))))
	      return 1;
	      else
	      return 0;
	  break;
     case  6 : /* raise */
	  if (IS_SET(clan_list[CLAN(ch)].pwr_raise, (1 << CLAN_LEVEL(ch))))
	      return 1;
	      else
	      return 0;
	  break;
     case  7 : /* demote */
	  if (IS_SET(clan_list[CLAN(ch)].pwr_demote, (1 << CLAN_LEVEL(ch))))
	      return 1;
	      else
	      return 0;
	  break;
     case  8 : /* expel */
	  if (IS_SET(clan_list[CLAN(ch)].pwr_expel, (1 << CLAN_LEVEL(ch))))
	      return 1;
	      else
	      return 0;
	  break;
     default : /* others have no check */
	  return 1;
     }

     return 0;
}


ACMD(do_clan)
{
  int i, l, num = 0;
  struct char_data *vict = NULL;
  char buf[SMALL_BUFSIZE];
  bool found = FALSE, file = FALSE;
  int player_i;
  struct char_file_u tmp_store;
  extern FILE *player_fl;
  extern sh_int mortal_start_room;
  extern sh_int frozen_start_room;

  ACMD(do_help); ACMD(do_gen_tog); ACMD(do_who);

  struct clan_struct {
    char *cmd;
    ubyte level;
    char type;
  } clancmds[] =
    {
      { "apply" ,                 0 , NUMBER},
      { "info"  ,                 0 , NUMBER},
      { "help"  ,                 0 , STRING},
      { "title" , CLANLEVEL_NEW     , NONE},
      { "resign", CLANLEVEL_NEW     , STRING},
      { "enlist", CLANLEVEL_NEW     , VICTIM}, /* 5 */
      { "raise" , CLANLEVEL_NEW     , VICTIM}, /* 6 */
      { "demote", CLANLEVEL_NEW     , VICTIM}, /* 7 */
      { "expel" , CLANLEVEL_NEW     , VICTIM}, /* 8 */
      { "tell"  , CLANLEVEL_NEW     , STRING},
      { "who"   , CLANLEVEL_NEW     , NONE  },
      { "symbol",                 0 , NUMBER},
      { "roster",                 0 , NUMBER},
      { "power",                  0 , NONE},
      { "\n"    ,                 0 , NONE}
    };

  if (IS_NPC(ch)) {
    send_to_char("Sorry man, you cannot join a clan.\r\n", ch);
    return;
  }

  half_chop(argument, buf, buf2);

  if (!*buf) {
    send_to_char("You have to supply a subcommand:\r\n", ch);

    i = 0;
    l = 0;
    *buf = '\0';
    while (*(clancmds[l].cmd) != '\n') {
      if (CLAN_LEVEL(ch) >= clancmds[l].level && clan_power_check(ch, l)) {
	i++;
	sprintf(buf, "%s %-12s", buf, clancmds[l].cmd);
	if (!(i % 5))
	  strcat (buf, "\r\n");
      }
      l++;
    }
    if (i % 5)
      strcat (buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  for (l = 0; *(clancmds[l].cmd) != '\n'; l++)
    if (!strncmp(buf, clancmds[l].cmd, strlen(buf)))
      break;

  if (CLAN_LEVEL(ch) < clancmds[l].level || !clan_power_check(ch, l)) {
    send_to_char("No such subcommand!\r\n", ch);
    return;
  }

  if (clancmds[l].type == VICTIM) {
       if ((vict = get_player_vis_exact(ch, buf2))) found = TRUE;

       if (!found) {
	    CREATE(vict, struct char_data, 1);
	    clear_char(vict);

	    if ((player_i = load_char(buf2, &tmp_store)) > -1) {
		 store_to_char(&tmp_store, vict);
		 file = TRUE;
		 found = TRUE;
	    } else {
		 free(vict);
		 send_to_char("There is no such player.\r\n", ch);
		 return;
	    }
       }
  } else if (clancmds[l].type == NUMBER) {
    num = atoi(buf2);
  }

  switch (l) {
  case 0:
    if (CLAN(ch) >= 0 && CLAN_LEVEL(ch) > 1) {
      send_to_char("You are already in a clan.  Try resigning there first.\r\n", ch);
      return;
    }
    if ((i = real_clan(num)) < 0) {
      send_to_char("There is no such clan.\r\n", ch);
      return;
    }
    if ((GET_LEVEL(ch) < 10)) {
      send_to_char("You must be of at least level 10 to apply for membership.\r\n", ch);
      return;
    }
    /* Bodpoint */

    CLAN(ch) = i;
    CLAN_LEVEL(ch) = 1;
    sprintf(buf, "Ok you are now applying for membership in clan %s\r\n",
	    clan_list[i].name);
    send_to_char(buf, ch);
    send_to_clan(ch, "$n is applying for membership into your clan.#N\r\n");
    break;
  case 1:
    if (num)
      show_clan_info(ch, num, 1);
    else
      show_clan_info(ch, -1, 1);
    break;
  case 2:
    if (!buf2 || *buf2 == '\0')
      strcpy(buf, "clan tutorial");
    else
      sprintf(buf, "clan %s", buf2);
    do_help(ch, buf, 0, 0);
    break;
  case 3:
    do_gen_tog(ch, buf2, cmd, SCMD_NOCLANTIT);
    break;
  case 4:
    if (strcmp(buf2, "resign")) {
      send_to_char("WARNING: You are going to leave your clan.\r\nIf that is really what you want, type 'clan resign resign'", ch);
      return;
    } else {
      if(CLAN(ch) < 0){
        send_to_char("You cannot resign from a clan if you're not in one.\r\n", ch);
         return;
      }
      CLAN_LEVEL(ch) = 0;
      CLAN(ch) = -1;
    }
    send_to_clan(ch, "#m$n has resigned from your clan.\r\n");
    break;
  case 5:
    if (CLAN(vict) != CLAN(ch)) {
      act("$N must be applying to your clan.", TRUE, ch, 0, vict, TO_CHAR);
      if (file) free(vict);
      return;
    } else if (CLAN_LEVEL(vict) > 1) {
      act("$N seems to be in your clan already :)", TRUE, ch, 0, vict, TO_CHAR);
      if (file) free(vict);
      return;
    }

    if(CLAN(ch) < 0) {
      send_to_char("You have to be in a clan to be able to do that", ch);
      if (file) free(vict);
      return;
    }
    CLAN_LEVEL(vict) = 2;
    send_to_char("Ok.\r\n", ch);
    act("$n has enlisted you to $s clan.", TRUE, ch, 0, vict, TO_VICT);
    sprintf(buf, "#m%s has enlisted %s into %s.\r\n", GET_NAME(ch), GET_NAME(vict), CLAN_NAME(ch));
    send_to_clan(ch, buf);
    break;
  case 6:
  case 7:
  case 8:
    if (CLAN(vict) != CLAN(ch)) {
      act("$N isn't even in your clan.", TRUE, ch, 0, vict, TO_CHAR);
      if (file) free(vict);
      return;
    } else if (CLAN_LEVEL(vict) >= CLAN_LEVEL(ch)) {
      act("You don't have the authority to do that to $N.", TRUE, ch, 0, vict, TO_CHAR);
      if (file) free(vict);
      return;
    } else if(CLAN(ch) < 0) {
      send_to_char("You have to be in a clan to be able to do that.", ch);
      if (file) free(vict);
      return;
    }

    if (l == 6) {          /* raise */
      if ((CLAN_LEVEL(vict) >= CLAN_LEVEL(ch) - 1) || (CLAN_LEVEL(vict) == 1)) {
	send_to_char("You cannot do that.\r\n", ch);
	if (file) free(vict);
	return;
      }
      CLAN_LEVEL(vict)++;
      sprintf(buf, "$n has raised you to %s %s.",
	      CLAN_NAME(ch), CLAN_RANKNAME(vict));
      act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      sprintf(buf, "You have raised $N to %s %s.",
	      CLAN_NAME(ch), CLAN_RANKNAME(vict));
      act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
      sprintf(buf, "%s has raised %s to %s#N.\r\n", GET_NAME(ch), GET_NAME(vict), CLAN_RANKNAME(vict));
    } else if (l == 7) {  /* demote */
      if (CLAN_LEVEL(vict) > 2) {
	CLAN_LEVEL(vict)--;
	sprintf(buf, "$n has demoted you to %s %s.",
		CLAN_NAME(ch), CLAN_RANKNAME(vict));
	act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
	sprintf(buf, "You have demoted $N to %s %s.",
		CLAN_NAME(ch), CLAN_RANKNAME(vict));
	act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
        sprintf(buf, "%s has demoted %s to %s#N.\r\n", GET_NAME(ch), GET_NAME(vict), CLAN_RANKNAME(vict));
      } else {
	send_to_char("At lowest possible already.\r\n", ch);
        if (file) free(vict);
        return;
      }
    } else if (l == 8) {  /* expel */
      CLAN_LEVEL(vict) = 0;
      CLAN(vict) = -1;
      if (GET_LOADROOM(vict) != frozen_start_room)
	GET_LOADROOM(vict) = mortal_start_room;
      sprintf(buf, "$n has expelled you from %s.",
	      CLAN_NAME(ch));
      act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      sprintf(buf, "You have expelled $N from %s.",
	      CLAN_NAME(ch));
      act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
      sprintf(buf, "%s has expelled %s from %s#N.\r\n", GET_NAME(ch), GET_NAME(vict), CLAN_NAME(ch));
    }
    send_to_clan(ch, buf);
    break;
  case 9:
    do_clantell(ch, buf2, 0, 0);
    break;
  case 10:
    do_who(ch, "-k", 0, 0);
    break;
  case 11:
    if (num)
      show_clan_info(ch, num, 0);
    else
      show_clan_info(ch, -1, 0);
    break;
  case 12:
    if (num)
      show_clan_roster(ch, num);
    else
      show_clan_roster(ch, -1);
    break;
  case 13:
    show_clan_power(ch);
    break;
  default:
    send_to_char("No such subcommand.\r\n", ch);
  }

  if (file) {
       char_to_store(vict, &tmp_store, FALSE, FALSE);
       fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
       fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
       send_to_char("Saved in file.\r\n", ch);
       free(vict);
  }
}


ACMD(do_clantell)
{
  struct descriptor_data *d;
  int  level = CLANLEVEL_APPLYING;
  char buf1[SMALL_BUFSIZE], buf2[SMALL_BUFSIZE];

  if (IS_NPC(ch)) {
    send_to_char("Sorry - no mob clan here (yet).\r\n", ch);
    return;
  }

  if (IS_SET(world[ch->in_room]->room_flags, SILENT)){
    send_to_char( "You are in a silent room so you can't clan tell.\r\n" ,ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument) {
       print_history(ch, CHAN_CLAN);
       /* send_to_char("Usage: ctell <text> | #<level> <text>\r\n", ch); */
       return;
  }

  switch (*argument) {
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), CLANLEVEL_APPLYING);
      if (level > CLAN_LEVEL(ch)) {
	send_to_char("You can't clantell above your own level.\r\n", ch);
	return;
      }
    }
    break;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }

  if (CLAN(ch) < 0 || CLAN_LEVEL(ch) < CLANLEVEL_NEW) {
    send_to_char("You are not a member in any clan.\r\n", ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("What do you want to tell your clan?\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_NOCLANTELL)) {
    send_to_char("You are not even on the channel.\r\n", ch);
    return;
  }

  if (level > CLANLEVEL_APPLYING) {
    sprintf(buf1, "§m$n tells your clan <%d>, '%s§m'§N", level, argument);
    sprintf(buf2, "§mYou tell your clan <%d>, '%s§m'§N\r\n", level, argument);
  } else {
    sprintf(buf1, "§m$n tells your clan, '%s§m'§N", argument);
    sprintf(buf2, "§mYou tell your clan, '%s§m'§N\r\n", argument);
  }

  delete_doubledollar(argument);
  delete_doubledollar(buf2);

  chan_history(argument, ch, NULL, MAX(level, CLANLEVEL_NEW), CHAN_CLAN);

 for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) &&
	((d->character) != ch) &&
	(CLAN(d->character) == CLAN(ch)) &&
	(CLAN_LEVEL(d->character) >= MAX(level, CLANLEVEL_NEW)) &&
	(IN_ROOM(d->character) >= 0 && !IS_SET(world[IN_ROOM(d->character)]->room_flags, SILENT)) &&
	(!is_ignoring(d->character, GET_NAME(ch))) &&
	(!PLR_FLAGGED(d->character, PLR_WRITING|PLR_MAILING|PLR_NOCLANTELL))) {
      act(buf1, FALSE, ch, 0, d->character, TO_VICT | TO_SLEEP);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Ok.\r\n", ch);
  else
    send_to_char(buf2, ch);
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
  char *message[2*4];  /* CLASS_TYPES * 2 */
};

const	struct pose_table_type	pose_table[]	=
{
  {
    {
      "You sizzle with energy.",
      "$n sizzles with energy.",
      "You feel very holy.",
      "$n looks very holy.",
      "You perform a small card trick.",
      "$n performs a small card trick.",
      "You show your bulging muscles.",
      "$n shows $s bulging muscles."
    }
  },

  {
    {
      "You turn into a butterfly, then return to your normal shape.",
      "$n turns into a butterfly, then returns to $s normal shape.",
      "You nonchalantly turn wine into water.",
      "$n nonchalantly turns wine into water.",
      "You wiggle your ears alternately.",
      "$n wiggles $s ears alternately.",
      "You crack nuts between your fingers.",
      "$n cracks nuts between $s fingers."
    }
  },

  {
    {
      "Blue sparks fly from your fingers.",
      "Blue sparks fly from $n's fingers.",
      "A halo appears over your head.",
      "A halo appears over $n's head.",
      "You nimbly tie yourself into a knot.",
      "$n nimbly ties $mself into a knot.",
      "You grizzle your teeth and look mean.",
      "$n grizzles $s teeth and looks mean."
    }
  },

  {
    {
      "Little red lights dance in your eyes.",
      "Little red lights dance in $n's eyes.",
      "You recite words of wisdom.",
      "$n recites words of wisdom.",
      "You juggle with daggers, apples, and eyeballs.",
      "$n juggles with daggers, apples, and eyeballs.",
      "You hit your head, and your eyes roll.",
      "$n hits $s head, and $s eyes roll."
    }
  },

  {
    {
      "A slimy green monster appears before you and bows.",
      "A slimy green monster appears before $n and bows.",
      "Deep in prayer, you levitate.",
      "Deep in prayer, $n levitates.",
      "You steal the underwear off every person in the room.",
      "Your underwear is gone!  $n stole it!",
      "Crunch, crunch -- you munch a bottle.",
      "Crunch, crunch -- $n munches a bottle."
    }
  },

  {
    {
      "You turn everybody into a little pink elephant.",
      "You are turned into a little pink elephant by $n.",
      "An angel consults you.",
      "An angel consults $n.",
      "The dice roll ... and you win again.",
      "The dice roll ... and $n wins again.",
      "... 98, 99, 100 ... you do pushups.",
      "... 98, 99, 100 ... $n does pushups."
    }
  },

  {
    {
      "A small ball of light dances on your fingertips.",
      "A small ball of light dances on $n's fingertips.",
      "Your body glows with an unearthly light.",
      "$n's body glows with an unearthly light.",
      "You count the money in everyone's pockets.",
      "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
      "Arnold Schwarzenegger admires $n's physique."
    }
  },

  {
    {
      "Smoke and fumes leak from your nostrils.",
      "Smoke and fumes leak from $n's nostrils.",
      "A spot light hits you.",
      "A spot light hits $n.",
      "You balance a pocket knife on your tongue.",
      "$n balances a pocket knife on your tongue.",
      "Watch your feet, you are juggling granite boulders.",
      "Watch your feet, $n is juggling granite boulders."
    }
  },

  {
    {
      "The light flickers as you rap in magical languages.",
      "The light flickers as $n raps in magical languages.",
      "Everyone levitates as you pray.",
      "You levitate as $n prays.",
      "You produce a coin from everyone's ear.",
      "$n produces a coin from your ear.",
      "Oomph!  You squeeze water out of a granite boulder.",
      "Oomph!  $n squeezes water out of a granite boulder."
    }
  },

  {
    {
      "Your head disappears.",
      "$n's head disappears.",
      "A cool breeze refreshes you.",
      "A cool breeze refreshes $n.",
      "You step behind your shadow.",
      "$n steps behind $s shadow.",
      "You pick your teeth with a spear.",
      "$n picks $s teeth with a spear."
    }
  },

  {
    {
      "A fire elemental singes your hair.",
      "A fire elemental singes $n's hair.",
      "The sun pierces through the clouds to illuminate you.",
      "The sun pierces through the clouds to illuminate $n.",
      "Your eyes dance with greed.",
      "$n's eyes dance with greed.",
      "Everyone is swept off their foot by your hug.",
      "You are swept off your feet by $n's hug."
    }
  },

  {
    {
      "The sky changes color to match your eyes.",
      "The sky changes color to match $n's eyes.",
      "The ocean parts before you.",
      "The ocean parts before $n.",
      "You deftly steal everyone's weapon.",
      "$n deftly steals your weapon.",
      "Your karate chop splits a tree.",
      "$n's karate chop splits a tree."
    }
  },

  {
    {
      "The stones dance to your command.",
      "The stones dance to $n's command.",
      "A thunder cloud kneels to you.",
      "A thunder cloud kneels to $n.",
      "The Grey Mouser buys you a beer.",
      "The Grey Mouser buys $n a beer.",
      "A strap of your armor breaks over your mighty thews.",
      "A strap of $n's armor breaks over $s mighty thews."
    }
  },

  {
    {
      "The heavens and grass change colour as you smile.",
      "The heavens and grass change colour as $n smiles.",
      "The Burning Man speaks to you.",
      "The Burning Man speaks to $n.",
      "Everyone's pocket explodes with your fireworks.",
      "Your pocket explodes with $n's fireworks.",
      "A boulder cracks at your frown.",
      "A boulder cracks at $n's frown."
    }
  },

  {
    {
      "Everyone's clothes are transparent, and you are laughing.",
      "Your clothes are transparent, and $n is laughing.",
      "An eye in a pyramid winks at you.",
      "An eye in a pyramid winks at $n.",
      "Everyone discovers your dagger a centimeter from their eye.",
      "You discover $n's dagger a centimeter from your eye.",
      "Mercenaries arrive to do your bidding.",
      "Mercenaries arrive to do $n's bidding."
    }
  },

  {
    {
      "A black hole swallows you.",
      "A black hole swallows $n.",
      "Valentine Michael Smith offers you a glass of water.",
      "Valentine Michael Smith offers $n a glass of water.",
      "Where did you go?",
      "Where did $n go?",
      "Four matched Percherons bring in your chariot.",
      "Four matched Percherons bring in $n's chariot."
    }
  },

  {
    {
      "The world shimmers in time with your whistling.",
      "The world shimmers in time with $n's whistling.",
      "The great god Mota gives you a staff.",
      "The great god Mota gives $n a staff.",
      "Click.",
      "Click.",
      "Atlas asks you to relieve him.",
      "Atlas asks $n to relieve him."
    }
  }
};



ACMD(do_pose)
{
  int level, class, pose;

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even pose.\r\n", ch);
    return;
  }

  level = MIN(GET_LEVEL(ch), sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
  pose  = number(0, level);

  class = GET_CLASS(ch);

  if (IS_NPC(ch))
    class = CLASS_WARRIOR;
  else if (class >= CLASS_3MULTI)
    class = number(1, 4);
  else if (class >= CLASS_DUAL)
    class = GET_1CLASS(ch);

  class = (class - 1) % 4;

  act(pose_table[pose].message[2*class+0], FALSE, ch, NULL, NULL, TO_CHAR);
  act(pose_table[pose].message[2*class+1], FALSE, ch, NULL, NULL, TO_ROOM);

}







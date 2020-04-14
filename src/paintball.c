/* ************************************************************************
*  File: paintball.c (Version 1.1c)                     Part of EliteMUD  *
*  Usage: Paintball                                                       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  (C) 1998 Petya Vachranukunkiet                                         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
************************************************************************ */

/* Still TODO
implement mines
*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "functions.h"
#include "db.h"
#include "comm.h"

#define WARGAME_DISABLE -1
#define NO_WARGAME    0
#define START_WARGAME 1
#define RUN_WARGAME   2

#define MAX_TEAMS     4
const char *team_names[] = { "UNDEFINED",
			     "blue" , 
			     "red" ,
			     "yellow" ,
			     "green" };

char *which_team[] = { "UNDEFINED", 
		       "#bBlue Team#N",
		       "#rRed Team#N",
		       "#yYellow Team#N",
		       "#gGreen Team#N"};

int team_members[] = { 0, 0, 0, 0, 0 };

/* These can be modified in-game */
int start_ammo   = 5;
int hits_to_kill = 2;
int teams        = 0;


extern sh_int r_wargame_start_room; 
extern sh_int r_mortal_start_room;
extern struct room_data **world;
extern struct descriptor_data *descriptor_list;


int wargame = WARGAME_DISABLE;
char gamemaster[20] = "";


/* send message to all chars in ARENA flagged room */
void wargame_announce(char *message)
{
  struct descriptor_data *pt;
  char buf[SMALL_BUFSIZE];

  sprintf(buf, "#w%s#N\r\n", message);

  for (pt = descriptor_list; pt; pt = pt->next)
    if (!pt->connected && pt->character &&
	(ROOM_FLAGGED(pt->character->in_room, ARENA) ||
	 PLR_FLAGGED(pt->character, PLR_ARENA)))
      send_to_char(buf, pt->character);
}

ACMD(do_wargame)
{
  struct descriptor_data *pt;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int i = 0;

  skip_spaces(&argument);
  half_chop(argument, arg1, arg2);

  if (r_wargame_start_room < 0) {
    send_to_char("Wargames disabled, start room doesn't exist.\r\n", ch);
    return;
  }

  /* Special IMPL functions */
  if (GET_LEVEL(ch) == LEVEL_IMPL) {
    if (!strcmp("disable", arg1)) {
      if (wargame == NO_WARGAME) {
	wargame = WARGAME_DISABLE;
	send_to_char("Wargames disabled.\r\n", ch);
	return;
      } else
	send_to_char("End the wargames first.\r\n", ch);
    } else
      if (!strcmp("enable", arg1)) {
	if (wargame == WARGAME_DISABLE) {
	  wargame = NO_WARGAME;
	  send_to_char("Wargames enabled.\r\n", ch);
	  return;
	} else
	  send_to_char("Wargames not even disabled.\r\n", ch);
      }
  }

  if ((wargame == WARGAME_DISABLE) &&
      (GET_LEVEL(ch) < LEVEL_IMPL)) {
    send_to_char("Wargames currently disabled, contact the IMPL for info.\r\n", ch);
    return;
  }

  
  if ((GET_LEVEL(ch) < LEVEL_IMPL) && 
      !(GODLEVEL(ch) & IMM_WARGAME) &&
      (wargame > NO_WARGAME) && 
      strcmp(gamemaster, GET_NAME(ch))) {
    send_to_char ("Only the Gamemaster can control the games...\r\n", ch);
    return;
  }

  if (!strcmp("end", arg1) && (wargame != WARGAME_DISABLE)) {
    send_to_char("Wargame ended.\r\n", ch);
    wargame = NO_WARGAME;
    start_ammo   = 5;
    hits_to_kill = 2;
    teams        = 0;
    *gamemaster = '\0';
    team_members[0] = 0;
    team_members[1] = 0;
    team_members[2] = 0;
    team_members[3] = 0;
    team_members[4] = 0;

    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character) {
	REMOVE_BIT(PLR_FLAGS(pt->character), PLR_ARENA);

	if (ROOM_FLAGGED(pt->character->in_room, ARENA)) {
	  char_from_room(pt->character);
	  char_to_room(pt->character, r_mortal_start_room);
	}
	
      }
    if (world[r_wargame_start_room]->dir_option[4])
      world[r_wargame_start_room]->dir_option[4]->to_room = NOWHERE;
  } else if (!strcmp("start", arg1) && (wargame != WARGAME_DISABLE)) {
    if (wargame > NO_WARGAME) {
      send_to_char("Wargame already started or running...\r\n", ch);
      return;
    }
    send_to_char("Wargame started.\r\n", ch);
    wargame = START_WARGAME;
    sprintf(gamemaster, GET_NAME(ch));
  } else if (!strcmp("run", arg1)) {
    if (wargame != START_WARGAME) {
      send_to_char("Wargame not even started yet!\r\n", ch);
      return;
    }
    send_to_char("Wargame running.\r\n", ch);
    wargame_announce("Let the games Begin!!!");
    wargame = RUN_WARGAME;

    /* check for up exit */
    if (!(world[r_wargame_start_room]->dir_option[4])) {
      CREATE(world[r_wargame_start_room]->dir_option[4], struct room_direction_data, 1);
      world[r_wargame_start_room]->dir_option[4]->general_description = NULL;
      world[r_wargame_start_room]->dir_option[4]->keyword = NULL;
      world[r_wargame_start_room]->dir_option[4]->key = -1;
      world[r_wargame_start_room]->dir_option[4]->to_room = r_wargame_start_room + 1;
      world[r_wargame_start_room]->dir_option[4]->exit_info = 0;
    } else {
       world[r_wargame_start_room]->dir_option[4]->to_room = r_wargame_start_room + 1;
    }
  } else if (!strcmp("ammo", arg1)) {
    if (wargame > NO_WARGAME) {
      send_to_char("It's too late to change that now...\r\n", ch);
      return;
    }
    if (*arg2) {
      i = atoi(arg2);
      if (i > 0)
	start_ammo = MIN(100, i);
    }
    sprintf(buf, "Starting ammo is now #Y%d#N.\r\n", start_ammo);
    send_to_char(buf, ch);
    if (start_ammo < hits_to_kill)
      send_to_char("Starting ammo was set to a value lower than hits to kill!\r\n", ch);
  } else if (!strcmp("hits", arg1)) {
    if (wargame > NO_WARGAME) {
      send_to_char("It's too late to change that now...\r\n", ch);
      return;
    }
    if (*arg2) {
      i = atoi(arg2);
      if (i > 0)
	hits_to_kill = MIN(100, i);
    }
    sprintf(buf, "Hits to kill is now #Y%d#N.\r\n", hits_to_kill);
    send_to_char(buf, ch);
    if (hits_to_kill > start_ammo)
      send_to_char("Hits to kill set to a value higher than starting ammo!\r\n", ch);
  } else if (!strcmp("teams", arg1)) {
    if (wargame > NO_WARGAME) {
      send_to_char("It's too late to change that now...\r\n", ch);
      return;
    }
    if (*arg2) {
      i = atoi(arg2);
      if ((i > 0) && (i != 1) && i <= MAX_TEAMS)
	teams = i;
      else
	send_to_char("Valid choices are 0, 2, 3, 4 for teams.\r\n", ch);
    }
    sprintf(buf, "Number of teams is now #Y%d#N.\r\n", teams);
    send_to_char(buf, ch);
  } else {
    send_to_char("Usage: wargame < start | run | end | ammo | hits | teams >.\r\n", ch);
    if (GET_LEVEL(ch) == LEVEL_IMPL)
      send_to_char("       wargame < disable | enable >.\r\n", ch);
    send_to_char("Current Wargame status: ", ch);
    switch (wargame) {
      case -1:
	send_to_char("Disabled.\r\n", ch);
	break;
      case 0 : 
	send_to_char("None.\r\n", ch);
        break;
      case 1 :
	send_to_char("Started.\r\n", ch);
	break;
      case 2 :
	send_to_char("Running.\r\n", ch);
 	break;
      default :
	send_to_char("UNDEFINED.\r\n", ch);
    }
    if (*gamemaster) {
      sprintf(buf, "Gamemaster: %s\r\n", gamemaster);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Hits to Kill: [#Y%d#N]     Starting Ammo: [#Y%d#N]     Teams: [#Y%d#N]\r\n",
	    hits_to_kill, start_ammo, teams);
    send_to_char(buf, ch);
  }
}


ACMD(do_shoot)
{
  static char	*dirs[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n"
  };

  struct char_data *victim = NULL;
  struct descriptor_data *pt;
  int distance = 0, dir, roomnr, players = 0, i = 0, j = 0;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];


  if (!ROOM_FLAGGED(ch->in_room, ARENA)) {
    send_to_char("You can't do that here!!!\r\n", ch);
    return;
  }

  if (wargame !=RUN_WARGAME) {
    send_to_char("The wargames haven't begun yet...\r\n", ch);
    return;
  }

  if (!PLR_FLAGGED(ch, PLR_ARENA)) {
    send_to_char("You aren't a part of the wargames.\r\n", ch);
    return;
  }

  if (ch->specials.wargame.ammo <= 0) {
    send_to_char("You are out of ammo!\r\n", ch);
    return;
  }
  
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Shoot who???\r\n", ch);
    return;
  }

  half_chop(argument, arg1, arg2);

  if (!*arg2) {    
    if (!(victim = get_char_room_vis(ch, arg1))) {
      send_to_char("You swear your target was there just a moment ago.\r\n", ch);
      return;
    }
  } else {
    dir = search_block(arg2, dirs, FALSE);    

    if (dir == -1) {
      send_to_char("Shoot which direction?\r\n", ch);
      return;
    }
    
    roomnr = ch->in_room;
    
    if (EXIT(ch, dir)) {
      for (distance = 1; distance <= 2; distance++) {
	if ((world[ch->in_room]->dir_option[dir]) &&
	    (world[ch->in_room]->dir_option[dir]->to_room != NOWHERE) &&
	    !IS_SET(world[ch->in_room]->dir_option[dir]->exit_info, EX_CLOSED))
	  {
	    ch->in_room = world[ch->in_room]->dir_option[dir]->to_room;
	    if ((victim = get_char_room_vis(ch, arg1)))
		break;
	  } else
	    break;
      }
    } else {
      send_to_char("You can't shoot that direction\r\n", ch);
      return;
    }
    
    ch->in_room = roomnr;
  }

  if (!victim) {
    send_to_char("You fire a round in that direction... but your target isn't there anymore.\r\n", ch);
  } else {
    if (victim == ch) {
      send_to_char("You can't shoot yourself!!!\r\n", ch);
      return;
    }
    
    if (!PLR_FLAGGED(victim, PLR_ARENA)) {
      send_to_char("That player isn't a part of the wargames.\r\n", ch);
      return;
    }
    
    if (!ROOM_FLAGGED(victim->in_room, ARENA)) {
      send_to_char("That player isn't in the arena.\r\n", ch);
      return;
    }

    if (teams > 0)
      if (victim->specials.wargame.team == ch->specials.wargame.team) {
	sprintf(buf, "%s is on your team!\r\n", GET_NAME(victim)); 
	send_to_char(buf, ch);
	return;
      }
  }

  ch->specials.wargame.ammo -= 1;
  sprintf(buf, "You only have %d shots left.\r\n", ch->specials.wargame.ammo);
  send_to_char(buf, ch);
  if (victim) {
    if (number(1, 100) <= (100 / (1 << distance))) {
      /* A hit! */
      victim->specials.wargame.hits += 1;
      sprintf(buf, "You have been shot by %s.\r\n", GET_NAME(ch));
      send_to_char(buf, victim);
      sprintf(buf, "%s has shot %s.", GET_NAME(ch), GET_NAME(victim));
      wargame_announce(buf);
      
      /* A kill! */
      if (victim->specials.wargame.hits >= hits_to_kill) {
	sprintf(buf, "%s has slain %s.", GET_NAME(ch), GET_NAME(victim));
	wargame_announce(buf);
	char_from_room(victim);
	char_to_room(victim, r_mortal_start_room);
	REMOVE_BIT(PLR_FLAGS(victim), PLR_ARENA);
	if (teams > 0)
	  team_members[(int)victim->specials.wargame.team]--;

	/* check for winner */

	if (teams > 0) {
	  *buf = '\0';
	  for (i = 1; i <= teams; i++) {
	    if (team_members[i] <= 0) j++;
	    sprintf(buf2, "The %16s has %d players left.\r\n", which_team[i], team_members[i]);
	    strcat(buf, buf2);
	  }

	  if (j == (teams-1)) {
	    sprintf(buf, "The %s#w has won!!!", which_team[(int)ch->specials.wargame.team]);
	    wargame_announce(buf);
	    wargame = START_WARGAME;
	  } else
	    wargame_announce(buf);
	} else {	
	  for (pt = descriptor_list; pt; pt = pt->next)
	    if (!pt->connected && pt->character)
	      if (PLR_FLAGGED(pt->character, PLR_ARENA))
		players++;
	  if (players > 1) {
	    sprintf(buf, "There are %d players remaining\r\n", players);
	    wargame_announce(buf);
	  } else {
	    sprintf(buf, "%s is the winner!!!\r\n", GET_NAME(ch));
	    wargame_announce(buf);
	  }
	}
      }
      
    } else {
      send_to_char("You couldn't hit the broad side of a barn with a shot like that.\r\n", ch);
      sprintf(buf, "A shot from %s whizzes past your head.\r\n", GET_NAME(ch));
      send_to_char(buf, victim);
    }
  }
  /* Check for Sudden death, all starting ammo exausted, all ammo set to 99 */
  if (ch->specials.wargame.ammo == 0) {
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character)
	if (PLR_FLAGGED(pt->character, PLR_ARENA))
	  if (pt->character->specials.wargame.ammo > 0)
	      return;

    /* All ammo is exausted so Sudden Death */
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character)
	if (PLR_FLAGGED(pt->character, PLR_ARENA))
	  pt->character->specials.wargame.ammo = 99;
    wargame_announce("Sudden Death!!! All Ammo has been set to 99.");
  }
}

ACMD(do_teams)
{
  struct descriptor_data *pt;
  int i = 0;
  char valid_teams[128];

  skip_spaces(&argument);
  *buf = '\0';

  if (teams <= 0) {
    send_to_char("There are no teams!\r\n", ch);
  }

  if (!strcmp(argument, "list")) {
    for (i = 1; i <= teams; i++) {
      sprintf(buf2, "\r\n%16s [%d]: ", which_team[i], team_members[i]);
      strcat(buf, buf2);
      for (pt = descriptor_list; pt; pt = pt->next)
	if (!pt->connected && pt->character)
	  if (PLR_FLAGGED(pt->character, PLR_ARENA))
	    if (pt->character->specials.wargame.team == i) {
	      strcat(buf, GET_NAME(pt->character));
	      strcat(buf, " ");
	    }
    }
    send_to_char(buf, ch);
    return;
  } else if (*argument) {
    if (!PLR_FLAGGED(ch, PLR_ARENA)) {
      send_to_char("You aren't even in the games!\r\n", ch);
      return;
    }
    team_members[(int)ch->specials.wargame.team]--;
    for (i = 1; i <= teams; i++)
      if (!strcmp(argument, team_names[i]))
	ch->specials.wargame.team = i;

    sprintf(buf, "You are on the %s.\r\n", which_team[(int)ch->specials.wargame.team]);
    send_to_char(buf, ch);
    sprintf(buf, "%s is now on the %s#w.", GET_NAME(ch), which_team[(int)ch->specials.wargame.team]);
    team_members[(int)ch->specials.wargame.team]++;
    wargame_announce(buf);
    
  } else {
    send_to_char("Usage: teams < list | new team >.\r\n", ch);
    valid_teams[0] = '\0';
    for (i = 1; i <= teams; i++) {
      strcat(valid_teams, team_names[i]);
      strcat(valid_teams, " ");
    }
    sprintf(buf, "Valid teams are: %s\r\n", valid_teams);
    send_to_char(buf, ch);

    if (PLR_FLAGGED(ch, PLR_ARENA)) {
      sprintf(buf, "You are on the %s.\r\n", which_team[(int)ch->specials.wargame.team]);
      send_to_char(buf, ch);
    }

    return;
  }

}


ACMD(do_join)
{
  int i = 0;
  char valid_teams[128];

  skip_spaces(&argument);

  if (PLR_FLAGGED(ch, PLR_ARENA)) {
    send_to_char("You have already joined the games...\r\n", ch);
    return;
  }

  switch (wargame) {
  case WARGAME_DISABLE :
    send_to_char("Wargames are currently disabled.\r\n", ch);
    return;
    break;
  case NO_WARGAME :
    send_to_char("No wargames going on right now...\r\n", ch);
    return;
    break;
  case START_WARGAME :
    break;
  case RUN_WARGAME :
    send_to_char("The wargames have already started.\r\n", ch);
    return;
    break;
  default :
    return;
    break;
  }


  if (teams > 0) {
    ch->specials.wargame.team = 0;
    valid_teams[0] = '\0';
    for (i = 1; i <= teams; i++) {
      strcat(valid_teams, team_names[i]);
      strcat(valid_teams, " ");
    }
    
    sprintf(buf, "You must select a team, valid teams are: %s\r\n", valid_teams);
    
    if (!*argument) {
      send_to_char("Usage: join <team>\r\n", ch);
      send_to_char(buf, ch);
      return;
    } else {
      for (i = 1; i <= teams; i++)
	if (!strcmp(argument, team_names[i]))
	  ch->specials.wargame.team = i;
   
   if (ch->specials.wargame.team == 0) {
	  send_to_char(buf, ch);
	  return;
      }
    }    
  }


  char_from_room(ch);
  char_to_room(ch, r_wargame_start_room);
  send_to_char("Welcome to the games...\r\n", ch);

  
  SET_BIT(PLR_FLAGS(ch), PLR_ARENA);
  ch->specials.wargame.ammo = start_ammo;
  ch->specials.wargame.hits = 0;

  if (teams > 0) {
    sprintf(buf, "%s has joined the %s#w.", GET_NAME(ch), which_team[(int)ch->specials.wargame.team]);
    team_members[(int)ch->specials.wargame.team]++;
    wargame_announce(buf);
  } else {
    sprintf(buf, "%s has joined the games.", GET_NAME(ch));
    wargame_announce(buf);
  }
}

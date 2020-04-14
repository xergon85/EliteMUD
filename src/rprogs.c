/* ************************************************************************
*  File: rprogs.c (Version 1.0)                         Part of EliteMUD  *
*  Usage: Room Progs                                                      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  (C) 1998 Petya Vachranukunkiet                                         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
************************************************************************ */

/*
   update_rprog_check is called from zone_update on PULSE_ZONE
   exit_rprog_check is called from do_simple_move upon exit
*/

/* The following is the format for rprogs in the .wld files
   place at the end of the room info before the trailing 'S'
   <percent> : Activation on percentage
   <dest> : vnum of room destination
   <direction> : Specify direction or direction as bits
   a[1] = N, b[2] = E, c[4] = S, d[8] = W, e[16] = U, f[32] = D
   message : Message upon success

Teleport to destination upon exit
>trans <percent> <dest> <direction>~
message~

Teleport to destination
>ttrans <percent> <dest>~
message~

Force PCs towards a direction
>push <percent> <direction>~
message~

Force NPCs and PCs towards a direction
>pushall <percent> <direction>~
message~

Echo a message to a room
>echo <percent>~
message~

*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "functions.h"
#include "interpreter.h"
#include "comm.h"

extern struct room_data **world;
int roomflag_check(struct char_data *ch);
ACMD(do_look);

int execute_trans_rprog(struct char_data *ch, RPROG_DATA *rprog, int cmd)
{
  int percent = 0, dest = 0, dir = -1, was_in = NOWHERE;
  struct follow_type *k, *next_dude;

  sscanf(rprog->arglist, " %d %d %d ", &percent, &dest, &dir);
  if ((dest = real_room(dest)) < 0) {
    sprintf(buf, "SYSERR: Room %d RPROG trans to nonexistant room", world[ch->in_room]->number);
    log(buf);
    rprog->type = ERROR_RPROG;
  } else if ((dir < 0) || (cmd == dir)) {
    if (!IS_NPC(ch) && (number(1, 100) <= percent)) {
      was_in = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, dest);
      send_to_char(rprog->comlist, ch);
      roomflag_check(ch);
      do_look(ch, "", 0, 0);

      /* check for group members or mounts */
      if (ch->specials.mounted_by) {
	char_from_room(ch->specials.mounted_by);
	char_to_room(ch->specials.mounted_by, dest);
	send_to_char(rprog->comlist, ch->specials.mounted_by);
	roomflag_check(ch->specials.mounted_by);
	do_look(ch->specials.mounted_by, "", 0, 0);
      }

      if (ch->followers) {
	for (k = ch->followers; k; k = next_dude) {
	  next_dude = k->next;
	  if ((was_in == k->follower->in_room) && 
	      (GET_POS(k->follower) >= POS_STANDING) &&
	      !PLR_FLAGGED(k->follower, PLR_WRITING | PLR_MAILING)) {
	    act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
	    char_from_room(k->follower);
	    char_to_room(k->follower, dest);
	    send_to_char(rprog->comlist, k->follower);
	    roomflag_check(k->follower);
	    do_look(k->follower, "", 0, 0);	    
	  }
	}
      }

      return 1;
    }
  }
  
  return 0;
}

void execute_ttrans_rprog(struct room_data *room, RPROG_DATA *rprog)
{
  int percent = 0, dest = 0;
  struct char_data *ch, *next;

  sscanf(rprog->arglist, " %d %d ", &percent, &dest);
  if ((dest = real_room(dest)) < 0) {
    sprintf(buf, "SYSERR: Room %d RPROG ttrans to nonexistant room", room->number);
    log(buf);
    rprog->type = ERROR_RPROG;
  } else {
    if (number(1, 100) <= percent) {
      for (ch = room->people; ch; ch = next) {
	next = ch->next_in_room;
	char_from_room(ch);
	char_to_room(ch, dest);
	send_to_char(rprog->comlist, ch);
	roomflag_check(ch);
	do_look(ch, "", 0, 0);
      }
    }
  }
  
  return;
}

void execute_echo_rprog(struct room_data *room, RPROG_DATA *rprog)
{
  struct char_data *ch;
  int percent = 0;

  sscanf(rprog->arglist, " %d ", &percent);

  if (number(1, 100) <= percent) {
    for (ch = room->people; ch; ch = ch->next_in_room)
      send_to_char(rprog->comlist, ch);
  }

  return;
}

void execute_push_rprog(struct room_data *room, RPROG_DATA *rprog, int mob)
{
  struct char_data *ch, *next;
  int percent = 0, dir = 0;

  sscanf(rprog->arglist, " %d %d ", &percent, &dir);
  if (dir < 0 || dir > 5)
  {
    sprintf(buf, "SYSERR:%d not a valid Dir in room %d in push RPROG", dir,room->number);
    log(buf);
    rprog->type = ERROR_RPROG;
    return;
  }
  if (!room->dir_option[dir] ||
      room->dir_option[dir]->to_room < 0) {
    sprintf(buf, "SYSERR: Room %d RPROG push to nonexistant exit", room->number);
    log(buf);
    rprog->type = ERROR_RPROG;
  } else {
    if (number(1, 100) <= percent) {
      for (ch = room->people; ch; ch = next) {
	next = ch->next_in_room;
	if (!mob && IS_NPC(ch)) continue;
	if ((EXIT(ch, dir)->to_room != NOWHERE) && 
	    !IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
	  char_from_room(ch);
	  char_to_room(ch, room->dir_option[dir]->to_room);
	  send_to_char(rprog->comlist, ch);
	  roomflag_check(ch);
	  do_look(ch, "", 0, 0);
	}
      }
    }
  }

  return;
}

/* Checks for rprogs upon PULSE_ZONE (1 for success, 0 for fail) */
int update_rprog_check(struct room_data *room)
{
  RPROG_DATA *rprog;

  for (rprog = room->rprogs; rprog; rprog = rprog->next) {
    switch (rprog->type) {
    case ECHO_RPROG :
      execute_echo_rprog(room, rprog); 
      break;
    case PUSH_RPROG :
      execute_push_rprog(room, rprog, 0);
      break;
    case PUSHALL_RPROG :
      execute_push_rprog(room, rprog, 1);
      break;
    case TTRANS_RPROG :
      execute_ttrans_rprog(room, rprog);
      break;
    default : 
      continue;
      break;
    }
  }
  
  return 1;
}

/* Checks for rprogs upon exit of a room (1 for success, 0 for fail) */
int exit_rprog_check(struct char_data *ch, int cmd)
{
  RPROG_DATA *rprog;
  int i = 0;

  for (rprog = world[IN_ROOM(ch)]->rprogs; rprog && !i; rprog = rprog->next) {
    switch (rprog->type) {
    case TRANS_RPROG :
      i = execute_trans_rprog(ch, rprog, cmd);
      break;
    default : 
      break;
    }
  }
  
  return i;
}

char *rprog_type_to_name(int type)
{
  switch(type) {
  case TRANS_RPROG : return "Trans";
    break;
  case TTRANS_RPROG : return "Time Trans";
    break;
  case ECHO_RPROG : return "Echo";
    break;
  case PUSH_RPROG : return "Push";
    break;
  case PUSHALL_RPROG : return "Push All";
    break;
  default : return "Error Prog";
    break;
  }

  return "Error Prog";
}

ACMD(do_rpstat)
{
  RPROG_DATA *rprog;

  skip_spaces(&argument);

  if (world[IN_ROOM(ch)]->rprogs) {
    send_to_char("Room Programs:\r\n", ch);
    for (rprog = world[IN_ROOM(ch)]->rprogs; rprog; rprog = rprog->next) {
      sprintf(buf, "Type: %s[%d], Prog: %s\r\n%s\r\n", rprog_type_to_name(rprog->type), rprog->type, rprog->arglist, rprog->comlist);
      send_to_char(buf, ch);
    }
  } else {
    send_to_char("No Room Programs in this room.\r\n", ch);
  }

  return;
}

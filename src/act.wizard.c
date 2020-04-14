/*************************************************************************
 *   File: act.wizard.c                                  Part of EliteMUD *
 *  Usage: Player-level god commands and other goodies                    *
 *                                                                        *
 *  All rights reserved.  See license.doc for complete information.       *
 *                                                                        *
 *  Copyright (C) 1993 by the Trustees of the Johns Hopkins University    *
 *  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
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
#include "limits.h"
#include "screen.h"
#include "history.h"
#include "functions.h"
#include "ocs.h"
#include "objsave.h"



/*   external vars  */
extern FILE *player_fl;
extern struct room_data **world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern struct title_type titles[20][40];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern struct clan_data *clan_list;
extern int	top_of_zone_table;
extern int	restrict;
extern int	top_of_world;
extern int	ocs_rooms;
extern int	ocs_room_buffer;
extern int	top_of_mobt;
extern int	top_of_objt;
extern int	top_of_p_table;
extern int      top_of_clan;
extern char    *ideas;              /* ideas by players             */
extern char    *bugs;               /* bugs reported                */
extern char    *typos;              /* typos reported               */
extern int     plr_cmds_per_sec;    /* Commands per sec by players */
extern int     cmds_per_sec;        /* Commands per sec */
extern float   elite_efficiency;    /* How much mud is calculation */
extern int     is_quest;
/* for objects */
extern char	*item_types[];
extern char	*wear_bits[];
extern char	*extra_bits[];
extern char	*drinks[];
extern char     *portal_bits[];

/* for rooms */
extern char	*dirs[];
extern char	*room_bits[];
extern char	*exit_bits[];
extern char	*sector_types[];

/* for chars */
extern char	*spells[];
extern char     *skills[];
extern char     *attacktypes[];
extern char     *damtypes[];
extern char	*equipment_types[];
extern char	*affected_bits[];
extern char     *affected_bits2[];
extern char	*apply_types[];
extern char	*pc_class_types[];
extern char	*npc_class_types[];
extern char	*action_bits[];
extern char	*player_bits[];
extern char     *race_table[];
extern char     *quest_bits[];
extern char     *npc_types[];
extern char	*preference_bits[];
extern char	*position_types[];
extern char	*connected_types[];
extern char     *mob_cast_array[];
extern int      str_max[];
extern int      con_max[];
extern int      dex_max[];
extern int      wis_max[];
extern int      int_max[];
extern int      cha_max[];
extern int      hp_start[];
extern int      mv_start[];
extern int      mn_start[];
extern char     *imm_powers[];
extern char     *item_affects[];


void Crash_listlocker(struct char_data *ch, char *name);
int obj_in_room(struct obj_data *obj);
ACMD(do_look);

ACMD(do_emote)
{
  skip_spaces(&argument);

  if (PLR_FLAGGED(ch, PLR_MUTE)) {
    send_to_char("You cannot even emote.\r\n", ch);
    return;
  }

  if (!*argument) {
    send_to_char("Yes.. But what?\r\n", ch);
    return;
  }

  sprintf(buf, "$n %s #N", argument);

  MOBTrigger = FALSE;
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Ok.\r\n", ch);
  else {
    MOBTrigger = FALSE;
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}


ACMD(do_send)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }

  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char("No such person around.\r\n", ch);
    return;
  }

  strcat(buf, "#N");

  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);

  }
/*  sprintf(buf2, "(GC) %s has sent '%s#G' to %s", GET_NAME(ch), buf, GET_NAME(vict));
  mudlog(buf2, CMP, LEVEL_GREATER, TRUE);*/
}



ACMD(do_echo)
{
  int	i;

  for (i = 0; *(argument + i) == ' '; i++)
    ;

  if (!*(argument + i))
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    sprintf(buf, "%s#N\r\n", argument + i);
    send_to_room_except(buf, ch->in_room, ch);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Ok.\r\n", ch);
    else
      send_to_char(buf, ch);
/*    sprintf(buf, "(GC) %s has echoed '%s#G' at %d", GET_NAME(ch), argument + i, ch->in_room);
    mudlog(buf, CMP, LEVEL_GREATER, TRUE);*/
  }
}


/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
sh_int find_target_room(struct char_data *ch, char *rawroomstr)
{
  int	tmp;
  sh_int location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char	roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return NOWHERE;
  }

  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("That object is not available.\r\n", ch);
      return NOWHERE;
    }
  } else if ((location = find_room_by_name(roomstr)) == NOWHERE) {
    send_to_char("No such creature or object around.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found. */
  if (IS_SET(world[location]->room_flags, GODROOM) && GET_LEVEL(ch) < LEVEL_DEITY) {
    send_to_char("You are not godly enough to use that room!\r\n", ch);
    return NOWHERE;
  }

  if (IS_SET(world[location]->room_flags, PRIVATE) &&
      (GET_LEVEL(ch) < LEVEL_ADMIN && !IS_SET(GODLEVEL(ch), IMM_OVERSEER)))
    if (world[location]->people && world[location]->people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return NOWHERE;
    }

  return location;
}



ACMD(do_at)
{
  char	command[MAX_INPUT_LENGTH];
  int	location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the guy's still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  sh_int location;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (ch->specials.poofOut)
    sprintf(buf, "$n %s", ch->specials.poofOut);
  else
    strcpy(buf, "$n disappears in a puff of smoke.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);

  if (ch->specials.poofIn)
    sprintf(buf, "$n %s", ch->specials.poofIn);
  else
    strcpy(buf, "$n appears with an ear-splitting bang.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  do_look(ch, "", 0, 0);
}


ACMD(do_trans)
{
  struct descriptor_data *i;
  struct char_data *victim;
  int invis = 0;

  invis = GET_INVIS_LEV(ch);

  one_argument(argument, buf);

  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf)))
      send_to_char("No-one by that name around.\r\n", ch);
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
        send_to_char("Go transfer someone your own size.\r\n", ch);
        return;
      }
      invis = MAX(invis, GET_INVIS_LEV(victim));

      if (ch->specials.transOut) {
        sprintf(buf, "%s", ch->specials.transOut);
        act(buf, FALSE, victim, 0, ch, TO_ROOM);
      }
      else
        act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);

      char_from_room(victim);
      char_to_room(victim, ch->in_room);

      if (ch->specials.transIn) {
        sprintf(buf, "%s", ch->specials.transIn);
        act(buf, FALSE, victim, 0, ch, TO_ROOM);
      }
      else
        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);

      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      do_look(victim, "", 0, 0);
      sprintf(buf, "(GC) %s has transfered %s to %s.",
              GET_NAME(ch), GET_NAME(victim), world[ch->in_room]->name);
      mudlog(buf, BRF, MAX(LEVEL_IMMORT, invis), TRUE);
    }
  } else {                      /* Trans All */
    if (GET_LEVEL(ch) < LEVEL_ADMIN)
      return;
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && i->character != ch) {
        victim = i->character;
        act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
        char_from_room(victim);
        char_to_room(victim, ch->in_room);
        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
        act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
        do_look(victim, "", 0, 0);
      }

    send_to_char("Ok.\r\n", ch);
    sprintf(buf, "(GC) %s has transfered ALL to %s.",
            GET_NAME(ch), world[ch->in_room]->name);
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, invis), TRUE);
  }
}


ACMD(do_teleport)
{
  struct char_data *victim;
  sh_int target;

  half_chop(argument, buf, buf2);

  if (!*buf)
    send_to_char("Who do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf)))
    send_to_char("No-one by that name around.\r\n", ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
      send_to_char("Try teleporting someone your own size.\r\n", ch);
      return;
    }
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *)victim, TO_VICT);
    do_look(victim, "", 0, 0);
    sprintf(buf, "(GC) %s has teleported %s to %s.",
	    GET_NAME(ch), GET_NAME(victim), world[victim->in_room]->name);
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
  }
}

ACMD(do_locate)
{
  int v_num = 0, r_num = 0, num = 0, room;
  struct obj_data *k;
  struct char_data *i;

  half_chop(argument, arg, buf2);

  if (!*arg || !*buf2 || (!is_abbrev(arg, "mob") && !is_abbrev(arg, "obj"))
       || !isdigit(*buf2)) {
    send_to_char("Usage: locate <obj | mob> <vnum>\r\n", ch);
    return;
  }

  v_num = atoi(buf2);

  if (is_abbrev(arg, "mob")) {

    if ((r_num = real_mobile(v_num)) < 0) {
      send_to_char("There is no mobile with that number.\r\n", ch);
      return;
    }

    sprintf(buf, "There are (%d) '#Y%s#N' in the game.\r\n",
	    mob_index[r_num].number,
	    mob_proto[r_num].player.short_descr);

    for (i = character_list;
	 (i) && (strlen(buf) < (MAX_STRING_LENGTH - 200));
	 i = i->next)
      if (CAN_SEE(ch, i) && IS_NPC(i) && (i->nr == r_num)) {
	num++;
	sprintf(buf, "%s#y%2d#N. %-25s - [#b%5d#N] %s\r\n", buf,
		num,
		GET_NAME(i),
		(i->in_room!=NOWHERE)?world[i->in_room]->number:-1,
		((i->in_room) != NOWHERE)?world[i->in_room]->name:"Nowhere");
      }

  } else
    if (is_abbrev(arg, "obj")) {

      if ((r_num = real_object(v_num)) < 0) {
	send_to_char("There is no object with that number.\r\n", ch);
	return;
      }

      sprintf(buf, "There are (%d) '#Y%s#N' in the game.\r\n",
	      obj_index[r_num].number,
	      (obj_proto[r_num].short_description) ? obj_proto[r_num].short_description : "<None>");

      for (k = object_list;
	   (k) && (strlen(buf) < (MAX_STRING_LENGTH - 200));
	   k = k->next)
	if (CAN_SEE_OBJ(ch, k) && (k->item_number == r_num)) {
	  if ((i = (k->used_by?k->used_by:obj_carried_by(k))) &&
	      CAN_SEE(ch, i)) {
	    num++;
	    sprintf(buf, "%s#y%2d#N. %s #rINOBJ:#N %s #rON:#N %s - [#b%5d#N] %s\r\n",
		    buf, num, k->short_description,
		    (k->in_obj?k->in_obj->short_description:"None"),
		    GET_NAME(i),
		    (IN_ROOM(i)!=NOWHERE)?world[IN_ROOM(i)]->number:-1,
		    (IN_ROOM(i)!=NOWHERE)?world[IN_ROOM(i)]->name:"Nowhere");
	  } else {
	    room = obj_in_room(k);

	    num++;
	    sprintf(buf, "%s#y%2d#N. %s #rINOBJ:#N %s - [#b%5d#N] %s\r\n",
		    buf, num,
		    k->short_description,
		    (k->in_obj?k->in_obj->short_description:"None"),
		    (room!=NOWHERE)?world[room]->number:-1,
		    (room!=NOWHERE)?world[room]->name:"Nowhere");
	  }
	}
    }

  if (strlen(buf) >= MAX_STRING_LENGTH - 200)
    strcat(buf, "String Length Exceeded\r\n");

  page_string(ch->desc, buf, TRUE);
}

#define VNUM_FORMAT \
"Usage: vnum < obj | mob | room > [[-n] name] [-l minlevel[-maxlevel]] [-p page]\r\n"

ACMD(do_vnum)
{
  char mode[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], tmp;
  int low = 0, high = LEVEL_IMPL, page = 1, use_name = 0, found = 0, nr;
  int current;
  char output[LARGE_BUFSIZE];

  half_chop(argument, mode, buf);
  *name = '\0';

  if (!*mode || !*buf || (!is_abbrev(mode,"mob") && !is_abbrev(mode,"obj") &&
      !is_abbrev(mode,"room"))) {
    send_to_char(VNUM_FORMAT, ch);
    return;
  }

  while(*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      tmp = *(arg + 1);

      switch(tmp) {
      case 'n' :
	half_chop(buf1, name, buf);
	use_name = 1;
	break;
      case 'l' :
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	low = MAX(low, 0);
	high = MIN(high, LEVEL_IMPL);
	break;
      case 'p' :
	half_chop(buf1, arg, buf);
	page = atoi(arg);
	break;
      default:
	send_to_char(VNUM_FORMAT, ch);
	return;
	break;
      }

    } else {
      if (!use_name) {
	use_name = 1;
	strcpy(name, arg);
	strcpy(buf, buf1);
      } else {
	send_to_char(VNUM_FORMAT, ch);
	return;
      }
    }
  } /* vnum while parser */

  page = MAX(0, page-1);
  *buf = '\0';
  *output = '\0';

  /* MOB */
  if (is_abbrev(mode, "mob")) {

    for (current = low; current <= high; current++)
      for (nr = 0;
	   (nr <= top_of_mobt) &&
	     ((found+1) <= ((100 * page) + 100));
	   nr++)
	if (mob_proto[nr].player.level == current) {
	  if ((use_name == 1) && isname(name, mob_proto[nr].player.name)) {
	    found++;
	    if (found >= (100 * page)) {
	      sprintf(buf, "%3d. [#R%5d#N][#Y%3d#N] #C%-60.60s#N\r\n", found,
		      mob_index[nr].virtual,
		      mob_proto[nr].player.level,
		      mob_proto[nr].player.short_descr);
	      strcat(output, buf);
	    }
	  } else if (use_name == 0) {
	    found++;
	    if (found >= (100 * page)) {
	      sprintf(buf, "%3d. [#R%5d#N][#Y%3d#N] #C%-60.60s#N\r\n", found,
		      mob_index[nr].virtual,
		      mob_proto[nr].player.level,
		      mob_proto[nr].player.short_descr);
	      strcat(output, buf);
	    }
	  }
	}

    if ((found == 0) || (found < (100 * page))) {
      send_to_char("No mobiles fit that query\r\n", ch);
      return;
    }

  } /* END vnum mob */


  /* OBJ */
  if (is_abbrev(mode, "obj")) {

    for (current = low; current <= high; current++)
      for (nr = 0;
	   (nr <= top_of_objt) &&
	     ((found+1) <= ((100 * page) + 100));
	   nr++)
	if (obj_proto[nr].obj_flags.level == current) {
	  if ((use_name == 1) && isname(name, obj_proto[nr].name)) {
	    found++;
	    if (found >= (100 * page)) {
	      sprintf(buf, "%3d. [#G%5d#N][#R%3d#N] #Y%-60.60s#N\r\n", found,
		      obj_index[nr].virtual,
		      obj_proto[nr].obj_flags.level,
		      obj_proto[nr].short_description);
	      strcat(output, buf);
	    }
	  } else if (use_name == 0) {
	    found++;
	    if (found >= (100 * page)) {
	      sprintf(buf, "%3d. [#G%5d#N][#R%3d#N] #Y%-60.60s#N\r\n", found,
		      obj_index[nr].virtual,
		      obj_proto[nr].obj_flags.level,
		      obj_proto[nr].short_description);
	      strcat(output, buf);
	    }
	  }
	}

    if ((found == 0) || (found < (100 * page))) {
      send_to_char("No objects fit that query\r\n", ch);
      return;
    }

  } /* END vnum obj */


  /* ROOM */
  if (is_abbrev(mode, "room")) {
      for (nr = 0;
           (nr <= top_of_world) &&
             ((found+1) <= ((100 * page) + 100));
           nr++)
        if ((use_name == 1) && isname(name, world[nr]->name)) {
          found++;
            if (found >= (100 * page)) {
              sprintf(buf, "%3d. [#G%5d#N] #Y%-60.60s#N\r\n", found,
                      world[nr]->number,
                      world[nr]->name);
              strcat(output, buf);
            }
        } else if (use_name == 0) {
          found++;
          if (found >= (100 * page)) {
            sprintf(buf, "%3d. [#G%5d#N] #Y%-60.60s#N\r\n", found,
                    world[nr]->number,
                    world[nr]->name);
            strcat(output, buf);
          }
        }

    if ((found == 0) || (found < (100 * page))) {
      send_to_char("No rooms fit that query\r\n", ch);
      return;
    }

  } /* END vnum room */

  page_string(ch->desc, output, TRUE);
}


void
do_stat_room(struct char_data *ch, struct room_data *rm)
{
  struct extra_descr_data *desc;
  int	i, found = 0;
  struct obj_data *j = 0;
  struct char_data *k = 0;
  char tmp[100];

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s, Lights: [%d]\r\n", rm->zone, CCGRN(ch, C_NRM), rm->number, CCNRM(ch, C_NRM), ch->in_room, buf2, world[IN_ROOM(ch)]->light);
  send_to_char(buf, ch);

  sprintbit((long) rm->room_flags, room_bits, buf2);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n", spec_proc_by_name(rm->funct), buf2);
  send_to_char(buf, ch);

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }

  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    if (!IS_NPC(k))
      sprintf(tmp, "(PC)");
    else if (!IS_MOB(k))
      sprintf(tmp, "(NPC)");
    else
      sprintf(tmp, "[%d]", (k->nr >= 0) ? mob_index[k->nr].virtual : -1);
    sprintf(buf2, "%s %s%s", found++ ? "," : "", GET_NAME(k),tmp);
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s[%d]", found++ ? "," : "", j->short_description, (j->item_number >= 0) ? obj_index[j->item_number].virtual : -1);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }


  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
		world[rm->dir_option[i]->to_room]->number, CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	      rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
    }
  }
}



void do_stat_object(struct char_data *ch, struct obj_data *j)
{
  bool found;
  int	i;
  struct obj_data *j2;
  struct extra_descr_data *desc;
  int	virtual;

  virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_ITEM_TYPE(j), item_types, buf1);
  if (j->item_number >= 0)
    strcpy(buf2, spec_proc_by_name(obj_index[j->item_number].func));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
	  CCGRN(ch, C_NRM), virtual, CCNRM(ch, C_NRM), j->item_number, buf1, buf2);
  send_to_char(buf, ch);
  sprintf(buf, "#NL-Des: %s#N\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "#NExtra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }

  if (GET_ITEM_ANTICLASS(j)) {
    send_to_char("Anti Class: ", ch);
    sprintbit(GET_ITEM_ANTICLASS(j), &pc_class_types[1], buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
  }

  send_to_char("Can be worn on: ", ch);
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Extra flags   : ", ch);
  sprintbit(j->obj_flags.extra_flags, extra_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Level: %d, Hp: %d, Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
	  GET_ITEM_LEVEL(j), GET_ITEM_HP(j),
	  j->obj_flags.weight, j->obj_flags.cost,
	  j->obj_flags.cost_per_day,  j->obj_flags.timer);
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%d", world[j->in_room]->number);
    strcat(buf, buf2);
  }
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description: "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Used by: ");
  strcat(buf, j->used_by ? GET_NAME(j->used_by) : "Nobody");
  sprintf(buf, "%s\r\nNumber in world: %d",
	  buf,obj_index[j->item_number].number);
  sprintf(buf, "%s   CLAN: %d Exchange: %d\r\n", buf, CLANEQ(j) ? clan_list[CLANEQ_CLAN(j)].vnum : -1, CLANEQ(j) ? CLANEQ_EX(j) : -1);
  send_to_char(buf, ch);

  switch (j->obj_flags.type_flag) {
  case ITEM_LIGHT :
    sprintf(buf, "Color: [%d], Type: [%d], Hours: [%d]",
	    GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j,1), GET_ITEM_VALUE(j,2));
    break;
  case ITEM_SCROLL :
  case ITEM_POTION :
    sprintf(buf, "Level: %d  Spells: %d %d, %d", GET_ITEM_VALUE(j,0),
	    GET_ITEM_VALUE(j,1), GET_ITEM_VALUE(j,2), GET_ITEM_VALUE(j,3));
    break;
  case ITEM_WAND :
  case ITEM_STAFF :
    sprintf(buf, "Spell: \"%s\"  Spell-level: %d  Mana: %d",
	    spells[GET_ITEM_VALUE(j, 3) - 1],
	    GET_ITEM_VALUE(j,0),
	    GET_ITEM_VALUE(j,1));
    break;
  case ITEM_FIREWEAPON :
  case ITEM_WEAPON :
    sprintf(buf, "Damage: %dd%d, Type: %d, Hp: %d",
	    GET_ITEM_VALUE(j,1), GET_ITEM_VALUE(j,2), GET_ITEM_VALUE(j,3),
	    GET_ITEM_HP(j));
    break;
  case ITEM_MISSILE :
    sprintf(buf, "Damage: %d, Type: %d",
	    GET_ITEM_VALUE(j,1), GET_ITEM_VALUE(j,3));
    break;
  case ITEM_ARMOR :
    sprintf(buf, "AC-apply: %d, Hp: %d",
	    GET_ITEM_VALUE(j,0), GET_ITEM_HP(j));
    break;
  case ITEM_WORN:
    *buf = '\0';
    break;
  case ITEM_TRAP :
    sprintf(buf, "Spell: %d, - Hitpoints: %d",
	    GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j,1));
    break;
  case ITEM_CONTAINER :
    sprintf(buf, "Max-contains: %d, Locktype: %d\r\n",
	    GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j,1));

    if (GET_ITEM_VALUE(j, 3) > 0)
      sprintf(buf, "%sCorpse: %s, Blood: %d, CorpseLevel: %d", buf,
	      GET_ITEM_VALUE(j,3) ? "Yes" : "No",
	      GET_ITEM_VALUE(j,4), GET_ITEM_VALUE(j, 5));

    break;
  case ITEM_DRINKCON :
  case ITEM_FOUNTAIN :
    sprinttype(GET_ITEM_VALUE(j,2), drinks, buf2);
    sprintf(buf, "Max-contains: %d, Contains: %d, Poisoned: %s, Liquid: %s",
	    GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j,1),
	    GET_ITEM_VALUE(j,3) ? "Yes" : "No", buf2);
    break;
  case ITEM_NOTE :
    sprintf(buf, "Tounge: %d", GET_ITEM_VALUE(j,0));
    break;
  case ITEM_KEY :
    sprintf(buf, "Keytype: %d  Timer: %d  Timer Set: %d", GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j, 4), GET_ITEM_VALUE(j, 5));
    break;
  case ITEM_FOOD :
    sprintf(buf, "Makes full: %d, Poisoned: %d",
	    GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j,3));
    break;
  case ITEM_PORTAL :
    sprintf(buf, "Dest: %d  Lock item: %d  Min lvl: %d  Max lvl: %d  Duration: %d\r\n",
	    GET_ITEM_VALUE(j, 0), GET_ITEM_VALUE(j, 2), GET_ITEM_VALUE(j, 3),
	    GET_ITEM_VALUE(j, 4), GET_ITEM_VALUE(j, 5));
    send_to_char(buf, ch);
    send_to_char("Portal status: ", ch);
    sprintbit((long) GET_ITEM_VALUE(j, 1), portal_bits, buf);
    break;
  default :
    sprintf(buf, "Values 0-5: [%d] [%d] [%d] [%d] [%d] [%d]",
	    GET_ITEM_VALUE(j,0), GET_ITEM_VALUE(j,1),
	    GET_ITEM_VALUE(j,2), GET_ITEM_VALUE(j,3),
	    GET_ITEM_VALUE(j,4), GET_ITEM_VALUE(j,5));
    break;
  }
  send_to_char(buf, ch);

  strcpy(buf, "\r\nEquipment Status: ");
  if (!j->carried_by)
    strcat(buf, "None");
  else {
    found = FALSE;
    for (i = 0; i < MAX_WEAR; i++) {
      if (j->carried_by->equipment[i] == j) {
	sprinttype(i, equipment_types, buf2);
	strcat(buf, buf2);
	found = TRUE;
      }
    }
    if (!found)
      strcat(buf, "Inventory");
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  if (j->contains) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }

  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);

  send_to_char("\r\n", ch);
}


/* function to return a skill/spell/attacktype name dep. on number */
char * typename(int nr)
{
  static char typename[256];

  if (nr < SKILL_START) {	/* spell */
    strcpy(typename, "Spell ");
    strcat(typename, spells[nr - 1]);
  } else if (nr < 400) {	/* skill */
    strcpy(typename, "Skill ");
    strcat(typename, skills[nr - SKILL_START]);
  } else if (nr < DAMTYPE_START) {
    strcpy(typename, attacktypes[nr - TYPE_START]);
    strcat(typename, " attack");
  } else if (nr <= 600) {
    strcpy(typename, damtypes[nr - DAMTYPE_START]);
    strcat(typename, " damage");
  } else
    strcpy(typename, "Undefined");

  return typename;
}


/* Ability score description list
 * If any abitlity is ABOVE 25 function will crash.  Add more names!
 */
const char *
abilityscore[] = {
  "#Rnone#N", "#Rhorrible#N", "#Rawful#N", "#Yfeeble#N", "#Ylow#N",
  "#Baverage#N", "#Bgood#N", "#Ghigh#N", "#Gsuper#N", "#Gawesome#N",
  "#Mexceptional#N", "#Mtoo high#N", "#Rcheat#N" };

void do_stat_character(struct char_data *ch, struct char_data *k)
{
  int	i, i2, found = 0, acmod, hrmod, drmod, mr_cap = 85;
  long	vektor;
  struct follow_type *fol;
  struct affected_type *aff;

  extern struct str_app_type str_app[];

  sprintf(buf, "Level #R%d#N - #C%s %s#N -\r\n",
	  GET_LEVEL(k),
	  GET_NAME(k), (k->player.title ? k->player.title : "<No title>"));
  send_to_char(buf, ch);

  switch (k->player.sex) {
  case SEX_NEUTRAL	: strcpy(buf2, "neutral-sex "); break;
  case SEX_MALE	        : strcpy(buf2, "male "); break;
  case SEX_FEMALE	: strcpy(buf2, "female "); break;
  default		: strcpy(buf2, "sex??? "); break;
  }

  /* New routine by Petrus */

  if (IS_NPC(k))
    sprinttype(GET_RACE(k), npc_types, buf);
  else
    sprinttype(GET_RACE(k), race_table, buf);
  strcat(buf2, buf);

  if (IS_NPC(k))
    sprinttype(GET_CLASS(k), npc_class_types, buf1);
  else
    sprinttype(GET_CLASS(k), pc_class_types, buf1);

  sprintf(buf, "#G%d year old %s %s %s#N\r\n",
	  age(k).year, buf2, buf1,
	  (!IS_NPC(k) ? "#Gplayer#N" : (!IS_MOB(k) ? "#BNPC#N" : "#BMOB#N")));

  send_to_char(buf, ch);

  /* Multi-level routine  -Petrus  */

  if (!IS_NPC(k) && (IS_DUAL(k) || IS_MULTI(k))) {
    if (IS_3MULTI(k))
      sprintf(buf, "/#G%d#N", GET_3LEVEL(k));
    sprintf(buf2, "Levels #R%d#N/#Y%d#N%s ", GET_1LEVEL(k), GET_2LEVEL(k),
	    (IS_3MULTI(k) ? buf : ""));
    sprinttype(k->specials2.class1, pc_class_types, buf);
    strcat(buf2, buf);
    strcat(buf2, "/");
    sprinttype(k->specials2.class2, pc_class_types, buf);
    strcat(buf2, buf);
    if (IS_3MULTI(k)) {
      strcat(buf2, "/");
      sprinttype(k->specials2.class3, pc_class_types, buf);
      strcat(buf2, buf);
    }
    strcat(buf2, "\r\n");
    send_to_char(buf2, ch);
  }

  sprintf(buf2, "/#B%d#N", GET_ADD(k));
  sprintf(buf, "Str: [#C%d%s#N]  Int: [#C%d#N]  Wis: [#C%d#N]  Dex: [#C%d#N]  Con: [#C%d#N]  Cha: [#C%d#N]\r\n",
	  GET_STR(k), ((GET_STR(k) == 18 && GET_ADD(k) > 0) ? buf2 : ""),
	  GET_INT(k),
	  GET_WIS(k),
	  GET_DEX(k),
	  GET_CON(k),
	  GET_CHA(k));

  send_to_char(buf, ch);

  if (!IS_NPC(k) && PLR_FLAGGED(ch, PLR_TEST)) {
    acmod = -((GET_DEX(k)) * 10 / 6);
    hrmod = (str_app[STRENGTH_APPLY_INDEX(k)].tohit + ((GET_INT(k) - 13) / 3 ) + \
             ((GET_WIS(k) - 13) / 3 ) - GET_COND(k, DRUNK) );
    drmod = str_app[STRENGTH_APPLY_INDEX(k)].todam;

    sprintf(buf, "AC[#R%d/10#N  Mod: #w%d/10#N] Hitroll[#R%2d#N  Mod: #w%d#N]"
                 " Damroll[#R%2d#N  Mod: #w%d#N] THAC0[#R%2d#N]\r\n"
                 "Saves[Physical: #R%d#N / Mental: #R%d#N / Magic: #R%d#N / Poison: #R%d#N] "
                 "Magic Resistance[#R%2d#N]\r\n",
          GET_AC(k), acmod, k->points.hitroll, hrmod, k->points.damroll, drmod, get_thaco(k),
          k->specials2.resistances[0], k->specials2.resistances[1],
          k->specials2.resistances[2], k->specials2.resistances[3],
          ((k->specials2.resistances[4] < 85) ? k->specials2.resistances[4] : mr_cap));
    send_to_char(buf, ch);
  } else {
    sprintf(buf, "AC: [#R%d/10#N]  Hitroll: [#R%2d#N]  Damroll: [#R%2d#N]  THAC0: [#R%2d#N]\r\n"
                 "Saves [Physical: #R%d#N / Mental: #R%d#N / Magic: #R%d#N / Poison: #R%d#N]\r\n"
                 "Magic Resistance[#R%2d#N]\r\n",
          GET_AC(k), k->points.hitroll, k->points.damroll, get_thaco(k),
          k->specials2.resistances[0], k->specials2.resistances[1],
          k->specials2.resistances[2], k->specials2.resistances[3],
          ((k->specials2.resistances[4] < 85) ? k->specials2.resistances[4] : mr_cap));
    send_to_char(buf, ch);
  }

  if (!IS_NPC(k)) {

    buf2[0] = '\0';

    if (REMORT(k) > 0)
      sprintf(buf2, "Remort [#g%d#N] ", k->specials2.remorts);

    /* In preperation for continents and different hometowns */
    if (GET_LEVEL(ch) < LEVEL_DEITY)
      sprintf(buf1, "Midgaard");
    else
      sprintf(buf1, "%d", k->player.hometown);

    sprintf(buf, "%sClan[#B%d#N] ClanLevel[#B%d#N] Hometown[#B%s#N] Pracs[#B%d#N]\r\n",
            (buf2?buf2:""),
	    CLAN(k)>=0?clan_list[CLAN(k)].vnum:-1,
	    CLAN_LEVEL(k), buf1, SPELLS_TO_LEARN(k));

    if (GET_LEVEL(k) < LEVEL_WORSHIP || GET_LEVEL(k) >= LEVEL_DEITY ||
        GET_LEVEL(ch) >= LEVEL_DEITY) {
      if (WORSHIPS(ch)) {
        strcpy(buf1, get_deity_name(ch));
        CAP(buf1);
      }
      else
        sprintf(buf1, "None");
      sprintf(buf, "%sWorships[#b%s#N] ", buf, buf1);
    }

    if (GET_LEVEL(k) >= LEVEL_WORSHIP)
      sprintf(buf, "%sWorshippers[#b%d#N] Power[#b%d#N]",
	      buf, WORSHIPPERS(k), POWER(k));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
  }

  if (GET_LEVEL(ch) >= LEVEL_DEITY) /* Special data for >=IMM */

    {

      if (IS_MOB(k)) {
	sprintf(buf, "VNum[#B%5d#N] Names: #B%s#N\r\n",
		mob_index[k->nr].virtual, k->player.name);
	send_to_char(buf, ch);
      }

      sprintf(buf, "LDes: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));
      send_to_char(buf, ch);

      sprintf(buf, "IDNum[#G%5ld#N] Rnum[#G%3d#N] In room[#G%5d#N] XP[#G%d#N] Alignment[#G%d#N]\r\n",
	      GET_IDNUM(k), k->nr,
	      world[(k->in_room>-1?k->in_room:0)]->number,
	      GET_EXP(k),
	      GET_ALIGNMENT(k));
      send_to_char(buf, ch);

      if (!IS_NPC(k)) {
	strcpy(buf1, (char *)asctime(localtime(&(k->player.time.birth))));
	strcpy(buf2, (char *)asctime(localtime(&(k->player.time.last_logon))));
        buf1[24] = buf2[24] = '\0';

	sprintf(buf, "Created on #Y%s#N\r\n"
                     "Last Logon #Y%s#N   Played %dh %dm\r\n",
                buf1, buf2, k->player.time.played / 3600,
		((k->player.time.played / 3600) % 60));
	send_to_char(buf, ch);
      }

      sprintf(buf, "HP:[#C%d#N/#C%d#N+#C%d#N]  MANA:[#C%d#N/#C%d#N+#C%d#N]  MOVE:[#C%d#N/#C%d#N+#C%d#N]\r\n",
	      GET_HIT(k), GET_MAX_HIT(k), hit_gain(k),
	      GET_MANA(k), GET_MAX_MANA(k), mana_gain(k),
	      GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k));
      send_to_char(buf, ch);

      sprintf(buf, "Gold: #Y%d#N  Bank: #Y%d#N  Total: #Y%d#N\r\n",
	      GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));
      send_to_char(buf, ch);

      sprinttype(GET_POS(k), position_types, buf2);
      sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	      ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody") );
      if (k->desc) {
	sprinttype(k->desc->connected, connected_types, buf2);
	strcat(buf, ", Connected: ");
	strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);

      strcpy(buf, "Default position: ");
      sprinttype((k->mob_specials.default_pos), position_types, buf2);
      strcat(buf, buf2);

      sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->specials.timer);
      strcat(buf, buf2);
      send_to_char(buf, ch);

      if (IS_NPC(k)) {
	sprintbit(MOB_FLAGS(k), action_bits, buf2);
	sprintf(buf, "NPC flags: #C%s#N\r\n", buf2);
	send_to_char(buf, ch);
      } else {
	sprintbit(QUEST_FLAGS(k), quest_bits, buf2);
	sprintf(buf, "Quest: %d, QFLG: %s\r\n", QUEST_NUM(k), buf2);
	sprintbit(PLR_FLAGS(k), player_bits, buf2);
	sprintf(buf, "%sPLR: #C%s#N", buf, buf2);
	if (GET_LEVEL(ch) >= LEVEL_ADMIN)
	     if (IS_SET(PLR_FLAGS(k), PLR_LOG))
		  strcat(buf, "#rLOG#N ");
        strcat(buf, "\r\n");
	send_to_char(buf, ch);
	sprintbit(PRF_FLAGS(k), preference_bits, buf2);
	sprintf(buf, "PRF: #C%s#N\r\n", buf2);
	send_to_char(buf, ch);
      }

      if (IS_MOB(k)) {
	sprintf(buf, "Mob Spec-Proc: %s\r\n",
		spec_proc_by_name(mob_index[k->nr].func));
	send_to_char(buf, ch);
      }

      sprintf(buf, "Hunger: #R%d#N, Thirst: #R%d#N, Drunkness: #R%d#N\r\n",
	      GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
      send_to_char(buf, ch);

      sprintf(buf, "Protector: #M%s#N, Protecting: #M%s#N, Riding: #M%s#N, Carrying: #M%s#N\r\nMaster: %s, Followers:",
	      ((k->specials.protected_by)?GET_NAME(k->specials.protected_by):"-"),
	      ((k->specials.protecting)?GET_NAME(k->specials.protecting):"-"),
	      ((k->specials.mounting)?GET_NAME(k->specials.mounting):"-"),
	      ((k->specials.mounted_by)?GET_NAME(k->specials.mounted_by):"-"),
	      ((k->master) ? GET_NAME(k->master) : "-"));

      for (fol = k->followers; fol; fol = fol->next) {
	sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
	strcat(buf, buf2);
	if (strlen(buf) >= 62) {
	  if (fol->next)
	    send_to_char(strcat(buf, ",\r\n"), ch);
	  else
	    send_to_char(strcat(buf, "\r\n"), ch);
	  *buf = found = 0;
	}
      }

      if (*buf)
	send_to_char(strcat(buf, "\r\n"), ch);
    }

  if (GET_LEVEL(ch) >= LEVEL_DEITY && IS_NPC(k)) {
    send_to_char("Attacks:\r\n", ch);
    i = 0;

    while (k->mob_specials.attacks[i].type != 0) {
      if (k->mob_specials.attacks[i].type < SKILL_START) {
	sprintf(buf2, "%3d%%  %-14s [%d] [%d]",
		k->mob_specials.attacks[i].damadd,
		mob_cast_array[(int) (k->mob_specials.attacks[i].damodice)],
		k->mob_specials.attacks[i].damtype,
		k->mob_specials.attacks[i].damsizedice);
      } else if (k->mob_specials.attacks[i].type < TYPE_START)
	strcpy(buf2, "Damage according to skill");
      else {
	sprintf(buf2, "%-16s %2dd%d+%d",
		typename(k->mob_specials.attacks[i].damtype),
		k->mob_specials.attacks[i].damodice,
		k->mob_specials.attacks[i].damsizedice,
		k->mob_specials.attacks[i].damadd);
      }

      sprintf(buf, "  %2d. %3d%%  %-20s %s\r\n",
	      i + 1,
	      k->mob_specials.attacks[i].percent_of_use,
	      typename(k->mob_specials.attacks[i].type),
	      buf2);
      send_to_char(buf, ch);
      i++;
    }


    if (k->mob_specials.resists) {
      send_to_char("Resistance:\r\n", ch);
      i = 0;

      while (k->mob_specials.resists[i].type != 0) {
	sprintf(buf, "  %2d. %-20s %3d%%\r\n",
		i + 1,
		typename(k->mob_specials.resists[i].type),
		to_percentage(k, k->mob_specials.resists[i].percentage));
	send_to_char(buf, ch);
	i++;
      }
    }

    if (k->mobskills) {
      send_to_char("Skills:\r\n", ch);

      for (i = 0, i2 = 1, *buf = '\0'; i < MOB_SKILLS; i++) {
	if (get_mob_skill(k, i + SKILL_START) > 0) {
	  sprintf(buf, "%s %-19s %3d%%", buf,
		  skills[i],
		  get_mob_skill(k, i + SKILL_START));
	  if (!(i2 % 3))
	    strcat (buf, "\r\n");
	  else
	    strcat (buf, " |");
	  i2++;
	}
      }
      if ((i2 - 1) % 3)
	strcat (buf, "\r\n");

      send_to_char(buf, ch);
    }

    if (k->mobaction) {
      send_to_char("Mob Action String:\r\n", ch);
      send_to_char(k->mobaction, ch);
      send_to_char("\r\n", ch);
    }
  }

  /* Show Godlevel stuff */
  if ((GET_LEVEL(k) >= LEVEL_DEITY) &&
      ((k == ch) || IS_SET(GODLEVEL(ch), IMM_OVERSEER) ||
       IS_SET(GODLEVEL(ch), IMM_ADMIN) || IS_SET(GODLEVEL(ch), IMM_ALL))) {
    sprintbit((long) GODLEVEL(k), imm_powers, buf2);
    sprintf(buf, "Imm Powers: [%ld] %s\r\n", GODLEVEL(k), buf2);
    send_to_char(buf, ch);
  }

  /* Show arena info */
  if (PLR_FLAGGED(k, PLR_ARENA)) {
    sprintf(buf, "Wargame info: Ammo: [%d]  Hits: [%d]\r\n",
	    k->specials.wargame.ammo,
	    k->specials.wargame.hits);
    send_to_char(buf, ch);
  }

  /* Showing the bitvector */
  if (GET_LEVEL(ch) >= LEVEL_DEITY) {
    sprintbit(k->specials.affected_by, affected_bits, buf2);
    sprintf(buf, "Affected by: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    sprintbit(k->specials.affected_by2, affected_bits2, buf2);
    sprintf(buf, "Affected(2) by: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  /* Routine to show what spells a char is affected by */

  /* Put something here later that restricts -Petrus */
    if (k->affected)
    {
      for (aff = k->affected; aff; aff = aff->next) {
        if (aff->type == SPELL_FROM_ITEM) {
          *buf = '\0';
          i2 = 0;
          vektor = aff->bitvector;
          if (vektor >= 0) {
            for (i = 0; vektor; vektor >>=1) {
              if (IS_SET(1, vektor)) {
                if (*item_affects[i] != '\n') {
                  if (i2)
                    strcat(buf, "\r\n");
                  sprintf(buf, "%sMagic: (from item) #C%-21s#N",
                          buf, item_affects[i]);
                  i2++;
	          if (GET_LEVEL(ch) >= LEVEL_DEITY)
                    sprintf(buf, "%s sets %s", buf, affected_bits[i]);
                } /* else SYSERR */
              }
              if (*item_affects[i] != '\n')
                i++;
            }
          }
        } else {
	  if (aff->duration >= 0)
	    sprintf(buf2, "(%3d hours)", aff->duration+1);
	  else if (aff->duration == DURATION_INNATE)
	    strcpy(buf2, "(innate)");
	  else
	    strcpy(buf2, "(permanent)");

  	  sprintf(buf, "%s: %-11s #C%-21s#N ",
		(aff->type<SKILL_START?"Magic":(
		 aff->type<TYPE_START?"Skill":"ERROR")), buf2,
		(aff->type<SKILL_START?spells[aff->type-1]:(
                 aff->type<TYPE_START?skills[aff->type-SKILL_START]:"ERROR")));

  	  if (aff->modifier && GET_LEVEL(ch) >= LEVEL_DEITY) {
	    sprintf(buf2, "%+d to %s ", aff->modifier, apply_types[(int)aff->location]);
	    strcat(buf, buf2);
	  }
	  if (aff->bitvector && GET_LEVEL(ch) >= LEVEL_DEITY) {
	    if (*buf2)
	      strcat(buf, "sets ");
	    else
	      strcat(buf, "sets ");

	    if (aff->location != APPLY_BV2)
	      sprintbit((unsigned)aff->bitvector,affected_bits,buf2);
	    else
	      sprintbit((unsigned)aff->bitvector,affected_bits2,buf2);

	    strcat(buf, buf2);
	  }
        }
  	send_to_char(strcat(buf, "\r\n"), ch);
      }
    }
}


ACMD(do_skillstat)
{
  int i, i2;
  struct char_data *vict = 0;
  struct char_data *cbuf = NULL;
  struct char_file_u tmp_store;
  char is_file = 0;
  int player_i = 0;

  skip_spaces(&argument);

  if(!argument || !*argument) {
    send_to_char("Usage: skillstat <name>", ch);
    return;
  }

  if(!(vict = get_player_vis_exact(ch, argument))) {
    is_file = 1;
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);

    if((player_i = load_char(argument, &tmp_store)) > -1 ) {
      store_to_char(&tmp_store, cbuf);
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }

  }

  sprintf (buf, "Skills known:\r\n");
  for (i = 0, i2 = 0; *skills[i] != '\n'; i++) {
    if (GET_SKILL(vict, i+SKILL_START)) {
      sprintf (buf2, "%-19s %3d%%", skills[i], GET_SKILL(vict, i+SKILL_START));
      strcat (buf, buf2);
      if (i2 < 2) {
	strcat (buf, " | ");
	i2++;
      }else{
	strcat (buf, "\r\n");
	i2 = 0;
      }
    }
  }

  strcat (buf, "\r\n");
  send_to_char(buf, ch);

  sprintf (buf, "Spells known:\r\n");
  for (i = 0, i2 = 0; *spells[i] != '\n' && i< NUM_OF_SPELLS; i++) {
    if (GET_SKILL(vict, i+1)) {
      sprintf (buf2, "%-19s %3d%%", spells[i], GET_SKILL(vict, i+1));
      strcat (buf, buf2);
      if (i2 < 2) {
	strcat (buf, " | ");
	i2++;
      }else{
	strcat (buf, "\r\n");
	i2 = 0;
      }
    }
  }
  strcat (buf, "\r\n");
  send_to_char(buf, ch);

  if (is_file)
    free(cbuf);
}


ACMD(do_stat)
{
  struct char_data *victim = 0;
  struct obj_data *object = 0;
  struct char_file_u tmp_store;

  half_chop(argument, buf1, buf2);

  if ((!*buf1) || (GET_LEVEL(ch) < LEVEL_DEITY) )
    do_stat_character(ch, ch);
  else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch, world[IN_ROOM(ch)]);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2) {
      send_to_char("Stats on which mobile?\r\n", ch);
    } else {
      if ((victim = get_char_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      if (load_char(buf2, &tmp_store) > -1) {
	store_to_char(&tmp_store, victim);
	stringdata_load(victim);
	if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	else
	  do_stat_character(ch, victim);
	free_char(victim);
      } else {
	send_to_char("There is no such player.\r\n", ch);
	free(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2) {
      send_to_char("Stats on which object?\r\n", ch);
    } else {
      if ((object = get_obj_vis(ch, buf2)))
	do_stat_object(ch, object);
      else
	send_to_char("No such object around.\r\n", ch);
    }
  } else {
    if ((victim = get_char_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}


ACMD(do_shutdown)
{
  extern int	elite_shutdown, elite_reboot;

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("If you want to shut something down, say so!\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all(buf);
    log(buf);
    elite_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    sprintf(buf, "(GC) Reboot by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Rebooting.. come back in a minute or two.");
    system("touch ../log/.fastboot");
    elite_shutdown = elite_reboot = 1;
  } else if (!str_cmp(arg, "die")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all(buf);
    log(buf);
    system("touch ../log/.killscript");
    elite_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all(buf);
    log(buf);
    system("touch ../pause");
    elite_shutdown = 1;
  } else
    send_to_char("Unknown shutdown option.\r\n", ch);
}




ACMD(do_snoop)
{
  struct char_data *victim;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Snoop who?\r\n", ch);
    return;
  }

  if (!(victim = get_char_vis(ch, arg))) {
    send_to_char("No such person around.\r\n", ch);
    return;
  }

  if (!victim->desc) {
    send_to_char("There's no link.. nothing to snoop.\r\n", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Ok, you just snoop yourself.\r\n", ch);
    if (ch->desc->snooping) {
      ch->desc->snooping->snoop_by = 0;
      ch->desc->snooping = 0;
      sprintf(buf2, "(GC) %s stops snooping.", GET_NAME(ch));
      mudlog(buf2, BRF, GET_LEVEL(ch), TRUE);
    }
    return;
  }

  if (victim->desc->snoop_by) {
    send_to_char("Busy already. \r\n", ch);
    return;
  }

  if (GET_LEVEL(victim) >= GET_LEVEL(ch)) {
    send_to_char("You failed.\r\n", ch);
    return;
  }

  send_to_char("Ok. \r\n", ch);

  sprintf(buf2, "(GC) %s starts snooping %s.", GET_NAME(ch), GET_NAME(victim));
  mudlog(buf2, BRF, GET_LEVEL(ch), TRUE);

  if (ch->desc->snooping)
    ch->desc->snooping->snoop_by = 0;

  ch->desc->snooping = victim->desc;
  victim->desc->snoop_by = ch->desc;
  return;
}



ACMD(do_switch)
{
  struct char_data *victim;

  if (GET_LEVEL(ch) < LEVEL_DEITY) {
    send_to_char("Switch? what? how? why?\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Switch with who?\r\n", ch);
  } else {
    if (!(victim = get_char_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else {
      if (ch == victim) {
	send_to_char("He he he... We are jolly funny today, eh?\r\n", ch);
	return;
      }

      if (!ch->desc || ch->desc->snoop_by || ch->desc->snooping) {
	send_to_char("Mixing snoop & switch is bad for your health.\r\n", ch);
	return;
      }

      if (victim->desc || (!IS_NPC(victim))) {
	send_to_char("You can't do that, the body is already in use!\r\n", ch);
      } else {
	send_to_char("Ok.\r\n", ch);

        if(GET_LEVEL(ch) < LEVEL_IMMORT) {
          sprintf(buf, "(GC) %s switches into %s.", GET_NAME(ch), GET_NAME(victim));
        mudlog(buf, BRF, LEVEL_GREATER, TRUE);
	}


	ch->desc->character = victim;
	ch->desc->original = ch;

	PRF_FLAGS(victim)  = PRF_FLAGS(ch);
	GET_SCRLEN(victim) = GET_SCRLEN(ch);

	if (IS_NPC(victim)) {
	  SET_BIT(MOB_FLAGS(victim), MOB_SWITCHED);
	}

	victim->desc = ch->desc;
	ch->desc = 0;
      }
    }
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);

    /* JE 2/22/95 */
    /* if someone switched into your original body, disconnect them */
    if (ch->desc->original->desc)
      close_socket(ch->desc->original->desc);

    ch->desc->character = ch->desc->original;

    PRF_FLAGS(ch->desc->original)  = PRF_FLAGS(ch);
    GET_SCRLEN(ch->desc->original) = GET_SCRLEN(ch);

    ch->desc->original = 0;

    ch->desc->character->desc = ch->desc;
    ch->desc = 0;
  } else
    send_to_char("Yeah, right...\r\n", ch);

  return;
}



ACMD(do_load)
{
  struct load_struct {
    char *cmd;
    int godlevel;
  } fields[] ={
    { "nothing" , 0 },
    { "mob"     , IMM_BASIC | IMM_QUESTOR },
    { "obj"     , IMM_STD | IMM_LOAD | IMM_QUESTOR },
    { "clan"    , IMM_CLAN },
    { "quest"   , IMM_QUEST },
    { "\n"      , 0 }
  };

  extern struct obj_data *obj_proto;
  struct char_data *mob;
  struct obj_data *obj;
  int	number, r_num, i, l;

  if (!*argument) {
    strcpy(buf, "Usage: load < obj | mob | clan | quest > <vnum>\r\nYou can load the following:\r\n\r\n");
    l = 1; i = 0;
    while (*(fields[l].cmd) != '\n') {
      if (IS_SET(GODLEVEL(ch), IMM_ALL) ||
	  (GODLEVEL(ch) & fields[l].godlevel)) {
	i++;
	sprintf(buf, "%s [%-12s]", buf, fields[l].cmd);
	if (!(i % 4))
	  strcat (buf, "\r\n");
      }
      l++;
    }
    if (i % 4)
      strcat (buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: load < obj | mob | clan | quest > <vnum>\r\n", ch);
    return;
  }

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(buf, fields[l].cmd, strlen(buf)))
      break;

  if (!IS_SET(GODLEVEL(ch), IMM_ALL))
    if (!(GODLEVEL(ch) & fields[l].godlevel)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return;
    }

  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }

  switch (l) { /* Do the load */
  case 1 : /* Mobs */
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    sprintf(buf, "(GC) %s loads mob:(%d) %s.", GET_NAME(ch),
	    number, GET_NAME(mob));
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    break; /* End mob */


  case 2 : case 3 : case 4 : /* Objs */
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }

    if (IS_SET(obj_proto[real_object(number)].obj_flags.extra_flags, ITEM_QUEST) && (l != 4)) {
      if (!IS_SET(GODLEVEL(ch), IMM_ALL))
	if (!(GODLEVEL(ch) & fields[4].godlevel)) {
	  send_to_char("You can't load quest items.\r\n", ch);
	  return;
	}
    }

    if (obj_proto[real_object(number)].clan_eq && (l != 3)) {
      if (!IS_SET(GODLEVEL(ch), IMM_ALL))
	if (!(GODLEVEL(ch) & fields[3].godlevel)) {
	  send_to_char("You can't load clan items.\r\n", ch);
	  return;
	}
    }

    obj = read_object(r_num, REAL);
    obj_to_room(obj, ch->in_room);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    sprintf(buf, "(GC) %s loads obj:(%d) %s.", GET_NAME(ch),
	    number, obj->short_description);
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    break; /* End load (obj, clan, quest) */
  default :
    break;
  }
}



ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  int	number, r_num, original_loc;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob | room } <number>\r\n", ch);
    return;
  }

  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    do_stat_character(ch, mob);
    char_to_room(mob,0);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else if (is_abbrev(buf, "room")) {
    if ((r_num = real_room(number)) < 0) {
      send_to_char("There is no room with that number.\r\n", ch);
      return;
    }
    /* a location has been found. */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, r_num);
    do_stat_room(ch, world[IN_ROOM(ch)]);
    /* check if the guy's still there */
    if (ch->in_room == r_num) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
  } else
    send_to_char("That'll have to be either 'obj', 'mob' or 'room'.\r\n", ch);
}






/* Clears out the contents of one room. This is a REAL number room */
void purge_room(int room)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  send_to_room("The world seems a little cleaner.\r\n", room);
  for (vict = world[room]->people; vict; vict = next_v) {
    next_v = vict->next_in_room;
    if (IS_NPC(vict))
      extract_char(vict);
  }
  for (obj = world[room]->contents; obj; obj = next_o) {
    next_o = obj->next_content;
    extract_obj(obj);
  }

}


/*
 * Various object/mobile removal forms
 */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;
  int count = 0, vnum, rnum, i;

  half_chop(argument, buf, buf1);

  if (!strcmp(buf, "obj") &&
             (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_WORLD))) {
    /* Destroy all objects of this vnum */
    if (!isdigit(*buf1)) {
      send_to_char("Usage: purge obj <vnum>\r\n", ch);
      return;
    }
    vnum = atoi(buf1);
    for(obj = object_list; obj; obj = next_o) {
      next_o = obj->next;
      if (obj->item_number && (obj_index[obj->item_number].virtual == vnum)) {
        extract_obj(obj);
        count++;
      }
    }
    sprintf(buf, "%d objects purged\r\n", count);
    sprintf(buf1, "(GC) %s purged %d instances of object (%d)",
                  GET_NAME(ch), count, vnum);
    mudlog(buf1, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    send_to_char(buf, ch);

  } else if (!strcmp(buf, "mob") &&
                    (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_WORLD))) {
    /* Destroy all mobiles of this vnum */
    if (!isdigit(*buf1)) {
      send_to_char("Usage: purge mob <vnum>\r\n", ch);
      return;
    }
    vnum = atoi(buf1);
    /* Destroy all objects and mobiles going by the name 'buf1' */
    for(vict = character_list; vict; vict = next_v) {
      next_v = vict->next;
      if (IS_NPC(vict) && vict->nr && (mob_index[vict->nr].virtual == vnum)) {
        extract_char(vict);
        count++;
      }
    }
    sprintf(buf, "%d mobiles purged\r\n", count);
    sprintf(buf1, "(GC) %s purged %d instances of mobile (%d)",
                  GET_NAME(ch), count, vnum);
    mudlog(buf1, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    send_to_char(buf, ch);

  } else if ((!strcmp(buf, "zone") ||
        !strcmp(buf, "reset")) &&
               (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_WORLD))) {
    /* Purge the entire contents of a zone <and reset it>. */
    if (!isdigit(*buf1)) {
      send_to_char("Usage: purge zone <zone num>, purge reset <zone num>\r\n", ch);
      return;
    }
    vnum = atoi(buf1);
    rnum = real_zone(vnum);

    for (i=0; i<=top_of_world; i++)
      if (world[i]->zone == rnum) purge_room(i);

    /* And now reset the zone if required. */
    if (!strcmp(buf, "reset")) {
      reset_zone(rnum, FALSE);

      sprintf(buf, "Purge-resetting zone %d (#%d): %s.\r\n",
          rnum, zone_table[rnum].number, zone_table[rnum].name);
      sprintf(buf1, "(GC) %s purge-reset zone %d (%s)",
          GET_NAME(ch), rnum, zone_table[rnum].name);
    } else {
      sprintf(buf, "Purging zone %d (#%d): %s.\r\n",
          rnum, zone_table[rnum].number, zone_table[rnum].name);
      sprintf(buf1, "(GC) %s purged zone %d (%s)",
          GET_NAME(ch), rnum, zone_table[rnum].name);
    }
    send_to_char(buf, ch);
    mudlog(buf1, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

  } else if (*buf) {      /* argument supplied. destroy single object
                           * or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char("Fuuuuuuuuu!\r\n", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (IS_NPC(vict)) {
        extract_char(vict);
      } else {
        sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
        mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        Crash_quitsave(vict);
        if (vict->desc) {
          close_socket(vict->desc);
          vict->desc = 0;
        }
        extract_char(vict);
      }
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room]->contents))) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char("Nothing here by that name.\r\n\r\n"
                   "Usage:\r\n"
                   "purge <obj> <vnum>        - Purge ALL objects of <vnum> in game\r\n"
                   "purge <mob> <vnum>        - Purge ALL mobiles of <vnum> in game\r\n"
                   "purge <zone> <zone num>   - Purge zone\r\n"
                   "purge <reset> <zone num>  - Purge zone and reset (sim)\r\n"
                   "purge <argument>          - Purge target mob/obj/player\r\n"
                   "purge                     - Purge contents of current room\r\n"
                   , ch);
      return;
    }

    send_to_char("Ok.\r\n", ch);
  } else {                      /* no argument. clean out the room */
    if (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_IMMORT)) {
      send_to_char("Don't... You would only kill yourself..\r\n", ch);
      return;
    }
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    purge_room(IN_ROOM(ch));
  }
}



/* Give pointers to the five abilities */
void	roll_abilities(struct char_data *ch)
{
  int temp;

  ch->abilities.str = 11;
  ch->abilities.con = 11;
  ch->abilities.dex = 11;
  ch->abilities.intel = 11;
  ch->abilities.wis = 11;
  ch->abilities.cha = 11;
  ch->abilities.str_add = 0;

  temp = GET_CLASS(ch);
  if (temp >= CLASS_2MULTI)
    temp = GET_1CLASS(ch);

  switch (temp) {
  case CLASS_ILLUSIONIST:
  case CLASS_PSIONICIST:
  case CLASS_WIZARD:
  case CLASS_MAGIC_USER:
  case CLASS_DRUID:
  case CLASS_CLERIC:
    ch->abilities.intel = 13;
    ch->abilities.wis = 13;
    break;
  case CLASS_MONK:
  case CLASS_PALADIN:
    ch->abilities.wis = 13;
    ch->abilities.str = 13;
    break;
  case CLASS_ASSASSIN:
  case CLASS_MARINER:
  case CLASS_BARD:
  case CLASS_THIEF:
  case CLASS_NINJA:
    ch->abilities.dex = 13;
    ch->abilities.str = 13;
    break;
  case CLASS_CAVALIER:
  case CLASS_RANGER:
  case CLASS_KNIGHT:
  case CLASS_WARRIOR:
    ch->abilities.str = 13;
    ch->abilities.con = 13;
    break;
  }
  if (ch->specials2.remorts >= 4) { /* Set to Max Stat for Race */
    ch->abilities.str = str_max[GET_RACE(ch)];
    ch->abilities.con = con_max[GET_RACE(ch)];
    ch->abilities.dex = dex_max[GET_RACE(ch)];
    ch->abilities.intel = int_max[GET_RACE(ch)];
    ch->abilities.wis = wis_max[GET_RACE(ch)];
    ch->abilities.cha = cha_max[GET_RACE(ch)];
    if (IS_WARRIOR(ch) || IS_CAVALIER(ch) || IS_KNIGHT(ch) || IS_RANGER(ch))
      ch->abilities.str_add = 100;
    else
      ch->abilities.str_add = 0;
  }
  ch->tmpabilities = ch->abilities;
}



void	do_start(struct char_data *ch)
{
  struct affected_type af;
  int i;

  /* Skill / Affect Clearance */
  if (ch->affected) {
    while (ch->affected)
      affect_remove(ch, ch->affected);
  }
  for (i = 0; i < MAX_SKILLS; i++)
   SET_SKILL(ch, i, 0);
  /* End of Skill / Affect Clearance */


  GET_LEVEL(ch) = 1;
  if (GET_CLASS(ch) >= CLASS_DUAL) {
    GET_1LEVEL(ch) = 1;
    GET_2LEVEL(ch) = 1;
    GET_3LEVEL(ch) = 1;
  }

  GET_EXP(ch) = 1;
  SPELLS_TO_LEARN(ch) = 0;
  set_title(ch);
  roll_abilities(ch);

  ch->points.max_hit  = hp_start[GET_RACE(ch)];
  ch->points.max_mana = mn_start[GET_RACE(ch)];
  ch->points.max_move = mv_start[GET_RACE(ch)];

  switch (GET_CLASS(ch)) {
  case CLASS_THIEF :
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    break;
  case CLASS_ASSASSIN :
    SET_SKILL(ch, SKILL_SNEAK, 15);
    SET_SKILL(ch, SKILL_HIDE, 10);
    SET_SKILL(ch, SKILL_BACKSTAB, 20);
    break;
  case CLASS_BARD :
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    break;
  case CLASS_MARINER :
    SET_SKILL(ch, SKILL_SWIM, 20);
    break;
  case CLASS_RANGER :
    SET_SKILL(ch, SKILL_HUNT, 75);
    break;
  }

  af.next = NULL;

  switch(GET_RACE(ch)) {
  case RACE_DWARF:
  case RACE_GNOME:
    af.type = SPELL_CAT_EYES;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration  = DURATION_INNATE;
    af.bitvector = AFF_LIGHT;
    affect_to_char(ch, &af);
    break;
  case RACE_DEMON:
    af.type = SPELL_CAT_EYES;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration  = DURATION_INNATE;
    af.bitvector = AFF_LIGHT;
    affect_to_char(ch, &af);

    af.type = SPELL_DETECT_ALIGN;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_DETECT_ALIGN;
    affect_to_char(ch, &af);
    break;
  case RACE_ELF:
  case RACE_HALFELF:
  case RACE_DROW:
  case RACE_RATMAN:
  case RACE_DRACONIAN:
    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(ch, &af);
    break;
  case RACE_VAMPIRE:
    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(ch, &af);

    GET_COND(ch, THIRST) = (char) 24;
    break;
  case RACE_ANGEL:
    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(ch, &af);

    af.type = SPELL_DETECT_ALIGN;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_DETECT_ALIGN;
    affect_to_char(ch, &af);
    break;
  case RACE_DRAGON:
    af.type = SPELL_ARMOR;
    af.bitvector = 0;
    af.modifier = -20;
    af.duration = DURATION_INNATE;
    af.location = APPLY_AC;
    affect_to_char(ch, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(ch, &af);

    SET_SKILL(ch, SKILL_TAIL_LASH, 75);
    break;
  case RACE_FAIRY:
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.type = SPELL_SANCTUARY;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(ch, &af);
    break;
  case RACE_WEREWOLF:
  case RACE_TROLL:
    af.type = SPELL_REGENERATION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_REGENERATION;
    affect_to_char(ch, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(ch, &af);
    break;
  case RACE_AVATAR:
    af.type = SPELL_BLESS;
    af.bitvector = 0;
    af.modifier = 2;
    af.duration = DURATION_INNATE;
    af.location = APPLY_HITROLL;
    affect_to_char(ch, &af);

    af.location = APPLY_SAVING_MAGIC;
    af.modifier = 5;
    affect_to_char(ch, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(ch, &af);

    GET_COND(ch, FULL) = (char) -1;
    GET_COND(ch, THIRST) = (char) -1;
    break;
    case RACE_FELINE:
    af.type = SPELL_CAT_EYES;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration  = DURATION_INNATE;
    af.bitvector = AFF_LIGHT;
    affect_to_char(ch, &af);

    SET_SKILL(ch, SKILL_SNEAK, 95);
    SET_SKILL(ch, SKILL_CLAW, 95);
    SET_SKILL(ch, SKILL_POUNCE, 95);
    break;
  }

  advance_level(ch, FALSE);

  switch(GET_RACE(ch)) {
   case RACE_VAMPIRE:
	GET_ADD(ch) = 100;
	GET_COND(ch, FULL) = -1;
	GET_COND(ch, DRUNK) = -1;
	break;
   case RACE_AVATAR:
	GET_COND(ch, FULL) = -1;
	GET_COND(ch, THIRST) = -1;
	break;
   case RACE_ANGEL:
   case RACE_DEMON:
        GET_COND(ch, FULL) = -1;
        GET_COND(ch, THIRST) = -1;
        break;
   default:
        if (ch->specials2.remorts >= 4) { /* Toggle Hunger / Thirst */
          GET_COND(ch, FULL) = -1;
          GET_COND(ch, THIRST) = -1;
        } else {
          GET_COND(ch, THIRST) = 24;
          GET_COND(ch, FULL) = 24;
        }
        GET_COND(ch, DRUNK) = 0;
	break;
  }

  sprintf(buf, "%s entering game as newbie.", GET_NAME(ch));
  mudlog(buf, BRF, LEVEL_DEITY, TRUE);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  GET_GOLD(ch) = 2000;
  WIMP_LEVEL(ch) = 0;

  ch->specials2.scrlen = 24;	/* Screen length for player -Petrus */

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

}


ACMD(do_advance)
{
  struct char_data *victim;
  char	name[100], level[100];
  int	newlevel;

  half_chop(argument, name, buf);
  one_argument(buf, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (ch == victim || GET_LEVEL(ch) <= GET_LEVEL(victim) || GET_LEVEL(victim) >=LEVEL_DEITY ) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }

  if (GET_LEVEL(victim) == 0) {
    newlevel = 1;
  } else if (!*level) {
    send_to_char("You must supply a level number.\r\n", ch);
    return;
  } else {
    if (!isdigit(*level)) {
      send_to_char("Second argument must be a positive integer.\r\n", ch);
      return;
    }
    if ((newlevel = atoi(level)) <= LOWEST_LEVEL(victim)) {
      do_start(victim);
      GET_LEVEL(victim) = 1;
    }
  }

  if (newlevel >= LEVEL_DEITY) {
    send_to_char("109 is the highest possible level.\r\n", ch);
    return;
  }

  act("$n makes some strange gestures.\r\nA strange feeling comes upon you,"
      "\r\nLike a giant hand, light comes down from\r\nabove, grabbing your"
      "body, that begins\r\nto pulse with colored lights from inside.\r\nYo"
      "ur head seems to be filled with demons\r\nfrom another plane as your"
      " body dissolves\r\nto the elements of time and space itself.\r\nSudde"
      "nly a silent explosion of light snaps\r\nyou back to reality.  You fee"
      "l slightly\r\ndifferent.", FALSE, ch, 0, victim, TO_VICT);

  send_to_char("Ok.\r\n", ch);

  if (GET_LEVEL(victim) == 0) {
    do_start(victim);
  } else {
    if (GET_LEVEL(victim) < LEVEL_IMPL) {
      sprintf(buf, "(GC) %s has advanced %s to level %d (from %d)",
	      GET_NAME(ch), GET_NAME(victim), newlevel, GET_LEVEL(victim));
      mudlog(buf, BRF, LEVEL_DEITY, TRUE);

      while (GET_LEVEL(victim) < newlevel) {
	advance_level(victim, FALSE);
	set_lowest_level(victim, LOWEST_LEVEL(victim) + 1);
      }
      save_char(victim, IN_VROOM(victim), 2);
    } else {
      send_to_char("Some idiot just tried to advance your level.\r\n", victim);
      send_to_char("IMPOSSIBLE!  IDIOTIC!\r\n", ch);
    }
  }
}




void restore_character(int level, struct char_data *vict)
{
  int i;

  GET_HIT(vict) = GET_MAX_HIT(vict);
  GET_MANA(vict) = GET_MAX_MANA(vict);
  GET_MOVE(vict) = GET_MAX_MOVE(vict);

  if ((level >= LEVEL_ADMIN) && (GET_LEVEL(vict) >= LEVEL_DEITY)) {
    for (i = 1; i <= MAX_SKILLS; i++)
      SET_SKILL(vict, i, 100);

    if (GET_LEVEL(vict) >= LEVEL_IMMORT) {
      /* Max stats */
      vict->abilities.str_add = 100;
      vict->abilities.intel = 25;
      vict->abilities.wis = 25;
      vict->abilities.dex = 25;
      vict->abilities.str = 25;
      vict->abilities.con = 25;
      vict->abilities.cha = 25;
    }

    /* No hunger */
    for (i = 0; i < 2; i++)
      GET_COND(vict, i) = -1;

    vict->tmpabilities = vict->abilities;

  }
  update_pos(vict);
}

void perform_restore(struct char_data *ch, struct char_data *vict)
{
  if (!IS_NPC(vict)) {
    if (GET_LEVEL(vict) < LEVEL_DEITY) {
      GET_HIT(vict) = GET_MAX_HIT(vict);
      GET_MANA(vict) = GET_MAX_MANA(vict);
      GET_MOVE(vict) = GET_MAX_MOVE(vict);
      update_pos(vict);
      act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
    }
  }
  return;
}

void restore_unaffect(struct char_data *ch, struct char_data *vict)
{

  if (affected_by_spell(vict, SPELL_BLINDNESS)) {
      affect_from_char(vict, SPELL_BLINDNESS);
      send_to_char("Your vision returns!\r\n", vict);
  }
  if (affected_by_spell(vict, SPELL_POISON)) {
      affect_from_char(vict, SPELL_POISON);
      send_to_char("A warm feeling runs through your body!\r\n", vict);
      act("$n looks better.", TRUE, vict, 0, 0, TO_ROOM);
  }
  if (affected_by_spell(vict, SPELL_CURSE)) {
      affect_from_char(vict, SPELL_CURSE);
      send_to_char("You don't feel so cursed anymore.\r\n", vict);
  }
  if (affected_by_spell(vict, SPELL_SLEEP)) {
      affect_from_char(vict, SPELL_SLEEP);
      GET_POS(vict) = POS_STANDING;
      send_to_char("You don't feel so sleepy anymore.\r\n", vict);
  }
  return;
}

void restore_mortals(struct char_data *ch, int SCMD)
{
  struct char_data *vict;


  switch (SCMD) {
  case SCMD_ALL:
    if (IS_SET(GODLEVEL(ch), IMM_ALL)) {
      for(vict = character_list; vict; vict = vict->next) {
        perform_restore(ch, vict);
        /* Remove negative affects from character */
        restore_unaffect(ch, vict);
      }
      sprintf(buf, "(GC) %s has restored all mortals.", GET_NAME(ch));
    }
    else
      send_to_char("I think not.\r\n", ch);
    break;
  case SCMD_ROOM:
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_STD)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (!IS_NPC(vict)) {
          if ((ch->in_room) == (vict->in_room)) {
            perform_restore(ch, vict);
            /* Remove negative affects from character */
            restore_unaffect(ch, vict);
          }
        }
      }
      sprintf(buf, "(GC) %s has restored mortals in room %d.", GET_NAME(ch),
              world[ch->in_room]->number);
    }
    else
      send_to_char("I think not.\r\n", ch);
    break;
  case SCMD_ZONE:
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_ADMIN)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (!IS_NPC(vict)) {
          if ((world[ch->in_room]->zone) == (world[vict->in_room]->zone)) {
            perform_restore(ch, vict);
            /* Remove negative affects from character */
            restore_unaffect(ch, vict);
          }
        }
      }
      sprintf(buf, "(GC) %s has restored mortals in zone %d.", GET_NAME(ch),
              zone_table[world[(ch->in_room)]->zone].number);
    }
    else
      send_to_char("I think not.\r\n", ch);
    break;
  case SCMD_QUEST:
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_STD)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (!IS_NPC(vict)) {
          if (IS_SET(PRF_FLAGS(ch), PRF_QUEST)) {
            if (IS_SET(PRF_FLAGS(vict), PRF_QUEST)) {
              perform_restore(ch, vict);
              /* Remove negative affects from character */
              restore_unaffect(ch, vict);
            }
          }
          else {
            send_to_char("You're not even part of the quest!\r\n", ch);
            return;
            break;
          }
        }
      }
      sprintf(buf, "(GC) %s has restored mortals on the quest!", GET_NAME(ch));
    }
    else
      send_to_char("I think not.\r\n", ch);
    break;
  default:
    sprintf(buf, "SYSERR: Invalid SCMD passed to restore_mortals!");
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Sorry, there is a problem.\r\n", ch);
    return;
  }
  mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
  send_to_char("Ok.\r\n", ch);
}

ACMD(do_restore)
{
  struct char_data *vict;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Usage:\r\n"
                 "restore <player> - Restore Player\r\n"
                 "restore all      - Restore ALL Players in Game\r\n"
                 "restore zone     - Restore Players in current zone\r\n"
                 "restore room     - Restore Players in current room\r\n"
                 "restore quest    - Restore Players on Quest Channel\r\n"
                 , ch);
  else if (!str_cmp("all", buf)) {
     restore_mortals(ch, SCMD_ALL);
  }
  else if (!str_cmp("zone", buf)) {
     restore_mortals(ch, SCMD_ZONE);
  }
  else if (!str_cmp("room", buf)) {
     restore_mortals(ch, SCMD_ROOM);
  }
  else if (!str_cmp("quest", buf)) {
     restore_mortals(ch, SCMD_QUEST);
  }
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char("No-one by that name here.\r\n", ch);
  else {
    sprintf(buf, "(GC) %s has restored %s.", GET_NAME(ch), GET_NAME(vict));
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
    restore_character(GET_LEVEL(ch), vict);
    /* Remove negative affects from character */
    restore_unaffect(ch, vict);
    send_to_char("Ok.\r\n", ch);
  }
}


ACMD(do_invis)
{
  int	level;

  one_argument (argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0) {
      GET_INVIS_LEV(ch) = 0;
      sprintf(buf, "You are now fully visible.\r\n");
    } else {
      GET_INVIS_LEV(ch) = GET_LEVEL(ch);
      sprintf(buf, "Your invisibility level is %d.\r\n", GET_LEVEL(ch));
    }
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch)) {
      send_to_char("You can't go invisible above your own level.\r\n", ch);
      return;
    } else if (level < 1) {
      GET_INVIS_LEV(ch) = 0;
      sprintf(buf, "You are now fully visible.\r\n");
    } else {
      GET_INVIS_LEV(ch) = level;
      sprintf(buf, "Your invisibility level is now %d.\r\n", level);
    }
  }
  send_to_char(buf, ch);
  aff_to_screen(ch->desc);
  save_char(ch, world[ch->in_room]->number, 2);
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);

  if (!*argument)
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    sprintf(buf, "%s\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character && pt->character != ch)
	act(buf, FALSE, ch, 0, pt->character, TO_VICT | TO_SLEEP);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Ok.\r\n", ch);
    else
      send_to_char(buf, ch);
    sprintf(buf, "(GC) %s gechoed '%s#G'",GET_NAME(ch), argument);
    mudlog(buf, CMP, LEVEL_GREATER, TRUE);
  }
}


ACMD(do_poofset)
{
  char	**msg;
  int pooftype;

  half_chop(argument, arg, buf1);

  if (!*arg) {
    send_to_char("Usage:- poof <in|out|view>\r\n", ch);
    return;
  }

  if (!str_cmp(arg, "in")) pooftype = SCMD_POOFIN;
  else if (!str_cmp(arg, "out")) pooftype = SCMD_POOFOUT;
  else if (!str_cmp(arg, "view")) pooftype = SCMD_POOFVIEW;
  else {
    send_to_char("Usage:- poof <in|out|view>\r\n", ch);
    return;
  }

  switch (pooftype) {
  case SCMD_POOFIN:
    msg = &(ch->specials.poofIn);
    break;
  case SCMD_POOFOUT:
    msg = &(ch->specials.poofOut);
    break;
  case SCMD_POOFVIEW:
    if (ch->specials.poofIn != NULL)
      sprintf(buf, "PoofIn : %s %s\r\n", GET_NAME(ch), ch->specials.poofIn);
    else
      sprintf(buf, "PoofIn : %s appears with an ear-splitting bang.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
    if (ch->specials.poofOut != NULL)
      sprintf(buf, "PoofOut: %s %s\r\n", GET_NAME(ch), ch->specials.poofOut);
    else
      sprintf(buf, "PoofOut: %s disappears in a puff of smoke.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
    return;
    break;
  default:
    send_to_char("Unknown option. Usage: poof <in|out|view>\r\n", ch);
    return;
    break;
  }

  if (*msg)
    free(*msg);

  if (!*buf1)
    *msg = NULL;
  else
    *msg = strdup(buf1);

  send_to_char("Ok.\r\n", ch);

  SET_BIT(PLR_FLAGS(ch), PLR_SAVESTR);
}





ACMD(do_dc)
{
  struct descriptor_data *d;
  int	num_to_dc;

  if (!(num_to_dc = atoi(argument))) {
    send_to_char("Usage: DC <connection number> (type USERS for a list)\r\n", ch);
    return;
  }

  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }

  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    return;
  }

  close_socket(d);
  sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
  send_to_char(buf, ch);
  sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
  log(buf);
}



ACMD(do_wizlock)
{
  int	value;
  char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > LEVEL_IMPL) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (restrict) {
  case 0 :
    sprintf(buf, "The game is %s completely open.", when);
    break;
  case 1 :
    sprintf(buf, "The game is %s closed to new players.", when);
    break;
  default :
    sprintf(buf, "Only level %d and above may enter the game %s.",
	    restrict, when);
    break;
  }
  if (!str_cmp("now", when))
    mudlog(buf, BRF, LEVEL_IMMORT, TRUE);
  sprintf(buf + strlen(buf), "\r\n");
  send_to_char(buf, ch);

}


ACMD(do_date)
{
  long	ct;
  char	*tmstr;

  ct = time(0);
  tmstr = (char *)asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf, "Current machine time: %s\r\n", tmstr);
  send_to_char(buf, ch);
}



ACMD(do_uptime)
{
  char	*tmstr;
  long	uptime;
  int	d, h, m;

  extern long	boot_time;

  tmstr = (char *)asctime(localtime(&boot_time));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  uptime = time(0) - boot_time;
  d = uptime / 86400;
  h = (uptime / 3600) % 24;
  m = (uptime / 60) % 60;

  sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
	  ((d == 1) ? "" : "s"), h, m);

  send_to_char(buf, ch);
}



ACMD(do_last)
{
  struct char_file_u chdata;
  extern char	*class_abbrevs[];

  if (!*argument) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (load_char(arg, &chdata) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }

  /* last will now work for all players */
  if ( (GET_LEVEL(ch) < LEVEL_IMMORT) || (chdata.level >= GET_LEVEL(ch)))
    sprintf(buf, "%s was last logged in on %-20s\r\n",
	    chdata.name, ctime(&chdata.last_logon));
  else
    sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
	    chdata.specials2.idnum, chdata.level, class_abbrevs[(int)chdata.class],
	    chdata.name, chdata.host, ctime(&chdata.last_logon));
  send_to_char(buf, ch);
}


ACMD(do_force)
{
  struct descriptor_data *i;
  struct char_data *vict;
  char	name[100], to_force[MAX_INPUT_LENGTH+2], *ptr;

  half_chop(argument, name, to_force);

  /* NO FORCE WORSHIP */
  ptr = to_force;
  skip_spaces(&ptr);
  if (!strn_cmp(ptr, "worship", 7)) {
    send_to_char("Forcing players to worship is illegal.  This has been reported.\r\n", ch);
    return;
  } /* **********  */

  /* NO FORCE SHUTDOWN */
  if (!strn_cmp(ptr, "shutdow", 7)) {
    send_to_char("Forcing a shutdown is illegal.  This has been reported.\r\n", ch);
    sprintf(buf, "(GC) %s tried to force %s to %s", GET_NAME(ch), name, to_force);
    mudlog(buf, NRM, MAX(LEVEL_ADMIN, GET_LEVEL(ch)), TRUE);
    return;
  }

  sprintf(buf1, "%s has forced you to %s.\r\n", GET_NAME(ch), to_force);
  sprintf(buf2, "Someone has forced you to %s.\r\n", to_force);

  if (!*name || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if (str_cmp("all", name) && str_cmp("room", name)) {
    if (!(vict = get_char_vis(ch, name)) || !CAN_SEE(ch, vict))
      send_to_char("No-one by that name here...\r\n", ch);
    else {
      if (GET_LEVEL(ch) > GET_LEVEL(vict)) {
	send_to_char("Ok.\r\n", ch);
	if (CAN_SEE(vict, ch) && GET_LEVEL(ch) < LEVEL_ADMIN)
	  send_to_char(buf1, vict);
	else if (GET_LEVEL(ch) < LEVEL_ADMIN) {
	  send_to_char(buf2, vict);
	}
	if (GET_LEVEL(ch) < LEVEL_ADMIN) {
	  sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), name, to_force);
	  mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_LEVEL(ch)), TRUE);

	}
	command_interpreter(vict, to_force);
      } else
	send_to_char("No, no, no!\r\n", ch);
    }
  } else if (str_cmp("room", name)) {

    if (GET_LEVEL(ch) < LEVEL_ADMIN) {
      send_to_char("You can't do that!!!\r\n", ch);
      return;
    }
    send_to_char("Okay.\r\n", ch);
    sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), name, to_force);
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_LEVEL(ch)), TRUE);
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected) {
	vict = i->character;
	if (GET_LEVEL(ch) > GET_LEVEL(vict)) {
	  if (CAN_SEE(vict, ch) && GET_LEVEL(ch) < LEVEL_ADMIN)
	    send_to_char(buf1, vict);
	  else if (GET_LEVEL(ch) < LEVEL_ADMIN)
	    send_to_char(buf2, vict);
	  command_interpreter(vict, to_force);
	}
      }
  } else {
    if (GET_LEVEL(ch) < LEVEL_ADMIN) {
      send_to_char("You can't do that!!!\r\n", ch);
      return;
    }

    send_to_char("Okay.\r\n", ch);
    sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), name, to_force);
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_LEVEL(ch)), TRUE);
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  i->character->in_room == ch->in_room) {
	vict = i->character;
	if (GET_LEVEL(ch) > GET_LEVEL(vict)) {
	  if (CAN_SEE(vict, ch) && GET_LEVEL(ch) < LEVEL_ADMIN)
	    send_to_char(buf1, vict);
	  else if (GET_LEVEL(ch) < LEVEL_ADMIN)
	    send_to_char(buf2, vict);
	  command_interpreter(vict, to_force);
	}
      }
  }
}



ACMD(do_wiznet)
{
  struct descriptor_data *d;
  char	emote = FALSE;
  char	any = FALSE;
  int	level = LEVEL_DEITY;
  int ok;
  struct char_data *dest;       /* To inherit state from original */

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    print_history(ch, CHAN_WIZLINE);
    return;
  }

  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LEVEL_DEITY);
      if (level > GET_LEVEL(ch)) {
	send_to_char("You can't wizline above your own level.\r\n", ch);
	return;
      }
    } else if (emote)
      argument++;
    break;
  case '@':
    *buf1 = '\0';
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected) {
        if (d->original)
          dest = d->original;
        else
          dest = d->character;
        if (GET_LEVEL(dest) >= LEVEL_DEITY &&
	    !PRF_FLAGGED(dest, PRF_NOWIZ) &&
	    (CAN_SEE(ch, dest) || GET_LEVEL(ch) >= LEVEL_IMPL) ) {
  	  if (!any) {
	    sprintf(buf1, "Gods online:\r\n");
	    any = TRUE;
  	  }
	  sprintf(buf1, "%s  %s", buf1, GET_NAME(dest));
	  if (PLR_FLAGGED(dest, PLR_WRITING))
	    sprintf(buf1, "%s (Writing)\r\n", buf1);
	  else if (PLR_FLAGGED(dest, PLR_MAILING))
	    sprintf(buf1, "%s (Writing mail)\r\n", buf1);
          else if (d->original)
            sprintf(buf1, "%s (Switched: %s)\r\n", buf1, GET_NAME(d->character));
	  else
	    sprintf(buf1, "%s\r\n", buf1);
        }
      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected) {
        if (d->original)
          dest = d->original;
        else
          dest = d->character;
        if (GET_LEVEL(dest) >= LEVEL_DEITY && PRF_FLAGGED(dest, PRF_NOWIZ) &&
	    CAN_SEE(ch, dest) ) {
  	  if (!any) {
	    sprintf(buf1, "%sGods offline:\r\n", buf1);
	    any = TRUE;
	  }
	  sprintf(buf1, "%s  %s\r\n", buf1, GET_NAME(dest));
        }
      }
    }
    send_to_char(buf1, ch);
    return;
    break;
  case '-':
    if (PRF_FLAGGED(ch, PRF_NOWIZ))
      send_to_char("You are already offline!\r\n", ch);
    else {
      send_to_char("You will no longer hear the wizline.\r\n", ch);
      SET_BIT(PRF_FLAGS(ch), PRF_NOWIZ);
    }
    return;
    break;
  case '+':
    if (!PRF_FLAGGED(ch, PRF_NOWIZ))
      send_to_char("You are already online!\r\n", ch);
    else {
      send_to_char("You can now hear the wizline again.\r\n", ch);
      REMOVE_BIT(PRF_FLAGS(ch), PRF_NOWIZ);
    }
    return;
    break;
  case '\\':
    ++argument;
    break;
  case '?':
       send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
		    "       wiznet @<level> | wiz @ | wiz - | wiz +\r\n", ch);
       return;
       break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }

  for ( ; *argument == ' '; argument++);

  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }

  if (level > LEVEL_DEITY) {
    sprintf(buf1, "§1C%s: <§1w%d§1C> %s%s#N\r\n", GET_NAME(ch), level,
	    emote ? "<--- " : "", argument);
    sprintf(buf2, "§1CSomeone: <§1w%d§1C> %s%s#N\r\n", level, emote ? "<--- " : "", argument);
  } else {
    sprintf(buf1, "§1C%s: %s%s#N\r\n", GET_NAME(ch), emote ? "<--- " : "",
	    argument);
    sprintf(buf2, "§1CSomeone: %s%s#N\r\n", emote ? "<--- " : "", argument);
  }

  chan_history(argument, ch, NULL, level, CHAN_WIZLINE);

  for (d = descriptor_list; d; d = d->next) {
    ok = 0;
    if (!d->connected) {
      if (d->original) {
        /* If switched, then get prefs and info from the original char,
         * and check level based on the original char */
        ok = 1;
        dest = d->original;
      } else if (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING)) {
        ok = 1;
        dest = d->character;
      }
    }

    if (ok && (GET_LEVEL(dest) >= level) &&
        !PRF_FLAGGED(dest, PRF_NOWIZ) &&
        !PLR_FLAGGED(dest, PLR_WRITING) &&
        ((d != ch->desc) || !PRF_FLAGGED(dest, PRF_NOREPEAT))) {
      if (CAN_SEE(dest, ch))
        send_to_char(buf1, d->character);
      else
        send_to_char(buf2, d->character);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Ok.\r\n", ch);
}

ACMD(do_zpurge)
{
     int i = 0, zone = 0, location = 0;
     struct char_data *vict, *next_v;
     struct obj_data *obj, *next_o;

     skip_spaces(&argument);

     if (!*argument) {
	  send_to_char("Usage: zpurge <zone | . = current zone>.\r\n", ch);
	  return;
     }

     if (*argument == '.')
        zone = world[ch->in_room]->zone;
     else {
        i = atoi(argument);
        zone = real_zone(i);
     }

     if (zone > -1) {
	  sprintf(buf, "Cleaning zone %s... ", zone_table[zone].name);
	  send_to_char(buf, ch);

	  for (i = world[zone_table[zone].lowest]->number;
	       i <= world[zone_table[zone].highest]->number;
	       i++) {
	       if ((location = real_room(i)) < 0)
		    continue;

	       if (ROOM_FLAGGED(location, NO_SWEEP))
		    continue;

	       send_to_room("The world seems a little cleaner.\r\n", location);

	       /* clean the room */
	       for (vict = world[location]->people; vict; vict = next_v) {
		    next_v = vict->next_in_room;
		    if (IS_NPC(vict))
			 extract_char(vict);
	       }

	       for (obj = world[location]->contents; obj; obj = next_o) {
		    next_o = obj->next_content;

		    /* don't kill nosweep items */
		    if(!IS_OBJ_STAT(obj, ITEM_NOSWEEP))
			 extract_obj(obj);
	       }
	  }
	  send_to_char("Done.\r\n", ch);
          sprintf(buf, "(GC) Zpurge: %s has purged zone %s", GET_NAME(ch), zone_table[zone].name);
          mudlog(buf, NRM, GET_INVIS_LEV(ch), TRUE);
     }
}

ACMD(do_zreset)
{
  int	i, j;
  bool type = FALSE;

  if (!*argument) {
    send_to_char("Usage: zreset [all | sim] < * | . | zone >.\r\n", ch);
    send_to_char("all = 100% repop (default) | sim = simulated repop\r\n", ch);
    send_to_char(" *  = all zones            |  .  = current zone\r\n", ch);
    return;
  }

  half_chop(argument, arg, buf);

  if (*buf) {
       if (!strcmp(arg, "all"))
	    type = TRUE;
       if (!strcmp(arg, "sim"))
	    type = FALSE;
       half_chop(buf, arg, buf);
       if (!*arg) {
	    send_to_char("You need to specify a zone.\r\n", ch);
	    return;
       }
  }

  if (*arg == '*') {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i, type);
    send_to_char("Reset world.\r\n", ch);
    sprintf(buf, "(GC) %s reset world.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LEVEL_GREATER, GET_INVIS_LEV(ch)), TRUE);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room]->zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i >= 0 && i <= top_of_zone_table) {
    reset_zone(i, type);
    sprintf(buf, "Reset zone %d: %s.\r\n", i, zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
    mudlog(buf, NRM, MAX(LEVEL_GREATER, GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


/*
   General fn for wizcommands of the sort: cmd <player>
   */

ACMD(do_wizutil)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH];
  long	result;

  one_argument(argument, name);
  if (!*name) {
    send_to_char("Yes, but for whom?!?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char("You can't do that to a mob!\r\n", ch);
    return;
  }
  if (GET_LEVEL(vict) >= GET_LEVEL(ch) && GET_LEVEL(ch) < LEVEL_IMPL) {
    send_to_char("Hmmm...you'd better not.\r\n", ch);
    return;
  }

  switch (subcmd) {
  case SCMD_PARDON:
    if (!PLR_FLAGGED(vict, PLR_THIEF | PLR_KILLER)) {
      send_to_char("Your victim is not flagged.\r\n", ch);
      return;
    }
    REMOVE_BIT(PLR_FLAGS(vict), PLR_THIEF | PLR_KILLER);
    send_to_char("Pardoned.\r\n", ch);
    send_to_char("You have been pardoned by the Gods!\r\n", vict);
    sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    break;
  case SCMD_NOTITLE:
    result = PLR_TOG_CHK(vict, PLR_NOTITLE);
    sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
	    GET_NAME(vict), GET_NAME(ch));
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    break;
  case SCMD_SQUELCH:
    result = PLR_TOG_CHK(vict, PLR_MUTE);
    sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
	    GET_NAME(vict), GET_NAME(ch));
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    break;
  case SCMD_FREEZE:
    if (ch == vict) {
      send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
      return;
    }
    if (PLR_FLAGGED(vict, PLR_FROZEN)) {
      send_to_char("Your victim is already pretty cold.\r\n", ch);
      return;
    }
    SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
    vict->specials2.freeze_level = GET_LEVEL(ch);
    send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n",
		 vict);
    send_to_char("Frozen.\r\n", ch);
    act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
    sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    break;
  case SCMD_THAW:
    if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
      send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
      return;
    }
    if (vict->specials2.freeze_level > GET_LEVEL(ch)) {
      sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	      vict->specials2.freeze_level,
	      GET_NAME(vict),
	      HMHR(vict));
      send_to_char(buf, ch);
      return;
    }
    sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
    mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
    send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
    send_to_char("Thawed.\r\n", ch);
    act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
    break;
  case SCMD_UNAFFECT:
    if (vict->affected) {
      while (vict->affected)
	affect_remove(vict, vict->affected);
      send_to_char("All spells removed.\r\n", ch);
      send_to_char("There is a brief flash of light!\r\n"
		   "You feel slightly different.\r\n", vict);
    } else {
      send_to_char("Your victim does not have any affections!\r\n", ch);
      return;
    }
    break;
  case SCMD_REROLL:
    send_to_char("Rerolled...\r\n", ch);
    roll_abilities(vict);
    sprintf(buf, "(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
    log(buf);
    break;
  }
  save_char(vict, IN_VROOM(vict), 2);
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void	print_zone_to_buf(char *bufptr, int zone)
{
  sprintf(bufptr, "%s%3d #C%-30.30s#N Age:#Y%3d#N Reset:#R%3d#N (#m%1d#N)  %5d-%-5d\r\n",
          bufptr, zone_table[zone].number, zone_table[zone].name,
          zone_table[zone].age, zone_table[zone].lifespan,
          zone_table[zone].reset_mode,
          world[zone_table[zone].lowest]->number,
          world[zone_table[zone].highest]->number);
}

/* Show room to char: help function to show dt|godroom|ocs| etc */
void    print_rooms_to_buf(char *buf, long flag)
{
  int i, j;

  for (i = 0, j = 0; i < top_of_world; i++)
    if (IS_SET(world[i]->room_flags, flag))
      sprintf(buf, "%s#w%2d#N: [#B%5d#N] #c%s#N\r\n", buf, ++j,
	      world[i]->number, world[i]->name);
}


void show_ident(struct char_data *ch, int mode)
{
  struct descriptor_data *d;
  char *format;
  extern char *ident_types[];
  char hostnme[HOST_LEN+1];

  format = "%3d %-12s %-14s %-25s %-18s\r\n";
  sprintf(buf, "Num Name         State          Ident                     Site\r\n");
  strcat(buf, "-=- -=-=-=-=-=-= -=-=-=-=-=-=-= -=-=-=-=-=-=-=-=-=-=-=-=- =-=-=-=-=-=-=-=-=-\r\n");
  for (d = descriptor_list; d; d = d->next) {
    if (d->host && *d->host)
      sprintf(hostnme, "[%s]", d->host);
    else
      sprintf(hostnme, "[Unknown Host]");

    if (d->character && d->character->player.name) {
      if (d->original)
	sprintf(buf2, format,
		d->desc_num,
		d->original->player.name,
		ident_types[d->ident_state],
		d->ident_name,
		hostnme);
      else
	sprintf(buf2, format,
		d->desc_num,
		d->character->player.name,
		ident_types[d->ident_state],
		d->ident_name,
		hostnme);
    } else
      sprintf(buf2, format,
	      d->desc_num,
	      "UNDEFINED",
	      ident_types[d->ident_state],
	      d->ident_name,
	      hostnme);

    if (strlen(buf) < MAX_STRING_LENGTH - 200)
      strcat(buf, buf2);
  }

  if (strlen(buf) >= MAX_STRING_LENGTH - 200)
    strcat(buf, "String Length Exceeded\r\n");

  page_string(ch->desc, buf, TRUE);
}


ACMD(do_show)
{
  int	i, j, k, l, con;
  long clan = 0;
  char	self = 0;
  struct descriptor_data *d;
  struct char_data *vict;
  struct char_file_u tmp_store;
  struct obj_data *obj;
  char	field[40], value[40], hostnme[HOST_LEN+1];
  struct room_list *crash_rooms;
  extern int buf_switches, buf_largecount, buf_overflows, newbie_clan;
  extern struct room_list *room_crash_list;
  static int new = 0, lev2_9 = 0, lev10_29 = 0, lev30_59 = 0,
  lev60_99 = 0, lev100 = 0, lev110 = 0, worshippers = 0;

  struct show_struct {
    char	*cmd;
    long        godlevel;
  } fields[] = {
    { "nothing",  0 },
    { "zones", 	  IMM_WORLD }, /* 1 */
    { "player",   IMM_BASIC },
    { "rent",     IMM_STD  },
    { "stats", 	  IMM_BASIC },
    { "passwd",   0 },
    { "death", 	  IMM_STD  },
    { "godrooms", IMM_BASIC },
    { "ideas",    IMM_WORLD },
    { "bugs",     IMM_WORLD },
    { "typos",    IMM_WORLD }, /* 10 */
    { "crooms",   IMM_STD  },
    { "clans",    IMM_CLAN  },
    { "private",  IMM_STD  },
    { "silent",   IMM_STD  },
    { "ocs",      IMM_STD  },
    { "locker",   IMM_STD  },
    { "ident",    0 },
    { "siteok",   0 },
    { "plog",     IMM_STD  },
    { "pkok",     IMM_PK | IMM_ADMIN }, /* 20 */
    { "immortal", IMM_OVERSEER },
    { "\n", 0 }
  };


  if (!*argument) {
    strcpy(buf, "Show options:\r\n\r\n");       i = 0;
    l = 1; i = 0;
    while (*(fields[l].cmd) != '\n') {
      if (IS_SET(GODLEVEL(ch), IMM_ALL) ||
	  (GODLEVEL(ch) & fields[l].godlevel)) {
	i++;
	sprintf(buf, "%s [%-12s]", buf, fields[l].cmd);
	if (!(i % 4))
	  strcat (buf, "\r\n");
      }
      l++;
    }
    if (i % 4)
      strcat (buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  half_chop(argument, field, arg);
  half_chop(arg, value, arg);

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (!IS_SET(GODLEVEL(ch), IMM_ALL))
    if (!(GODLEVEL(ch) & fields[l].godlevel)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return;
    }

  if (!strcmp(value, "."))
    self = 1;
  buf[0] = '\0';
  switch (l) {
  case 1:			/* zone */
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf, world[ch->in_room]->zone);
    else if (is_number(value)) {
      for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
      if (i <= top_of_zone_table)
        print_zone_to_buf(buf, i);
      else {
        send_to_char("That is not a valid zone.\r\n", ch);
        return;
      }
    } else
         for (i = 0; i <= top_of_zone_table; i++)
           sprintf(buf, "%s%3d. #C%s#N\r\n", buf, zone_table[i].number, zone_table[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 2:			/* player */
    if (lev100 == 0) {
      for (i = 0; i <= top_of_p_table; i++) {
	l = player_table[i].level;

	if (l == 1)
	  new++;
	else if (l < 10)
	  lev2_9++;
	else if (l < 30)
	  lev10_29++;
	else if (l < 60)
	  lev30_59++;
	else if (l < 100)
	  lev60_99++;
	else if (l < 110)
	  lev100++;
	else
	  lev110++;

	if (player_table[i].worships)
	  worshippers++;

      }
    }

    strcpy(buf, "Player information:\r\n");
    sprintf(buf, "%s   #gNewbies#N     : %5.2f%%", buf, (float)new/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #gLevel  2-9#N  : %5.2f%%\r\n", buf, (float)lev2_9/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #bLevel 10-29#N : %5.2f%%", buf, (float)lev10_29/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #bLevel 30-59#N : %5.2f%%\r\n", buf, (float)lev30_59/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #mLevel 60-99#N : %5.2f%%", buf, (float)lev60_99/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #mLevel 100+#N  : %5.2f%%\r\n", buf, (float)lev100/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #rLevel 110+#N  : %5.2f%%\r\n", buf, (float)lev110/(float)(top_of_p_table + 1)*100.0);
    sprintf(buf, "%s   #cWorshippers#N : %2.2f%%\r\n", buf, (float)worshippers/(float)(top_of_p_table + 1)*100.0);
    send_to_char(buf, ch);
    break;
  case 3:
    Crash_listrent(ch, value);
    break;
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    sprintf(buf, "#rSystem stats:#N\r\n");
    sprintf(buf, "%s  #w%5d#N players in game  #w%5d#N connected        #w%5d#N registered\r\n", buf, i, con, top_of_p_table + 1);
    sprintf(buf, "%s  #w%5d#N commands/sec     #w%5d#N playercmds/sec   #w%5.2f#N cmds/player_sec\r\n", buf, cmds_per_sec, plr_cmds_per_sec, (float)plr_cmds_per_sec/(float)con);
    sprintf(buf, "%s  #w%5d#N large bufs       #w%5d#N buf switches     #w%5d#N overflows\r\n", buf, buf_largecount, buf_switches, buf_overflows);
    sprintf(buf, "%s #w%5.2f%%#N of max efficiency is used.  (>100%% = System lag)\r\n", buf, elite_efficiency*100.0);

    sprintf(buf, "%s#rWorld stats:#N\r\n", buf);
    sprintf(buf, "%s  #w%5d#N mobiles          #w%5d#N prototypes\r\n",
	    buf, j, top_of_mobt + 1);
    sprintf(buf, "%s  #w%5d#N objects          #w%5d#N prototypes\r\n",
	    buf, k, top_of_objt + 1);
    sprintf(buf, "%s  #w%5d#N rooms            #w%5d#N ocs rooms        #w%5d#N used ocs bufs (%d)\r\n",
	    buf, top_of_world + 1, ocs_rooms, ocs_room_buffer, OCS_ROOMS);
    sprintf(buf, "%s  #w%5d#N zones\r\n",
	    buf, top_of_zone_table + 1);
    sprintf(buf, "%s  #w%5d#N clans\r\n",
	    buf, top_of_clan + 1);
    send_to_char(buf, ch);
    break;
  case 5:
       if (GET_LEVEL(ch) < LEVEL_IMPL) return;
       CREATE(vict, struct char_data, 1);
       clear_char(vict);

       if ((i = load_char(value, &tmp_store)) > -1) {
	    store_to_char(&tmp_store, vict);
       } else {
	    free(vict);
	    send_to_char("There is no such player.\r\n", ch);
	    return;
       }

    sprintf(buf, "%s's Password: \"%s\"\r\n", GET_NAME(vict), GET_PASSWD(vict));
    send_to_char(buf, ch);
    break;
  case 6:
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    print_rooms_to_buf(buf, DEATH);
    page_string(ch->desc, buf, TRUE);
    break;
  case 7:
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    print_rooms_to_buf(buf, GODROOM);
    page_string(ch->desc, buf, TRUE);
    break;
  case 8:
/*    file_to_string_alloc(IDEAS_FILE, &ideas); */
    page_string(ch->desc, ideas, FALSE);
    break;
  case 9:
/*    file_to_string_alloc(BUGS_FILE, &bugs); */
    page_string(ch->desc, bugs, FALSE);
    break;
  case 10:
/*    file_to_string_alloc(TYPOS_FILE, &typos); */
    page_string(ch->desc, typos, FALSE);
    break;
  case 11:
    strcpy(buf, "Crash rooms\r\n-----------\r\n");
    crash_rooms = room_crash_list;
    while (crash_rooms) {
      sprintf(buf, "%s [#G%5d#N] #R%s#N\r\n", buf,
	      world[crash_rooms->number]->number,
	      world[crash_rooms->number]->name);
      crash_rooms = crash_rooms->next;
    }
    page_string(ch->desc, buf, TRUE);
    break;
  case 12:			/* clans */
    if (!strcmp(value, "reset")) {
      for (i = 0; i <= top_of_clan; i++) {
	clan_list[i].on_power_rec = 0;
      }

      send_to_char("Clan Power stats reset.\r\n", ch);
      break;
    }

    sprintf(buf, "Elite Mud Online Clan Statistics (Recorded):\r\n");
    strcat(buf, "     #bActivity  #wClan#N\r\n");
    strcat(buf, "=-=-=-=-=-=-=  =-=-\r\n");

    for (i = 0; i <= top_of_clan; i++)
      if (clan_list[i].vnum != newbie_clan)
	clan += clan_list[i].on_power_rec;

    for (i = 0; i <= top_of_clan; i++)
      if (clan_list[i].vnum != newbie_clan) {
	sprintf(buf2, "%7ld(%3ld%%)  %3d. %s\r\n", clan_list[i].on_power_rec,
		clan ? (100 * clan_list[i].on_power_rec / clan): 0,
		clan_list[i].vnum, clan_list[i].name);
	strcat(buf, buf2);
      } else {
	sprintf(buf1, "%7ld( N/A)  %3d. %s\r\n", clan_list[i].on_power_rec,
		clan_list[i].vnum, clan_list[i].name);
      }

    strcat(buf, "\r\n");
    strcat(buf, buf1);
    page_string(ch->desc, buf, TRUE);

    break;
  case 13:
    strcpy(buf, "Private rooms\r\n--------------------------\r\n");
    print_rooms_to_buf(buf, PRIVATE);
    page_string(ch->desc, buf, TRUE);
    break;
  case 14:
    strcpy(buf, "Silent rooms\r\n--------------------------\r\n");
    print_rooms_to_buf(buf, SILENT);
    page_string(ch->desc, buf, TRUE);
    break;
  case 15:
    strcpy(buf, "OCS rooms\r\n--------------------------\r\n");
    print_rooms_to_buf(buf, OCS);
    page_string(ch->desc, buf, TRUE);
    break;
  case 16:
    Crash_listlocker(ch, value);
    break;
  case 17:
    show_ident(ch, -1);
    break;
  case 18:
    send_to_char("SITEOK\r\n------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (d->character && d->character->player.name)
	if (PLR_FLAGGED(d->character, PLR_SITEOK)) {
	  if (d->host && *d->host)
	    sprintf(hostnme, "[%s]", d->host);
	  else
	    sprintf(hostnme, "[Hostname unknown]");

	  if (d->original)
	    sprintf(buf, "%3d %-12s %s\r\n",
		    d->desc_num, d->original->player.name, hostnme);
	  else
	    sprintf(buf, "%d %-12s %s\r\n",
		    d->desc_num, d->character->player.name, hostnme);
	  send_to_char(buf, ch);
	}

    break;
  case 19:
    list_plog(ch, value);
    break;
  case 20:
    send_to_char("PKOK Players Online\r\n--------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (d->character && d->character->player.name)
	if (PLR_FLAGGED(d->character, PLR_PKOK)) {

	  if (d->original)
	    sprintf(buf, "%3d %-12s\r\n",
		    d->desc_num, d->original->player.name);
	  else
	    sprintf(buf, "%3d %-12s\r\n",
		    d->desc_num, d->character->player.name);
	  send_to_char(buf, ch);
	}

    break;
  case 21:
    *buf = '\0';
    send_to_char("Immortals\r\n---------\r\n", ch);
    for (i = 0; i <= top_of_p_table; i++) {
      l = player_table[i].level;

      if (l >= LEVEL_DEITY) {
        strcpy(buf2, player_table[i].name);
        CAP(buf2);
        sprintf(buf, "%s%-20s %-3d\r\n", buf, buf2, l);
      }
    }
    page_string(ch->desc, buf, TRUE);
    break;

  default:
    send_to_char("Sorry, I don't understand that.\r\n", ch);
    break;
  }
}



/* ******************************** do_set ****************************** */

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
					  if (on) SET_BIT(flagset, flags); \
					  else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))

struct set_struct {
    char	*cmd;
    long	godlevel;
    char	pcnpc;
    char	type;
  }
set_fields[] =
{
  { "brief", 	    IMM_BASIC              , PC   , BINARY },
  /* 1 */
  { "invstart",     IMM_STD                , PC   , BINARY },
  { "title",        IMM_BASIC              , PC   , MISC   },
  { "nosummon",     IMM_BASIC              , PC   , BINARY },
  { "maxhit", 	    0                      , BOTH , NUMBER },
  { "maxmana", 	    0                      , BOTH , NUMBER },
  { "maxmove", 	    0                      , BOTH , NUMBER },
  { "hit", 	    0                      , BOTH , NUMBER },
  { "mana", 	    0                      , BOTH , NUMBER },
  { "move", 	    0                      , BOTH , NUMBER },
  { "align", 	    IMM_ADMIN              , BOTH , NUMBER },
  /* 11 */
  { "str", 	    IMM_REMORT             , BOTH , NUMBER },
  { "stradd", 	    IMM_REMORT             , BOTH , NUMBER },
  { "int", 	    IMM_REMORT             , BOTH , NUMBER },
  { "wis", 	    IMM_REMORT             , BOTH , NUMBER },
  { "dex", 	    IMM_REMORT             , BOTH , NUMBER },
  { "con", 	    IMM_REMORT             , BOTH , NUMBER },
  { "cha",          IMM_REMORT             , BOTH , NUMBER },
  { "sex",          IMM_ADMIN              , BOTH , MISC   },
  { "ac", 	    0                      , BOTH , NUMBER },
  { "gold", 	    IMM_CLAN               , BOTH , NUMBER },
  /* 21 */
  { "bank", 	    IMM_CLAN               , BOTH , NUMBER },
  { "exp", 	    IMM_REMORT | IMM_ADMIN , BOTH , NUMBER },
  { "hitroll", 	    0                      , BOTH , NUMBER },
  { "damroll", 	    0                      , BOTH , NUMBER },
  { "invis",        0                      , PC   , NUMBER },
  { "nohassle",     IMM_ADMIN              , PC   , BINARY },
  { "frozen", 	    IMM_ADMIN              , PC   , BINARY },
  { "practices",    IMM_ADMIN              , PC   , NUMBER },
  { "lessons", 	    IMM_ADMIN              , PC   , NUMBER },
  { "drunk", 	    IMM_STD | IMM_QUESTOR  , BOTH , MISC   },
  /* 31 */
  { "hunger", 	    IMM_REMORT             , BOTH , MISC   },
  { "thirst", 	    IMM_REMORT             , BOTH , MISC   },
  { "killer", 	    IMM_ADMIN | IMM_QUESTOR, PC   , BINARY },
  { "thief", 	    IMM_ADMIN              , PC   , BINARY },
  { "level", 	    IMM_ADMIN              , BOTH , NUMBER },
  { "room",         0                      , PC   , NUMBER },
  { "roomflag",     0                      , PC   , BINARY },
  { "siteok", 	    IMM_OVERSEER           , PC   , BINARY },
  { "deleted", 	    IMM_ADMIN              , PC   , BINARY },
  { "class", 	    IMM_REMORT             , BOTH , MISC   },
  /* 41 */
  { "nowizlist",    IMM_OVERSEER           , PC   , BINARY },
  { "questpt", 	    IMM_QUEST              , PC   , NUMBER },
  { "loadroom",     IMM_ADMIN  | IMM_CLAN  , PC   , MISC   },
  { "color", 	    IMM_BASIC              , PC   , BINARY },
  { "idnum", 	    0                      , PC   , NUMBER },
  { "passwd", 	    IMM_OVERSEER           , PC   , MISC   },
  { "nodelete",     IMM_ADMIN              , PC   , BINARY },
  { "race",         IMM_REMORT             , PC   , MISC   },
  { "scrlen",       IMM_BASIC              , PC   , NUMBER },
  { "clan",         IMM_CLAN               , PC   , MISC   },
  /* 51 */
  { "clanlevel",    IMM_BASIC              , PC   , NUMBER },
  { "log",          0                      , PC   , BINARY },
  { "name",         0                      , PC   , MISC   },
  { "cryo",         IMM_OVERSEER           , PC   , BINARY },
  { "class1",       IMM_REMORT             , PC   , MISC   },
  { "class2",       IMM_REMORT             , PC   , MISC   },
  { "class3",       IMM_REMORT             , PC   , MISC   },
  { "remort",       IMM_REMORT             , PC   , NUMBER },
  { "godlevel",     0                      , PC   , NUMBER },
  { "nowho",        0                      , PC   , BINARY },
  /* 61 */
  { "mute",         IMM_ADMIN              , PC   , BINARY },
  { "notitle",      IMM_ADMIN              , PC   , BINARY },
  { "test",         IMM_CODE | IMM_CODER   , PC   , BINARY },
  { "pkok",         IMM_PK | IMM_ADMIN     , PC   , BINARY },
  { "\n",           0                      , BOTH ,   MISC }
};

int perform_set(struct char_data *ch, struct char_data *vict, int mode,
		char *val_arg, int is_file, char *name)
{
  int	on = 0, off = 0, value = 0;
  int i;

  if (GET_LEVEL(ch) < LEVEL_IMPL) {
    if (!IS_NPC(vict) &&
	(GET_LEVEL(ch) < GET_LEVEL(vict)) &&
	(vict != ch) &&
	GET_IDNUM(ch) != 1) {
      send_to_char("Maybe that's not such a great idea...\r\n", ch);
      return 0;
    }
  }

  if (!(IS_SET(GODLEVEL(ch), IMM_ALL)))
    if(!(GODLEVEL(ch) & set_fields[mode].godlevel)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return 0;
    }

  if (IS_NPC(vict) && (!set_fields[mode].pcnpc && NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    return 0;
  } else if (!IS_NPC(vict) && (!set_fields[mode].pcnpc && PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    return 0;
  }

  if (set_fields[mode].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be on or off.\r\n", ch);
      return 0;
    }
  } else if (set_fields[mode].type == NUMBER) {
    value = atoi(val_arg);
  }


  strcpy(buf, "Okay.");
  switch (mode) {
  case 0:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2:
    strcat(val_arg, "#N");
    if (GET_TITLE(vict))
      RECREATE(GET_TITLE(vict), char, strlen(val_arg) + 1);
    else
      CREATE(GET_TITLE(vict), char, strlen(val_arg) + 1);
  /*  if(!(val_arg[0] == '\''))   {
        strcpy(temparg, val_arg);
        val_arg[0] = ' ';
        for(j = 1;j < strlen(val_arg) + 1;j++) {
                val_arg[j] = temparg[j-1];
        }
    }       */
    strcpy(GET_TITLE(vict), val_arg);
    stringdata_save(vict); /* Save into plrstrings file so set file <player> title works */
    sprintf(buf, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
    break;
  case 3:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
    on = !on;			/* so output will be correct */
    break;
  case 4:
    vict->points.max_hit = RANGE(1, 9999);
    affect_total(vict);
    break;
  case 5:
    vict->points.max_mana = RANGE(1, 9999);
    affect_total(vict);
    break;
  case 6:
    vict->points.max_move = RANGE(1, 9999);
    affect_total(vict);
    break;
  case 7:
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 8:
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 9:
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 10:
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 11:
    RANGE(3, str_max[GET_RACE(vict)]);
    vict->abilities.str = value;
    vict->abilities.str_add = 0;
    affect_total(vict);
    break;
  case 12:
    if (value > 0 && vict->abilities.str < 18)
      value = 0;
    else
      RANGE(1, 100);
    vict->abilities.str_add = value;
    affect_total(vict);
    break;
  case 13:
    RANGE(3, int_max[GET_RACE(vict)]);
    vict->abilities.intel = value;
    affect_total(vict);
    break;
  case 14:
    RANGE(3, wis_max[GET_RACE(vict)]);
    vict->abilities.wis = value;
    affect_total(vict);
    break;
  case 15:
    RANGE(3, dex_max[GET_RACE(vict)]);
    vict->abilities.dex = value;
    affect_total(vict);
    break;
  case 16:
    RANGE(3, con_max[GET_RACE(vict)]);
    vict->abilities.con = value;
    affect_total(vict);
    break;
  case 17:
    RANGE(3, cha_max[GET_RACE(vict)]);
    vict->abilities.cha = value;
    affect_total(vict);
    break;
  case 18:
    if (!str_cmp(val_arg, "male"))
      vict->player.sex = SEX_MALE;
    else if (!str_cmp(val_arg, "female"))
      vict->player.sex = SEX_FEMALE;
    else if (!str_cmp(val_arg, "neutral"))
      vict->player.sex = SEX_NEUTRAL;
    else {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      return 0;
    }
    break;
  case 19:
    vict->points.armor = RANGE(-2000, 100);
    affect_total(vict);
    break;
  case 20:
    GET_GOLD(vict) = RANGE(0, INT_MAX);
    break;
  case 21:
    GET_BANK_GOLD(vict) = RANGE(0, INT_MAX);
    break;
  case 22:
    vict->points.exp = RANGE(0, INT_MAX);
    break;
  case 23:
    if(GET_LEVEL(ch) < LEVEL_GREATER)
      vict->points.hitroll = RANGE(-10, 50);
    else
      vict->points.hitroll = RANGE(-10, 100);
    affect_total(vict);
    break;
  case 24:
    if(GET_LEVEL(ch) < LEVEL_GREATER)
      vict->points.damroll = RANGE(-10, 50);
    else
      vict->points.damroll = RANGE(-10, 100);
    affect_total(vict);
    break;
  case 25:
    if (GET_LEVEL(ch) < LEVEL_ADMIN && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return 0;
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 26:
    if (GET_LEVEL(ch) < LEVEL_ADMIN && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 27:
    if (ch == vict) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 28:
  case 29:
    SPELLS_TO_LEARN(vict) = RANGE(0, 250);
    break;
  case 30:
  case 31:
  case 32:
    if (!str_cmp(val_arg, "off")) {

      GET_COND(vict, (mode - 30)) = (char) -1;
      sprintf(buf, "%s's %s now off.", GET_NAME(vict),
	      set_fields[mode].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (mode - 30)) = (char) value;
      sprintf(buf, "%s's %s set to %d.", GET_NAME(vict),
	      set_fields[mode].cmd, value);
    } else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      return 0;
    }
    break;
  case 33:
    if (GET_LEVEL(ch) <= GET_LEVEL(vict) && GET_LEVEL(ch) < LEVEL_IMPL) {
      send_to_char("NO no no!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
    break;
  case 34:
    if (GET_LEVEL(ch) <= GET_LEVEL(vict) && GET_LEVEL(ch) < LEVEL_IMPL) {
      send_to_char("NO no no!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    break;
  case 35:
    if (GET_LEVEL(ch) < LEVEL_IMPL && value > GET_LEVEL(ch)) {
      send_to_char("You cannot raise someone above your own level!\r\n", ch);
      return 0;
    }
    RANGE(0, LEVEL_IMPL);
    vict->player.level = (byte) value;
    break;
  case 36:
    if ((i = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return 0;
    }
    char_from_room(vict);
    char_to_room(vict, i);
    break;
  case 37:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
    break;
  case 38:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 39:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 40:
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("Don't mess with that one!\r\n", ch);
      return 0;
    }
    *buf2 = '\0';

    for (i = 1;i < CLASS_MAX;i++) {
      if (!strncmp(val_arg, pc_class_types[i], 4)) {
	GET_CLASS(vict) = i;
	i = 100;		/* is set! */
	break;
      }
    }
    if (i== 100) {
      if (GET_CLASS(ch) == CLASS_2MULTI) {
	if (GET_1CLASS(ch) < 1) GET_1CLASS(ch) = 1;
	if (GET_2CLASS(ch) < 1) GET_2CLASS(ch) = 1;
      } else
	if (GET_CLASS(ch) == CLASS_3MULTI) {
	  if (GET_1CLASS(ch) < 1) GET_1CLASS(ch) = 1;
	  if (GET_2CLASS(ch) < 1) GET_2CLASS(ch) = 1;
	  if (GET_3CLASS(ch) < 1) GET_3CLASS(ch) = 1;
	}
    } else {
      send_to_char("Must be one of the following:\r\n", ch);
      display_string_array(ch, &pc_class_types[1]);
      return 0;
    }
    break;
  case 41:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 42:
    QUEST_NUM(vict) = RANGE(0, 99999);
    break;
  case 43:
    if (!str_cmp(val_arg, "on"))
      SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
    else if (!str_cmp(val_arg, "off"))
      REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
    else {
      if (real_room(i = atoi(val_arg)) > -1) {
	GET_LOADROOM(vict) = i;
	sprintf(buf, "%s will enter at %d.", GET_NAME(vict),
	        GET_LOADROOM(vict));
      } else
	sprintf(buf, "That room does not exist!");
    }
    break;
  case 44:
    SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
    break;
  case 45:
    if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
      return 0;
    vict->specials2.idnum = value;
    break;
  case 46:
    if (GET_LEVEL(ch) <= GET_LEVEL(vict)) {
      send_to_char("You do not have that authority.\r\n", ch);
      return 0;
    }
    strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);
    *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
    break;
  case 47:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;
  case 48:
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("Don't mess with that one!\r\n", ch);
      return 0;
    }
    *buf2 = '\0';

    for (i = 0;i < RACE_MAX;i++) {
      if (!strncmp(val_arg, race_table[i], 7)) {
	GET_RACE(vict) = i;
        race_config(vict);
	i = 100;		/* is set! */
	break;
      }
    }
    if (i != 100) {
      send_to_char("Must be a valid race:\r\n", ch);
      display_string_array(ch, race_table);
      return 0;
    }
    break;
  case 49:
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("Don't mess with that one!\r\n", ch);
      return 0;
    }
    vict->specials2.scrlen = RANGE(20, 99);
    break;
  case 50:
    if (!str_cmp(val_arg, "off")) {
      CLAN(vict) = -1;
      CLAN_LEVEL(vict) = 0;
      sprintf(buf, "%s doesn't belong to any clan now.", GET_NAME(vict));
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      if ((i = real_clan(value)) >= 0) {
	CLAN(vict) = i;
	sprintf(buf, "%s's clan set to %d.", GET_NAME(vict), value);
      } else {
	send_to_char("Not a valid clan vnum.\r\n", ch);
	return 0;
      }
    } else {
      send_to_char("Must be 'off' or the vnum of the clan.\r\n", ch);
      return 0;
    }
    break;
  case 51:
    if ((GET_LEVEL(ch) >= LEVEL_ADMIN) ||
	IS_SET(GODLEVEL(ch), IMM_CLAN) ||
	((CLAN(ch) == CLAN(vict)) &&
      CLAN_LEVEL(ch) > CLAN_LEVEL(vict)))
	CLAN_LEVEL(vict) = RANGE(1, 10);
    else {
      send_to_char("You can't do that.\r\n", ch);
      return 0;
    }
    break;
  case 52:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_LOG);
    break;
  case 53: /* name */
    if (is_file) {
      send_to_char("Can't be done to a player not in game.\r\n", ch);
      return 0;
    }

    if (IS_NPC(vict)) {
	 send_to_char("Can't be done to mobs\r\n", ch);
	 return 0;
    }

    for (i = 0; i <= top_of_p_table; i++)
	 if (!str_cmp(val_arg, player_table[i].name)) {
	      send_to_char("Name in use.\r\n", ch);
	      return 0l;
	 }

    if (GET_LEVEL(ch) < LEVEL_ADMIN) {
      send_to_char("This function is currently disabled\r\n", ch);
      return 0;
    }

    if (strlen(val_arg) > MAX_NAME_LENGTH || strlen(val_arg) < 2) {
      send_to_char("Name too long or short.\r\n", ch);
      return 0;
    }

    for (i = 0; *(name + i); *(name + i) = LOWER(*(name + i)), i++);

    if (str_cmp(name, player_table[vict->nr].name)) {
      send_to_char("Player name and index name doesn't match!\r\n", ch);
      return 0;
    }

    free(vict->player.name);
    CAP(val_arg);
    vict->player.name = strdup(val_arg);
    /*
     * Name in player table isn't lowercase anymore - Charlene
     * for (i = 0; *(val_arg + i); *(val_arg + i) = LOWER(*(val_arg + i)), i++);
     */
    free(player_table[vict->nr].name);
    player_table[vict->nr].name = strdup(val_arg);
    break;

  case 54:  /* cryo */
    /*if (GET_LEVEL(ch) < LEVEL_GREATER)
      SET_BIT(PLR_FLAGS(vict), PLR_CRYO);
      else*/
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_CRYO);
    break;

  case 55:  /* first multiple class */
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("Don't mess with that one!\r\n", ch);
      return 0;
    }
    *buf2 = '\0';

    for (i = 1;i < CLASS_MAX;i++) {
      if (!strncmp(val_arg, pc_class_types[i], 4)) {
	GET_1CLASS(vict) = i;
	i = 100;		/* is set! */
	break;
      }
    }
    if (i != 100) {
      send_to_char("Must be one of the following:\r\n", ch);
      display_string_array(ch, &pc_class_types[1]);
      return 0;
    }
    break;

  case 56:  /* second multiple class */
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("Don't mess with that one!\r\n", ch);
      return 0;
    }
    *buf2 = '\0';

    for (i = 1;i < CLASS_MAX;i++) {
      if (!strncmp(val_arg, pc_class_types[i], 4)) {
	GET_2CLASS(vict) = i;
	i = 100;		/* is set! */
	break;
      }
    }
    if (i != 100) {
      send_to_char("Must be one of the following:\r\n", ch);
      display_string_array(ch, &pc_class_types[1]);
      return 0;
    }
    break;

  case 57:  /* third multiple class */
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("Don't mess with that one!\r\n", ch);
      return 0;
    }
    *buf2 = '\0';

    for (i = 1;i < CLASS_MAX;i++) {
      if (!strncmp(val_arg, pc_class_types[i], 4)) {
	GET_3CLASS(vict) = i;
	i = 100;		/* is set! */
	break;
      }
    }
    if (i != 100) {
      send_to_char("Must be one of the following:\r\n", ch);
      display_string_array(ch, &pc_class_types[1]);
      return 0;
    }
    break;

  case 58:  /* How many times remorted */
    REMORT(vict) = RANGE(0,5);
    break;

  case 59:  /* What godpowers this immortal has*/
    if (GET_LEVEL(ch) < LEVEL_ADMIN) return 0;
      GODLEVEL(vict) = value;
    break;

  case 60: /* Set or remove nowho flag */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWHO);
    break;

  case 61: /* Set or remove mute flag */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_MUTE);
    break;

  case 62: /* Set or remove notitle flag */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOTITLE);
    break;

  case 63: /* Set or remove test flag */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_TEST);
    break;

  case 64: /* Set or remove pkok flag */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_PKOK);
    break;

  default:
    send_to_char("Can't set that!\r\n", ch);
    return 0;
  }

  if (set_fields[mode].type == BINARY) {
    sprintf(buf, "%s %s for %s.", set_fields[mode].cmd, ONOFF(on),
	    GET_NAME(vict));
    CAP(buf);
  } else if (set_fields[mode].type == NUMBER) {
    sprintf(buf, "%s's %s set to %d.", GET_NAME(vict), set_fields[mode].cmd, value);
  } else
    sprintf(buf, "%s's %s set to %s.", GET_NAME(vict), set_fields[mode].cmd, val_arg);

  sprintf(buf2, "(GC) %s: %s", GET_NAME(ch), buf);
  mudlog(buf2, BRF, GET_LEVEL(ch), TRUE);

  strcat(buf, "\r\n");
  send_to_char(buf, ch);


  return 1;
}

void race_config(struct char_data *vict)
{
  struct affected_type *aff;
  struct affected_type af;


  SET_SKILL(vict, SKILL_TAIL_LASH, 0);
  SET_SKILL(vict, SKILL_CLAW, 0);
  SET_SKILL(vict, SKILL_POUNCE, 0);

  if (GET_SKILL(vict, SKILL_SNEAK) == 95) {
    if (!IS_THIEF(vict) && !IS_ASSASSIN(vict) && !IS_BARD(vict) &&
        !IS_NINJA(vict) && !IS_MARINER(vict) && !IS_RANGER(vict))
      SET_SKILL(vict, SKILL_SNEAK, 0);
  }

  if (GET_ADD(vict) == 100) {
    if (!IS_WARRIOR(vict) && !IS_CAVALIER(vict) && !IS_KNIGHT(vict) &&
        !IS_RANGER(vict))
      GET_ADD(vict) = 0;
  }

  if (vict->specials2.remorts >= 4) { /* Toggle Hunger / Thirst */
    GET_COND(vict, FULL) = (char) -1;
    GET_COND(vict, THIRST) = (char) -1;
    GET_COND(vict, DRUNK) = 0;
  } else {
    GET_COND(vict, FULL) = 24;
    GET_COND(vict, THIRST) = 24;
    GET_COND(vict, DRUNK) = 0;
  }

  if (vict->affected) {
    for (aff = vict->affected; aff; aff = aff->next) {
      if (aff->duration == DURATION_INNATE)
        affect_remove(vict, vict->affected);
    }
  }

  switch(GET_RACE(vict)) {
  case RACE_DWARF:
  case RACE_GNOME:
    af.type = SPELL_CAT_EYES;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration  = DURATION_INNATE;
    af.bitvector = AFF_LIGHT;
    affect_to_char(vict, &af);
    break;
  case RACE_ELF:
  case RACE_HALFELF:
  case RACE_DROW:
  case RACE_RATMAN:
  case RACE_DRACONIAN:
    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);
    break;
  case RACE_FAIRY:
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.type = SPELL_SANCTUARY;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(vict, &af);
    break;
  case RACE_TROLL:
    af.type = SPELL_REGENERATION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_REGENERATION;
    affect_to_char(vict, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);
    break;
  case RACE_ANGEL:
    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);

    af.type = SPELL_DETECT_ALIGN;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_DETECT_ALIGN;
    affect_to_char(vict, &af);

    GET_COND(vict, FULL) = (char) -1;
    GET_COND(vict, THIRST) = (char) -1;
    break;
  case RACE_AVATAR:
    af.type = SPELL_BLESS;
    af.bitvector = 0;
    af.modifier = 2;
    af.duration = DURATION_INNATE;
    af.location = APPLY_HITROLL;
    affect_to_char(vict, &af);

    af.location = APPLY_SAVING_MAGIC;
    af.modifier = 5;
    affect_to_char(vict, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);

    GET_COND(vict, FULL) = (char) -1;
    GET_COND(vict, THIRST) = (char) -1;
    break;
  case RACE_DEMON:
    af.type = SPELL_CAT_EYES;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration  = DURATION_INNATE;
    af.bitvector = AFF_LIGHT;
    affect_to_char(vict, &af);

    af.type = SPELL_DETECT_ALIGN;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_DETECT_ALIGN;
    affect_to_char(vict, &af);

    GET_COND(vict, FULL) = (char) -1;
    GET_COND(vict, THIRST) = (char) -1;
    break;
  case RACE_DRAGON:
    af.type = SPELL_ARMOR;
    af.bitvector = 0;
    af.modifier = -20;
    af.duration = DURATION_INNATE;
    af.location = APPLY_AC;
    affect_to_char(vict, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);

    SET_SKILL(vict, SKILL_TAIL_LASH, 100);
    break;
    case RACE_FELINE:
    af.type = SPELL_CAT_EYES;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration  = DURATION_INNATE;
    af.bitvector = AFF_LIGHT;
    affect_to_char(vict, &af);

    SET_SKILL(vict, SKILL_SNEAK, 95);
    SET_SKILL(vict, SKILL_CLAW, 95);
    SET_SKILL(vict, SKILL_POUNCE, 95);
    break;
  case RACE_VAMPIRE:
    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);

    GET_ADD(vict) = 100;
    GET_COND(vict, FULL) = -1;
    GET_COND(vict, THIRST) = 24;
    GET_COND(vict, DRUNK) = -1;
    break;
  case RACE_WEREWOLF:
    af.type = SPELL_REGENERATION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_REGENERATION;
    affect_to_char(vict, &af);

    af.type = SPELL_INFRAVISION;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = DURATION_INNATE;
    af.bitvector = AFF_INFRARED;
    affect_to_char(vict, &af);
    break;

  default:
    break;
  }

  return;
}

ACMD(do_set)
{
  int	i, l, player_i = 0, retval;
  struct char_data *vict;
  struct char_data *cbuf = NULL;
  struct char_file_u tmp_store;
  char	field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH],
  val_arg[MAX_INPUT_LENGTH];
  char	is_file = 0, is_mob = 0, is_player = 0;
  int mode = -1;

  half_chop(argument, name, buf);
  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  }

  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name || !*field) {
    send_to_char("Usage: set [file|player|mob] <victim> <field> <value>\r\n", ch);
    send_to_char("You can set the following:\r\n\r\n", ch);

    i = 0;
    l = 0;
    *buf = '\0';
    while (*(set_fields[l].cmd) != '\n') {
      if (IS_SET(GODLEVEL(ch), IMM_ALL) ||
	  (GODLEVEL(ch) & set_fields[l].godlevel)) {
	i++;
	sprintf(buf, "%s [%-12s]", buf, set_fields[l].cmd);
	if (!(i % 4))
	  strcat (buf, "\r\n");
      }
      l++;
    }
    if (i % 4)
      strcat (buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  if (!is_file) {
    if (is_player){
      if (!(vict = get_player_vis_exact(ch, name))) {
	send_to_char("Player not found online, searching pfile...\r\n", ch);
	is_file = 1;
      }
    } else {
      if (!(vict = get_char_vis(ch, name))) {
	send_to_char("There is no such creature.\r\n", ch);
	return;
      }
    }
  }

  if (is_file) {
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);

    if ((player_i = load_char(name, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      stringdata_load(cbuf);
      if (GET_LEVEL(cbuf) > GET_LEVEL(ch) && GET_LEVEL(ch) < LEVEL_IMPL) {
	free_char(cbuf);
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }

      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }


  /* find the set */
  for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
    if (!strncmp(field, set_fields[mode].cmd, strlen(field)))
      break;



  /* perform the set */
  retval = perform_set(ch, vict, mode, val_arg, is_file, name);

  if (retval) {
    if(!is_file)
      save_char(vict, IN_VROOM(vict), 2);
    if(is_file) {
      char_to_store(vict, &tmp_store, FALSE, FALSE);
      fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
      fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
      send_to_char("Saved in file.\r\n", ch);
    }
  }

  if (is_file)
    free_char(cbuf);

}

ACMD(do_award) {
  struct char_data *vict;
  struct char_data *cbuf = NULL;
  struct char_file_u tmp_store;
  char is_file = 0;
  int value, player_i = 0;
  char name[MAX_INPUT_LENGTH];

  skip_spaces(&argument);

  half_chop(argument, name, buf);

  if(!*name || !*buf) {
    send_to_char("Usage: award <name> <+/- amount>", ch);
    return;
  }

  if(!(value = atoi(buf))) {
    send_to_char("You need to specify a number.", ch);
    return;
  }

  if(!(vict = get_player_vis_exact(ch, name))) {
    is_file = 1;
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);

    if((player_i = load_char(name, &tmp_store)) > -1 ) {
      store_to_char(&tmp_store, cbuf);
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }

  }

  if(IS_NPC(vict)) {
    send_to_char("Don't mess with the mobs", ch);
    return;
  }

  QUEST_NUM(vict) += value;

  sprintf(buf, "(GC) %s: %s's quest set to %d(%d).", GET_NAME(ch), GET_NAME(vict), QUEST_NUM(vict), value);
  mudlog(buf, BRF, LEVEL_GREATER, TRUE);

  sprintf(buf, "%s's quest set to %d.\r\n", GET_NAME(vict), QUEST_NUM(vict));
  send_to_char(buf, ch);

  if(!is_file)
    save_char(vict, IN_VROOM(vict), 2);

  if(is_file) {
    char_to_store(vict, &tmp_store, FALSE, FALSE);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
  }
}


ACMD(do_quest)
{
  struct descriptor_data *i;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

  skip_spaces(&argument);
  half_chop(argument, arg1, arg2);

  if (!strcmp(arg1, "end")) {
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character)
        REMOVE_BIT(PRF_FLAGS(i->character), PRF_QUEST);
    is_quest = 0;
    send_to_char("Quest has been ended.\r\n", ch);
    sprintf(buf, "(Quest) %s has ended a quest.", GET_NAME(ch));
  } else
    if (!strcmp(arg1, "start")) {
	 if (*arg2) {
              strcat(arg2, "#N\r\n");
	      send_to_all(arg2);
	 } else
	      send_to_all("#rA Quest has begun!#N\r\n");
	 is_quest = 1;
	 sprintf(buf, "(Quest) %s has started a quest.", GET_NAME(ch));
    } else {
	 send_to_char("You need to specify either \"quest start <message>\" or \"quest end\"\r\n", ch);
	 return;
    }

  mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

  return;
}

ACMD(do_questeq)
{
  struct obj_data *questeq;
  extern struct obj_data *questeq_list;
  char buf[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];

  if (!questeq_list) return;

  *buf = '\0';

  for (questeq = questeq_list ; questeq ; questeq = questeq->next) {
    sprintf(buf2, " [#G%6d#N]  Level:#M%4d#N  Cost:#C%4d#N  #Y%s#N\r\n",
	    obj_index[questeq->item_number].virtual,
	    questeq->obj_flags.level,
	    questeq->obj_flags.cost,
	    questeq->short_description);
    strcat(buf, buf2);
  }

  page_string(ch->desc, buf, TRUE);
}

ACMD(do_claneq)
{
  struct obj_data *claneq;
  extern struct obj_data *claneq_list;
  char buf[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];
  int i = 0;

  if (!claneq_list) return;

  *buf = '\0';
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Which clan would you like to list the eq for?\r\n", ch);
    show_clan_info(ch, -1, 1);
  } else
    i = atoi(argument);


  for (claneq = claneq_list ; claneq ; claneq = claneq->next)
    if (CLANEQ(claneq))
      if (clan_list[CLANEQ_CLAN(claneq)].vnum == i) {
	sprintf(buf2, " [#G%6d#N]  Clan:#C%3d#N  Ex: [#G%6d#N] #Y%s#N\r\n",
		obj_index[claneq->item_number].virtual,
		clan_list[CLANEQ_CLAN(claneq)].vnum,
		CLANEQ_EX(claneq),
		claneq->short_description);
	strcat(buf, buf2);
      }

  page_string(ch->desc, buf, TRUE);
}

ACMD(do_duplicate)
{
  struct obj_data *newobj, *obj=0;
  struct char_data *newmob, *mob=0;
  char buf[SMALL_BUFSIZE], target[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH];
  int found, i, num;

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: duplicate <target mob/object> <quantity>.\r\n", ch);
    return;
  } else {

    strcpy(arg, two_arguments(argument, target, value));

    /* look for target */
    if (!(obj = get_obj_in_list_vis(ch, target, ch->carrying)) &&
	!(mob = get_char_room_vis(ch, target))) {
      send_to_char("Be sure the obj is in your inventory, or mob is in the same room.\r\n", ch);
      return;
    }

    if (!*value)
      num = 1;
    else {
      if ((num = atoi(value)) <= 0) {
        send_to_char("How many times do you want to duplicate the obj/mob?\r\n", ch);
        return;
      }
      if (num > 20) {
        num = 20;
        send_to_char(
          "You are not allowed to duplicate the target mob/object more than 20 times!\r\n"
          "Therefore only 20 will be created. Please use duplicate again if you require more.\r\n"
          , ch);
      }
    }

    if (obj) {
     for (i = 0; i < num; i++) {
      found = 0; /* Reset found check for each pass */
      CREATE(newobj, struct obj_data, 1);
      clear_object(newobj);
      *newobj = *obj;

      newobj->contains = NULL;
      newobj->next_content = NULL;

      if (obj->name)
	newobj->name = strdup(obj->name);
      if (obj->description)
	newobj->description = strdup(obj->description);
      if (obj->short_description)
	newobj->short_description = strdup(obj->short_description);
      if (obj->action_description)
	newobj->action_description = strdup(obj->action_description);

      newobj->next = object_list;
      object_list = newobj;

      obj_index[newobj->item_number].number++;

      obj_to_char(newobj, ch);

      strcpy(buf, newobj->name);
      if (strstr(buf, GET_NAME(ch))) found = 1;

      if (!found) {
	sprintf(buf, "%s _%s_", newobj->name, GET_NAME(ch));

	if (newobj->name && newobj->name == obj_proto[newobj->item_number].name)
	  newobj->name = NULL;
	change_string(&newobj->name, buf);
      }
     }
     if (num == 1) {
      sprintf(buf, "%s duplicated!\r\n", newobj->short_description);
      send_to_char(buf, ch);
     } else {
      sprintf(buf, "%s duplicated! (x%d)\r\n", newobj->short_description, num);
      send_to_char(buf, ch);
     }
    } else
      if (mob) { /* duplicate a mob */
       if (!IS_NPC(mob)) {
	send_to_char("You can't duplicate a character!\r\n", ch);
        return;
       }

       for (i = 0; i < num; i++) {
	newmob = read_mobile(mob_index[mob->nr].virtual, VIRTUAL);

	if (mob->player.name)
	  newmob->player.name = strdup(mob->player.name);
	if (mob->player.short_descr)
	  newmob->player.short_descr = strdup(mob->player.short_descr);
	if (mob->player.long_descr)
	  newmob->player.long_descr = strdup(mob->player.long_descr);
	if (mob->player.description)
	  newmob->player.description = strdup(mob->player.description);

	char_to_room(newmob, ch->in_room);
       }
       if (num == 1) {
	sprintf(buf, "%s duplicated!\r\n", mob->player.short_descr);
	send_to_char(buf, ch);
       } else {
        sprintf(buf, "%s duplicated! (x%d)\r\n", mob->player.short_descr, num);
        send_to_char(buf, ch);
       }
      }

    sprintf(buf, "(GC) %s: Duplicates %s (x%d)", GET_NAME(ch), target, num);
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

    return;
  }
}

ACMD(do_xlag)
{
     struct char_data *victim;
     int lag = 0;
     char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

     skip_spaces(&argument);
     two_arguments(argument, arg1, arg2);

     if (!*arg1 || !*arg2) {
	  send_to_char("Usage: xlag <victim> <rounds>. 1 round = 2 sec.\r\n", ch);
	  return;
     }

     if(!(lag = atoi(arg2))) {
	  send_to_char("You need to specify the amount.\r\n", ch);
	  return;
     }

     if(!(victim = get_player_vis(ch, arg1))) {
	  send_to_char("You can't seem to find your victim.\r\n", ch);
	  return;
     }

     if (IS_NPC(victim)) {
	  send_to_char("You can't xlag a mob.\r\n", ch);
	  return;
     }

     if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
	  send_to_char("Nice try...\r\n", ch);
	  return;
     }

     lag = MIN(10000, MAX(0, lag));

     WAIT_STATE(victim, lag * PULSE_VIOLENCE);
     sprintf(buf, "(GC) %s has xlagged %s for %d.", GET_NAME(ch), GET_NAME(victim), lag);
     mudlog(buf, BRF, LEVEL_ADMIN, FALSE);
     return;

}


ACMD(do_fubar)
{
     struct char_data *victim;

     skip_spaces(&argument);
     if (!*argument) {
	  send_to_char("Usage fubar <target>.\r\n", ch);
	  return;
     }

     if(!(victim = get_player_vis(ch, argument))) {
	  send_to_char("You can't seem to find your victim.\r\n", ch);
	  return;
     }

     if (IS_NPC(victim)) {
	  send_to_char("You can't do that to a mob.\r\n", ch);
	  return;
     }

     if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
	  send_to_char("Maybe when you are a little bigger...\r\n", ch);
	  return;
     }

     sprintf(buf, "(GC) %s had fubared %s.", GET_NAME(ch), GET_NAME(victim));
     mudlog(buf, BRF, LEVEL_IMPL, FALSE);

     Crash_delete_file(GET_NAME(victim), OBJECT_FILE);
     Crash_delete_file(GET_NAME(victim), OBJECT_BACKUP);

     if (victim->desc) {
	  close_socket(victim->desc);
	  victim->desc = 0;
     }
     extract_char(victim);

     return;
}


/*   *********************** Immortal Powers **************************** */

/* immcommand_info definition in interpreter.h */

const struct immcommand_info immcmd_info[] = {
  /* name */      /* godlevel */
  { "-"          , IMM_BASIC },
  { "advance"    , IMM_REMORT | IMM_ADMIN },
  { "afssave"    , IMM_CODE },
  { "at"         , IMM_BASIC },
  { "award"      , IMM_QUEST },
  { "ban"        , IMM_STD  },
  { "chanset"    , IMM_OVERSEER },
  { "claneq"     , IMM_CLAN },
  { "config"     , IMM_CODE },
  { "dc"         , IMM_OVERSEER },
  { "disable"    , IMM_OVERSEER },
  { "duplicate"  , IMM_STD | IMM_LOAD },
  { "echo"       , IMM_BASIC },
  { "eedit"      , IMM_OVERSEER },
  { "finddoor"   , IMM_WORLD },
  { "findkey"    , IMM_WORLD },
  { "force"      , IMM_STD | IMM_QUESTOR },
  { "freeze"     , IMM_ADMIN | IMM_OVERSEER },
  { "gecho"      , IMM_REMORT | IMM_ADMIN },
  { "goto"       , IMM_BASIC },
  { "handbook"   , IMM_BASIC },
  { "holylight"  , IMM_BASIC },
  { "ident"      , IMM_OVERSEER },
  { "identname"  , IMM_OVERSEER },
  { "immset"     , IMM_OVERSEER },
  { "immstat"    , IMM_ADMIN | IMM_OVERSEER },
  { "imotd"      , IMM_BASIC },
  { "invis"      , IMM_BASIC  },
  { "load"       , IMM_STD | IMM_LOAD },
  { "locate"     , IMM_WORLD },
  { "log"        , IMM_BASIC },
  { "medit"      , IMM_STD | IMM_QUESTOR },
  { "melt"       , IMM_BASIC },
  { "mpstat"     , 0 },
  { "mute"       , IMM_STD  },
  { "nohassle"   , IMM_BASIC },
  { "nolastcmd"  , IMM_CODE },
  { "notitle"    , IMM_STD  },
  { "ocsrsave"   , 0 },
  { "oedit"      , IMM_STD | IMM_QUESTOR },
  { "page"       , IMM_STD  },
  { "pardon"     , IMM_STD | IMM_QUESTOR },
  { "peace"      , IMM_ADMIN | IMM_OVERSEER },
  { "pfunctions" , IMM_CODE | IMM_CODER },
  { "plog"       , IMM_STD },
  { "poof"       , IMM_BASIC },
  { "prename"    , IMM_BASIC },
  { "purge"      , IMM_STD | IMM_QUESTOR },
  { "qecho"      , IMM_STD | IMM_QUESTOR },
  { "quest"      , IMM_STD | IMM_QUESTOR },
  { "questeq"    , IMM_STD | IMM_QUESTOR },
  { "reboot"     , IMM_OVERSEER },
  { "redit"      , IMM_OVERSEER },
  { "reroll"     , IMM_OVERSEER },
  { "restore"    , IMM_STD | IMM_QUESTOR },
  { "roomflags"  , IMM_BASIC },
  { "rpstat"     , IMM_WORLD },
  { "send"       , IMM_STD | IMM_QUESTOR },
  { "set"        , IMM_BASIC },
  { "setinnate"  , IMM_REMORT },
  { "show"       , IMM_BASIC },
  { "shutdow"    , IMM_SHUTDOWN },
  { "shutdown"   , IMM_SHUTDOWN },
  { "sitename"   , IMM_OVERSEER },
  { "skillclear" , IMM_REMORT },
  { "skillset"   , IMM_REMORT },
  { "slowns"     , IMM_OVERSEER },
  { "snoop"      , IMM_STD  },
  { "snowball"   , IMM_BASIC },
  { "stskill"    , IMM_REMORT | IMM_ADMIN },
  { "switch"     , IMM_STD | IMM_QUESTOR },
  { "syslog"     , IMM_BASIC },
  { "tedit"      , IMM_OVERSEER | IMM_CODE | IMM_NEWS | IMM_HELP },
  { "teleport"   , IMM_STD | IMM_QUESTOR },
  { "thaw"       , IMM_ADMIN | IMM_OVERSEER },
  { "transet"    , IMM_STD | IMM_QUESTOR },
  { "transfer"   , IMM_STD | IMM_QUESTOR },
  { "unaffect"   , IMM_REMORT | IMM_QUESTOR },
  { "unban"      , IMM_OVERSEER },
  { "uptime"     , IMM_BASIC },
  { "users"      , IMM_STD  },
  { "view"       , IMM_STD },
  { "vlist"      , IMM_WORLD },
  { "vnum"       , IMM_BASIC },
  { "vstat"      , IMM_STD  },
  { "whereload"  , IMM_WORLD },
  { "wizhelp"    , IMM_BASIC },
  { "wizlock"    , IMM_OVERSEER },
  { "wizname"    , IMM_BASIC },
  { "xlag"       , IMM_OVERSEER },
  { "zpurge"     , IMM_WORLD },
  { "zreset"     , IMM_WORLD },
 { "\n"         , 0 }	/* this must be last */
};

int immortal_interpreter(struct char_data *ch, char *command)
{
  int cmd, found = 0;

  /* find command */
  for (cmd = 0; *immcmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(immcmd_info[cmd].command, command)) {
      found = 1;
      break;
    }

  /* check IMM_ALL */
  if (found && IS_SET(GODLEVEL(ch), IMM_ALL))
    return 1;

  /* check GODLEVEL */
  if (found && (immcmd_info[cmd].godlevel & GODLEVEL(ch)))
    return 1;
  else {
    send_to_char("You can't do that!\r\n", ch);
    return 0;
  }

  if (!found) {
    log("SYSERR: immortal_interpreter command not found");
    return 0;
  }

  log("SYSERR: immortal_interpreter failed");
  return 0;
}

ACMD(do_immset)
{
  ACMD(do_immstat);
  void print_bits(char **vec, char *buf);

  char name[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  int level, player_i = 0;
  bool found = FALSE, file = FALSE;
  struct char_data *vict = NULL;
  struct char_file_u tmp_store;


  half_chop(argument, name, arg);

  if (!*name || !*arg ) {
    send_to_char("Usage: immset <name> <level>\r\n", ch);
    *buf = '\0';
    print_bits(imm_powers, buf);
    send_to_char(buf, ch);
    send_to_char("  0. Clear All\r\n", ch);
    return;
  }

  level = atoi(arg);

  if ((vict = get_player_vis_exact(ch, name))) found = TRUE;

  if (!found) {
       CREATE(vict, struct char_data, 1);
       clear_char(vict);

       if ((player_i = load_char(name, &tmp_store)) > -1) {
	    store_to_char(&tmp_store, vict);
	    file = TRUE;
	    found = TRUE;
       } else {
	    free(vict);
	    send_to_char("There is no such player.\r\n", ch);
	    return;
       }
  }

  if (found) {
    if (ch == vict && GET_LEVEL(ch) < LEVEL_ADMIN) {
      send_to_char("You can't set that!\r\n", ch);
      return;
    }
    if ((GET_LEVEL(ch) > GET_LEVEL(vict) ||
        (GET_LEVEL(ch) >= LEVEL_ADMIN && !(GET_LEVEL(vict) > GET_LEVEL(ch)))) &&
	!(((1 << (level -1)) == IMM_ALL) && GET_LEVEL(ch) < LEVEL_ADMIN) ) {
      if (level == 0)
	GODLEVEL(vict) = 0;
      else
	TOGGLE_BIT(GODLEVEL(vict), (long) (1 << (level - 1)));

      sprintbit((long) GODLEVEL(vict), imm_powers, buf2);
      sprintf(buf, "(GC) Immset: %s has set %s: %s", GET_NAME(ch), GET_NAME(vict), buf2);
      mudlog(buf, NRM, LEVEL_ADMIN, TRUE);
    } else
      send_to_char("You can't set that!\r\n", ch);
  } else
    send_to_char("Immset what?\r\n", ch);

  if (file) {
       char_to_store(vict, &tmp_store, FALSE, FALSE);
       fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
       fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
       send_to_char("Saved in file.\r\n", ch);
       free(vict);
  }

}

ACMD(do_immstat)
{
  struct char_data *target;
  char name[MAX_INPUT_LENGTH];

  one_argument(argument, name);

  if (!*name) {
    send_to_char("Usage: immstat <target>\r\n", ch);
    return;
  }

  if ((target = get_char_vis(ch, name))) {
    if ((target == ch) || (GET_LEVEL(ch) >= GET_LEVEL(target))) {
      sprintbit((long) GODLEVEL(target), imm_powers, buf2);
      sprintf(buf, "%s Imm Powers: %s\r\n", GET_NAME(target), buf2);
      send_to_char(buf, ch);
    } else
      send_to_char("You can't do that!\r\n", ch);
  } else
    send_to_char("Immstat what?\r\n", ch);
}

ACMD(do_log)
{
  if (!*argument) {
    send_to_char("Usage: log <message>\r\n", ch);
    return;
  }
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (strlen(argument) > MAX_STRING_LENGTH) {
    sprintf(buf, "Sorry, your message is too long. %d characters max.\r\n", MAX_STRING_LENGTH);
    send_to_char(buf, ch);
  }
  sprintf(buf, "Your message has been logged:\r\n%s\r\n", argument);
  send_to_char(buf, ch);
  sprintf(buf, "(GC) %s LOGS: %s", GET_NAME(ch), argument);
  mudlog(buf, NRM, GET_LEVEL(ch), TRUE);
}

ACMD(do_plog)
{
  struct char_data *tch = 0;
  struct char_file_u chdata;
  char name[SMALL_BUFSIZE];
  int is_show = 0;

  if (IS_NPC(ch))
    return;

  half_chop(argument, arg, buf1);
  half_chop(buf1, buf, buf2);

 if (!str_cmp(buf, "show")) {
    is_show = 1;
 }
 else if (str_cmp(buf, "chat") && str_cmp(buf, "title") && str_cmp(buf, "other"))
    *buf = 0; /* Make it  0 to report usage */

  if (!*arg || !*buf) {
    send_to_char("Usage: plog <player> <chat|title|other> <message>\r\n"
                 "       plog <player> show\r\n", ch);
    return;
  }

  *chdata.name = 0;

  if (load_char(arg, &chdata) < 0 && !(tch = get_player_vis(ch, arg))) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }

  if (is_show) {
    list_plog(ch, arg);
    return;
  }
  else {
    strcpy(name, GET_NAME(ch));
    *name = UPPER(*name);
    *buf = UPPER(*buf);

    sprintf(buf1, "%-12s : %-5s :: %s", name, buf, buf2);

    send_to_char(write_plog(arg, buf1), ch);
  }

  return;
}

ACMD(do_peace)
{
  struct char_data *vict, *next_v;

  act("$n decides that everyone should just be friends.",
                             FALSE,ch,0,0,TO_ROOM);
  send_to_room("Everything is quite peaceful now.\r\n", IN_ROOM(ch));
  for(vict=world[IN_ROOM(ch)]->people; vict; vict=next_v) {
    next_v = vict->next_in_room;
    if (IS_NPC(vict) && FIGHTING(vict)) {
      if (FIGHTING(FIGHTING(vict)) == vict)
        stop_fighting(FIGHTING(vict));
      stop_fighting(vict);
    }
  }
}

/*
 * List all rooms/mobiles/objects within a defined range
 */
ACMD(do_vlist)
{

  int nr, first, last, zone = -1, found = 0, len;
  int is_room = 0, is_mob = 0, is_obj = 0;

  half_chop(argument, arg, buf1);
  two_arguments(buf1, buf, buf2);

  if (is_abbrev(arg, "room"))
    is_room = 1;
  else if (is_abbrev(arg, "mob"))
    is_mob = 1;
  else if (is_abbrev(arg, "obj"))
    is_obj = 1;
  else
    *arg = 0;   /* Make it 0 to report usage */

  if (!*arg || !*buf) {
    send_to_char("Usage: vlist <room|obj|mob> <begining number> <ending number>\r\n"
                 "       vlist <room|obj|mob> z<zone number>\r\n", ch);
    return;
  }

  /* Check for z<num> */
  if (*buf == 'z') { /* If no arg will default to zone 0 */
    if (*buf2) { /* Catches arg of z <num> */
      nr = atoi(buf2);
      zone = real_zone(nr);
    } else {
      nr = atoi((buf + 1));
      zone = real_zone(nr);
    }
    if (zone < 0) {
      send_to_char("That zone could not be found.\r\n", ch);
      return;
    }
    first = (nr * 100);
    last = ((nr * 100) + 99);

  } else { /* Obvious error checking */
    first = atoi(buf);
    last = atoi(buf2);
    if ((first < 0) || (first > 999999) || (last < 0) || (last > 999999)) {
      send_to_char("Values must be between 0 and 999999.\n\r", ch);
      return;
    }

    if (first >= last) {
      send_to_char("Second value must be greater than first.\n\r", ch);
      return;
    }
  }

  *buf = 0;

  if (is_room) {
     for (nr=0; nr <= top_of_world && (world[nr]->number <= last); nr++) {
       if (world[nr]->number >= first) {
         len = strlen(buf);
         if (len > (MAX_STRING_LENGTH-200)) {
           strcat(buf, "*** TRUNCATED ***");
           break;
         }
       sprintf(buf+len, "%5d. [#G%5d#N] (%3d) #Y%s#N\r\n", ++found,
               world[nr]->number, world[nr]->zone, world[nr]->name);
      }
    }
  } else if (is_mob) {
    for (nr=0; nr <= top_of_mobt && (mob_index[nr].virtual <= last); nr++) {
      if (mob_index[nr].virtual >= first) {
        len = strlen(buf);
        if (len > (MAX_STRING_LENGTH-200)) {
          strcat(buf, "*** TRUNCATED ***");
          break;
        }
        sprintf(buf+len, "%5d. [#G%5d#N] #Y%s#N\r\n", ++found,
              mob_index[nr].virtual,
              mob_proto[nr].player.short_descr);
      }
    }
  } else if (is_obj) {
    for (nr=0; nr <= top_of_objt && (obj_index[nr].virtual <= last); nr++) {
      if (obj_index[nr].virtual >= first) {
        len = strlen(buf);
        if (len > (MAX_STRING_LENGTH-200)) {
          strcat(buf, "*** TRUNCATED ***");
          break;
        }
        sprintf(buf+len, "%5d. [#G%5d#N] #Y%s#N\r\n", ++found,
              obj_index[nr].virtual,
              obj_proto[nr].short_description);
      }
    }

  }
  if (!found) send_to_char("Nothing was found in that range.\r\n", ch);
  else page_string(ch->desc, buf, 1);

}


ACMD(do_tedit) {
   int l, i;
   char field[MAX_INPUT_LENGTH];
   extern char *credits;
   extern char *news;
   extern char *motd;
   extern char *imotd;
   extern char *help;
   extern char *info;
   extern char *newbie;
   extern char *wizlist;
   extern char *immlist;
   extern char *remlist;
   extern char *background;
   extern char *handbook;
   extern char *policies;

   struct editor_struct {
      char *cmd;
      long godlevel;
      char *buffer;
      int  size;
      char *filename;
   } fields[] = {
        { "credits",    IMM_CODE               ,   credits,        2400,   CREDITS_FILE},
        { "news",       IMM_OVERSEER | IMM_NEWS,   news,           8192,   NEWS_FILE},
        { "motd",       IMM_OVERSEER           ,   motd,           2400,   MOTD_FILE},
        { "imotd",      IMM_OVERSEER           ,   imotd,          2400,   IMOTD_FILE},
        { "help",       IMM_OVERSEER | IMM_HELP,   help,           2400,   HELP_PAGE_FILE},
        { "info",       IMM_OVERSEER | IMM_HELP,   info,           8192,   INFO_FILE},
        { "newbie",     IMM_OVERSEER | IMM_HELP,   newbie,         2400,   NEWBIE_FILE},
        { "wizlist",    IMM_OVERSEER           ,   wizlist,        2400,   WIZLIST_FILE},
        { "immlist",    IMM_OVERSEER           ,   immlist,        2400,   IMMLIST_FILE},
        { "remlist",    IMM_OVERSEER           ,   remlist,        2400,   REMLIST_FILE},
        { "background", IMM_OVERSEER           ,   background,     8192,   BACKGROUND_FILE},
        { "handbook",   IMM_HELP               ,   handbook,       8192,   HANDBOOK_FILE},
        { "policies",   IMM_OVERSEER           ,   policies,       8192,   POLICIES_FILE},
        { "\n",         0       ,       NULL,           0,      NULL }
   };

   if (ch->desc == NULL) {
      send_to_char("Get outta here you linkdead head!\r\n", ch);
      return;
   }

   half_chop(argument, field, buf);

   if (!*field) {
      strcpy(buf, "Files available to be edited:\r\n");
      i = 1;
      for (l = 0; *fields[l].cmd != '\n'; l++) {
         if ((GODLEVEL(ch) & fields[l].godlevel) ||
             (IS_SET(GODLEVEL(ch), IMM_ALL))) {
            sprintf(buf, "%s%-11.11s", buf, fields[l].cmd);
            if (!(i % 7)) strcat(buf, "\r\n");
            i++;
         }
      }
      if (--i % 7) strcat(buf, "\r\n");
      if (i == 0) strcat(buf, "None.\r\n");
      send_to_char(buf, ch);
      return;
   }
   for (l = 0; *(fields[l].cmd) != '\n'; l++)
     if (!strncmp(field, fields[l].cmd, strlen(field)))
     break;

   if (*fields[l].cmd == '\n') {
      send_to_char("Invalid text editor option.\r\n", ch);
      return;
   }

  if (!(IS_SET(GODLEVEL(ch), IMM_ALL)))
    if(!(GODLEVEL(ch) & fields[l].godlevel)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return;
    }

   switch (l) {
    case 0:  ch->desc->str = &credits; break;
    case 1:  ch->desc->str = &news; break;
    case 2:  ch->desc->str = &motd; break;
    case 3:  ch->desc->str = &imotd; break;
    case 4:  ch->desc->str = &help; break;
    case 5:  ch->desc->str = &info; break;
    case 6:  ch->desc->str = &newbie; break;
    case 7:  ch->desc->str = &wizlist; break;
    case 8:  ch->desc->str = &immlist; break;
    case 9:  ch->desc->str = &remlist; break;
    case 10: ch->desc->str = &background; break;
    case 11: ch->desc->str = &handbook; break;
    case 12: ch->desc->str = &policies; break;
    default:
      send_to_char("Invalid text editor option.\r\n", ch);
      return;
   }

   /* set up editor stats */
   send_to_char("\x1B[H\x1B[J", ch);
   send_to_char("Edit file below: (/s saves /h for help)\r\n", ch);
   ch->desc->backstr = NULL;
   if (fields[l].buffer) {
      send_to_char(fields[l].buffer, ch);
      ch->desc->backstr = str_dup(fields[l].buffer);
   }
   ch->desc->max_str = fields[l].size;
   ch->desc->storage = str_dup(fields[l].filename);
   act("$n begins editing a scroll.", TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(PLR_FLAGS(ch), PLR_WRITING);
   STATE(ch->desc) = CON_TEXTED;
}

void view_players(struct char_data *ch)
{
  struct descriptor_data *d;
  struct char_data *i;
  char wiz[20];

  sprintf(buf, "Players\r\n"
    "            #wName             |   Hp   Mn   Mv | DR HR   AC MR#N\r\n");
  send_to_char(buf, ch);
  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      i = d->character;
      if (i && CAN_SEE(ch, i)) {
        CLASS_ABBR(i, wiz);
        sprintf(buf2, "%3d %s", GET_LEVEL(i), wiz);
        sprintf(buf, "[%s] %-16s#w | #R%4d #C%4d #Y%4d#w | #N%2d %2d %4d %2d#N\r\n",
                buf2, GET_NAME(i), GET_MAX_HIT(i), GET_MAX_MANA(i),
                GET_MAX_MOVE(i), GET_DAMROLL(i), GET_HITROLL(i),
                GET_AC(i), i->specials2.resistances[4]);
        send_to_char(buf, ch);
      }
    }

}


void view_mortals(struct char_data *ch)
{
  struct descriptor_data *d;
  struct char_data *i;
  char wiz[20], pos[MAX_INPUT_LENGTH];
  int count = 0;

  sprintf(buf, "Mortal Activity\r\n"
    "            #wName              Room Health Idle Activity#N\r\n");
  send_to_char(buf, ch);
  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      i = d->character;
      if (i && CAN_SEE(ch, i) && (GET_LEVEL(i) < LEVEL_DEITY)) {
        CLASS_ABBR(i, wiz);
        sprintf(buf2, "%3d %s", GET_LEVEL(i), wiz);
        if (i->specials.fighting)
          sprintf(pos, "#r%s: #N%s", position_types[(int)GET_POS(i)],
                                     GET_NAME(i->specials.fighting));
        else if (PLR_FLAGGED(i, PLR_WRITING))
          sprintf(pos, "#wWriting#N");
        else if (PLR_FLAGGED(i, PLR_MAILING))
          sprintf(pos, "#wWriting Mail#N");
        else
          sprintf(pos, "#y%s#N", position_types[(int)GET_POS(i)]);
        sprintf(buf, "[%s] %-16s #G%5d#N  #R%3d%%#N   %3d %s\r\n",
                buf2, GET_NAME(i), world[IN_ROOM(i)]->number,
                (int)(GET_HIT(i)*100)/GET_MAX_HIT(i),
                (i->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN), pos);
        send_to_char(buf, ch);
        count++;
      }
    }
  if (!count)
    send_to_char("            No mortals are on!\r\n", ch);

}


void view_immortals(struct char_data *ch)
{
  struct descriptor_data *d;
  struct char_data *i;          /* To inherit state from original */
  char wiz[20], pos[MAX_INPUT_LENGTH];

  sprintf(buf, "Immortal Activity\r\n"
    "            #wName              Room Idle Activity#N\r\n");
  send_to_char(buf, ch);
  for (d = descriptor_list; d; d = d->next)
     if (!d->connected) {
        if (d->original)
          i = d->original;
        else
          i = d->character;
        CLASS_ABBR(i, wiz);
        sprintf(buf2, "%3d %s", GET_LEVEL(i), wiz);
      if (i && CAN_SEE(ch, i) && (GET_LEVEL(i) >= LEVEL_DEITY)) {
        if (d->snooping && (GET_LEVEL(ch) >= GET_LEVEL(d->character)))
          sprintf(pos, "#ySnooping#N");
        else if (PLR_FLAGGED(i, PLR_WRITING))
          sprintf(pos, "#CWriting#N");
        else if (PLR_FLAGGED(i, PLR_MAILING))
          sprintf(pos, "#CWriting Mail#N");
        else if (d->original)
          sprintf(pos, "#ySwitched: #N%s", GET_NAME(d->character));
        else if (i->specials.fighting)
          sprintf(pos, "#r%s: #N%s", position_types[(int)GET_POS(i)],
                                     GET_NAME(i->specials.fighting));
        else
          sprintf(pos, "#y%s#N", position_types[(int)GET_POS(i)]);
        sprintf(buf, "[%s] %-16s #G%5d#N  %3d %s\r\n",
                buf2, GET_NAME(i), world[IN_ROOM(i)]->number,
                (i->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN), pos);
        send_to_char(buf, ch);
      }
    }

}

void view_questors(struct char_data *ch)
{
  struct descriptor_data *d;
  struct char_data *i;          /* To inherit state from original */
  char wiz[20], pos[MAX_INPUT_LENGTH];
  int count = 0;

  sprintf(buf, "Quest Activity\r\n"
    "            #wName              Room Health Idle Activity#N\r\n");
  send_to_char(buf, ch);
  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
        if (d->original)
          i = d->original;
        else
          i = d->character;
      if (i && CAN_SEE(ch, i) && IS_SET(PRF_FLAGS(i), PRF_QUEST)) {
        CLASS_ABBR(i, wiz);
        sprintf(buf2, "%3d %s", GET_LEVEL(i), wiz);
        if (d->snooping && (GET_LEVEL(ch) >= GET_LEVEL(d->character)))
          sprintf(pos, "#ySnooping#N");
        else if (PLR_FLAGGED(i, PLR_WRITING))
          sprintf(pos, "#CWriting#N");
        else if (PLR_FLAGGED(i, PLR_MAILING))
          sprintf(pos, "#CWriting Mail#N");
        else if (d->original)
          sprintf(pos, "#ySwitched: #N%s", GET_NAME(d->character));
        else if (i->specials.fighting)
          sprintf(pos, "#r%s: #N%s", position_types[(int)GET_POS(i)],
                                     GET_NAME(i->specials.fighting));
        else
          sprintf(pos, "#y%s#N", position_types[(int)GET_POS(i)]);
        sprintf(buf, "[%s] %-16s #G%5d  #R%3d%%#N   %3d %s\r\n",
                buf2, GET_NAME(i), world[IN_ROOM(i)]->number,
                (int)(GET_HIT(i)*100)/GET_MAX_HIT(i),
                (i->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN), pos);
        send_to_char(buf, ch);
        count++;
      }
    }
  if (!count)
    send_to_char("            No-one is on the quest channel!\r\n", ch);
}


void view_pkok(struct char_data *ch)
{
  struct descriptor_data *d;
  struct char_data *i;          /* To inherit state from original */
  char wiz[20], pos[MAX_INPUT_LENGTH];
  int count = 0;

  sprintf(buf, "PKOK Activity\r\n"
    "            #wName              Room Health Idle Activity#N\r\n");
  send_to_char(buf, ch);
  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
        if (d->original)
          i = d->original;
        else
          i = d->character;
      if (i && CAN_SEE(ch, i) && IS_SET(PLR_FLAGS(i), PLR_PKOK)) {
        CLASS_ABBR(i, wiz);
        sprintf(buf2, "%3d %s", GET_LEVEL(i), wiz);
        if (d->snooping && (GET_LEVEL(ch) >= GET_LEVEL(d->character)))
          sprintf(pos, "#ySnooping#N");
        else if (PLR_FLAGGED(i, PLR_WRITING))
          sprintf(pos, "#CWriting#N");
        else if (PLR_FLAGGED(i, PLR_MAILING))
          sprintf(pos, "#CWriting Mail#N");
        else if (d->original)
          sprintf(pos, "#ySwitched: #N%s", GET_NAME(d->character));
        else if (i->specials.fighting)
          sprintf(pos, "#r%s: #N%s", position_types[(int)GET_POS(i)],
                                     GET_NAME(i->specials.fighting));
        else
          sprintf(pos, "#y%s#N", position_types[(int)GET_POS(i)]);
        sprintf(buf, "[%s] %-16s #G%5d  #R%3d%%#N   %3d %s\r\n",
                buf2, GET_NAME(i), world[IN_ROOM(i)]->number,
                (int)(GET_HIT(i)*100)/GET_MAX_HIT(i),
                (i->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN), pos);
        send_to_char(buf, ch);
        count++;
      }
    }
  if (!count)
    send_to_char("            No PKOK players online!\r\n", ch);
}


ACMD(do_view)
{
  /*
   * Editing and other related utility views: 'hard objs' list and things.
   * Currently only players option coded
   */
  char name[MAX_INPUT_LENGTH];

  half_chop(argument, name, buf);
  if (!str_cmp(name, "players")) {
    view_players(ch);
  } else if (!str_cmp(name, "mortals")) {
    view_mortals(ch);
  } else if (!str_cmp(name, "immortals")) {
    view_immortals(ch);
  } else if (!str_cmp(name, "questors")) {
    view_questors(ch);
  } else if (!str_cmp(name, "pkok")) {
    view_pkok(ch);
  } else
    send_to_char("Currently accepted view formats:\r\n"
                "view players   - view abbreviated players stats\r\n"
                "view questors  - View quest activity\r\n"
                "view pkok      - View PKOK player activity\r\n"
                "view mortals   - view mortal activity\r\n"
                "view immortals - view immortal activity\r\n"
                , ch);

}

int perform_unaffect(struct char_data *vict)
{

  struct affected_type *aff;
  int count = 0;

  if (vict->affected) {
    for (aff = vict->affected; aff; aff = aff->next) {
      if (aff->duration != DURATION_INNATE) {
        affect_remove(vict, vict->affected);
        count++;
      }
    }
  send_to_char("There is a brief flash of light!\r\n"
               "You feel slightly different.\r\n", vict);
  }
  if (count)
    return(1);
  else
    return(0);

}



ACMD(do_unaffect)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH];
  int success = 0;


  half_chop(argument, name, buf1);
  if (!*name) {
    send_to_char("Usage:\r\n"
      "unaffect <player>         - Remove Spells from Player\r\n"
      "unaffect all              - Remove Spells from ALL Players in Game\r\n"
      "unaffect zone             - Remove Spells from Players in current zone\r\n"
      "unaffect room             - Remove Spells from Players in current room\r\n"
      "unaffect quest            - Remove Spells from Players on Quest Channel\r\n"
      "unaffect innates <player> - Remove Spells and Innates from Player\r\n"
      , ch);
    return;
  }
  else if (!str_cmp("all", name)) {
    if (IS_SET(GODLEVEL(ch), IMM_ALL)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (GET_LEVEL(vict) < LEVEL_DEITY)
          perform_unaffect(vict);
      }
      send_to_char("All spells removed.\r\n", ch);
      sprintf(buf, "(GC) %s has unaffected all mortals.", GET_NAME(ch));
    }
    else {
      send_to_char("I think not.\r\n", ch);
      return;
    }
  }
  else if (!str_cmp("zone", name)) {
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_ADMIN)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (!IS_NPC(vict)) {
          if ((world[ch->in_room]->zone) == (world[vict->in_room]->zone)) {
            if (GET_LEVEL(vict) < LEVEL_DEITY)
              perform_unaffect(vict);
          }
        }
      }
      send_to_char("All spells removed.\r\n", ch);
      sprintf(buf, "(GC) %s has unaffected mortals in zone %d.", GET_NAME(ch),
              zone_table[world[(ch->in_room)]->zone].number);
    }
    else {
      send_to_char("I think not.\r\n", ch);
      return;
    }
  }
  else if (!str_cmp("room", name)) {
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_STD)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (!IS_NPC(vict)) {
          if ((ch->in_room) == (vict->in_room)) {
            if (GET_LEVEL(vict) < LEVEL_DEITY)
              perform_unaffect(vict);
          }
        }
      }
      send_to_char("All spells removed.\r\n", ch);
      sprintf(buf, "(GC) %s has unaffected mortals in room %d.", GET_NAME(ch),
              world[ch->in_room]->number);
    }
    else {
      send_to_char("I think not.\r\n", ch);
      return;
    }
  }
  else if (!str_cmp("quest", name)) {
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_STD)) {
      for(vict = character_list; vict; vict = vict->next) {
        if (!IS_NPC(vict)) {
          if (IS_SET(PRF_FLAGS(ch), PRF_QUEST)) {
            if (IS_SET(PRF_FLAGS(vict), PRF_QUEST)) {
              if (GET_LEVEL(vict) < LEVEL_DEITY)
                perform_unaffect(vict);
            }
          }
          else {
            send_to_char("You're not even part of the quest!\r\n", ch);
            return;
          }
        }
      }
      send_to_char("All spells removed.\r\n", ch);
      sprintf(buf, "(GC) %s has unaffected mortals on the quest!", GET_NAME(ch));
    }
    else {
      send_to_char("I think not.\r\n", ch);
      return;
    }
  }
  else if (!str_cmp("innates", name)) {
    if (IS_SET(GODLEVEL(ch), IMM_ALL) || IS_SET(GODLEVEL(ch), IMM_REMORT)) {
      if (!(vict = get_char_vis(ch, buf1)) || IS_NPC(vict)) {
        send_to_char("Yes, but for whom?!?\r\n", ch);
        return;
      }
      else if (GET_LEVEL(vict) > GET_LEVEL(ch)) {
        send_to_char("I think not.\r\n", ch);
        return;
      }
      else
        if (vict->affected) {
          while (vict->affected)
            affect_remove(vict, vict->affected);
        send_to_char("All spells and innates removed.\r\n", ch);
        send_to_char("There is a brief flash of light!\r\n"
                     "You feel slightly different.\r\n", vict);
        } else {
          send_to_char("Your victim does not have any affections!\r\n", ch);
      }
    sprintf(buf, "(GC) %s has unaffected and removed innates from %s.",
            GET_NAME(ch), GET_NAME(vict));
    }
    else {
      send_to_char("I think not.\r\n", ch);
      return;
    }
  }
  else if (!(vict = get_char_vis(ch, name))) {
    send_to_char("Yes, but for whom?!?\r\n", ch);
    return;
  }
  else if (GET_LEVEL(vict) > GET_LEVEL(ch)) {
    send_to_char("I think not.\r\n", ch);
    return;
  }
  else {
    success = perform_unaffect(vict);
    if (success)
      send_to_char("All spells removed.\r\n", ch);
    else
      send_to_char("Your victim does not have any affections!\r\n", ch);
    sprintf(buf, "(GC) %s has unaffected %s.", GET_NAME(ch), GET_NAME(vict));
  }
  mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}

ACMD(do_config)
{

  extern int pk_allowed;
  extern int pt_allowed;
  extern int pkok_allowed;
  extern int loot_allowed;
  extern int level_can_chat;
  extern int holler_move_cost;
  extern int max_obj_save;
  extern int max_recall_level;
  extern int max_bank_gold;
  extern int max_npc_corpse_time;
  extern int max_pc_corpse_time;
  extern int dts_are_dumps;
  extern int newbie_clan;
  extern int auto_save;
  extern int autosave_time;
  extern int use_autowiz;
  extern int min_wizlist_lev;
  extern void write_elite_config();


  int toggle = 0, binary = 0, i = 0, l = 0;
  char opt[MAX_INPUT_LENGTH], val[MAX_INPUT_LENGTH];

  struct config_struct {
    char *cmd;
    char type;
  } game[] = {
    { "pk"         , BINARY },
    { "pt"         , BINARY },
    { "pkok"       , BINARY },
    { "loot"       , BINARY },
    { "recall"     , NUMBER },
    { "shout"      , NUMBER },
    { "yell"       , NUMBER },
    { "bank"       , NUMBER },
    { "npc"        , NUMBER },
    { "pc"         , NUMBER },
    { "dts"        , BINARY },
    { "objlimit"   , NUMBER },
    { "newbieclan" , NUMBER },
    { "autosave"   , BINARY },
    { "savetime"   , NUMBER },
    { "autowiz"    , BINARY },
    { "minwizlist" , NUMBER },
    { "\n"         , MISC   }
  };

  half_chop(argument, opt, val);

  if (!*opt || !*val) {
    sprintf(buf, "Current game settings are:\r\n\r\n"
                 "Player Killing: #r%s#N.  Player Stealing: #r%s#N.\r\n"
                 "Global PKOK System Enabled: #r%s#N.  Pcorpse Looting: #r%s#N.\r\n"
                 "Players may chat at level #G%d#N, and Yelling costs #G%d#N movement.\r\n"
                 "Players may rent #w%d#N objects, and Recall upto level #G%d#N.\r\n"
                 "Gold Limit for Player Bank: #w%dM#N. (%d)\r\n"
                 "NPC corpses exist for #y%d#N minutes.\r\n"
                 "PC corpses exist for #y%d#N minutes.\r\n"
                 "DT's destroy equipment: #r%s#N. Newbie Clan Vnum: #y%d#N.\r\n"
                 "Auto Save Enabled: #G%s#N. Saves every #G%d#N minutes.\r\n"
                 "Auto Wiz Enabled: #G%s#N. Min Level to appear on wizlist: #G%d#N.\r\n\r\n"
                 "Usage : config <option> <value|on|off>\r\n"
                 "You can configure the following:\r\n\r\n",
            YESNO(pk_allowed), YESNO(pt_allowed), YESNO(pkok_allowed), YESNO(loot_allowed),
            level_can_chat, holler_move_cost, max_obj_save, max_recall_level,
            max_bank_gold/1000000, max_bank_gold, max_npc_corpse_time, max_pc_corpse_time,
            YESNO(dts_are_dumps), newbie_clan, YESNO(auto_save), autosave_time,
            YESNO(use_autowiz), min_wizlist_lev);

    send_to_char(buf, ch);

    i = 0;
    l = 0;
    *buf = '\0';
    while (*(game[l].cmd) != '\n') {
        i++;
        sprintf(buf, "%s [%-12s]", buf, game[l].cmd);
        if (!(i % 4))
          strcat (buf, "\r\n");
      l++;
    }
    if (i % 4)
      strcat (buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  for (l = 0; *(game[l].cmd) != '\n'; l++)
    if (!strncmp(opt, game[l].cmd, strlen(opt)))
      break;


  binary = -1;

  if (game[l].type == BINARY) {
    if (!strcmp(val, "on") || !strcmp(val, "yes"))
      binary = 1;
    if (!strcmp(val, "off") || !strcmp(val, "no"))
      binary = 0;
    if (binary == -1) {
      send_to_char("Value must be on or off.\r\n", ch);
      return;
    }
  }

  if (game[l].type == NUMBER)
    toggle = atoi(val);

  switch(l) {
    case 0:
        pk_allowed = binary;
      break;
    case 1:
        pt_allowed = binary;
      break;
    case 2:
        pkok_allowed = binary;
      break;
    case 3:
        loot_allowed = binary;
      break;
    case 4:
        if (toggle < 1 || toggle > LEVEL_DEITY) {
          sprintf(buf, "Value must be between the following range: 1 - %d.\r\n",
                       LEVEL_DEITY);
          send_to_char(buf, ch);
          return;
        } else
          max_recall_level = toggle;
      break;
    case 5:
        if (toggle < 1 || toggle > LEVEL_DEITY) {
          sprintf(buf, "Value must be between the following range: 1 - %d.\r\n",
                       LEVEL_DEITY);
          send_to_char(buf, ch);
          return;
        } else
          level_can_chat = toggle;
      break;
    case 6:
        if (toggle < 0) {
          send_to_char("Value must be positive or zero!\r\n", ch);
          return;
        } else
          holler_move_cost = toggle;
      break;
    case 7:
        if (toggle < 0) {
          send_to_char("Value must be positive or zero!\r\n", ch);
          return;
        } else
          max_bank_gold = toggle;
      break;
    case 8:
        if (toggle < 2) {
          send_to_char("Value must exceed #w1#N minute.\r\n", ch);
          return;
        } else
          max_npc_corpse_time = toggle;
      break;
    case 9:
        if (toggle < 2) {
          send_to_char("Value must exceed #w1#N minute.\r\n", ch);
          return;
        } else
          max_pc_corpse_time = toggle;
      break;
    case 10:
        dts_are_dumps = binary;
      break;
    case 11:
        if (toggle < 30) {
          send_to_char("Value must be at least #w30#N objects.\r\n", ch);
          return;
        } else
          max_obj_save = toggle;
      break;
    case 12:
        if (toggle < 1) {
          send_to_char("Value must be greater than zero!\r\n", ch);
          return;
        } else
          newbie_clan = toggle;
      break;
    case 13:
        auto_save = binary;
      break;
    case 14:
        if (toggle < 2) {
          send_to_char("Value must exceed #w1#N minute.\r\n", ch);
          return;
        } else
          autosave_time = toggle;
      break;
    case 15:
        use_autowiz = binary;
      break;
    case 16:
        if (toggle < LEVEL_DEITY || toggle > LEVEL_IMPL) {
          sprintf(buf, "Please select a number within the following range: %d - %d\r\n",
                       LEVEL_DEITY, LEVEL_IMPL);
          send_to_char(buf, ch);
          return;
        } else
          min_wizlist_lev = toggle;
      break;
    default:
      send_to_char("Unknown option - type 'config' for help.\r\n", ch);
      return;
      break;
  }

  send_to_char("Option changed - updating Elite config.\r\n", ch);

  if (game[l].type == BINARY) {
    sprintf(buf, "(CONFIG) : %s set %s to %s.", GET_NAME(ch), game[l].cmd,
                                                YESNO(binary));
  } else {
    sprintf(buf, "(CONFIG) : %s set %s to %d.", GET_NAME(ch), game[l].cmd,
                                                toggle);
  }
  mudlog(buf, NRM, MIN(GET_LEVEL(ch), LEVEL_IMPL), TRUE);

  write_elite_config();

}


ACMD(do_transet)
{
  char  **msg;
  int transtype;

  half_chop(argument, arg, buf1);
  delete_doubledollar(buf1); // Should enable $n and other act $vars to work

  if (!*arg) {
    send_to_char("Usage:- transet <in|out|view>\r\n"
                 "Used to display unique transfer messsages.\r\n"
                 "Use $n for victim and $N for self (to include is message)\r\n", ch);
    return;
  }

  if (!str_cmp(arg, "in")) transtype = SCMD_TRANSIN;
  else if (!str_cmp(arg, "out")) transtype = SCMD_TRANSOUT;
  else if (!str_cmp(arg, "view")) transtype = SCMD_TRANSVIEW;
  else {
    send_to_char("Usage:- transet <in|out|view>\r\n"
                 "Used to display unique transfer messsages.\r\n"
                 "Use $n for victim and $N for self (to include is message)\r\n", ch);
    return;
  }

  switch (transtype) {
  case SCMD_TRANSIN:
    msg = &(ch->specials.transIn);
    break;
  case SCMD_TRANSOUT:
    msg = &(ch->specials.transOut);
    break;
  case SCMD_TRANSVIEW:
    if (ch->specials.transIn != NULL)
      sprintf(buf, "TransIn : %s\r\n", ch->specials.transIn);
    else
      sprintf(buf, "TransIn : <PERSON> arrives from a puff of smoke.\r\n");
    send_to_char(buf, ch);
    if (ch->specials.transOut != NULL)
      sprintf(buf, "TransOut: %s\r\n", ch->specials.transOut);
    else
      sprintf(buf, "TransOut: <PERSON> disappears in a mushroom cloud.\r\n");
    send_to_char(buf, ch);
    return;
    break;
  default:
    send_to_char("Unknown option. Usage: transet <in|out|view>\r\n", ch);
    return;
    break;
  }

  if (*msg)
    free(*msg);

  if (!*buf1)
    *msg = NULL;
  else
    *msg = strdup(buf1);

  send_to_char("Ok.\r\n", ch);

  SET_BIT(PLR_FLAGS(ch), PLR_SAVESTR);
}


ACMD(do_chanset)
{

  extern sbyte channel_allowed[];

  int binary = 0, i = 0, l = 0;
  char opt[MAX_INPUT_LENGTH], val[MAX_INPUT_LENGTH];

  struct channel_struct {
    char *cmd;
  } comm[] = {
    { "newbie"  },
    { "yell"    },
    { "gossip"  },
    { "auction" },
    { "chat"    },
    { "pksay"   },
    { "grat"    },
    { "\n"      }
  };

  half_chop(argument, opt, val);

  if (!*opt || !*val) {
    sprintf(buf, "Current channel settings are:\r\n\r\n"
                 "Newbie  : #G%-3s#N   "
                 "Yell    : #G%-3s#N   "
                 "Gossip  : #G%-3s#N   "
                 "Auction : #G%-3s#N\r\n"
                 "Chat    : #G%-3s#N   "
                 "PK Say  : #G%-3s#N   "
                 "Grat    : #G%-3s#N\r\n\r\n"
                 "Usage : chanset <option> <value|on|off>\r\n"
                 "You can configure the following:\r\n\r\n",
                 YESNO(channel_allowed[0]), YESNO(channel_allowed[1]),
                 YESNO(channel_allowed[2]), YESNO(channel_allowed[3]),
                 YESNO(channel_allowed[4]), YESNO(channel_allowed[5]),
                 YESNO(channel_allowed[6]));

    send_to_char(buf, ch);

    *buf = '\0';

    while (*(comm[l].cmd) != '\n') {
        i++;
        sprintf(buf, "%s [%-12s]", buf, comm[l].cmd);
        if (!(i % 4))
          strcat (buf, "\r\n");
      l++;
    }
    if (i % 4)
      strcat (buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }


  for (l = 0; *(comm[l].cmd) != '\n'; l++)
    if (!strncmp(opt, comm[l].cmd, strlen(opt)))
      break;

  binary = -1;

  if (!strcmp(val, "on") || !strcmp(val, "yes"))
    binary = 1;
  if (!strcmp(val, "off") || !strcmp(val, "no"))
    binary = 0;
  if (binary == -1) {
    send_to_char("Value must be on or off.\r\n", ch);
    return;
  }

  if (l < 0 || l > 6) {
     send_to_char("Unknown option - type 'chanset' for help.\r\n", ch);
     return;
  }

  channel_allowed[l] = binary;

  sprintf(buf, "Channel: %s now %s.\r\n", comm[l].cmd, YESNO(binary));
  send_to_char(buf, ch);

  sprintf(buf, "(CHANSET) : %s set %s to %s.", GET_NAME(ch), comm[l].cmd,
                                                YESNO(binary));
  mudlog(buf, NRM, MIN(GET_LEVEL(ch), LEVEL_IMPL), TRUE);

}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* do_whereload, finds where an object or mobile loads to using the zone 
   files */
ACMD(do_whereload)
{
  int cmd_no, zone, room = -1, number, found = 0;
  sh_int mob;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH] = {0};
  two_arguments(argument, arg1, arg2);

  if (!*arg2 || !is_number(arg2)) {
    send_to_char("Format: whereload { mob | obj } <number>\r\n", ch);
    return;
  }

  number = atoi(arg2);

  if (is_abbrev(arg1, "mobile")) {
    number = real_mobile(number);

    /* we only have to search the zone files for 'M' lines */
    for (zone = 0; zone < top_of_zone_table; zone++)
      for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
        if (ZCMD.command == 'M')
          if (ZCMD.arg1 == number)
            sprintf(buf + strlen(buf), "[%3.3d] Room %d (max %d)\r\n", ++found,
              world[ZCMD.arg3]->number, ZCMD.arg2);

    if (!found)
      strcat(buf, "No mobiles with that vnum load.\r\n");
  } else if (is_abbrev(arg1, "object")) {
    number = real_object(number);

    /* but this one needs a ton of them :( */
    for (zone = 0, mob = -1; zone < top_of_zone_table; zone++, mob = -1)
      for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
        switch (ZCMD.command) {
          case 'M': /* this is needed just to set mob */
            mob = ZCMD.arg1;
            room = world[ZCMD.arg3]->number;
            break;
          case 'O':
            if (ZCMD.arg1 == number)
              sprintf(buf + strlen(buf), "[%3.3d] In room %d (max %d)\r\n", ++found,
                world[ZCMD.arg3]->number, ZCMD.arg2);
            break;
          case 'G':
            if (mob < 0) continue;
            if (ZCMD.arg1 == number)
              sprintf(buf + strlen(buf), "[%3.3d] On mob %d (%s) in room %d (max %d)\r\n", ++found,
                mob_index[mob].virtual, mob_proto[mob].player.short_descr, room, ZCMD.arg2);
            break;
          case 'P':
            if (ZCMD.arg1 == number)
              sprintf(buf + strlen(buf), "[%3.3d] In obj %d (%s) (max %d) (unknown location)\r\n", ++found,
                obj_index[ZCMD.arg3].virtual, obj_proto[ZCMD.arg3].short_description, ZCMD.arg2);
            break;
          case 'E':
            if (mob < 0) continue;
            if (ZCMD.arg1 == number)
              sprintf(buf + strlen(buf), "[%3.3d] On mob %d (%s) [%s] in room %d (max %d)\r\n", ++found,
                mob_index[mob].virtual, mob_proto[mob].player.short_descr, equipment_types[ZCMD.arg3],
                room, ZCMD.arg2);
            break;
          default:
            break;
        }

    if (!found)
      strcat(buf, "No objects with that vnum load.\r\n");
  } else {
    send_to_char("Format: whereload { mob | obj } <number>\r\n", ch);
    return;
  }
  page_string(ch->desc, buf, 1);
}

/* do_findkey, finds where the key to a door loads to using do_whereload() */
ACMD(do_findkey)
{
  int dir, key, rkey;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Format: findkey <dir>\r\n", ch);
    return;
  }

  switch (*arg) {
    case 'n':
      dir = NORTH; break;
    case 'e':
      dir = EAST; break;
    case 's':
      dir = SOUTH; break;
    case 'w':
      dir = WEST; break;
    case 'u':
      dir = UP; break;
    case 'd':
      dir = DOWN; break;
    default:
      send_to_char("What direction is that?!?\r\n", ch);
      return;
  }
  if (!EXIT(ch, dir)) {
    send_to_char("There's no exit in that direction!\r\n", ch);
    return;
  }
  if ((key = EXIT(ch, dir)->key) <= 0) {
    send_to_char("There's no key for that exit.\r\n", ch);
    return;
  }
  sprintf(buf, "Vnum: %d (%s).\r\n", key, (rkey = real_object(key)) > -1 ? obj_proto[rkey].short_description :
"doesn't exist");
  send_to_char(buf, ch);

  sprintf(buf, "obj %d", key);
  do_whereload(ch, buf, 0, 0);
}

/* do_finddoor, finds the door(s) that a key goes to */
ACMD(do_finddoor)
{
  int d, vnum = 0, num = 0;
  sh_int i;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH] = {0};
  struct char_data *tmp_char;
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Format: finddoor <obj/vnum>\r\n", ch);
    return;
  }

  if (is_number(arg)) {
    /* must be a vnum - easy */
    vnum = atoi(arg);
  } else {
    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_WORLD | FIND_OBJ_EQUIP,
                 ch, &tmp_char, &obj);
    if (!obj) {
      send_to_char("What key do you want to find a door for?\r\n", ch);
      return;
    }
    if (GET_ITEM_TYPE(obj) != ITEM_KEY) {
      act("$p isn't a key, it seems.", FALSE, ch, obj, 0, TO_CHAR);
      return;
    }
    vnum = obj_index[obj->item_number].virtual;
  }
  if (vnum <= 0) {
    send_to_char("Sorry, an unknown error occured.\r\n", ch);
    return;
  }
  for (i = 0; i <= top_of_world; i++)
    for (d = 0; d < NUM_OF_DIRS; d++)
      if (world[i]->dir_option[d] && world[i]->dir_option[d]->key &&
          world[i]->dir_option[d]->key == vnum)
        sprintf(buf + strlen(buf), "[%3d] Room %d, %s (%s)\r\n", ++num, world[i]->number,
          dirs[d], world[i]->dir_option[d]->keyword);

  page_string(ch->desc, buf, 1);
}



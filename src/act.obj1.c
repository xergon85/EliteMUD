/* ************************************************************************
*   File: act.obj1.c                                    Part of EliteMUD  *
*  Usage: object handling routines -- get/drop and container handling     *
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
#include "functions.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data **world;
extern struct clan_data *clan_list;
extern int    top_of_clan;


#define MAX_DONATED_OBJECTS_IN_ROOM  60

int same_clan(struct char_data *ch, struct obj_data *obj);


void
obj_to_don_room(struct obj_data *obj, sh_int room)
{
  int num_of_objs = 0;
  struct obj_data *tmp;

  tmp = world[room]->contents;

  while (tmp && tmp->next_content) {
    ++num_of_objs;
    tmp = tmp->next_content;
  }

  if (tmp && num_of_objs > MAX_DONATED_OBJECTS_IN_ROOM) {
    obj_from_room(tmp);
    extract_obj(tmp);
  }

  obj_to_room(obj, room);
}


/* Function: generic function for object handling
 * Given a character, a list of objects, and some info on what to do, it
 * will perform that task, and set up a list in a string with all the
 * objects handled, in the format:
 * (2) bread, an axe, (3) bag ...
 * All the object handling will use this low level routine.
 * -Petrus 950819
 */
#define MODE_PUT_ALL_CONT    1
#define MODE_PUT_ALLX_CONT   2
#define MODE_PUT_OBJ_CONT    3
#define MODE_GET_ALL_CONT    4
#define MODE_GET_ALLX_CONT   5
#define MODE_GET_OBJ_CONT    6
#define MODE_GET_ALL_ROOM    7
#define MODE_GET_ALLX_ROOM   8
#define MODE_GET_OBJ_ROOM    9
#define MODE_DROP_ALL       10
#define MODE_DROP_ALLX      11
#define MODE_DROP_OBJ       12
#define MODE_GIVE_ALL_CHAR  13
#define MODE_GIVE_ALLX_CHAR 14
#define MODE_GIVE_OBJ_CHAR  15
#define MODE_DONATE_ALLX    16
#define MODE_DONATE_OBJ     17
#define MODE_SAC_ALLX       18
#define MODE_SAC_OBJ        19
#define MODE_SAC_ALLX_ROOM  20
#define MODE_SAC_OBJ_ROOM   21

int
perform_object_handling(struct char_data *ch,
			struct char_data *vict,
			struct obj_data *ls,
			struct obj_data *container,
			char *sname,
			int mode,
			sh_int RDR,
			int amount)
{
  struct obj_data *obj, *next_obj;
  char buffer[LARGE_BUFSIZE], buf2[256], *end;
  int  ptr, len, found = 0, reward = 0, number = 1;
  int add_to_list = FALSE, gold = 0, total_gold = 0, how_many = 0;


  if (!ls || !ch)              /* No valid list, bug */
    return 0;

  *buffer = '\0';
  end = buffer;
  ptr = strlen(GET_NAME(ch)) + 12;

  for (obj = ls; obj; obj = next_obj) {      /* MAIN LOOP */

    switch(mode) {                           /* HOW TO GET NEXT OBJ */
    case MODE_PUT_ALL_CONT:
    case MODE_GET_ALL_CONT:
    case MODE_GET_ALL_ROOM:
    case MODE_DROP_ALL:
    case MODE_GIVE_ALL_CHAR:
      next_obj = obj->next_content;
      break;
    case MODE_PUT_ALLX_CONT:
    case MODE_GET_ALLX_CONT:
    case MODE_GET_ALLX_ROOM:
    case MODE_DROP_ALLX:
    case MODE_GIVE_ALLX_CHAR:
    case MODE_DONATE_ALLX:
    case MODE_SAC_ALLX:
    case MODE_SAC_ALLX_ROOM:
      next_obj = get_obj_in_list_vis(ch, sname, obj->next_content);
      break;
    case MODE_PUT_OBJ_CONT:
    case MODE_GET_OBJ_CONT:
    case MODE_GET_OBJ_ROOM:
    case MODE_DROP_OBJ:
    case MODE_GIVE_OBJ_CHAR:
    case MODE_DONATE_OBJ:
    case MODE_SAC_OBJ:
    case MODE_SAC_OBJ_ROOM:
      if (number < amount)
	next_obj = get_obj_in_list_vis(ch, sname, obj->next_content);
      else
	next_obj = NULL;
      break;
    default:
      log("SYSERR: Incorrect argument passed to perform_object_handling 1");
      return 0;
      break;
    }

    add_to_list = TRUE;

    switch(mode) {                       /* DIFFERENT CHECKS */

    case MODE_PUT_ALL_CONT:
    case MODE_PUT_ALLX_CONT:
    case MODE_PUT_OBJ_CONT:
      if (!container) return 0;
      if (obj != container) {
	if (GET_OBJ_WEIGHT(container) + GET_OBJ_WEIGHT(obj) > container->obj_flags.value[0]) {
	  act("$p won't fit in $P.", FALSE, ch, obj, container, TO_CHAR);
	  add_to_list = FALSE;
	} else {
	  obj_from_char(obj);
	  obj_to_obj(obj, container);
	}
      } else
	add_to_list = FALSE;
      break;

    case MODE_GET_ALL_CONT:
    case MODE_GET_ALLX_CONT:
    case MODE_GET_OBJ_CONT:
      if ((mode != MODE_GET_ALL_CONT || CAN_SEE_OBJ(ch, obj)) && can_take_obj(ch, obj)) {
	if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
	  act("$p: You can't carry anymore items in your hands.", FALSE, ch, obj, 0, TO_CHAR);
	  add_to_list = FALSE;
	} else {
	  obj_from_obj(obj);
	  obj_to_char(obj, ch);
	  if ((gold = get_check_money(ch, obj))) {
	    add_to_list = FALSE;
	    total_gold += gold;
	  }
	}
      } else
	add_to_list = FALSE;
      break;

    case MODE_GET_ALL_ROOM:
    case MODE_GET_ALLX_ROOM:
    case MODE_GET_OBJ_ROOM:
      if ((mode != MODE_GET_ALL_ROOM || CAN_SEE_OBJ(ch, obj)) &&
	  can_take_obj(ch, obj)) {
	obj_from_room(obj);
	obj_to_char(obj, ch);
	if ((gold = get_check_money(ch, obj))) {
	  add_to_list = FALSE;
	  total_gold += gold;
	}
      } else
	add_to_list = FALSE;
      break;

    case MODE_DROP_ALL:
    case MODE_DROP_ALLX:
    case MODE_DROP_OBJ:
      if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) &&
          !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
	sprintf(buf, "You cannot drop %s, it must be CURSED!\r\n", OBJS(obj, ch));
	send_to_char(buf, ch);
	add_to_list = FALSE;
      } else if(ROOM_FLAGGED(IN_ROOM(ch), NO_DROP) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
        sprintf(buf, "A strange force prevents you from dropping %s!\r\n", OBJS(obj, ch));
        send_to_char(buf, ch);
        add_to_list = FALSE;
      } else {
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room);
      }
      break;

    case MODE_GIVE_ALL_CHAR:
    case MODE_GIVE_ALLX_CHAR:
    case MODE_GIVE_OBJ_CHAR:
      add_to_list = FALSE;
      if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) &&
          !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
      else if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict))
	act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
      else if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) > CAN_CARRY_W(vict))
	act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
      else if (GET_ITEM_LEVEL(obj) > GET_LEVEL(vict))
	act("$E can't touch $p.  It's too mighty for $M.",
	    FALSE, ch, obj, vict, TO_CHAR);
      else if (highest_obj_level(obj->contains) > GET_LEVEL(vict))
	act("$E can't touch $p.  It contains something too mighty.",
	    FALSE, ch, obj, vict, TO_CHAR);
      else if (!same_clan(vict, obj) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	   act("$p: Wrong Clan!.", FALSE, ch, obj, 0, TO_CHAR);
      else {
	add_to_list = TRUE;
	obj_from_char(obj);
	obj_to_char(obj, vict);
	mprog_give_trigger(vict, ch, obj);
      }
      break;

    case MODE_DONATE_ALLX:
    case MODE_DONATE_OBJ:
      if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP | ITEM_NODONATE)) {
	sprintf(buf, "You cannot donate %s.\r\n", OBJS(obj, ch));
	send_to_char(buf, ch);
	add_to_list = FALSE;
      } else {
	obj_from_char(obj);
	obj_to_don_room(obj, RDR);
	SET_BIT((obj)->obj_flags.extra_flags, ITEM_DONATED);
      }
      break;

    case MODE_SAC_ALLX:
    case MODE_SAC_OBJ:
    case MODE_SAC_ALLX_ROOM:
    case MODE_SAC_OBJ_ROOM:
      add_to_list = FALSE;
      if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
	sprintf(buf, "You cannot sacrifice %s, it must be CURSED!\r\n", OBJS(obj, ch));
	send_to_char(buf, ch);
      } else if (IS_SET(obj->obj_flags.extra_flags, ITEM_QUEST)) {
        sprintf(buf, "You cannot sacrifice %s.\r\n", OBJS(obj, ch));
        send_to_char(buf, ch);
      } else if (GET_ITEM_LEVEL(obj) > GET_LEVEL(ch)) {
        sprintf(buf, "You are not powerful enough to sacrifice %s.\r\n", OBJS(obj, ch));
        send_to_char(buf, ch);
      } else if ((mode == MODE_SAC_OBJ_ROOM || mode == MODE_SAC_ALLX_ROOM) &&
		 obj->obj_flags.type_flag == ITEM_CONTAINER &&
		 obj->obj_flags.value[3] == 2)
	send_to_char("Your sacrifice is not accepted!\r\n", ch);
      else {
	if (mode == MODE_SAC_OBJ_ROOM || mode == MODE_SAC_ALLX_ROOM)
	  obj_from_room(obj);
	else
	  obj_from_char(obj);
	reward += MAX(1, MIN(200, obj->obj_flags.cost >> 4));
	add_to_list = TRUE;
      }
      break;

    default:
      log("SYSERR: Incorrect argument passed to perform_object_handling 2");
      return 0;
      break;
    }

    ++number;

    if (how_many && !add_to_list) {
      add_to_list = TRUE;
      --how_many;
    }

    if (add_to_list) {
      if (next_obj &&
	  (obj->item_number == next_obj->item_number) &&
	  !str_cmp(obj->short_description, next_obj->short_description))
	{
	  ++how_many;
      } else {
	if (how_many)
	  sprintf(buf2, " (%d) %s,", how_many + 1, OBJS(obj, ch));
	else
	  sprintf(buf2, " %s,", OBJS(obj, ch));
	how_many = 0;

	if (ptr + (len = strlen(buf2)) > 79) {
	  strcat(buffer, "\r\n");
	  ptr = 0;
	}
	strcat(buffer, buf2);
	ptr += len;
	do
	  ++end;
	while (*end != ',');
      }

      ++found;
    }

    if (add_to_list &&
	(mode == MODE_SAC_ALLX_ROOM ||
	 mode == MODE_SAC_OBJ_ROOM  ||
	 mode == MODE_SAC_OBJ       ||
	 mode == MODE_SAC_ALLX))
	extract_obj(obj);


  } /* End of objects to add to list */

  /* add money */
  if (total_gold) {
    sprintf(buf2, " %d gold coin%s", total_gold, (total_gold>1?"s":""));
    if (ptr + (len = strlen(buf2)) > 79) {
      strcat(buffer, "\r\n");
      ptr = 0;
    }
    strcat(buffer, buf2);
    ptr += len;
    found++;
  } else
    *end = '\0';

  if (found) {
    switch(mode) {
    case MODE_PUT_ALL_CONT:
    case MODE_PUT_ALLX_CONT:
    case MODE_PUT_OBJ_CONT:
      act("You put$T in $p.", FALSE, ch, container, (void*) buffer, TO_CHAR);
      act("$n puts$T in $p.", TRUE, ch, container, (void*) buffer, TO_ROOM);
      break;

    case MODE_GET_ALL_CONT:
    case MODE_GET_ALLX_CONT:
    case MODE_GET_OBJ_CONT:
      act("You get$T from $p.", FALSE, ch, container, (void*) buffer, TO_CHAR);
      act("$n gets$T from $p.", TRUE, ch, container, (void*) buffer, TO_ROOM);
      break;

    case MODE_GET_ALL_ROOM:
    case MODE_GET_ALLX_ROOM:
    case MODE_GET_OBJ_ROOM:
      act("You get$T.", FALSE, ch, 0, (void*) buffer, TO_CHAR);
      act("$n gets$T.", TRUE, ch, 0, (void*) buffer, TO_ROOM);
      break;

    case MODE_DROP_ALL:
    case MODE_DROP_ALLX:
    case MODE_DROP_OBJ:
      act("You drop$T.", FALSE, ch, 0, (void*) buffer, TO_CHAR);
      act("$n drops$T.", TRUE, ch, 0, (void*) buffer, TO_ROOM);
    break;

    case MODE_GIVE_ALL_CHAR:
    case MODE_GIVE_ALLX_CHAR:
    case MODE_GIVE_OBJ_CHAR:
      act("You give$t to $N.", FALSE, ch, (void*) buffer, vict, TO_CHAR);
      act("$n gives$t to you.", FALSE, ch, (void*) buffer, vict, TO_VICT);
    act("$n gives$t to $N.", TRUE, ch, (void*) buffer, vict, TO_NOTVICT);
      break;

    case MODE_DONATE_ALLX:
    case MODE_DONATE_OBJ:
      act("You donate$T.", FALSE, ch, 0, (void*) buffer, TO_CHAR);
      act("$n donates$T, disappearing in a puff of smoke.", TRUE, ch, 0, (void*) buffer, TO_ROOM);
      send_to_room("Something suddenly appears in a puff a smoke!\r\n", RDR);
      break;

    case MODE_SAC_ALLX:
    case MODE_SAC_OBJ:
    case MODE_SAC_ALLX_ROOM:
    case MODE_SAC_OBJ_ROOM:
      act("You sacrifice$T.", FALSE, ch, 0, (void*) buffer, TO_CHAR);
      act("$n sacrifices$T, disappearing in a puff of smoke.", TRUE, ch, 0, (void*) buffer, TO_ROOM);
      if (reward) {
	sprintf(buf, "You have been rewarded by the gods with %d coins!\r\n",
		reward);
	send_to_char(buf, ch);
	act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
	GET_GOLD(ch) += reward;
      }
      break;

    default:
      log("SYSERR: Incorrect argument passed to perform_object_handling 3");
      return 0;
    break;

    }
  }

  return found;
}

int check_don(struct char_data *ch)
{
  int i, num_clans;

  num_clans = top_of_clan+1;

  /* Check if we are standing in a donation room and if its not our own return 0 */
  for (i=0;i<num_clans;i++)
    if (ch->in_room == clan_list[i].donation && (CLAN(ch) != i || CLAN_LEVEL(ch) < 2))
      return 0;

  /* Its either our room or not a donation room */
  return 1;
}

/* Get the number of the clan that is the owner of this donation room */
int get_clan_room(sh_int room)
{
  int i, num_clans;

  num_clans = top_of_clan+1;

  /* Check if we are standing in a donation room and
  /  return number of the clan (not real number) */
  for (i=0;i<num_clans;i++)
    if (room == clan_list[i].donation)
      return i;

  /* Return 0 to prevent error use only after you are
  /  sure that it is a clan donation room */
  return 0;
}

/* The following put modes are supported by the code below:

   1) put <object> <container>
   2) put all.<object> <container>
   3) put all <container>
   4) put #NUM <object> <container>

   <container> must be in inventory or on ground.
   all objects to be put into container must be in inventory.
*/

ACMD(do_put)
{
  char	arg1[MAX_INPUT_LENGTH];
  char	arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *container;
  struct char_data *tmp_char;
  int	obj_dotmode, cont_dotmode, amount = 1;

  argument = one_argument(argument, arg1);

  if (is_number(arg1)) {
    amount = atoi(arg1);
    two_arguments(argument, arg1, arg2);
  } else
    one_argument(argument, arg2);

  obj_dotmode = find_all_dots(arg1);
  cont_dotmode = find_all_dots(arg2);

  if (cont_dotmode != FIND_INDIV)
    send_to_char("You can only put things into one container at a time.\r\n", ch);
  else if (!*arg1)
    send_to_char("Put what in what?\r\n", ch);
  else if (!*arg2) {
    sprintf(buf, "What do you want to put %s in?\r\n",
	    ((obj_dotmode != FIND_INDIV) ? "them" : "it"));
    send_to_char(buf, ch);
  } else {
    if (check_don(ch))
      generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &container);
    else
      generic_find(arg2, FIND_OBJ_INV, ch, &tmp_char, &container);
    if (!container)
    {
      if (!check_don(ch))
      {
        generic_find(arg2, FIND_OBJ_ROOM, ch, &tmp_char, &container);
        if (container)
        {
          sprintf(buf, "You have to be a member of %s#N to drop equipment here.\r\n",
                  clan_list[get_clan_room(ch->in_room)].name);
          send_to_char(buf, ch);
        }
        else
        {
          sprintf(buf, "You don't see a %s here.\r\n", arg2);
          send_to_char(buf, ch);
        }
      }
      else
      {
        sprintf(buf, "You don't see a %s here.\r\n", arg2);
        send_to_char(buf, ch);
      }
    } else if (GET_ITEM_TYPE(container) != ITEM_CONTAINER) {
      act("$p is not a container.", FALSE, ch, container, 0, TO_CHAR);
    } else if (IS_SET(container->obj_flags.value[1], CONT_CLOSED)) {
      send_to_char("You'd better open it first!\r\n", ch);
    } else {
      if (obj_dotmode == FIND_ALL) {	    /* "put all <container>" case */
	/* check and make sure the guy has something first */
	if (container == ch->carrying && !ch->carrying->next_content)
	  send_to_char("You don't seem to have anything to put in it.\r\n", ch);
	else
	  perform_object_handling(ch, 0, ch->carrying, container, "", MODE_PUT_ALL_CONT, 0, 1);
      } else if (obj_dotmode == FIND_ALLDOT) {  /* "put all.x <cont>" case */
	if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	  sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
	  send_to_char(buf, ch);
	} else
	  perform_object_handling(ch, 0, obj, container, arg1, MODE_PUT_ALLX_CONT, 0, 1);
      } else {		    /* "put <thing> <container>" case */
	if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	  sprintf(buf, "You aren't carrying %s %s.\r\n", AN(arg1), arg1);
	  send_to_char(buf, ch);
	} else if (obj == container)
	  send_to_char("You attempt to fold it into itself, but fail.\r\n", ch);
	else
	  perform_object_handling(ch, 0, obj, container, arg1, MODE_PUT_OBJ_CONT, 0, amount);
      }
    }
  }
}


/*Help funcs - get the highest obj level in a container if it is a container */
int highest_obj_level(struct obj_data *obj)
{
  if (obj)
    return (MAX(GET_ITEM_LEVEL(obj),
		MAX(highest_obj_level(obj->contains),
		    highest_obj_level(obj->next_content))));
  else
    return 0;
}

int same_clan_cont(struct char_data *ch, struct obj_data *obj)
{
     if (obj) {
	  if (CLANEQ(obj)) {
	       if ((CLANEQ_CLAN(obj) != CLAN(ch)) ||
		    (CLAN_LEVEL(ch) <= 1)) return 0;
	   } else {
		if (!same_clan_cont(ch, obj->contains)) return 0;
		if (!same_clan_cont(ch, obj->next_content)) return 0;
	   }
     } else
	  return 1;

     return 1;
/* returns 0 if fail, 1 if true */
}


int same_clan(struct char_data *ch, struct obj_data *obj)
{
  if (obj) {
    if (CLANEQ(obj)) {
      if ((CLANEQ_CLAN(obj) != CLAN(ch)) ||
	  (CLAN_LEVEL(ch) <= 1)) return 0;
    }
    if (obj->contains) {
      if (!same_clan_cont(ch, obj->contains))
	return 0;
    }
  }
  return 1;
}


int
can_take_obj(struct char_data *ch, struct obj_data *obj)
{
  if (obj_carried_by(obj) == ch)
    return 1;
  else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
    act("$p: You can't carry that many items.", FALSE, ch, obj, 0,  TO_CHAR);
    return 0;
  } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    act("$p: You can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (!(CAN_WEAR(obj, ITEM_TAKE))) {
    act("$p: You can't take that!", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (GET_ITEM_LEVEL(obj) > GET_LEVEL(ch)) {
    act("$p: This item surpasses your might.",
	FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (highest_obj_level(obj->contains) > GET_LEVEL(ch)) {
    act("$p: Contains something too mighty for you.",
	FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (!same_clan(ch, obj) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
    act("$p: Wrong Clan!.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (!check_don(ch) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
    sprintf(buf, "You have to be a member of %s#N to get equipment from here.", clan_list[get_clan_room(ch->in_room)].name);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  }

  return 1;
}


int get_check_money(struct char_data *ch, struct obj_data *obj)
{
  int amount;

  if ((GET_ITEM_TYPE(obj) == ITEM_MONEY) && (obj->obj_flags.value[0] > 0)) {
    obj_from_char(obj);
    GET_GOLD(ch) += (amount = obj->obj_flags.value[0]);
    extract_obj(obj);

    return amount;
  }

  return 0;
}


void
get_from_container(struct char_data *ch, struct obj_data *cont, char *arg, int mode, int amount)
{
  struct obj_data *obj;
  int obj_dotmode, found = 0;
  char looting[80];
  extern int loot_allowed;

  obj_dotmode = find_all_dots(arg);

  if (IS_SET(cont->obj_flags.value[1], CONT_CLOSED))
    act("The $p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_ALL) {    /* ALL */
    found =
      perform_object_handling(ch, 0, cont->contains, cont, "", MODE_GET_ALL_CONT, 0, 1);
    if(found) {
      if(!isname(GET_NAME(ch), cont->short_description))
          if(cont->obj_flags.value[3] == 2)  {
/* Bodpoint - disabled until fixed
            if (GET_LEVEL(ch) < LEVEL_DEITY && !loot_allowed) {
              send_to_char("Looting another players corpse is not allowed.\r\n", ch);
              return;
            } else {
 */
              sprintf(looting, "(LOOT) %s looting %s.", GET_NAME(ch), cont->short_description);
              mudlog(looting, BRF, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
 /*           } */
          }
    }
    if (!found)
      act("$p doesn't contain anything you can get.", FALSE, ch, cont, 0, TO_CHAR);
  } else if (obj_dotmode == FIND_ALLDOT) {
    if (!*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    obj = get_obj_in_list_vis(ch, arg, cont->contains);
    found =
      perform_object_handling(ch, 0, obj, cont, arg, MODE_GET_ALLX_CONT, 0, 1);
    if (!found) {
      sprintf(buf, "You can't find any %ss in $p.", arg);
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    } else {
      if(!isname(GET_NAME(ch), cont->short_description))
          if(cont->obj_flags.value[3] == 2)  {
/* Bodpoint - disabled until fixed
            if (GET_LEVEL(ch) < LEVEL_DEITY && !loot_allowed) {
              send_to_char("Looting another players corpse is not allowed.\r\n", ch);
              return;
            } else {
 */
              sprintf(looting, "(LOOT) %s looting %s.", GET_NAME(ch), cont->short_description);
              mudlog(looting, BRF, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
/*            } */
          }
      }
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains))) {
      sprintf(buf, "There doesn't seem to be %s $T in $p.", AN(arg));
      act(buf, FALSE, ch, cont, (void*) arg, TO_CHAR);
    } else  {
      perform_object_handling(ch, 0, obj, cont, arg, MODE_GET_OBJ_CONT, 0, amount);
      if(!isname(GET_NAME(ch), cont->short_description))
          if(cont->obj_flags.value[3] == 2)  {
/* Bodpoint - disabled until fixed
            if (GET_LEVEL(ch) < LEVEL_DEITY && !loot_allowed) {
              send_to_char("Looting another players corpse is not allowed.\r\n", ch);
              return;
            } else {
 */
              sprintf(looting, "(LOOT) %s looting %s.", GET_NAME(ch), cont->short_description);
              mudlog(looting, BRF, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
/*            } */
          }
    }
  }
}


void get_from_room(struct char_data *ch, char *arg, int amount)
{
  struct obj_data *obj;
  int	dotmode, found = 0;

  dotmode = find_all_dots(arg);

  if (dotmode == FIND_ALL) {
    found =
      perform_object_handling(ch, 0, world[ch->in_room]->contents, 0, "", MODE_GET_ALL_ROOM, 0, 1);

    if (!found)
      send_to_char("There doesn't seem to be anything you can get here.\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents))) {
      sprintf(buf, "You don't see any %ss here.\r\n", arg);
      send_to_char(buf, ch);
    } else
      perform_object_handling(ch, 0, obj, 0, arg, MODE_GET_ALLX_ROOM, 0, 1);
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents))) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    } else
      perform_object_handling(ch, 0, obj, 0, arg, MODE_GET_OBJ_ROOM, 0, amount);
  }
}



ACMD(do_get)
{
  char	arg1[MAX_INPUT_LENGTH];
  char	arg2[MAX_INPUT_LENGTH];

  int	cont_dotmode, found = 0, mode, amount = 0;
  struct obj_data *cont, *next_cont;
  struct char_data *tmp_char;

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    send_to_char("Your arms are already full!\r\n", ch);
  else {
    argument = one_argument(argument, arg1);
    if (is_number(arg1)) {
      amount = atoi(arg1);
      two_arguments(argument, arg1, arg2);
    } else
      one_argument(argument, arg2);

    if (!*arg1)
      send_to_char("Get what?\r\n", ch);
    else {
      if (!*arg2)
	get_from_room(ch, arg1, amount);
      else {
	cont_dotmode = find_all_dots(arg2);
	if (cont_dotmode == FIND_ALL) { /* use all in inv. and on ground */
	  for(cont = ch->carrying; cont; cont = cont->next_content)
	    if (GET_ITEM_TYPE(cont) == ITEM_CONTAINER) {
	      found = 1;
	      get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
	    }
	  for(cont = world[ch->in_room]->contents; cont; cont = cont->next_content)
	    if (CAN_SEE_OBJ(ch, cont) && GET_ITEM_TYPE(cont) == ITEM_CONTAINER) {
	      found = 1;
	      get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
	    }
	  if (!found)
	    send_to_char("You can't seem to find any containers.\r\n", ch);
	} else if (cont_dotmode == FIND_ALLDOT) {
	  if (!*arg2) {
	    send_to_char("Get from all of what?\r\n", ch);
	    return;
	  }
	  cont = get_obj_in_list_vis(ch, arg2, ch->carrying);
	  while (cont) {
	    found = 1;
	    next_cont = get_obj_in_list_vis(ch, arg2, cont->next_content);
	    if (GET_ITEM_TYPE(cont) != ITEM_CONTAINER)
	      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    else
	      get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
	    cont = next_cont;
	  }
	  cont = get_obj_in_list_vis(ch, arg2, world[ch->in_room]->contents);
	  while (cont) {
	    found = 1;
	    next_cont = get_obj_in_list_vis(ch, arg2, cont->next_content);
	    if (GET_ITEM_TYPE(cont) != ITEM_CONTAINER)
	      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    else
	      get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
	    cont = next_cont;
	  }
	  if (!found) {
	    sprintf(buf, "You can't seem to find any %ss here.\r\n", arg2);
	    send_to_char(buf, ch);
	  }
	} else {		/* get <items> <container> (no all or all.x) */
	  mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
	  if (!cont) {
	    sprintf(buf, "You don't have %s %s.\r\n", AN(arg2), arg2);
	    send_to_char(buf, ch);
	  } else if (GET_ITEM_TYPE(cont) != ITEM_CONTAINER)
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	  else
	    get_from_container(ch, cont, arg1, mode, amount);
	}
      }
    }
  }
}


void perform_drop_gold(struct char_data *ch, int amount, byte mode, sh_int RDR)
{
  struct obj_data *obj;

  if (amount <= 0)
    send_to_char("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
  else if (GET_GOLD(ch) < amount)
    send_to_char("You don't have that many coins!\r\n", ch);
  else {
    if (mode != SCMD_JUNK) {
      WAIT_STATE(ch, PULSE_VIOLENCE);   /* to prevent coin-bombing */
      obj = create_money(amount);
      if (mode == SCMD_DONATE) {
	send_to_char("You throw some gold into the air where it disappears in a puff of smoke!\r\n", ch);
	act("$n throws some gold into the air where it disappears in a puff of smoke!", FALSE, ch, 0, 0, TO_ROOM);
	send_to_room("Some gold suddenly appears in mid-air, then drops to the ground!\r\n", RDR);
	obj_to_room(obj, RDR);
      } else {
	send_to_char("You drop some gold.\r\n", ch);
	act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, ch->in_room);
      }
    } else {
      act("$n drops some gold which disappears in a puff of smoke!", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You drop some gold which disappears in a puff of smoke!\r\n", ch);
    }
    GET_GOLD(ch) -= amount;
  }
}


ACMD(do_drop)
{
  extern sh_int donation_room_1;
  struct obj_data *obj;
  sh_int RDR = 0;
  int	dotmode, amount = 0;

  /* Check if its a clan donation room and if you are in the clan */
  // TODO: check if the item that we want to do something with is in
  //       the room instead of just blocking it all...
  if (!check_don(ch))
  {
    if (CMD_IS("donate"))
      sprintf(buf, "You have to be a member of %s#N to donate equipment from here.\r\n", clan_list[get_clan_room(ch->in_room)].name);
    else
      if (CMD_IS("sacrifice"))
        sprintf(buf, "You have to be a member of %s#N to sacrifice equipment from here.\r\n", clan_list[get_clan_room(ch->in_room)].name);
      else
        if (CMD_IS("drop"))
          sprintf(buf, "You have to be a member of %s#N to drop equipment here.\r\n", clan_list[get_clan_room(ch->in_room)].name);
        else
          sprintf(buf, "You are not a member of %s#N.\r\n", clan_list[get_clan_room(ch->in_room)].name);

    send_to_char(buf, ch);
    return;
  }

  if (CMD_IS("donate")) {
	/* prevent eq from dropping down in clan donation room - Charlene
    switch (number(0, 1)) {
    case 0: RDR = 0; break;
    case 1: RDR = (CLAN(ch) >= 0?CLAN_LEVEL(ch) >= 2?clan_list[CLAN(ch)].donation:0:0); break;
    }
    if (RDR <= 0)*/
    RDR = real_room(donation_room_1);
    if (RDR == NOWHERE) {
      send_to_char("Sorry, you can't donate anything right now.\r\n", ch);
      return;
    }
  } /* IF DONATE */

  argument = one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf, ch);
    return;
  } else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("gold", arg) || !str_cmp("coins", arg) || !str_cmp("coin", arg)) {
      perform_drop_gold(ch, amount, subcmd, RDR);
      return;
    }
  }

  dotmode = find_all_dots(arg);

  /* Can't junk or donate all */
  if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
    if (subcmd == SCMD_JUNK)
      send_to_char("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
    else
      send_to_char("Go do the donation room if you want to donate EVERYTHING!\r\n", ch);
    return;
  }

  if (dotmode == FIND_ALL) {
    if (!ch->carrying)
      send_to_char("You don't seem to be carrying anything.\r\n", ch);
    else
      perform_object_handling(ch, 0, ch->carrying, 0, "", MODE_DROP_ALL, RDR, 1);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg) {
      sprintf(buf, "What do you want to %s all of?\r\n", CMD_NAME);
      send_to_char(buf, ch);
      return;
    }
    if (subcmd == SCMD_JUNK &&
	(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents))) {
      perform_object_handling(ch, 0, obj, 0, arg, MODE_SAC_ALLX_ROOM, RDR, 1);
    } else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
      send_to_char(buf, ch);
    } else
      switch(subcmd) {
      case SCMD_DONATE:
	perform_object_handling(ch, 0, obj, 0, arg, MODE_DONATE_ALLX, RDR, 1);
	break;
      case SCMD_JUNK:
	perform_object_handling(ch, 0, obj, 0, arg, MODE_SAC_ALLX, RDR, 1);
	break;
      default:
	perform_object_handling(ch, 0, obj, 0, arg, MODE_DROP_ALLX, RDR, 1);
	break;
      }
  } else {
    if (subcmd == SCMD_JUNK &&
	(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents))) {
      perform_object_handling(ch, 0, obj, 0, arg, MODE_SAC_OBJ_ROOM, RDR, amount);
    } else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    } else
      switch(subcmd) {
      case SCMD_DONATE:
	perform_object_handling(ch, 0, obj, 0, arg, MODE_DONATE_OBJ, RDR, amount);
	break;
      case SCMD_JUNK:
	perform_object_handling(ch, 0, obj, 0, arg, MODE_SAC_OBJ, RDR, amount);
	break;
      default:
	perform_object_handling(ch, 0, obj, 0, arg, MODE_DROP_OBJ, RDR, amount);
	break;
      }
  }
}



/* utility function for give */
struct char_data *give_find_vict(struct char_data *ch, char *arg1)
{
  struct char_data *vict;
  char arg2[MAX_INPUT_LENGTH];

  strcpy(buf, arg1);
  two_arguments(buf, arg1, arg2);
  if (!*arg1) {
    send_to_char("Give what to who?\r\n", ch);
    return 0;
  } else if (!*arg2) {
    send_to_char("To who?\r\n", ch);
    return 0;
  } else if (!(vict = get_char_room_vis(ch, arg2))) {
    send_to_char("No-one by that name here.\r\n", ch);
    return 0;
  } else if (vict == ch) {
    send_to_char("What's the point of that?\r\n", ch);
    return 0;
  } else
    return vict;
}


void perform_give_gold(struct char_data *ch, struct char_data *vict, int amount)
{

  if (amount <= 0) {
    send_to_char("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
    return;
  }

  if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_GREATER))) {
    send_to_char("You haven't got that many coins!\r\n", ch);
    return;
  }

  send_to_char("Ok.\r\n", ch);
  sprintf(buf, "$n gives you %d gold coins.", amount);
  MOBTrigger = FALSE;
  act(buf, FALSE, ch, 0, vict, TO_VICT);
  MOBTrigger = FALSE;
  act("$n gives some gold to $N.", TRUE, ch, 0, vict, TO_NOTVICT);
  mprog_bribe_trigger(vict, ch, amount);
  if (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_GREATER))
    GET_GOLD(ch) -= amount;
  GET_GOLD(vict) += amount;
}


ACMD(do_give)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int	amount = 0, dotmode;
  struct char_data *vict = 0;
  struct obj_data *obj;

  half_chop(argument, arg1, arg2);

  /* Jail -- Phobos */

  if (ROOM_FLAGGED(ch->in_room, GODROOM) && ROOM_FLAGGED(ch->in_room, LAWFULL) && ROOM_FLAGGED(ch->in_room, NO_DROP)) {
    send_to_char("You cannot give items away in jail.\r\n", ch);
    return;
  }

  if (!*arg1) {
    send_to_char("Give what to who?\r\n", ch);
    return;
  } else if (is_number(arg1)) {
    amount = atoi(arg1);
    if (!(vict = give_find_vict(ch, arg2)))
      return;
    if (!str_cmp("gold", arg2) || !str_cmp("coins", arg2) || !str_cmp("coin", arg2)) {
      perform_give_gold(ch, vict, amount);
      return;
    }
    argument = arg2;
  }

  if (!vict && !(vict = give_find_vict(ch, argument)))
    return;
  dotmode = find_all_dots(argument);
  if (dotmode == FIND_ALL) {
    if (!ch->carrying)
      send_to_char("You don't seem to be holding anything.\r\n", ch);
    else
      perform_object_handling(ch, vict, ch->carrying, 0, "", MODE_GIVE_ALL_CHAR, 0, 1);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*argument) {
      send_to_char("All of what?\r\n", ch);
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, argument, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", argument);
      send_to_char(buf, ch);
    } else
      perform_object_handling(ch, vict, obj, 0, argument, MODE_GIVE_ALLX_CHAR, 0, 1);

  } else {
    if (!(obj = get_obj_in_list_vis(ch, argument, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(argument), argument);
      send_to_char(buf, ch);
    } else
      perform_object_handling(ch, vict, obj, 0, argument, MODE_GIVE_OBJ_CHAR, 0, amount);
  }
}

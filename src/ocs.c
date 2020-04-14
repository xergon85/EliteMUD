/* ************************************************************************
 *   File: ocs.c                                         Part of EliteMUD  *
 *  Usage: Online Creation System                                          *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1994 EliteMud RIT                                        *
 *  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
 ************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "ocs.h"
#include "db.h"
#include "interpreter.h"
#include "functions.h"

extern struct room_data **world;
extern int	top_of_world;
extern int      mini_mud;
extern struct char_data *character_list;
extern struct char_data *mob_proto;;		
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern struct room_list *room_crash_list;
extern int	top_of_zone_table;	
extern int	top_of_world;
extern char	*sector_types[];
extern char	*room_bits[];
extern char	*dirs[];
extern char     *exit_bits[];
extern char	*item_types[];
extern char	*wear_bits[];
extern char	*extra_bits[];
extern char	*affected_bits[];
extern char	*pc_class_types[]; 


/* FOR THE OCS SYSTEM */
extern int     ocs_rooms;
extern int     ocs_room_buffer;
extern int     ocs_objs;
extern int     ocs_mobs;
extern int     ocs_zones;
extern int     ocs_shops;

extern sh_int r_immort_start_room;
extern sh_int r_mortal_start_room;
extern sh_int r_frozen_start_room;



/********************************************************************
 * PRINT_ROOM Used for printing rooms when you are on line creating! *
 *********************************************************************/

#define CHECK_NULL(ptr,to_return) ((ptr)== NULL ? to_return : ptr)


void
print_room(struct char_data *ch, struct room_data *room)
{
  char buffer[LARGE_BUFSIZE], minibuf[SMALL_BUFSIZE];
    
  struct extra_descr_data *descr;
  int i;

  send_to_char("§r 0. §yQuit OCS REDIT and save!§N\r\n", ch);
    
  sprintf(buffer, "§r 1. ROOM NAME   :§N %s\r\n", room->name);
  send_to_char(buffer, ch);
    
  sprinttype(room->sector_type, sector_types, minibuf);
  sprintf(buffer, "§r 2. SECTOR TYPE :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintbit((long) room->room_flags, room_bits, minibuf);
  sprintf(buffer, "§r 3. ROOM FLAGS  :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintf(buffer, "§r 4. ROOM DESCR  :§N\r\n%s",
	  CHECK_NULL(room->description,"<NOT_SET>"));
  send_to_char(buffer, ch);

  if(room->ex_description) {
    sprintf(buffer, "§r 5. EXTRA DESCR  :§N");
    for(descr = room->ex_description; descr; descr = descr->next) {
      strcat(buffer, " ");
      strcat(buffer, descr->keyword);
    }
    strcat(buffer, "\r\n");
    send_to_char(buffer, ch);
  } else
    send_to_char("§r 5. EXTRA DESCR  :§N <NO EXTRA DESCRIPTION>\r\n", ch);
    
  for(i = 0; i < NUM_OF_DIRS; i++) {
    sprintf(buffer, "     EXIT: §N%-5s TO: [%5d]\r\n",
	    dirs[i],
	    (room->dir_option[i]?
	     world[room->dir_option[i]->to_room]->number:
	     -1));
    send_to_char(buffer, ch);
  }
    
  send_to_char("§r99. §yABORT OCS REDIT!§N\r\n", ch);
  send_to_char("Change what > ", ch);
}


void
print_exit(struct char_data *ch, struct room_direction_data *dir)
{
  char buffer[SMALL_BUFSIZE], minibuf[SMALL_BUFSIZE];
  
  sprintf(buffer, "§r 0. §yEXIT AND SAVE§N\r\n");
  send_to_char(buffer, ch);
  
  sprintf(buffer, "§r 1. EXIT-DESC     :§N\r\n%s",
	  CHECK_NULL(dir->general_description,"<NOT_SET>\r\n"));
  send_to_char(buffer, ch);
  
  sprintf(buffer, "§r 2. EXIT-KEYWORDS :§N \"%s\"\r\n",
	  CHECK_NULL(dir->keyword,"<NOT SET>"));
  send_to_char(buffer, ch);
  
  sprintbit((long) dir->exit_info, exit_bits, minibuf);
  sprintf(buffer, "§r 3. EXIT INFO     :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintf(buffer, "§r 4. EXIT-KEY      :§N [%d]\r\n",
	  dir->key);
  send_to_char(buffer, ch);
  
  sprintf(buffer, "§r 5. EXIT-TO-ROOM  :§N [%d] RNUM\r\n",
	  dir->to_room);
  send_to_char(buffer, ch);
  
  send_to_char("§r 99. §yPurge this exit!§N\r\n", ch);
  send_to_char("Change what > ", ch);
}

void
print_object(struct char_data *ch, struct obj_data *obj)
{
  char buffer[LARGE_BUFSIZE], minibuf[SMALL_BUFSIZE];
    
  struct extra_descr_data *descr;

  send_to_char("§r 0. §yQuit OCS OEDIT and save!§N\r\n", ch);
    
  sprinttype(obj->obj_flags.type_flag, item_types, minibuf);
  sprintf(buffer, "§N 1. OBJECT TYPE:§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintf(buffer, "§r 2. OBJECT NAMELIST:§N %s\r\n", obj->name);
  send_to_char(buffer, ch);
  sprintf(buffer, "§r 3. OBJECT ROOMDESC:§N %s\r\n", obj->description);
  send_to_char(buffer, ch);
  sprintf(buffer, "§r 4. OBJECT INV.NAME:§N %s\r\n", obj->short_description);
  send_to_char(buffer, ch);
  sprintf(buffer, "§r 5. OBJECT ACT.DESC:§N %s\r\n", obj->action_description);
  send_to_char(buffer, ch);

  if(obj->ex_description) {
    sprintf(buffer, "§N 6. EXTRA DESCR  :§N");
    for(descr = obj->ex_description; descr; descr = descr->next) {
      strcat(buffer, " ");
      strcat(buffer, descr->keyword);
    }
    strcat(buffer, "\r\n");
    send_to_char(buffer, ch);
  } else
    send_to_char("§N 6. EXTRA DESCR  :§N <NO EXTRA DESCRIPTION>\r\n", ch);
    
  sprintf(buffer, "§N 7. OBJECT LEVEL:§N %d\r\n", obj->obj_flags.level);
  send_to_char(buffer, ch);

  sprintbit((long) obj->obj_flags.wear_flags, wear_bits, minibuf);
  sprintf(buffer, "§y 8. WEAR FLAGS  :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintbit((long) obj->obj_flags.extra_flags, extra_bits, minibuf);
  sprintf(buffer, "§y 9. EXTRA FLAGS :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintbit((long) obj->obj_flags.anticlass, &pc_class_types[1], minibuf);
  sprintf(buffer, "§y10. ANTICLASSES :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintbit((long) obj->obj_flags.bitvector, affected_bits, minibuf);
  sprintf(buffer, "§y11. AFFECTING   :§N %s\r\n", minibuf);
  send_to_char(buffer, ch);

  sprintf(buffer, "§N12. OBJECT COST :§N %d\r\n", obj->obj_flags.cost);
  send_to_char(buffer, ch);

  sprintf(buffer, "§N13. OBJECT WEIGHT:§N %d\r\n", obj->obj_flags.weight);
  send_to_char(buffer, ch);

  sprintf(buffer, "§y2x. OBJECT VALUES:§N 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d\r\n", obj->obj_flags.value[0],obj->obj_flags.value[1],obj->obj_flags.value[2],obj->obj_flags.value[3],obj->obj_flags.value[4],obj->obj_flags.value[5]);
  send_to_char(buffer, ch);

  /* NO_RENT TOGGLE */
  sprintf(buffer, "§r50. NO_RENT TOGGLE:§N Currently set to %s\r\n",
          IS_SET((obj->obj_flags.extra_flags), ITEM_NORENT) ? "NORENT" : "RENT");
  send_to_char(buffer, ch);

  /* NO_SWEEP TOGGLE */
  sprintf(buffer, "§r51. NO_SWEEP TOGGLE:§N Currently set to %s\r\n",
          IS_SET((obj->obj_flags.extra_flags), ITEM_NOSWEEP) ? "NOSWEEP" : "SWEEP");
  send_to_char(buffer, ch);

  /* NO_LOCATE TOGGLE */
  sprintf(buffer, "§r52. NO_LOCATE TOGGLE:§N Currently set to %s\r\n",
          IS_SET((obj->obj_flags.extra_flags), ITEM_NOLOCATE) ? "NOLOCATE" : "LOCATE");
  send_to_char(buffer, ch);

  send_to_char("§r99. §yABORT OCS OEDIT!§N\r\n", ch);
  send_to_char("Change what > ", ch);
}


void
print_mob(struct char_data *ch, struct char_data *mob)
{
  char buffer[LARGE_BUFSIZE];
  
  send_to_char("§r 0. §yQuit OCS MEDIT and save!§N\r\n", ch);
  
  sprintf(buffer, "§r 1. MOB NAMELIST:§N %s\r\n", mob->player.name);
  send_to_char(buffer, ch);
  sprintf(buffer, "§r 2. MOB NAME:§N %s\r\n", mob->player.short_descr);
  send_to_char(buffer, ch);
  sprintf(buffer, "§r 3. MOB ROOM DESC:§N %s\r\n", mob->player.long_descr);
  send_to_char(buffer, ch);
  sprintf(buffer, "§r 4. MOB LONG DESC:§N %s\r\n", mob->player.description);
  send_to_char(buffer, ch);

  send_to_char("§r99. §yABORT OCS MEDIT!§N\r\n", ch);
  send_to_char("Change what > ", ch);
}


void
print_bits(char **vec, char *buf)
{
  int i = 0;
    
  while(*vec[i] != '\n') {
    sprintf(buf, "%s %2d. %-10s", buf, i+1,vec[i]);
    if(++i % 4 == 0)
      strcat(buf,"\r\n");
  }
  if (i % 4 != 0)
    strcat(buf,"\r\n");
}

void
print_sector_type(struct char_data *ch)
{
  char buffer[MAX_STRING_LENGTH];
    
  *buffer = '\0';
  print_bits(sector_types, buffer);
  send_to_char(buffer, ch);

  send_to_char("§rChange to > §N", ch);
    
}

void
get_sector_type(struct char_data *ch, struct room_data *room, int type)
{
  if(type < 1 || type > SECT_MAX){
    send_to_char("§o§rINVALID CHOICE!!§N\r\n§rChange to > §N", ch);
    return;
  }

  room->sector_type = (byte) (type - 1);
}


void
print_room_flags(struct char_data *ch, struct room_data *room)
{
  char buffer[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];
 
  sprintbit((long) room->room_flags, room_bits, buf2);

  sprintf(buffer, "Current: %s\r\n", buf2);
  print_bits(room_bits, buffer);
  send_to_char(buffer, ch);
    
  send_to_char("§rToggle what > §N", ch);
}


void
print_obj_flags(struct char_data *ch, struct obj_data *obj, int type)
{
  char buffer[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];

  switch(type) {
    case 8:   /* Wear Flags */
      sprintbit((long) obj->obj_flags.wear_flags, wear_bits, buf2);
      sprintf(buffer, "Current: %s\r\n", buf2);
      print_bits(wear_bits, buffer);
      break;
    case 9:   /* Extra Flags */
      sprintbit((long) obj->obj_flags.extra_flags, extra_bits, buf2);
      sprintf(buffer, "Current: %s\r\n", buf2);
      print_bits(extra_bits, buffer);
      break;
    case 10:  /* Anti-class Flags */
      sprintbit((long) obj->obj_flags.anticlass, pc_class_types, buf2);
      sprintf(buffer, "Current: %s\r\n", buf2);
      print_bits(pc_class_types, buffer);
      break;
    case 11:  /* Affecting Flags */
      sprintbit((long) obj->obj_flags.bitvector, affected_bits, buf2);
      sprintf(buffer, "Current: %s\r\n", buf2);
      print_bits(affected_bits, buffer);
      break;
  }

  send_to_char(buffer, ch);
  send_to_char("§rToggle what > §N", ch);
}


void
print_exit_info(struct char_data *ch, struct room_direction_data *dir)
{
  char buffer[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];
  
  sprintbit((long) dir->exit_info, exit_bits, buf2);
  
  sprintf(buffer, "Current: %s\r\n", buf2);
  print_bits(exit_bits, buffer);
  send_to_char(buffer, ch);
    
  send_to_char("§rToggle what > §N", ch);
}


void
make_new_extra(struct extra_descr_data **descr)
{
  CREATE((*descr), struct extra_descr_data, 1);

  (*descr)->keyword = NULL;
  (*descr)->description = NULL;
  (*descr)->next = NULL;
}

void
get_room_flags(struct char_data *ch, struct room_data *room, int type)
{
  if(type < 1 || type > MAX_ROOM_FLAGS){
    send_to_char("§o§rINVALID CHOICE!!§N\r\n§rToggle what > §N", ch);
    return;
  }

  TOGGLE_BIT(room->room_flags,(long)(1 << (type -1)));
}

void
get_exit_info(struct char_data *ch, struct room_direction_data *dir, int type)
{
  if(type < 1 || type > MAX_EXIT_INFO){
    send_to_char("§o§rINVALID CHOICE!!§N\r\n§rToggle what > §N", ch);
    return;
  }

  TOGGLE_BIT(dir->exit_info,(long)(1 << (type -1)));
}


void
print_extra_desc(struct char_data *ch, struct extra_descr_data **descr)
{
  char buffer[MAX_STRING_LENGTH];
    
  if(!(*descr))
    make_new_extra(descr);
    
  sprintf(buffer, "§r1. Keywords    :§N %s\r\n", 
	  CHECK_NULL((*descr)->keyword, "<NOT SET>"));
  send_to_char(buffer, ch);

  sprintf(buffer, "§r2. Description :§N\r\n%s",
	  CHECK_NULL((*descr)->description, "<NOT SET>\r\n"));
  send_to_char(buffer, ch);

  sprintf(buffer, "§r3. New Extra   :\r\n   Prev. Extra :§N %s\r\n",
	  ((*descr)->next?
	   CHECK_NULL((*descr)->next->keyword, "<NOT SET>"):
	   "<NOT SET>"));
  send_to_char(buffer, ch);

  send_to_char("§r4. Delete This Extra Descr!!!\r\n", ch);
  send_to_char("§rChange What >§N ", ch);

}


int check_extra_descr(struct extra_descr_data *descr)
{
  if (!descr || !descr->keyword || !descr->description)
    return 0;

  return 1;
}


void
free_extra_descr(struct extra_descr_data **descr)
{
  struct extra_descr_data *temp;

  if(!descr)
    return;

  if((*descr)->keyword)
    free((*descr)->keyword);
  if((*descr)->description)
    free((*descr)->description);
  temp = *descr;
  *descr = temp->next;
  free(temp);
}


void
change_string(char **old,char *new)
{
    
  if(*old)
    free(*old);
  *old = strdup(new);
}


/* function to adjust the exits after inserting a room to the world_list */

void
adjust_reset_cmds(struct reset_com *com, sh_int rnum)
{
  register int i;

  for (i = 0;com[i].command != 'S';i++) {
    switch(com[i].command) {
	    
    case 'M':
    case 'O':
      if (com[i].arg3 >= rnum)
	com[i].arg3++;
      break;
    case 'R':
    case 'D':
      if (com[i].arg1 >= rnum)
	com[i].arg1++;
      break;
    }
  }
}

void 
adjust_exits(sh_int rnum)
{
  register int room, door;
    
  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door <= 5; door++)
      if (world[room]->dir_option[door])
	if (world[room]->dir_option[door]->to_room >= rnum)
	  (world[room]->dir_option[door]->to_room)++;
}

void
adjust_crashrooms(sh_int rnum)
{
  register struct room_list *room_ls;

  room_ls = room_crash_list;

  while (room_ls) {
    if (room_ls->number >= rnum)
      room_ls->number++;
    room_ls = room_ls->next;
  }
}

void
adjust_chars(sh_int rnum)
{
  register struct char_data *tch;

  tch = character_list;
    
  while(tch) {
    if (IN_ROOM(tch) >= rnum)
      IN_ROOM(tch)++;
    if (IS_NPC(tch) && tch->player.hometown >= rnum)
      tch->player.hometown++;

    tch = tch->next;
  }
}

void
adjust_objs(sh_int rnum)
{
  register struct obj_data *tobj;

  tobj = object_list;

  while(tobj) {
    if (tobj->in_room >= rnum)
      tobj->in_room++;
    tobj = tobj->next;
  }
}

void
adjust_zones(sh_int rnum)
{
  register int i;

  for (i = 0;i <= top_of_zone_table;i++) {
    if (zone_table[i].lowest == rnum) {
      if (i > 0 && zone_table[i-1].top >= world[rnum]->number) {
	zone_table[i-1].highest++;
	zone_table[i].lowest++;
      }
    } else if (zone_table[i].lowest > rnum)
      zone_table[i].lowest++;
    if (zone_table[i].highest >= rnum)
      zone_table[i].highest++;
    adjust_reset_cmds(zone_table[i].cmd, rnum);
  }
}


/* function to free an ex_description list */
void free_ex_descriptions(struct extra_descr_data *ls)
{
  if (ls) {
    if (ls->next)
      free_ex_descriptions(ls->next);
    if (ls->keyword)
      free(ls->keyword);
    if (ls->description)
      free(ls->description);
    free(ls);
  }
}


/* function to free a direction struct and it's allocated */
void free_dir_data(struct room_direction_data *dir)
{
  if (dir) {
    if (dir->general_description)
      free(dir->general_description);
    if (dir->keyword)
      free(dir->keyword);
    free(dir);
  }
}


/* function to free a room and it's allocated */
void free_room(struct room_data *room)
{
  int i;

  if (room) {
    if (room->name)
      free(room->name);
    if (room->description)
      free(room->description);
    
    free_ex_descriptions(room->ex_description);
    
    for (i = 0; i < NUM_OF_DIRS; i++)
      free_dir_data(room->dir_option[i]);

    free(room);
  }
}

/* Moves a block of rooms one step up
 * Assumes there is place in the vector for this operation
 */
void  move_roomblock(int from, int to)
{
  register int i;

  for (i = to; i >= from; i--)
    world[i + 1] = world[i];
}


/* function to add a new room to existing world_list 
 * Assumes that world_list has one more allocated place */

void  insert_room(struct room_data *room)
{
  register int i;
  int vnum;

  if (!room)
    return;

  vnum = room->number;

  if (!IS_SET(room->room_flags, OCS)) {
    SET_BIT(room->room_flags, OCS);
    ocs_rooms++;
  }

  if ((i = real_room(vnum)) > -1) {
    room->light     = world[i]->light;
    room->contents  = world[i]->contents;
    room->people    = world[i]->people;

    free_room(world[i]);
    world[i] = room;
    
    return;
  }

  if (ocs_room_buffer >= OCS_ROOMS) {
    mudlog("(SYSERR): OCS ROOM BUFFER FULL - COULDN'T INSERT NEW ROOM",
	   BRF, LEVEL_DEITY, FALSE);
    return;
  }

  for (i = top_of_world; i >= 0; i--)
    if (world[i]->number > vnum) {
      world[i + 1] = world[i];
    } else {
      world[i+1] = room;
      break;
    }

  if (i == 0)
    world[0] = room;
  else
    i++;

  adjust_exits(i);
  adjust_crashrooms(i);
  adjust_chars(i);
  adjust_objs(i);
  adjust_zones(i);

  if (r_immort_start_room >= i)
    r_immort_start_room++;
  if (r_mortal_start_room >= i)
    r_mortal_start_room++;
  if (r_frozen_start_room >= i)
    r_frozen_start_room++;


  ocs_room_buffer++;
  top_of_world++;
}


/* ex_descr_cpy: copies the ex_description */
void ex_descr_cpy(struct extra_descr_data **to, struct extra_descr_data *from)
{
  if (from) {
    CREATE(*to, struct extra_descr_data, 1);
    
    if (from->keyword)
      (*to)->keyword = strdup(from->keyword);
    if (from->description)
      (*to)->description = strdup(from->description);
    if (from->next)
      ex_descr_cpy(&(*to)->next, from->next);
  } else
    *to = NULL;
}


/* dir_data_cpy: copies the dir_data */
void dir_data_cpy(struct room_direction_data **to, struct room_direction_data *from)
{
  if (from) {
    CREATE(*to, struct room_direction_data, 1);
    
    **to = *from;
    
    if (from->general_description)
      (*to)->general_description = strdup(from->general_description);
    if (from->keyword)
      (*to)->keyword = strdup(from->keyword);
  } else
    *to = NULL;
}

/* dir_cpy: copies dir_data to another */
void dir_cpy(struct room_direction_data *to, struct room_direction_data *from)
{
  if (from) {
    *to = *from;
    
    if (from->general_description)
      to->general_description = strdup(from->general_description);
    if (from->keyword)
      to->keyword = strdup(from->keyword);
  } else {
    to->general_description = NULL;
    to->keyword = NULL;
    to->exit_info = 0;
    to->key = -1;
    to->to_room = NOWHERE;
  }
}


/* room_cpy: copies a room to another */
void room_cpy(struct room_data *to, struct room_data *from)
{
  int i;
    
  *to = *from;

  if (from->name)
    to->name = strdup(from->name);
  if (from->description)
    to->description = strdup(from->description);
    
  /* copy the extra desc etc also */
  ex_descr_cpy(&(to->ex_description), from->ex_description); 
    
  for (i = 0;i < NUM_OF_DIRS; i++)
    dir_data_cpy(&(to->dir_option[i]), from->dir_option[i]);

  to->funct = from->funct;

  to->light = 0;
  to->contents = NULL;
  to->people = NULL;
}

ACMD(do_redit)
{
  int vnum, rnum;
  struct room_data *room;

  skip_spaces(&argument);

  if (!is_number(argument)) {
    send_to_char("You have to supply a room number.\r\n", ch);
    return;
  }

  vnum = atoi(argument);

  if (vnum < 0 || vnum > 99998) {
    send_to_char("Sorry, vnum out of bounds.\r\n", ch);
    return;
  }

  CREATE(room, struct room_data, 1);

  if ((rnum = real_room(vnum)) > -1) {
    room_cpy(room, world[rnum]);
  } else {
    room_cpy(room, world[1]);
    room->number = vnum;
  }

  room->zone = real_zone(vnum/100);

  OCS1(ch) = (void*)room;

  /* Prepare ch for OCS */

  if (FIGHTING(ch))
    stop_fighting(ch);

  OCSMODE(ch) = OCS_ROOM_PRINT;
  ocs_main(ch, "");
}


int dir_check(struct room_direction_data *dir)
{
    return 1;
}

  
ACMD(do_eedit)
{
  int dir;
  struct room_direction_data *tmp;

  skip_spaces(&argument);
  
  dir = search_block(argument, dirs, FALSE);
  
  if (dir < 0 || dir > NUM_OF_DIRS - 1) {
    send_to_char("You have to supply a direction.\r\n", ch);
    return;
  }
  
  CREATE(tmp, struct room_direction_data, 1);

  OCS2(ch) = &(world[IN_ROOM(ch)]->dir_option[dir]);

  dir_cpy(tmp, world[IN_ROOM(ch)]->dir_option[dir]);

  OCS1(ch) = (void*)tmp;
  
  if (FIGHTING(ch))
    stop_fighting(ch);
  
  OCSMODE(ch) = OCS_EXIT_PRINT;
  ocs_main(ch, "");
}



ACMD(do_oedit)
{
  struct obj_data *obj, *tmp;

  skip_spaces(&argument);
  
  if (!*argument) {
    send_to_char("OEdit what object?\r\n", ch);
    return;
  }

  if (!(obj = get_obj_in_list_vis(ch, argument, ch->carrying))) {
    if (!(obj = get_obj_in_list_vis(ch, argument, world[ch->in_room]->contents)))
      {
	send_to_char("No such item around.\r\n", ch);
	return;
      } else {
	obj_from_room(obj);
	obj_to_char(obj, ch);
      }
  }

  if (obj->contains) {
    send_to_char("You can't OEdit objects with contents.\r\n", ch);
    return;
  }

  /* Make Temp Object */
  CREATE(tmp, struct obj_data, 1);
  clear_object(tmp);
  *tmp = *obj;
  
  if (obj->name)
    tmp->name = strdup(obj->name);
  if (obj->description)
    tmp->description = strdup(obj->description);
  if (obj->short_description)
    tmp->short_description = strdup(obj->short_description);
  if (obj->action_description)
    tmp->action_description = strdup(obj->action_description);

  tmp->in_room = -1;
  tmp->in_obj = NULL;
  tmp->contains = NULL;
  tmp->next_content = NULL;
  tmp->next = NULL;
  tmp->carried_by = NULL;

  if (GET_LEVEL(ch) < LEVEL_LESSER) { /* NO_RENT TAG ETC */
    SET_BIT(tmp->obj_flags.extra_flags, ITEM_NORENT);
    SET_BIT(tmp->obj_flags.extra_flags, ITEM_NOLOCATE);
    SET_BIT(tmp->obj_flags.extra_flags, ITEM_NOSWEEP);
  }

  OCS1(ch) = (void*)tmp;  /*  Temp Object in OCS1     */
  OCS2(ch) = (void*)obj;  /*  Original Object in OCS2 */
 
  if (FIGHTING(ch))
    stop_fighting(ch);
  
  OCSMODE(ch) = OCS_OBJ_PRINT;
  ocs_main(ch, "");
}

/*  Saves and Tags object with the name of the immortal that edited it   */

void ocs_tag_object(struct char_data *ch)
{
  int found = 0;

  /*  Remove the Original Object and Add New Object  */
  extract_obj((struct obj_data*)OCS2(ch));
  obj_to_char((struct obj_data*)OCS1(ch), ch);
  ((struct obj_data*)OCS1(ch))->next = object_list;
  object_list = ((struct obj_data*)OCS1(ch));
  obj_index[((struct obj_data*)OCS1(ch))->item_number].number++;
  

  /*    Tag Object    */

  strcpy(buf, ((struct obj_data*)OCS1(ch))->name);
  if (strstr(buf, GET_NAME(ch))) found = 1;

  if (!found) {
    sprintf(buf, "%s _%s_", ((struct obj_data*)OCS1(ch))->name, GET_NAME(ch));
    
    if (((struct obj_data*)OCS1(ch))->name && ((struct obj_data*)OCS1(ch))->name == obj_proto[((struct obj_data*)OCS1(ch))->item_number].name)
      ((struct obj_data*)OCS1(ch))->name = NULL;
    change_string(&((struct obj_data*)OCS1(ch))->name, buf);
  }

  sprintf(buf, "(GC) %s oedit %s saving [%d] %s", GET_NAME(ch), 
	  IS_SET((((struct obj_data*)OCS1(ch))->obj_flags.extra_flags), ITEM_NORENT) ? "NORENT" : "RENT", 
	  (((struct obj_data*)OCS1(ch))->item_number >= 0) ? obj_index[((struct obj_data*)OCS1(ch))->item_number].virtual : 0, 
	  ((struct obj_data*)OCS1(ch))->short_description);
  mudlog(buf, NRM, MIN(GET_LEVEL(ch), LEVEL_GREATER), TRUE); 
  
  return;
}

void ocs_tag_mob(struct char_data *ch)
{
     int found = 0;

     strcpy(buf, ((struct char_data*)OCS1(ch))->player.name);
     if (strstr(buf, GET_NAME(ch))) found = 1;
     
     if (!found) {
	  sprintf(buf, "%s _%s_", ((struct char_data*)OCS1(ch))->player.name, GET_NAME(ch));
	  if (((struct char_data*)OCS1(ch))->player.name && ((struct char_data*)OCS1(ch))->player.name == mob_proto[((struct char_data*)OCS1(ch))->nr].player.name)
	       ((struct char_data*)OCS1(ch))->player.name = NULL;
	  change_string(&((struct char_data*)OCS1(ch))->player.name, buf);
     }
     
     return;
}


ACMD(do_medit)
{
  struct char_data *mob;
  
  skip_spaces(&argument);
  
  if (!*argument) {
    send_to_char("MEdit what mob?\r\n", ch);
    return;
  }

  if (!(mob = get_char_room_vis(ch, argument))) {
    send_to_char("No such mob here.\r\n", ch);
    return;
  }
  
  if (!IS_NPC(mob)) {
    send_to_char("Don't touch the players please!\r\n", ch);
    return;
  }

  if (mob->desc) {
    send_to_char("Mixing switch and medit is a bad idea...\r\n", ch);
    return;
  }

  mob->specials.was_in_room = mob->in_room; /* this is a patch for a crash bug */
  char_from_room(mob);

  OCS1(ch) = (void*)mob;
  
  if (FIGHTING(ch))
    stop_fighting(ch);
  
  OCSMODE(ch) = OCS_MOB_PRINT;
  ocs_main(ch, "");
}


/* ****************************************************************************
 * OCS MAIN 
 *************************************************************************** */


void ocs_main(struct char_data *ch, char *arg)
{
  int i, type;
  struct extra_descr_data *ex_desc_ptr = 0; 
  
  switch(OCSMODE(ch)) {
  case OCS_ROOM_PRINT:
    print_room(ch, (struct room_data*)OCS1(ch));
    OCSMODE(ch) = OCS_ROOM_MAIN;
    break;
    
  case OCS_ROOM_MAIN:
    if (!is_number(arg)) {
      send_to_char("Change what > ", ch);
      OCSMODE(ch) = OCS_ROOM_PRINT;
      return;
    }
    
    i = atoi(arg);
    
    switch(i) {
    case 0:
      insert_room((struct room_data*)OCS1(ch));
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;
    case 1:
      send_to_char("§rChange Room-Name to > §N ", ch);
      OCSMODE(ch) = OCS_ROOM_GET_NAME;
      break;
    case 2:
      print_sector_type(ch);
      OCSMODE(ch) = OCS_ROOM_GET_SECT;
      break;
    case 3:
      print_room_flags(ch, (struct room_data*)OCS1(ch));
      OCSMODE(ch) = OCS_ROOM_GET_FLAGS;
      break;
    case 4:
      send_to_char("§rChange Room-Desc: (/s save or /h for help) > §N\r\n", ch);
      if (((struct room_data*)OCS1(ch))->description) {
	send_to_char("Current Room Desc:\r\n", ch);
	send_to_char(((struct room_data*)OCS1(ch))->description, ch);
	ch->desc->backstr = strdup(((struct room_data*)OCS1(ch))->description);
      }
      ch->desc->str = &(((struct room_data*)OCS1(ch))->description);
      ch->desc->max_str = 1024;
      OCSMODE(ch) = OCS_ROOM_GET_DESC;
      break;
    case 5:
      print_extra_desc(ch, &(((struct room_data*)OCS1(ch))->ex_description));
      OCSMODE(ch) = OCS_ROOM_GET_EMAIN;
      break;
    case 99:
      free_room((struct room_data*)OCS1(ch));
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;
    default:
      break;
    }
    break;
  case OCS_ROOM_GET_NAME:
    skip_spaces(&arg);
    change_string(&((struct room_data*)OCS1(ch))->name, arg);
    print_room(ch, (struct room_data*)OCS1(ch));
    OCSMODE(ch) = OCS_ROOM_MAIN;
    break;
  case OCS_ROOM_GET_SECT:
    i = atoi(arg);
    get_sector_type(ch, (struct room_data*)OCS1(ch), i);
    print_room(ch, (struct room_data*)OCS1(ch));
    OCSMODE(ch) = OCS_ROOM_MAIN;
    break;
  case OCS_ROOM_GET_FLAGS:
    i = atoi(arg);
    if (i != 0) {
      get_room_flags(ch, (struct room_data*)OCS1(ch), i);
      print_room_flags(ch, (struct room_data*)OCS1(ch));
    } else {
      print_room(ch, (struct room_data*)OCS1(ch));
      OCSMODE(ch) = OCS_ROOM_MAIN;
    }
    break;
  case OCS_ROOM_GET_EMAIN :
    i = atoi(arg);
    switch(i) {
    case 1:
      send_to_char("§rChange keywords to what >§N ", ch);
      OCSMODE(ch) = OCS_ROOM_GET_KEYWRD;
      break;
    case 2:
      send_to_char("§rChange Extra-Desc: (/s saves or /h for help) > §N\r\n", ch);
      if (((struct room_data*)OCS1(ch))->ex_description->description) {
	send_to_char("Current Extra Desc:\r\n", ch);
	send_to_char(((struct room_data*)OCS1(ch))->ex_description->description, ch);
	ch->desc->backstr = strdup(((struct room_data*)OCS1(ch))->ex_description->description);
      }
      ch->desc->str = &(((struct room_data*)OCS1(ch))->ex_description->description);
      ch->desc->max_str = 1024;
      OCSMODE(ch) = OCS_ROOM_GET_EDESC;
      break;
    case 3:
      if (check_extra_descr(((struct room_data*)OCS1(ch))->ex_description)) {
	ex_desc_ptr = ((struct room_data*)OCS1(ch))->ex_description;
	make_new_extra(&((struct room_data*)OCS1(ch))->ex_description);
	((struct room_data*)OCS1(ch))->ex_description->next = ex_desc_ptr;
	print_extra_desc(ch, &((struct room_data*)OCS1(ch))->ex_description);
      }
      break;

    case 4:
      free_extra_descr(&((struct room_data*)OCS1(ch))->ex_description);
      print_room(ch, (struct room_data*)OCS1(ch));
      OCSMODE(ch) = OCS_ROOM_MAIN;
      break; 
    default:
      if (!check_extra_descr(((struct room_data*)OCS1(ch))->ex_description))
	free_extra_descr(&((struct room_data*)OCS1(ch))->ex_description);
      print_room(ch, (struct room_data*)OCS1(ch));
      OCSMODE(ch) = OCS_ROOM_MAIN;
      break; 
    }
    break;
  case OCS_ROOM_GET_KEYWRD:
    skip_spaces(&arg);
    change_string(&((struct room_data*)OCS1(ch))->ex_description->keyword, arg);
    print_extra_desc(ch, &(((struct room_data*)OCS1(ch))->ex_description));
    OCSMODE(ch) = OCS_ROOM_GET_EMAIN;
    break;
  case OCS_EXIT_PRINT:
    print_exit(ch, (struct room_direction_data*)OCS1(ch));
    OCSMODE(ch) = OCS_EXIT_MAIN;
    break;
	
  case OCS_EXIT_MAIN:
    if (!is_number(arg)) {
      send_to_char("Change what > ", ch);
      OCSMODE(ch) = OCS_EXIT_PRINT;
      return;
    }

    i = atoi(arg);

    switch(i) {
    case 0:
      if (!dir_check((struct room_direction_data*)OCS1(ch))) {
	send_to_char("Format not correct!\r\n", ch);
	OCSMODE(ch) = OCS_EXIT_PRINT;
      } else {
	free_dir_data(*((struct room_direction_data**)OCS2(ch)));
	*((struct room_direction_data**)OCS2(ch)) =
	  (struct room_direction_data*)OCS1(ch);
	OCS1(ch) = NULL;
	OCS2(ch) = NULL;
	OCSMODE(ch) = OCS_OFF;
      }
      break;
    case 1:
      send_to_char("§rChange General-Desc: (/s saves or /h for help) > §N\r\n", ch);
      if (((struct room_direction_data*)OCS1(ch))->general_description) {
	send_to_char("Current General Desc:\r\n", ch);
	send_to_char(((struct room_direction_data*)OCS1(ch))->general_description, ch);
	ch->desc->backstr = strdup(((struct room_direction_data*)OCS1(ch))->general_description);}
      ch->desc->str = &(((struct room_direction_data*)OCS1(ch))->general_description);
      ch->desc->max_str = 512;
      OCSMODE(ch) = OCS_EXIT_GET_DESC;
      break;
    case 2:
      send_to_char("§rChange keywords to > §N ", ch);
      OCSMODE(ch) = OCS_EXIT_GET_KEYWRD;
      break;
    case 3:
      print_exit_info(ch, (struct room_direction_data*)OCS1(ch));
      OCSMODE(ch) = OCS_EXIT_GET_INFO;
      break;
    case 4:
      send_to_char("§rChange key vnum to > §N ", ch);
      OCSMODE(ch) = OCS_EXIT_GET_KEY;
      break;

    case 5:
      send_to_char("§rChange to_room (vnum)> §N ", ch);
      OCSMODE(ch) = OCS_EXIT_GET_TO;
      break;
      
    case 99:
      free_dir_data((struct room_direction_data*)OCS1(ch));
      free_dir_data(*((struct room_direction_data**)OCS2(ch)));
      *((struct room_direction_data**)OCS2(ch)) = NULL;
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;
    }
    break;
  case OCS_EXIT_GET_KEYWRD:
    skip_spaces(&arg);
    change_string(&((struct room_direction_data*)OCS1(ch))->keyword, arg);
    print_exit(ch, (struct room_direction_data*)OCS1(ch));
    OCSMODE(ch) = OCS_EXIT_MAIN;
    break;

  case OCS_EXIT_GET_INFO:
    i = atoi(arg);
    if (i != 0) {
      get_exit_info(ch, (struct room_direction_data*)OCS1(ch), i);
      print_exit_info(ch, (struct room_direction_data*)OCS1(ch));
    } else {
      print_exit(ch, (struct room_direction_data*)OCS1(ch));
      OCSMODE(ch) = OCS_EXIT_MAIN;
    }
    break;
  case OCS_EXIT_GET_KEY:
    ((struct room_direction_data*)OCS1(ch))->key = atoi(arg);
    print_exit(ch, (struct room_direction_data*)OCS1(ch));
    OCSMODE(ch) = OCS_EXIT_MAIN;
    break;

  case OCS_EXIT_GET_TO:
    i = atoi(arg);
    ((struct room_direction_data*)OCS1(ch))->to_room = real_room(i);
    print_exit(ch, (struct room_direction_data*)OCS1(ch));
    OCSMODE(ch) = OCS_EXIT_MAIN;
    break;
    
  case OCS_OBJ_PRINT:
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;
    
  case OCS_OBJ_MAIN:
    if (!is_number(arg)) {
      send_to_char("Change what > ", ch);
      return;
    }
    
    i = atoi(arg);
    
    switch(i) {
    case 0:
      ocs_tag_object(ch); /* Tags AND saves object */
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;      
    case 2:
      send_to_char("Change namelist to:>", ch);
      OCSMODE(ch) = OCS_OBJ_GET_NAME;
      break;
    case 3:
      send_to_char("Change roomdesc to:>", ch);
      OCSMODE(ch) = OCS_OBJ_GET_RDESC;
      break;
    case 4:
      send_to_char("Change inventory desc to:>", ch);
      OCSMODE(ch) = OCS_OBJ_GET_IDESC;
      break;
    case 5:
      send_to_char("Change action desc to:>", ch);
      OCSMODE(ch) = OCS_OBJ_GET_ADESC;
      break;
    case 8:   /* Wear Flags */
      if (GET_LEVEL(ch) >= LEVEL_GREATER) {
         print_obj_flags(ch, (struct obj_data*)OCS1(ch), 8);
         OCSMODE(ch) = OCS_OBJ_GET_WEAR;
      } else 
         send_to_char("Sorry, not for your level", ch);
      break;
    case 9:   /* Extra Flags */
      if (GET_LEVEL(ch) >= LEVEL_GREATER) {
         print_obj_flags(ch, (struct obj_data*)OCS1(ch), 9);
         OCSMODE(ch) = OCS_OBJ_GET_XTRA;
      } else 
         send_to_char("Sorry, not for your level", ch);
      break;
    case 10:  /* Anti-class Flags */
      if (GET_LEVEL(ch) >= LEVEL_GREATER) {
         print_obj_flags(ch, (struct obj_data*)OCS1(ch),10);
         OCSMODE(ch) = OCS_OBJ_GET_ACLASS;
      } else 
         send_to_char("Sorry, not for your level", ch);
      break;
    case 11:  /* Affecting Flags */
      if (GET_LEVEL(ch) >= LEVEL_GREATER) {
      print_obj_flags(ch, (struct obj_data*)OCS1(ch),11);
      OCSMODE(ch) = OCS_OBJ_GET_AFF;
      } else 
         send_to_char("Sorry, not for your level", ch);
      break;
    case 20:  /* Object Value 0 */
      if (GET_ITEM_TYPE((struct obj_data*)OCS1(ch)) == ITEM_PORTAL || 
	  GET_LEVEL(ch) >= LEVEL_GREATER) {
         send_to_char("Change Value[0] to:>", ch);
         OCSMODE(ch) = OCS_OBJ_GET_V0;
      }
      break;
    case 21:  /* Object Value 1 */
      if (GET_ITEM_TYPE((struct obj_data*)OCS1(ch)) == ITEM_PORTAL || 
	  GET_LEVEL(ch) >= LEVEL_GREATER) {
         send_to_char("Change Value[1] to:>", ch);
         OCSMODE(ch) = OCS_OBJ_GET_V1;
      }
      break;
    case 22:  /* Object Value 2 */
      if (GET_ITEM_TYPE((struct obj_data*)OCS1(ch)) == ITEM_PORTAL || 
	  GET_LEVEL(ch) >= LEVEL_GREATER) {
         send_to_char("Change Value[2] to:>", ch);
         OCSMODE(ch) = OCS_OBJ_GET_V2;
      }
      break;
    case 23:  /* Object Value 3 */
      if (GET_ITEM_TYPE((struct obj_data*)OCS1(ch)) == ITEM_PORTAL || 
	  GET_LEVEL(ch) >= LEVEL_GREATER) {
         send_to_char("Change Value[3] to:>", ch);
         OCSMODE(ch) = OCS_OBJ_GET_V3;
      }
      break;
    case 24:  /* Object Value 4 */
      if (GET_ITEM_TYPE((struct obj_data*)OCS1(ch)) == ITEM_PORTAL || 
	  GET_LEVEL(ch) >= LEVEL_GREATER) {
         send_to_char("Change Value[4] to:>", ch);
         OCSMODE(ch) = OCS_OBJ_GET_V4;
      }
      break;
    case 25:  /* Object Value 5 */
      if (GET_ITEM_TYPE((struct obj_data*)OCS1(ch)) == ITEM_PORTAL || 
	  GET_LEVEL(ch) >= LEVEL_GREATER) {
         send_to_char("Change Value[5] to:>", ch);
         OCSMODE(ch) = OCS_OBJ_GET_V5;
      }
      break;
    case 50:
      if (GET_LEVEL(ch) >= LEVEL_LESSER) /* NO_RENT TOGGLE */
	TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.extra_flags, ITEM_NORENT);
      print_object(ch, (struct obj_data*)OCS1(ch));
      break;
    case 51:
      if (GET_LEVEL(ch) >= LEVEL_LESSER) /* NO_SWEEP TOGGLE */
        TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.extra_flags, ITEM_NOSWEEP);
      print_object(ch, (struct obj_data*)OCS1(ch));
      break;
    case 52:
      if (GET_LEVEL(ch) >= LEVEL_LESSER) /* NO_LOCATE TOGGLE */
        TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.extra_flags, ITEM_NOLOCATE);
      print_object(ch, (struct obj_data*)OCS1(ch));
      break;
    case 99:
      free_obj((struct obj_data*)OCS1(ch));
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;
    default:
      send_to_char("Sorry, option not implemented yet.Change what? >\r\n", ch);
      break;
    }
    break;

    
  case OCS_OBJ_GET_NAME:
    skip_spaces(&arg);
    if (((struct obj_data*)OCS1(ch))->name && ((struct obj_data*)OCS1(ch))->name == obj_proto[((struct obj_data*)OCS1(ch))->item_number].name)
      ((struct obj_data*)OCS1(ch))->name = NULL;
    change_string(&((struct obj_data*)OCS1(ch))->name, arg);
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_RDESC:
    skip_spaces(&arg);
    if (((struct obj_data*)OCS1(ch))->description && ((struct obj_data*)OCS1(ch))->description == obj_proto[((struct obj_data*)OCS1(ch))->item_number].description)
      ((struct obj_data*)OCS1(ch))->description = NULL;
    change_string(&((struct obj_data*)OCS1(ch))->description, arg);
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_IDESC:
    skip_spaces(&arg);
    if (((struct obj_data*)OCS1(ch))->short_description && ((struct obj_data*)OCS1(ch))->short_description == obj_proto[((struct obj_data*)OCS1(ch))->item_number].short_description)
      ((struct obj_data*)OCS1(ch))->short_description = NULL;
    change_string(&((struct obj_data*)OCS1(ch))->short_description, arg);
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_ADESC:
    skip_spaces(&arg);
    if (((struct obj_data*)OCS1(ch))->action_description && ((struct obj_data*)OCS1(ch))->action_description == obj_proto[((struct obj_data*)OCS1(ch))->item_number].action_description)
      ((struct obj_data*)OCS1(ch))->action_description = NULL;
    change_string(&((struct obj_data*)OCS1(ch))->action_description, arg);
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_WEAR:
    type = atoi(arg);
    if (type != 0) {
      if (type < 1 || type > MAX_WEAR_FLAGS)
         send_to_char("§o§rINVALID CHOICE!!§N\r\n§rToggle what > §N", ch);
      else
         TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.wear_flags,(long)(1 << (type -1)));
      print_obj_flags(ch, (struct obj_data*)OCS1(ch), 8);
    } else {
      print_object(ch, (struct obj_data*)OCS1(ch));
      OCSMODE(ch) = OCS_OBJ_MAIN;
    }
    break;

  case OCS_OBJ_GET_XTRA:
    type = atoi(arg);
    if (type != 0) {
      if (type < 1 || type > MAX_EXTRA_FLAGS)
         send_to_char("§o§rINVALID CHOICE!!§N\r\n§rToggle what > §N", ch);
      else
         TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.extra_flags,(long)(1 << (type -1)));
      print_obj_flags(ch, (struct obj_data*)OCS1(ch), 9);
    } else {
      print_object(ch, (struct obj_data*)OCS1(ch));
      OCSMODE(ch) = OCS_OBJ_MAIN;
    }
    break;

  case OCS_OBJ_GET_ACLASS:
    type = atoi(arg);
    if (type != 0) {
      if (type < 1 || type > MAX_ACLASS_FLAGS)
         send_to_char("§o§rINVALID CHOICE!!§N\r\n§rToggle what > §N", ch);
      else
         TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.anticlass,(long)(1 << (type -1)));
      print_obj_flags(ch, (struct obj_data*)OCS1(ch), 10);
    } else {
      print_object(ch, (struct obj_data*)OCS1(ch));
      OCSMODE(ch) = OCS_OBJ_MAIN;
    }
    break;

  case OCS_OBJ_GET_AFF:
    type = atoi(arg);
    if (type != 0) {
      if (type < 1 || type > AFF_MAX)
         send_to_char("§o§rINVALID CHOICE!!§N\r\n§rToggle what > §N", ch);
      else
         TOGGLE_BIT(((struct obj_data*)OCS1(ch))->obj_flags.bitvector,(long)(1 << (type -1)));
      print_obj_flags(ch, (struct obj_data*)OCS1(ch), 11);
    } else {
      print_object(ch, (struct obj_data*)OCS1(ch));
      OCSMODE(ch) = OCS_OBJ_MAIN;
    }
    break;

  case OCS_OBJ_GET_V0:
    i = atoi(arg);
    ((struct obj_data*)OCS1(ch))->obj_flags.value[0] = i;
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;
 
 case OCS_OBJ_GET_V1:
    i = atoi(arg);
    ((struct obj_data*)OCS1(ch))->obj_flags.value[1] = i;
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_V2:
    i = atoi(arg);
    ((struct obj_data*)OCS1(ch))->obj_flags.value[2] = i;
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_V3:
    i = atoi(arg);
    ((struct obj_data*)OCS1(ch))->obj_flags.value[3] = i;
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_V4:
    i = atoi(arg);
    ((struct obj_data*)OCS1(ch))->obj_flags.value[4] = i;
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

  case OCS_OBJ_GET_V5:
    i = atoi(arg);
    ((struct obj_data*)OCS1(ch))->obj_flags.value[5] = i;
    print_object(ch, (struct obj_data*)OCS1(ch));
    OCSMODE(ch) = OCS_OBJ_MAIN;
    break;

    
  case OCS_MOB_PRINT:
    print_mob(ch, (struct char_data*)OCS1(ch));
    OCSMODE(ch) = OCS_MOB_MAIN;
    break;

  case OCS_MOB_MAIN:
    if (!is_number(arg)) {
      send_to_char("Change what > ", ch);
      return;
    }
    
    i = atoi(arg);
    
    switch(i) {
    case 0:
      ocs_tag_mob(ch);
      char_to_room((struct char_data*)OCS1(ch), ((struct char_data*)OCS1(ch))->specials.was_in_room);
      ((struct char_data*)OCS1(ch))->specials.was_in_room = NOWHERE;
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;      
    case 1:
      send_to_char("Change namelist to:>", ch);
      OCSMODE(ch) = OCS_MOB_GET_NLIST;
      break;
    case 2:
      send_to_char("Change name to:>", ch);
      OCSMODE(ch) = OCS_MOB_GET_NAME;
      break;
    case 3:
      send_to_char("Change room desc to:>", ch);
      OCSMODE(ch) = OCS_MOB_GET_RDESC;
      break;
    case 4:
      send_to_char("Change long desc to:>", ch);
      OCSMODE(ch) = OCS_MOB_GET_LDESC;
      break;
    case 99:
      char_to_room((struct char_data*)OCS1(ch), ((struct char_data*)OCS1(ch))->specials.was_in_room);
      ((struct char_data*)OCS1(ch))->specials.was_in_room = NOWHERE;
      OCS1(ch) = NULL;
      OCS2(ch) = NULL;
      OCSMODE(ch) = OCS_OFF;
      break;
    default:
      send_to_char("Sorry, option not implemented yet.Change what? >\r\n", ch);
      break;
    }
    break;
    
  case OCS_MOB_GET_NLIST:
    skip_spaces(&arg);
    if (((struct char_data*)OCS1(ch))->player.name && ((struct char_data*)OCS1(ch))->player.name == mob_proto[((struct char_data*)OCS1(ch))->nr].player.name)
      ((struct char_data*)OCS1(ch))->player.name = NULL;
    change_string(&((struct char_data*)OCS1(ch))->player.name, arg);
    print_mob(ch, (struct char_data*)OCS1(ch));
    OCSMODE(ch) = OCS_MOB_MAIN;
    break;
    
  case OCS_MOB_GET_NAME:
    skip_spaces(&arg);
    if (((struct char_data*)OCS1(ch))->player.short_descr && ((struct char_data*)OCS1(ch))->player.short_descr == mob_proto[((struct char_data*)OCS1(ch))->nr].player.short_descr)
      ((struct char_data*)OCS1(ch))->player.short_descr = NULL;
    change_string(&((struct char_data*)OCS1(ch))->player.short_descr, arg);
    print_mob(ch, (struct char_data*)OCS1(ch));
    OCSMODE(ch) = OCS_MOB_MAIN;
    break;

  case OCS_MOB_GET_RDESC:
    skip_spaces(&arg);
    strcat(arg, "\r\n");
    if (((struct char_data*)OCS1(ch))->player.long_descr && ((struct char_data*)OCS1(ch))->player.long_descr == mob_proto[((struct char_data*)OCS1(ch))->nr].player.long_descr)
      ((struct char_data*)OCS1(ch))->player.long_descr = NULL;
    change_string(&((struct char_data*)OCS1(ch))->player.long_descr, arg);
    print_mob(ch, (struct char_data*)OCS1(ch));
    OCSMODE(ch) = OCS_MOB_MAIN;
    break;

  case OCS_MOB_GET_LDESC:
    skip_spaces(&arg);
    strcat(arg, "\r\n");
    if (((struct char_data*)OCS1(ch))->player.description && ((struct char_data*)OCS1(ch))->player.description == mob_proto[((struct char_data*)OCS1(ch))->nr].player.description)
      ((struct char_data*)OCS1(ch))->player.description = NULL;
    change_string(&((struct char_data*)OCS1(ch))->player.description, arg);
    print_mob(ch, (struct char_data*)OCS1(ch));
    OCSMODE(ch) = OCS_MOB_MAIN;
    break;

  default:
    break;
  }
}





/* save the room */
void   save_room(FILE *fl, struct room_data *room)
{
  int i;
  struct extra_descr_data *tmpdesc;
  char flags[64];

  fprintf(fl, "#%d\n", room->number); /* VNUM */
  fwrite_string(fl, room->name); /* NAME */
  fwrite_string(fl, room->description); /* DESC */

  sprintflags(room->room_flags, flags);
  
  fprintf(fl, "%d %s %d\n",
	  (room->zone>-1?zone_table[room->zone].number:-1),
	  flags,
	  room->sector_type);
  
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (room->dir_option[i] && room->dir_option[i]->to_room > -1) {
      fprintf(fl, "D%d\n", i);
      fwrite_string(fl, room->dir_option[i]->general_description);
      fwrite_string(fl, (room->dir_option)[i]->keyword);
      fprintf(fl, "%d %d %d\n",
	      room->dir_option[i]->exit_info,
	      room->dir_option[i]->key,
	      world[room->dir_option[i]->to_room]->number);
    }
  }

  tmpdesc = room->ex_description;

  while(tmpdesc) {
    fprintf(fl, "E\n");
    fwrite_string(fl, tmpdesc->keyword);
    fwrite_string(fl, tmpdesc->description);
    tmpdesc = tmpdesc->next;
  }

  fprintf(fl, "S\n");
}



int ocs_save(int mode, int flag)
{
  char	*filename = NULL, file[128];
  FILE  *ocs_file;
  int	i;

  if (mini_mud)
    return 0;

  switch (mode) {
  case OCS_SAVE_ROOM : filename = OCS_ROOMFILE; break;
  case OCS_SAVE_OBJS : filename = OCS_OBJFILE ; break;
  case OCS_SAVE_MOBS : filename = OCS_MOBFILE ; break;
  case OCS_SAVE_ZONE : filename = OCS_ZONEFILE; break;
  case OCS_SAVE_SHOP : filename = OCS_SHOPFILE; break;
  case OCS_PSAVE_ROOM:
    sprintf(file, "world/wld/%d.wld", zone_table[flag].number);
    filename = file;
    break;
  default:
    log("SYSERR: Unknown subcommand to ocs_save!");
    break;
  }

  if (!(ocs_file = fopen(filename, "w"))) {
    log("SYSERR: Unable to open ocs file for writing");
    return 0;
  }
  
  switch (mode) {
  case OCS_SAVE_ROOM:
    for (i = 0; i < top_of_world; i++) {
      if (ROOM_FLAGGED(i, OCS))
	save_room(ocs_file, world[i]);
    }
    break;
  case OCS_PSAVE_ROOM:
    for (i = zone_table[flag].lowest; i <= zone_table[flag].highest; i++) {
      if (ROOM_FLAGGED(i, OCS)) {
	REMOVE_BIT(world[i]->room_flags, OCS);
	ocs_rooms--;	  
      }
      save_room(ocs_file, world[i]);
    }
    break;  
  }
  
  fprintf(ocs_file, "#99999\n$~\n");

  fclose(ocs_file);

  return 1;
}


ACMD(do_ocsrsave)
{
  int zone = 0;

  if (!ch)
    return;

  skip_spaces(&argument);

  if (mini_mud) {
    send_to_char("Cannot use OCS from minimud.\r\n", ch);
    return;
  }

  if (isdigit(*argument)) {
    if ((zone = real_zone(atoi(argument))) < 0) {
      send_to_char("Not a valid zone.\r\n", ch);
      return;
    }
  
    if (!ocs_save(OCS_PSAVE_ROOM, zone)) {
      send_to_char("WARNING: OCS PERMANENT SAVE ERROR - PLEASE CHECK!\r\n", ch);
      return;
    }
  
  } 
  
  
  if (!ocs_save(OCS_SAVE_ROOM, 0)) {
    send_to_char("OCS save error.\r\n", ch);
    return;
  } else {
    send_to_char("Ok.\r\n", ch);
    return;
  }
}

  

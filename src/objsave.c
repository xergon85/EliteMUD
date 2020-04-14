/* ************************************************************************
*   File: objsave.c                                     Part of EliteMUD  *
*  Usage: loading/saving player objects for rent and crash-save           *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"
#include "objsave.h"
#include "fparser.h"
#include "functions.h"

/* these factors should be unique integers */
#define RENT_FACTOR 	1
#define CRYO_FACTOR 	4

extern struct str_app_type str_app[];
extern struct room_data **world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern struct descriptor_data *descriptor_list;
extern struct room_list *room_crash_list;
extern int	top_of_p_table;
extern int	min_rent_cost;
extern char  *where[];
extern struct player_index_element *player_table;
/* Extern functions */
ACMD(do_tell);
/*
SPECIAL(receptionist);
SPECIAL(cryogenicist);
*/

/* Internal funcs */
int	Crash_is_junk(struct obj_data *obj);
FILE *fd;


int
get_filename(char *orig_name, char *filename, int mode)
{
  char	*ptr, dir[32], name[32], ext[32];

    if (!*orig_name)
      return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = tolower(*ptr);

  switch (mode) {
  case OBJECT_FILE: strcpy(dir, "plrobjs"); strcpy(ext, "objs"); break;
  case ALIAS_FILE: strcpy(dir, "plraliases"); strcpy(ext, "als"); break;
  case ROOMOBJ_FILE: strcpy(dir, "roomobjs"); strcpy(ext, "objs"); break;
  case STRING_FILE: strcpy(dir, "plrstrings"); strcpy(ext, "str"); break;
  case OBJECT_BACKUP: strcpy(dir, "plrobjsbak"); strcpy(ext, "objs"); break;
  case MAIL_FILE: strcpy(dir, "plrmail"); strcpy(ext, "mail"); break;
  case LOCKER_FILE: strcpy(dir, "lockers"); strcpy(ext, "lok"); break;
  case PLOG_FILE: strcpy(dir, "plrlogs"); strcpy(ext, "log"); break;
  case IGNORE_FILE: strcpy(dir, "plrignore"); strcpy(ext, "ign"); break;
  default:
    return 0;
  }

  switch (tolower(*name)) {
  case 'a': case 'b': case 'c': case 'd': case 'e':
    sprintf(filename, "%s/A-E/%s.%s", dir, name, ext); break;
  case 'f': case 'g': case 'h': case 'i': case 'j':
	sprintf(filename, "%s/F-J/%s.%s", dir, name, ext); break;
  case 'k': case 'l': case 'm': case 'n': case 'o':
    sprintf(filename, "%s/K-O/%s.%s", dir, name, ext); break;
  case 'p': case 'q': case 'r': case 's': case 't':
    sprintf(filename, "%s/P-T/%s.%s", dir, name, ext); break;
  case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    sprintf(filename, "%s/U-Z/%s.%s", dir, name, ext); break;
  case '1': case '2': case '3': case '4': case '5':
  case '6': case '7': case '8': case '9': case '0':
    sprintf(filename, "%s/room%s.%s", dir, name, ext); break;
  default:
	sprintf(filename, "%s/ZZZ/%s.%s", dir, name, ext); break;
  }

  return 1;
}


int
Crash_delete_file(char *name, int mode)
{
  char	filename[50];
  FILE * fl;

  if (!get_filename(name, filename, mode))
    return 0;

  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) { /* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
      perror(buf1);
    }
    return 0;
   }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) { /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
      perror(buf1);
    }
  }

  return(1);
}


int
Crash_delete_crashfile(struct char_data *ch)
{
  char	fname[MAX_INPUT_LENGTH];
  FILE * fl;

  if (!get_filename(GET_NAME(ch), fname, OBJECT_FILE))
    return 0;
  if (!(fl = fopen(fname, "rb"))) {
    if (errno != ENOENT) { /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: checking for crash file %s (3)", fname);
      perror(buf1);
    }
    return 0;
  }

  /*   if (!feof(fl))
       fread(&rent, sizeof(struct rent_info), 1, fl); */
  fclose(fl);

  /*   if (rent.rentcode == RENT_CRASH)
       Crash_delete_file(GET_NAME(ch), MAIN_FILE); */

  return 1;
}


int
Crash_clean_file(char *name)
{
  return(0);
}


void
Crash_restore_file(char *name)
{
  char  fname[MAX_INPUT_LENGTH], backup[MAX_INPUT_LENGTH];
  FILE  *new, *bak;
  struct obj_file_elem object;

  if (!get_filename(name, fname, OBJECT_FILE))
    return;
  if (!get_filename(name, backup, OBJECT_BACKUP))
    return;

  if (!(bak = fopen(backup, "r+b")))
    return;

  if (!(new = fopen(fname, "wb")))
    return;

  while (!feof(bak)) {
    fread(&object, sizeof(struct obj_file_elem), 1, bak);
    if (ferror(bak) ||
	fwrite(&object, sizeof(struct obj_file_elem), 1, new) < 1) {
      fclose(bak);
      fclose(new);
      Crash_delete_file(name, OBJECT_BACKUP);
      return;
    }
  }
  fclose(bak);
  fclose(new);
  Crash_delete_file(name, OBJECT_BACKUP);
}

void
update_obj_file(void)
{
  int i;

  for (i = 0; i < top_of_p_table; i++)
    Crash_restore_file((player_table + i)->name);

  return;
}


static int
object_load(struct obj_data *obj, int * pos, FILE *fp)
{
  int i, aff_pos = 0;

  while (1) {
    if ((i = fparse_token(fp)) != F_STRTOKEN)
      return i;

    if (!obj)
      return i;

    switch (*(fp_val.val.string)) {

    case 'P':
      if (!strcmp(fp_val.val.string, "POS")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("POS");
	*pos = fp_val.val.number;
      } else
	return i;
      break;

    case 'N':
      if (!strcmp(fp_val.val.string, "NAME")) {
	if (fparse_token(fp) != F_STRING)
	  return fparse_error("NAME");
	obj->name = strdup(fp_val.val.string);
      } else
	return i;
      break;

    case 'D':
      if (!strcmp(fp_val.val.string, "DESC")) {
	if (fparse_token(fp) != F_STRING)
	  return fparse_error("DESC");
	obj->description = strdup(fp_val.val.string);
      } else
	return i;
      break;

    case 'S':
      if (!strcmp(fp_val.val.string, "SDESC")) {
	if (fparse_token(fp) != F_STRING)
	  return fparse_error("SDESC");
	obj->short_description = strdup(fp_val.val.string);
      } else
	return i;
      break;

    case 'V':
      if (!strcmp(fp_val.val.string, "V0")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("V0");
	obj->obj_flags.value[0] = fp_val.val.number;
      } else if (!strcmp(fp_val.val.string, "V1")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("V1");
	obj->obj_flags.value[1] = fp_val.val.number;
      } else if (!strcmp(fp_val.val.string, "V2")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("V2");
	obj->obj_flags.value[2] = fp_val.val.number;
      } else if (!strcmp(fp_val.val.string, "V3")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("V3");
	obj->obj_flags.value[3] = fp_val.val.number;
      } else if (!strcmp(fp_val.val.string, "V4")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("V4");
	obj->obj_flags.value[4] = fp_val.val.number;
      } else if (!strcmp(fp_val.val.string, "V5")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("V5");
	obj->obj_flags.value[5] = fp_val.val.number;
      } else
	return i;
      break;

    case 'E':
      if (!strcmp(fp_val.val.string, "EFLGS")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("EFLGS");
	obj->obj_flags.extra_flags = fp_val.val.number;
      } else
	return i;
      break;

    case 'W':
      if (!strcmp(fp_val.val.string, "WEIGHT")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("WEIGHT");
	obj->obj_flags.weight = fp_val.val.number;
      } else
	return i;
      break;

    case 'T':
      if (!strcmp(fp_val.val.string, "TIMER")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("TIMER");
	obj->obj_flags.timer = fp_val.val.number;
      } else
	return i;
      break;

    case 'B':
      if (!strcmp(fp_val.val.string, "BVEC")) {
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("BVEC");
	obj->obj_flags.bitvector = fp_val.val.number;
      } else
	return i;
      break;

    case 'A':
      if (!strcmp(fp_val.val.string, "AFF")) {
        if (aff_pos >= 4) /* Bod - To trap merged objs with too many AFFs */
          return fparse_error("AFF1 - aff_pos >= 4");
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("AFF1");
	obj->affected[aff_pos].location = fp_val.val.number;
	if (fparse_token(fp) != F_NUMBER)
	  return fparse_error("AFF2");
        obj->affected[aff_pos].modifier = fp_val.val.number;
        ++aff_pos;
      } else
	return i;
      break;

    default:
      return i;
    }
  }
}


void
Crash_clear_weight(struct obj_data *obj)
{
   if (obj) {
      if (obj->in_obj)
	 GET_OBJ_WEIGHT(obj->in_obj) -= GET_OBJ_WEIGHT(obj);
      Crash_clear_weight(obj->next_content);
      Crash_clear_weight(obj->contains);
   }
}

void
Crash_restore_weight(struct obj_data *obj)
{
   if (obj) {
      Crash_restore_weight(obj->contains);
      Crash_restore_weight(obj->next_content);
      if (obj->in_obj)
	 GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
   }
}

/* TEMPORARY CONVERTION SOLUTION -P */
static void
temp_Crash_listrent(struct char_data *ch, char *name)
{
  FILE * fl;
  char	fname[MAX_INPUT_LENGTH], buf[LARGE_BUFSIZE];
  struct obj_file_elem object;
  struct obj_data *obj;
  int pos;

  if (!get_filename(name, fname, OBJECT_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no rent file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  sprintf(buf, "%s\r\n", fname);

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      return;
    }
    if (!feof(fl))
      if (real_object(object.item_number) > -1) {
	    obj = read_object(object.item_number, VIRTUAL);

	    pos = MIN(IN_MAXDEPTH, object.pos);

	    sprintf(buf, "%s [%5d] %s (%9dg) %-20s\r\n", buf,
		    object.item_number, where[pos], obj->obj_flags.cost,
		    obj->short_description);
	    extract_obj(obj);
	  }
  }
  page_string(ch->desc, buf, TRUE);
  fclose(fl);
}

void
Crash_listrent(struct char_data *ch, char *name)
{
  FILE * fl;
  char	fname[MAX_INPUT_LENGTH], buf[LARGE_BUFSIZE];
  struct obj_data *obj;
  int pos, i, vnum;

  if (!get_filename(name, fname, OBJECT_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no rent file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  sprintf(buf, "%s\r\n", fname);

  i = fparse_token(fl);

  if (i != '#') {   /* Old objfile */
    fclose(fl);
    temp_Crash_listrent(ch, name);
    return;
  }

  while (i == '#') {

    pos = IN_INVENTORY;

    if (fparse_token(fl) != F_NUMBER)
      break;

    if (real_object((vnum = fp_val.val.number)) > -1) {

      obj = read_object(vnum, VIRTUAL);

      if ((i = object_load(obj, &pos, fl)) == F_ERROR)
	break;

      pos = MIN(IN_MAXDEPTH, pos);

      sprintf(buf, "%s [%5d] %s (%9dg) %-20s\r\n", buf,
	      vnum, where[pos], obj->obj_flags.cost,
	      obj->short_description);
      extract_obj(obj);
    } else {
      obj = read_object(0, VIRTUAL);

      if ((i = object_load(obj, &pos, fl)) == F_ERROR)
	break;

      extract_obj(obj);
    }
  }
  page_string(ch->desc, buf, TRUE);
  fclose(fl);
}


int
Crash_write_rentcode(struct char_data *ch, FILE *fl, struct rent_info *rent)
{
  return 1;
}


int	Crash_load(struct char_data *ch)
     /* return values:
	0 - successful load, keep char in rent room.
	1 - load failure or load of crash items -- put char in temple.
	2 - rented equipment lost (no $)
	*/
{
  FILE * fl;
  char	fname[MAX_STRING_LENGTH];
  struct obj_data *last_obj, *tmp, *objs[IN_MAXDEPTH - IN_INVENTORY + 1];
  int	i, num = 0, last_pos = IN_INVENTORY, pos = IN_INVENTORY;
  bool cont = FALSE;
  extern int max_obj_save;

  /*  int custom = 0;   Counter var for custom eq */

  if (!get_filename(GET_NAME(ch), fname, OBJECT_FILE))
    return 1;

  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
      perror(buf1);
      send_to_char("\r\n********************* NOTICE *********************\r\n"
		   "There was a problem loading your objects from disk.\r\n"
		   "Contact a God for assistance.\r\n", ch);
    }

    sprintf(buf, "%s entering game with no objs.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_LEVEL(ch)), TRUE);
    return 1;
  }


  for (i = IN_INVENTORY;i <= IN_MAXDEPTH;i++)
    objs[i - IN_INVENTORY] = 0;
  last_obj = 0;
  num = 0;

  i = fparse_token(fl);

  while (i == '#') {

    pos = IN_INVENTORY;

    if (fparse_token(fl) != F_NUMBER)
      break;

    if (real_object(fp_val.val.number) > -1) {

      tmp = read_object(fp_val.val.number, VIRTUAL);

      if ((i = object_load(tmp, &pos, fl)) == F_ERROR)
	break;

      if (tmp) {
	num++;
	tmp->carried_by = 0;
	tmp->in_obj = 0;
	tmp->in_room = NOWHERE;
	tmp->contains = 0;
	tmp->next_content = 0;

	assert(pos >= 0 && pos <= IN_MAXDEPTH);

	if (pos >= 0 && pos < IN_INVENTORY) {
	     equip_char(ch, tmp, pos);
	     cont = TRUE;
	} else if (pos <= IN_MAXDEPTH) {
	  if (pos == IN_INVENTORY) {
	    obj_to_char(tmp, ch);
	    cont = TRUE;
	  } else if (pos == last_pos && last_obj && cont) {
	       obj_to_obj(tmp, last_obj->in_obj);
	  } else if (pos > last_pos && last_obj && cont) {
	       obj_to_obj(tmp, last_obj);
	       if (last_pos >= IN_INVENTORY)
		    objs[last_pos - IN_INVENTORY] = last_obj;
	  } else {
	       if (objs[pos - IN_INVENTORY] && cont)
		    obj_to_obj(tmp, objs[pos - IN_INVENTORY]->in_obj);
	       else {
		    obj_to_char(tmp, ch);
	       }
	  }
	}

	last_pos = pos;
	last_obj = tmp;

	/*	if ((tmp->short_description) != (obj_proto[tmp->item_number].short_description)) custom++;*/
      }

      if (num >= max_obj_save) {
	send_to_char("You couldn't keep track of all your equipment\r\nSome has been lost.  Try to keep less in your inventory.\r\n", ch);
	break;
      }
    } else {
      tmp = read_object(0, VIRTUAL);

      if ((i = object_load(tmp, &pos, fl)) == F_ERROR)
	break;

      cont = FALSE;

      extract_obj(tmp);
    }
  }
  fclose(fl);


  if (num) {
    sprintf(buf, "%s loading (%d objs).", GET_NAME(ch), num);
    mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_LEVEL(ch)), TRUE);
    sprintf(buf, "§1g%d items recovered.%s#N\r\n", num, (num > 80 ? " §1rBetter clean up your inventory, or equipment may be lost." : ""));
    send_to_char(buf, ch);
    return 0;
  } else {
    sprintf(buf, "%s entering game with no objs.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_LEVEL(ch)), TRUE);
    return 1;
  }
  return 0;
}

/* New obj-save handler -Petrus */
int
storesave_obj(struct obj_data *obj, int pos, FILE *fp)
{
  int	j;
  char  str[60];

  if (obj) {
    fparse_putchar(fp, '#');
    sprintf(str,"%.5d  ",obj_index[obj->item_number].virtual);
    fparse_write_strtoken(fp,str);
/*    fparse_write_number(fp, obj_index[obj->item_number].virtual); */

    if (pos >= 0 && pos <= IN_MAXDEPTH && pos != IN_INVENTORY) {
      fparse_write_strtoken(fp, "POS ");
      fparse_write_number(fp, pos);
    }

#define OBJ_CHECK(data)  ((obj->data) == (obj_proto[obj->item_number].data))

    if (!OBJ_CHECK(name)) {
      fparse_write_strtoken(fp, "NAME ");
      fparse_write_string(fp, obj->name);
    }
    if (!OBJ_CHECK(description)) {
      fparse_write_strtoken(fp, "DESC ");
      fparse_write_string(fp, obj->description);
    }
    if (!OBJ_CHECK(short_description)) {
      fparse_write_strtoken(fp, "SDESC ");
      fparse_write_string(fp, obj->short_description);
    }
    for (j = 0; j < 6; ++j)
      if (!OBJ_CHECK(obj_flags.value[j])) {
	fparse_putchar(fp, 'V');
	fparse_putchar(fp, '0' + j);
	fparse_putchar(fp, ' ');
	fparse_write_number(fp, obj->obj_flags.value[j]);
      }

    if (!OBJ_CHECK(obj_flags.extra_flags)) {
      fparse_write_strtoken(fp, "EFLGS ");
      fparse_write_numberl(fp, obj->obj_flags.extra_flags);
    }
    if (!OBJ_CHECK(obj_flags.weight)) {
      fparse_write_strtoken(fp, "WEIGHT ");
      fparse_write_number(fp, obj->obj_flags.weight);
    }
    if (!OBJ_CHECK(obj_flags.timer)) {
      fparse_write_strtoken(fp, "TIMER ");
      fparse_write_number(fp, obj->obj_flags.timer);
    }
    if (!OBJ_CHECK(obj_flags.bitvector)) {
       fparse_write_strtoken(fp, "BVEC ");
       fparse_write_numberl(fp, obj->obj_flags.bitvector);
     }

    for (j = 0; j < MAX_OBJ_AFFECT; ++j)
      if (obj->affected[j].location) {
	fparse_write_strtoken(fp, "AFF ");
	fparse_write_number(fp, obj->affected[j].location);
	fparse_write_number(fp, obj->affected[j].modifier);
       } else break;

    fparse_putchar(fp, '\n');
  }
  return 1;
}

int	Crash_obj2store(struct obj_data *obj, struct char_data *ch, FILE *fl)
{
   int	j;
   struct obj_file_elem object;

   object.pos = IN_INVENTORY;
   object.item_number = obj_index[obj->item_number].virtual;
   object.value[0] = obj->obj_flags.value[0];
   object.value[1] = obj->obj_flags.value[1];
   object.value[2] = obj->obj_flags.value[2];
   object.value[3] = obj->obj_flags.value[3];
   object.value[4] = obj->obj_flags.value[4];
   object.value[5] = obj->obj_flags.value[5];

   object.extra_flags = obj->obj_flags.extra_flags;
   object.weight = obj->obj_flags.weight;
   object.timer = obj->obj_flags.timer;
   object.bitvector = obj->obj_flags.bitvector;
   for (j = 0; j < MAX_OBJ_AFFECT; j++)
       object.affected[j] = obj->affected[j];

   if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
       perror("Writing crash data Crash_obj2store");
	   return 0;
   }

   return 1;
}

/*  New obj-save handler  -Petrus */
int save_obj(struct obj_data *obj, int pos, FILE *fl)
{
    if (obj &&
	((pos < IN_MAXDEPTH) || (pos == IN_MAXDEPTH && !obj->contains)))
    {
	if (save_obj(obj->next_content, pos, fl) < 1)
	    return 0;

	if (obj->item_number >= 0) {
	   if (storesave_obj(obj, pos, fl) < 1)
	       return 0;

	   if (pos < IN_INVENTORY)
	       pos = IN_INVENTORY;

	   if (save_obj(obj->contains, pos + 1, fl) < 1)
	       return 0;
        }
    }

    return 1;
}


/* New obj-save handler  -Petrus */
int   save_char_objs(struct char_data *ch, FILE *fl)
{
    int i;

    for (i = 0;i < MAX_WEAR;i++)
	if (ch->equipment[i]) {
	    Crash_clear_weight(ch->equipment[i]);
	    if (save_obj(ch->equipment[i], i, fl) < 1)
		return 0;
	}

    Crash_clear_weight(ch->carrying);

    if (save_obj(ch->carrying, IN_INVENTORY, fl) < 1)
	return 0;

    return 1;
}

/*
int	Crash_save(struct obj_data *obj, struct char_data *ch, FILE *fp)
{
   struct obj_data *tmp;
   int	result;

   if (obj) {
      Crash_save(obj->contains, ch, fp);
      Crash_save(obj->next_content, ch, fp);
      result = Crash_obj2store(obj, ch, fp);
      if (!result) {
	 return 0;
      }

      for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
	 GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
   }
   return TRUE;
}
*/

void	Crash_extract_objs(struct obj_data *obj, int *num)
{
   if (obj) {
      Crash_extract_objs(obj->contains, num);
      Crash_extract_objs(obj->next_content, num);
      extract_obj(obj);
      ++(*num);
   }
}


int	Crash_is_unrentable(struct obj_data *obj)
{
   if (!obj)
      return 0;
   if (IS_SET(obj->obj_flags.extra_flags, ITEM_NORENT) ||
       obj->obj_flags.cost_per_day < 0 ||
       obj->item_number <= -1 ||
       GET_ITEM_TYPE(obj) == ITEM_KEY )
      return 1;
   return 0;
}


void	Crash_extract_norents(struct obj_data *obj)
{
   if (obj) {
      Crash_extract_norents(obj->contains);
      Crash_extract_norents(obj->next_content);
      if (Crash_is_unrentable(obj))
	 extract_obj(obj);
   }
}

int	Crash_is_junk(struct obj_data *obj)
{
    if (!obj)
	return 0;
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NORENT) ||
	obj->obj_flags.cost_per_day < 0  ||
	GET_ITEM_TYPE(obj) == ITEM_KEY   ||
	obj->item_number <= -1)

	return 1;

   return 0;
}


void	Crash_extract_junk(struct obj_data *obj, struct char_data *ch)
{
    if (obj) {
	Crash_extract_junk(obj->contains, ch);
	Crash_extract_junk(obj->next_content, ch);
	if (Crash_is_junk(obj)) {
	    if (obj->carried_by)
		obj_from_char(obj);
	    if (obj->in_obj)
		obj_from_obj(obj);
	    if (GET_ITEM_TYPE(obj) == ITEM_NOTE && isname("mail", obj->name))
	      extract_obj(obj);
	    else {
	      obj_to_room(obj, ch->in_room);
	      act("$p cannot be saved - dropped.", FALSE, ch, obj, 0, TO_CHAR);

	    }
	}
    }
}


void	Crash_extract_expensive(struct obj_data *obj)
{
   struct obj_data *tobj, *max;

   max = obj;
   for (tobj = obj; tobj; tobj = tobj->next_content)
      if (tobj->obj_flags.cost_per_day > max->obj_flags.cost_per_day)
	 max = tobj;
   extract_obj(max);
}



void	Crash_calculate_rent(struct obj_data *obj, int *cost)
{
   if (obj) {
      *cost += MAX(0, obj->obj_flags.cost_per_day >> 1);
      Crash_calculate_rent(obj->contains, cost);
      Crash_calculate_rent(obj->next_content, cost);
   }
}


void	Crash_crashsave(struct char_data *ch, int mode)
{
   char	buf[MAX_INPUT_LENGTH], backup[MAX_INPUT_LENGTH];
   int	j;
   FILE * fp;

   if (IS_NPC(ch))
      return;

   if (!get_filename(GET_NAME(ch), buf, OBJECT_FILE))
      return;

   if (!get_filename(GET_NAME(ch), backup, OBJECT_BACKUP))
     return;

   if (mode == CRASH_SAVE) {
       if ((fp = fopen(backup, "r"))) {
	   fclose(fp);
	   return;
       }

   } else if (mode == MANUAL_SAVE) {
       Crash_delete_file(GET_NAME(ch), OBJECT_BACKUP);

   } else if (mode == BACKUP_SAVE) {
     strcpy(buf, backup);
   }

   if (!(fp = fopen(buf, "wb")))
       return;

   if (!save_char_objs(ch, fp)) {
      fclose(fp);
      return;
   }
   fclose(fp);

   for (j = 0; j < MAX_WEAR; j++)
       if (ch->equipment[j])
	   Crash_restore_weight(ch->equipment[j]);

   Crash_restore_weight(ch->carrying);

   REMOVE_BIT(PLR_FLAGS(ch), PLR_SAVEOBJS);
}


/*
void	Crash_rentsave(struct char_data *ch, int cost)
{
   char	buf[MAX_INPUT_LENGTH];
   int	j;
   FILE * fp;

   if (IS_NPC(ch))
      return;

   if (!get_filename(GET_NAME(ch), buf, OBJECT_FILE))
      return;
   if (!(fp = fopen(buf, "wb")))
      return;

   for (j = 0; j < MAX_WEAR; j++)
      if (ch->equipment[j])
	 obj_to_char(unequip_char(ch, j), ch);

   Crash_extract_norents(ch->carrying);

   if (!Crash_save(ch->carrying, ch, fp)) {
      fclose(fp);
      return;
   }
   fclose(fp);

   Crash_extract_objs(ch->carrying);
}
*/

/*
void	Crash_cryosave(struct char_data *ch, int cost)
{
   char	buf[MAX_INPUT_LENGTH];
   int	j;
   FILE * fp;

   if (IS_NPC(ch))
      return;

   if (!get_filename(GET_NAME(ch), buf, OBJECT_FILE))
      return;
   if (!(fp = fopen(buf, "wb")))
      return;

   for (j = 0; j < MAX_WEAR; j++)
      if (ch->equipment[j])
	 obj_to_char(unequip_char(ch, j), ch);

   Crash_extract_norents(ch->carrying);

   GET_GOLD(ch) = MAX(0, GET_GOLD(ch) - cost);

   if (!Crash_save(ch->carrying, ch, fp)) {
      fclose(fp);
      return;
   }
   fclose(fp);

   Crash_extract_objs(ch->carrying);
   SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
}
*/

void	Crash_quitsave(struct char_data *ch)
{
   char	buf[MAX_INPUT_LENGTH];
   int	j;
   FILE * fp;

   if (IS_NPC(ch))
      return;

   if (!get_filename(GET_NAME(ch), buf, OBJECT_FILE))
      return;
   if (!(fp = fopen(buf, "wb")))
      return;

   for (j = 0; j < MAX_WEAR; j++)
       if (ch->equipment[j]) {
	   if (Crash_is_junk(ch->equipment[j]))
	       obj_to_char(unequip_char(ch, j), ch);
	   else
	       Crash_extract_junk(ch->equipment[j], ch);
       }

   Crash_extract_junk(ch->carrying, ch);

   if (!save_char_objs(ch, fp)) {
      fclose(fp);
      return;
   }
   fclose(fp);

   for (j = 0; j < MAX_WEAR; j++)
       if (ch->equipment[j])
	 obj_to_char(unequip_char(ch, j), ch);

   j = 0;
   Crash_extract_objs(ch->carrying, &j);
   sprintf(buf, "%s saving (%d objs).", GET_NAME(ch), j);
   mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
   Crash_delete_file(GET_NAME(ch), OBJECT_BACKUP);

}

/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */

int	Crash_report_unrentables(struct char_data *ch, struct char_data *recep,
struct obj_data *obj)
{
   char	buf[128];
   int	has_norents = 0;

   if (obj) {
      if (Crash_is_unrentable(obj)) {
	 has_norents = 1;
	 sprintf(buf, "$n tells you, 'You cannot store %s.'", OBJS(obj, ch));
	 act(buf, FALSE, recep, 0, ch, TO_VICT);
      }
      has_norents += Crash_report_unrentables(ch, recep, obj->contains);
      has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
   }
   return(has_norents);
}



void	Crash_report_rent(struct char_data *ch, struct char_data *recep,
struct obj_data *obj, long *cost, long *nitems, int display, int factor)
{
   static char	buf[256];

   if (obj) {
      if (!Crash_is_unrentable(obj)) {
	 (*nitems)++;
	 *cost += MAX(0, (obj->obj_flags.cost_per_day >> 1) * factor);
	 if (display) {
	    sprintf(buf, "$n tells you, '%5d coins for %s..'",
	        ((obj->obj_flags.cost_per_day >> 1) * factor), OBJS(obj, ch));
	    act(buf, FALSE, recep, 0, ch, TO_VICT);
	 }
      }
      Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor);
      Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
   }
}



int	Crash_offer_rent(struct char_data *ch, struct char_data *receptionist,
int display, int factor)
{
   extern int	max_obj_save;	/* change in config.c */
   char	buf[MAX_INPUT_LENGTH];
   int	i;
   long	totalcost = 0, numitems = 0, norent = 0, rent_deadline;

   norent = Crash_report_unrentables(ch, receptionist, ch->carrying);
   for (i = 0; i < MAX_WEAR; i++)
      norent += Crash_report_unrentables(ch, receptionist, ch->equipment[i]);

   if (norent)
      return 0;

   totalcost = min_rent_cost * factor;

   Crash_report_rent(ch, receptionist, ch->carrying, &totalcost, &numitems, display, factor);

   for (i = 0; i < MAX_WEAR; i++)
      Crash_report_rent(ch, receptionist, ch->equipment[i], &totalcost, &numitems, display, factor);

   if (!numitems) {
      act("$n tells you, 'But you are not carrying anything!  Just quit!'",
          FALSE, receptionist, 0, ch, TO_VICT);
      return(0);
   }

   if (numitems > max_obj_save) {
      sprintf(buf, "$n tells you, 'Sorry, but I cannot store more than %d items.'",
          max_obj_save);
      act(buf, FALSE, receptionist, 0, ch, TO_VICT);
      return(0);
   }

   if (display) {
      sprintf(buf, "$n tells you, 'Plus, my %d coin fee..'",
          min_rent_cost * factor);
      act(buf, FALSE, receptionist, 0, ch, TO_VICT);
      sprintf(buf, "$n tells you, 'For a total of %ld coins%s.'",
          totalcost, (factor == RENT_FACTOR ? " per day" : ""));
      act(buf, FALSE, receptionist, 0, ch, TO_VICT);
      if (totalcost > GET_GOLD(ch)) {
	 act("$n tells you, '...which I see you can't afford.'",
	     FALSE, receptionist, 0, ch, TO_VICT);
	 return(0);
      } else if (factor == RENT_FACTOR) {
	 if (totalcost) {
	    rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / (totalcost));
	    sprintf(buf,
	        "$n tells you, 'You can rent for %ld day%s with the gold"
	        " you have\r\n     on hand and in the bank.'\r\n",
	        rent_deadline, (rent_deadline > 1) ? "s" : "");
	 }
	 act(buf, FALSE, receptionist, 0, ch, TO_VICT);
      }
   }

   return(totalcost);
}


/*
int	gen_receptionist(struct char_data *ch, int cmd, char *arg, int mode)
{
  int	cost = 0;
  struct char_data *recep = 0;
  struct char_data *tch;
  sh_int save_room;
  sh_int action_tabel[9] = { 23, 24, 36, 105, 106, 109, 111, 142, 147 };
  long	rent_deadline;

  ACMD(do_action);
  int	number(int from, int to);

  if ((!ch->desc) || IS_NPC(ch))
    return(FALSE);

  for (tch = world[ch->in_room]->people; (tch) && (!recep); tch = tch->next_in_room)
    if (IS_MOB(tch) && ((mob_index[tch->nr].func == receptionist ||
			 mob_index[tch->nr].func == cryogenicist)))
      recep = tch;
  if (!recep) {
    log("SYSERR: Fubar'd receptionist.");
    exit(1);
  }

  if (!CMD_IS("rent") && !CMD_IS("offer")) {
    if (!number(0, 30))
      do_action(recep, "", action_tabel[number(0,8)], 0);
    return(FALSE);
  }
  if (!AWAKE(recep)) {
    send_to_char("She is unable to talk to you...\r\n", ch);
    return(TRUE);
  }
  if (!CAN_SEE(recep, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return(TRUE);
  }

  if (CMD_IS("rent")) {

    act("$n says 'There's need to rent here.  IO has sacked me.'",
	FALSE, recep, 0, ch, TO_VICT);
    return 1;


    if (mode == RENT_FACTOR) {

    }

    if (!(cost = Crash_offer_rent(ch, recep, FALSE, mode)))
      return 1;
    if (mode == RENT_FACTOR)
      sprintf(buf, "$n tells you, 'Rent will cost you %d gold coins per day.'", cost);
    else if (mode == CRYO_FACTOR)
      sprintf(buf, "$n tells you, 'It will cost you %d gold coins to be frozen.'", cost);
    act(buf, FALSE, recep, 0, ch, TO_VICT);
    if (cost > GET_GOLD(ch)) {
      act("$n tells you, '...which I see you can't afford.'",
	  FALSE, recep, 0, ch, TO_VICT);
      return(1);
    }

    if (cost && (mode == RENT_FACTOR)) {
      rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / (cost));
      sprintf(buf,
	      "$n tells you, 'You can rent for %d day%s with the gold you have\r\n"
	      "on hand and in the bank.'\r\n",
	      rent_deadline, (rent_deadline > 1) ? "s" : "");
      act(buf, FALSE, recep, 0, ch, TO_VICT);
    }

    if (mode == RENT_FACTOR) {
      act("$n stores your belongings and helps you into your private chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_rentsave(ch, cost);
      sprintf(buf, "%s has rented (%d/day, %d tot.)", GET_NAME(ch),
	      cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
    } else {
      act("$n stores your belongings and helps you into your private chamber.\r\n"
	  "A white mist appears in the room, chilling you to the bone...\r\n"
	  "You begin to lose consciousness...",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_cryosave(ch, cost);
      sprintf(buf, "%s has cryo-rented.", GET_NAME(ch));
      SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
    }

    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
    save_room = ch->in_room;
    extract_char(ch);
    ch->in_room = world[save_room]->number;
    save_char(ch, IN_VROOM(ch), 2);
  } else {
    Crash_offer_rent(ch, recep, TRUE, mode);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  return(TRUE);
}


SPECIAL(receptionist)
{
   return(gen_receptionist(ch, cmd, arg, RENT_FACTOR));
}


SPECIAL(cryogenicist)
{
   return(gen_receptionist(ch, cmd, arg, CRYO_FACTOR));
}
*/

/****************************************************************************
  ROOM CRASH SAVE SYSTEM -Petrus
*****************************************************************************/


int    Crash_load_rooms(void)
{
  FILE * fl;
  char	buf[MAX_INPUT_LENGTH], room_number[32];
  struct obj_data *last_obj, *tmp, *objs[IN_MAXDEPTH - IN_INVENTORY + 1];
  int	i, last_pos = IN_INVENTORY, pos;
  struct room_list *rooms;

  rooms = room_crash_list;

  while(rooms) {

    sprintf(room_number, "%d", world[rooms->number]->number);

    if (!get_filename(room_number, buf, ROOMOBJ_FILE))
      return 0;
    if (!(fl = fopen(buf, "r+b"))) {
      rooms = rooms->next;
      continue;
    }


    for (i = IN_INVENTORY;i <= IN_MAXDEPTH;i++)
      objs[i - IN_INVENTORY] = 0;
    last_obj = 0;

    i = fparse_token(fl);

    while (i == '#') {

      pos = IN_INVENTORY;

      if (fparse_token(fl) != F_NUMBER)
	break;

      if (real_object(fp_val.val.number) > -1) {

	tmp = read_object(fp_val.val.number, VIRTUAL);

	if ((i = object_load(tmp, &pos, fl)) == F_ERROR)
	  break;

	if (tmp) {
	  tmp->carried_by = 0;
	  tmp->in_obj = 0;
	  tmp->in_room = NOWHERE;
	  tmp->contains = 0;
	  tmp->next_content = 0;

	  assert(pos >= 0 && pos <= IN_MAXDEPTH);

	  if (pos >= 0 && pos <= IN_INVENTORY) {
	    obj_to_room(tmp, rooms->number);
	  } else if (pos == last_pos && last_obj) {

            /* obj_to_obj without adding weight to container */
            tmp->next_content = (last_obj->in_obj)->contains;
            (last_obj->in_obj)->contains = tmp;
            tmp->in_obj = (last_obj->in_obj);

	  } else if (pos > last_pos && last_obj) {

            /* obj_to_obj without adding weight to container */
            tmp->next_content = last_obj->contains;
            last_obj->contains = tmp;
            tmp->in_obj = last_obj;

	    if (last_pos >= IN_INVENTORY)
	      objs[last_pos - IN_INVENTORY] = last_obj;
	  } else {

            /* obj_to_obj without adding weight to container */
            tmp->next_content = (objs[pos - IN_INVENTORY]->in_obj)->contains;
            (objs[pos - IN_INVENTORY]->in_obj)->contains = tmp;
            tmp->in_obj = (objs[pos - IN_INVENTORY]->in_obj);

	  }
	}

	last_pos = pos;
	last_obj = tmp;
      } else {
	tmp = read_object(0, VIRTUAL);

	if ((i = object_load(tmp, &pos, fl)) == F_ERROR)
	  break;

	extract_obj(tmp);
      }
    }

    fclose(fl);
    rooms = rooms->next;
  }

  return 1;
}




void	Crash_crashsave_room(sh_int roomnr)
{
   char	buf[MAX_INPUT_LENGTH], room_number[32];
   FILE * fp;

   sprintf(room_number, "%d", world[roomnr]->number);

   if (!get_filename(room_number, buf, ROOMOBJ_FILE))
      return;
   if (!(fp = fopen(buf, "wb")))
       return;

   if (!save_obj(world[roomnr]->contents, IN_INVENTORY, fp)) {
       fclose(fp);
       return;

   }
   fclose(fp);
}


void	Crash_save_all(void)
{
  struct descriptor_data *d;
  struct room_list *rooms;

  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLYNG) && !IS_NPC(d->character)) {

      if (PLR_FLAGGED(d->character, PLR_SAVEOBJS))
	Crash_crashsave(d->character, CRASH_SAVE);
      if (PLR_FLAGGED(d->character, PLR_SAVECHR))
	save_char(d->character, IN_VROOM(d->character), 1);
      if (PLR_FLAGGED(d->character, PLR_SAVEALS))
	alias_save(d->character);
      if (PLR_FLAGGED(d->character, PLR_SAVESTR))
	stringdata_save(d->character);
    }
  }

  rooms = room_crash_list;

  while (rooms) {
    Crash_crashsave_room(rooms->number);
    rooms = rooms->next;
  }
}



/****************************************************************************
  ALIAS-SAVE MODULE TO ELITE MUD
*****************************************************************************/

void  alias_save(struct char_data *ch)
{
  char  buf[81];
  struct alias_list  *ls;
  int i;
  FILE  *fp;

  if (!ch || IS_NPC(ch))
    return;

  // Remove first of all in case we have no aliases at all..
  REMOVE_BIT(PLR_FLAGS(ch), PLR_SAVEALS);

  if (!ch->specials.aliases) {
    // He/She doesnt need to know?
	//send_to_char("You don't have any aliases defined - none saved.\r\n", ch);

	// No aliases make sure we unlink the file to prevent it from loading old
	// file when entering game next time
	get_filename(GET_NAME(ch), buf, ALIAS_FILE);
    unlink(buf);
    return;
  }

  if (!get_filename(GET_NAME(ch), buf, ALIAS_FILE)) {
    send_to_char("Alias Save Error 1!\r\n", ch);
    return;
  }

  if (!(fp = fopen(buf, "wb"))) {
    send_to_char("Alias Save Error 2!\r\n", ch);
    return;
  }

  ls = ch->specials.aliases;
  for (i = 0; ls;ls = ls->next, i++) {
    if (!ls->alias || !ls->replace) {
      send_to_char("Alias Save Error 3!\r\n", ch);
      fclose(fp);
      return;
    }

    fputs(ls->alias, fp);
    fputc('\n', fp);
    fputs(ls->replace, fp);
    fputc('\n', fp);
  }

  fclose(fp);
  /* prevent tick counting....
  sprintf(buf, "Ok, %d aliases saved.\r\n", i);
  send_to_char(buf, ch);*/
}


/* Helpfunction due to the fact that fgets sucks  -Petrus */
int strlen_w_o_blanks(char *s)
{
  char *p;
  int i;

  if ((i = strlen(s)) == 0)
    return 0;
  p = (s + i - 1);

  while(p != s && (*p == ' ' || *p == '\n' || *p == '\r')) {
    p--;
    i--;
  }

  return i;
}

void  alias_load(struct char_data *ch)
{
  void free_alias_list(struct alias_list *ls);

  char  buf[100];
  struct alias_list  *ls, *prev = NULL;
  int i = 0, l;
  FILE  *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, ALIAS_FILE)) {
    send_to_char("Alias Load Error 1!\r\n", ch);
    return;
  }

  if (!(fp = fopen(buf, "r+b"))) {
    send_to_char("You don't have an alias file.\r\n", ch);
    return;
  }

  if (ch->specials.aliases) {
    send_to_char("Removing current aliases...\r\n", ch);
    free_alias_list(ch->specials.aliases);
  }

  while (fgets(buf, 10, fp) != 0) {
    i++;
    CREATE(ls, struct alias_list, 1);

    if (i == 1)
      ch->specials.aliases = ls;
    else
      prev->next = ls;
    prev = ls;

    l = strlen_w_o_blanks(buf);
    CREATE(ls->alias, char, l+1);
    strncpy(ls->alias, buf, l);

    if (fgets(buf, 82, fp) == 0) {
      send_to_char("Alias Load Error 2!\r\n", ch);
      free(ls->alias);
      free(ls);
      fclose(fp);
      return;
    }

    l = strlen_w_o_blanks(buf);
    CREATE(ls->replace, char, l+1);
    strncpy(ls->replace, buf, l);
  }

  fclose(fp);
  sprintf(buf, "Ok, %d aliases loaded.\r\n", i);
  send_to_char(buf, ch);
}


void delete_alias_file(struct char_data *ch)
{
  char	filename[50];
  FILE * fl;

  if (!get_filename(GET_NAME(ch), filename, ALIAS_FILE))
    return;

  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) { /* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting alias file %s (1)", filename);
      perror(buf1);
    }
    return;
  }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) { /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting alias file %s (2)", filename);
      perror(buf1);
    }
  }
}


/* ****************************************************************************
   STRING DATA SAVE MODULE
******************************************************************************/

void  stringdata_save(struct char_data *ch)
{
  char  buf[64];
  FILE  *fp;

  if (!ch || IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, STRING_FILE)) {
    send_to_char("String Save Error 1!\r\n", ch);
    return;
  }

  if (!(fp = fopen(buf, "wb"))) {
    send_to_char("String Save Error 2!\r\n", ch);
    return;
  }

  if (ch->player.title) {
    fparse_write_strtoken(fp, "TITLE ");
    fparse_write_string(fp, ch->player.title);
  }
  if (ch->player.description) {
    fparse_write_strtoken(fp, "DESC ");
    fparse_write_string(fp, ch->player.description);
  }
  if (ch->player.plan) {
    fparse_write_strtoken(fp, "PLAN ");
    fparse_write_string(fp, ch->player.plan);
  }
  if (ch->specials.wizname) {
    fparse_write_strtoken(fp, "WIZNAME ");
    fparse_write_string(fp, ch->specials.wizname);
  }
  if (ch->specials.prename) {
    fparse_write_strtoken(fp, "PRENAME ");
    fparse_write_string(fp, ch->specials.prename);
  }
  if (ch->specials.poofIn) {
    fparse_write_strtoken(fp, "POOFIN ");
    fparse_write_string(fp, ch->specials.poofIn);
  }
  if (ch->specials.poofOut) {
    fparse_write_strtoken(fp, "POOFOUT ");
    fparse_write_string(fp, ch->specials.poofOut);
  }
  if (ch->specials.transIn) {
    fparse_write_strtoken(fp, "TRANSIN ");
    fparse_write_string(fp, ch->specials.transIn);
  }
  if (ch->specials.transOut) {
    fparse_write_strtoken(fp, "TRANSOUT ");
    fparse_write_string(fp, ch->specials.transOut);
  }

  fclose(fp);

  REMOVE_BIT(PLR_FLAGS(ch), PLR_SAVESTR);
}


void  stringdata_load(struct char_data *ch)
{
  FILE  *fp;

  int i;
  char **ptr;

  if (!ch || IS_NPC(ch))
    return;

  if (ch->player.title)
    free(ch->player.title);
  ch->player.title         = NULL;
  if (ch->player.description)
    free(ch->player.description);
  ch->player.description   = NULL;
  if (ch->player.plan)
    free(ch->player.plan);
  ch->player.plan          = NULL;
  if (ch->specials.wizname)
    free(ch->specials.wizname);
  ch->specials.wizname     = NULL;
  if (ch->specials.prename)
    free(ch->specials.prename);
  ch->specials.prename     = NULL;
  if (ch->specials.poofIn)
    free(ch->specials.poofIn);
  ch->specials.poofIn      = NULL;
  if (ch->specials.poofOut)
    free(ch->specials.poofOut);
  ch->specials.poofOut     = NULL;
  if (ch->specials.transIn)
    free(ch->specials.transIn);
  ch->specials.transIn     = NULL;
  if (ch->specials.transOut)
    free(ch->specials.transOut);
  ch->specials.transOut    = NULL;

  if (!get_filename(GET_NAME(ch), buf, STRING_FILE)) {
    send_to_char("String Load Error 1!\r\n", ch);
    return;
  }

  if (!(fp = fopen(buf, "r+b"))) {
    return;
  }

  while (1) {
    if ((i = fparse_token(fp)) != F_STRTOKEN)
      break;

    ptr = 0;

    switch (*(fp_val.val.string)) {

    case 'T':
      if (!strcmp(fp_val.val.string, "TITLE"))
	ptr = &ch->player.title;
      else if (!strcmp(fp_val.val.string, "TRANSIN"))
        ptr = &ch->specials.transIn;
      else if (!strcmp(fp_val.val.string, "TRANSOUT"))
        ptr = &ch->specials.transOut;
      break;

    case 'D':
      if (!strcmp(fp_val.val.string, "DESC"))
	ptr = &ch->player.description;
      break;

    case 'W':
      if (!strcmp(fp_val.val.string, "WIZNAME"))
	ptr = &ch->specials.wizname;
      break;

    case 'P':
      if (!strcmp(fp_val.val.string, "PLAN"))
	ptr = &ch->player.plan;
      else if (!strcmp(fp_val.val.string, "PRENAME"))
	ptr = &ch->specials.prename;
      else if (!strcmp(fp_val.val.string, "POOFIN"))
	ptr = &ch->specials.poofIn;
      else if (!strcmp(fp_val.val.string, "POOFOUT"))
	ptr = &ch->specials.poofOut;
      break;

    }

    if (!ptr) {
      send_to_char("Your string data file is corrupt. Redefine your strings and save.\r\n", ch);
      fclose(fp);
      return;
    }

    while ((i = fparse_token(fp)) != F_STRING)
      if (i != ':' && i != '=') {
	send_to_char("Your string data file is corrupt. Redefine your strings and save.\r\n", ch);
	fclose(fp);
	return;
      }

    *ptr = strdup(fp_val.val.string);
  }

  fclose(fp);
}




/* Crash_listlocker included for reimb only */

void Crash_listlocker(struct char_data *ch, char *name)
{
  FILE * fl;
  char fname[MAX_INPUT_LENGTH], buf[LARGE_BUFSIZE], stuff[50];
  struct obj_data *obj;
  int pos, i, vnum, line;

  if(!get_filename(name, fname, LOCKER_FILE))
    return;
  if(!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no locker file.\n\r", name);
    send_to_char(buf, ch);
    return;
  }
  name = CAP(name);

  sprintf(buf, "#w%s's Locker File#y\r\n", name);
  sprintf(stuff, buf);

  for(line = 0;line < strlen(stuff) - 4;line++) {
    sprintf(buf, "%s~", buf);
  }
  sprintf(buf, "%s#N\r\n", buf);
if(GET_LEVEL(ch) >= LEVEL_DEITY)
    sprintf(buf, "%s filename: %s\n\r", buf, fname);

  i = fparse_token(fl);

  while (i == '#') {
    pos = IN_INVENTORY;
    if(fparse_token(fl) != F_NUMBER)
      break;

    if(real_object((vnum = fp_val.val.number)) > -1) {
      obj = read_object(vnum, VIRTUAL);
      if((i = object_load(obj, &pos, fl)) == F_ERROR)
        break;
      pos = MIN(IN_MAXDEPTH, pos);

      if(GET_LEVEL(ch) >= LEVEL_DEITY) {
        sprintf(buf, "%s [%5d] (%9dg) %-20s\n\r",
                buf, vnum, obj->obj_flags.cost,
                obj->short_description);
      } else {
        sprintf(buf, "%s %-20s\n\r", buf, obj->short_description);
      }
      extract_obj(obj);
    } else {
      obj = read_object(0, VIRTUAL);
      if((i = object_load(obj, &pos, fl)) == F_ERROR)
        break;
      extract_obj(obj);
    }
  }
  page_string(ch->desc, buf, TRUE);
  fclose(fl);
}


/* END Crash_listlocker */

/* List plog to screen */
void list_plog(struct char_data *ch, char *name)
{
  char  fname[MAX_INPUT_LENGTH];

  if (!get_filename(name, fname, PLOG_FILE) ||
      file_to_string(fname, buf) < 0) {
    *name = UPPER(*name);
    sprintf(buf, "%s has no log file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  send_to_char(fname, ch);
  send_to_char("\r\n", ch);
  page_string(ch->desc, buf, TRUE);
}

char *write_plog(char *file, char *output)
{
  FILE *ptFHndl;
  char pcFileName[128];
  time_t ct;
  char *tmstr;

  /* get name of plog file */
  get_filename(file, pcFileName, PLOG_FILE);

  if ((ptFHndl = fopen(pcFileName,"a")) == NULL)
    return("Please try again, file in use.\r\n");

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  fprintf(ptFHndl, "%-19.19s :: %s\n", tmstr, output);
  fclose(ptFHndl);

  return("Ok.\r\n");
}

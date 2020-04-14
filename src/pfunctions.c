/* ****************************************************************************
*  File: pfunctions.c                                       Part of EliteMUD  *
*  Usage: Function definitions for EliteC                                     *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) Mr Wang 1995 EliteMud RIT                                    *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "parser.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "functions.h"

extern struct room_data **world;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern struct index_data *mob_index;
extern struct char_data *mob_proto;

/* Commands for Elite-C ******************************************************/


int
p_do(struct parse_data ch, struct parse_data string)
{
  command_interpreter(ch.val.chr, string.val.string);
  p_tmpval.type = P_NUMBER;
  return p_tmpval.val.number = 1;
}

int
p_number(struct parse_data from, struct parse_data to)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = number(from.val.number, to.val.number);
  return 1;
}

int
p_isnpc(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = IS_NPC(ch.val.chr);
  return 1;
}

int
p_ispc(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = !IS_NPC(ch.val.chr);
  return 1;
}

int
p_isplayer(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = !IS_NPC(ch.val.chr) && GET_LEVEL(ch.val.chr) < LEVEL_DEITY;
  return 1;
}

int
p_isgood(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = IS_GOOD(ch.val.chr);
  return 1;
}

int
p_isneutral(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = IS_NEUTRAL(ch.val.chr);
  return 1;
}

int
p_isevil(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = IS_EVIL(ch.val.chr);
  return 1;
}

int
p_name(struct parse_data obj)
{
  p_tmpval.type = P_STRING;
  switch (obj.type) {
  case P_CHR: 
    p_tmpval.val.string = parse_strdup(fname((obj.val.chr)->player.name));
    break;
  case P_OBJ:
    p_tmpval.val.string = parse_strdup(fname((obj.val.obj)->name)); break;
  case P_ROOM:
    p_tmpval.val.string = parse_strdup((obj.val.room)->name); break;
  default:
    perror("type mismatch for arg to function 'name'");
    return 0;
  }
  return 1;
}

int
p_full_name(struct parse_data obj)
{
  p_tmpval.type = P_STRING;
  switch (obj.type) {
  case P_CHR: 
    p_tmpval.val.string = parse_strdup(GET_NAME(obj.val.chr)); break;
  case P_OBJ:
    p_tmpval.val.string = parse_strdup((obj.val.obj)->short_description);
    break;
  case P_ROOM:
    p_tmpval.val.string = parse_strdup((obj.val.room)->name); break;
  default:
    perror("type mismatch for arg to function 'full_name'");
    return 0;
  }
  return 1;
}

int
p_level(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = GET_LEVEL(ch.val.chr);
  return 1;
}

int
p_isimmort(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = GET_LEVEL(ch.val.chr) >= LEVEL_DEITY;
  return 1;
}

int
p_ischarmed(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = IS_AFFECTED(ch.val.chr, AFF_CHARM);
  return 1;
}

int
p_capitalize(struct parse_data str)
{
  p_tmpval.type = P_STRING;
  p_tmpval.val.string = CAP(str.val.string);
  return 1;
}

int
p_inroom(struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = IN_VROOM(ch.val.chr);
  return 1;
}

int
p_send_to_char(struct parse_data string, struct parse_data ch)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = 1;
  send_to_char(string.val.string, ch.val.chr);
  return 1;
}

int
p_strncmp(struct parse_data str1, struct parse_data str2, struct parse_data n)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = 
    strncmp(str1.val.string, str2.val.string, n.val.number);
  return 1;
}

int
p_echoaround(struct parse_data ch, struct parse_data mob, struct parse_data str)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = 1;
  act(str.val.string, FALSE, ch.val.chr, 0, mob.val.chr, TO_NOTVICT);
  return 1;
}

int
p_echoat(struct parse_data ch, struct parse_data str)
{
  p_tmpval.type = P_NUMBER;
  p_tmpval.val.number = 1;
  act(str.val.string, FALSE, ch.val.chr, 0, 0, TO_CHAR);
  return 1;
}

int
p_GOLD(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_NUMBER;
  p_tmpval.val.ref.ptr = &(GET_GOLD(ch.val.chr));
  return 1;
}

int
p_BANK(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_NUMBER;
  p_tmpval.val.ref.ptr = &(GET_BANK_GOLD(ch.val.chr));
  return 1;
}

int
p_EXP(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_NUMBER;
  p_tmpval.val.ref.ptr = &(GET_EXP(ch.val.chr));
  return 1;
}

int
p_ALIGNMENT(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_NUMBER;
  p_tmpval.val.ref.ptr = &(GET_ALIGNMENT(ch.val.chr));
  return 1;
}

int
p_QUEST(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_NUMBER;
  p_tmpval.val.ref.ptr = &(QUEST_NUM(ch.val.chr));
  return 1;
}

int
p_QUESTFLAG(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_LONG;
  p_tmpval.val.ref.ptr = &(QUEST_FLAGS(ch.val.chr));
  return 1;
}

int
p_HIT(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_SHINT;
  p_tmpval.val.ref.ptr = &(GET_HIT(ch.val.chr));
  return 1;
}

int
p_MAX_HIT(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_SHINT;
  p_tmpval.val.ref.ptr = &(GET_MAX_HIT(ch.val.chr));
  return 1;
}

int
p_MANA(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_SHINT;
  p_tmpval.val.ref.ptr = &(GET_MANA(ch.val.chr));
  return 1;
}

int
p_MAX_MANA(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_SHINT;
  p_tmpval.val.ref.ptr = &(GET_MAX_MANA(ch.val.chr));
  return 1;
}

int
p_MOVE(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_SHINT;
  p_tmpval.val.ref.ptr = &(GET_MOVE(ch.val.chr));
  return 1;
}

int
p_MAX_MOVE(struct parse_data ch)
{
  p_tmpval.type = P_REF;
  p_tmpval.val.ref.type = P_SHINT;
  p_tmpval.val.ref.ptr = &(GET_MAX_MOVE(ch.val.chr));
  return 1;
}

static int num_of_symrecs = 0;

/* Command table */
struct symrec parse_symbols[] = {
  { "if"          , P_IF     , 0 , 0 , 0 , 0 },
  { "else"        , P_ELSE   , 0 , 0 , 0 , 0 },
  { "return"      , P_RETURN , 0 , 0 , 0 , 0 },
  { "int"         , P_TYPE   , P_NUMBER, 0 , 0 , 0 },
  { "long"        , P_TYPE   , P_LONG, 0 , 0 , 0 },
  { "float"       , P_TYPE   , P_REAL, 0 , 0 , 0 },
  { "string"      , P_TYPE   , P_STRING, 0 , 0 , 0 },
  { "char_data"   , P_TYPE   , P_CHR , 0 , 0 , 0 },
  { "obj_data"    , P_TYPE   , P_OBJ, 0 , 0 , 0 },
  { "room_data"   , P_TYPE   , P_ROOM, 0 , 0 , 0 },
  { "include"     , P_INCLUDE, 0 , 0 , 0 },
  { "do"          , P_NUMBER , P_CHR, P_STRING, 0, p_do },
  { "send_to_char", P_NUMBER , P_STRING, P_CHR, 0, p_send_to_char },
  { "level"       , P_NUMBER , P_CHR, 0 , 0 , p_level },
  { "name"        , P_STRING , P_ANY, 0 , 0 , p_name },
  { "full_name"   , P_STRING , P_ANY, 0 , 0 , p_full_name },
  { "ispc"        , P_NUMBER , P_CHR, 0 , 0 , p_ispc },
  { "isplayer"    , P_NUMBER , P_CHR, 0 , 0 , p_isplayer },
  { "isnpc"       , P_NUMBER , P_CHR, 0 , 0 , p_isnpc },
  { "isgood"      , P_NUMBER , P_CHR, 0 , 0 , p_isgood },
  { "isneutral"   , P_NUMBER , P_CHR, 0 , 0 , p_isneutral },
  { "isevil"      , P_NUMBER , P_CHR, 0 , 0 , p_isevil },
  { "isimmort"    , P_NUMBER , P_CHR, 0 , 0 , p_isimmort },
  { "ischarmed"   , P_NUMBER , P_CHR, 0 , 0 , p_ischarmed },
  { "CAP"         , P_STRING , P_STRING, 0 , 0, p_capitalize },
  { "inroom"      , P_NUMBER , P_CHR, 0 , 0 , p_inroom },
  { "number"      , P_NUMBER , P_NUMBER, P_NUMBER, 0 , p_number },
  { "strncmp"     , P_NUMBER , P_STRING, P_STRING, P_NUMBER , p_strncmp },
  { "echoat"      , P_NUMBER , P_CHR, P_STRING, 0, p_echoat },
  { "echoaround"  , P_NUMBER , P_CHR, P_CHR, P_STRING, p_echoaround },
  { "HIT"         , P_REF    , P_CHR, 0 , 0 , p_HIT },  
  { "MAX_HIT"     , P_REF    , P_CHR, 0 , 0 , p_MAX_HIT },  
  { "MANA"        , P_REF    , P_CHR, 0 , 0 , p_MANA },  
  { "MAX_MANA"    , P_REF    , P_CHR, 0 , 0 , p_MAX_MANA },  
  { "MOVE"        , P_REF    , P_CHR, 0 , 0 , p_MOVE },  
  { "MAX_MOVE"    , P_REF    , P_CHR, 0 , 0 , p_MAX_MOVE },  
  { "GOLD"        , P_REF    , P_CHR, 0 , 0 , p_GOLD },  
  { "EXP"         , P_REF    , P_CHR, 0 , 0 , p_EXP },  
  { "BANK"        , P_REF    , P_CHR, 0 , 0 , p_BANK },  
  { "ALIGNMENT"   , P_REF    , P_CHR, 0 , 0 , p_ALIGNMENT },  
  { "QUEST"       , P_REF    , P_CHR, 0 , 0 , p_QUEST },  
  { "QUESTFLAG"   , P_REF    , P_CHR, 0 , 0 , p_QUESTFLAG },  

  { "\n"        , 0, 0, 0, 0, 0 }
};


static char*  p_type_table[] = {
  "INT", "LONG", "FLOAT", "REF", "TYPE", "STRING", "CHAR_DATA",
  "OBJ_DATA", "ROOM_DATA", "TYPE_ANY", "UKNOWN"
};

char *
parse_get_typename(int type)
{
  if (type >= P_NUMBER && type <= P_ANY)
    return p_type_table[type - P_NUMBER];
  else
    return "UNKNOWN";
}

/* Sorts symrecs with bubble sort (preferably executed at runtime)
 * so that binary search can be used
 */
static void
parse_sort_symrecs()
{
  struct symrec val;
  int i, j;
  
  for (num_of_symrecs = 0;  /* Count records */
       *parse_symbols[num_of_symrecs].name != '\n';
       ++num_of_symrecs);
  
  for (j = num_of_symrecs - 1; j > 0; --j)
    for (i = 0; i < j; ++i)
      if (strcmp(parse_symbols[i].name, parse_symbols[i+1].name) > 0) {
	val = parse_symbols[i+1];
	parse_symbols[i+1] = parse_symbols[i];
	parse_symbols[i] = val;
      }
}


/* Uses binary search - must have entries sorted -use fparse_sort_fsymrecs()*/
const struct symrec *
parse_get_tokentype(const char *str)
{
  int  bot = 0, top = num_of_symrecs, mid, cmp;
  
  if (!num_of_symrecs)
    parse_sort_symrecs();

  /* perform binary search on fsymrec-table */
  while (bot <= top) {
    mid = (bot + top) / 2;
    cmp = strcmp(str, parse_symbols[mid].name);
    if (!cmp)
      return &(parse_symbols[mid]);
    if (cmp < 0)
      top = mid - 1;
    else
      bot = mid + 1;
  }
  
  return 0;
}


ACMD(do_pfunctions)
{
  int tmp;

  *buf = 0;

  /* Find the command or function, it it exists */
  for (tmp = 0; *parse_symbols[tmp].name != '\n'; tmp++)
    if (parse_symbols[tmp].fptr) {
      sprintf(buf2, "#w%10s #r%-16s#N(",
	      parse_get_typename(parse_symbols[tmp].type),
	      parse_symbols[tmp].name);
      if (parse_symbols[tmp].arg1type)
	sprintf(buf2, "%s#w%10s", buf2,
		parse_get_typename(parse_symbols[tmp].arg1type));
      if (parse_symbols[tmp].arg2type) {
	strcat(buf2, "#N ,");
	sprintf(buf2, "%s#w%10s", buf2,
		parse_get_typename(parse_symbols[tmp].arg2type));
      }
      if (parse_symbols[tmp].arg3type) {
	strcat(buf2, "#N ,");
	sprintf(buf2, "%s#w%10s", buf2,
		parse_get_typename(parse_symbols[tmp].arg3type));
      }
      strcat(buf2, "#N )\r\n");
      strcat(buf, buf2);
    }
  
  page_string(ch->desc, buf, TRUE);
}

/* END: Commands for Elite-C *************************************************/

/* ************************************************************************
*   File: spec_procs.c                                  Part of EliteMUD  *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
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
#include "limits.h"
#include "functions.h"

/*   external vars  */
extern struct room_data **world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];

extern char	*spells[];
extern char     *skills[];
extern char     skill_minlevel[88][20];
extern char     skill_max[88][20];
extern char     skill_difficulty[88][20];
extern char     spell_minlevel[111][20];
extern char     spell_max[111][20];
extern char     spell_difficulty[111][20];

extern int top_of_world;
extern int max_bank_gold;

/* extern functions */
void	add_follower(struct char_data *ch, struct char_data *leader);


struct social_type {
   char	*cmd;
   int	next_line;
};

ACMD(do_action);
ACMD(do_emote);
ACMD(do_gen_com);
ACMD(do_say);

SPECIAL(shop_keeper); /* Added to enable combi shopkeeper specs */


/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

char	*how_good(int percent)
{
   static char	buf[256];

   if (percent == 0)
      strcpy(buf, "(not learned)");
   else if (percent <= 10)
      strcpy(buf, "(awful)");
   else if (percent <= 20)
      strcpy(buf, "(poor)");
   else if (percent <= 40)
      strcpy(buf, "(bad)");
   else if (percent <= 55)
      strcpy(buf, "(average)");
   else if (percent <= 70)
      strcpy(buf, "(fair)");
   else if (percent <= 80)
      strcpy(buf, "(good)");
   else if (percent <= 85)
      strcpy(buf, "(very good)");
   else if (percent <= 95)
       strcpy(buf, "(excellent)");
   else
       strcpy(buf, "(Superb)");

   return (buf);
}

static char	*diff_levels[] = {
     "(impossible)",
     "(extremely)",
     "(very hard)",
     "(difficult)",
     "(tricky)",
     "(normal)",
     "(quite easy)",
     "(easy)",
     "(simple)",
     "(very simple)",
     "(piece of cake)",
     "(rediculous)",
     "(Zzzzzzzz)"
     };


char	*how_hard(int value)
{
   value = MIN(value, 12);

   return diff_levels[value];
}


int know_skill(struct char_data *ch, int skillnr)
{
    int knowskill = FALSE;

    if (skillnr < SKILL_START) {   /* Spell */
	skillnr--;

	if (IS_MULTI(ch) || IS_DUAL(ch)) {
	    if (GET_1LEVEL(ch) >= spell_minlevel[skillnr][GET_1CLASS(ch)-1])
		knowskill = TRUE;
	    if (GET_2LEVEL(ch) >= spell_minlevel[skillnr][GET_2CLASS(ch)-1])
		knowskill = TRUE;
	    if (IS_3MULTI(ch) &&
		(GET_3LEVEL(ch) >= spell_minlevel[skillnr][GET_3CLASS(ch)-1]))
		knowskill = TRUE;
	} else
	    if (GET_LEVEL(ch) >= spell_minlevel[skillnr][GET_CLASS(ch)-1])
		knowskill = TRUE;
    } else {
	skillnr -= SKILL_START;

	if (IS_MULTI(ch) || IS_DUAL(ch)) {
	    if (GET_1LEVEL(ch) >= skill_minlevel[skillnr][GET_1CLASS(ch)-1])
		knowskill = TRUE;
	    if (GET_2LEVEL(ch) >= skill_minlevel[skillnr][GET_2CLASS(ch)-1])
		knowskill = TRUE;
	    if (IS_3MULTI(ch) &&
		(GET_3LEVEL(ch) >= skill_minlevel[skillnr][GET_3CLASS(ch)-1]))
		knowskill = TRUE;
	} else
	    if (GET_LEVEL(ch) >= skill_minlevel[skillnr][GET_CLASS(ch)-1])
		knowskill = TRUE;
    }

    return knowskill;
}


int skill_min(struct char_data *ch, int skillnr)
{
  int min = LEVEL_DEITY;

  if (skillnr < SKILL_START) {   /* Spell */
    skillnr--;

    if (IS_MULTI(ch) || IS_DUAL(ch)) {
      if (min > spell_minlevel[skillnr][GET_1CLASS(ch)-1])
	min = spell_minlevel[skillnr][GET_1CLASS(ch)-1];
      if (min > spell_minlevel[skillnr][GET_2CLASS(ch)-1])
	min = spell_minlevel[skillnr][GET_2CLASS(ch)-1];
      if (IS_3MULTI(ch) &&
	  (min > spell_minlevel[skillnr][GET_3CLASS(ch)-1]))
	min = spell_minlevel[skillnr][GET_3CLASS(ch)-1];
    } else
      min = spell_minlevel[skillnr][GET_CLASS(ch)-1];
  } else {
    skillnr -= SKILL_START;

    if (IS_MULTI(ch) || IS_DUAL(ch)) {
      if (min > skill_minlevel[skillnr][GET_1CLASS(ch)-1])
        min = skill_minlevel[skillnr][GET_1CLASS(ch)-1];
      if (min > skill_minlevel[skillnr][GET_2CLASS(ch)-1])
	min = skill_minlevel[skillnr][GET_2CLASS(ch)-1];
      if (IS_3MULTI(ch) &&
	  (min > skill_minlevel[skillnr][GET_3CLASS(ch)-1]))
	min = skill_minlevel[skillnr][GET_3CLASS(ch)-1];
    } else
      min = skill_minlevel[skillnr][GET_CLASS(ch)-1];
  }

  return min;
}


int skill_cost(struct char_data *ch, int skillnr)
{
  int cost = skill_min(ch, skillnr);

  return (cost*cost*10);
}


int get_skill_max(struct char_data *ch, int skillnr)
{
    int max = 1;

    if (skillnr < SKILL_START) {   /* spell */
	skillnr--;

	if (IS_MULTI(ch) || IS_DUAL(ch)) {
	    if (max < spell_max[skillnr][GET_1CLASS(ch)-1])
		max = spell_max[skillnr][GET_1CLASS(ch)-1];
	    if (max < spell_max[skillnr][GET_2CLASS(ch)-1])
		max = spell_max[skillnr][GET_2CLASS(ch)-1];
	    if (IS_3MULTI(ch) &&
		(max < spell_max[skillnr][GET_3CLASS(ch)-1]))
		max = spell_max[skillnr][GET_3CLASS(ch)-1];
	} else
	    max = spell_max[skillnr][GET_CLASS(ch)-1];

    } else {
	skillnr -= SKILL_START;

	if (IS_MULTI(ch) || IS_DUAL(ch)) {
	    if (max < skill_max[skillnr][GET_1CLASS(ch)-1])
		max = skill_max[skillnr][GET_1CLASS(ch)-1];
	    if (max < skill_max[skillnr][GET_2CLASS(ch)-1])
		max = skill_max[skillnr][GET_2CLASS(ch)-1];
	    if (IS_3MULTI(ch) &&
		(max < skill_max[skillnr][GET_3CLASS(ch)-1]))
		max = skill_max[skillnr][GET_3CLASS(ch)-1];
	} else
	    max = skill_max[skillnr][GET_CLASS(ch)-1];
    }

    return max;
}


int get_skill_diff(struct char_data *ch, int skillnr)
{
    int diff = 1;

    if (skillnr < SKILL_START) {   /* spell */
	skillnr--;

	if (IS_MULTI(ch) || IS_DUAL(ch)) {
	    if (diff < spell_difficulty[skillnr][GET_1CLASS(ch)-1])
		diff = spell_difficulty[skillnr][GET_1CLASS(ch)-1];
	    if (diff < spell_difficulty[skillnr][GET_2CLASS(ch)-1])
		diff = spell_difficulty[skillnr][GET_2CLASS(ch)-1];
	    if (IS_3MULTI(ch) &&
		(diff < spell_difficulty[skillnr][GET_3CLASS(ch)-1]))
		diff = spell_difficulty[skillnr][GET_3CLASS(ch)-1];
	} else
	    diff = spell_difficulty[skillnr][GET_CLASS(ch)-1];

    } else {
	skillnr -= SKILL_START;

	if (IS_MULTI(ch) || IS_DUAL(ch)) {
	    if (diff < skill_difficulty[skillnr][GET_1CLASS(ch)-1])
		diff = skill_difficulty[skillnr][GET_1CLASS(ch)-1];
	    if (diff < skill_difficulty[skillnr][GET_2CLASS(ch)-1])
		diff = skill_difficulty[skillnr][GET_2CLASS(ch)-1];
	    if (IS_3MULTI(ch) &&
		(diff < skill_difficulty[skillnr][GET_3CLASS(ch)-1]))
		diff = skill_difficulty[skillnr][GET_3CLASS(ch)-1];
	} else
	    diff = skill_difficulty[skillnr][GET_CLASS(ch)-1];
    }

    return diff;
}


void  practices(struct char_data *ch, int mode)
{
  char buf[LARGE_BUFSIZE];
  int i, l;
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  sprintf(buf, "You have got #w%d#N practice sessions left.\r\n",
	  SPELLS_TO_LEARN(ch));
  if (mode == 1) {
    strcat(buf, "\r\n#wThese are the skills you know: #g(* = Special/Extra Skill)#N\r\n");
    for (i = 0, l = 0; *skills[i] != '\n'; i++) {
      if (GET_SKILL(ch, i + SKILL_START)) {
	sprintf(buf, "%s#r%-20s%s#b%14s#N", buf, skills[i],
		(know_skill(ch, i + SKILL_START)?" ":"#g*"),
		how_good(GET_SKILL(ch, i + SKILL_START)));
	if (l % 2)
	  strcat (buf, "\r\n");
	else
	  strcat (buf, " | ");
	l++;
      }
    }
  }

  if (mode == 2) {
    strcat(buf, "\r\n#wThese are the spells you know: #g(* = Special/Extra Spell)#N\r\n");
    for (i = 0, l = 0; *spells[i] != '\n' && i < NUM_OF_SPELLS; i++)
      if (spell_info[i+1].type &&
	  GET_SKILL(ch, i + 1)) {
	sprintf(buf2, "#y%3d #r%-20s%s#b%12s#N", i + 1, spells[i],
		(know_skill(ch, i + 1)?" ":"#g*"),
		how_good(GET_SKILL(ch, i + 1)));
	if (l % 2)
	  strcat (buf2, "\r\n");
	else
	  strcat (buf2, " | ");
	l++;

	strcat(buf, buf2);
      }
  }

  if (mode == 3) {
    strcat(buf, "\r\n#wThese are the skills your class will learn:#N\r\n");
    for (l = 1; l < LEVEL_DEITY; l++)
      for (i = 0; *skills[i] != '\n'; i++) {
	if (skill_min(ch, i + SKILL_START) == l) {
	  sprintf(buf, "%s#YLevel %3d: #r%-20s#N\r\n", buf, l, skills[i]);
	}
      }
  }

  if (mode == 4) {
    strcat(buf, "\r\n#wThese are the spells your class will learn:#N\r\n");
    for (l = 1; l < LEVEL_DEITY; l++)
      for (i = 0; *spells[i] != '\n'; i++) {
	if (spell_info[i+1].type && skill_min(ch, i + 1) == l) {
	  sprintf(buf, "%s#YLevel %3d: #r%-20s#N\r\n", buf, l, spells[i]);
	}
      }
  }

  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}


int
guild(struct char_data *ch, struct char_data *mob, char *arg)
{
  int	number, i, percent;
  extern struct spell_info_type spell_info[MAX_SPL_LIST];
  char buf[LARGE_BUFSIZE];

  skip_spaces(&arg);

  sprintf(buf, "You have got #w%d#N practice sessions left.\r\n\r\n",
	  SPELLS_TO_LEARN(ch));

  if (!*arg) {
    sprintf(buf, "%s#rProficiency          How good       Difficulty             Cost#N\r\n#w~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#N\r\n", buf);
    for (i = 0; *skills[i] != '\n'; i++) {
      if (know_skill(ch, i + SKILL_START) &&
	  GET_SKILL(ch, i + SKILL_START) < get_skill_max(ch, i + SKILL_START))
	{
	  sprintf(buf2, "#r%-20s #b%-14s #g%-18s #y%8d#N\r\n", skills[i],
		  how_good(GET_SKILL(ch, i + SKILL_START)),
		  how_hard(get_skill_diff(ch, i + SKILL_START)),
		  skill_cost(ch, i + SKILL_START));
	  strcat(buf, buf2);
	}
	}
    for (i = 0; i < NUM_OF_SPELLS && *spells[i] != '\n'; i++)
      if (spell_info[i+1].type && know_skill(ch, i + 1) &&
		GET_SKILL(ch, i + 1) < get_skill_max(ch, i + 1)) {
	sprintf(buf2, "#r%-20s #b%-14s #g%-18s #y%8d#N\r\n", spells[i],
		how_good(GET_SKILL(ch, i + 1)),
		how_hard(get_skill_diff(ch, i + 1)),
		skill_cost(ch, i + 1));
	strcat(buf, buf2);
      }
    page_string(ch->desc, buf, 1);
    return(TRUE);
  }

  number = search_block(arg, spells, FALSE);

  if (number++ == -1 || !know_skill(ch, number)) {
    number = search_block(arg, skills, FALSE);
    if (number == -1 || !know_skill(ch, number + SKILL_START)) {
      act("$N tells you, 'You do not know of this proficiency!'", FALSE, ch, 0, mob, TO_CHAR);
	  return(TRUE);
    }
    number += SKILL_START;
  }

  if (SPELLS_TO_LEARN(ch) <= 0) {
      act("$N tells you, 'You don't seem to be able to practice now!'", FALSE, ch, 0, mob, TO_CHAR);
      return(TRUE);
    }

    if (GET_GOLD(ch) < skill_cost(ch, number)) {
      act("$N tells you, 'You can't afford my training fee!'", FALSE, ch, 0, mob, TO_CHAR);
      return(TRUE);
    }

  if (GET_SKILL(ch, number) >= get_skill_max(ch, number)) {
    act("$N tells you, 'You are already learned in this area.'", FALSE, ch, 0, mob, TO_CHAR);
    return(TRUE);
    }

  send_to_char("You practice for a while...\r\n", ch);
  act("$n practices some skill with $N.", FALSE, ch, 0, mob, TO_ROOM);
  SPELLS_TO_LEARN(ch)--;
  GET_GOLD(ch) -= skill_cost(ch, number);

  percent = GET_SKILL(ch, number) + (GET_INT(ch)*get_skill_diff(ch, number))/5;;
    SET_SKILL(ch, number, MIN(get_skill_max(ch, number), percent));

  if (GET_SKILL(ch, number) >= get_skill_max(ch, number)) {
    act("$N tells you, 'You are now learned in this area.'", FALSE, ch, 0, mob, TO_CHAR);
    return(TRUE);
  }
  return TRUE;
}


SPECIAL(mu_guild)
{
    if (!CMD_IS("practice") || IS_NPC(ch) || !AWAKE(mob))
        return(FALSE);

    return guild(ch, mob, arg);
    /*
    if (IS_MAGIC_USER(ch))
	return guild(ch, mob, arg);
    else {
	send_to_char("You are not of a member of this guild!\r\n", ch);
	return(TRUE);
    }*/
}

SPECIAL(cl_guild)
{
    if (!CMD_IS("practice") || IS_NPC(ch) || !AWAKE(mob))
	return(FALSE);

    return guild(ch, mob, arg);

    /*
    if (IS_CLERIC(ch))
      return guild(ch, mob, arg);
    else {
	send_to_char("You are not of a member of this guild!\r\n", ch);
	return(TRUE);
    }*/
}

SPECIAL(th_guild)
{
    if (!CMD_IS("practice") || IS_NPC(ch) || !AWAKE(mob))
	return(FALSE);

    return guild(ch, mob, arg);
    /*
    if (IS_THIEF(ch))
	return guild(ch, mob, arg);
    else {
	send_to_char("You are not of a member of this guild!\r\n", ch);
	return(TRUE);
    }*/
}

SPECIAL(wa_guild)
{
    if (!CMD_IS("practice") || IS_NPC(ch) || !AWAKE(mob))
	return(FALSE);

    return guild(ch, mob, arg);
    /*
    if (IS_WARRIOR(ch))
	return guild(ch, mob, arg);
    else {
	send_to_char("You are not of a member of this guild!\r\n", ch);
	return(TRUE);
    }*/
}


SPECIAL(dump)
{
   struct obj_data *k;
   struct char_data *tmp_char;
   int	value = 0;

   ACMD(do_drop);
   char	*fname(char *namelist);

   for (k = world[ch->in_room]->contents; k ; k = world[ch->in_room]->contents) {
      sprintf(buf, "The %s vanishes in a puff of smoke.\r\n" , fname(k->name));
      for (tmp_char = world[ch->in_room]->people; tmp_char;
          tmp_char = tmp_char->next_in_room)
	 if (CAN_SEE_OBJ(tmp_char, k))
	    send_to_char(buf, tmp_char);
      extract_obj(k);
   }

   if (!CMD_IS("drop"))
      return(FALSE);

   do_drop(ch, arg, cmd, 0);

   value = 0;

   for (k = world[ch->in_room]->contents; k ; k = world[ch->in_room]->contents) {
      sprintf(buf, "The %s vanishes in a puff of smoke.\r\n", fname(k->name));
      for (tmp_char = world[ch->in_room]->people; tmp_char;
          tmp_char = tmp_char->next_in_room)
	 if (CAN_SEE_OBJ(tmp_char, k))
	    send_to_char(buf, tmp_char);
      value += MAX(1, MIN(50, k->obj_flags.cost / 10));

      extract_obj(k);
   }

   if (value) {
      act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

      if (GET_LEVEL(ch) < 3)
	 gain_exp(ch, value);
      else
	 GET_GOLD(ch) += value;
   }

   return(TRUE);
}


SPECIAL(mayor)
{
   static char	open_path[] =
   "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

   static char	close_path[] =
   "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

   /*
  const struct social_type open_path[] = {
	 {"G",0}
  };

  static void *thingy = 0;
  static int cur_line = 0;

  for (i=0; i < 1; i++) {
    if (*(open_path[cur_line].cmd) == '!') {
      i++;
      exec_social(mob, (open_path[cur_line].cmd)+1,
        open_path[cur_line].next_line, &cur_line, &thingy);
  } else {
      exec_social(mob, open_path[cur_line].cmd,
        open_path[cur_line].next_line, &cur_line, &thingy);
  }
*/
   static char	*path;
   static int	index;
   static bool move = FALSE;

   ACMD(do_move);
   ACMD(do_open);
   ACMD(do_lock);
   ACMD(do_unlock);
   ACMD(do_close);

   if (!move) {
      if (time_info.hours == 6) {
	 move = TRUE;
	 path = open_path;
	 index = 0;
      } else if (time_info.hours == 20) {
	 move = TRUE;
	 path = close_path;
	 index = 0;
      }
   }

   if (cmd || !move || (GET_POS(mob) < POS_SLEEPING) ||
       (GET_POS(mob) == POS_FIGHTING))
      return FALSE;

   switch (path[index]) {
   case '0' :
   case '1' :
   case '2' :
   case '3' :
      do_move(mob, "", path[index] - '0' + 1, 0);
      break;

   case 'W' :
      GET_POS(mob) = POS_STANDING;
      act("$n awakens and groans loudly.", FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'S' :
      GET_POS(mob) = POS_SLEEPING;
      act("$n lies down and instantly falls asleep.", FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'a' :
      act("$n says 'Hello Honey!'", FALSE, mob, 0, 0, TO_ROOM);
      act("$n smirks.", FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'b' :
      act("$n says 'What a view!  I must get something done about that dump!'",
          FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'c' :
      act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
          FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'd' :
      act("$n says 'Good day, citizens!'", FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'e' :
      act("$n says 'I hereby declare the bazaar open!'", FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'E' :
      act("$n says 'I hereby declare Midgaard closed!'", FALSE, mob, 0, 0, TO_ROOM);
      break;

   case 'O' :
      do_unlock(mob, "gate", 0, 0);
      do_open(mob, "gate", 0, 0);
      break;

   case 'C' :
      do_close(mob, "gate", 0, 0);
      do_lock(mob, "gate", 0, 0);
      break;

   case '.' :
      move = FALSE;
      break;

   }

   index++;
   return FALSE;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void	exec_social(struct char_data *npc, char *cmd, int next_line,
int *cur_line, void **thing)
{
   bool ok;

   ACMD(do_move);
   ACMD(do_open);
   ACMD(do_lock);
   ACMD(do_unlock);
   ACMD(do_close);

   if (GET_POS(npc) == POS_FIGHTING)
      return;

   ok = TRUE;

   switch (*cmd) {

   case 'G' :
      *cur_line = next_line;
      return;

   case 'g' :
      *cur_line += next_line;
      return;

   case 'e' :
      act(cmd + 1, FALSE, npc, *thing, *thing, TO_ROOM);
      break;

   case 'E' :
      act(cmd + 1, FALSE, npc, 0, *thing, TO_VICT);
      break;

   case 'B' :
      act(cmd + 1, FALSE, npc, 0, *thing, TO_NOTVICT);
      break;

   case 'm' :
      do_move(npc, "", *(cmd + 1) - '0' + 1, 0);
      break;

   case 'w' :
      if (GET_POS(npc) != POS_SLEEPING)
	 ok = FALSE;
      else
	 GET_POS(npc) = POS_STANDING;
      break;

   case 's' :
      if (GET_POS(npc) <= POS_SLEEPING)
	 ok = FALSE;
      else
	 GET_POS(npc) = POS_SLEEPING;
      break;

   case 'c' :  /* Find char in room */
      *thing = get_char_room_vis(npc, cmd + 1);
      ok = (*thing != 0);
      break;

   case 'o' : /* Find object in room */
      *thing = get_obj_in_list_vis(npc, cmd + 1, world[npc->in_room]->contents);
      ok = (*thing != 0);
      break;

   case 'r' : /* Test if in a certain room */
      ok = (npc->in_room == atoi(cmd + 1));
      break;

   case 'O' : /* Open something */
      do_open(npc, cmd + 1, 0, 0);
      break;

   case 'C' : /* Close something */
      do_close(npc, cmd + 1, 0, 0);
      break;

   case 'L' : /* Lock something  */
      do_lock(npc, cmd + 1, 0, 0);
      break;

   case 'U' : /* UnLock something  */
      do_unlock(npc, cmd + 1, 0, 0);
      break;

   case '?' : /* Test a random number */
      if (atoi(cmd + 1) <= number(1, 100))
	 ok = FALSE;
      break;

   default:
      break;
   }  /* End Switch */

   if (ok)
      (*cur_line)++;
   else
      (*cur_line) += next_line;
}


void	npc_steal(struct char_data *ch, struct char_data *victim)
{
  int	gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LEVEL_DEITY)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your pocket.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


SPECIAL(snake)
{
   if (cmd)
      return FALSE;

   if (GET_POS(mob) != POS_FIGHTING)
      return FALSE;

   if (FIGHTING(mob) &&
       IN_ROOM(FIGHTING(mob)) == IN_ROOM(mob) &&
       (number(0, GET_LEVEL(mob)) > GET_LEVEL(FIGHTING(mob)))) {
	 act("$n bites $N!", 1, mob, 0, FIGHTING(mob), TO_NOTVICT);
	 act("$n bites you!", 1, mob, 0, FIGHTING(mob), TO_VICT);
	 call_magic(ch, "", FIGHTING(mob), NULL, SPELL_POISON,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      return TRUE;
  }
   return FALSE;
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(mob) != POS_STANDING)
    return FALSE;

  for (cons = world[IN_ROOM(mob)]->people; cons; cons = cons->next_in_room )
    if ((!IS_NPC(cons)) && (GET_LEVEL(cons) < GET_LEVEL(mob)) && (number(1, 5) == 1))
      npc_steal(mob, cons);

  return TRUE;
}


SPECIAL(magic_user)
{
   struct char_data *vict;

   if (cmd)
      return FALSE;

   if (GET_POS(mob) != POS_FIGHTING)
      return FALSE;

   if (!FIGHTING(mob))
      return FALSE;

   vict = FIGHTING(mob);

   if (!vict)
      return FALSE;


   if ( (GET_LEVEL(mob) > 13) && (number(0, 7) == 0) ) {
      act("$n utters the words 'dilan osozzz'.", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_SLEEP,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      return TRUE;
   }

   if ( (GET_LEVEL(mob) > 7) && (number(0, 5) == 0) ) {
      act("$n utters the words 'koholian antidia'.", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_BLINDNESS,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      return TRUE;
   }

   if ( (GET_LEVEL(mob) > 12) && (number(0, 8) == 0) && IS_EVIL(mob)) {
      act("$n utters the words 'ib er dranker'.", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_ENERGY_DRAIN,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      return TRUE;
   }

   switch (GET_LEVEL(mob)) {
   case 1:
   case 2:
   case 3:
      break;
   case 4:
   case 5:
      act("$n utters the words 'hahili duvini'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_MAGIC_MISSILE,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   case 6:
   case 7:
      act("$n utters the words 'frigid enton'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_CHILL_TOUCH,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   case 8:
   case 9:
      act("$n utters the words 'grynt oef'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_BURNING_HANDS,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   case 10:
   case 11:
      act("$n utters the words 'juier vrahe'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_SHOCKING_GRASP,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   case 12:
   case 13:
      act("$n utters the words 'sjulk divi'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_LIGHTNING_BOLT,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   case 14:
   case 15:
   case 16:
   case 17:
      act("$n utters the words 'nasson hof'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_COLOUR_SPRAY,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   default:
      act("$n utters the words 'tuborg luca'!", 1, mob, 0, 0, TO_ROOM);
      call_magic(mob, "", vict, NULL, SPELL_FIREBALL,
		 GET_LEVEL(mob), SPELL_TYPE_SPELL);
      break;
   }
   return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(guild_guard)
{

  if (!IS_MOVE(cmd) || !AWAKE(mob))
    return FALSE;

  if ( ((IN_ROOM(ch) == real_room(3017)) && CMD_IS("south")) ||
      ((IN_ROOM(ch) == real_room(21075)) && CMD_IS("north")) ||
      ((IN_ROOM(ch) == real_room(16268)) && CMD_IS("north")) ||
      ((IN_ROOM(ch) == real_room(5825)) && CMD_IS("north"))) {
    if (!IS_MAGIC_USER(ch)) {
      act("$N humiliates you, and blocks your way.", FALSE, ch, 0, mob, TO_CHAR);
      act("$N humiliates $n, and blocks $s way.", FALSE, ch, 0, mob, TO_ROOM);
      return TRUE;
    }
  } else if ( ((IN_ROOM(ch) == real_room(3004)) && CMD_IS("north")) ||
	     ((IN_ROOM(ch) == real_room(21019)) && CMD_IS("west")) ||
             ((IN_ROOM(ch) == real_room(16265)) && CMD_IS("west")) ||
	     ((IN_ROOM(ch) == real_room(5812)) && CMD_IS("south"))) {
    if (!IS_CLERIC(ch)) {
      act("$N humiliates you, and blocks your way.", FALSE, ch, 0, mob, TO_CHAR);
      act("$N humiliates $n, and blocks $s way.", FALSE, ch, 0, mob, TO_ROOM);
      return TRUE;
    }
  } else if ( ((IN_ROOM(ch) == real_room(3027)) && CMD_IS("east")) ||
	     ((IN_ROOM(ch) == real_room(21014)) && CMD_IS("down")) ||
             ((IN_ROOM(ch) == real_room(16296)) && CMD_IS("south")) ||
	     ((IN_ROOM(ch) == real_room(5832)) && CMD_IS("south")) ) {
    if (!IS_THIEF(ch)) {
      act("$N humiliates you, and blocks your way.", FALSE, ch, 0, mob, TO_CHAR);
      act("$N humiliates $n, and blocks $s way.", FALSE, ch, 0, mob, TO_ROOM);
      return TRUE;
    }
  } else if ( ((IN_ROOM(ch) == real_room(3021)) && CMD_IS("east")) ||
	     ((IN_ROOM(ch) == real_room(21023)) && CMD_IS("down")) ||
             ((IN_ROOM(ch) == real_room(16228)) && CMD_IS("north")) ||
	     ((IN_ROOM(ch) == real_room(5826)) && CMD_IS("south")) ) {
    if (!IS_WARRIOR(ch)) {
      act("$N humiliates you, and blocks your way.", FALSE, ch,0, mob, TO_CHAR);
      act("$N humiliates $n, and blocks $s way.", FALSE, ch, 0, mob, TO_ROOM);
      return TRUE;
    }
  }
  return FALSE;
}


SPECIAL(brass_dragon)
{
   char	buf[256], buf2[256];

   if (!IS_MOVE(cmd) || !AWAKE(mob))
      return FALSE;

   strcpy(buf,  "The brass dragon humiliates you, and blocks your way.\r\n");
   strcpy(buf2, "The brass dragon humiliates $n, and blocks $s way.");

   if ((IN_ROOM(ch) == real_room(5065)) && CMD_IS("west")) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
   }

   return FALSE;

}


SPECIAL(fido)
{

   struct obj_data *i, *temp, *next_obj;

   if (cmd || !AWAKE(mob))
      return(FALSE);

   if (IN_ROOM(mob) == NOWHERE)
     return (FALSE);

   for (i = world[IN_ROOM(mob)]->contents; i; i = i->next_content) {
      if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3] == 1) {
	 act("$n savagely devours a corpse.", FALSE, mob, 0, 0, TO_ROOM);
	 for (temp = i->contains; temp; temp = next_obj) {
	    next_obj = temp->next_content;
	    obj_from_obj(temp);
	    obj_to_room(temp, IN_ROOM(mob));
	 }
	 extract_obj(i);
	 return(TRUE);
      }
   }
   return(FALSE);
}



SPECIAL(janitor)
{
   struct obj_data *i;

   if (cmd || !AWAKE(mob))
      return(FALSE);

   if (IN_ROOM(mob) == NOWHERE)
     return (FALSE);

   for (i = world[IN_ROOM(mob)]->contents; i; i = i->next_content) {
      if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
          ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
          (i->obj_flags.cost <= 10))) {
	 act("$n picks up some trash.", FALSE, mob, 0, 0, TO_ROOM);

	 obj_from_room(i);
	 obj_to_char(i, mob);
	 return(TRUE);
      }
   }
   return(FALSE);
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(mob) || (GET_POS(mob) == POS_FIGHTING))
    return (FALSE);

  if (IN_ROOM(mob) == NOWHERE)
    return (FALSE);

  max_evil = 1000;
  evil = 0;

  for (tch = world[IN_ROOM(mob)]->people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(mob, tch) && IS_SET(PLR_FLAGS(tch), PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, mob, 0, 0, TO_ROOM);
      hit(mob, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[IN_ROOM(mob)]->people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(mob, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)){
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, mob, 0, 0, TO_ROOM);
      hit(mob, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[IN_ROOM(mob)]->people; tch; tch = tch->next_in_room) {
    if (CAN_SEE(mob, tch) && FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, mob, 0, 0, TO_ROOM);
    hit(mob, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(cryo_shopkeeper)
{
  if (IS_NPC(ch)) return(FALSE);

  if (CMD_IS("list")) {
    sprintf(buf, "I offer CRYO services for a reasonable 1 million gold.\r\n");
    strcat(buf,  "CRYO will save you from player-file purges for upto\r\n");
    strcat(buf,  "6 months (180 days).  CRYO needs to be re-purchased\r\n");
    strcat(buf,  "when you login.\r\n");
    send_to_char(buf, ch);

    return(TRUE);
  }

  if (CMD_IS("buy")) {
    if (GET_GOLD(ch) < 1000000) {
      send_to_char("You can't afford my fee!\r\n", ch);
      return(TRUE);
    } else {
      send_to_char("Thank You!\r\n", ch);
      SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
      GET_GOLD(ch) -= 1000000;
      return(TRUE);
    }
  }

  return(FALSE);
}

const int questeq_tradein[][2] =
{/*  OLD  ,  NEW          DESCRIPTION           */
  {   114 , 99064 },   /* Qualen                */
  {  1911 , 99063 },   /* Iron Barbed Flail     */
  {  3413 , 99062 },   /* Girdle of Power       */
  {  5704 , 99061 },   /* Poisoned Dagger       */
  {  5790 , 99060 },   /* Chessboard            */
  { 99000 , 99058 },   /* Fang                  */
  { 99003 , 99045 },   /* Tattoos               */
  { 99004 , 99055 },   /* Wyrmslayer            */
  { 99006 , 99059 },   /* Heartseeker           */
  { 99007 , 99033 },   /* Hexagonal Necklace    */
  { 99008 , 99048 },   /* Sha-Gren skin         */
  { 99010 , 99028 },   /* Aura of Alcmene       */
  { 99011 , 99057 },   /* Rabbit Slayer         */
  { 99012 , 99047 },   /* Crystal Shield        */
  { 99014 , 99037 },   /* Diamond Helmet        */
  { 99015 , 99044 },   /* Real Wings            */
  { 99019 , 99056 },   /* Rigel Slayer/Sunblade */
  { 99020 , 99036 },   /* Rigel Breastplate     */
  { 99021 , 99046 },   /* Rigel Armplates       */
  { 99022 , 99039 },   /* Rigel Legplates       */
  { 99023 , 99041 },   /* Rigel Shoes           */
  { 99025 , 99025 }    /* Blue Sabre (no vnum change) */
};

#define QUESTEQ_TRADE_MAX 22

SPECIAL(questeq_shopkeeper)
{
  char buf[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];
  struct obj_data *questeq, *newobj;
  int num = 0, i = 0;
  extern struct obj_data *questeq_list;
  extern struct str_app_type str_app[];

  *buf = '\0';

  if (IS_NPC(ch)) return(FALSE);

  if (!questeq_list) return(FALSE);

  if (CMD_IS("list")) { /* List */
    sprintf(buf, "Number  %-40s %s  %s\r\n", "Name:", "Level", "Quest Points");
    for (questeq = questeq_list; questeq; questeq = questeq->next) {
      if (questeq->obj_flags.cost > 0) {
	num++;
	sprintf(buf2, "%3d.  %40s  [%4d]  %d\r\n",
		num,
		questeq->short_description,
		questeq->obj_flags.level,
		questeq->obj_flags.cost);

	strcat(buf, buf2);
      }
    }

    page_string(ch->desc, buf, TRUE);
    return (TRUE);
  }

  if (CMD_IS("buy")) {
    one_argument(arg, buf);

    if (!is_number(buf)) {
      send_to_char ("Place your purchase using the item number.\r\n", ch);
      return (TRUE);
    } else
      num = atoi(buf);

    for (questeq = questeq_list; questeq; questeq = questeq->next) {
      if (questeq->obj_flags.cost > 0) {
	i++;
	if (i == num) break;
      }
    }

    if (i == num) {
      if (GET_LEVEL(ch) < questeq->obj_flags.level) {
	send_to_char("You can't buy an item above your level!\r\n", ch);
	return (TRUE);
      }

      if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
	send_to_char("You can't carry that many items!\r\n", ch);
	return (TRUE);
      }

      if ((IS_CARRYING_W(ch) + questeq->obj_flags.weight) > CAN_CARRY_W(ch)) {
	send_to_char("You can't carry that much weight.\r\n",  ch);
	return (TRUE);
      }

      if (QUEST_NUM(ch) >= questeq->obj_flags.cost) {
	newobj = read_object(questeq->item_number, REAL);
	obj_to_char(newobj, ch);
	send_to_char("Sold!\r\n", ch);
	QUEST_NUM(ch) -= questeq->obj_flags.cost;

	sprintf(buf, "%s bought %s [%d]",
		GET_NAME(ch),
		questeq->short_description,
		obj_index[questeq->item_number].virtual);
	mudlog(buf, NRM, LEVEL_DEITY, TRUE);
      } else
	send_to_char("You don't have enough quest points!\r\n", ch);
    } else
      send_to_char("That item does not exist!\r\n", ch);

    return (TRUE);
  }

  if (CMD_IS("sell")) {
    one_argument(arg, buf);

    if (!is_number(buf)) {
      send_to_char ("You can trade-in the following Quest Items:\r\n", ch);
      for (questeq = ch->carrying; questeq; questeq = questeq->next_content)
	for (i = 1; i <= QUESTEQ_TRADE_MAX; i++)
	  if (questeq_tradein[i-1][0] == obj_index[questeq->item_number].virtual) {
	    sprintf(buf, "%3d. %s\r\n", i, questeq->short_description);
	    send_to_char(buf, ch);
	  }
      return (TRUE);
    } else
      num = atoi(buf);

    /* do the trade-in */
    if ((num < 1) || (num > QUESTEQ_TRADE_MAX)) {
      send_to_char("Invalid Choice\r\n", ch);
      return(TRUE);
    }

    for (questeq = ch->carrying; questeq && !i; questeq = questeq->next_content)
	if (questeq_tradein[num-1][0] == obj_index[questeq->item_number].virtual) {
	  sprintf(buf, "%s traded-in %s [%d] for [%d]",
		  GET_NAME(ch),
		  questeq->short_description,
		  obj_index[questeq->item_number].virtual,
		  questeq_tradein[num-1][1]);
	  mudlog(buf, NRM, LEVEL_DEITY, TRUE);
	  sprintf(buf, "You traded in your %s.\r\n",
		  questeq->short_description);
	  send_to_char(buf, ch);

	  extract_obj(questeq);
	  newobj = read_object(real_object(questeq_tradein[num-1][1]), REAL);
	  obj_to_char(newobj, ch);
	  return (TRUE);
	}

    send_to_char("Item not found.\r\n", ch);
    return (TRUE);
  }

  if (CMD_IS("help")) {
    send_to_char("Once you have accumulated enough quest points you\r\n", ch);
    send_to_char("may place your purchase by the following command:\r\n", ch);
    send_to_char("buy <number>\r\n", ch);
    send_to_char("<number> is the number of the item you want to buy\r\n", ch);
    send_to_char("\r\nIf you would like to trade-in old questeq, use:\r\n", ch);
    send_to_char("sell <number>\r\n", ch);
    send_to_char("sell w/o <number> will list all items in your inventory\r\n", ch);
    send_to_char("that you can trade back.  NOTE: items must be in inventory\r\n", ch);
    send_to_char("NOTE 2: Your Oedit WILL be lost, contact an immortal\r\n", ch);
    send_to_char("if you wish to keep your oedit.\r\n", ch);
    return (TRUE);
  }

  /* all other commands */
  return (FALSE);
}


SPECIAL(claneq_shopkeeper)
{
  char buf[MAX_STRING_LENGTH], buf2[SMALL_BUFSIZE];
  struct obj_data *claneq, *newobj, *oldobj;
  int num = 0, i = 0, j;
  bool found = FALSE;
  extern struct obj_data *claneq_list;
  extern struct obj_data *obj_proto;
  extern struct str_app_type str_app[];

  *buf = '\0';

  if (IS_NPC(ch)) return(FALSE);

  if (!claneq_list) return(FALSE);

  if (CMD_IS("list")) { /* List */

    if (CLAN_LEVEL(ch) <= 1) {
      send_to_char("You need to be in a clan to buy clan items.\r\n", ch);
      return(TRUE);
    }

    sprintf(buf, "Number  %-40s %s  %s\r\n", "Name:", "Level", "Exchange");
    for (claneq = claneq_list; claneq; claneq = claneq->next) {
	 if (CLANEQ_CLAN(claneq) == CLAN(ch) &&
	     (CLANEQ_EX(claneq) != -1)) {
	      num++;
	      i = real_object(CLANEQ_EX(claneq));
	      sprintf(buf2, "%3d.  %40s  [%4d]  %s\r\n",
		      num,
		      claneq->short_description,
		      claneq->obj_flags.level,
		      (i > 0) ? obj_proto[i].short_description : "N/A");

	      strcat(buf, buf2);
	 }
    }
    page_string(ch->desc, buf, TRUE);
    return (TRUE);
  }

  if (CMD_IS("exchange")) {

    if (CLAN_LEVEL(ch) <= 1) {
      send_to_char("You need to be in a clan to buy clan items.\r\n", ch);
      return(TRUE);
    }

    one_argument(arg, buf);

    if (!is_number(buf)) {
      send_to_char ("Place your purchase using the item number.\r\n", ch);
      send_to_char ("Place the item to be exchanged in the room.\r\n", ch);
      return (TRUE);
    } else
      num = atoi(buf);

    for (claneq = claneq_list; claneq; claneq = claneq->next) {
	 if (CLANEQ_CLAN(claneq) == CLAN(ch) &&
	     (CLANEQ_EX(claneq) != -1))
	      i++;
      if (i == num) break;
    }

    if (claneq) {
      if ( (j = real_object(CLANEQ_EX(claneq))) < 0) {
	send_to_char("You can't exchange for that item.\r\n", ch);
	return(TRUE);
    /* Bodpoint - Commented out so that spec will work
      } else {
	send_to_char("That item doesn't exist...\r\n", ch);
	return(TRUE);
     */
      }
    }

    if (i == num) {
      if (GET_LEVEL(ch) < claneq->obj_flags.level) {
	send_to_char("You can't exchange for an item above your level!\r\n", ch);
	return (TRUE);
      }

      if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
	send_to_char("You can't carry that many items!\r\n", ch);
	return (TRUE);
      }

      if ((IS_CARRYING_W(ch) + claneq->obj_flags.weight) > CAN_CARRY_W(ch)) {
	send_to_char("You can't carry that much weight.\r\n",  ch);
	return (TRUE);
      }

      for (oldobj = world[ch->in_room]->contents; oldobj && !found; ) {
	   if (oldobj->item_number == j) {
		found = TRUE;
	   } else
		oldobj = oldobj->next_content;
      }

      if (found) {
	newobj = read_object(claneq->item_number, REAL);
	obj_to_char(newobj, ch);
	extract_obj(oldobj);
	send_to_char("Exchanged!\r\n", ch);
      } else {
        send_to_char("Place the item to be exchanged on the ground.\r\n", ch);
      }
    }

    return (TRUE);
  }

  if (CMD_IS("help")) {
    send_to_char("Once you have obtained the exchange item you\r\n", ch);
    send_to_char("may exchange by using the following command:\r\n", ch);
    send_to_char("exchange <number>\r\n", ch);
    send_to_char("<number> is the number of the item you want to buy\r\n", ch);
    send_to_char("Be sure to place the exchange item on the ground.\r\n", ch);
    return (TRUE);
  }

  /* all other commands */
  return (FALSE);
}


SPECIAL(pet_shops)
{
   char	buf[MAX_STRING_LENGTH], pet_name[256];
   int	pet_room;
   struct char_data *pet;

   pet_room = ch->in_room + 1;

   if (CMD_IS("list")) { /* List */
      send_to_char("Available pets are:\r\n", ch);
      for (pet = world[pet_room]->people; pet; pet = pet->next_in_room) {
	 sprintf(buf, "%8d - %s\r\n", 3 * GET_EXP(pet), pet->player.short_descr);
	 send_to_char(buf, ch);
      }
      return(TRUE);
   } else if (CMD_IS("buy")) { /* Buy */

      arg = one_argument(arg, buf);
      arg = one_argument(arg, pet_name);
      /* Pet_Name is for later use when I feel like it */

      if (!(pet = get_char_room(buf, pet_room))) {
	 send_to_char("There is no such pet!\r\n", ch);
	 return(TRUE);
      }

      if (GET_GOLD(ch) < (GET_EXP(pet) * 3)) {
	 send_to_char("You don't have enough gold!\r\n", ch);
	 return(TRUE);
      }

      if (!allow_follower(ch)) {
	send_to_char("You can't have any more followers.\r\n", ch);
	return(TRUE);
      }

      GET_GOLD(ch) -= GET_EXP(pet) * 3;

      pet = read_mobile(pet->nr, REAL);
      GET_EXP(pet) = 0;
      SET_BIT(pet->specials.affected_by, AFF_CHARM);

      if (*pet_name) {
	 sprintf(buf, "%s %s _%s_", pet->player.name, pet_name, GET_NAME(ch));
	 /* free(pet->player.name); don't free the prototype! */
	 pet->player.name = strdup(buf);

	 sprintf(buf, "%sA small sign on a chain around the neck says 'My Name is %s'\r\n",
	     pet->player.description, pet_name);
	 /* free(pet->player.description); don't free the prototype! */
	 pet->player.description = strdup(buf);
      } else {
	sprintf(buf, "%s _%s_", pet->player.name, GET_NAME(ch));
	pet->player.name = strdup(buf);
      }

      char_to_room(pet, ch->in_room);
      add_follower(pet, ch);

      /* Be certain that pet's can't get/carry/use/weild/wear items */
      IS_CARRYING_W(pet) = 1000;
      IS_CARRYING_N(pet) = 100;

      send_to_char("May you enjoy your pet.\r\n", ch);
      act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

      return(TRUE);
   }

   /* All commands except list and buy */
   return(FALSE);
}


/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

SPECIAL(pray_for_items)
{
   char	buf[256];
   int	key_room, gold;
   bool found;
   struct obj_data *tmp_obj, *object;
   struct extra_descr_data *ext;

   if (!CMD_IS("pray")) /* You must pray to get the stuff */
      return FALSE;

   key_room = 1 + ch->in_room;

   strcpy(buf, "item_for_");
   strcat(buf, GET_NAME(ch));

   gold = 0;
   found = FALSE;

   for (tmp_obj = world[key_room]->contents; tmp_obj; tmp_obj = tmp_obj->next_content)
      for (ext = tmp_obj->ex_description; ext; ext = ext->next)
	 if (str_cmp(buf, ext->keyword) == 0) {
	    if (gold == 0) {
	       gold = 1;
	       act("$n kneels and at the altar and chants a prayer to Odin.",
	           FALSE, ch, 0, 0, TO_ROOM);
	       act("You notice a faint light in Odin's eye.",
	           FALSE, ch, 0, 0, TO_CHAR);
	    }
	    object = read_object(tmp_obj->item_number, REAL);
	    obj_to_room(object, ch->in_room);
	    act("$p slowly fades into existence.", FALSE, ch, object, 0, TO_ROOM);
	    act("$p slowly fades into existence.", FALSE, ch, object, 0, TO_CHAR);
	    gold += object->obj_flags.cost;
	    found = TRUE;
	 }


   if (found) {
      GET_GOLD(ch) -= gold;
      GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
      return TRUE;
   }

   return FALSE;
}



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
  int amount;

  /* Skip eventual spaces */
  for (; isspace(*arg); arg++);

  if (is_abbrev(arg, "all"))
  {
    if (CMD_IS("deposit"))
    {
      if (GET_GOLD(ch))
        amount = GET_GOLD(ch);
      else
      {
        send_to_char("You don't have anything to deposit.\r\n", ch);
        return 1;
      }
    }
    else
    {
      if (CMD_IS("withdraw"))
      {
        if (GET_BANK_GOLD(ch))
          amount = GET_BANK_GOLD(ch);
        else
        {
          send_to_char("You don't have anything to withdraw.\r\n", ch);
          return 1;
        }
      }
    }
  }
  else
    amount = atoi(arg);

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %d coins.\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if (amount <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    if ((GET_BANK_GOLD(ch) + amount) > max_bank_gold) {
	  // Make it deposit max possible - Charlene
	  amount = max_bank_gold-GET_BANK_GOLD(ch);
	  GET_GOLD(ch) -= amount;
	  GET_BANK_GOLD(ch) += amount;
      sprintf(buf, "The bank won't accept more than %d gold.\r\n"
                   "You deposit %d coins.\r\n",
                max_bank_gold, amount);
      send_to_char(buf, ch);
      act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if (amount <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else
    return 0;
}

SPECIAL(rigelskey)
{
    if(!CMD_IS("unlock"))
	return(0);

    if(str_cmp(ch->player.name,"rigel"))
    {
	send_to_char("The key says: Naah You are not Rigel..GO AWAY!!"
		     , ch);
	act("$n tries to unlock a door, but the key say: 'Naah You are not Rigel..GO AWAY!!'", TRUE,ch,0,0,TO_ROOM);
    }
     else
	 return(0);

    return(1);
}

static  char  *drunk_strings[] =  {
    "$n says 'Let's drink to your health!'",
    "$n pukes...  Yuck!",
    "$n sings a bawdy song...",
    "$n hiccups... *HIC*",
    "$n stares at the sky... He just fell.",
    "$n asks 'Why aren't you drinking?'",
    "$n begs you for a drink.",
    "$n smiles happily.",
    "$n stumbles around looking for somrthing.",
    "$n wonders where the toilet is."
    };


SPECIAL(drunk)
{
    ACMD(do_drink);

    if (cmd || !AWAKE(mob))
       return(0);

    switch (number(0, 10)) {
    case 0:
    case 1:
	act(drunk_strings[number(0, 9)], FALSE, mob, 0, 0, TO_ROOM);
	return(1);
    case 2:
	do_drink(mob, "vodka", 0, SCMD_DRINK);
	return(1);
    default:
	return(0);
    }
}


static  char  *priest_strings[] =  {
    "$n says 'Live in the way of the Gods!'",
    "$n is blessed by the Gods.",
    "$n looks around for customers.",
    "$n says 'Buy CURE from me in the name of the Gods.  Only 5000 coins!'",
    "$n says 'Buy RESTORE from me in the name of the Gods. Only 1000000 coins!'",
    "$n says 'speak 'list' and find out how I may help you!'",
    "$n looks compassionately at the unfaithful godless mortals."
};


SPECIAL(priest)
{
    if (!AWAKE(ch) || !AWAKE(mob))
	return(0);

    if (CMD_IS("list")) {
      send_to_char("Command Cost(GP)     Effect     \r\n", ch);
      send_to_char("======= ======== ==============\r\n", ch);
      send_to_char("  cure     5000   Cure all 3  \r\n", ch);
      send_to_char("restore 1000000   Full Hp/Mn/Mv\r\n", ch);
      send_to_char("To buy effect, type:\r\n", ch);
      send_to_char("buy <Command>\r\n", ch);
      send_to_char("Example: buy cure\r\n", ch);
      return(TRUE);
    }

    if (CMD_IS("buy")) {
	arg = one_argument(arg, buf);

	if (!strncmp(buf, "cure", 4)) {
	    if (GET_GOLD(ch) > 5000) {
		GET_GOLD(ch) -= 5000;
		if (affected_by_spell(ch, SPELL_POISON))
		    affect_from_char(ch, SPELL_POISON);
		if (affected_by_spell(ch, SPELL_CURSE))
		    affect_from_char(ch, SPELL_CURSE);
                if (affected_by_spell(ch, SPELL_BLINDNESS))
                    affect_from_char(ch, SPELL_BLINDNESS);
		send_to_char("You have been cured by the Priest!\r\n", ch);
		act("$n has been cured by the Priest.", TRUE, ch, 0, 0, TO_ROOM);
	    }
	} else if (!strncmp(buf, "restore", 7)){
	    if (GET_GOLD(ch) > 1000000){
		GET_GOLD(ch) -= 1000000;
		GET_HIT(ch) = GET_MAX_HIT(ch);
		GET_MANA(ch) = GET_MAX_MANA(ch);
		GET_MOVE(ch) = GET_MAX_MOVE(ch);
		send_to_char("You have been fully restored by the priest !!\r\n", ch);
		act("$n has been fully restored by the priest.", TRUE, ch, 0,0, TO_ROOM);
	    }
	    else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\r\n", ch);
	    }
	} else {
	    send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\r\n", ch);
	}

	return(TRUE);
    }

    if (!cmd) {
	if (!number(0, 15)) {
	    act(priest_strings[number(0,7)], FALSE, mob, 0, 0, TO_ROOM);
	    return(1);
	}
    }

    return(FALSE);
}


static  char  *undertaker_strings[] =  {
    "$n shouts 'Bring out your dead!'",
    "$n says 'The monster is comming!  He is comming for you.'",
    "$n yells 'Beware!  Judgementday is near!'",
    "$n looks around for customers.",
    "$n says 'Coffins in all sizes.'",
    "$n says 'Let me bury you when you die.'",
    "$n shouts 'Bring out your dead!'"
    };

SPECIAL(undertaker)
{
    struct char_data *tch;

    if (cmd || !AWAKE(mob))
	return(0);

    switch (number(0, 6)) {
    case 0:
	act(undertaker_strings[number(0,6)], FALSE, mob, 0, 0, TO_ROOM);
	break;
    case 1:
	for (tch = world[mob->in_room]->people; tch; tch = tch->next_in_room) {
	    if (GET_HIT(tch) < GET_MAX_HIT(tch)/5) {
		act("$n measures $N up for a coffin.", TRUE, mob, 0, tch, TO_NOTVICT);
		act("$n measures you up for a coffin.", TRUE, mob, 0, tch, TO_VICT);
		return(TRUE);
	    }
	}
	break;
    }

    return(FALSE);
}


SPECIAL(newbie_only)
{
    if (!CMD_IS("north"))
	return FALSE;

    if (GET_LEVEL(ch) > 1 && GET_LEVEL(ch) < LEVEL_DEITY) {
	send_to_char("You are not a newbie anymore.\r\n", ch);
	return TRUE;
    }

    return FALSE;
}


ACMD(do_action);

SPECIAL(social_object)
{
  static struct char_data social_character;

  social_character.player.name = obj->name;
  social_character.player.short_descr = obj->short_description;
  GET_SEX(&social_character) = SEX_NEUTRAL;
  GET_POS(&social_character) = POS_STANDING;
  SET_BIT(MOB_FLAGS(&social_character), MOB_ISNPC);
  social_character.nr = -1;

  if (!cmd || IS_NPC(ch))
    return FALSE;

  if (cmd_info[cmd].command_pointer != do_action)
    return FALSE;

  if (GET_POS(ch) < cmd_info[cmd].minimum_position)
    return FALSE;

  skip_spaces(&arg);

  if (*arg && isname(arg, social_character.player.name)) {
    char_to_room(&social_character, IN_ROOM(ch));
    do_action(ch, arg, cmd, 0);
    char_from_room(&social_character);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(spraycan)
{
  struct obj_data *graffiti;

  if (!CMD_IS("use"))
    return FALSE;

  skip_spaces(&arg);

  if (*arg && isname(arg, obj->name)) {
    if (ch->equipment[HOLD] != obj)
      return FALSE;

    CREATE(graffiti, struct obj_data, 1);
    clear_object(graffiti);

    graffiti->item_number = NOWHERE;
    graffiti->in_room = NOWHERE;

    graffiti->description = strdup("Some vandal has sprayed a graffiti here.");
    graffiti->short_description = strdup("a graffiti");
    graffiti->obj_flags.type_flag = ITEM_NOTE;
    graffiti->obj_flags.wear_flags = 0;
    graffiti->obj_flags.extra_flags = ITEM_NODONATE | ITEM_NODROP;
    graffiti->obj_flags.weight = 0;
    graffiti->obj_flags.cost_per_day = 100000;
    graffiti->obj_flags.cost = 0;
    sprintf(buf, "graffiti _%s_", GET_NAME(ch));
    graffiti->name = strdup(buf);

    graffiti->next = object_list;
    object_list = graffiti;

    obj_to_room(graffiti, ch->in_room);

    send_to_char("Ok.. go ahead and make art.. (/s to end /h for help with your graffiti)\r\n", ch);
    act("$n begins to spray a graffiti.", TRUE, ch, 0, 0, TO_ROOM);
    ch->desc->str = &graffiti->action_description;
    ch->desc->max_str = 128;

    extract_obj(unequip_char(ch, HOLD));

    return TRUE;
  }

  return FALSE;
}


SPECIAL(guard_object)
{
  long class = 0;

  /* Check if obj is in the intended room and that DIR is forbidden */
  if ((!GET_ITEM_VALUE(obj, 0) ||
       IN_ROOM(ch) == real_room(GET_ITEM_VALUE(obj, 0))) &&
      IS_MOVE(cmd) && IS_SET(GET_ITEM_VALUE(obj, 1), (1 << (cmd-1)))) {

    /* Check clan first */
    if (GET_ITEM_VALUE(obj, 2) && CLAN(ch) != real_clan(GET_ITEM_VALUE(obj, 2))) {
      send_to_char("You cannot go into other's clanhouses.\r\n", ch);
      return TRUE;
      /* clan level */
    } else if (GET_ITEM_VALUE(obj, 3) &&
	       CLAN_LEVEL(ch) < GET_ITEM_VALUE(obj, 3)) {
      send_to_char("Your clan rank is too low to go in there.\r\n", ch);
      return TRUE;
      /* min level */
    } else if (GET_LEVEL(ch) < GET_ITEM_VALUE(obj, 4)) {
      send_to_char("Your level is too low to go in there.\r\n", ch);
      return TRUE;
      /* max level */
    } else if (GET_LEVEL(ch) > GET_ITEM_VALUE(obj, 5)) {
      send_to_char("Your level is too high to go in there.\r\n", ch);
      return TRUE;
      /* check anticlass */
    } else {
      /* New anti-class check _Petrus */
      if (IS_MULTI(ch) || IS_DUAL(ch)) {
	SET_BIT(class, (1 << (GET_1CLASS(ch) - 1)));
	SET_BIT(class, (1 << (GET_2CLASS(ch) - 1)));
	if (IS_3MULTI(ch))
	  SET_BIT(class, (1 << (GET_3CLASS(ch) - 1)));
      } else
	SET_BIT(class, (1 << (GET_CLASS(ch) - 1)));

      if (!IS_NPC(ch) && ((GET_ITEM_ANTICLASS(obj) & class) == class)) {
	send_to_char("You are of the wrong profession to enter there.\r\n", ch);
	return TRUE;
      }
    }

    /* add fee check too -P */

  }

  return FALSE;
}

/* ZONE 131 Special Procedures */

SPECIAL(mob_block)
{

  if (!IS_MOVE(cmd) || !AWAKE(mob))
    return FALSE;

  if (((IN_ROOM(ch) == real_room(13194)) && CMD_IS("east")) ||
      ((IN_ROOM(ch) == real_room(13193)) && CMD_IS("west")) ||
      ((IN_ROOM(ch) == real_room(16986)) && CMD_IS("west")) ||
      ((IN_ROOM(ch) == real_room(16987)) && CMD_IS("east"))) {

    act("$N blocks your way.", FALSE, ch, 0, mob, TO_CHAR);
    act("$N blocks $s way.", FALSE, ch, 0, mob, TO_ROOM);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(tithe_cleric)
{
     int tithe = 0, amt = 0;

     if (!AWAKE(ch) || !AWAKE(mob))
         return(0);

     if (CMD_IS("list"))
     {
       send_to_char("#wSpecial:\r\n#N", ch);
       send_to_char("Buy tithe\r\n", ch);
       return shop_keeper(ch,mob,obj,cmd,arg);
     }

     if (CMD_IS("buy"))
     {
       arg = one_argument(arg, buf);

       if (!strncmp(buf, "tithe", 5))
       {
         if (GET_GOLD(ch) >= 100000)
         {
             tithe = GET_GOLD(ch) / 10;
             amt = GET_BANK_GOLD(ch) / 100;
             GET_GOLD(ch) -=tithe;
             GET_BANK_GOLD(ch) -=amt;
             send_to_char("A blessed feeling fills you as you give your "
                          "tithe to the Cleric!\r\n", ch);
            return (TRUE);
         }
         else
         {
           send_to_char("The community needs the money - bring more "
                        "before you tithe next time.\r\n", ch);
          return(TRUE);
         }
       }
       return shop_keeper(ch,mob,obj,cmd,buf);
     }
     return shop_keeper(ch,mob,obj,cmd,arg);
}


SPECIAL(goto_mayor)
{
   if (!cmd)
   {
   char in_buf[MAX_INPUT_LENGTH];
   int goto_rooms[6] = { 13225, 13241, 13246, 13261, 13277, 13280 };
   int i = 0, j = 0;
   int location;
   static int cmd_emote;
   static int cmd_say;
   static int cmd_sigh;
   static int cmd_whine;
   static int cmd_yell;
   while (cmd_emote < 1)
     cmd_emote = find_command("emote");
   while (cmd_say < 1)
     cmd_say = find_command("say");
   while (cmd_sigh < 1)
     cmd_sigh = find_command("sigh");
   while (cmd_whine < 1)
     cmd_whine = find_command("whine");
   while (cmd_yell < 1)
     cmd_yell = find_command("yell");
   char  *move_messages[][2] = {
     { "The Mayor suddenly is no more.",
       "Suddenly the Mayor is right next to you." },
     { "POFF!!!",
       "POFF!!!" },
     { "A flaming rift opens up digesting the Mayor fully.",
       "Flames erupt out of nowhere leaving a slightly disoriented Mayor behind." },
     { "There is a smoke remnant where the Mayor once stood.",
       "Stepping out of a smokescreen, that was not there a second ago, "
       "is the local Mayor." },
     { "You hear a high pitched screech as the world is torn apart.",
       "Without any sound comes the Mayor walking from nowhere." },
     { "A thin bluish line passes through the room and when it is gone, "
       "so is the Mayor.",
       "A thin bluish line passes through the room and when it is gone, "
       "there is the Mayor." },
     { "You try to flee but a transspatial interference has already "
       "passed by catching the Mayor.",
       "The world goes insane, shifting in colour and dimension. When it "
       "stabilises, you have gained a Mayor." },
     { "A faint murmur, not from your belly this time, grows more tense "
       "erupting in a roar swallowing the Mayor.",
       "You burp, unwillingly, and out jumps a slightly moist Mayor." },
     { "Suddenly a flash of light almost blinds you.",
       "The world grows dark as if all lanterns suddenly stopped illuminating." },
     { "A soft female voice says 'It is time'",
       "A soft female voice says 'There you go'" },
     { "The ground leaps up and engulfs the Mayor in one piece.",
       "With an annoyed grumble the Mayor comes falling from above." },
     { "As a reversed ATM machine withdrawal the Mayor is sucked away.",
       "You hear a slight 'ka-sching' and from nowhere the Mayor appears." },
     { "Gone, as a paycheque on pay-day, is the Mayor.",
       "Securely present, as a hangover the day after, is the Mayor." },
     { "The Mayor is disintegrated before your very eyes.",
       "The air starts to shimmer and from all the facettes a sturdy "
       "little dwarf manifests." },
     { "A small hole no larger than a single gold coin appears and drags "
       "the Mayor into it.",
       "What could have been an insect suddenly erupts disclosing a "
       "fully living Mayor." }
   };

   // Random Dice Roll to see if moving
   if (number(0, 8)/8) {

     if (FIGHTING(mob))
         stop_fighting(mob);

     // Random Dice Roll to pick location
     i = number(0, 5);
     location = real_room(goto_rooms[i]);
     while (IN_ROOM(mob)== real_room(goto_rooms[i]))
     {
         i = number(0, 5);
         location = real_room(goto_rooms[i]);
     }

     if (location > 0 || location < top_of_world) {


       // Random Dice Roll to pick paired exit / entry messages
       j = number(0, 14);


       // Outbound Message
       switch (number(0, 18)) {

       case 0:
         do_say(mob, "Oh No!", cmd_say, 0);
         do_say(mob, "It is happening again.", cmd_say, 0);
         break;
       case 1:
         do_say(mob, "LOOK OUT!  Behind you!  The rift is coming.", cmd_say, 0);
         break;
       case 2:
         do_say(mob, "Excuse me.", cmd_say, 0);
         do_say(mob, "Yes you.", cmd_say, 0);
         do_emote(mob, "points at you.", cmd_emote, 0);
         do_say(mob, "Do you know anyhing about the space and time continuum?", cmd_say, 0);
         do_say(mob, "Thought so.", cmd_say, 0);
         break;
       case 3:
         do_say(mob, "Wish I could bring you along.", cmd_say, 0);
         break;
       case 4:
         do_say(mob, "Why does this have to happen to me.", cmd_say, 0);
         break;
       case 5:
         do_say(mob, "Can you help me out.", cmd_say, 0);
         do_say(mob, "I would really like to have some help.", cmd_say, 0);
         break;
       case 6:
         do_say(mob, "Wonder if I may set the destination, for once.", cmd_say, 0);
         break;
       case 7:
         do_say(mob, "Wonder if I may set the arrival date, for once.", cmd_say, 0);
         break;
       case 8:
         do_say(mob, "Could you hold this for me?", cmd_say, 0);
         do_say(mob, "Asch, you already carry too much.", cmd_say, 0);
         do_say(mob, "Too late.", cmd_say, 0);
         break;
       case 9:
         do_say(mob, "Hurray, for once I am glad it come. ", cmd_say, 0);
         do_say(mob, "Now I do not have to look at you for a while.", cmd_say, 0);
         break;
       case 10:
         do_say(mob, "Last time there was some beautiful longbearded "
                     "female dwarves where I ended up.", cmd_say, 0);
         do_say(mob, "Probably will not end up there again in a long "
                     "time...", cmd_say, 0);
         do_action(mob, 0, cmd_sigh, 0);
         break;
       case 11:
         do_say(mob, "I will kill that Glob!", cmd_say, 0);
         do_say(mob, "He did this to me you know.", cmd_say, 0);
         break;
       case 12:
         do_say(mob, "Beware of the Deep Gnomes!", cmd_say, 0);
         do_say(mob, "They are a really bad element of this town.", cmd_say, 0);
         do_say(mob, "I am glad they keep themselves to the outskirts.", cmd_say, 0);
         break;
       case 13:
         do_say(mob, "Ah, silence...", cmd_say, 0);
         break;
       case 14:
         do_emote(mob,"mutters quietly to Himself.", cmd_emote, 0);
         break;
       case 15:
         do_emote(mob, "goes all wobbly and wiggly all over his body. "
                       "Scary.", cmd_emote, 0);
         break;
       case 16:
         do_say(mob, "Try to catch me if you can.", cmd_say, 0);
         break;
       case 17:
         do_say(mob, "Sorry visitors.", cmd_say, 0);
         do_say(mob, "This is a private transportation.", cmd_say, 0);
         break;
       case 18:
         do_say(mob, "Did you remember to vote in the city elections?", cmd_say, 0);
         break;

       default:
         break;
       }

       // Do Move
       sprintf(in_buf, "%s\r\n", move_messages[j][0]);
       act(in_buf, FALSE, mob, 0, 0, TO_ROOM);

       char_from_room(mob);
       char_to_room(mob, location);

       sprintf(in_buf, "%s\r\n", move_messages[j][1]);
       act(in_buf, FALSE, mob, 0, 0, TO_ROOM);

       // Inbound Message
       switch (number(0, 19)) {

       case 0:
         do_say(mob, "Hello!", cmd_say, 0);
         break;
       case 1:
         do_say(mob, "Where did YOU come from?", cmd_say, 0);
         break;
       case 2:
         do_say(mob, "I want my longbearded dwarfesses back!", cmd_say, 0);
         do_action(mob, 0, cmd_whine, 0);
         break;
       case 3:
         do_say(mob, "Did you remember to vote in the city elections?", cmd_say, 0);
         break;
       case 4:
         do_say(mob, "What time is it?", cmd_say, 0);
         break;
       case 5:
         do_say(mob, "Where Am I?", cmd_say, 0);
         break;
       case 6:
         do_say(mob, "Asch, not alone this time either...", cmd_say, 0);
         break;
       case 7:
         do_emote(mob, "tosses away some leftover part of whoever was "
                       "close to him earlier.", cmd_emote, 0);
         break;
       case 8:
         do_emote(mob, "looks at the fizzling rift closing temporarely.", cmd_emote, 0);
         break;
       case 9:
         do_emote(mob, "yells, 'Thank you Bod, for bringing me back.'",
                       cmd_emote, 0);
         break;
       case 10:
         do_say(mob, "There is no place like home.", cmd_say, 0);
         break;
       case 11:
         do_say(mob, "Home sweet, harbour town.", cmd_say, 0);
         break;
       case 12:
         do_say(mob, "Who are you?", cmd_say, 0);
         break;
       case 13:
         do_say(mob, "What are you doing here?", cmd_say, 0);
         break;
       case 14:
         do_say(mob, "You can not imagine where I have been.", cmd_say, 0);
         break;
       case 15:
         do_say(mob, "I am stil on the mainland, am I not?", cmd_say, 0);
         do_say(mob, "Cause I always imagined the Northern Continent to "
                     "be much more different.", cmd_say, 0);
         break;
       case 16:
         do_emote(mob,"shudders violently.", cmd_emote, 0);
         do_emote(mob, "pukes {on your clothes.", cmd_emote, 0);
         do_emote(mob, "sways unsteadily, and falls into your arms.",
                       cmd_emote, 0);
         do_say(mob, "Rough ride, I say.", cmd_say, 0);
         break;
       case 17:
         do_gen_com(mob, "Yiikes...", cmd_yell, SCMD_YELL);
         break;
       case 18:
         do_say(mob, "Hmmm, I need a boozer.", cmd_say, 0);
         break;
       case 19:
         do_say(mob, "Feels like I have bought myself a full treatment "
                     "at Fritz the Slaughterer", cmd_say, 0);
         break;

       default:
         break;
       }
     }
   }
   }
   return(FALSE);
}



void display_caller_calling(struct char_data *mob)
{
   static int cmd_emote;
   static int cmd_say;
   while (cmd_emote < 1)
     cmd_emote = find_command("emote");
   while (cmd_say < 1)
     cmd_say = find_command("say");


   switch (number(1, 20))
     {
     case 1:
     case 2:
     case 3:
     case 4:
     case 5:
         do_emote(mob,"yells, 'GUARDS!!!'", cmd_emote, 0);
         break;
     case 6:
         do_say(mob, "How come you dared this far from your parental woom.", cmd_say, 0);
         break;
     case 7:
         do_say(mob, "Meet a friend of mine.", cmd_say, 0);
         break;
     case 8:
         do_say(mob, "I got someone for you.", cmd_say, 0);
         break;
     case 9:
         do_say(mob, "There is someone you really need to meet.", cmd_say, 0);
         break;
     case 10:
         do_say(mob, "You are boring me.", cmd_say, 0);
         break;
     case 11:
         do_say(mob, "Why don't you bugger someone else.", cmd_say, 0);
         break;
     case 12:
         do_say(mob, "You ought to practise some more. Here, let me give "
                     "you some tutoring.", cmd_say, 0);
         break;
     case 13:
         do_say(mob, "I have more urgent matters to attend, let me call "
                     "upon a standin.", cmd_say, 0);
         break;
     case 14:
         do_say(mob, "Nice trick. I have a trick too.", cmd_say, 0);
         break;
     case 15:
         do_say(mob, "Smooth movement. Want to see a smooth entry?", cmd_say, 0);
         break;
     case 16:
         do_say(mob, "Getting shaky yet? Want to feel some earthquake.", cmd_say, 0);
         break;
     case 17:
         do_say(mob, "Your legs feeling like spagetti? I get someone to "
                     "check if it's al dente.",  cmd_say, 0);
         break;
     case 18:
         do_say(mob, "You look at bit saggy. Let me bring you a masseur.", cmd_say, 0);
         break;
     case 19:
         do_say(mob, "By the way, have you met our people from the "
                     "reclamation department?", cmd_say, 0);
         break;
     case 20:
         do_say(mob, "I feel more guests coming, your way.", cmd_say, 0);
         break;

     default:
         break;
     }

}

void display_helper_entering(struct char_data *mob)
{
   static int cmd_emote;
   static int cmd_say;
   while (cmd_emote < 1)
     cmd_emote = find_command("emote");
   while (cmd_say < 1)
     cmd_say = find_command("say");

   switch (number(1, 20))
     {
     case 1:
     case 2:
     case 3:
     case 4:
     case 5:
         do_emote(mob,"yells, 'CHARGE!!!'", cmd_emote, 0);
         break;
     case 6:
         do_say(mob, "You should have followed your mothers advice to stay at home.", cmd_say, 0);
         break;
     case 7:
         do_say(mob, "Oh, Party!", cmd_say, 0);
         break;
     case 8:
         do_say(mob, "Hi! What are you doing ?", cmd_say, 0);
         break;
     case 9:
         do_say(mob, "I have been dying to meet you. Have you been dying too?", cmd_say, 0);
         break;
     case 10:
         do_say(mob, "Now, lets spice things up a bit.", cmd_say, 0);
         break;
     case 11:
         do_say(mob, "You rang, Sir.", cmd_say, 0);
         break;
     case 12:
         do_say(mob, "Tasted any real steel yet?", cmd_say, 0);
         break;
     case 13:
         do_say(mob, "Long live, Dwarven Kingdom and GlobInc. Joint ventures!", cmd_say, 0);
         break;
     case 14:
         do_say(mob, "Tadaaa!", cmd_say, 0);
         break;
     case 15:
         do_say(mob, "Look! Behind you!", cmd_say, 0);
         break;
     case 16:
         do_say(mob, "You look a bit shaky. Allow me to stabilize things.", cmd_say, 0);
         break;
     case 17:
         do_say(mob, "Is this tunnel 45B? The new pasta joint?", cmd_say, 0);
         break;
     case 18:
         do_say(mob, "Tendering coming straight up!", cmd_say, 0);
         break;
     case 19:
         do_say(mob, "Think this is messy? Then you ought to meet the purchase people.", cmd_say, 0);
         break;
     case 20:
         do_say(mob, "Only you? Lets invite some more friends.", cmd_say, 0);
         break;

     default:
         break;
     }
}

void guard_caller_move (struct char_data *mob,int vnum_mob,int can_call)
{
     struct char_data *victim;

     //Is there any mob free to be transfered ?
     for(victim=character_list;victim;victim=victim->next)
     {
         if (IS_NPC(victim) && !FIGHTING(victim) && victim->nr &&
             mob_index[victim->nr].virtual == vnum_mob &&
             IN_ROOM(mob)!=IN_ROOM(victim))
         {
             if (can_call) /*added this*/
                 can_call--;
             else
             {
               //help coming !!!
               display_caller_calling(mob);
               char_from_room(victim);
               char_to_room(victim,IN_ROOM(mob));
               display_helper_entering(victim);
               return;
             }
         }
     }
     return;
}

SPECIAL(guard_caller)
{
     int can_call,probability;
     int mob_call[2] ={ 13160, 13161};

     // Its a fighting function so no need to go thru function if not fighting
     if (GET_POS(mob) != POS_FIGHTING) return FALSE;

     // Seting up vars   TO BE CHANGED after testing (djmc)
     switch (mob_index[mob->nr].virtual)
     {
         case 13149:
             can_call=13;
             probability=10;
             break;
         case 13161:
             can_call=11;
             probability=10;
             break;
         case 13160:
             can_call=9;
             probability=10;
             break;
         case 13130:
             can_call=6;
             probability=10;
             break;
         case 13128:
             can_call=3;
             probability=10;
             break;
         case 13148:
             can_call=1;
             probability=10;
             break;
         default:
             log("guard_caller called by a not suported mob");
             return FALSE;
             break;
     }

     //mob calls for help depending on individual probability
     if (number(0,99)/probability) return FALSE;


     //What mob am I going to call
     if (mob_index[real_mobile(mob_call[0])].number > can_call)
     {
         if (mob_index[real_mobile(mob_call[1])].number > can_call)
           number(0,1)?guard_caller_move(mob,mob_call[0],can_call-1): \
                       guard_caller_move(mob,mob_call[1],can_call-1);
         else
             guard_caller_move(mob,mob_call[0],can_call-1);
     }
     else
         if (mob_index[real_mobile(mob_call[1])].number > can_call)
             guard_caller_move(mob,mob_call[1],can_call-1);
     return FALSE;
}


/* Generic Guild Master Special for Zone 169 */
SPECIAL(guildmaster)
{
    if (!CMD_IS("practice") || IS_NPC(ch) || !AWAKE(mob))
        return(FALSE);

    return guild(ch, mob, arg);
}



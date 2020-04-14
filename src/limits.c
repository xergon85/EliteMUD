/* ************************************************************************
*   File: limits.c                                      Part of EliteMUD  *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "functions.h"

/* Temp. solution.  Not GOOD   - Petrus */
#define READ_TITLE(ch, cl) (GET_SEX(ch) == SEX_MALE ?  \
        titles[cl - 1][(int)(GET_LEVEL(ch)/3)+1].title_m :  \
        titles[cl - 1][(int)(GET_LEVEL(ch)/3)+1].title_f)


extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;
extern struct clan_data *clan_list;
extern struct title_type titles[20][40];
extern char   *race_title_names[];
extern struct room_data **world;
extern int      str_max[];
extern int      con_max[];
extern int      dex_max[];
extern int      wis_max[];
extern int      int_max[];
extern int      cha_max[];


void improve_abilities(struct char_data *ch, int class)
{
    int str = 25, con = 25, dex = 25, intel = 25, wis = 25, cha = 25;
    bool stradd = FALSE;

    switch (class) {
    case CLASS_ILLUSIONIST:
    case CLASS_PSIONICIST:
    case CLASS_WIZARD:
    case CLASS_MAGIC_USER:
	intel = 0;
	wis = 0;
	break;
    case CLASS_MONK:
    case CLASS_PALADIN:
    case CLASS_DRUID:
    case CLASS_CLERIC:
	wis = 0;
	intel = 0;
	break;
    case CLASS_ASSASSIN:
    case CLASS_MARINER:
    case CLASS_BARD:
    case CLASS_THIEF:
    case CLASS_NINJA:
	dex = 0;
	str = 0;
	break;
    case CLASS_CAVALIER:
    case CLASS_RANGER:
    case CLASS_KNIGHT:
    case CLASS_WARRIOR:
        stradd = TRUE;
	str = 0;
	con = 0;
	break;
    }


    if (number(str, 100) < GET_LEVEL(ch) &&
	ch->abilities.str < str_max[GET_RACE(ch)] &&
	number(1, str_max[GET_RACE(ch)]) > ch->abilities.str) {
	ch->abilities.str++;
	send_to_char("Your strength has improved.\r\n", ch);
    }
    if (number(con, 100) < GET_LEVEL(ch) &&
	ch->abilities.con < con_max[GET_RACE(ch)] &&
	number(1, con_max[GET_RACE(ch)]) > ch->abilities.con) {
	ch->abilities.con++;
	send_to_char("Your constitution has improved.\r\n", ch);
    }
    if (number(dex, 100) < GET_LEVEL(ch) &&
	ch->abilities.dex < dex_max[GET_RACE(ch)] &&
	number(1, dex_max[GET_RACE(ch)]) > ch->abilities.dex) {
	ch->abilities.dex++;
	send_to_char("You are more dextrous.\r\n", ch);
    }
    if (number(intel, 100) < GET_LEVEL(ch) &&
	ch->abilities.intel < int_max[GET_RACE(ch)] &&
	number(1, int_max[GET_RACE(ch)]) > ch->abilities.intel) {
	ch->abilities.intel++;
	send_to_char("Your intelligence is improved.\r\n", ch);
    }
    if (number(wis, 100) < GET_LEVEL(ch) &&
	ch->abilities.wis < wis_max[GET_RACE(ch)] &&
	number(1, wis_max[GET_RACE(ch)]) > ch->abilities.wis) {
	ch->abilities.wis++;
	send_to_char("Your have gained more wisdom.\r\n", ch);
    }
    if (number(cha, 100) < GET_LEVEL(ch) &&
	ch->abilities.cha < cha_max[GET_RACE(ch)] &&
	number(1, cha_max[GET_RACE(ch)]) > ch->abilities.cha) {
	ch->abilities.cha++;
	send_to_char("Your personality has improved.\r\n", ch);
    }
    if (number(str, 100) < GET_LEVEL(ch) &&
	ch->abilities.str > 17 &&
	ch->abilities.str_add < 100 &&
	number(1, 100) > ch->abilities.str_add &&
	stradd) {
	ch->abilities.str_add += number(3, 7);
	send_to_char("Your strength grows.\r\n", ch);
    }

    affect_total(ch);
}


void set_lowest_level(struct char_data *ch, int newlevel)
{
    if (IS_NPC(ch))
	return;
    if (!IS_MULTI(ch) && !IS_DUAL(ch))
	GET_LEVEL(ch) = newlevel;
    else if (IS_3MULTI(ch) && GET_3LEVEL(ch)<GET_2LEVEL(ch))
	GET_3LEVEL(ch) = newlevel;
    else if (GET_2LEVEL(ch)<GET_1LEVEL(ch))
	GET_2LEVEL(ch) = newlevel;
    else {
	GET_1LEVEL(ch) = newlevel;
	GET_LEVEL(ch) = newlevel;
    }
}


int exp_needed(struct char_data *ch)
{
    int level, needed, diff, class;

    if (IS_NPC(ch)) {
      class = CLASS_WARRIOR - 1;     /* MObs will be Wa */
      level = GET_LEVEL(ch);
    } else {
      class = LOWEST_CLASS(ch)-1;
      level = LOWEST_LEVEL(ch);
    }

    if (level > LEVEL_DEITY-1)
	level = LEVEL_DEITY - 1;
    needed = titles[class][level/3+1].exp;
    diff = titles[class][level/3+2].exp - needed;
    needed += (diff/3*(level % 3));

    return needed;
}


/* manapoint gain pr. game hour */
int	mana_gain(struct char_data *ch)
{
    int	gain;

    if (IS_NPC(ch)) {
	/* Neat and fast */
	gain = 2 * GET_LEVEL(ch);
    } else {
	gain = GET_MAX_MANA(ch) * ch->specials.gain_count / (480 - 12 * GET_WIS(ch)) + GET_WIS(ch)/2;
/*	gain = GET_MAX_MANA(ch) * ch->specials.gain_count / 360 + GET_WIS(ch)/2;*/
    }

    if (IS_AFFECTED(ch, AFF_POISON))
	gain >>= 2;

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
	gain >>= 2;

    return (gain);
}


int	hit_gain(struct char_data *ch)
/* Hitpoint gain pr. game hour */
{
   int	gain;

   if (IS_NPC(ch)) {
      gain = 2 * GET_LEVEL(ch);
      /* Neat and fast */
   } else {
       gain = GET_MAX_HIT(ch) * ch->specials.gain_count / (480 - 12 * GET_CON(ch)) + GET_CON(ch)/2;
/*       gain = GET_MAX_HIT(ch) * ch->specials.gain_count / 360 + GET_CON(ch)/2; */
   }

   if (IS_AFFECTED(ch, AFF_REGENERATION) ||
       (IN_ROOM(ch) > NOWHERE && IS_SET(world[IN_ROOM(ch)]->room_flags, REGEN)))
     gain += gain/2;

   if (IS_AFFECTED(ch, AFF_POISON))
     gain >>= 2;

   if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
     gain >>= 2;

   return (gain);
}



int	move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
   int	gain;

   if (IS_NPC(ch)) {
      return(4 * GET_LEVEL(ch));
      /* Neat and fast */
   } else {
       gain = GET_MAX_MOVE(ch) * ch->specials.gain_count / (70 - GET_STR(ch)) + GET_STR(ch)/2;
   }

   if (IS_AFFECTED(ch, AFF_POISON))
      gain >>= 2;

   if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain >>= 2;

   return (gain);
}


/* Gain maximum in various points */
void	advance_level(struct char_data *ch, int log)
{
   int	add_hp, i, add_mana = 0, div;

   switch (GET_CLASS(ch)) {

   case CLASS_DUAL: div = 2; break;
   case CLASS_2MULTI: div = 2; break;
   case CLASS_3MULTI: div = 3; break;
   default: div = 1;
   }

   add_hp = number(1, 5);

   switch (LOWEST_CLASS(ch)) {

   case CLASS_MAGIC_USER :
   case CLASS_ILLUSIONIST:
   case CLASS_WIZARD:
   case CLASS_PSIONICIST:
       add_hp += GET_CON(ch)/5;
       add_mana = 10;
       break;

   case CLASS_CLERIC :
   case CLASS_MONK:
   case CLASS_PALADIN:
   case CLASS_DRUID:
       add_hp += GET_CON(ch)/4;
       add_mana = 10;
       break;

   case CLASS_THIEF :
   case CLASS_BARD:
   case CLASS_ASSASSIN:
   case CLASS_MARINER:
   case CLASS_NINJA:
       add_hp += GET_CON(ch)/3;
       add_mana = 5;
       break;

   case CLASS_WARRIOR :
   case CLASS_KNIGHT:
   case CLASS_RANGER:
   case CLASS_CAVALIER:
       add_hp += GET_CON(ch)/2;
       add_mana = 5;
       break;
   }

   ch->points.max_hit += MAX(1, add_hp / div);
   if ((GET_LEVEL(ch) != 1)) {
      ch->points.max_mana += MAX(1, add_mana/div);
   } /* if */
   ch->points.max_move += MAX(1, 3 / div );

   SPELLS_TO_LEARN(ch) += MAX(1, GET_WIS(ch)/3);
   SPELLS_TO_LEARN(ch) = MIN(SPELLS_TO_LEARN(ch), 100);

   if (GET_LEVEL(ch) >= LEVEL_DEITY)
       for (i = 0; i < 3; i++)
	   GET_COND(ch, i) = (char) -1;

   improve_abilities(ch, LOWEST_CLASS(ch));


   if (GET_LEVEL(ch) >= LEVEL_WORSHIP) {
       send_to_char("You can now be worshipped.\r\n", ch);
       WORSHIPS(ch) = 0;
   }

   save_char(ch, IN_VROOM(ch), 2);

   if (log) {
     sprintf(buf, "%s advanced to level %d", GET_NAME(ch), LOWEST_LEVEL(ch)+1);
     mudlog(buf, BRF, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
   }
}


void	set_title(struct char_data *ch)
{

  sprintf(buf, "the %s ", race_title_names[GET_RACE(ch)]);

  if (GET_CLASS(ch) < CLASS_DUAL) {
    strcat(buf, READ_TITLE(ch, GET_CLASS(ch)));
  }
  if (IS_DUAL(ch) || IS_MULTI(ch)) {
    strcat(buf, READ_TITLE(ch, GET_1CLASS(ch)));
    strcat(buf, "/");
    strcat(buf, READ_TITLE(ch, GET_2CLASS(ch)));
  }
  if (IS_3MULTI(ch)) {
    strcat(buf, "/");
    strcat(buf, READ_TITLE(ch, GET_3CLASS(ch)));
  }

  if (GET_TITLE(ch))
    RECREATE(GET_TITLE(ch), char, strlen(buf) + 1);
  else
    CREATE(GET_TITLE(ch), char, strlen(buf) + 1);
/*
    if(!(buf[0] == '\''))   {
        strcpy(temparg, buf);
        buf[0] = ' ';
        for(i = 1;i < strlen(buf) + 1;i++) {
                buf[i] = temparg[i-1];
        }
    }
  */
  strcpy(GET_TITLE(ch), buf);
}


void check_autowiz(struct char_data *ch)
{
  char	buf[100];
  extern int	use_autowiz;
  extern int	min_wizlist_lev;

  if (use_autowiz && GET_LEVEL(ch) >= LEVEL_DEITY) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LEVEL_DEITY, IMMLIST_FILE, getpid());
    mudlog("Initiating autowiz.", CMP, LEVEL_DEITY, FALSE);
    system(buf);
  }
}



void	gain_exp(struct char_data *ch, int gain)
{
  int	needed, level;
  bool is_altered = FALSE;

  if (GET_LEVEL(ch) >= LEVEL_DEITY) return;

  if (IS_NPC(ch) || ((LOWEST_LEVEL(ch) < LEVEL_DEITY - 1) && (GET_LEVEL(ch) > 0))) {
    if (gain > 0) {
      level = LOWEST_LEVEL(ch);
      needed = exp_needed(ch);
      gain = MIN(needed/(10+GET_LEVEL(ch)/15), gain); /* MAXgain  -Petrus */

      /* Hack for Remort -Helm */
      if (REMORT(ch) >= 5) gain = 1;
      /* End Remort Hack */

      GET_EXP(ch) += gain;
      if (!IS_NPC(ch) && level < LEVEL_DEITY - 1) {
	if (GET_EXP(ch) > needed) {
	  act("You {rise^gain^advance} a level!", FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
	  advance_level(ch, TRUE);
	  set_lowest_level(ch, level + 1);
	  GET_EXP(ch) = 0;     /*  Xp to 0 after level */
	  is_altered = TRUE;

	  /* Online Clan Power Tracking */
	  if (!IS_NPC(ch) && (CLAN(ch) >=0) && (CLAN_LEVEL(ch) > 1))
	    clan_list[CLAN(ch)].on_level += 1;
	}
	else {
	  if (gain > 1)
	    sprintf(buf2, "You receive %d experience points.\r\n",
		    gain);
	  else
	    strcpy(buf2, "You receive 1 lousy experience point.\r\n");
	  send_to_char(buf2, ch);
	}
      }
    }
  }
  if (gain < 0)
  {
	if (GET_EXP(ch) == 0)
	  send_to_char("You have no experience to lose.\r\n", ch);
	else
	{
	  if (abs(gain) > GET_EXP(ch))
	    gain = -GET_EXP(ch);

	  GET_EXP(ch) += gain;

	  sprintf(buf2, "You lose %d experience point%s.\r\n",
	    	  0 - gain, (gain == -1) ? "":"s");

      send_to_char(buf2, ch);

      // just in case
      if (GET_EXP(ch) < 0)
        GET_EXP(ch) = 0;
    }
  }

  if (is_altered) {
    if (!PLR_FLAGGED(ch, PLR_NOTITLE)) set_title(ch);
    check_autowiz(ch);
  }
}



void	gain_exp_regardless(struct char_data *ch, int gain)
{
    int	needed, level;
    bool is_altered = FALSE;

   if (GET_LEVEL(ch) >= LEVEL_DEITY) return;

    if (!IS_NPC(ch)) {
	level = LOWEST_LEVEL(ch);
	needed = exp_needed(ch);
	if (gain > 0) {
	    GET_EXP(ch) += gain;
	    if (GET_EXP(ch) > needed && level < LEVEL_DEITY - 1) {
	        act("You {rise^gain^advance} a level!", FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
		advance_level(ch, TRUE);
		set_lowest_level(ch, level + 1);
		GET_EXP(ch) = 0;     /*  Xp to 0 ater adv */
		is_altered = TRUE;
	    }
	}
    }
    if (gain < 0)
	GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
	GET_EXP(ch) = 0;
    if (is_altered) {
	set_title(ch);
	check_autowiz(ch);
    }
}


void	gain_condition(struct char_data *ch, int condition, int value)
{
  bool intoxicated;

  if (GET_COND(ch, condition) == -1) /* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition)  += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  if(GET_RACE(ch) == RACE_VAMPIRE)
  	stat_check(ch);   /* Special for Vampires */

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
    return;

  switch (condition) {
  case FULL :
    act("You{ are hungry^ are starving^ feel hungry^r stomach rumbles}.",
	FALSE, ch, 0, 0, TO_CHAR|TO_SLEEP);
    return;
  case THIRST :
    if(GET_RACE(ch) == RACE_VAMPIRE) {
	act("You crave blood.", FALSE, ch, 0, 0, TO_CHAR|TO_SLEEP);
        return;
    }
    act("You{ are thirsty^ are dehydrated^ feel thirsty^r throat is dry}.",
	FALSE, ch, 0, 0, TO_CHAR|TO_SLEEP);
    return;
  case DRUNK :
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  default :
    break;
  }
}

void
check_idling(struct char_data *ch)
{
  extern sh_int mortal_start_room;
/* kick out the linkless people - switched people get nuked too*/
/*
     if (!ch->desc) {
	  if (ch->specials.aliases)
	       alias_save(ch);
	  stringdata_save(ch);
	  Crash_quitsave(ch);
	  sprintf(buf, "%s linkless extracted.", GET_NAME(ch));
	  mudlog(buf, CMP, LEVEL_DEITY, FALSE);
	  extract_char(ch);
	  return;
     }
*/
  if (++(ch->specials.timer) > 8 && GET_LEVEL(ch) < LEVEL_ADMIN) {
    if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE) {
      ch->specials.was_in_room = ch->in_room;
      if (ch->specials.fighting) {
	stop_fighting(ch->specials.fighting);
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      if (ROOM_FLAGGED(IN_ROOM(ch), NO_QUIT) &&
	  (GET_LEVEL(ch) < LEVEL_DEITY))
	save_char(ch, mortal_start_room, 2);
      else
	save_char(ch, IN_VROOM(ch), 2);
      Crash_crashsave(ch, CRASH_SAVE);
      if (!ROOM_FLAGGED(IN_ROOM(ch), GODROOM)) {
	if (ch->in_room != NOWHERE) char_from_room(ch);
	char_to_room(ch, 1);
      }
    } else if (ch->specials.timer > 24 && GET_LEVEL(ch) < LEVEL_ADMIN) {
      if (!ROOM_FLAGGED(IN_ROOM(ch), GODROOM)) {
	if (ch->in_room != NOWHERE) char_from_room(ch);
	char_to_room(ch, 0);
      }
      if (ch->specials.aliases)
	alias_save(ch);
      ch->player.time.logon = time(0); /* idle extraction no time */
      stringdata_save(ch);
      Crash_quitsave(ch);
      if (ch->desc)
	close_socket(ch->desc);
      ch->desc = 0;
      sprintf(buf, "%s idle-saved and extracted.", GET_NAME(ch));
      mudlog(buf, CMP, LEVEL_DEITY, TRUE);
      extract_char(ch);
    }
  }
}



/* Update both PC's & NPC's and objects*/
void	point_update( void )
{
  void	update_char_objects( struct char_data *ch ); /* handler.c */
  void	extract_obj(struct obj_data *obj); /* handler.c */
  struct char_data *i, *next_dude;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  /* characters */
  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;

    if (GET_POS(i) >= POS_STUNNED) {
      GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  GET_MAX_HIT(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      if (i->desc && PRF_FLAGGED(i, PRF_DISPVT)
	  && !PLR_FLAGGED(i, PLR_WRITING))
	stats_to_screen(i->desc);
      if (IS_AFFECTED(i, AFF_POISON))
	damage(i, i, 2, SPELL_POISON);
      if (GET_POS(i) == POS_STUNNED)
	update_pos(i);
    } else if (GET_POS(i) == POS_INCAP)
      damage(i, i, 1, TYPE_SUFFERING);
    else if (!IS_NPC(i) && (GET_POS(i) == POS_MORTALLYW))
      damage(i, i, 2, TYPE_SUFFERING);
    // Fix for Dead people to make them die!
    else if (!IS_NPC(i) && (GET_POS(i) == POS_DEAD)) {
      GET_POS(i) = POS_MORTALLYW;
      damage(i, i, 2, TYPE_SUFFERING);
    }

    if (!IS_NPC(i)) {
      update_char_objects(i);
      check_idling(i);
    }

    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
    i->specials.gain_count = 0;
    if (i->specials.skillgain < 100)
      i->specials.skillgain++;

  } /* for */

  /* objects */
  for (j = object_list; j ; j = next_thing) {
    next_thing = j->next;	/* Next in object list */

    /* portal timeout code */
    if (GET_ITEM_TYPE(j) == ITEM_PORTAL) {
      if (!IS_SET(GET_ITEM_VALUE(j, 1), PORTAL_CLOSED) && GET_ITEM_VALUE(j, 5) > 0)
	GET_ITEM_VALUE(j, 5) -= 1;

      if (GET_ITEM_VALUE(j, 5) == 0) {
        if ((j->in_room != NOWHERE) && (world[j->in_room]->people)) {
	   act("The $p closes mysteriously.",
	       TRUE, world[j->in_room]->people, j, 0, TO_ROOM);
           act("The $p closes mysteriously.",
               TRUE, world[j->in_room]->people, j, 0, TO_CHAR);
        }
	SET_BIT(GET_ITEM_VALUE(j, 1), PORTAL_CLOSED);
        GET_ITEM_VALUE(j, 5) = obj_proto[j->item_number].obj_flags.value[5];
      }
      }

    /* If this is a corpse */
    if ( (GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3]) ) {
      /* timer count down */
      if (j->obj_flags.timer > 0)
	j->obj_flags.timer--;

      if(j->obj_flags.value[4])   {
          j->obj_flags.value[4] -= 1;
      }

      if (!j->contains)   {
	    j->obj_flags.timer = MIN(10, j->obj_flags.timer);
      }

       if(j->obj_flags.timer < 1) {
	if (j->carried_by)
	  act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room]->people)) {
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room]->people, j, 0, TO_ROOM);
	  act("A quivering hoard of maggots consumes $p.",
	      TRUE, world[j->in_room]->people, j, 0, TO_CHAR);
	}

	if(j->contains)

	for (jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content; /* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj, j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj, j->carried_by->in_room);
	  else if (j->in_room != NOWHERE)
	    obj_to_room(jj, j->in_room);
	  else
	    assert(FALSE);
	}
	extract_obj(j);
      }
    }
  } /* end objects */

  clan_update();
}

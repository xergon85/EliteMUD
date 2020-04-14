/* ************************************************************************
*   File: spell_parser.c                                Part of EliteMUD *
*  Usage: command interpreter for 'cast' command (spells)                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"
#include "functions.h"

#define MANA_MU 1
#define MANA_CL 1

#define SINFO spell_info[spellnum]

#define MANUAL_SPELL(spellname)\
        spellname(level, ch, arg, victim, tar_obj)

/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, sn)  spell_info[sn].min_usesmana
      
/* Global data */

extern struct room_data **world;
extern struct char_data *character_list;
extern char     *spells[];
extern char     *skills[];
struct spell_info_type spell_info[MAX_SPL_LIST];


void	affect_update( void )
{
    static struct affected_type *af, *next_af_dude;
    static struct char_data *i;
    
    for (i = character_list; i; i = i->next)
	for (af = i->affected; af; af = next_af_dude) {
	    next_af_dude = af->next;
	    if (af->duration >= 1)
		af->duration--;
	    else if (af->duration != DURATION_INNATE && 
		     af->duration != DURATION_PERMANENT) 
	    {
		if ((af->type > 0) && (af->type <= NUM_OF_SPELLS))
		    if (!af->next || (af->next->type != af->type) || 
			(af->next->duration > 0))
			if (spell_info[af->type].wearoffmess) {
			  act(spell_info[af->type].wearoffmess, FALSE, i, 0, 0, TO_CHAR);  
			/*send_to_char(spell_info[af->type].wearoffmess, i);
			  send_to_char("\r\n", i); */
			}
		affect_remove(i, af);
		if (i->desc && PRF_FLAGGED(i, PRF_DISPVT))
		    aff_to_screen(i->desc);
	    }
	}
}


void	clone_obj(struct obj_data *obj)
{
   struct obj_data *clone;

   CREATE(clone, struct obj_data, 1);

   *clone = *obj;

   clone->name               = strdup(obj->name);
   clone->description        = strdup(obj->description);
   clone->short_description  = strdup(obj->short_description);
   clone->action_description = strdup(obj->action_description);
   clone->ex_description     = 0;

   /* REMEMBER EXTRA DESCRIPTIONS */
   clone->carried_by         = 0;
   clone->in_obj             = 0;
   clone->contains           = 0;
   clone->next_content       = 0;
   clone->next               = 0;

   /* VIRKER IKKE ENDNU */
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool elite_follow(struct char_data *ch, struct char_data *victim)
{
   struct char_data *k;

   for (k = victim; k; k = k->master) {
      if (k == ch)
	 return(TRUE);
   }

   return(FALSE);
}


#define MAX_NUMBER_OF_FOLLOWERS   10

/* Check if the character doesn't have too many followers already */
int allow_follower(struct char_data *ch)
{
  struct follow_type *f;
  int followers = 0;

  f = ch->followers;

  while (f) {
    f = f->next;
    followers++;
  }

  return (followers < MAX_NUMBER_OF_FOLLOWERS);
}


/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void	stop_follower(struct char_data *ch)
{
   struct follow_type *j, *k;

   assert(ch->master);

   return_all_in_hand(ch->master, ch);

   if (IS_AFFECTED(ch, AFF_CHARM)) {
      act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
      act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
      if (affected_by_spell(ch, SPELL_CHARM_PERSON))
	 affect_from_char(ch, SPELL_CHARM_PERSON);
   } else {
      act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
      act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
   }

   if (ch->master->followers->follower == ch) { /* Head of follower-list? */
      k = ch->master->followers;
      ch->master->followers = k->next;
      free(k);
   } else { /* locate follower who is not head of list */
      for (k = ch->master->followers; k->next->follower != ch; k = k->next)
	 ;

      j = k->next;
      k->next = j->next;
      free(j);
   }

   ch->master = 0;
   REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void	die_follower(struct char_data *ch)
{
   struct follow_type *j, *k;

   if (ch->master)
      stop_follower(ch);

   for (k = ch->followers; k; k = j) {
      j = k->next;
      stop_follower(k->follower);
   }
}

/* called when a character that is mounted or mounting dies */
void  die_mount(struct char_data *ch)
{
    if (ch->specials.mounting) {
	act("You stop riding $N.", FALSE, ch, 0, ch->specials.mounting, TO_CHAR);
	act("$n no longer rides you.", TRUE, ch, 0, ch->specials.mounting, TO_VICT);
	(ch->specials.mounting)->specials.mounted_by = 0;
	ch->specials.mounting = 0;
    }

    if (ch->specials.mounted_by)
	die_mount(ch->specials.mounted_by);
}


/* called when a character that is protecting/protected dies/stops */
void  die_protector(struct char_data *ch)
{
    if (ch->specials.protecting) {
	act("You stop protecting $N.", FALSE, ch, 0, ch->specials.protecting, TO_CHAR);
	act("$n stops protecting you.", TRUE, ch, 0, ch->specials.protecting, TO_VICT);
	act("$n stops protecting $N.", TRUE, ch, 0, ch->specials.protecting, TO_NOTVICT);
	(ch->specials.protecting)->specials.protected_by = 0;
	ch->specials.protecting = 0;
    }

    if (ch->specials.protected_by)
	die_protector(ch->specials.protected_by);
}


/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void	add_follower(struct char_data *ch, struct char_data *leader)
{
   struct follow_type *k;

   assert(!ch->master);

   ch->master = leader;

   CREATE(k, struct follow_type, 1);

   k->follower = ch;
   k->next = leader->followers;
   leader->followers = k;

   act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
   act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
   act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}



void	say_spell(struct char_data *ch, int si )
{
   char splwd[MAX_INPUT_LENGTH];

   int	j, offs;
   struct char_data *temp_char;

   struct syllable {
      char	org[10];
      char	new[10];
   };

   struct syllable syls[] = {
      { " ", " " },
      { "ar", "abra"   },
      { "au", "kada"    },
      { "bless", "fido" },
      { "blind", "nose" },
      { "bur", "mosa" },
      { "cu", "judi" },
      { "de", "oculo" },
      { "en", "unso" },
      { "light", "dies" },
      { "lo", "hi" },
      { "mor", "zak" },
      { "move", "sido" },
      { "ness", "lacri" },
      { "ning", "illa" },
      { "per", "duda" },
      { "ra", "gru"   },
      { "re", "candus" },
      { "son", "sabru" },
      { "tect", "infra" },
      { "tri", "cula" },
      { "venus", "berg" },
      { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" }, { "e", "z" },
      { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "i" }, { "o", "a" },
      { "p", "s" }, { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
      { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, { "y", "l" },
      { "z", "k" }, { "", "" }
   };   
   
   
   *buf = '\0';
   strcpy(splwd, spells[si-1]);
   
   offs = 0;
   
   while (*(splwd + offs)) {
       for (j = 0; *(syls[j].org); j++)
	   if (strncmp(syls[j].org, splwd + offs, strlen(syls[j].org)) == 0) {
	       strcat(buf, syls[j].new);
	       if (strlen(syls[j].org))
		   offs += strlen(syls[j].org);
	       else
		   ++offs;
	   }
   }

   /* New random thing -Petrus */
   offs = 0;
   while (*(buf + offs)) {
       if (!number(0, 5))
	   *(buf + offs) = 'a' + number(0, 25);
       offs++;
   }
   
   sprintf(buf2, "$n {utters^mumbles^recites} the {words^formula}, '%s'", buf);
   sprintf(buf, "$n {utters^mumbles^recites} the {words^formula}, '%s'", spells[si-1]);
   
   for (temp_char = world[ch->in_room]->people; 
	temp_char; 
	temp_char = temp_char->next_in_room)
       if (temp_char != ch) {
	   if (GET_CLASS(ch) == GET_CLASS(temp_char))
	       act(buf, FALSE, ch, 0, temp_char, TO_VICT);
	   else if (number(1, 85) + GET_LEVEL(ch) < GET_SKILL(temp_char, SKILL_SPELLCRAFT)) {
	       act(buf, FALSE, ch, 0, temp_char, TO_VICT);
	       improve_skill(temp_char, SKILL_SPELLCRAFT);
	   } else
	       act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
       }
}



int saves_spell(struct char_data *ch, sh_int save_type, int *dam, int type)
{
  int i = 0, percent = 0;
  
  if (save_type >= SAVING_PHYSICAL && save_type <= SAVING_POISON) {
    percent = (ch->specials2.resistances[save_type - SAVING_PHYSICAL]);
  } else if (ch->mob_specials.resists) {
    while (ch->mob_specials.resists[i].type != 0) {
      if (save_type == ch->mob_specials.resists[i].type) {
	percent = to_percentage(ch, (ch->mob_specials.resists[i].percentage));
	break;
      }
      i++;
    }
  }

  switch (type) {

    case SAVE_NONE : break;
 
    case SAVE_HALF :
      if (number(1, 100) <= percent) {
	if (dam) *dam >>= 1;
	return 1;
      }
      break;
  
    case SAVE_NEGATE :
      if (number(1, 100) <= percent) {
	if (dam) *dam = 0;
	return 1;
      }
      break;
     
    case SAVE_SPECIAL :
      return (percent);
      break;
    
    default : break;
  }

  return 0;
}

int magic_resist(struct char_data *ch, struct char_data *victim)
{
  if (number(1, 100) < MIN(85, victim->specials2.resistances[4])) {
    act("Your magic fails to affect $N.\r\n", FALSE, ch, 0, victim, TO_CHAR);
    act("$n's magic fails to affect you.\r\n", TRUE, ch, 0, victim, TO_VICT);
    act("$n's spell fails to affect $N.\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
    return 1;
  }

  return 0;
}


	

/* rem all spaces before string */
void  skip_spaces(char **string)
{
    for (; *string && **string && (**string) == ' '; (*string)++);
}



/*
 * All invocations of any spell must come through this function,
 * call_magic(). This is also the entry point for non-spoken or unrestricted
 * spells. Spellnum 0 is legal but silently ignored here, to make callers
 * simpler.
 */
int call_magic(struct char_data *ch, char *arg, struct char_data *victim,
	       struct obj_data *tar_obj, int spellnum,
	       int level, int casttype)
{
  if (spellnum < 1 || spellnum > NUM_OF_SPELLS)
    return 0;
    
  if (ROOM_FLAGGED(ch->in_room, NO_MAGIC)) {
    send_to_char("Your magic fizzles out and dies.\r\n", ch);
    act("$n's magic fizzles out and dies.", FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }
  if (IS_SET(ROOM_FLAGS(ch->in_room), LAWFULL) &&
      (SINFO.offensive || IS_SET(SINFO.type, MAG_DAMAGE))) {
    send_to_char("A flash of white light fills the room, dispelling your "
		 "violent magic!\r\n", ch);
    act("White light from no particular source suddenly fills the room, "
	"then vanishes.", FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }
  if (!IS_NPC(ch) && victim && !IS_NPC(victim) &&
      GET_LEVEL(ch) >= LEVEL_DEITY && GET_LEVEL(victim) < LEVEL_DEITY) {
    sprintf(buf2, "(GC) %s casts spell '%s' on %s at %s",
	    GET_NAME(ch), spells[spellnum-1], GET_NAME(victim), world[victim->in_room]->name);
    mudlog(buf2, BRF, MIN(GET_LEVEL(ch)+1, LEVEL_IMPL), TRUE);
  }
    
  /*  CHeck/fix this later -P 
      switch (casttype) {
      
      case SPELL_TYPE_STAFF:
      case SPELL_TYPE_SCROLL:
      case SPELL_TYPE_POTION:
      case SPELL_TYPE_WAND:
      case SPELL_TYPE_SPELL:
      savetype = SAVING_MAGIC;
      default:
      savetype = SAVING_MAGIC;
      break;
      } 
  */
    
  if (IS_SET(SINFO.type, MAG_DAMAGE))
    mag_damage(level, ch, victim, spellnum, casttype);

  if (IS_SET(SINFO.type, MAG_AFFECTS))
    mag_affects(level, ch, victim, spellnum, casttype);
        
  if (IS_SET(SINFO.type, MAG_UNAFFECTS))
    mag_unaffects(level, ch, victim, spellnum, casttype);
    
  if (IS_SET(SINFO.type, MAG_POINTS))
    mag_points(level, ch, victim, spellnum, casttype);

  /*    
     if (IS_SET(SINFO.type, MAG_ALTER_OBJS))
     mag_alter_objs(level, ch, tar_obj, spellnum, savetype);
          
     if (IS_SET(SINFO.type, MAG_MASSES))
     mag_masses(level, ch, spellnum, savetype);
  */

  if (IS_SET(SINFO.type, MAG_GROUPS))
    mag_groups(level, ch, spellnum, casttype);

  if (IS_SET(SINFO.type, MAG_AREAS))
    mag_areas(level, ch, spellnum, casttype);
    
  if (IS_SET(SINFO.type, MAG_SUMMONS))
    mag_summons(level, ch, tar_obj, spellnum, casttype);
    
  if (IS_SET(SINFO.type, MAG_CREATIONS))
    mag_creations(level, ch, spellnum);
    
  if (IS_SET(SINFO.type, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_ENCHANT_WEAPON:
      MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_CHARM_PERSON:
      MANUAL_SPELL(spell_charm_person); break;
    case SPELL_WORD_OF_RECALL:
      MANUAL_SPELL(spell_word_of_recall); break;
    case SPELL_CLAN_RECALL:
      MANUAL_SPELL(spell_clan_recall); break;
    case SPELL_IDENTIFY:
      MANUAL_SPELL(spell_identify); break;
    case SPELL_SUMMON:
      MANUAL_SPELL(spell_summon); break;
    case SPELL_LOCATE_OBJECT:
      MANUAL_SPELL(spell_locate_object); break;
    case SPELL_LOCATE_PERSON:
      MANUAL_SPELL(spell_locate_person); break;
    case SPELL_CONTROL_WEATHER:
      MANUAL_SPELL(spell_control_weather); break;
    case SPELL_TELEPORT:
      MANUAL_SPELL(spell_teleport); break;
    case SPELL_CREATE_WATER:
      MANUAL_SPELL(spell_create_water); break;
    } 
	
  return 1;
}


/*
 * mag_objectmagic: This is the entry-point for all magic items.
 *
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified.
 */

#define DEFAULT_STAFF_LVL  14
#define DEFAULT_WAND_LVL   14

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
		     char *argument)
{
  int i, k;
  struct char_data *tch, *next_tch;
  struct obj_data *tobj;
    
  tch = next_tch = NULL;
  tobj = NULL;

  one_argument(argument, arg);
    
  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);
    
  switch (GET_ITEM_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);
	
    if (GET_ITEM_VALUE(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_ITEM_VALUE(obj, 2)--;
      for (tch = world[ch->in_room]->people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	
	if (ch == tch)
	  continue;
	if (spell_info[GET_ITEM_VALUE(obj, 3)].offensive &&
	    !pkok_check(ch, tch))
	  continue;
	if (GET_ITEM_VALUE(obj, 0))
	  call_magic(ch, argument, tch, NULL, GET_ITEM_VALUE(obj, 3),
		     GET_ITEM_VALUE(obj, 0), SPELL_TYPE_STAFF);
	else
	  call_magic(ch, argument, tch, NULL, GET_ITEM_VALUE(obj, 3),
		     DEFAULT_STAFF_LVL, SPELL_TYPE_STAFF);

	if (IS_SET(spell_info[GET_ITEM_VALUE(obj, 3)].type, MAG_AREAS)) break;
	/* Fixes missing target crash when staff is area affect spell -Helm */
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description != NULL)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description != NULL)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }
	
    if (GET_ITEM_VALUE(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      return;
    }
    GET_ITEM_VALUE(obj, 2)--;
    if (GET_ITEM_VALUE(obj, 0))
      call_magic(ch, argument, tch, tobj, GET_ITEM_VALUE(obj, 3),
		 GET_ITEM_VALUE(obj, 0), SPELL_TYPE_WAND);
    else
      call_magic(ch, argument, tch, tobj, GET_ITEM_VALUE(obj, 3),
		 DEFAULT_WAND_LVL, SPELL_TYPE_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;
	
    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);
	
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, argument, tch, tobj, GET_ITEM_VALUE(obj, i),
		       GET_ITEM_VALUE(obj, 0), SPELL_TYPE_SCROLL)))
	break;
	
    if (obj != NULL) {
      if (obj == ch->equipment[HOLD])
	unequip_char(ch, HOLD);
      extract_obj(obj);
    }
    break;
  case ITEM_POTION:
    tch = ch;
    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);
	
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, argument, ch, NULL, GET_ITEM_VALUE(obj, i),
		       GET_ITEM_VALUE(obj, 0), SPELL_TYPE_POTION)))
	break;
	
    if (obj != NULL) {
      if (obj == ch->equipment[HOLD])
	unequip_char(ch, HOLD);
      extract_obj(obj);
    }
    break;
  default:
    log("SYSERR: Unknown object_type in mag_objectmagic");
    break;
  }
}


/* Assumes that *argument does start with first letter of chopped string */

ACMD(do_cast)
{
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char	name[MAX_STRING_LENGTH];
  int	qend, spl, i;
  bool target_ok;
  
  if (IS_NPC(ch))
    return;
  
  skip_spaces(&argument);
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where?\r\n", ch);
    return;
  }
  
  if (*argument >= '0' && *argument <= '9') {
    one_argument(argument, name);
    spl = atoi(name);
    qend = strlen(name) - 1;
  } else {
    
    if (*argument != '\'') {
      send_to_char("Magic must always be enclosed by the holy magic symbols: '\r\n", ch);
      return;
    }
    
    /* Locate the last quote && lowercase the magic words (if any) */
    
    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\'') ; qend++)
      *(argument + qend) = LOWER(*(argument + qend));
    
    if (*(argument + qend) != '\'') {
      send_to_char("Magic must always be enclosed by the holy magic symbols: '\r\n", ch);
      return;
    }

    strncpy(name, argument + 1, qend - 1);
    *(name + qend - 1) = 0;
    spl = 1 + search_block(name, spells, FALSE);
  }
  
  if ((spl > 0) && (spl <= NUM_OF_SPELLS) && spell_info[spl].type) {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      switch (GET_POS(ch)) {
      case POS_SLEEPING :
	send_to_char("You dream about great magical powers.\r\n", ch);
	break;
      case POS_RESTING :
	send_to_char("You can't concentrate enough while resting.\r\n", ch);
	break;
      case POS_SITTING :
	send_to_char("You can't do this sitting!\r\n", ch);
	break;
      case POS_FIGHTING :
	send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
	break;
      default:
	send_to_char("It seems like you're in a pretty bad shape!\r\n", ch);
	break;
      } /* Switch */
    } else {
      
      if (!GET_SKILL(ch, spl)) {
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }
      
      if (GET_LEVEL(ch) < LEVEL_DEITY) {
	   if (GET_MANA(ch) < USE_MANA(ch, spl)) {
		send_to_char("You can't summon enough energy to cast the spell.\r\n", ch);
		return;
	   }
      }

      argument += qend + 1;	/* Point to the last ' */
      skip_spaces(&argument);
      
      /* **************** Locate targets **************** */
      
      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
	
	argument = one_argument(argument, name);
	
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    if ((tar_char = get_char_room_vis(ch, name)))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    if ((tar_char = get_char_vis(ch, name)))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    if ((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    if ((tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room]->contents)))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    if ((tar_obj = get_obj_vis(ch, name)))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
	    for (i = 0; i < MAX_WEAR && !target_ok; i++)
	      if (ch->equipment[i] && !str_cmp(name, ch->equipment[i]->name)) {
		tar_obj = ch->equipment[i];
		target_ok = TRUE;
	      }
	  }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
	    if (str_cmp(GET_NAME(ch), name) == 0) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  
	} else { /* No argument was typed */
	  
	  if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
	    if (ch->specials.fighting) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
	    if (ch->specials.fighting) {
	      /* WARNING, MAKE INTO POINTER */
	      tar_char = ch->specials.fighting;
	      target_ok = TRUE;
	    }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	    tar_char = ch;
	    target_ok = TRUE;
	  }
	  
	}
	
      } else {
	target_ok = TRUE; /* No target, is a good target */
      }
      
      if (!target_ok) {
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    send_to_char("Nobody here by that name.\r\n", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    send_to_char("Nobody playing by that name.\r\n", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    send_to_char("You are not carrying anything like that.\r\n", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    send_to_char("Nothing here by that name.\r\n", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\r\n", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
	    send_to_char("You are not wearing anything like that.\r\n", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\r\n", ch);
	  
	  WAIT_STATE(ch, spell_info[spl].beats);
	} else { /* Nothing was given as argument */
	  if (spell_info[spl].targets < TAR_OBJ_INV)
	    send_to_char("Who should the spell be cast upon?\r\n", ch);
	  else
	    send_to_char("What should the spell be cast upon?\r\n", ch);
	}
	return;
      } else { /* TARGET IS OK */
	if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
	  send_to_char("You can not cast this spell upon yourself.\r\n", ch);
	  return;
	} else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY) &&
                   (GET_LEVEL(ch) < LEVEL_IMMORT)) {
	  send_to_char("You can only cast this spell upon yourself.\r\n", ch);
	  return;
	} else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
	  send_to_char("You are afraid that it could harm your master.\r\n", ch);
	  return;
	} else if (!IS_AFFECTED(ch, AFF_GROUP) && spell_info[spl].type == 
                   MAG_GROUPS) {
	  send_to_char("You need to be grouped to cast this spell!\r\n", ch);
	  return;
	}
      }
            
      
      if (spl != SPELL_VENTRILOQUATE)  /* :-) */
	say_spell(ch, spl);
      
      WAIT_STATE(ch, spell_info[spl].beats);
      
      if (!spell_info[spl].type && spl > 0)
	send_to_char("Sorry, this magic has not yet been implemented :(\r\n", ch);
      else {
	if (spell_info[spl].offensive &&
	    tar_char &&
	    !pkok_check(ch, tar_char))
	  return;

	if (number(1, 101) > GET_SKILL(ch, spl)) { /* 101% is failure */
	  send_to_char("You lost your concentration!\r\n", ch);
	  GET_MANA(ch) -= (USE_MANA(ch, spl) >> 1);
	  return;
	}
	if (spl != SPELL_SUMMON)
	  send_to_char("Ok.\r\n", ch);

	/* DO THE ACTUAL CASTING */
	if (call_magic(ch, argument, tar_char, tar_obj, spl,
		       GET_LEVEL(ch), SPELL_TYPE_SPELL)) {
	  GET_MANA(ch) -= (USE_MANA(ch, spl));
	  improve_skill(ch, spl);
	  /*
	     if (spell_info[spl].offensive &&
	     tar_char && IS_NPC(tar_char) &&
	     !FIGHTING(tar_char) &&
	     CAN_SEE(tar_char, ch))
	     set_fighting(tar_char, ch);
	     */}
      }
    }	/* if GET_POS < min_pos */
    
    return;
  }
  
  act("{Sim Sala Bim Bam Bom???^{Olle^Gryffe^Bolle} {Bolle^Gryffe^Gryf^Snylle} {Snop^Snylle^Bolle^Snok} {Bylle^Bonke^Sanke}?!?}", FALSE, ch, 0, 0, TO_CHAR);

  /*
  switch (number(1, 5)) {
  case 1:
    send_to_char("Sim Sala Bim Bam Bom???\r\n", ch);
    break;
  case 2:
    send_to_char("Olle Bolle Snop Snyf?\r\n", ch);
    break;
  case 3:
    send_to_char("Olle Grylle Bolle Bylle?!?\r\n", ch);
    break;
  case 4:
    send_to_char("Gryffe Olle Gnyffe Snop???\r\n", ch);
    break;
  default:
    send_to_char("Bolle Snylle Gryf Bylle?!!?\r\n", ch);
    break;
  }*/
}


int mob_cast_spell(struct char_data *ch, 
		   struct char_data *victim,
		   int spellnum, 
		   sbyte percent,
		   byte flag,
		   int value1,
		   byte value2)
{
    struct obj_data *obj = 0;

    if (!SINFO.type || !ch || !victim)
	return 0;
    
    if (flag == CAST_AFF_SELF) {
	if (affected_by_spell(ch, spellnum))
	    return 0;
	victim = ch;
    } else if (flag == CAST_AFF_VICT) {
	if (affected_by_spell(victim, spellnum))
	    return 0;
    } else if (flag == CAST_HEAL) {
	if (GET_HIT(ch) > GET_MAX_HIT(ch) / 2)
	    return 0;
	victim = ch;
    } else if (flag == CAST_FLEE) {
	if (GET_HIT(ch) > GET_MAX_HIT(ch) / 5)
	    return 0;
	victim = ch;
    } else if (flag == CAST_AFF_OWNWPN) {
	if (!ch->equipment[WIELD])
	    return 0;
	else
	    obj = ch->equipment[WIELD];
    } else if (flag == CAST_AFF_VICTWPN) {
	if (!victim->equipment[WIELD])
	    return 0;
	else
	    obj = victim->equipment[WIELD];
    } else if (flag == CAST_CURE_SELF) {
	if (!affected_by_spell(ch, value1))
	    return 0;
	victim = ch;
    }

    say_spell(ch, spellnum);

    if (number(1, 101) > percent) { 
	send_to_char("You lost your concentration!\r\n", ch);
	return 0;
    }
    send_to_char("Ok.\r\n", ch);

    /* DO THE ACTUAL CASTING */
    call_magic(ch, "", victim, obj, spellnum,
	       GET_LEVEL(ch), SPELL_TYPE_SPELL);

    return 1;
}


void  spello(int nr, int type, byte pos, ubyte mana, byte beat,
             sh_int targs, byte off, char* mess)
{  
    spell_info[nr].type = (type);
    spell_info[nr].minimum_position = (pos);
    spell_info[nr].min_usesmana = (mana);
    spell_info[nr].beats = (beat);
    spell_info[nr].targets = (targs);
    spell_info[nr].offensive = (off);
    spell_info[nr].wearoffmess = (mess);
}


void	assign_spell_pointers(void)
{
    int	i;
    
    for (i = 0; i < MAX_SPL_LIST; i++)
	spell_info[i].type = 0;
    
    /*  SPELLNR, TYPE, MINPOS, MANA, BEATS, TARGS, OFFENSIVE   */

    /* DAMAGE SPELLS MAINLY */
    spello(SPELL_MAGIC_MISSILE, MAG_DAMAGE, POS_FIGHTING, 6, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_MIND_JAB, MAG_DAMAGE, POS_FIGHTING, 6, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_PHASE_KNIFE, MAG_DAMAGE, POS_FIGHTING, 6, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_FLAME_RAY, MAG_DAMAGE, POS_FIGHTING, 9, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_PSYCHIC_SCREAM, MAG_DAMAGE, POS_FIGHTING, 10, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_CHILL_TOUCH, MAG_DAMAGE | MAG_AFFECTS, POS_FIGHTING, 11, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, "You don't feel so {weak^chilled} any{more^ longer}.");
    
    spello(SPELL_BURNING_HANDS, MAG_DAMAGE, POS_FIGHTING, 12, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_SHOCKING_GRASP, MAG_DAMAGE, POS_FIGHTING, 12, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_SPECTRE_TOUCH, MAG_DAMAGE | MAG_AFFECTS, POS_FIGHTING, 18, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, "You don't feel as drained any{more^ longer}.");

    spello(SPELL_LIGHTNING_BOLT, MAG_DAMAGE, POS_FIGHTING, 22, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_COLOUR_SPRAY, MAG_DAMAGE, POS_FIGHTING, 13, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_ENERGY_DRAIN, MAG_DAMAGE, POS_FIGHTING, 24, 12,TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_FIREBALL, MAG_DAMAGE, POS_FIGHTING, 40, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_PROJECT_FORCE, MAG_DAMAGE, POS_FIGHTING, 28, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_DETONATE, MAG_DAMAGE, POS_FIGHTING, 32, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_PHASEFIRE, MAG_DAMAGE, POS_FIGHTING, 36, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_DISPEL_EVIL, MAG_DAMAGE, POS_FIGHTING, 30, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_CALL_LIGHTNING, MAG_DAMAGE, POS_FIGHTING, 25, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);
    
    spello(SPELL_HARM, MAG_DAMAGE, POS_FIGHTING, 50, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_ARC_FIRE, MAG_DAMAGE, POS_FIGHTING, 11, 6, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_WARSTRIKE, MAG_DAMAGE, POS_FIGHTING, 30, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_SHOCKING_SPHERE, MAG_DAMAGE, POS_FIGHTING, 35, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_SPECTRE_TOUCH, MAG_DAMAGE | MAG_AFFECTS, POS_FIGHTING, 30, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_DEATH_STRIKE, MAG_DAMAGE, POS_STANDING, 220, 20, TAR_CHAR_ROOM, TRUE, NULL);
    
    spello(SPELL_MIND_BLADE, MAG_DAMAGE, POS_FIGHTING, 40, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_RIMEFANG, MAG_DAMAGE, POS_FIGHTING, 40, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_DRAIN_LIFE, MAG_DAMAGE, POS_FIGHTING, 45, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_SHADOW_KNIFE, MAG_DAMAGE, POS_FIGHTING, 60, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_GRAVITY_FOCUS, MAG_DAMAGE, POS_STANDING, 200, 20, TAR_CHAR_ROOM, TRUE, NULL);

    /* spello(SPELL_DISINTEGRATE, MAG_DAMAGE, POS_FIGHTING, 55, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL); */

    spello(SPELL_TERRORWEAVE, MAG_DAMAGE, POS_FIGHTING, 80, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL); 

    spello(SPELL_ELEMENTAL_CANNON, MAG_DAMAGE, POS_FIGHTING, 75, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_UNLEASH_MIND, MAG_DAMAGE, POS_FIGHTING, 80, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    spello(SPELL_PHANTASMAL_KILLER, MAG_DAMAGE, POS_FIGHTING, 85, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, NULL);

    /* HEAL SPELLS MAINLY */
    spello(SPELL_WORD_OF_HEALING, MAG_POINTS, POS_SITTING, 6, 12, TAR_CHAR_ROOM, FALSE, NULL);

    spello(SPELL_CURE_CRITIC, MAG_POINTS, POS_FIGHTING, 25, 12, TAR_CHAR_ROOM, FALSE, NULL);
    
    spello(SPELL_CURE_LIGHT, MAG_POINTS, POS_FIGHTING, 8, 12, TAR_CHAR_ROOM, FALSE, NULL);
    
    spello(SPELL_FLESH_RESTORE, MAG_POINTS, POS_FIGHTING, 14, 12, TAR_CHAR_ROOM, FALSE, NULL);

    spello(SPELL_FLESH_ANEW, MAG_POINTS, POS_FIGHTING, 30, 12, TAR_CHAR_ROOM, FALSE, NULL);

    spello(SPELL_HEAL, MAG_POINTS | MAG_UNAFFECTS, POS_FIGHTING, 60, 12, TAR_CHAR_ROOM, FALSE, NULL);

    spello(SPELL_QUICK_FIX, MAG_POINTS, POS_FIGHTING, 5, 12, TAR_CHAR_ROOM, FALSE, NULL);

    /* AFFECT SPELLS MAINLY */
    spello(SPELL_ARMOR, MAG_AFFECTS, POS_STANDING, 12, 12, TAR_CHAR_ROOM, FALSE, "You feel {less protected^little exposed^less armored}.");

    spello(SPELL_BLESS, MAG_AFFECTS, POS_STANDING, 8, 12, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, FALSE, "You feel less righteous.");
    
    spello(SPELL_IMPROVED_BLESS, MAG_AFFECTS, POS_STANDING, 8, 12, TAR_SELF_ONLY, FALSE, "You feel less watched.");

    spello(SPELL_BLINDNESS, MAG_AFFECTS, POS_STANDING, 13, 12,TAR_CHAR_ROOM, TRUE, "You feel a cloak of blindness disolve.");
    
    spello(SPELL_CURSE, MAG_AFFECTS, POS_STANDING, 12, 12, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, "You feel better.");

    spello(SPELL_DETECT_INVISIBLE, MAG_AFFECTS, POS_STANDING, 20, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "{Your ability to^The} detect invisible wears off.");
   
    spello(SPELL_DETECT_MAGIC, MAG_AFFECTS, POS_STANDING, 16, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "{Your ability to^The} detect magic wears off.");

    spello(SPELL_DETECT_POISON, MAG_AFFECTS, POS_STANDING, 24, 12, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, "{Your ability to^The} detect poison wears off.");

    spello(SPELL_DETECT_ALIGN, MAG_AFFECTS, POS_STANDING, 16, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "{Your ability to^The} detect alignment wears off.");

    spello(SPELL_INFRAVISION, MAG_AFFECTS, POS_STANDING, 40, 12, TAR_CHAR_ROOM, FALSE, "Your vision is briefly blurred.");
    
    spello(SPELL_INVISIBLE, MAG_AFFECTS, POS_STANDING, 22, 12, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, FALSE, "You feel yourself exposed.");

    spello(SPELL_POISON, MAG_AFFECTS, POS_STANDING, 16, 12, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP, TRUE, "You feel less sick.");

    spello(SPELL_PROTECT_FROM_EVIL, MAG_AFFECTS, POS_STANDING, 16, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "You feel less protected.");

    spello(SPELL_SANCTUARY, MAG_AFFECTS, POS_STANDING, 50, 12, TAR_CHAR_ROOM, FALSE, "The white aura around your body fades.");

    spello(SPELL_HOLY_WRATH, MAG_AFFECTS, POS_STANDING, 80, 12, TAR_SELF_ONLY, FALSE, "The power of the gods has left you.");

    spello(SPELL_SLEEP, MAG_AFFECTS, POS_STANDING, 24, 12, TAR_CHAR_ROOM, FALSE, "You feel less tired.");

    spello(SPELL_STRENGTH, MAG_AFFECTS, POS_STANDING, 25, 12, TAR_CHAR_ROOM, FALSE, "You feel weaker.");

    spello(SPELL_SENSE_LIFE, MAG_AFFECTS, POS_STANDING, 13, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "You feel less aware of your surroundings.");

    spello(SPELL_LEVITATION, MAG_AFFECTS, POS_STANDING, 20, 12, TAR_CHAR_ROOM, FALSE, "You start floating down.");

    spello(SPELL_FLY, MAG_AFFECTS, POS_STANDING, 40, 12, TAR_CHAR_ROOM, FALSE, "You start falling!");
   
    spello(SPELL_REGENERATION, MAG_AFFECTS, POS_FIGHTING, 60, 12, TAR_CHAR_ROOM, FALSE, "Your metabolism has returned to normal.");

    spello(SPELL_CAT_EYES, MAG_AFFECTS, POS_STANDING, 12, 12, TAR_SELF_ONLY, FALSE, "Your night vision is gone.");
    
    spello(SPELL_VORPAL_PLATING, MAG_AFFECTS, POS_STANDING, 20, 12, TAR_CHAR_ROOM, FALSE, "Your protecting magical force disappears.");

    spello(SPELL_MAGE_GAUNTLETS, MAG_AFFECTS, POS_STANDING, 18, 12, TAR_CHAR_ROOM, FALSE, "Your invisible gauntlets disappear.");

    spello(SPELL_MYSTIC_SHIELD, MAG_AFFECTS, POS_STANDING, 40, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "The misty shield vaporizes");

    spello(SPELL_MYSTICAL_COAT, MAG_AFFECTS, POS_STANDING, 60, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "The misty coat vaporizes");

    spello(SPELL_PHASE_BLUR, MAG_AFFECTS, POS_FIGHTING, 26, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, "Your body must have stopped blurring.");
    
    spello(SPELL_WATER_BREATHING, MAG_AFFECTS, POS_STANDING, 40, 12, TAR_CHAR_ROOM, FALSE, "Your gills disappear.");

    /* UNAFFECT SPELLS MAINLY */
    spello(SPELL_CURE_BLIND, MAG_UNAFFECTS, POS_STANDING, 14, 12, TAR_CHAR_ROOM, FALSE, NULL);
    
    spello(SPELL_REMOVE_CURSE, MAG_UNAFFECTS, POS_STANDING, 14, 12, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, FALSE, NULL);

    spello(SPELL_REMOVE_POISON, MAG_UNAFFECTS, POS_STANDING, 14, 12, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, NULL);
   

    
    /* SPELLS WITH OWN PROCEDURES */
    spello(SPELL_ENCHANT_WEAPON, MAG_MANUAL, POS_STANDING, 80, 12, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, NULL);
    
    spello(SPELL_CHARM_PERSON, MAG_MANUAL, POS_STANDING, 45, 12, TAR_CHAR_ROOM | TAR_SELF_NONO, FALSE, "You feel more self-confident.");

    spello(SPELL_WORD_OF_RECALL, MAG_MANUAL, POS_STANDING, 30, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, NULL);

    spello(SPELL_CLAN_RECALL, MAG_MANUAL, POS_STANDING, 30, 13, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, NULL);

    spello(SPELL_IDENTIFY, MAG_MANUAL, POS_STANDING, 100, 12, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, NULL);

    spello(SPELL_LOCATE_OBJECT, MAG_MANUAL, POS_STANDING, 50, 12, TAR_IGNORE, FALSE, NULL);

    spello(SPELL_LOCATE_PERSON, MAG_MANUAL, POS_STANDING, 100, 25, TAR_IGNORE, FALSE, NULL);

    /* Match the mana use in summon in magic.c */
    spello(SPELL_SUMMON, MAG_MANUAL, POS_STANDING, 45, 12, TAR_CHAR_WORLD, FALSE, NULL);

    spello(SPELL_TELEPORT, MAG_MANUAL, POS_FIGHTING, 50, 12, TAR_SELF_ONLY, FALSE, NULL);

    spello(SPELL_CONTROL_WEATHER, MAG_MANUAL, POS_STANDING, 10, 12, TAR_IGNORE, FALSE, NULL);
    
    spello(SPELL_CREATE_WATER, MAG_MANUAL, POS_STANDING, 15, 12, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, NULL);
    

    /* CREATE OBJECTS SPELLS MAINLY */
    spello(SPELL_CREATE_FOOD, MAG_CREATIONS, POS_STANDING, 15, 12, TAR_IGNORE, FALSE, NULL);


    /* MASS DAMAGE SPELLS MAINLY */
    spello(SPELL_EARTHQUAKE, MAG_AREAS, POS_FIGHTING, 20, 12, TAR_IGNORE, TRUE, NULL);
    
    spello(SPELL_FIRE_BREATH, MAG_AREAS, POS_FIGHTING, 100, 20, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_GAS_BREATH, MAG_AREAS, POS_FIGHTING, 100, 20, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_FROST_BREATH, MAG_AREAS, POS_FIGHTING, 100, 20, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_ACID_BREATH, MAG_AREAS, POS_FIGHTING, 100, 20, TAR_IGNORE, TRUE, NULL);
   
    spello(SPELL_LIGHTNING_BREATH, MAG_AREAS, POS_FIGHTING, 100, 20, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_ICE_STORM, MAG_AREAS, POS_FIGHTING, 46, 12, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_MAELSTROM, MAG_AREAS, POS_FIGHTING, 58, 12, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_DRAGON_BREATH, MAG_AREAS, POS_FIGHTING, 80, 12, TAR_IGNORE, TRUE, NULL);

    spello(SPELL_STAR_FLARE, MAG_AREAS, POS_FIGHTING, 15, 12, TAR_IGNORE, TRUE, NULL);

    /* SUMMON / GATE SPELLS */
    spello(SPELL_ANIMATE_DEAD, MAG_SUMMONS, POS_STANDING, 150, 15, TAR_OBJ_ROOM, FALSE, NULL);

    spello(SPELL_INSTANT_WOLF, MAG_SUMMONS, POS_FIGHTING, 100, 15, TAR_IGNORE, FALSE, NULL);

    spello(SPELL_INSTANT_SLAYER, MAG_SUMMONS, POS_FIGHTING, 120, 15, TAR_IGNORE, FALSE, NULL);

    /*    Group Spells       */
    spello(SPELL_GROUP_HEAL, MAG_GROUPS, POS_FIGHTING, 25, 12, TAR_IGNORE, FALSE, NULL);

    spello(SPELL_GROUP_RECALL, MAG_GROUPS, POS_STANDING, 15, 12, TAR_IGNORE, FALSE, NULL);


    /*
    
    SPELLO(18, 12, POS_STANDING, 12, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_evil, "You sense the red in your vision disappear.");
    
    SPELLO(41, 12, POS_STANDING, 4, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);
    
    SPELLO(49, 4, POS_STANDING, 10, TAR_CHAR_ROOM, 0);
    
    */

}

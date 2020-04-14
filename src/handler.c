/*
************************************************************************
*   File: handler.c                                     Part of EliteMUD  * 
*  Usage: internal funcs: moving and finding chars/objs                   *
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
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "functions.h"
#include "spells.h"

/* external vars */
extern long top_idnum;
extern struct room_data **world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct char_data **mob_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct room_affect_type *room_affect_list;
extern struct clan_data *clan_list;
extern char	*MENU;
extern char     *PC_MENU;

/* external functions */
void	free_char(struct char_data *ch);
void	stop_fighting(struct char_data *ch);
void	remove_follower(struct char_data *ch);
void	clearMemory(struct char_data *ch);

extern int      str_max[];
extern int      con_max[];
extern int      dex_max[];
extern int      wis_max[];
extern int      int_max[];
extern int      cha_max[];
extern int    top_of_world;
extern int    top_of_clan;


char *
fname(char *namelist)
{
  static char	holder[32];
  register char	*point;
  
  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;
  
  *point = '\0';
  
  return (holder);
}

int
isname(char *str, char *namelist)
{
  register char	*curname, *curstr;
    
  /* Safety check - Petrus */
  if (!*namelist || !*str)
    return FALSE;

  curname = namelist;

  for (; ; ) {
    for (curstr = str; ; curstr++, curname++) {
      if (!*curstr && !isalpha(*curname))
	return 1;
	    
      if (!*curname)
	return(0);
	    
      if (!*curstr || *curname == ' ')
	break;
	    
      if (LOWER(*curstr) != LOWER(*curname))
	break;
    }
	
    /* skip to next name */
	
    for (; isalpha(*curname) || *curname == '_'; curname++);

    if (!*curname)
      return(0);
    
    curname++;			/* first char of new name */
  }
}



void	affect_modify(struct char_data *ch, byte loc, sbyte mod, long bitv, bool add)
{
    int	maxabil;
   
    if (loc == APPLY_BV2) {                        /* affected_by2 */
      if (add) {
        SET_BIT(ch->specials.affected_by2, bitv);
      } else {
        REMOVE_BIT(ch->specials.affected_by2, bitv);
        mod = -mod;
      }
    } else {
      if (add) {                                   /* affected_by  */
        SET_BIT(ch->specials.affected_by, bitv);
      } else {
        REMOVE_BIT(ch->specials.affected_by, bitv);
        mod = -mod;
      }
    }
    
    
    maxabil = (IS_NPC(ch) ? 25 : 18);
    
    switch (loc) {
    case APPLY_NONE: break; case APPLY_BV2: break;
	
    case APPLY_STR: GET_STR(ch) += mod; break;
    case APPLY_DEX: GET_DEX(ch) += mod; break;
    case APPLY_INT: GET_INT(ch) += mod; break;
    case APPLY_WIS: GET_WIS(ch) += mod; break;
    case APPLY_CON: GET_CON(ch) += mod; break;
    case APPLY_CHA: GET_CHA(ch) += mod; break;
	
    case APPLY_CLASS:
	/* ??? GET_CLASS(ch) += mod; */
	break;
	
    case APPLY_LEVEL:
	/* ??? GET_LEVEL(ch) += mod; */
	break;
	
    case APPLY_AGE:ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);break;
	
    case APPLY_CHAR_WEIGHT:GET_WEIGHT(ch) += mod;break;
	
    case APPLY_CHAR_HEIGHT:GET_HEIGHT(ch) += mod;break;
	
    case APPLY_MANA:ch->points.max_mana += mod;break;
	
    case APPLY_HIT:ch->points.max_hit += mod;break;
	
    case APPLY_MOVE:ch->points.max_move += mod;break;
	
    case APPLY_GOLD:break;

    case APPLY_EXP:break;

    case APPLY_AC:GET_AC(ch) += mod;break;

    case APPLY_HITROLL:GET_HITROLL(ch) += mod;break;
	
    case APPLY_DAMROLL:GET_DAMROLL(ch) += mod;break;
	
    case APPLY_SAVING_PHYSICAL:ch->specials2.resistances[0] += mod;break;
	
    case APPLY_SAVING_MENTAL:ch->specials2.resistances[1] += mod;break;
	
    case APPLY_SAVING_MAGIC:ch->specials2.resistances[2] += mod;break;
	
    case APPLY_SAVING_POISON:ch->specials2.resistances[3] += mod;break;
	
    case APPLY_MAGIC_RESISTANCE:ch->specials2.resistances[4] += mod;break;
	
    default:
	sprintf(buf2, "Unknown apply adjust attempt (location %d) (handler.c, affect_modify).", loc);
	log(buf2);
	break;
	
    } /* switch */
}


/* Modifies the character's innates for quest races - Helm */
void innate_check(struct char_data *ch)
{
  int i;
  struct affected_type af, *aff;

  i = GET_RACE(ch);

  switch(i) {
  case RACE_ANGEL:
    if (GET_ALIGNMENT(ch) >= 350) {
      affect_from_char(ch, SPELL_SANCTUARY);
      af.next = NULL;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.type = SPELL_SANCTUARY;
      af.duration = DURATION_INNATE;
      af.bitvector = AFF_SANCTUARY;
      affect_to_char(ch, &af);
    } else
      for (aff = ch->affected; aff; aff = aff->next)
	if ((aff->type == SPELL_SANCTUARY) &&
	    (aff->duration == DURATION_INNATE))
	  affect_remove(ch, aff);
    break;    
  default: break;
  }
}

/* Modifies the character's stats for quest races - Helm */
void stat_check(struct char_data *ch)
{
     int i = 0, stat = 0;

     if (IS_NPC(ch)) 
	  i = 0;
     else {
	  i = GET_ADD(ch);
	  if (GET_STR(ch) >= 18) 
	       GET_ADD(ch) = MIN(i, 100);

	  i = GET_RACE(ch);

	  /* Special modifiers */
	  switch(i) {
	  case RACE_VAMPIRE :
	       stat = MAX(5, GET_COND(ch, THIRST));
	       GET_DEX(ch) = stat;
	       GET_INT(ch) = stat;
	       GET_WIS(ch) = stat;
	       GET_CON(ch) = stat;
	       GET_STR(ch) = stat;
	       GET_CHA(ch) = stat;
	       break;
	  case  RACE_DEMON :
	       if (GET_ALIGNMENT(ch) <= -900) stat = 24;
	       else if (GET_ALIGNMENT(ch) <= -500) stat = 22;
	       else if (GET_ALIGNMENT(ch) <= -350) stat = 20;
	       else if (GET_ALIGNMENT(ch) <= -150) stat = 18;
	       else if (GET_ALIGNMENT(ch) <=  150) stat = 14;
	       else if (GET_ALIGNMENT(ch) <=  350) stat = 7;
	       else if (GET_ALIGNMENT(ch) <= 500) stat = 5;
	       else stat = 3;
	       GET_DEX(ch) = stat;
	       GET_INT(ch) = stat;
	       GET_WIS(ch) = stat;
	       GET_CON(ch) = stat;
	       GET_STR(ch) = stat;
	       GET_CHA(ch) = stat;
	       break;
          case RACE_ANGEL :
	       if (GET_ALIGNMENT(ch) >= 900) stat = 20;
	       else if (GET_ALIGNMENT(ch) >= 500) stat = 19;
	       else if (GET_ALIGNMENT(ch) >= 350) stat = 18;
	       else if (GET_ALIGNMENT(ch) >= 150) stat = 16;
	       else if (GET_ALIGNMENT(ch) >= -150) stat = 14;
	       else if (GET_ALIGNMENT(ch) >= -350) stat = 7;
	       else if (GET_ALIGNMENT(ch) >= -500) stat = 5;
	       else stat = 3;
	       GET_DEX(ch) = stat;
	       GET_INT(ch) = stat;
	       GET_WIS(ch) = stat;
	       GET_CON(ch) = stat;
	       GET_STR(ch) = stat;
	       GET_CHA(ch) = stat;	  
	       break;
	  default : break;
	  }
     }

     /* Race Max for Stats */
     GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), dex_max[i]));
     GET_INT(ch) = MAX(0, MIN(GET_INT(ch), int_max[i]));
     GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), wis_max[i]));
     GET_CON(ch) = MAX(0, MIN(GET_CON(ch), con_max[i]));
     GET_STR(ch) = MAX(0, MIN(GET_STR(ch), str_max[i]));
     GET_CHA(ch) = MAX(0, MIN(GET_CHA(ch), cha_max[i]));
     
     return;
}


/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void	affect_total(struct char_data *ch)
{
    struct affected_type *af;
    int	i, j;
    
    for (i = 0; i < MAX_WEAR; i++) {
	if (ch->equipment[i])
	    for (j = 0; j < MAX_OBJ_AFFECT; j++)
		affect_modify(ch, ch->equipment[i]->affected[j].location,
			      ch->equipment[i]->affected[j].modifier,
			      0, FALSE);
    }
    
    
    for (af = ch->affected; af; af = af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
    
    ch->tmpabilities = ch->abilities;
    
    for (i = 0; i < MAX_WEAR; i++) {
	if (ch->equipment[i])
	    for (j = 0; j < MAX_OBJ_AFFECT; j++)
		affect_modify(ch, ch->equipment[i]->affected[j].location,
			      ch->equipment[i]->affected[j].modifier,
	                      0, TRUE);
    }

    
    for (af = ch->affected; af; af = af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
    
    /* Make certain values are between 0..25, not < 0 and not > 25! */
    /* Change by Petrus.  Race differences */

    /* New stat check procedure */
    stat_check(ch);
}




/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void	affect_to_char( struct char_data *ch, struct affected_type *af)
{
  struct affected_type *affected_alloc;
    
  /* change that will fix the 'cast sanc' on sanced mobs -P*/
  if (IS_NPC(ch) && IS_SET(ch->specials.affected_by, af->bitvector))
    return;

  CREATE(affected_alloc, struct affected_type, 1);
    
  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;
    
  affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  affect_total(ch);
  if (ch->desc && STATE(ch->desc) == CON_PLYNG)
    aff_to_screen(ch->desc);
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply                                                */
void	affect_remove( struct char_data *ch, struct affected_type *af )
{
    struct affected_type *hjp;
    
    if (!ch->affected)
	return;

    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
    
   /* remove structure *af from linked list */
    if (ch->affected == af) {
	/* remove head of list */
	ch->affected = af->next;
    } else {
      for (hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next)
	  ;
      
      if (hjp->next != af) {
	 log("SYSERR: FATAL : Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
	 exit(1);
     }
      hjp->next = af->next; /* skip the af element */
   }
    
    free (af);
    
    affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void	affect_from_char( struct char_data *ch, int skill)
{
    struct affected_type *hjp;
    
    for (hjp = ch->affected; hjp; hjp = hjp->next)
      if (hjp->type == skill)
	  affect_remove(ch, hjp);
    
    if (ch->desc && STATE(ch->desc) == CON_PLYNG)
	aff_to_screen(ch->desc);
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell( struct char_data *ch, int skill )
{
    struct affected_type *hjp;
    
    for (hjp = ch->affected; hjp; hjp = hjp->next)
	if (hjp->type == skill)
	    return(TRUE);
    
    return(FALSE);
}


void	affect_join( struct char_data *ch, struct affected_type *af,
		    bool avg_dur, bool avg_mod )
{
  struct affected_type *hjp;
  bool found = FALSE;
    
  for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
    if ( hjp->type == af->type && hjp->location == af->location) {
	    
      if (hjp->duration != DURATION_PERMANENT &&
	  hjp->duration != DURATION_INNATE) {
		
	af->duration = MIN(af->duration += hjp->duration, 999);
	if (avg_dur)
	  af->duration /= 2;
		
	af->modifier = MIN(af->modifier += hjp->modifier, 99);
	if (avg_mod)
	  af->modifier /= 2;

	affect_remove(ch, hjp);
	affect_to_char(ch, af);
      }
      found = TRUE;
    }
  }
  if (!found)
    affect_to_char(ch, af);
}


/* Insert an affect_type in a room_data structure
   Automatically sets apropriate bit 
void	affect_to_room(sh_int roomnr, struct room_affect_type *af)
{
    int r_room;
    struct room_affect_type *tmp_af;
    
    assert(af && af->type && af->duration > 0);
    
    if ((r_room = real_room(roomnr)) < 0) {
	log("SYSERR: FATAL : Could not locate the real room. (handler.c, affect_to_room)");
	exit(1);
    }
    
    if (IS_SET(world[r_room]->room_flags, af->bitvector))
	return;
    
    tmp_af = world[r_room]->affects;
    
    while(tmp_af) {
	if (tmp_af->type == af->type) {
	    tmp_af->duration = MAX(tmp_af->duration, af->duration);
	    return;
	}
    }
    
    CREATE(tmp_af, struct room_affect_type, 1);
    
    *tmp_af = *af;
    tmp_af->next = world[r_room]->affects;
    world[r_room]->affects = tmp_af;
    tmp_af->next_in_list = room_affect_list;
    room_affect_list = tmp_af;
    tmp_af->room_num = r_room;
    
    SET_BIT(world[r_room]->room_flags, tmp_af->bitvector);
}
*/

/* Remove a magic affection from a room 
void	affect_from_room(sh_int roomnr, int spell)
{
    int r_room;
    struct room_affect_type *tmp_af, *af;
    
    if ((r_room = real_room(roomnr)) < 0) {
	log("SYSERR: FATAL : Could not locate the real room. (handler.c, affect_from_room)");
	exit(1);
    }
    
    for (af = world[r_room]->affects; af; tmp_af = af->next)
	if (af->type == spell) {
	    REMOVE_BIT(world[r_room]->room_flags, af->bitvector);
	    break;
	}
	    
    if (!af) {
	log("SYSERR: Trying to remove affect from a room without the affected type. (handler.c, affect_from_room)");
	return;
    }
    
 
    if (world[r_room]->affects == af) {
	world[r_room]->affects = af->next;
    } else {
	for (tmp_af = world[r_room]->affects; 
	     (tmp_af->next) && (tmp_af->next->type != spell);
	     tmp_af = tmp_af->next);
	if (tmp_af->next->type != spell) {
	    log("SYSERR: FATAL : Could not locate affected_type in ROOM affects. (handler.c, affect_from_room)");
	    exit(1);
	}
	tmp_af->next = af->next; 
    }
    
  
    if (room_affect_list == af) {
        room_affect_list = af->next;
    } else {
	for (tmp_af = room_affect_list; 
	     (tmp_af->next) && (tmp_af->next->type != spell);
	     tmp_af = tmp_af->next);
	if (tmp_af->next->type != spell) {
	    log("SYSERR: FATAL : Could not locate affected_type in Global affects. (handler.c, affect_from_room)");
	    exit(1);
	}
	
	tmp_af->next = af->next; 
    }
    
    free (af);
}
*/



/* insert_to_char_list:  Insert a character, player or mob to the character
 * list.  Assumes that mob_list points at the place where mobs start in 
 * character list.
 */
void insert_to_char_list(struct char_data *ch)
{
    if (IS_NPC(ch)) {   /* Is mob */
	ch->next = *mob_list;
	*mob_list = ch;
    } else {            /* player */
	ch->next = character_list;
	character_list = ch;
	if (mob_list == &character_list)
	    mob_list = &ch->next;
    }
}


/* extract_from_char_list: extracts a char from char_list
 */
void extract_from_char_list(struct char_data *ch)
{
    struct char_data *k;
    
    if (ch == character_list) {
	character_list = ch->next;
	
	if (mob_list == &ch->next)
	    mob_list = &character_list;
    } else {
	for (k = character_list; (k) && (k->next != ch); k = k->next);
	if (k) {
	    k->next = ch->next;
	    if (mob_list == &ch->next)
		mob_list = &k->next;
	} else {
	    log("SYSERR: Trying to remove ?? from character_list. (handler.c, extract_from_char_list)");
	    abort();
	}
    }
}

/* mob_to_character_list: put mobs behind all players in list -Petrus
 *  Will speedup get_player_vis etc
 */
void mob_to_character_list(struct char_data *mob, struct char_data **ptr)
{
    struct char_data **tmp;
    
    if (!IS_NPC(mob) || !(&ptr)) {
	mob->next = *ptr;
	*ptr = mob;
	return;
    }
    
    tmp = ptr;
    
    while ((*tmp))
    {
	if (IS_NPC(*tmp))
	    break;
	
	tmp = &(*tmp)->next;
    }
    
    if ((*tmp)) 
	ptr = tmp;
    
    mob->next = *ptr;
    *ptr = mob;
}


/* Put a char to a list using bubble sort */
void  bubble_char_to_list(struct char_data *ch, struct char_data **ptr)
{
    struct char_data **tmp;
    
    if (!IS_NPC(ch) || !(&ptr)) {
	ch->next_in_room = *ptr;
	*ptr = ch;
	return;
    }
    
    tmp = ptr;
    
    while ((*tmp)) 
    {
	if (IS_NPC(*tmp) &&
	    ch->nr == (*tmp)->nr &&
	    ch->specials.affected_by == (*tmp)->specials.affected_by &&
	    !strcmp(ch->player.short_descr, (*tmp)->player.short_descr))
	{
	    break;
	}
	
	tmp = &(*tmp)->next_in_room;
    }
    
    if ((*tmp)) 
	ptr = tmp;
    
    ch->next_in_room = *ptr;
    *ptr = ch;
}

/* move a player out of a room */
void	char_from_room(struct char_data *ch)
{
    struct char_data *i;

    if (ch->in_room == NOWHERE) {
	log("SYSERR: NOWHERE extracting char from room (handler.c, char_from_room)");
	exit(1);
    }

    if (FIGHTING(ch) != NULL)
      stop_fighting(ch);
    
    if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	  if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
	    (world[ch->in_room]->light)--;
	    
    
    if (ch == world[ch->in_room]->people)  /* head of list */
	world[ch->in_room]->people = ch->next_in_room;
    
    else /* locate the previous element */ {
	for (i = world[ch->in_room]->people; 
          i->next_in_room != ch; i = i->next_in_room);
	
	i->next_in_room = ch->next_in_room;
    }
    
    ch->in_room = NOWHERE;
    ch->next_in_room = 0;
}


/* place a character in a room */
void	char_to_room(struct char_data *ch, int room)
{
    
  if (!ch || room < 0 || room > top_of_world)
    log("SYSERR: Illegal value(s) passed to char_to_room");
  else {
    bubble_char_to_list(ch, &(world[room]->people));
    
    ch->in_room = room;

    if (ch->specials.mounting &&
	(ch->in_room != (ch->specials.mounting)->in_room))
	die_mount(ch);

    if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	    if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /*Light is ON*/
		(world[room]->light)++;
  }
}


/* New object handlers -Petrus */

/* Put an obj to a list using bubble sort */
void  bubble_obj_to_list(struct obj_data **obj, struct obj_data **ptr)
{
    struct obj_data **tmp;
    
    if (!(&ptr)) {
	*ptr = *obj;
	(*obj)->next_content = 0;
	return;
    }
    
    tmp = ptr;
    
    while ((*tmp)) 
    {
	if ((*obj)->item_number == (*tmp)->item_number &&
	    (*obj)->obj_flags.extra_flags == (*tmp)->obj_flags.extra_flags &&
	    /*  ((*obj)->obj_flags.type_flag == ITEM_MONEY ||
		!strcmp((*obj)->short_description, (*tmp)->short_description)))*/
	    !strcmp((*obj)->short_description, (*tmp)->short_description))
	{
	    break;
	}
	
	tmp = &(*tmp)->next_content;
    }
    
    if ((*tmp)) {            /* found similar obj */
	ptr = tmp;
	/*
	  if ((*obj)->obj_flags.type_flag == ITEM_MONEY) {  
	  money = create_money((*ptr)->obj_flags.value[0] +
	  (*obj)->obj_flags.value[0]);
	  money->carried_by = (*ptr)->carried_by;
	    money->in_obj = (*ptr)->in_obj;
	    money->in_room = (*ptr)->in_room;
	    
	    money->next_content = *ptr;
	    *ptr = money;
	    extract_obj((*obj));
	    extract_obj(money->next_content);
	    *obj = money;
	    return;
	    }*/
    }
    
    (*obj)->next_content = *ptr;
    *ptr = (*obj);
}
	

/* give an object to a char   */
void	obj_to_char(struct obj_data *object, struct char_data *ch)
{
    bubble_obj_to_list(&object, &ch->carrying);
    
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;

    /* set killer for ITEM_KILLER objects */
    if (IS_OBJ_STAT(object, ITEM_KILLER))
      SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
    
    /* set flag for crash-save system */
    SET_BIT(PLR_FLAGS(ch), PLR_SAVEOBJS);
}


/* take an object from a char */
void	obj_from_char(struct obj_data *object)
{
    struct obj_data *tmp;
    
    if (!object || !object->carried_by) {
	log("SYSERR: obj_from_char when is not carried by anyone.");
	return;
    }

    /* remove killer for ITEM_KILLER objects */
    if (IS_OBJ_STAT(object, ITEM_KILLER))
      REMOVE_BIT(PLR_FLAGS(object->carried_by), PLR_KILLER);

    if (object->carried_by->carrying == object)   /* head of list */
	object->carried_by->carrying = object->next_content;
    
    else {
	for (tmp = object->carried_by->carrying; 
	   tmp && (tmp->next_content != object); 
	     tmp = tmp->next_content)
	    ; /* locate previous */
	
	tmp->next_content = object->next_content;
    }
    
    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;
    object->carried_by = 0;
    object->next_content = 0;
}



/* Return the effect of a piece of armor in position eq_pos */
int	apply_ac(struct char_data *ch, int eq_pos)
{
    int factor;

    if (!ch->equipment[eq_pos])
	return 0;
    
    if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
      return 0;
    
    switch (eq_pos) {
	
    case WEAR_BODY	: factor = 3; break;	/* 300% */
    case WEAR_HEAD	: factor = 2; break;	/* 200% */
    case WEAR_LEGS	: factor = 2; break;	/* 200% */
    case HOLD           : factor = 0; break;    /*  0% */
	default		: factor = 1; break;	/* all others 100% */
    }
    
    return (factor * ch->equipment[eq_pos]->obj_flags.value[0]);
}



void	equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int	j;
    struct affected_type af;
    
    if (pos < 0 || pos >= MAX_WEAR) {
	sprintf(buf2, "SYSERR: Equip pos out of bounds: %s, %s", GET_NAME(ch),
		obj->short_description);
	log(buf2);
	return;
    }

    if (ch->equipment[pos]) {
	sprintf(buf2, "SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
		obj->short_description);
	log(buf2);
	return;
    }
    
    if (obj->carried_by) {
	log("SYSERR: EQUIP: Obj is carried_by when equip.");
	return;
    }
    
    if (obj->in_room != NOWHERE) {
	sprintf(buf2, "SYSERR: Obj is in room when equip: %s, %s, %d", GET_NAME(ch), obj->short_description, obj->in_room);
	log(buf2);
	return;
    }
    
    if (!PRF_FLAGGED(ch, PRF_HOLYLIGHT)	 && IN_ROOM(ch) != NOWHERE &&
	((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) || 
	(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) || 
	(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))) 
    {
	act("You are zapped by $p and instantly release it.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n is zapped by $p and instantly releases it.", FALSE, ch, obj, 0, TO_ROOM);
	obj_to_char(obj, ch); /* changed to drop in inventory instead of ground */

        /* Kill Ghost Stats from Object as well - required for autosave - Bod */
        GET_HIT(ch)  = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
        GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
        GET_MOVE(ch) = MIN(GET_MOVE(ch), GET_MAX_MOVE(ch));

        return;
    }

    if (CLANEQ(obj))
	 if(((CLANEQ_CLAN(obj) != CLAN(ch)) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) ||
            (CLAN_LEVEL(ch) <= 1)) {
	      act("You are zapped by $p and release it.", FALSE, ch, obj, 0, TO_CHAR);
	      act("$n is zapped by $p and release it.", FALSE, ch, obj, 0, TO_ROOM);
	      obj_to_char(obj, ch);

            /* Kill Ghost Stats from Object as well - required for autosave - Bod */
            GET_HIT(ch)  = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
            GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
            GET_MOVE(ch) = MIN(GET_MOVE(ch), GET_MAX_MOVE(ch));

            return;
	 }
    
    ch->equipment[pos] = obj;
    obj->used_by = ch;
    obj->worn_on = pos;

                               /* For 2H weapons 
    if (CAN_WEAR(obj, ITEM_WIELD_2H))  {
      if (pos == WIELD) ch->equipment[HOLD]  = obj;
      if (pos == HOLD)  ch->equipment[WIELD] = obj;
      }*/
    
    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) -= apply_ac(ch, pos);
    
    if (IN_ROOM(ch) != NOWHERE) {
      if ( GET_ITEM_TYPE(obj) == ITEM_LIGHT)
          if (obj->obj_flags.value[2])
                world[ch->in_room]->light++;
    }


    for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
		      obj->affected[j].modifier, 0, TRUE);
 
    /* Fix to make object bits work OK */
    if (obj->obj_flags.bitvector) {
      /* Can only apply it if the char doesn't already have the bit */
      if (!IS_AFFECTED(ch, obj->obj_flags.bitvector)) {
        af.type       = SPELL_FROM_ITEM;
        af.bitvector  = obj->obj_flags.bitvector;
        af.modifier   = 0;
        af.location   = APPLY_NONE;
        af.duration   = DURATION_PERMANENT; /* Permanent until removed */
        affect_to_char(ch, &af);
      }
    }

    affect_total(ch);
}



struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int	j;
    struct obj_data *obj;
    struct affected_type *af;
    
    assert(pos >= 0 && pos < MAX_WEAR);
    assert(ch->equipment[pos]);
    
    obj = ch->equipment[pos];
    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) += apply_ac(ch, pos);
    
    if (IN_ROOM(ch) != NOWHERE) {
      if ( GET_ITEM_TYPE(obj) == ITEM_LIGHT)
          if (obj->obj_flags.value[2])
                world[ch->in_room]->light--;
    }

    ch->equipment[pos] = 0;
    obj->used_by = NULL;
    obj->worn_on = -1;

                          /* for 2H weapons 
    if (CAN_WEAR(obj, ITEM_WIELD_2H))  {
      if (pos == WIELD) ch->equipment[HOLD]  = 0;
      if (pos == HOLD)  ch->equipment[WIELD] = 0;
    }*/
    
    for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
		      obj->affected[j].modifier, 0, FALSE);

    /* Fix to make object bits work OK */
    if (obj->obj_flags.bitvector)
      for (af = ch->affected; af; af = af->next)
        if ((af->type == SPELL_FROM_ITEM) &&
            (af->bitvector == obj->obj_flags.bitvector)) {
          if (af->duration != DURATION_PERMANENT)
            mudlog("SYSERR: Item bitvector set is not permanent", 
                    BRF, LEVEL_LESSER, TRUE);
          affect_remove(ch, af);
          break;
        }

    affect_total(ch);
    
    return(obj);
}

int
get_number(char **name)
{
  int	i;
  char	*ppos;
  char	number[MAX_INPUT_LENGTH] = "";
    
  if ((ppos = strchr(*name, '.'))) {
    *ppos++ = '\0';
    strcpy(number, *name);
    strcpy(*name, ppos);
	
    for (i = 0; *(number + i); i++)
      if (!isdigit(*(number + i)))
	return(0);
	
    return(atoi(number));
  }
    
  return(1);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
    struct obj_data *i;
    int	j, number;
    char	tmpname[MAX_INPUT_LENGTH];
    char	*tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return(0);
    
    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name)) {
	    if (j == number)
		return(i);
	    j++;
	}
    
    return(0);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i;
    
    for (i = list; i; i = i->next_content)
	if (i->item_number == num)
	    return(i);
    
   return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = object_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->name)) {
	 if (j == number)
	    return(i);
	 j++;
      }

   return(0);
}





/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
   struct obj_data *i;

   for (i = object_list; i; i = i->next)
      if (i->item_number == nr)
	 return(i);

   return(0);
}

/* search a room for a char, and return a pointer if found..  */
struct char_data *
get_char_room(char *name, int room)
{
  struct char_data *i;
  int	j, number;
  char	tmpname[MAX_INPUT_LENGTH];
  char	*tmp;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(number = get_number(&tmp)))
    return(0);

  for (i = world[room]->people, j = 1; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, i->player.name)) {
      if (j == number)
	return(i);
      j++;
    }

  return(0);
}

/* search all over the world for a char, and return a pointer if found */
struct char_data *
get_char(char *name)
{
  struct char_data *i;
  int	j, number;
  char	tmpname[MAX_INPUT_LENGTH];
  char	*tmp;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(number = get_number(&tmp)))
    return(0);

  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, i->player.name)) {
      if (j == number)
	return(i);
      j++;
    }

  return(0);
}

/* search all over the world for a char num, and return a pointer if found */
struct char_data *
get_char_num(int nr)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (i->nr == nr)
      return(i);

  return(0);
}


/* put an object in a room */
void
obj_to_room(struct obj_data *object, int room)
{
  bubble_obj_to_list(&object, &(world[room]->contents));
  object->in_room = room;
  object->carried_by = 0;
}

/* Take an object from a room */
void
obj_from_room(struct obj_data *object)
{
  struct obj_data *i;

  /* remove object from room */

  if (object == world[object->in_room]->contents)  /* head of list */
    world[object->in_room]->contents = object->next_content;

  else /* locate previous element in list */	 {
    for (i = world[object->in_room]->contents; i && 
	   (i->next_content != object); i = i->next_content)
      ;

    i->next_content = object->next_content;
  }

  object->in_room = NOWHERE;
  object->next_content = 0;
}

/* put an object in an object (quaint)  */
void
obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
  struct obj_data *tmp_obj;

  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;

  for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

  /* top level object.  Subtract weight from inventory if necessary. */
  GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
  if (tmp_obj->carried_by)
    IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
}

/* remove an object from an object */
void
obj_from_obj(struct obj_data *obj)
{
  struct obj_data *tmp, *obj_from;

  if (obj->in_obj) {
    obj_from = obj->in_obj;
    if (obj == obj_from->contains)   /* head of list */
      obj_from->contains = obj->next_content;
    else {
      for (tmp = obj_from->contains; 
	   tmp && (tmp->next_content != obj); 
	   tmp = tmp->next_content)
	; /* locate previous */

      if (!tmp) {
	perror("SYSERR: Fatal error in object structures.");
	abort();
      }

      tmp->next_content = obj->next_content;
    }


    /* Subtract weight from containers container */
    for (tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

    /* Subtract weight from char that carries the object */
    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    if (tmp->carried_by)
      IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);

    obj->in_obj = 0;
    obj->next_content = 0;
  } else {
    perror("SYSERR: Trying to object from object when in no object.");
    abort();
  }
}

/* Set all carried_by to point to new owner */
void
object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}

/* Extract an object from the world */
void extract_obj(struct obj_data * obj)
{
  struct obj_data *temp;

  if (obj->used_by != NULL)
    if (unequip_char(obj->used_by, obj->worn_on) != obj)
      log("SYSERR: Inconsistent used_by and worn_on pointers!!");
  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->in_obj)
    obj_from_obj(obj);

  /* Get rid of the contents of the object, as well. */
  while (obj->contains)
    extract_obj(obj->contains);


  REMOVE_FROM_LIST(obj, object_list, next);

  if (obj->item_number >= 0)
    (obj_index[obj->item_number].number)--;
  free_obj(obj);
}



void	update_object( struct obj_data *obj, int use)
{

   if (obj->obj_flags.timer > 0)
      obj->obj_flags.timer -= use;
   if (obj->contains)
      update_object(obj->contains, use);
   if (obj->next_content)
      update_object(obj->next_content, use);
}


void	update_char_objects( struct char_data *ch )
{
  int	i;
 
  if (ch->equipment[WEAR_LIGHT])
    if (GET_ITEM_TYPE(ch->equipment[WEAR_LIGHT]) == ITEM_LIGHT)
      if (GET_ITEM_VALUE(ch->equipment[WEAR_LIGHT], 2) > 0) {
	i = --GET_ITEM_VALUE(ch->equipment[WEAR_LIGHT], 2);
	if (i == 1) {
	  act("Your $o begins to flicker and fade.", FALSE, ch, ch->equipment[WEAR_LIGHT], 0, TO_CHAR);
	  act("$n's $o begins to flicker and fade.", FALSE, ch, ch->equipment[WEAR_LIGHT], 0, TO_ROOM);
	} else if (i == 0) {
	  act("Your $o sputters out and dies.", FALSE, ch, ch->equipment[WEAR_LIGHT], 0, TO_CHAR);
	  act("$n's $o sputters out and dies.", FALSE, ch, ch->equipment[WEAR_LIGHT], 0, TO_ROOM);
	  (world[ch->in_room]->light)--;
	}
      }
  
  for (i = 0; i < MAX_WEAR; i++)
    if (ch->equipment[i])
      update_object(ch->equipment[i], 2);
  
  if (ch->carrying)
    update_object(ch->carrying, 1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void	extract_char(struct char_data *ch)
{
    char buf[200];
    struct obj_data *i;
    struct char_data *k, *next_char;
    struct descriptor_data *t_desc, *c_desc;
    int	l, was_in, freed = 0, dead;
    
    extern struct char_data *combat_list;
    extern char *DEAD_MESSG;
    
    ACMD(do_save);
    ACMD(do_return);
    
    void die_follower(struct char_data *ch);
    
    dead = (GET_POS(ch) == POS_DEAD);

    if (!IS_NPC(ch) && !ch->desc) {
	for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
	    if (t_desc->original == ch)
		do_return(t_desc->character, "", 0, 0);
    }


    /* Added by Phobos */
    /* Disconnects all players with your id_num when you die */
    /* Prevents gold duping */

    if(!IS_NPC(ch)) {
	for (c_desc = descriptor_list; c_desc; c_desc = c_desc->next) {
	     if(c_desc->character) {	
	 	    if((GET_IDNUM(c_desc->character) == GET_IDNUM(ch)) && (!(ch->desc == c_desc))) {
			close_socket(c_desc);
			sprintf(buf, "Multiple connection detected, %s disconnected.", ch->player.name);
     			mudlog(buf, CMP, LEVEL_IMMORT, FALSE); 
	    	    }
	     } 
        }
    }

    if (ch->in_room == NOWHERE) {
	log("SYSERR: NOWHERE extracting char. (handler.c, extract_char)");
	exit(1);
    }

    die_protector(ch);
    die_mount(ch);

    if (ch->followers || ch->master)
	die_follower(ch);

    if (ch->desc) {
	/* Forget snooping */
	if (ch->desc->snooping)
	    ch->desc->snooping->snoop_by = 0;

	if (ch->desc->snoop_by) {
	    SEND_TO_Q("Your victim is no longer among us.\r\n",
			 ch->desc->snoop_by);
	    ch->desc->snoop_by->snooping = 0;
	}
	
	ch->desc->snooping = ch->desc->snoop_by = 0;
    }

    if (ch->carrying) {
	/* transfer ch's objects to room */
	if (world[ch->in_room]->contents)  /* room nonempty */ {
	    /* locate tail of room-contents */
	    for (i = world[ch->in_room]->contents; i->next_content; 
		 i = i->next_content);
	    /* append ch's stuff to room-contents */
	    i->next_content = ch->carrying;
	} else
	    world[ch->in_room]->contents = ch->carrying;
	/* connect the stuff to the room */
	for (i = ch->carrying; i; i = i->next_content) {
	    i->carried_by = 0;
	    i->in_room = ch->in_room;
	}
    }
    
    if (ch->specials.fighting)
	stop_fighting(ch);
    
    for (k = combat_list; k ; k = next_char) {
	next_char = k->next_fighting;
	if (k->specials.fighting == ch)
	    stop_fighting(k);
    }

    /* Must remove from room before removing the equipment! */
    was_in = ch->in_room;
    char_from_room(ch);
    
    /* clear equipment_list */
    for (l = 0; l < MAX_WEAR; l++)
	if (ch->equipment[l])
	    obj_to_room(unequip_char(ch, l), was_in);
    
    extract_from_char_list(ch);
    
    if (ch->desc) {
	if (ch->desc->original) {
	    do_return(ch, "", 0, 0);
      }
	ch->specials2.load_room = world[was_in]->number;
	save_char(ch, world[was_in]->number, 2); /* I was here -Petrus */
	Crash_delete_crashfile(ch);
   }
    
    if (IS_NPC(ch)) {
	if (ch->nr > -1) /* if mobile */
	    mob_index[ch->nr].number--;
	clearMemory(ch);   /* Only NPC's can have memory */
	free_char(ch);
	freed = 1;
    }

   
    if (!freed && ch->desc != NULL) {
      if (dead) {
        STATE(ch->desc) = CON_DEADWAIT;
        SEND_TO_Q(DEAD_MESSG, ch->desc);
      } else {
        STATE(ch->desc) = CON_SLCT;
        if (PRF_FLAGGED(ch, PRF_IBM_PC)) 
	  SEND_TO_Q(PC_MENU, ch->desc);
        else
	  SEND_TO_Q(MENU, ch->desc);
      }
    } else {  /* if a player gets purged from within the game */
      if (!freed)
	free_char(ch);
    }
    
}


/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */

struct char_data *
get_char_room_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int	j, number;
  char	tmpname[MAX_INPUT_LENGTH];
  char	*tmp;

  alias_interpreter(ch, name);   /* Alias name replace -Petrus */

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(number = get_number(&tmp)))
    return(0);
  
  for (i = world[ch->in_room]->people, j = 1; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, i->player.name))
      if (CAN_SEE(ch, i))	 {
	if (j == number)
	  return(i);
	j++;
      }
  
  /* Try abbrev */
  if (strlen(name) > 1 && number == 1) {
    for (i = world[ch->in_room]->people; i; i = i->next_in_room)
      if (!IS_NPC(i) && is_abbrev(tmp, i->player.name) && CAN_SEE(ch, i))
	return i;
  }
  
  return(0);
}

struct char_data *
get_player_vis_exact(struct char_data *ch, char *name)
{
     struct char_data *i = NULL;

     alias_interpreter(ch, name);   /* Alias name replace -Petrus */

   /* New: Assumes the players are ALL at the beginning of list */
     for (i = character_list; i && !IS_NPC(i); i = i->next)
	  if (!str_cmp(i->player.name, name) && CAN_SEE(ch, i))
	       return i;

     return NULL;
}

struct char_data *
get_player_vis(struct char_data *ch, char *name)
{
   struct char_data *i = NULL, *tmp = NULL;

   alias_interpreter(ch, name);   /* Alias name replace -Petrus */

   /* New: Assumes the players are ALL at the beginning of list */
   for (i = character_list; i && !IS_NPC(i); i = i->next)
      if (!str_cmp(i->player.name, name) && CAN_SEE(ch, i))
	 return i;
   
   /* Try abbrev */
   if (strlen(name) > 1) {
	for (i = character_list; i && !IS_NPC(i); i = i->next) {
	     if (isname(name, i->player.name) && CAN_SEE(ch, i))
		  return i;
	     if (is_abbrev(name, i->player.name) && CAN_SEE(ch, i))
		  tmp = i;
	    }
	if (tmp) return tmp;
   }

   return 0;
}

struct char_data *
get_char_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int	j, number;
  char	tmpname[MAX_INPUT_LENGTH];
  char	*tmp;
  
  alias_interpreter(ch, name);   /* Alias name replace -Petrus */

  /* check location */
  if ((i = get_char_room_vis(ch, name)))
    return(i);

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(number = get_number(&tmp)))
    return(0);

  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, i->player.name))
      if (IN_ROOM(i) != NOWHERE)
	if (CAN_SEE(ch, i)) {
	  if (j == number)
	    return(i);
	  j++;
	}

  return(0);
}

struct obj_data *
get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = list, j = 1; i && (j <= number); i = i->next_content)
      if (isname(tmp, i->name))
	 if (CAN_SEE_OBJ(ch, i)) {
	    if (j == number)
	       return(i);
	    j++;
	 }
   return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   /* scan items carried */
   if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
      return(i);

   /* scan room */
   if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room]->contents)))
      return(i);

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   /* ok.. no luck yet. scan the entire obj list   */
   for (i = object_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->name))
	 if (CAN_SEE_OBJ(ch, i)) {
	    if (j == number)
	       return(i);
	    j++;
	 }
   return(0);
}



struct obj_data *get_object_in_equip_vis(struct char_data *ch,
char *arg, struct obj_data *equipment[], int *j)
{
  int number, k;
  char	tmpname[MAX_INPUT_LENGTH];
  char	*tmp;

  strcpy(tmpname, arg);
  tmp = tmpname;
  number = get_number(&tmp);

  for ((*j) = 0, k = 1; (*j) < MAX_WEAR && k <= number; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	if (isname(tmp, equipment[(*j)]->name)) {
	  if (k == number)
	    return(equipment[(*j)]);
	  k++;
	}
	    
   return (0);
}


struct obj_data *create_money(int amount)
{
   char	buf[200];

   struct obj_data *obj;
   struct extra_descr_data *new_descr;

   if (amount <= 0) {
      log("SYSERR: Try to create negative or 0 money.");
      exit(1);
   }

   CREATE(obj, struct obj_data, 1);
   CREATE(new_descr, struct extra_descr_data, 1);
   clear_object(obj);
   if (amount == 1) {
      obj->name = strdup("coin gold");
      obj->short_description = strdup("a gold coin");
      obj->description = strdup("One miserable gold coin is lying here.");
      new_descr->keyword = strdup("coin gold");
      new_descr->description = strdup("It's just one miserable little gold coin.");
   } else {
      obj->name = strdup("coins gold");
      if (amount <= 100) {
	 obj->short_description = strdup("a small pile of gold coins");
	 obj->description = strdup("A small pile of gold coins is lying here.");
      } else if (amount <= 1000) {
	 obj->short_description = strdup("a pile of gold coins");
	 obj->description = strdup("A pile of gold coins is lying here.");
      } else if (amount <= 25000) {
	 obj->short_description = strdup("a large heap of gold coins");
	 obj->description = strdup("A large heap of gold coins is lying here.");
      } else if (amount <= 500000) {
	 obj->short_description = strdup("a huge mound of gold coins");
	 obj->description = strdup("A huge mound of gold coins is lying here.");
      } else {
	 obj->short_description = strdup("an enormous mountain of gold coins");
	 obj->description = strdup("An enormous mountain of gold coins is lying here.");
      }

      new_descr->keyword = strdup("coins gold");
      if (amount < 10) {
	 sprintf(buf, "There are %d coins.", amount);
	 new_descr->description = strdup(buf);
      } else if (amount < 100) {
	 sprintf(buf, "There are about %d coins.", 10 * (amount / 10));
	 new_descr->description = strdup(buf);
      } else if (amount < 1000) {
	 sprintf(buf, "It looks to be about %d coins.", 100 * (amount / 100));
	 new_descr->description = strdup(buf);
      } else if (amount < 100000) {
	 sprintf(buf, "You guess there are, maybe, %d coins.",
	     1000 * ((amount / 1000) + number(0, (amount / 1000))));
	 new_descr->description = strdup(buf);
      } else
	 new_descr->description = strdup("There are a LOT of coins.");
   }

   new_descr->next = 0;
   obj->ex_description = new_descr;

   obj->obj_flags.type_flag = ITEM_MONEY;
   obj->obj_flags.wear_flags = ITEM_TAKE;
   obj->obj_flags.value[0] = amount;
   obj->obj_flags.cost = amount;
   obj->item_number = -1;

   obj->next = object_list;
   object_list = obj;

   return(obj);
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int	generic_find(char *arg, int bitvector, struct char_data *ch,
struct char_data **tar_ch, struct obj_data **tar_obj)
{
   static char	*ignore[] = {
      "the",
      "in",
      "on",
      "at",
      "\n"       };

   int	i;
   char	name[256];
   bool found;

   found = FALSE;


   /* Eliminate spaces and "ignore" words */
   while (*arg && !found) {

      for (; *arg == ' '; arg++)
	 ;

      for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++)
	 ;
      name[i] = 0;
      arg += i;
      if (search_block(name, ignore, TRUE) > -1)
	 found = TRUE;

   }

   if (!name[0])
      return(0);

   *tar_ch  = 0;
   *tar_obj = 0;

   if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
      if ((*tar_ch = get_char_room_vis(ch, name))) {
	 return(FIND_CHAR_ROOM);
      }
   }

   if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
      if ((*tar_ch = get_char_vis(ch, name))) {
	 return(FIND_CHAR_WORLD);
      }
   }

   if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
      for (found = FALSE, i = 0; i < MAX_WEAR && !found; i++)
	 if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
	    *tar_obj = ch->equipment[i];
	    found = TRUE;
	 }
      if (found) {
	 return(FIND_OBJ_EQUIP);
      }
   }

   if (IS_SET(bitvector, FIND_OBJ_INV)) {
      if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
	 return(FIND_OBJ_INV);
      }
   }

   if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
      if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room]->contents))) {
	 return(FIND_OBJ_ROOM);
      }
   }

   if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
      if ((*tar_obj = get_obj_vis(ch, name))) {
	 return(FIND_OBJ_WORLD);
      }
   }

   return(0);
}


/* a function to scan for "all" or "all.x" */
int	find_all_dots(char *arg)
{
   if (!strcmp(arg, "all"))
      return FIND_ALL;
   else if (!strncmp(arg, "all.", 4)) {
      strcpy(arg, arg+4);
      return FIND_ALLDOT;
   } else
      return FIND_INDIV;
}


/* function find_room_by_name(char *roomname)  -Petrus
 */
sh_int  find_room_by_name(char *roomname)
{
    int i, num;

    if (!(num = get_number(&roomname)))
	return NOWHERE;
    
    for (i = 0; i < top_of_world; i++)
	if (isname(roomname, world[i]->name) && !(--num))
	    return i;
    
    return NOWHERE;
    
}

/* Pointer validation routine: returns 1 if pointer is valid
 * and 0 if not */

/* Validate that this char pointer is still valid */
int validate_char(struct char_data *ch)
{
  register struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (i == ch) return(1);

  return(0);
}


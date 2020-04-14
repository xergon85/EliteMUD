/************************************************************************
*   File: act.informative.c                             Part of EliteMUD  *
*  Usage: Player-level commands of an informative nature                  *
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
#include "screen.h"
#include "functions.h"
#include "scrcol.h"

/* extern variables */
extern struct room_data **world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[20][40];
extern struct command_info cmd_info[];
extern struct clan_data *clan_list;
extern struct immcommand_info immcmd_info[];
 
extern char	*credits;
extern char	*news;
extern char	*info;
extern char	*motd;
extern char	*imotd;
extern char	*newbie;
extern char	*wizlist;
extern char	*immlist;
extern char     *remlist;
extern char	*policies;
extern char	*handbook;
extern char	*dirs[];
extern char	*where[];
extern char	*color_liquid[];
extern char	*fullness[];
extern char	*connected_types[];
extern char	*class_abbrevs[];
extern char     *class_dual[4][4];
extern char     *race_table[];
extern char     *class_multi[];
extern char	*room_bits[];
extern char     *sector_types[];
extern char     *pc_class_types[];
extern char     *clan_ranks[][3];
extern char     *which_team[];

extern int MAX_PLAYERS;
int        max_so_far = 1;    /* The max in game this period */

extern int exp_needed(struct char_data *ch);

/* intern functions & vars*/
int	num_of_cmds;
void	list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show);
char * convert_to_color(struct descriptor_data *d, char *text);

/* Procedures related to 'look' */
void	argument_split_2(char *argument, char *first_arg, char *second_arg)
{
   int	look_at, found, begin;
   found = begin = 0;

   /* Find first non blank */
   for ( ; *(argument + begin ) == ' ' ; begin++)
      ;

   /* Find length of first word */
   for (look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

      /* Make all letters lower case, AND copy them to first_arg */
      *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
   *(first_arg + look_at) = '\0';
   begin += look_at;

   /* Find first non blank */
   for ( ; *(argument + begin ) == ' ' ; begin++)
      ;

   /* Find length of second word */
   for ( look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

      /* Make all letters lower case, AND copy them to second_arg */
      *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
   *(second_arg + look_at) = '\0';
   begin += look_at;
}


/* Function to display a string array to a char - end array with "\n" */
void  display_string_array(struct char_data *ch, char **array)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';
    for (i = 0; **(array+i) != '\n';i++) {
      if (strcmp(array[i],"DO_NOT_USE")!=0) {
	sprintf(buf, "%s  %-12s", buf, array[i]);
	if (!((i+1) % 5))
	    strcat(buf, "\r\n");	  
      }
    }
    if (i % 5)
	strcat(buf, "\r\n");
    send_to_char(buf, ch);
}


char	*find_ex_description(char *word, struct extra_descr_data *list)
{
   struct extra_descr_data *i;

   for (i = list; i; i = i->next)
      if (isname(word, i->keyword))
	 return(i->description);

   return(0);
}


void	show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
   bool found;

   *buf = '\0';
   if ((mode == 0) && object->description)
      strcpy(buf, object->description);
   else if (object->short_description && ((mode == 1) || 
       (mode == 2) || (mode == 3) || (mode == 4)))
      strcpy(buf, object->short_description);
   else if (mode == 5) {
      if (object->obj_flags.type_flag == ITEM_NOTE) {
	 if (object->action_description) {
	    strcpy(buf, "There is something written upon it:\r\n\r\n");
	    strcat(buf, object->action_description);
	    page_string(ch->desc, buf, 1);
	 } else
	    act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      } else if ((object->obj_flags.type_flag != ITEM_DRINKCON)) {
	 strcpy(buf, "You see nothing special..");
      } else /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
	 strcpy(buf, "It looks like a drink container.");
   }


   if (mode != 3) {    /* Added effect flags 5/96 -- DT */
      found = FALSE;
      if (GET_ITEM_LEVEL(object) > GET_LEVEL(ch)) {
	  strcat(buf, " (+)");
	  found = TRUE;
      }

      /* Bodpoint - Disabled at Mokky's request
       * - Brought back into use for imms flagged HOLYLIGHT only
       */
      if (CLANEQ(object) && PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	   if ((CLANEQ_CLAN(object) != CLAN(ch)) ||
               (CLAN_LEVEL(ch) <= 1)) {
		strcat(buf, " (!CLAN)");
		found = TRUE;
	   }

      if (IS_OBJ_STAT(object, ITEM_GLOW)) {
	  strcat(buf, " (glowing)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_HUM)) {
	  strcat(buf, " (humming)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_EVIL) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
	  strcat(buf, " (evil)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
	 strcat(buf, " (invisible)");
	 found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
	  strcat(buf, " (magic)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
	  strcat(buf, " (blessed)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_BROKEN)) {
	  strcat(buf, " (broken)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_CHAOTIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
        strcat(buf, " (chaotic)");
	  found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_FLAME)) {
	  strcat(buf, " (flaming)");
	  found = TRUE;
      }
    }
   
   strcat(buf, "\r\n");
   page_string(ch->desc, buf, 1);
 }


void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show)
{
  struct obj_data *i;
  bool found;
  int how_many = 0;
  
  found = FALSE;
  for ( i = list; i ; i = i->next_content ) {
    if (CAN_SEE_OBJ(ch, i)) {
      found = TRUE;
      if (i->next_content && 
	  (i->item_number == i->next_content->item_number) &&
	  (i->obj_flags.extra_flags == i->next_content->obj_flags.extra_flags) &&
	  (show != 2 || number(0, 50) < GET_LEVEL(ch)) &&
	  !str_cmp(i->short_description, i->next_content->short_description))
	{
          if (mode != 0 || (mode == 0 && !IS_OBJ_STAT(i, ITEM_HIDDEN)))
  	    how_many++;
	} else {
	  if (how_many) {
	    sprintf(buf, "(%2d) ", how_many + 1);
	    send_to_char(buf, ch);
	    how_many = 0;
	  } else if (mode > 0 && mode < 5) {
	    send_to_char("     ", ch);
	  }
          if (mode != 0 || (mode == 0 && !IS_OBJ_STAT(i, ITEM_HIDDEN)))
	    show_obj_to_char(i, ch, mode);
	}
    }      
  }
    
  if ((!found) && (show))
    send_to_char("      Nothing.\r\n", ch);
}


/* The following arrays are for the diag_to_char, used to describe the
 * the char's charisma
 */
static char *
char_charisma[3][9] = {
{/* Neutral */
    "plug-ugly ", "repulsive ", "ugly ", "grotesque ", "average looking ", "fine ", "beautiful ", "charming ", "ravishing " },
{/* Male */
    "plug-ugly ", "foul ", "smelly ", "dirty ", "fair ", "handsome ", "attractive ", "sexy ", "ravishing " },
{/* Female */
    "plug-ugly ", "ugly ", "warty ", "plump ", "pretty ", "lovely ", "attractive ", "sexy ", "ravishing " }
};

/* The following function is used by prompt to get the victims
 * condition.
 */
static char *
victimcondition[3][9] = {
{
    "#rdying#N", "#rmutilated#N", "#yawful#N", "#yhurt#N", "#ynasty#N", "#ywounded#N", "#gscratched#N", "#gexcellent#N", "#wperfect#N" },
{
    "*#r>#N        *",
    "*#r>>#N       *",
    "*#r>>#y>#N      *",
    "*#r>>#y>>#N     *",
    "*#r>>#y>>>#N    *",
    "*#r>>#y>>>>#N   *",
    "*#r>>#y>>>>#g>#N  *",
    "*#r>>#y>>>>#g>>#N *",
    "*#r>>#y>>>>#g>>>#N*"
    },
{
    " is totally mutilated.\r\n",
    " is bleeding awfully from big wounds.\r\n",
    " is in awful condition.\r\n",
    " looks pretty hurt.\r\n",
    " has some big wounds and scratches.\r\n",
    " has quite a few wounds.\r\n",
    " has some small wounds and bruises.\r\n",
    " has a few scratches.\r\n",
    " is in excellent condition.\r\n" }
    };

void	diag_char_to_char(struct char_data *i, struct char_data *ch)
{
   int	cond;

   if (GET_MAX_HIT(i) > 0)
      cond = (8 * GET_HIT(i)) / GET_MAX_HIT(i);
   else
      cond = 0; /* How could MAX_HIT be < 1?? */

   strcpy(buf, GET_NAME(i));
   CAP(buf);

   if (!IS_NPC(i)) {
       strcat(buf, " the ");
       strcat(buf, char_charisma[atoi(&GET_SEX(i))][GET_CHA(i)/3]);
       strcat(buf, race_table[GET_RACE(i)]);
   }

   if (cond > 8)
       cond = 8;
   else if (cond < 0)
       cond = 0;

   strcat(buf, victimcondition[2][cond]);

   send_to_char(buf, ch);
}


char *
diag_to_prompt(struct char_data *i, int mode)
{
    int	cond;

    if (GET_MAX_HIT(i) > 0 && GET_HIT(i) > 0)
	cond = (8 * GET_HIT(i)) / GET_MAX_HIT(i);
    else
	cond = 0;     /* How could MAX_HIT be < 1?? */
                      /* Simple! If you have MAXHIT = 20 and wear a item */
                      /* HIT -25                                   */
    if (cond > 8)
	cond = 8;
    if (cond < 0)
	cond = 0;

    
    return victimcondition[mode][cond];
}


void blind_routine(struct char_data *ch)
{
  struct affected_type af;

  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.duration = 2;
  af.bitvector = AFF_BLIND;
  af.type = SPELL_BLINDNESS;
  affect_join(ch, &af, TRUE, TRUE);

  af.location = APPLY_AC;
  af.modifier = 40;
  affect_join(ch, &af, TRUE, TRUE);
  return;
}


void	show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
   int	j, found;

   if (mode == 0) {
       
       if ((IS_AFFECTED(i, AFF_HIDE) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) 
	   || !CAN_SEE(ch, i)) {
	 if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
	    send_to_char("You sense a hidden life form in the room.\r\n", ch);
	 return;
      }

      if (!(i->player.long_descr) || (GET_POS(i) != i->mob_specials.default_pos)) {
     /* A player char or a mobile without long descr, or not in default pos. */

/* New routine by Petrus */

	  *buf = '\0';

	  if (IS_AFFECTED(i, AFF_SANCTUARY))
	      strcat(buf, "+");

	  if (IS_AFFECTED(i, AFF_INVISIBLE))
	      strcat(buf, "*");

	  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
            if (IS_EVIL(i))
              strcat(buf, "(#rE#N)");
	    else if (IS_GOOD(i))
              strcat(buf, "(#wG#N)");
            else
              strcat(buf, "(#bN#N)");
          }

	  if (!IS_NPC(i))
            /* Player Names in Cyan (bright red for PKOK chars) */
	    sprintf(buf, "%s%s%s%s#N %s", buf,
		    (GET_PRENAME(i)?GET_PRENAME(i):""),
                    (PLR_FLAGGED(i, PLR_PKOK)?"#r":"#C"),
		    i->player.name, GET_TITLE(i));
	  else {
	      strcpy(buf2, i->player.short_descr);
	      CAP(buf2);
	      sprintf(buf, "%s#Y%s#N", buf, buf2);
	  }

	 if (!IS_NPC(i) && !i->desc)
	    strcat(buf, " (#Rlinkless#N)");

	 if (PLR_FLAGGED(i, PLR_WRITING))
	    strcat(buf, " (#Mwriting#N)");

	 switch (GET_POS(i)) {
	 case POS_STUNNED  :
	    strcat(buf, " is lying here, stunned.");
	    break;
	 case POS_INCAP    :
	    strcat(buf, " is lying here, incapacitated.");
	    break;
	 case POS_MORTALLYW:
	    strcat(buf, " is lying here, mortally wounded.");
	    break;
	 case POS_DEAD     :
	    strcat(buf, " is lying here, dead.");
	    break;
	case POS_STANDING :
	    if (i->specials.mounting)
		sprintf(buf, "%s is here mounted on %s.",
			buf, GET_NAME(i->specials.mounting));
	    else
		strcat(buf, " is standing here.");
	    break;
	 case POS_SITTING  :
	    strcat(buf, " is sitting here.");
	    break;
	 case POS_RESTING  :
	    strcat(buf, " is resting here.");
	    break;
	 case POS_SLEEPING :
	    strcat(buf, " is sleeping here (Zzzzzzz).");
	    break;
	case POS_FIGHTING :
	    if (i->specials.fighting) {
	       strcat(buf, " is here, fighting ");
	       if (i->specials.fighting == ch)
		  strcat(buf, "YOU!");
	       else {
		  if (i->in_room == i->specials.fighting->in_room)
		      strcat(buf, GET_NAME(i->specials.fighting));
		  else
		      strcat(buf, "someone who has already left");
		  strcat(buf, ".");
	       }
	    } else /* NIL fighting pointer */
	       strcat(buf, " is here struggling with thin air.");
	    break;
	 default :
	    strcat(buf, " is floating here.");
	    break;
	}
	  
	 strcat(buf, "\r\n");
	 send_to_char(buf, ch);
      } else { /* npc with long */
	  if (IS_AFFECTED(i, AFF_SANCTUARY))
	      strcpy(buf, "+");
	  else
	      *buf = '\0';
	  
	  if (IS_AFFECTED(i, AFF_INVISIBLE))
	      strcat(buf, "*");
	  	 
	  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
            if (IS_EVIL(i))
              strcat(buf, "(#rE#N)");
            else if (IS_GOOD(i))
              strcat(buf, "(#wG#N)");
            else
              strcat(buf, "(#bN#N)");
          }

	  sprintf(buf, "%s#Y%s#N", buf, i->player.long_descr);
	  send_to_char(buf, ch);
      }

   /* 
    *  Removed by Petrus. Replaced with special routine
    *  if (IS_AFFECTED(i, AFF_SANCTUARY))
    *      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    */ 
   
  } else if (mode == 1) {

      if (i->player.description)
	  send_to_char(i->player.description, ch);
      else {
	  act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
      }

      /* Perform blinding routine for MOB_BLINDER and return - Bod */
      if (IS_NPC(i) && MOB_FLAGGED(i, MOB_BLINDER)) {
        blind_routine(ch);
        return;
      }

      /* Show a character to another */

      diag_char_to_char(i, ch);

      found = FALSE;
      for (j = 0; j < MAX_WEAR; j++) {
	 if (i->equipment[j]) {
	    if (CAN_SEE_OBJ(ch, i->equipment[j])) {
	       found = TRUE;
	    }
	 }
      }
      if (found) {
	 act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
	 for (j = 0; j < MAX_WEAR; j++) {
	    if (i->equipment[j]) {
	       if (CAN_SEE_OBJ(ch, i->equipment[j])) {
		  send_to_char(where[j], ch);
		  show_obj_to_char(i->equipment[j], ch, 1);
	       }
	    }
	 }
      }
   } else if (mode == 2) {

      /* Lists inventory */
      act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
      list_obj_to_char(i->carrying, ch, 1, TRUE);
   }
}



void	list_char_to_char(struct char_data *list, struct char_data *ch, 
			  int mode)
{
    struct char_data *i;
    int num;
    
    for (i = list; i; i = i->next_in_room)
	if (ch != i) {
	    num = 1;
	    while (IS_NPC(i) &&
		   i->next_in_room && 
		   i->nr == i->next_in_room->nr &&
		   GET_POS(i) == GET_POS(i->next_in_room) &&
		   i->specials.affected_by == i->next_in_room->specials.affected_by) 
	    {
                if (!MOB_FLAGGED(i, MOB_HIDDEN) || (MOB_FLAGGED(i, MOB_HIDDEN) &&
                   GET_HIT(i) < GET_MAX_HIT(i))) /* Don't count Hidden Mobs */
		  num++;
		i = i->next_in_room;
	    }
	    
	    if ((CAN_SEE(ch, i) && 
		 (IS_AFFECTED(ch, AFF_SENSE_LIFE) || 
		  !IS_AFFECTED(i, AFF_HIDE) ||
		  PRF_FLAGGED(ch, PRF_HOLYLIGHT)))) 
	    {
		if (num > 1) {
		    sprintf(buf2, "[%2d] ", num);
		    send_to_char(buf2, ch);
		}
                if (!MOB_FLAGGED(i, MOB_HIDDEN) || (MOB_FLAGGED(i, MOB_HIDDEN) &&
                    GET_HIT(i) < GET_MAX_HIT(i))) /* Don't show Hidden Mobs */
		  show_char_to_char(i, ch, 0);
	    } else if ((IS_DARK(ch->in_room)) && (IS_AFFECTED(i, AFF_INFRARED) && (GET_LEVEL(i) < LEVEL_DEITY)))
	    {
		sprintf(buf2, "You see %s pair%s of glowing red eyes looking your way.\r\n", (num==1?"a":(num<4?"some":"many")), (num>1?"s":""));
		send_to_char(buf2, ch);
	    }
	}
}

ACMD(do_peek)
{
  struct char_data *tch;

  skip_spaces(&argument);


  if (!IS_THIEF(ch) && (GET_LEVEL(ch) < LEVEL_DEITY)) {
    send_to_char("Nice try...\r\n", ch);
    return;
  }

  if(!(tch = get_char_room_vis(ch, argument))) {
    sprintf(buf, "You don't see %s anywhere in the room.\r\n", argument);
    send_to_char(buf, ch);
    return;
  }

  if (ch == tch) {
    send_to_char("Try peeking at someone else...\r\n", ch);
    return;
  }

  if ((GET_LEVEL(tch) >= LEVEL_DEITY) && (GET_LEVEL(ch) < GET_LEVEL(tch))) {
    send_to_char("I don't think that would be a good idea.\r\n", ch);
    return;
  }

  /* Perform blinding routine for MOB_BLINDER and return - Bod */
  if (IS_NPC(tch) && MOB_FLAGGED(tch, MOB_BLINDER)) {
    sprintf(buf, "You are blinded as you try to peek at %s.\r\n", GET_NAME(tch));
    send_to_char(buf, ch);
    blind_routine(ch);
    return;
  }

  if ((GET_LEVEL(ch) < LEVEL_DEITY) && 
      (number(1, 100) > (GET_SKILL(ch, SKILL_STEAL) - (2 * (GET_LEVEL(tch) - GET_LEVEL(ch)))))) {
    sprintf(buf,"%s was just trying to peek at your stuff.\r\n", GET_NAME(ch));
    send_to_char(buf, tch);
    send_to_char("Caught peeking!\r\n", ch);
    return;
  }

  send_to_char("\r\nYou attempt to peek at the inventory:\r\n", ch);
  list_obj_to_char(tch->carrying, ch, 1, 2);
}

ACMD(do_look)
{
   static char	arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   int door;
   char *c;
   char *obv_exits[] =
   {
       "n",
       "e",
       "s",
       "w",
       "u",
       "d",
       "\n"
       };
   char abuf[128];
   int	keyword_no;
   int	j, bits, temp;
   bool found;
   struct obj_data *tmp_object, *found_object;
   struct char_data *tmp_char;
   char	*tmp_desc;
   static char	*keywords[] = {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "in",
      "at",
      "",  /* Look at '' case */
      "\n"          };

   if (!ch->desc)
      return;

   if (GET_POS(ch) < POS_SLEEPING)
      send_to_char("You can't see anything but stars!\r\n", ch);
   else if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("You can't see anything, you're sleeping!\r\n", ch);
   else if ( IS_AFFECTED(ch, AFF_BLIND) )
      send_to_char("You can see - nothing, you're blinded!\r\n", ch);
   else if ( IS_DARK(ch->in_room) && 
	    (!PRF_FLAGGED(ch, PRF_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_LIGHT))) {
      send_to_char("It is pitch black...\r\n", ch);
      list_char_to_char(world[ch->in_room]->people, ch, 0);
   } else {
      argument_split_2(argument, arg1, arg2);
      keyword_no = search_block(arg1, keywords, FALSE); /* Partial Match */

      if ((keyword_no == -1) && *arg1) {
	 keyword_no = 7;
	 strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
      }

      found = FALSE;
      tmp_object = 0;
      tmp_char	 = 0;
      tmp_desc	 = 0;

      switch (keyword_no) {
	 /* look <dir> */
      case 0 :
      case 1 :
      case 2 :
      case 3 :
      case 4 :
      case 5 :
	 if (EXIT(ch, keyword_no)) {
	    if (EXIT(ch, keyword_no)->general_description)
	       send_to_char(EXIT(ch, keyword_no)->general_description, ch);
	    else
	       send_to_char("You see nothing special.\r\n", ch);

	    if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
	        (EXIT(ch, keyword_no)->keyword)) {
	       sprintf(buf, "The %s is closed.\r\n",
	           fname(EXIT(ch, keyword_no)->keyword));
	       send_to_char(buf, ch);
	    } else {
	       if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) && 
	           EXIT(ch, keyword_no)->keyword) {
		  sprintf(buf, "The %s is open.\r\n",
		      fname(EXIT(ch, keyword_no)->keyword));
		  send_to_char(buf, ch);
	       }
	    }
	    
	    /* Trap detection on the exit -- DT */
	    
	    if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_TRAP) &&
		number(1, 100) < GET_SKILL(ch, SKILL_DET_TRAP)) {
	      improve_skill(ch, SKILL_DET_TRAP);
	      
	      if (EXIT(ch, keyword_no)->keyword) {
		sprintf(buf, "The %s looks trapped.\r\n",
			fname(EXIT(ch, keyword_no)->keyword));
		send_to_char(buf, ch);
	      } else
		send_to_char("The exit looks trapped.\r\n", ch);
	    }
	    
	    
	 } else {
	   send_to_char("Nothing special there...\r\n", ch);
	 }
	 
	 break;

	 /* look 'in'	*/
      case 6:
	 if (*arg2) {
	    /* Item carried */

	    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | 
	        FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	    if (bits) { /* Found something */
	       if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) || 
	           (GET_ITEM_TYPE(tmp_object) == ITEM_FOUNTAIN)) {
		  if (tmp_object->obj_flags.value[1] <= 0) {
		     act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
		  } else {
		     temp = ((tmp_object->obj_flags.value[1] * 3) / tmp_object->obj_flags.value[0]);
		     sprintf(buf, "It's %sfull of a %s liquid.\r\n",
		         fullness[temp], color_liquid[tmp_object->obj_flags.value[2]]);
		     send_to_char(buf, ch);
		  }
	       } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
		  if (!IS_SET(tmp_object->obj_flags.value[1], CONT_CLOSED)) {
		     send_to_char(fname(tmp_object->name), ch);
		     switch (bits) {
		     case FIND_OBJ_INV :
			send_to_char(" (carried) : \r\n", ch);
			break;
		     case FIND_OBJ_ROOM :
			send_to_char(" (here) : \r\n", ch);
			break;
		     case FIND_OBJ_EQUIP :
			send_to_char(" (used) : \r\n", ch);
			break;
		     }
		     list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
		  } else
		     send_to_char("It is closed.\r\n", ch);
	       } else {
		  send_to_char("That is not a container.\r\n", ch);
	       }
	    } else { /* wrong argument */
	       send_to_char("You do not see that item here.\r\n", ch);
	    }
	 } else { /* no argument */
	    send_to_char("Look in what?!\r\n", ch);
	 }
	 break;

	 /* look 'at'	*/
      case 7 :
	 if (*arg2) {

	    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | 
	        FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);

	    if (tmp_char) {
	       show_char_to_char(tmp_char, ch, 1);
	       if (ch != tmp_char) {
		  if (CAN_SEE(tmp_char, ch))
		     act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
		  act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	       }
	       return;
	    }

	    /* Search for Extra Descriptions in room and items */

	    /* Extra description in room?? */
	    if (!found) {
	       tmp_desc = find_ex_description(arg2, 
	           world[ch->in_room]->ex_description);
	       if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 0);
		  return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
		  /* Old system was: found = TRUE; */
	       }
	    }

	    /* Search for extra descriptions in items */

	    /* Equipment Used */

	    if (!found) {
	       for (j = 0; j < MAX_WEAR && !found; j++) {
		  if (ch->equipment[j]) {
		     if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
			tmp_desc = find_ex_description(arg2, 
			    ch->equipment[j]->ex_description);
			if (tmp_desc) {
			   page_string(ch->desc, tmp_desc, 1);
			   found = TRUE;
			}
		     }
		  }
	       }
	    }

	    /* In inventory */

	    if (!found) {
	       for (tmp_object = ch->carrying; 
	           tmp_object && !found; 
	           tmp_object = tmp_object->next_content) {
		  if CAN_SEE_OBJ(ch, tmp_object) {
		     tmp_desc = find_ex_description(arg2, 
		         tmp_object->ex_description);
		     if (tmp_desc) {
			page_string(ch->desc, tmp_desc, 1);
			found = TRUE;
		     }
		  }
	       }
	    }

	    /* Object In room */

	    if (!found) {
	       for (tmp_object = world[ch->in_room]->contents; 
	           tmp_object && !found; 
	           tmp_object = tmp_object->next_content) {
		  if CAN_SEE_OBJ(ch, tmp_object) {
		     tmp_desc = find_ex_description(arg2, 
		         tmp_object->ex_description);
		     if (tmp_desc) {
			page_string(ch->desc, tmp_desc, 1);
			found = TRUE;
		     }
		  }
	       }
	    }
	    /* wrong argument */

	    if (bits) { /* If an object was found */
	       if (!found)
		  show_obj_to_char(found_object, ch, 5); /* Show no-description */
	       else
		  show_obj_to_char(found_object, ch, 6); /* Find hum, glow etc */
	    } else if (!found) {
	       send_to_char("You do not see that here.\r\n", ch);
	    }
	 } else {
	    /* no argument */

	    send_to_char("Look at what?\r\n", ch);
	 }

	 break;


	 /* look ''		*/
     case 8 :
         
         if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) { /*rewritten to display more info*/
           sprintbit((long) world[ch->in_room]->room_flags, room_bits, buf);
           sprinttype(world[ch->in_room]->sector_type, sector_types, buf1);
           sprintf(buf2, "[#b%5d#N] #C%s#N [ #G%s#N] [#C%s#N]\r\n", world[ch->in_room]->number,
                        world[ch->in_room]->name, buf, buf1);
         } else
           sprintf(buf2, "#C%s#N\r\n", world[ch->in_room]->name);
         send_to_char(buf2, ch);

	 strcpy(abuf, "[Exits: #C");
	 *buf1 = '\0';
         
         /* New Autoexit to provide more information to Imms / Ptesters */
         if (PRF_FLAGGED(ch, PRF_ROOMFLAGS) && 
             !(PRF_FLAGGED(ch, PRF_DISPVT) || PRF_FLAGGED(ch, PRF_DISPANSI))) { 

           for (door = 0; door < NUM_OF_DIRS; door++)
             if (EXIT(ch, door) && (EXIT(ch, door)->to_room != NOWHERE)) {
               if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) c = ":L";
               else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) c = ":C";
               else if (IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) c = ":O";
               else c = "";
               sprintf(buf1, "%s%s (%d%s)  ", buf1, obv_exits[door],
                            world[EXIT(ch, door)->to_room]->number, c);
           }
         } else {
           for (door = 0; door < NUM_OF_DIRS; door++) {
             if (EXIT(ch, door) && (EXIT(ch, door)->to_room != NOWHERE) &&
                 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
               sprintf(buf1, "%s%s", buf1, obv_exits[door]);
             else if (PRF_FLAGGED(ch, PRF_DISPVT) || PRF_FLAGGED(ch, PRF_DISPANSI))
               sprintf(buf1, "%s ", buf1);
           }
         }
	 if (*buf1 != '\0')
	     strcat(abuf, buf1);
	 else
	     strcat(abuf, "None!");
	 strcat(abuf, "#N]");

	 if (!PRF_FLAGGED(ch, PRF_BRIEF) || (cmd == find_command("look")))
           send_to_char(world[ch->in_room]->description, ch);

	 send_to_char(CCGRN(ch, C_CMP), ch);
	 list_obj_to_char(world[ch->in_room]->contents, ch, 0, FALSE);
	 send_to_char(CCNRM(ch, C_CMP), ch);
	 list_char_to_char(world[ch->in_room]->people, ch, 0);

	 if (PRF_FLAGGED(ch, PRF_DISPVT) && ch->desc) 
	 {
	     sprintf(buf2, "%14s", abuf);
	     write_to_pos(ch->desc, 66, GET_SCRLEN(ch) - 2,
			  convert_to_color(ch->desc, buf2), 0);
	 } else if (!PRF_FLAGGED(ch, PRF_NOEXITS)) {
	     strcat (abuf, "\r\n");
	     send_to_char(abuf,ch);
	 }
	 
	 track_check(ch);
	 
	 break;

	 /* wrong arg	*/
      case -1 :
	 send_to_char("Sorry, I didn't understand that!\r\n", ch);
	 break;
      }
   }
}


/* end of look */




ACMD(do_read)
{
   /* This is just for now - To be changed later.! */
   sprintf(buf1, "at %s", argument);
   do_look(ch, buf1, 15, 0);
}



ACMD(do_examine)
{
   char	name[100], buf[100];
   int	bits;
   struct char_data *tmp_char;
   struct obj_data *tmp_object;

   sprintf(buf, "at %s", argument);
   do_look(ch, buf, 15, 0);

   one_argument(argument, name);

   if (!*name) {
      send_to_char("Examine what?\r\n", ch);
      return;
  }

   bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM | 
       FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

   if (tmp_object) {
      if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) || 
          (GET_ITEM_TYPE(tmp_object) == ITEM_FOUNTAIN) || 
          (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER)) {
	 send_to_char("When you look inside, you see:\r\n", ch);
	 sprintf(buf, "in %s", argument);
	 do_look(ch, buf, 15, 0);
      }
   }
}



ACMD(do_exits)
{
   int	door;
   char	*exits[] = 
    {
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
   };

   *buf = '\0';

   for (door = 0; door <= 5; door++)
     if (EXIT(ch, door)) {
       if (EXIT(ch, door)->to_room != NOWHERE && 
	   !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
	 if (GET_LEVEL(ch) >= LEVEL_DEITY)
	   sprintf(buf + strlen(buf), "%-5s - [%5d] %s\r\n",
		   exits[door], world[EXIT(ch, door)->to_room]->number,
		   world[EXIT(ch, door)->to_room]->name);
	 else if (IS_DARK(EXIT(ch, door)->to_room) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	   sprintf(buf + strlen(buf), "%-5s - Too dark to tell\r\n", exits[door]);
	 else
	   sprintf(buf + strlen(buf), "%-5s - %s\r\n", exits[door],
		   world[EXIT(ch, door)->to_room]->name);
       }
     }
   
   send_to_char("Obvious exits:\r\n", ch);

   if (*buf)
      send_to_char(buf, ch);
   else
      send_to_char(" None.\r\n", ch);
}


ACMD(do_score)
{
  char *align;
  int intalign, totac, i, j;
  struct obj_data *obj;
  
  struct time_info_data playing_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  
  if (IS_NPC(ch)) {
    send_to_char("You are a mob without intelligence :(\r\n", ch);
    return;
  }

  sprintf(buf, "You are #B%s#N#C %s#N (#Rlevel %d#N).\r\n",
	  GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch));
  
  sprintf(buf, "%sYou are a %d year old %s.", 
	  buf, GET_AGE(ch), race_table[GET_RACE(ch)]);
  
  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf, "  #bIt's your birthday today.#N\r\n");
  else
    strcat(buf, "\r\n");
  
  if (WORSHIPS(ch)) {
    strcpy(buf2, get_deity_name(ch));
    CAP(buf2);
    sprintf(buf, "%sYou are the devout worshipper of %s.\r\n",
	    buf, buf2);
  } 
  
  sprintf(buf, 
	  "%sYou have #R%d#N(#G%d#N) hp, #C%d#N(#B%d#N) mana and #Y%d#N(#G%d#N) movement points.\r\n",
	  buf,
	  GET_HIT(ch), GET_MAX_HIT(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch));
  
  
  intalign = GET_ALIGNMENT(ch);
  totac  = GET_AC(ch);
  
  if (intalign <= -900)
    align = "#rso evil your horns are beginning to show#N";
  else if (intalign <= -500)
    align = "#Rvery evil he he#N";
  else if (intalign <= -350)
    align = "#Revil#N";
  else if (intalign <= -150)
    align = "#Bneutral with an #Revil #Btendency#N";
  else if (intalign < 150)
    align = "#Bneutral#N";
  else if (intalign < 350)
    align = "#Bneutral with a touch of #Ggoodness#N";
  else if (intalign < 500)
    align = "#wgood#N";
  else if (intalign < 900)
    align = "#wso very much good#N";
  else
    align = "#wso good that you have developed a halo#N";
  
  sprintf(buf, "%sYou are %s.\r\n",
	  buf,align);
  
  sprintf(buf, "%sYou have scored #G%d#N exp, and have #B%d#N gold coins.\r\n",
	  buf, GET_EXP(ch), GET_GOLD(ch));
  
  if (LOWEST_LEVEL(ch) < LEVEL_DEITY) {
    sprintf(buf, "%sYou need #R%d#N exp to reach your next level.\r\n", buf,
	    exp_needed(ch) - GET_EXP(ch));
  }
  
  playing_time = real_time_passed((time(0) - ch->player.time.logon) + 
				  ch->player.time.played, 0);
  sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
	  buf, playing_time.day, playing_time.hours);

  if(QUEST_NUM(ch))
    sprintf(buf, "%sYou have #m%d#N quest points.\r\n", buf, QUEST_NUM(ch));
  
  if(IS_CARRYING_N(ch) > 0){
    sprintf(buf, "%sYou are carrying %d item%s with the total weight of %d pounds.\r\n", buf, IS_CARRYING_N(ch),(IS_CARRYING_N(ch) > 1) ? "s" : "",
	    IS_CARRYING_W(ch));
  } else 
    strcat(buf, "You are not carrying anything.\r\n");
  
  for (i = 0, obj = ch->carrying; obj; obj = obj->next_content, i++);
  
  if (i > 0)
    sprintf(buf, "%sThere %s %d item%s in your inventory and ", 
	    buf, (i > 1) ? "are" : "is", i, (i > 1) ? "s" : "");
  else
    sprintf(buf, "%sYou have nothing in your inventory and ",buf);
  for (i = 0, j = 0; i < MAX_WEAR; i++)
    if (ch->equipment[i])
      j++;
  if (j > 0) {
    sprintf(buf2, "%d item%s equipped.\r\n", j, (j > 1) ? "s" :"");
    strcat(buf, buf2);
  }
  else {
    sprintf(buf2, "no items equipped. \r\n");
    strcat(buf, buf2);
  }
  send_to_char(buf, ch); 
  
  switch (GET_POS(ch)) {
  case POS_DEAD :
    strcpy(buf, "#RYou are dead!#N\r\n");
    break;
  case POS_MORTALLYW :
    strcpy(buf, "#RYou are mortally wounded!  You should seek help!#N\r\n");
    break;
  case POS_INCAP :
    strcpy(buf, "#RYou are incapacitated, slowly fading away...#N\r\n");
    break;
  case POS_STUNNED :
    strcpy(buf, "#YYou are stunned!  You can't move!#N\r\n");
    break;
  case POS_SLEEPING :
    strcpy(buf, "#GYou are sleeping.#N\r\n");
    break;
  case POS_RESTING  :
    strcpy(buf, "#GYou are resting.#N\r\n");
    break;
  case POS_SITTING  :
    strcpy(buf, "#BYou are sitting.#N\r\n");
    break;
  case POS_FIGHTING :
    if (ch->specials.fighting)
      sprintf(buf, "#RYou are fighting %s.#N\r\n", PERS(ch->specials.fighting, ch));
    else
      strcpy(buf, "You are fighting thin air.\r\n");
    break;
  case POS_STANDING :
    if (ch->specials.mounting)
      sprintf(buf, "#CYou are mounted on %s.#N\r\n", GET_NAME(ch->specials.mounting));
    else
      strcpy(buf, "#BYou are standing.#N\r\n");
    break;
  default :
    strcpy(buf, "#BYou are floating.#N\r\n");
    break;
  }
  
  if (GET_COND(ch, DRUNK) > 10)
    strcat(buf, "#RYou are intoxicated.#N\r\n");
  
  if (GET_COND(ch, FULL) == 0)
    strcat(buf, "#YYou are hungry.#N\r\n");
  
  if (GET_COND(ch, THIRST) == 0)
    strcat(buf, "#YYou are thirsty.#N\r\n");
  
  if (IS_AFFECTED(ch, AFF_BLIND))
    strcat(buf, "#RYou have been blinded!#N\r\n");
  
  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    strcat(buf, "#BYou are invisible.#N\r\n");
  
  if (IS_AFFECTED(ch, AFF_DETECT_INVIS))
    strcat(buf, "#GYou are sensitive to the presence of invisible things.#N\r\n");
  
  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN))
    strcat(buf, "#GYou sense karma.#N\r\n");
    
  if (IS_AFFECTED(ch, AFF_SANCTUARY))
    strcat(buf, "#GYou are protected by Sanctuary.#N\r\n");
  
  if (IS_AFFECTED(ch, AFF_POISON))
    strcat(buf, "#RYou are poisoned!#N\r\n");
  
  if (IS_AFFECTED(ch, AFF_CHARM))
    strcat(buf, "#RYou have been charmed!#N\r\n");
  
  if (affected_by_spell(ch, SPELL_ARMOR))
    strcat(buf, "#GYou feel protected.#N\r\n");

  if (affected_by_spell(ch, SPELL_HOLY_WRATH))
    strcat(buf, "#CYou feel the power of the gods.#N\r\n");
  
  send_to_char(buf, ch);

  if (PLR_FLAGGED(ch, PLR_PKOK))
    send_to_char("#rYou are a player killer!#N (PKOK)\r\n", ch);

  if (PLR_FLAGGED(ch, PLR_ARENA)) {
    sprintf(buf, "You are currently in the wargames!\r\nYou have %d shots left and have taken %d hits.\r\n",
	    ch->specials.wargame.ammo,
	    ch->specials.wargame.hits);
    send_to_char(buf, ch);
    if (ch->specials.wargame.team > 0) {
      sprintf(buf, "You are on the %s.\r\n", which_team[(int)ch->specials.wargame.team]);
      send_to_char(buf, ch);
    }
  }
  
}


ACMD(do_time)
{
   char	*suf;
   int	weekday, day;
   extern struct time_info_data time_info;
   extern const char	*weekdays[];
   extern const char	*month_name[];

   sprintf(buf, "It is %d o'clock %s, on ",
       ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
       ((time_info.hours >= 12) ? "pm" : "am") );

   weekday = ((35 * time_info.month) + time_info.day + 1) % 7;/* 35 days in a month */

   strcat(buf, weekdays[weekday]);
   strcat(buf, "\r\n");
   send_to_char(buf, ch);

   day = time_info.day + 1;   /* day in [1..35] */

   if (day == 1)
      suf = "st";
   else if (day == 2)
      suf = "nd";
   else if (day == 3)
      suf = "rd";
   else if (day < 20)
      suf = "th";
   else if ((day % 10) == 1)
      suf = "st";
   else if ((day % 10) == 2)
      suf = "nd";
   else if ((day % 10) == 3)
      suf = "rd";
   else
      suf = "th";

   sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
       day, suf, month_name[(int)time_info.month], time_info.year);

   send_to_char(buf, ch);
}


ACMD(do_weather)
{
   static char *sky_look[4] = {
      "cloudless",
      "cloudy",
      "rainy",
      "lit by flashes of lightning" };

   if (OUTSIDE(ch)) {
      sprintf(buf, 
          "The sky is %s and %s.\r\n",
          sky_look[weather_info.sky],
          (weather_info.change >= 0 ? "you feel a warm wind from south" : 
          "your sense that bad weather is due"));
      send_to_char(buf, ch);
   } else
      send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


ACMD(do_help)
{
   extern int	top_of_helpt;
   extern struct help_index_element *help_index;
   extern FILE *help_fl;
   extern char	*help;

   int	chk, bot, top, mid, minlen;

   if (!ch->desc)
      return;

   for (; isspace(*argument); argument++)
      ;

   if (*argument) {
      if (!help_index) {
	 send_to_char("No help available.\r\n", ch);
	 return;
      }
      bot = 0;
      top = top_of_helpt;

      for (; ; ) {
	 mid = (bot + top) / 2;
	 minlen = strlen(argument);

	 if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {

         /* trace backwards to find first matching entry. Thanks Jeff Fink! */
	    while ((mid > 0) &&
	       (!(chk = strn_cmp(argument, help_index[mid-1].keyword, minlen))))
	          mid--;
	    fseek(help_fl, help_index[mid].pos, SEEK_SET);
	    *buf2 = '\0';
	    for (; ; ) {
	       fgets(buf, 80, help_fl);
	       if (*buf == '#')
		  break;
	       strcat(buf2, buf);
	       strcat(buf2, "\r");
	    }
	    page_string(ch->desc, buf2, 1);
	    return;
	 } else if (bot >= top) {
	    send_to_char("There is no help on that word.\r\n", ch);
	    return;
	 } else if (chk > 0)
	    bot = ++mid;
	 else
	    top = --mid;
      }
      return;
   }

   send_to_char(help, ch);
}



#define WHO_FORMAT \
"format: who [minlev[-maxlev]][-n sname][-c class][-r race][-k clanvnum]" \
" [-s -o -p -q -r -z -R -< ->]"


/* wiznamelist
 * Array with special wiznames of certain levels.
 */

static char *wiznamelist_m[] = {
  "Veteran ",
  " Master ",
  "  Hero  ",
  " Knight ",
  "  Lord  ",
  " Baron  ",
  " Count  ",
  "  Duke  ",
  " Avatar ",
  "Demigod ",
  " DEITY  ",
  "RETIRED ",
  " IMMORT ",
  " LESSER ",
  "GREATER ",
  " ADMIN  ",
  "  IMPL  "
};

static char *wiznamelist_f[] = {
  "Veteran ",
  "Mistress",
  " Heroine",
  " Knight ",
  "  Lady  ",
  "Baroness",
  "Countess",
  " Duchess",
  " Avatar ",
  " Angel  ",
  " DEITY  ",
  "RETIRED ",
  " IMMORT ",
  " LESSER ",
  "GREATER ",
  " ADMIN  ",
  "  IMPL  "
};

#define GET_COLORWIZ(ch) ((GET_LEVEL(ch) < LEVEL_DEITY)  ? "#R" :(\
(GET_LEVEL(ch) == LEVEL_DEITY)  ? "#m" :(\
(GET_LEVEL(ch) == LEVEL_RETIRED) ? "#e" :(\
(GET_LEVEL(ch) == LEVEL_IMMORT)  ? "#w" :(\
(GET_LEVEL(ch) == LEVEL_LESSER) ? "#c" :(\
(GET_LEVEL(ch) == LEVEL_GREATER) ? "#y" :(\
(GET_LEVEL(ch) == LEVEL_ADMIN) ? "#b" :(\
(GET_LEVEL(ch) == LEVEL_IMPL)   ? "#g" : "#R"))))))))
				       
ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char	name_search[MAX_INPUT_LENGTH];
  static char who_list[LARGE_BUFSIZE];
  char	mode;
  int	low = 1, high = LEVEL_IMPL, current = -1, i, localwho = 0, questwho = 0;
  int	clanlist = 0, short_list = 0, outlaws = 0, num_can_see = 0;
  int	who_room = 0, race = -1, class = -1, sort = 0;
  int   ptr = 0, clan_search = 0, pkok = 0;
  char buf[MAX_STRING_LENGTH], wiz[30];
 
  int level;
  char *wizname = NULL;
  
  /* skip spaces */
  for (i = 0; *(argument + i) == ' '; i++);
  strcpy(buf, (argument + i));
  name_search[0] = '\0';
  
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
	 mode = *(arg + 1); /* just in case; we destroy arg in the switch */
	 switch (mode) {
	 case 'o':
	   outlaws = 1;
	   strcpy(buf, buf1);
	   break;
	 case 'c':
	   half_chop(buf1, arg, buf);
	   for (i = 1;i < CLASS_MAX;i++) { 
	     if (!strncmp(arg, pc_class_types[i], 4)) {
	       class = i;
	       break;
	     }
	   }
	   if (i >= CLASS_MAX) {
	     send_to_char("Must be one of the following classes:\r\n", ch);
	     display_string_array(ch, &pc_class_types[1]);
	     return;
	   }
	   break;
	 case 'r':
	   half_chop(buf1, arg, buf);
	   for (i = 0;i < RACE_MAX;i++) {
	     if (!strncmp(arg, race_table[i], 8)) {
	       race = i;
	       break;
	     }
	   }
	   if (i >= RACE_MAX) {
	     send_to_char("Must be a valid race:\r\n", ch);
	     display_string_array(ch, race_table);
	     return;
	   }
	   break;
	 case '<':
	   sort = 1;
	   strcpy(buf, buf1);
	   break;
	 case '>':
	   sort = -1;
	   strcpy(buf, buf1);
	   break;
	 case 'z':
	   localwho = 1;
	   strcpy(buf, buf1);
	   break;
	 case 's':
	   short_list = 1;
	   strcpy(buf, buf1);
	   break;
	 case 'q':
	   questwho = 1;
	   strcpy(buf, buf1);
	   break;
	 case 'l':
	   half_chop(buf1, arg, buf);
	   sscanf(arg, "%d-%d", &low, &high);
	   break;
	 case 'n':
	   half_chop(buf1, name_search, buf);
	   break;
	 case 'R':
	   who_room = 1;
	   strcpy(buf, buf1);
	   break;
         case 'k':
            half_chop(buf1, arg, buf);
            if (!*arg)
                 clanlist = 2;
            else {
                 i = atoi(arg);
                 clan_search = real_clan(i);
                 if((clan_search == -1)) {
                      send_to_char("That clan exists only in your imagination.\r\n", ch);
                      return;
                 } else
                      clanlist = 1;
            }
            break;
         case 'p':
           pkok = 1;
           strcpy(buf, buf1);
           break;
	 default:
	   send_to_char(WHO_FORMAT, ch);
	   return;
	   break;
	 } /* end of switch */
	 
    } else { /* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  } /* end while (parser) */
  
  strcpy(who_list, "#rPlayers\r\n-------#N\r\n");
  ptr = strlen(who_list);
  
  low = MAX(low, 1);
  high = MIN(high, LEVEL_IMPL);

  if (sort < 0 )
    current = high;
  else if (sort > 0)
    current = low;

  for (; current == -1 || (current <= high && current >= low); current += sort) {
    
    for (d = descriptor_list; d; d = d->next) {
      if (d->connected)
	continue;
      
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;
      
      if (current != -1 && GET_LEVEL(tch) != current)
	continue;
      
      if (*name_search && str_cmp(GET_NAME(tch), name_search) &&  
	  (!GET_TITLE(tch) || !strstr(GET_TITLE(tch), name_search)))
	continue;
      if (PLR_FLAGGED(tch, PLR_NOWHO) && (GET_LEVEL(ch) < LEVEL_DEITY) && (ch != tch))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) && 
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (pkok && !PLR_FLAGGED(tch, PLR_PKOK))
	continue;
      if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
	continue;
      if (localwho && world[ch->in_room]->zone != world[tch->in_room]->zone)
	continue;
      if (who_room && (tch->in_room != ch->in_room))
	continue;
      if (clanlist == 2)
        if (CLAN(ch) != CLAN(tch))
          continue;
      if ((clanlist == 1) && (clan_search != CLAN(tch)))
        continue;
      if (class != -1 && 
	  !((GET_CLASS(tch) == class) ||
	    ((GET_CLASS(tch) == CLASS_2MULTI) &&
	     ((GET_1CLASS(tch) == class) || (GET_2CLASS(tch) == class))) ||
	    ((GET_CLASS(tch) == CLASS_3MULTI) &&
	     ((GET_1CLASS(tch) == class) || (GET_2CLASS(tch) == class) || (GET_3CLASS(tch) == class)))))
	continue;
      if (race != -1 && GET_RACE(tch) != race)
	continue;
      
      level = GET_LEVEL(tch);
      
      if (level >= LEVEL_WIZNAME) {
	if (GET_SEX(tch) == SEX_FEMALE)
	  wizname = wiznamelist_f[level - LEVEL_WIZNAME];
	else
	  wizname = wiznamelist_m[level - LEVEL_WIZNAME];
      }
      
      if (short_list) {
	if (level >= LEVEL_WIZNAME || GET_WIZNAME(tch)) {
	  sprintf(buf, "[%s%8s#N] %s%-12.12s#N%s",
		  GET_COLORWIZ(tch),
		  (GET_WIZNAME(tch)?GET_WIZNAME(tch):wizname),
                  (PLR_FLAGGED(tch, PLR_PKOK)?"#r":"#G"),
		  GET_NAME(tch),
		  ((!(++num_can_see % 3)) ? "\r\n" : ""));
	} else {
          CLASS_ABBR(tch, wiz);
	  sprintf(buf, "[#Y%2d#N %s] %s%-12.12s#N%s",
		  level, wiz, (PLR_FLAGGED(tch, PLR_PKOK)?"#r":"#C"),
                  GET_NAME(tch),
		  ((!(++num_can_see % 3)) ? "\r\n" : ""));
	}
	
      } else {
	num_can_see++;
	if (level >= LEVEL_WIZNAME || GET_WIZNAME(tch)) {
	  sprintf(buf, "[%s%8s#N] %s%s%s#N %s", 
		  GET_COLORWIZ(tch),
		  (GET_WIZNAME(tch) ? GET_WIZNAME(tch) : wizname),
		  (GET_PRENAME(tch) ? GET_PRENAME(tch) : ""),
                  (PLR_FLAGGED(tch, PLR_PKOK)?"#r":"#G"),
		  GET_NAME(tch),
		  GET_TITLE(tch));
	} else {
          CLASS_ABBR(tch, wiz);
	  sprintf(buf, "[#Y%2d#N %s] %s%s%s#N %s",
		  level, wiz,
		  (GET_PRENAME(tch)?GET_PRENAME(tch):""),
                  (PLR_FLAGGED(tch, PLR_PKOK)?"#r":"#C"), GET_NAME(tch),
		  GET_TITLE(tch));
	}
	
	if (CLAN(tch) >= 0 && !PLR_FLAGGED(tch, PLR_NOCLANTITLE))
	  sprintf(buf, "%s [%s %s]", buf,
		  CLAN_NAME(tch), CLAN_RANKNAME(tch));
	
	if (GET_INVIS_LEV(tch))
	  sprintf(buf, "%s #B(i%d)#N", buf, GET_INVIS_LEV(tch));
	else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	  strcat(buf, " #B(invis)#N");

	if (PLR_FLAGGED(tch, PLR_NOWHO))
	  strcat(buf, " #c(nowho)#N");
	
	if (PLR_FLAGGED(tch, PLR_MAILING))
	  strcat(buf, " #C(mailing)#N");
	else if (PLR_FLAGGED(tch, PLR_WRITING))
	  strcat(buf, " #M(writing)#N");
	
	if ((tch->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN) > 5 && 
            GET_LEVEL(tch) < LEVEL_ADMIN)
	  sprintf(buf, "%s #r(idle:%d)#N", buf, 
           (tch->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN));

	if (PRF_FLAGGED(tch, PRF_NOTELL))
	  strcat(buf, " #G(notell)#N");
	if (PRF_FLAGGED(tch, PRF_QUEST))
	  strcat(buf, " #M(quest)#N");
	if (PLR_FLAGGED(tch, PLR_THIEF))
	  strcat(buf, " #R(THIEF)#N");
	if (PLR_FLAGGED(tch, PLR_KILLER))
	  strcat(buf, " #R(KILLER)#N");
	if (PLR_FLAGGED(tch, PLR_PKOK))
	  strcat(buf, " #R(PKOK)#N");
	if (PLR_FLAGGED(tch, PLR_ARENA))
	  strcat(buf, " #R(arena)#N");
	strcat(buf, "\r\n");
	
      } /* endif shortlist */
      
      strcpy((who_list + ptr), buf);
      ptr += strlen(buf);
      
    } /* end of for */

    if (!sort)
      break;
  }

  if (short_list && (num_can_see % 4)) {
    strcpy((who_list + ptr), "\r\n");
    ptr += strlen("\r\n");
  }
  
  if (num_can_see > max_so_far)
    max_so_far = num_can_see; 
  
  sprintf(buf,"\r\n#N%d visible characters displayed.  Max so far: #R%d#N\r\n",
	  num_can_see, max_so_far);
  
  strcpy((who_list + ptr), buf);
  
  page_string(ch->desc, who_list, TRUE);
}


ACMD(do_finger)
{
  struct char_data *tch = 0;
  struct char_file_u chdata;
  int in_game = TRUE;
  char wiz[20];

  if (!*argument) {
    send_to_char("Finger whom?\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  *chdata.name = 0;

  if (load_char(buf, &chdata) < 0 && !(tch = get_player_vis(ch, buf))) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }

  if ((!tch && !(tch = get_player_vis(ch, buf))) || 
      (*chdata.name != 0 && strcasecmp(GET_NAME(tch), chdata.name))) {
    in_game = FALSE;
    CREATE(tch, struct char_data, 1);
    clear_char(tch);
    store_to_char(&chdata, tch);
    stringdata_load(tch);
  }
   
  /* Do not show deleted characters */
  if (PLR_FLAGGED(tch, PLR_DELETED)) {
    if (!in_game)
      free_char(tch);
    send_to_char("There is no such player.\r\n", ch);
    return;
  }

  CLASS_ABBR(tch, wiz);
  sprintf(buf, "[%3d %5s] %s %s",
	  GET_LEVEL(tch), wiz, GET_NAME(tch),
	  GET_TITLE(tch));
    
  if (CLAN(tch) >= 0)
    sprintf(buf, "%s [%s %s]", buf,
	    CLAN_NAME(tch), CLAN_RANKNAME(tch));
    
  if (in_game && IS_AFFECTED(tch, AFF_GROUP))
    strcat(buf, " (grouped)");
  else
    strcat(buf, " (not grouped)");
    
  if (IS_AFFECTED(tch, AFF_INVISIBLE))
    strcat(buf, " (invis)");
    
  if (PLR_FLAGGED(tch, PLR_MAILING))
    strcat(buf, " (mailing)");
  else if (PLR_FLAGGED(tch, PLR_WRITING))
    strcat(buf, " (writing)");
    
  if (PRF_FLAGGED(tch, PRF_NOTELL))
    strcat(buf, " (notell)");
  if (PRF_FLAGGED(tch, PRF_QUEST))
    strcat(buf, " (quest)");
  if (PLR_FLAGGED(tch, PLR_THIEF))
    strcat(buf, " (THIEF)");
  if (PLR_FLAGGED(tch, PLR_KILLER))
    strcat(buf, " (KILLER)");    

  strcat(buf, "\r\n");
    
  if (!in_game)
    /* 
       if (GET_LEVEL(tch) >= LEVEL_DEITY && (GET_LEVEL(tch) > GET_LEVEL(ch)))   
       sprintf(buf, "%sNot Logged on.\r\n", buf);
       else */
    sprintf(buf, "%sLast logged in on %-20s\r\n",
	    buf, ctime(&chdata.last_logon));

  if(has_mail(GET_NAME(tch))){
    sprintf(buf, "%s%s", buf, get_mail_date(GET_NAME(tch)));
  } else 
    sprintf(buf, "%sHas no new mail.\r\n", buf);

  if(tch->player.plan)
    strcat(buf, tch->player.plan);
  else
    strcat(buf, "No plan.\r\n");

  if (!in_game)
    free_char(tch);

  page_string(ch->desc, buf, TRUE);
}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c clanlist] [-o] [-p]\r\n"

ACMD(do_users)
{
   extern char	*connected_types[];
   char	line[200], line2[220], idletime[10], classname[30], wiz[30];
   char	state[30], *timeptr;

   struct char_data *tch;
   char	name_search[80];
   char	host_search[80];
   char	mode, *format;
   int	low = 0, high = LEVEL_IMPL, i = 0;
   int	clanlist = 0, outlaws = 0, num_can_see = 0, playing = 0, deadweight = 0, clan_search = 0;

   struct descriptor_data *d;

   name_search[0] = '\0';
   host_search[0] = '\0';

   strcpy(buf, argument);
   while (*buf) {
      half_chop(buf, arg, buf1);
      if (*arg == '-') {
	 mode = *(arg + 1); /* just in case; we destroy arg in the switch */
	 switch (mode) {
	 case 'o':
	 case 'k':
	    outlaws = 1;
	    playing = 1;
	    strcpy(buf, buf1);
	    break;
	 case 'p':
	    playing = 1;
	    strcpy(buf, buf1);
	    break;
	 case 'd':
	    deadweight = 1;
	    strcpy(buf, buf1);
	    break;
	 case 'l':
	    playing = 1;
	    half_chop(buf1, arg, buf);
	    sscanf(arg, "%d-%d", &low, &high);
	    break;
	 case 'n':
	    playing = 1;
	    half_chop(buf1, name_search, buf);
	    break;
	 case 'h':
	    playing = 1;
	    half_chop(buf1, host_search, buf);
	    break;
	 case 'c':
	    playing = 1;
	    half_chop(buf1, arg, buf);
	    if (!*arg)
		 clanlist = 2;
	    else {
		 i = atoi(arg);
		 clan_search = real_clan(i);
		 if((clan_search == -1)) {
		      send_to_char("That clan exists only in your imagination.\r\n", ch);
	              return;
		 } else
                      clanlist = 1;
	    }
	    break;
	 default:
	    send_to_char(USERS_FORMAT, ch);
	    return;
	    break;
	 } /* end of switch */

      } else { /* endif */
	 send_to_char(USERS_FORMAT, ch);
	 return;
      }
   } /* end while (parser) */
   strcpy(line,
       "Num Class       Name         State    Idl Login@   Site\r\n");
   strcat(line,
       "-=- -=-=-=-=-=- -=-=-=-=-=-= -=-=-=-= -=- -=-=-=-= -=-=-=-=-=-=-=-=-=-=-=-=-=\r\n");
   send_to_char(line, ch);

   one_argument(argument, arg);

   for (d = descriptor_list; d; d = d->next) {
      if (d->connected && playing)
	 continue;
      if (!d->connected && deadweight)
	 continue;
      if (!d->connected) {
	 if (d->original)
	    tch = d->original;
	 else if (!(tch = d->character))
	    continue;

	 if (*host_search && !strstr(d->host, host_search))
	    continue;
	 if (*name_search && str_cmp(GET_NAME(tch), name_search) && 
	      (!GET_TITLE(tch) || !strstr(GET_TITLE(tch), name_search)))
	    continue;
	 if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	    continue;
	 if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) && 
	     !PLR_FLAGGED(tch, PLR_THIEF))
	    continue;

         if (clanlist == 2) 
     	   if (CLAN(ch) != CLAN(tch))
	     continue;
        
         if ((clanlist == 1) && (clan_search != CLAN(tch)))
             continue;

	 if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	    continue;

	 if (d->original) {
            CLASS_ABBR(d->original, wiz);
	    sprintf(classname, "[%3d %s]", GET_LEVEL(d->original),
	        wiz);
	 } else{
            CLASS_ABBR(d->character, wiz);
	    sprintf(classname, "[%3d %s]", GET_LEVEL(d->character),
	        wiz);
         }
      } else
	 strcpy(classname, "     -     ");

      timeptr = asctime(localtime(&d->login_time));
      timeptr += 11;
      *(timeptr + 8) = '\0';

      if (!d->connected && d->original)
	 strcpy(state, "Switched");
      else
	 strcpy(state, connected_types[d->connected]);

      if (d->character && !d->connected)
	 sprintf(idletime, "%3d",
	     d->character->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
      else
	 strcpy(idletime, "");

      format = "%3d %-7s %-12s %-8s %-3s %-8s ";

      if (d->character && d->character->player.name) {
	 if (d->original)
	    sprintf(line, format, d->desc_num, classname,
		    d->original->player.name, state, idletime, timeptr);
	 else
	    sprintf(line, format, d->desc_num, classname,
		    d->character->player.name, state, idletime, timeptr);
      } else
	 sprintf(line, format, d->desc_num, "     -     ", "UNDEFINED",
		 state, idletime, timeptr);

      if (d->host && *d->host)
	 sprintf(line + strlen(line), "[%s]\r\n", d->host);
      else
	 strcat(line, "[Hostname unknown]\r\n");

      if (d->connected) {
	 sprintf(line2, "ง1G%s#N", line);
	 strcpy(line, line2);
      }

      if (d->connected || (!d->connected && CAN_SEE(ch, d->character))) {
	 send_to_char(line, ch);
	 num_can_see++;
      }
   }

   sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
   send_to_char(line, ch);
}



ACMD(do_inventory)
{
  struct char_data *vict;

  if (GET_LEVEL(ch) >= LEVEL_DEITY) {
    one_argument(argument, buf);
    if (!*buf || !(vict = get_char_vis(ch, buf)))
      vict = ch;
  } else vict = ch;


  if (vict == ch) send_to_char("You are carrying:\r\n", ch);
  else act("$N is carrying:", FALSE, ch, 0, vict, TO_CHAR);
  list_obj_to_char(vict->carrying, ch, 1, TRUE);
}


ACMD(do_equipment)
{
   int	j;
   bool found;
   struct char_data *vict;

   if (GET_LEVEL(ch) >= LEVEL_DEITY) {
     one_argument(argument, buf);
     if (!*buf || !(vict = get_char_vis(ch, buf)))
       vict = ch;
   } else vict = ch;


   if (vict == ch) send_to_char("You are using:\r\n", ch);
   else act("$N is using:", FALSE, ch, 0, vict, TO_CHAR);

   found = FALSE;
   for (j = 0; j < MAX_WEAR; j++) {
      if (vict->equipment[j]) {
	 if (CAN_SEE_OBJ(ch, vict->equipment[j])) {
	    send_to_char(where[j], ch);
	    show_obj_to_char(vict->equipment[j], ch, 1);
	    found = TRUE;
	 } else {
	    send_to_char(where[j], ch);
	    send_to_char("Something.\r\n", ch);
	    found = TRUE;
	 }
      }
   }
   if (!found) {
      send_to_char(" Nothing.\r\n", ch);
   }
}


ACMD(do_gen_ps)
{
   extern char	elitemud_version[];

   void redraw_screen(struct descriptor_data *d);

   switch (subcmd) {
   case SCMD_CREDITS : page_string(ch->desc, credits, 0); break;
   case SCMD_NEWS    : page_string(ch->desc, news, 0); break;
   case SCMD_INFO    : page_string(ch->desc, info, 0); break;
   case SCMD_WIZLIST : page_string(ch->desc, wizlist, 0); break;
   case SCMD_IMMLIST : page_string(ch->desc, immlist, 0); break;
   case SCMD_REMLIST : page_string(ch->desc, remlist, 0); break;
   case SCMD_HANDBOOK: page_string(ch->desc, handbook, 0); break;
   case SCMD_POLICIES: page_string(ch->desc, policies, 0); break;
   case SCMD_NEWBIE  : page_string(ch->desc, newbie, 0); break;
   case SCMD_MOTD    : page_string(ch->desc, motd, 0); break;
   case SCMD_IMOTD   : page_string(ch->desc, imotd, 0); break;
   case SCMD_CLEAR   : 
       send_to_char(VTCLS, ch); 
       if (PRF_FLAGGED(ch, PRF_DISPVT))
	   redraw_screen(ch->desc);
       break;
   case SCMD_VERSION : send_to_char(elitemud_version, ch); break;
   case SCMD_WHOAMI  : send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch); break;
   default: return; break;
   }
}


void perform_mortal_where(struct char_data *ch, char *arg)
{
  ACMD(do_who);
  do_who(ch, "-z", 0, 0);

  /*   register struct char_data *i;
   register struct descriptor_data *d;

   if (!*arg) {
      send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
      for (d = descriptor_list; d; d = d->next)
	 if (!d->connected) {
	    i = (d->original ? d->original : d->character);
            if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
	     (world[ch->in_room]->zone == world[i->in_room]->zone)) {
		sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room]->name);
		send_to_char(buf, ch);
	    }
	 }
   } else { /* print only FIRST char, not all. */ /*
      for (i = character_list; i; i = i->next)
         if (world[i->in_room]->zone == world[ch->in_room]->zone && CAN_SEE(ch, i) &&
	     (i->in_room != NOWHERE) && isname(arg, i->player.name)) {
	    sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room]->name);
	    send_to_char(buf, ch);
	    return;
	 }
      send_to_char("No-one around by that name.\r\n", ch);
   }*/
}


/* This function return the room num a obj is in
 * it will also check if the obj is in another obj and return that roomnum
 */
int obj_in_room(struct obj_data *obj)
{
  if (obj) {
    if (obj->in_room != NOWHERE)
      return obj->in_room;
    if (obj->in_obj)
      return obj_in_room(obj->in_obj);
  }

  return NOWHERE;
}

/* This function return the character who carries an obj
 * if the obj is in a nother obj - that obj's carrier is returned
 */
struct char_data *
obj_carried_by(struct obj_data *obj)
{
  if (obj) {
    if (obj->carried_by)
      return (obj->carried_by);
    if (obj->in_obj)
      return (obj->in_obj->used_by) ?
	   obj->in_obj->used_by :
	 obj_carried_by(obj->in_obj);
  }

  return NULL;
}

void perform_immort_where(struct char_data *ch, char *argument)
{
  register struct char_data *i;
  register struct obj_data *k;
  char buf3[100];
  struct descriptor_data *d;
  int	num=0, found = 0, room, numr = 50, num_per;


  num_per = numr;
  one_argument(argument, buf2);

  if (!*buf2) {
    send_to_char("Players\r\n-------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i)) {
	  if (d->original)
	    sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
		    GET_NAME(i), 
		    world[d->character->in_room]->number,
		    (d->character->in_room != NOWHERE) ? world[d->character->in_room]->name : "NOWHERE", 
		    GET_NAME(d->character));
	  else
	    sprintf(buf, "%-20s - [%5d] %s\r\n", 
		    GET_NAME(i),
		    (i->in_room!=NOWHERE)?world[i->in_room]->number:-1, 
		    (world[d->character->in_room]->number != NOWHERE) ? world[d->character->in_room]->name : "Nowhere");
	  send_to_char(buf, ch);
	}
      }
  } else {
    *buf = '\0';
    *buf3 = '\0';
    two_arguments (argument, buf2, buf3);
    if (*buf3) numr = num_per * atoi(buf3);

    for (i = character_list; (i) && (num < numr) && (strlen(buf) < (MAX_STRING_LENGTH - 200)); i = i->next)
      if (CAN_SEE(ch, i) && isname(buf2, i->player.name)) {
	num++;
	   if (num >= numr - num_per) {
	     found = 1;
	     sprintf(buf, "%s#y%2d#N. %-25s - [#b%5d#N] %s\r\n", buf, 
		     num, 
		     GET_NAME(i),
		     (i->in_room!=NOWHERE)?world[i->in_room]->number:-1, 
		     (i->in_room!=NOWHERE)?world[i->in_room]->name:"Nowhere");
	}
      }
    
    for (k = object_list; (k) && (num < numr) && (strlen(buf) < (MAX_STRING_LENGTH - 200)); k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(buf2, k->name)) {
	   if ((i = (k->used_by?k->used_by:obj_carried_by(k))) && CAN_SEE(ch, i)) {
	     num++;
	     if (num >= numr - num_per) {
		     found = 1;
		     sprintf(buf, "%s#y%2d#N. %s #rINOBJ:#N %s #rON:#N %s - [#b%5d#N] %s\r\n", buf,
			     num, k->short_description,
			     (k->in_obj?k->in_obj->short_description:"None"),
			     GET_NAME(i),
			     (IN_ROOM(i)!=NOWHERE)?world[IN_ROOM(i)]->number:-1,
			     (IN_ROOM(i)!=NOWHERE)?world[IN_ROOM(i)]->name:"Nowhere");
		}
		
	   } else {
		room = obj_in_room(k);
		num++;
		if (num >= numr - num_per) {
		     found = 1;
		     sprintf(buf, "%s#y%2d#N. %s #rINOBJ:#N %s - [#b%5d#N] %s\r\n", buf, num,
			     k->short_description,
			     (k->in_obj?k->in_obj->short_description:"None"),
			     (room!=NOWHERE)?world[room]->number:-1,
			     (room!=NOWHERE)?world[room]->name:"Nowhere");
		}
		
	   }
      }
      
    if (strlen(buf) >= (MAX_STRING_LENGTH - 200))
	 strcat(buf, "String Length Exceeded.\r\n");
    if (!found)
	 send_to_char("Couldn't find any such thing.\r\n", ch);
    else
	 page_string(ch->desc, buf, 1);
  }
}
   


ACMD(do_where)
{
   if (GET_LEVEL(ch) >= LEVEL_DEITY)
      perform_immort_where(ch, argument);
   else
      perform_mortal_where(ch, argument);
}



ACMD(do_levels)
{
   int	i, needed, diff, class;

   one_argument(argument, buf);

   if (!*buf) {
       send_to_char("You must specify a class.\r\n", ch);
       return;
   }

   class = search_block(buf, pc_class_types, FALSE) - 1;

   if (class < 0 || class >= CLASS_NINJA) {
       send_to_char("Not a valid class.\r\n", ch);
       return;
   }

   *buf = '\0';

   for (i = 1; i < 100; i++) {
       needed = titles[class][i/3+1].exp;
       diff = titles[class][i/3+2].exp - needed;
       needed += (diff/3*(i % 3));      
       sprintf(buf + strlen(buf), "[#Y%2d#N] %9d  ",
	       i, needed);
       if (!(i%3)) {
	   switch (GET_SEX(ch)) {
	   case SEX_MALE:
	   case SEX_NEUTRAL:
	       strcat(buf, titles[class][i/3+1].title_m);
	       break;
	   case SEX_FEMALE:
	       strcat(buf, titles[class][i/3+1].title_f);
	       break;
	   default:
	       send_to_char("Oh dear.\r\n", ch);
	   }
	   strcat(buf, "\r\n");
       }
   }
   page_string(ch->desc, buf, TRUE);
}


/* Function to calculate a char's combat rating */
int combat_rating(struct char_data *ch)
{
    int rating;
   
    if (IS_NPC(ch))
	rating = GET_LEVEL(ch);
    else if (IS_DUAL(ch) || IS_2MULTI(ch)) 
	rating = ((GET_1LEVEL(ch) + GET_2LEVEL(ch)) * 3)/4;
    else if (IS_3MULTI(ch))
	rating = ((GET_1LEVEL(ch) + GET_2LEVEL(ch) + GET_3LEVEL(ch)) * 8)/15;
    else
	rating = GET_LEVEL(ch);
    
    rating = (int)(rating * (float)GET_HIT(ch)/(float)GET_MAX_HIT(ch));

    return rating;
}


ACMD(do_consider)
{
   struct char_data *victim;
   int	diff;

   one_argument(argument, buf);

   if (!(victim = get_char_room_vis(ch, buf))) {
      send_to_char("Consider killing who?\r\n", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("Easy!  Very easy indeed!\r\n", ch);
      return;
   }

   if (!IS_NPC(victim)) {
      send_to_char("Would you like to borrow a cross and a shovel?\r\n", ch);
      return;
   }

    if (GET_HIT(victim) < 1) {
       send_to_char("I think it's dead already.\r\n", ch);
       return;
    }
 
    diff = GET_AC(victim) - GET_AC(ch);     /*Print AC comparison */
    if (diff <= -140)
        send_to_char("Your victim is massively better protected than you.\r\n", ch);
    else if (diff <= -80)
        send_to_char("Your victim is well better armored than you!\r\n", ch);
    else if (diff <= -20)
        send_to_char("Your victim is better armored than you.\r\n", ch);
    else if (diff <=  30)
        send_to_char("Your victim is about evenly armored with you.\r\n", ch);
    else if (diff <=  90)
        send_to_char("Your victim lacks some of your protection.\r\n", ch);
    else if (diff <= 150)
        send_to_char("Your victim lacks much of your protection.\r\n", ch);
    else
        send_to_char("Your victim is grossly under armored compared to you.\r\n", ch);
  
 
    diff = (int) (100 - GET_HIT(victim)*100/GET_HIT(ch)); /* HP % Compare */ 
    if (diff <= -49)
        send_to_char("Your victim is massively healthier than you!\r\n", ch);
    else if (diff <= -29)
        send_to_char("Your victim is considerably healthier than you.\r\n", ch);
    else if (diff <=  -9)
        send_to_char("Your victim is healthier than you.\r\n", ch);
    else if (diff <=  10)
        send_to_char("Your victim is about the same health as you.\r\n", ch);
    else if (diff <=  30)
        send_to_char("Your victim is not as healthy as you.\r\n", ch);
    else if (diff <=  50)
        send_to_char("Your victim lacks your vigor.\r\n", ch);
    else
        send_to_char("Your victim is puny in comparison.\r\n", ch);
 

   diff = (combat_rating(victim) - combat_rating(ch));
   
   if (diff <= -10)
       send_to_char("Now where did that chicken go?\r\n", ch);
   else if (diff <= -5)
       send_to_char("You could do it with a needle!\r\n", ch);
   else if (diff <= -2)
       send_to_char("Easy.\r\n", ch);
   else if (diff <= -1)
       send_to_char("Fairly easy.\r\n", ch);
   else if (diff == 0)
       send_to_char("The perfect match!\r\n", ch);
   else if (diff <= 1)
       send_to_char("You would need some luck!\r\n", ch);
   else if (diff <= 2)
       send_to_char("You would need a lot of luck!\r\n", ch);
   else if (diff <= 3)
       send_to_char("You would need a lot of luck and great equipment!\r\n", ch);
   else if (diff <= 5)
       send_to_char("Do you feel lucky, punk?\r\n", ch);
   else if (diff <= 10)
       send_to_char("Are you mad!?\r\n", ch);
   else if (diff <= 15)
       send_to_char("You ARE mad!\r\n", ch);
   else if (diff <= 20)
       send_to_char("Why not pretend you are dead instead?\r\n", ch);
   else if (diff <= 30)
       send_to_char("Your brain will be a nice decoration on the walls!!", ch);
   else
       send_to_char("You are a very dumb player for even considering.\r\n", ch);
}


ACMD(do_toggle)
{
  ACMD(do_gen_tog);

    char  *togglecommands[] = {
	"help",
	"summon",
	"brief",
	"compact",
	"tell",
	"auction",
	"newbie",
	"gossip",
	"chat",
	"quest",
	"repeat",
	"speedwalk",
	"alias",
	"verbatim",
	"exits",
	"gag",
	"pc",
	"grat",
	"clantitle",
	"clantell",
	"linewrap",
	"autoassist",
	"nowho",
        "detail",
        "pksay",
	"\n"
	};

    int togglescmds[] = {
	0,
	SCMD_NOSUMMON, 
	SCMD_BRIEF,
	SCMD_COMPACT,
	SCMD_NOTELL,
	SCMD_NOAUCTION,
	SCMD_NONEWBIE,
	SCMD_NOGOSSIP,
	SCMD_NOCHAT,
	SCMD_QUEST,
	SCMD_NOREPEAT,
	SCMD_NOSPDWLK,
	SCMD_NOALIAS,
	SCMD_VERBATIM,
	SCMD_EXITS,
	SCMD_GAG,
	SCMD_PC,
	SCMD_NOGRAT,
	SCMD_NOCLANTIT,
	SCMD_NOCLANTELL,
	SCMD_LINEWRAP,
	SCMD_AUTOASSIST,
	SCMD_NOWHO,
        SCMD_DETAIL,
        SCMD_NOPKSAY,
	0
	};

    int num;

   if (IS_NPC(ch))
      return;

    one_argument(argument, buf);

    if (!*buf) {
	
	if (WIMP_LEVEL(ch) == 0)
	    strcpy(buf2, "OFF");
	else
	    sprintf(buf2, "%-3d", WIMP_LEVEL(ch));
	
	sprintf(buf,
		"You current settings:  Type TOGGLE HELP to get help.\r\n\r\n"
		"#w   Alias          : %-3s   "
		"#wVerbatim       : %-3s   "
		"#wSpeedwalk      : %-3s\r\n"
		"#w   Brief Mode     : %-3s   "
		"#wCompact Mode   : %-3s   "
		"#wRepeat Mode    : %-3s\r\n"
		"#w   Chat Channel   : %-3s   "
		"#wGossip Channel : %-3s   "
		"#wGratz Channel  : %-3s\r\n"
		"#w   Auction Channel: %-3s   "
                "#wPK Say Channel : %-3s   "
		"#wQuest Channel  : %-3s\r\n"
		"#w   Clan Tell      : %-3s   "
		"#wDeaf to Newbie : %-3s   "
		"#wDeaf to Tells  : %-3s\r\n"
		"#w   Summon Protect : %-3s   "
		"#wExit Display   : %-3s   "
		"#wWimp Out At    : %-3s\r\n"
		"#w   VT Display     : %-3s   " 
		"#wMessage gag    : %-3s   "
		"#wScreen Length  : %2d\r\n"
		"#w   Linewrap       : %-3s   "
		"#wPC/IBM-MODE    : %-3s   "
		"#wAutoassist     : %-3s#N\r\n",
		ONOFF(!PRF_FLAGGED(ch, PRF_NOALIAS)),
		YESNO(PRF_FLAGGED(ch, PRF_VERBATIM)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOSPDWLK)),
		YESNO(PRF_FLAGGED(ch, PRF_BRIEF)),
		YESNO(PRF_FLAGGED(ch, PRF_COMPACT)),
		YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOCHAT)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOGRAT)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
                ONOFF(!PLR_FLAGGED(ch, PLR_NOPKSAY)),
		ONOFF(PRF_FLAGGED(ch, PRF_QUEST)),
		ONOFF(!PLR_FLAGGED(ch, PLR_NOCLANTELL)),
		YESNO(PRF_FLAGGED(ch, PRF_NONEWBIE)),
		YESNO(PRF_FLAGGED(ch, PRF_NOTELL)),
		YESNO(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOEXITS)),
		buf2,
		ONOFF(PRF_FLAGGED(ch, PRF_DISPVT)),
		ONOFF(PRF_FLAGGED(ch, PRF_GAG)),
		GET_SCRLEN(ch),
		ONOFF(PLR_FLAGGED(ch, PLR_LINEWRAP)),
		ONOFF(PRF_FLAGGED(ch, PRF_IBM_PC)),
		ONOFF(PLR_FLAGGED(ch, PLR_AUTOASSIST)));
	send_to_char(buf, ch);
        if (GET_LEVEL(ch) > LEVEL_RETIRED) {
          sprintf(buf, "#w   Nowho          : %-3s#N\r\n", ONOFF(PLR_FLAGGED(ch, PLR_NOWHO)));
          send_to_char(buf, ch);
        }
        if (PLR_FLAGGED(ch, PLR_TEST)) {
          sprintf(buf, "#w   Damage Detail  : %-3s#N\r\n", ONOFF(PLR_FLAGGED(ch, PLR_DETAIL)));
          send_to_char(buf, ch);
        }
	return;
    }
    
    num = search_block(buf, togglecommands, 0);

    if (num < 1) {
	send_to_char("You can toggle the following:\r\n", ch);
	display_string_array(ch, &togglecommands[1]);
	send_to_char("\r\nUse DISPLAY command to change display.\r\n", ch);
    } else
	do_gen_tog(ch, argument, cmd, togglescmds[num]);
}



struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void sort_commands(void)
{
  int a, b, tmp;
  
  ACMD(do_action);
  
  num_of_cmds = 0;
  
  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;
  
  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);
  
  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }
  
  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;
  cmd_sort_info[find_command("pose")].is_social = TRUE;
  
  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}


ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;
  
  one_argument(argument, arg);
  
  if (*arg) {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;
  
  sprintf(buf, "#wThe following %s%s are available to %s:#N\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  if (!wizhelp)
    for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
      i = cmd_sort_info[cmd_num].sort_pos;
      if (cmd_info[i].minimum_level >= 0 &&
	  (cmd_info[i].minimum_level < LEVEL_DEITY) &&
	  GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
	  (socials == cmd_sort_info[i].is_social)) {
	sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
	if (!(no % 7))
	  strcat(buf, "\r\n");
	no++;
      }
    }
  else {
/*
 * This generates zero output but is used unnecessarily everytime
 * a 111 - 114 types wizh - thus commented out - Bod 05/02/02
    if (GET_LEVEL(vict) > LEVEL_DEITY)
      for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
	i = cmd_sort_info[cmd_num].sort_pos;
	if(cmd_info[i].minimum_level > LEVEL_DEITY && 
	   GET_LEVEL(vict) >= cmd_info[i].minimum_level) {
	  sprintf(buf + strlen(buf), " [%3d] %-12s", 
		  cmd_info[i].minimum_level, cmd_info[i].command);
	  if (!(no % 4))
	    strcat(buf, "\r\n");
	  no++;
	}
      }
 */
    strcat(buf, "\r\n");
    for (no=1, cmd_num=0;*immcmd_info[cmd_num].command != '\n'; cmd_num++) {
      if (IS_SET(GODLEVEL(vict), IMM_ALL) ||
	  (GODLEVEL(vict) & immcmd_info[cmd_num].godlevel)) {
	sprintf(buf + strlen(buf), "%-13s", immcmd_info[cmd_num].command);
	if (!(no % 5))
	  strcat(buf, "\r\n");
	no++;
      }
    }
  }
  
  strcat(buf, "#N\r\n");
  page_string(ch->desc, buf, TRUE);
}

ACMD(do_diagnose)
{
   struct char_data *vict;

   one_argument(argument, buf);

   if (*buf) {
      if (!(vict = get_char_room_vis(ch, buf))) {
	 send_to_char("No-one by that name here.\r\n", ch);
	 return;
      } else
	 diag_char_to_char(vict, ch);
   } else {
      if (ch->specials.fighting)
	 diag_char_to_char(ch->specials.fighting, ch);
      else
	 send_to_char("Diagnose who?\r\n", ch);
   }
}


/*****************************************************************************
* New screen handling using vt102 code  -Petrus   940518                     *
*****************************************************************************/

/* write string to screen at x,y pos -Petrus */
int  write_to_pos(struct descriptor_data *d, int x, int y, char *text, int mode)
{
    char vt[32];
    byte ok = 0;

    if(PRF_FLAGGED(d->character, PRF_DISPANSI)) {
	sprintf(vt, "\033[37m\0337" VTCURSOFF VTCURPOS, y, x);
	if (mode == 1)   /* reverse */
	    strcat(vt, VTREVERSE);
    } else
	sprintf(vt, VTCURPOS, y, x);
    
    if (write_to_descriptor(d->descriptor, vt) < 0)
	ok = -1;
    if (write_to_descriptor(d->descriptor, text) < 0)
	ok = -1;
    
    if(PRF_FLAGGED(d->character, PRF_DISPANSI)) {
	strcpy(vt, "\0338" VTCURSON);
	if (mode)
	    strcat(vt, VTOFF);
    } else
	sprintf(vt, VTCURPOS VTDELEOS, GET_SCRLEN(d->character), 1);
    if (write_to_descriptor(d->descriptor, vt) < 0)
	ok = -1;

    return ok;
}


char * convert_to_color(struct descriptor_data *d, char *text)
{
  static char output[SMALL_BUFSIZE];
  
  int colorcode = SCRCOL_REMCODE;

  if (d && d->character) {
    if (PRF_FLAGGED((d->character), PRF_COLOR_1))
      colorcode |= SCRCOL_ADDCODE1;
    if (PRF_FLAGGED((d->character), PRF_COLOR_2))
      colorcode |= SCRCOL_ADDCODE2;
    if (PRF_FLAGGED((d->character), PRF_DISPVT))
      colorcode |= SCRCOL_ADDCODEVT;
    if (PRF_FLAGGED((d->character), PRF_IBM_PC))
      colorcode |= SCRCOL_ADDCODEPC;
  }
  
  scrcol_copy(output, text, colorcode, 99999);
  
  return output;
}


void  diag_to_screen(struct descriptor_data *d)
{
    if(!PRF_FLAGGED(d->character, PRF_DISPVT))
	return;
    
    if(d->character->specials.fighting) {
	sprintf(buf, "%s:%s", 
		GET_NAME(d->character->specials.fighting),
		diag_to_prompt(d->character->specials.fighting, 1));
	CAP(buf);
	write_to_pos(d, 2, GET_SCRLEN(d->character) - 1,
		     convert_to_color(d, buf), 0);
    }
}


/* displays the stats on the info-bar */

void  stats_to_screen(struct descriptor_data *d)
{
    if (PRF_FLAGGED(d->character, PRF_IBM_PC)) {
	sprintf(buf, "บ#g%5dHp#m%5dMn#b%5dMv#Nบ #R%4d%sexp#Nบ #y%4d%sgold#Nบ ",
		GET_HIT(d->character), GET_MANA(d->character), 
		GET_MOVE(d->character),
		
		(GET_EXP(d->character)/1000000 >9)?(GET_EXP(d->character)/1000000):((GET_EXP(d->character)/1000>9)?(GET_EXP(d->character)/1000):(GET_EXP(d->character))), 
		(GET_EXP(d->character)/1000000 >9)?"M":((GET_EXP(d->character)/1000>9)?"k":" "),
		(GET_GOLD(d->character)/1000000 >9)?(GET_GOLD(d->character)/1000000):((GET_GOLD(d->character)/1000>9)?(GET_GOLD(d->character)/1000):(GET_GOLD(d->character))),
		(GET_GOLD(d->character)/1000000 >9)?"M":((GET_GOLD(d->character)/1000>9)?"k":" "));
	
	write_to_pos(d, 1, GET_SCRLEN(d->character) - 2, convert_to_color(d, buf), 0);
    } else {
	sprintf(buf, "|#g%5dHp#m%5dMn#b%5dMv#N  #R%4d%sexp  #y%4d%sgold#N  ",
		GET_HIT(d->character), GET_MANA(d->character), 
		GET_MOVE(d->character),
		
		(GET_EXP(d->character)/1000000 >9)?(GET_EXP(d->character)/1000000):((GET_EXP(d->character)/1000>9)?(GET_EXP(d->character)/1000):(GET_EXP(d->character))), 
		(GET_EXP(d->character)/1000000 >9)?"M":((GET_EXP(d->character)/1000>9)?"k":" "),
		(GET_GOLD(d->character)/1000000 >9)?(GET_GOLD(d->character)/1000000):((GET_GOLD(d->character)/1000>9)?(GET_GOLD(d->character)/1000):(GET_GOLD(d->character))),
		(GET_GOLD(d->character)/1000000 >9)?"M":((GET_GOLD(d->character)/1000>9)?"k":" "));
    }	
    
    write_to_pos(d, 1, GET_SCRLEN(d->character) - 2, convert_to_color(d, buf), 0);
    
    
    diag_to_screen(d);
}

/* display all the affects on the screen */
void  aff_to_screen(struct descriptor_data *d)
{
    if(!d || !PRF_FLAGGED(d->character, PRF_DISPVT))
	return;

    if (PRF_FLAGGED(d->character, PRF_IBM_PC)) {

	if (GET_INVIS_LEV(d->character)) 
	    sprintf(buf2, "#bi%-3d#N", GET_INVIS_LEV(d->character));
	else
	    strcpy(buf2, "ออออ");
	
	sprintf(buf, "ษอ%sอ%sอ%sอ%sห%sอ%sอ%sอ%sอ%sอ%sอ%sอ%sอ%sออออออป",
		(IS_AFFECTED(d->character, AFF_BLIND)?"#WBLIND#N":"อออออ"),
		(IS_AFFECTED(d->character, AFF_INVISIBLE)?"#BINV#N":"อออ"),
		(IS_AFFECTED(d->character, AFF_SANCTUARY)?"#ySANC#N":"ออออ"),
		(IS_AFFECTED(d->character, AFF_CURSE)?"#RCURSE#N":"อออออ"),
		(IS_AFFECTED(d->character, AFF_POISON)?"#RPOISON#N":"ออออออ"),
		(IS_AFFECTED(d->character, AFF_PARALYSIS)?"#MPARA#N":"ออหอ"),
		(IS_AFFECTED(d->character, AFF_SNEAK)?"#YSNEAK#N":"อออออ"),
		(IS_AFFECTED(d->character, AFF_FEAR)?"#yFEAR#N":"ออหอ"),
		(IS_AFFECTED(d->character, AFF_INFRARED)?"#rINFRA#N":"อออออ"),
		(IS_AFFECTED(d->character, AFF_HOVER)?"#GHOVER#N":"อออออ"),
		(IS_AFFECTED(d->character, AFF_FLY)?"#gFLY#N":"อออ"),
		(IS_AFFECTED(d->character, AFF_BREATH_WATER)?"#BBWATER#N":"ออออออ"),
		buf2
		);
	
    } else {

	if (GET_INVIS_LEV(d->character)) 
	    sprintf(buf2, "#bi%-3d#N", GET_INVIS_LEV(d->character));
	else
	    strcpy(buf2, "----");
	
	sprintf(buf, "*-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s------*",
		(IS_AFFECTED(d->character, AFF_BLIND)?"#WBLIND#N":"-----"),
		(IS_AFFECTED(d->character, AFF_INVISIBLE)?"#BINV#N":"---"),
		(IS_AFFECTED(d->character, AFF_SANCTUARY)?"#ySANC#N":"----"),
		(IS_AFFECTED(d->character, AFF_CURSE)?"#RCURSE#N":"-----"),
		(IS_AFFECTED(d->character, AFF_POISON)?"#RPOISON#N":"------"),
		(IS_AFFECTED(d->character, AFF_PARALYSIS)?"#MPARA#N":"----"),
		(IS_AFFECTED(d->character, AFF_SNEAK)?"#YSNEAK#N":"-----"),
		(IS_AFFECTED(d->character, AFF_FEAR)?"#yFEAR#N":"----"),
		(IS_AFFECTED(d->character, AFF_INFRARED)?"#rINFRA#N":"-----"),
		(IS_AFFECTED(d->character, AFF_HOVER)?"#GHOVER#N":"-----"),
		(IS_AFFECTED(d->character, AFF_FLY)?"#gFLY#N":"---"),
		(IS_AFFECTED(d->character, AFF_BREATH_WATER)?"#BBWATER#N":"------"),
		buf2
		);
    }    
    
    write_to_pos(d, 1, GET_SCRLEN(d->character) - 3,
		 convert_to_color(d, buf), 0);
}


void  line_to_screen(struct descriptor_data *d)
{ 	
    if(!PRF_FLAGGED(d->character, PRF_DISPVT))
	return;
    
    if (PRF_FLAGGED(d->character, PRF_IBM_PC)) {
	write_to_pos(d, 1, GET_SCRLEN(d->character) - 1,
		     convert_to_color(d, "#wศอออออออออออออออออออออสอออออออออสออออออออออสอออออออออออออออออออออออออออออออออออผ#N"), 0);
    } else {
	write_to_pos(d, 1, GET_SCRLEN(d->character) - 1,
		     convert_to_color(d, "*------------------------------------------------------------------------------*"), 0);
    } 
}


/* function to redraw the info-bar and set scroll size etc */
void  redraw_screen(struct descriptor_data *d)
{
    if(!PRF_FLAGGED(d->character, PRF_DISPVT))
	return;

    sprintf(buf, VTCURPOS VTDELEOS VTSCRREG, 
	    GET_SCRLEN(d->character) - 4, 1,
	    1, GET_SCRLEN(d->character) - 4);
    write_to_descriptor(d->descriptor, buf);
    
    aff_to_screen(d);

    if (PRF_FLAGGED(d->character, PRF_IBM_PC)) {
	write_to_pos(d, 1, GET_SCRLEN(d->character) - 2,"บ     Hp     Mn     Mvบ      expบ      goldบ                                   บ", 0); 
    } else {
	write_to_pos(d, 1, GET_SCRLEN(d->character) - 2,"|     Hp     Mn     Mv       exp       gold                                    |", 0); 
    }
    
    stats_to_screen(d);

    line_to_screen(d);

    diag_to_screen(d);

    sprintf(buf, VTCURPOS VTDELEOS, GET_SCRLEN(d->character), 1);
    write_to_descriptor(d->descriptor, buf);
}


ACMD(do_redraw)
{
    if (!PRF_FLAGGED(ch, PRF_DISPVT)) {
	send_to_char("You have to have advanced display on.\r\n", ch);
	return;
    }

    if (ch->desc)
	redraw_screen(ch->desc);
}

ACMD(do_resize)
{
    int num;

    argument = one_argument(argument, buf);

    if (!(num = atoi(buf))) {
	send_to_char("Have to supply a number.\r\n", ch);
	return;
    }

    num = MAX(num, 20);
    num = MIN(num, 99);

    GET_SCRLEN(ch) = num;

    if (PRF_FLAGGED(ch, PRF_DISPVT))
	redraw_screen(ch->desc);
    else if (ch->desc) {
	sprintf(buf, VTSCRREG VTCLS, 1, num);
	write_to_descriptor(ch->desc->descriptor, buf);
    }

    sprintf(buf, "Ok. New screen length is %d.\r\n", num);
    send_to_char(buf, ch);
}


ACMD(do_appraise)
{
     struct obj_data *obj;
     char buf[SMALL_BUFSIZE];
     int modify;
     one_argument(argument, buf);

     if (*buf) {
	  if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying)))
	       sprintf(buf2, "You don't seem to have any %ss.\r\n", buf);
	  else {
	       if (number(1, 101) > GET_SKILL(ch, SKILL_APPRAISE))
		    sprintf(buf2, "The %s is worth %d gold coins.\r\n", buf, obj->obj_flags.cost);
	       
	       else {
		    modify = (obj->obj_flags.cost)*number(5, 20)/100;
		    if (number(1, 2) == 2)
			 sprintf(buf2, "The %s is worth about %d gold coins.\r\n", buf, obj->obj_flags.cost + modify);
		    else
			 sprintf(buf2, "The %s is worth about %d gold coins.\r\n", buf, obj->obj_flags.cost - modify);
	       }
	  }
	  send_to_char(buf2, ch);
     } else
	  send_to_char("Appraise what?\r\n", ch);

}


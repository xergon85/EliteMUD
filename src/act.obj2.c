/* ************************************************************************
*   File: act.obj2.c                                    Part of EliteMUD  *
*  Usage: eating/drinking and wearing/removing equipment                  *
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

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data **world;
extern char	*drinks[];
extern int	drink_aff[][3];
extern struct obj_data *obj_proto;


/* This func damages an object and returns TRUE if it breaks */
int     damage_object(struct obj_data *obj, int dam)
{
    if (obj && GET_ITEM_HP(obj) > 0) {
	GET_ITEM_HP(obj) -= dam;
	if (GET_ITEM_HP(obj) < 1) {
	    GET_ITEM_HP(obj) = 0;
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_BROKEN);
	    return 1;
	}
    }

    return 0;
}


void	weight_change_object(struct obj_data *obj)
{
   struct obj_data *tmp_obj;
   struct char_data *tmp_ch;
   int weight = 0;

   /* Calculate the weight change */

   weight = -GET_OBJ_WEIGHT(obj) + (obj_proto[obj->item_number].obj_flags.weight + (obj->obj_flags.value[1] - obj->obj_flags.value[0])/10);

   if (weight == 0)
     return;

   if (obj->in_room != NOWHERE) {
      GET_OBJ_WEIGHT(obj) += weight;
   } else if ((tmp_ch = obj->carried_by)) {
      obj_from_char(obj);
      GET_OBJ_WEIGHT(obj) += weight;
      obj_to_char(obj, tmp_ch);
   } else if ((tmp_obj = obj->in_obj)) {
      obj_from_obj(obj);
      GET_OBJ_WEIGHT(obj) += weight;
      obj_to_obj(obj, tmp_obj);
   } else {
      log("SYSERR: Unknown attempt to subtract weight from an object.");
   }
   if (GET_OBJ_WEIGHT(obj) < 0) GET_OBJ_WEIGHT(obj) = 0;
}

void name_from_drinkcon(struct obj_data *obj)
{
  char *liqname, *new_name, *cur_name, *next;
  int liqlen, cpylen;
  extern struct obj_data *obj_proto;
  extern char  *drinknames[];

  if (!obj || (GET_ITEM_TYPE(obj) != ITEM_DRINKCON && GET_ITEM_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  liqname = drinknames[GET_ITEM_VALUE(obj, 2)];
  if (!isname(liqname, obj->name)) {
    sprintf(buf, "SYSERR: Can't remove liquid '%s' from '%s' (%d) item.", liqname, obj->name, obj->item_number);
    log(buf);
    return;
  }

  liqlen = strlen(liqname);
  CREATE(new_name, char, strlen(obj->name) - strlen(liqname)); /* +1 for NUL, -1 for space */

  for (cur_name = obj->name; cur_name; cur_name = next) {
    if (*cur_name == ' ')
      cur_name++;

    if ((next = strchr(cur_name, ' ')))
      cpylen = next - cur_name;
    else
      cpylen = strlen(cur_name);

    if (!strn_cmp(cur_name, liqname, liqlen))
      continue;

    if (*new_name)
      strcat(new_name, " ");    /* strcat: OK (size precalculated) */
    strncat(new_name, cur_name, cpylen);        /* strncat: OK (size precalculated) */
  }

  if (obj->item_number < 0 || obj->name != obj_proto[obj->item_number].name)
    free(obj->name);
  obj->name = new_name;
}



void name_to_drinkcon(struct obj_data *obj, int type)
{
  char *new_name;
  extern struct obj_data *obj_proto;
  extern char  *drinknames[];

  if (!obj || (GET_ITEM_TYPE(obj) != ITEM_DRINKCON && GET_ITEM_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", obj->name, drinknames[type]);      /* sprintf: OK */

  if (obj->item_number < 0 || obj->name != obj_proto[obj->item_number].name)
    free(obj->name);

  obj->name = new_name;
}



ACMD(do_drink)
{
   struct obj_data *temp;
   struct affected_type af;
   int	amount;
   int	on_ground = 0;

   one_argument(argument, arg);

   if (!*arg) {
      send_to_char("Drink from what?\r\n", ch);
      return;
   }

   if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room]->contents))) {
	 act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      } else
	 on_ground = 1;
   }

   if(GET_RACE(ch) == RACE_VAMPIRE) {
       if(!(GET_ITEM_TYPE(temp) == ITEM_CONTAINER)
		&& (temp->obj_flags.value[3] < 1))  {
	act("You can only drink from corpses.", FALSE, ch, 0, 0, TO_CHAR);
	return;
       }
       if(temp->obj_flags.value[4] == 0) {
         act("This corpse is already drained.", FALSE, ch, 0, 0, TO_CHAR);
         return;
       }
       if(GET_COND(ch, THIRST) > 23) {
	send_to_char("You can drink no more blood.\r\n", ch);
	return;
       }
       if((temp->obj_flags.value[4] < 4) &&
	(temp->obj_flags.value[4] > 0)) {
	 act("$n drinks blood from $p.", TRUE, ch, temp, 0, TO_ROOM);
	 act("You drink blood from $p.", FALSE, ch, temp, 0, TO_CHAR);
	 act("You drank old blood!", FALSE, ch, 0, 0, TO_CHAR);
         af.type = SPELL_POISON;
         af.duration = 13;
         af.modifier = 0;
         af.location = APPLY_NONE;
         af.bitvector = AFF_POISON;
         affect_join(ch, &af, FALSE, FALSE);
	 temp->obj_flags.value[4] -= 1;
	 return;
       }

       act("You drink blood from $p.", FALSE, ch, temp, 0, TO_CHAR);
       act("$n drinks blood from $p.", TRUE, ch, temp, 0, TO_ROOM);

       temp->obj_flags.value[4] -= 1;

       if(temp->obj_flags.value[4] == 3)
	temp->obj_flags.value[4] = 0;

       if(temp->obj_flags.value[5] < (GET_LEVEL(ch) - 40)) {
         act("This blood is too weak to sustain you.", FALSE, ch, 0, 0, TO_CHAR);
         return;
       }

       gain_condition(ch, THIRST, 1);


       if(GET_COND(ch, THIRST) > 20) {
	send_to_char("Your craving for blood has been sated.\r\n", ch);
       }
       if(temp->obj_flags.value[5] > (GET_LEVEL(ch) - 10)) {
	 GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + number(1, 6));
       }
       if(temp->obj_flags.value[5] >= (GET_LEVEL(ch))) {
	 GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + number(1, 15));
       }

       return;
    }   /* End of PHOBOS blood vampire loop */

   if ((GET_ITEM_TYPE(temp) != ITEM_DRINKCON) &&
       (GET_ITEM_TYPE(temp) != ITEM_FOUNTAIN)) {
      send_to_char("You can't drink from that!\r\n", ch);
      return;
   }

   if (on_ground && (GET_ITEM_TYPE(temp) == ITEM_DRINKCON)) {
      send_to_char("You have to be holding that to drink from it.\r\n", ch);
      return;
   }

   if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
      /* The pig is drunk */
      send_to_char("You can't seem to get close enough to your mouth.\r\n", ch);
      act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
      return;
   }

   if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0)) {
      send_to_char("Your stomach can't contain anymore!\r\n", ch);
      return;
   }

   if (!temp->obj_flags.value[1]) {
      send_to_char("It's empty.\r\n", ch);
      return;
   }

   if (CLANEQ(temp))
	if ((CLANEQ_CLAN(temp) != CLAN(ch) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) ||
            (CLAN_LEVEL(ch) <= 1)) {
	     send_to_char("You aren't in the correct clan to drink that.\r\n", ch);
	     return;
	}

   if (subcmd == SCMD_DRINK) {
/*      if (temp->obj_flags.type_flag != ITEM_FOUNTAIN) { */
         sprintf(buf, "$n drinks %s from $p.", drinks[temp->obj_flags.value[2]]);
         act(buf, TRUE, ch, temp, 0, TO_ROOM);
   /*   } */

      sprintf(buf, "You drink the %s.\r\n", drinks[temp->obj_flags.value[2]]);
      send_to_char(buf, ch);

      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
         amount = (25 - GET_COND(ch, THIRST)) / drink_aff[temp->obj_flags.value[2]][DRUNK];
      else
         amount = number(3,10);

   } else {
      act("$n sips from the $o.", TRUE, ch, temp, 0, TO_ROOM);
      sprintf(buf, "It tastes like %s.\r\n", drinks[temp->obj_flags.value[2]]);
      send_to_char(buf, ch);
      amount = 1;
   }

   /* Can't drink more than what's there */
   amount = MIN(amount, temp->obj_flags.value[1]);

   gain_condition(ch, DRUNK,
      (int)((int)drink_aff[temp->obj_flags.value[2]][DRUNK]*amount) / 4);

   gain_condition(ch, FULL,
      (int)((int)drink_aff[temp->obj_flags.value[2]][FULL]*amount) / 4);

   gain_condition(ch, THIRST,
      (int)((int)drink_aff[temp->obj_flags.value[2]][THIRST]*amount) / 4);

   if (GET_COND(ch, DRUNK) > 10)
      send_to_char("You feel drunk.\r\n", ch);

   if (GET_COND(ch, THIRST) > 20)
      send_to_char("You don't feel thirsty any more.\r\n", ch);

   if (GET_COND(ch, FULL) > 20)
      send_to_char("You are full.\r\n", ch);

   if (temp->obj_flags.value[3]) { /* The shit was poisoned ! */
      send_to_char("Oops, it tasted rather strange!\r\n", ch);
      act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = amount * 3;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join(ch, &af, FALSE, FALSE);
   }

   /* empty the container, and no longer poison. */
   temp->obj_flags.value[1] -= amount;

   weight_change_object(temp);

   if (!temp->obj_flags.value[1]) {  /* The last bit */
      name_from_drinkcon(temp);
      temp->obj_flags.value[2] = 0;
      temp->obj_flags.value[3] = 0;
   }

   return;
}



ACMD(do_eat)
{
   struct obj_data *food;
   struct affected_type af;
   int	amount;

   one_argument(argument, arg);

   if(GET_RACE(ch) == RACE_VAMPIRE)  {
	send_to_char("Vampires must drink blood to survive.\r\n", ch);
	return;
   }

   if (!*arg) {
      send_to_char("Eat what?\r\n", ch);
      return;
   }

   if (!(food = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      send_to_char("You don't seem to have any.\r\n", ch);
      return;
   }

   if (subcmd == SCMD_TASTE && ((GET_ITEM_TYPE(food) == ITEM_DRINKCON) ||
      (GET_ITEM_TYPE(food) == ITEM_FOUNTAIN))) {
         do_drink(ch, argument, 0, SCMD_SIP);
	 return;
   }

   if ((GET_ITEM_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LEVEL_DEITY)) {
      send_to_char("You can't eat THAT!\r\n", ch);
      return;
   }

   if (GET_COND(ch, FULL) > 20) { /* Stomach full */
      act("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if (CLANEQ(food))
	if ((CLANEQ_CLAN(food) != CLAN(ch) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) ||
            (CLAN_LEVEL(ch) <= 1)) {
	     send_to_char("You aren't in the correct clan to eat that.\r\n", ch);
	     return;
	}

   if (subcmd == SCMD_EAT) {
      act("You eat $p.", FALSE, ch, food, 0, TO_CHAR);
      act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
   } else {
      act("You nibble a little bit of $p.", FALSE, ch, food, 0, TO_CHAR);
      act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
   }

   amount = (subcmd == SCMD_EAT ? food->obj_flags.value[0] : 1);

   gain_condition(ch, FULL, amount);

   if (GET_COND(ch, FULL) > 20)
      act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

   if (food->obj_flags.value[3] && (GET_LEVEL(ch) < LEVEL_DEITY)) {
      /* The shit was poisoned ! */
      send_to_char("Oops, that tasted rather strange!\r\n", ch);
      act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = amount * 2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join(ch, &af, FALSE, FALSE);
   }

   if (subcmd == SCMD_EAT)
      extract_obj(food);
   else {
      if (!(--food->obj_flags.value[0])) {
	 send_to_char("There's nothing left now.\r\n", ch);
	 extract_obj(food);
      }
   }
}


ACMD(do_pour)
{
   char	arg1[MAX_INPUT_LENGTH];
   char	arg2[MAX_INPUT_LENGTH];
   struct obj_data *from_obj = NULL;
   struct obj_data *to_obj = NULL;
   int	amount;

   two_arguments(argument, arg1, arg2);

   if (subcmd == SCMD_POUR) {
      if (!*arg1) /* No arguments */ {
	 act("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	 act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if (from_obj->obj_flags.type_flag != ITEM_DRINKCON) {
	 act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }
   }

   if (subcmd == SCMD_FILL) {
      if (!*arg1) /* no arguments */ {
	 send_to_char("What do you want to fill?  And what are you filling it from?\r\n", ch);
	 return;
      }

      if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	 send_to_char("You can't find it!", ch);
	 return;
      }

      if (GET_ITEM_TYPE(to_obj) != ITEM_DRINKCON) {
	 act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
	 return;
      }

      if (!*arg2) /* no 2nd argument */ {
	 act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
	 return;
      }

      if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room]->contents))) {
	 sprintf(buf, "There doesn't seem to be any '%s' here.\r\n", arg2);
	 send_to_char(buf, ch);
	 return;
      }

      if (GET_ITEM_TYPE(from_obj) != ITEM_FOUNTAIN) {
	 act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
	 return;
      }
   }

   if (from_obj->obj_flags.value[1] == 0) {
      act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
   }

   if (subcmd == SCMD_POUR) /* pour */ {
      if (!*arg2) {
	 act("Where do you want it?  Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if (!str_cmp(arg2, "out")) {
	 act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
	 act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

	 /* Empty */

         name_from_drinkcon(from_obj);
	 from_obj->obj_flags.value[1] = 0;
	 from_obj->obj_flags.value[2] = 0;
	 from_obj->obj_flags.value[3] = 0;
	 weight_change_object(from_obj);

	 return;
      }

      if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
	 act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if ((to_obj->obj_flags.type_flag != ITEM_DRINKCON) &&
          (to_obj->obj_flags.type_flag != ITEM_FOUNTAIN)) {
	 act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }
   }

   if (to_obj == from_obj) {
      act("A most unproductive effort.", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if ((to_obj->obj_flags.value[1] != 0) &&
       (to_obj->obj_flags.value[2] != from_obj->obj_flags.value[2])) {
      act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if (!(to_obj->obj_flags.value[1] < to_obj->obj_flags.value[0])) {
      act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if (subcmd == SCMD_POUR) {
      sprintf(buf, "You pour the %s into the %s.\r\n",
          drinks[from_obj->obj_flags.value[2]], arg2);
      send_to_char(buf, ch);
   }

   if (subcmd == SCMD_FILL) {
      act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
      act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
   }

   /* New alias */
   if (to_obj->obj_flags.value[1] == 0)
      name_to_drinkcon(to_obj, from_obj->obj_flags.value[2]);

   /* First same type liq. */
   to_obj->obj_flags.value[2] = from_obj->obj_flags.value[2];

   /* Then how much to pour */
   from_obj->obj_flags.value[1] -= (amount =
       (to_obj->obj_flags.value[0] - to_obj->obj_flags.value[1]));

   to_obj->obj_flags.value[1] = to_obj->obj_flags.value[0];

   if (from_obj->obj_flags.value[1] < 0) {  /* There was too little */
      to_obj->obj_flags.value[1] += from_obj->obj_flags.value[1];
      amount += from_obj->obj_flags.value[1];
      name_from_drinkcon(from_obj);
      from_obj->obj_flags.value[1] = 0;
      from_obj->obj_flags.value[2] = 0;
      from_obj->obj_flags.value[3] = 0;
   }

   /* Then the poison boogie */
   to_obj->obj_flags.value[3] =
      (to_obj->obj_flags.value[3] || from_obj->obj_flags.value[3]);

   /* And the weight boogie */

   weight_change_object(from_obj);
   weight_change_object(to_obj);

   return;
}



void	wear_message(struct char_data *ch, struct obj_data *obj, int where)
{
   char *wear_messages[][2] = {
      { "$n lights $p and holds it.",
	"You light $p and hold it." },

      { "$n slides $p on to $s right ring finger.",
	"You slide $p on to your right ring finger." },

      { "$n slides $p on to $s left ring finger.",
	"You slide $p on to your left ring finger." },

      { "$n wears $p around $s neck.",
        "You wear $p around your neck." },

      { "$n wears $p around $s neck.",
        "You wear $p around your neck." },

      { "$n wears $p on $s body." ,
	"You wear $p on your body.", },

      { "$n wears $p on $s head.",
	"You wear $p on your head." },

      { "$n puts $p on $s legs.",
	"You put $p on your legs." },

      { "$n wears $p on $s feet.",
	"You wear $p on your feet." },

      { "$n puts $p on $s hands.",
	"You put $p on your hands." },

      { "$n wears $p on $s arms.",
	"You wear $p on your arms." },

      { "$n straps $p around $s arm as a shield.",
	"You start to use $p as a shield." },

      { "$n wears $p about $s body." ,
	"You wear $p around your body." },

      { "$n wears $p around $s waist.",
	"You wear $p around your waist." },

      { "$n puts $p on around $s right wrist.",
	"You put $p on around your right wrist." },

      { "$n puts $p on around $s left wrist.",
	"You put $p on around your left wrist." },

      { "$n wields $p.",
        "You wield $p." },

      { "$n grabs $p.",
	"You grab $p." }
   };

   act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM | ACT_GAG);
   act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}



void	perform_wear(struct char_data *ch, struct obj_data *obj, int where)
{
    int class = 0;

    int wear_bitvectors[] = {
	ITEM_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
	ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
	ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
	ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
	ITEM_WIELD, ITEM_HOLD | ITEM_WIELD };

    char *already_wearing[] = {
	"You're already using $p as light source.",
	"YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.(perf_wear 01).",
	"You're already wearing something on both of your ring fingers.",
	"YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.(perf_wear 03).",
	"You can't wear anything more around your neck.",
	"You're already wearing $p on your body.",
	"You're already wearing $p on your head.",
	"You're already wearing $p on your legs.",
	"You're already wearing $p on your feet.",
	"You're already wearing $p on your hands.",
	"You're already wearing $p on your arms.",
	"You're already using $p.",
	"You're already wearing $p about your body.",
	"You already have $p around your waist.",
	"YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.(perf_wear 14).",
	"You're already wearing things around both of your wrists.",
	"You're already wielding $p.",
	"You're already holding $p."
	};

    /* first, make sure that the wear position is valid. */

    if (!CAN_WEAR(obj, wear_bitvectors[where])) {
	act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
	return;
    }

    /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
    if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
	if (ch->equipment[where])
	    where++;

    if (ch->equipment[where]) {
	act(already_wearing[where], FALSE, ch, ch->equipment[where], 0, TO_CHAR);
	return;
    }

    if (CAN_WEAR(obj, ITEM_WIELD_2H)) {
      if (ch->equipment[HOLD] || ch->equipment[WIELD]) {
        act("You need both hands free to use $p.", FALSE, ch, obj, 0, TO_CHAR);
        return;
      }
    }

    if (where == HOLD) {
	 if (CAN_WEAR(obj, ITEM_WIELD_2H)) {
	      act("You can't hold $p, wield it instead.", FALSE, ch, obj, 0, TO_CHAR);
	      return;
	 } else if (ch->equipment[WIELD])
	      if (CAN_WEAR(ch->equipment[WIELD], ITEM_WIELD_2H)) {
		   send_to_char("Both hands are already full.\r\n", ch);
		   return;
	      }
    }

    /* Broken items :(   -Petrus */
    if (IS_OBJ_STAT(obj, ITEM_BROKEN)) {
	send_to_char("No good.  It is broken. :(\r\n", ch);
	return;
    }

    /* Lightsource that has gone out -Petrus*/
    if (where == WEAR_LIGHT && obj->obj_flags.value[2] == 0) {
	send_to_char("You can't make it to give any more light.\r\n", ch);
	act("$n tries to light $p but fails.", TRUE, ch, obj, 0, TO_ROOM);
	return;
    }

    /* New levelbased items  -Petrus */
    if (GET_LEVEL(ch) < GET_ITEM_LEVEL(obj)) {
	send_to_char("You can't use this item yet.\r\n", ch);
	return;
    }


    /* New anti-class check _Petrus */

    if (!IS_NPC(ch)) {
	 if (IS_MULTI(ch) || IS_DUAL(ch)) {
	      SET_BIT(class, (1 << (GET_1CLASS(ch) - 1)));
	      SET_BIT(class, (1 << (GET_2CLASS(ch) - 1)));
	      if (IS_3MULTI(ch))
		   SET_BIT(class, (1 << (GET_3CLASS(ch) - 1)));
	 } else
	      SET_BIT(class, (1 << (GET_CLASS(ch) - 1)));

	 if (((GET_ITEM_ANTICLASS(obj) & class) == class) &&
	     GET_LEVEL(ch) < LEVEL_DEITY) {
	      send_to_char("This item is not for your profession.\r\n", ch);
	      return;
	 }
    }

    wear_message(ch, obj, where);
    obj_from_char(obj);
    equip_char(ch, obj, where);

    /* Moved here from equip_char to prevent capping on
     * autosave and entering game.
     * - Charlene
     */
	GET_HIT(ch)  = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
	GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
        GET_MOVE(ch) = MIN(GET_MOVE(ch), GET_MAX_MOVE(ch));
}



int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg)
{
   int where = -1;

   static char	*keywords[] = {
      "!RESERVED!",
      "finger",
      "!RESERVED!",
      "neck",
      "!RESERVED!",
      "body",
      "head",
      "legs",
      "feet",
      "hands",
      "arms",
      "shield",
      "about",
      "waist",
      "wrist",
      "!RESERVED!",
      "!RESERVED!",
      "!RESERVED!",
      "\n"
   };

   if (!arg || !*arg) {
      if (CAN_WEAR(obj, ITEM_WEAR_FINGER))	where = WEAR_FINGER_R;
      if (CAN_WEAR(obj, ITEM_WEAR_NECK))	where = WEAR_NECK_1;
      if (CAN_WEAR(obj, ITEM_WEAR_BODY))	where = WEAR_BODY;
      if (CAN_WEAR(obj, ITEM_WEAR_HEAD))	where = WEAR_HEAD;
      if (CAN_WEAR(obj, ITEM_WEAR_LEGS))	where = WEAR_LEGS;
      if (CAN_WEAR(obj, ITEM_WEAR_FEET))	where = WEAR_FEET;
      if (CAN_WEAR(obj, ITEM_WEAR_HANDS))	where = WEAR_HANDS;
      if (CAN_WEAR(obj, ITEM_WEAR_ARMS))	where = WEAR_ARMS;
      if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))	where = WEAR_SHIELD;
      if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))	where = WEAR_ABOUT;
      if (CAN_WEAR(obj, ITEM_WEAR_WAIST))	where = WEAR_WAIST;
      if (CAN_WEAR(obj, ITEM_WEAR_WRIST))	where = WEAR_WRIST_R;
   } else {
      if ((where = search_block(arg, keywords, FALSE)) < 0) {
	 sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
	 send_to_char(buf, ch);
      }
   }

   if(where == 0) {
	where = -1;
        sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
        send_to_char(buf, ch);
   }
   return where;
}



ACMD(do_wear)
{
   char	arg1[MAX_INPUT_LENGTH];
   char	arg2[MAX_INPUT_LENGTH];
   struct obj_data *obj, *next_obj;
   int	where, dotmode, items_worn = 0;

   two_arguments(argument, arg1, arg2);

   if (!*arg1) {
      send_to_char("Wear what?\r\n", ch);
      return;
   }

   dotmode = find_all_dots(arg1);

   if (*arg2 && (dotmode != FIND_INDIV)) {
	send_to_char("You can't specify the same body location for more than one item!\r\n", ch);
	return;
   }

   if (dotmode == FIND_ALL) {
	for (obj = ch->carrying; obj; obj = next_obj) {
	     next_obj = obj->next_content;
	     if ((where = find_eq_pos(ch, obj, 0)) >= 0) {
		  items_worn = 1;
		  perform_wear(ch, obj, where);
	     }
	}
	if (!items_worn)
	     send_to_char("You don't seem to have anything wearable.\r\n", ch);
	return;
   }

   if (dotmode == FIND_ALLDOT) {
	if (!*arg1) {
	     send_to_char("Wear all of what?\r\n", ch);
	     return;
	}
	if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	     sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
	 send_to_char(buf, ch);
	} else while (obj) {
	     next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
	     if ((where = find_eq_pos(ch, obj, 0)) >= 0)
		  perform_wear(ch, obj, where);
	     else
		  act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
	     obj = next_obj;
	}
	return;
   }

   if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
	send_to_char(buf, ch);
   } else {
	if ((where = find_eq_pos(ch, obj, arg2)) >= 0)
	     perform_wear(ch, obj, where);
	else if (!*arg2)
	     act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
   }
   return;
}




ACMD(do_wield)
{
   struct obj_data *obj;

   one_argument(argument, arg);

   if (!*arg)
      send_to_char("Wield what?\r\n", ch);
   else if (!(obj = get_obj_in_list(arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
   } else {
      if (!CAN_WEAR(obj, ITEM_WIELD) && !CAN_WEAR(obj, ITEM_WIELD_2H))
	 send_to_char("You can't wield that.\r\n", ch);
      else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	 send_to_char("It's too heavy for you to use.\r\n", ch);
      else
	 perform_wear(ch, obj, WIELD);
   }
}



ACMD(do_grab)
{
   struct obj_data *obj;

   one_argument(argument, arg);

   if (!*arg)
      send_to_char("Hold what?\r\n", ch);
   else if (!(obj = get_obj_in_list(arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
   } else {
      if (GET_ITEM_TYPE(obj) == ITEM_LIGHT)
	 perform_wear(ch, obj, WEAR_LIGHT);
      else if (GET_ITEM_TYPE(obj) == ITEM_KEY) {
        sprintf(buf, "You don't need to hold a key.\r\n");
        send_to_char(buf, ch);

      } else
	 perform_wear(ch, obj, HOLD);
   }
}



void	perform_remove(struct char_data *ch, int pos)
{
   struct obj_data *obj;

   if (!(obj = ch->equipment[pos])) {
      log("Error in perform_remove: bad pos passed.");
      return;
   }

   if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
   else {
      obj_to_char(unequip_char(ch, pos), ch);

      act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM | ACT_GAG);

      /* Added here to complete Ghost Stat Capping -Bod */
      GET_HIT(ch)  = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
      GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
      GET_MOVE(ch) = MIN(GET_MOVE(ch), GET_MAX_MOVE(ch));
   }
}



ACMD(do_remove)
{
   struct obj_data *obj;
   int	i, dotmode, found;

   one_argument(argument, arg);

   if (!*arg) {
      send_to_char("Remove what?\r\n", ch);
      return;
   }

   dotmode = find_all_dots(arg);

   if (dotmode == FIND_ALL) {
      found = 0;
      for (i = 0; i < MAX_WEAR; i++)
         if (ch->equipment[i]) {
            perform_remove(ch, i);
	    found = 1;
	 }
      if (!found)
	 send_to_char("You're not using anything.\r\n", ch);
   } else if (dotmode == FIND_ALLDOT) {
      if (!*arg)
         send_to_char("Remove all of what?\r\n", ch);
      else {
         found = 0;
         for (i = 0; i < MAX_WEAR; i++)
            if (ch->equipment[i] && CAN_SEE_OBJ(ch, ch->equipment[i]) &&
	     isname(arg, ch->equipment[i]->name)) {
	       perform_remove(ch, i);
	       found = 1;
	    }
	 if (!found) {
	    sprintf(buf, "You don't seem to be using any %ss.\r\n", arg);
	    send_to_char(buf, ch);
	 }
      }
   } else {
      if (!(obj = get_object_in_equip_vis(ch, arg, ch->equipment, &i))) {
	 sprintf(buf, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
	 send_to_char(buf, ch);
      } else
	 perform_remove(ch, i);
   }
}

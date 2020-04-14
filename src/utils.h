/* ************************************************************************
*   File: utils.h                                       Part of EliteMUD  *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */



/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);

extern struct weather_data weather_info;

/* various constants *****************************************************/

#define READ_SIZE       256

/* defines for mudlog() */

#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3

#define SECS_PER_REAL_MIN    60
#define SECS_PER_REAL_HOUR  (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY   (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR   75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )


#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

/* memory utils **********************************************************/

#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("realloc failure"); abort(); } } while(0)

/* the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)      \
   if ((item) == (head))                \
      head = (item)->next;              \
   else {                               \
      temp = head;                      \
      while (temp && (temp->next != (item))) \
         temp = temp->next;             \
      if (temp)                         \
         temp->next = (item)->next;     \
   }                                    \


/* basic bitvector utils *************************************************/

#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

#define IS_NPC(ch)  (IS_SET((ch)->specials2.act, MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define AFF_FLAGS(ch) ((ch)->specials.affected_by)
#define AFF_FLAGS2(ch) ((ch)->specials.affected_by2)
#define MOB_FLAGS(ch) ((ch)->specials2.act)
#define PLR_FLAGS(ch) ((ch)->specials2.act)
#define PRF_FLAGS(ch) ((ch)->specials2.pref)

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS(ch), (flag)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

#define ROOM_FLAGS(loc) (world[(loc)]->room_flags)
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)))

#define IN_ROOM(ch)   ((ch)->in_room)
#define IN_VROOM(ch)  (IN_ROOM(ch)>NOWHERE?world[IN_ROOM(ch)]->number:NOWHERE)
#define IN_ZONE(ch)   (world[IN_ROOM(ch)]->zone)

#define IS_AFFECTED(ch,skill) ( IS_SET(AFF_FLAGS(ch), (skill)) )
#define IS_AFFECTED2(ch, skill) (IS_SET(AFF_FLAGS2(ch), (skill)))

/* room utils ************************************************************/

#define SECT(room) (world[room]->sector_type)

#define IS_DARK(room)  ( !world[room]->light && \
                         (IS_SET(world[room]->room_flags, DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))

/* char utils ************************************************************/

#define OCSMODE(ch)   ((ch)->ocsmode)
#define OCS1(ch)      ((ch)->ocs_ptr1)
#define OCS2(ch)      ((ch)->ocs_ptr2)

#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)

#define GET_NAME(ch)    (IS_NPC(ch) ? (ch)->player.short_descr : (ch)->player.name)

#define GET_IDNUM(ch) (IS_NPC(ch) ? -1 : (ch)->specials2.idnum)

#define GET_MOB_WAIT(ch)  ((ch)->mob_specials.wait_state)

#define GET_INVIS_LEV(ch) ((ch)->specials.invis_level)
#define GET_POS(ch)       ((ch)->specials.position)
#define WORSHIPPERS(ch)   ((ch)->specials.worshippers)
#define POWER(ch)         ((ch)->specials.power)
#define GET_WIZNAME(ch)   ((ch)->specials.wizname)
#define GET_PRENAME(ch)   ((ch)->specials.prename)
#define GET_LAST_TELL(ch) ((ch)->specials.last_tell)
#define FIGHTING(ch)	  ((ch)->specials.fighting)
#define HUNTING(ch)	  ((ch)->specials.hunting)
#define MOUNTING(ch)      ((ch)->specials.mounting)
#define PROTECTING(ch)    ((ch)->specials.protecting)


#define GET_COND(ch, i)   ((ch)->specials2.conditions[(i)])
#define GET_LOADROOM(ch)  ((ch)->specials2.load_room)
#define QUEST_NUM(ch)     ((ch)->specials2.quest) 
#define QUEST_FLAGS(ch)   ((ch)->specials2.questflags)
#define CLAN(ch)          ((ch)->specials2.clan)
#define CLAN_LEVEL(ch)    ((ch)->specials2.clanlevel)
#define CLAN_NAME(ch)     (clan_list[CLAN(ch)].name)
#define CLAN_RANKNAME(ch) (clan_list[CLAN(ch)].ranknames[CLAN_LEVEL(ch)][(int)GET_SEX(ch)])     
#define REMORT(ch)        ((ch)->specials2.remorts)
#define GODLEVEL(ch)      ((ch)->specials2.godlevel)
#define WORSHIPS(ch)      ((ch)->specials2.worships)
#define WIMP_LEVEL(ch)    ((ch)->specials2.wimp_level)
#define GET_1LEVEL(ch)    ((ch)->specials2.level1)
#define GET_2LEVEL(ch)    ((ch)->specials2.level2)
#define GET_3LEVEL(ch)    ((ch)->specials2.level3)
#define GET_RACE(ch)      ((ch)->specials2.race)
#define GET_1CLASS(ch)    ((ch)->specials2.class1)
#define GET_2CLASS(ch)    ((ch)->specials2.class2)
#define GET_3CLASS(ch)    ((ch)->specials2.class3)
#define GET_SCRLEN(ch)    ((ch)->specials2.scrlen)
#define GET_ALIGNMENT(ch) ((ch)->specials2.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define GET_PASSWD(ch)       ((ch)->player.passwd)
#define GET_TITLE(ch)        ((ch)->player.title)
#define GET_DESCRIPTION(ch)  ((ch)->player.description)
#define GET_PLAN(ch)         ((ch)->player.plan)
#define GET_LEVEL(ch)        ((ch)->player.level)
#define GET_CLASS(ch)        ((ch)->player.class)
#define GET_HOME(ch)	     ((ch)->player.hometown)
#define GET_HEIGHT(ch)	     ((ch)->player.height)
#define GET_WEIGHT(ch)	     ((ch)->player.weight)
#define GET_SEX(ch)	     ((ch)->player.sex)


#define IS_VAMPIRE(ch)	(GET_RACE(ch) == RACE_VAMPIRE ? 1 : 0)

#define GET_STR(ch)     ((ch)->tmpabilities.str)
#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)
#define GET_DEX(ch)     ((ch)->tmpabilities.dex)
#define GET_INT(ch)     ((ch)->tmpabilities.intel)
#define GET_WIS(ch)     ((ch)->tmpabilities.wis)
#define GET_CON(ch)     ((ch)->tmpabilities.con)
#define GET_CHA(ch)     ((ch)->tmpabilities.cha)

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define GET_AGE(ch)     (age(ch).year)

/* The hit_limit, move_limit, and mana_limit functions are gone.  See
   limits.c for details.
*/

#define GET_HIT(ch)	  ((ch)->points.hit)
#define GET_MAX_HIT(ch)	  ((ch)->points.max_hit)
#define GET_MOVE(ch)	  ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)	  ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_GOLD(ch)	  ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_EXP(ch)	  ((ch)->points.exp)
#define GET_HITROLL(ch)   ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)
#define GET_AC(ch)        ((ch)->points.armor)


/* Wargames player utils */

#define GET_WAR_TEAM(ch)    ((ch)->specials.wargame.team)
#define GET_WAR_AMMO(ch)    ((ch)->specials.wargame.ammo)
#define GET_WAR_HEALTH(ch)  ((ch)->specials.wargame.health)
#define GET_WAR_WEAPON(ch)  ((ch)->specials.wargame.weapon)
#define GET_WAR_AFF(ch)     ((ch)->specials.wargame.affections)
#define AFF_WAR_SANC(ch)    (GET_WAR_AFF%2?1:0)
#define AFF_WAR_DUAL(ch)    ((GET_WAR_AFF(ch)/2)%2?1:0)
#define AFF_WAR_SNEAK(ch)   ((GET_WAR_AFF(ch)/4)%2?1:0)
#define TOG_WAR_SANC(ch) \
        (AFF_WAR_SANC(ch)?GET_WAR_AFF(ch)-=1:GET_WAR_AFF(ch)+=1)
#define TOG_WAR_DUAL(ch) \
        (AFF_WAR_DUAL(ch)?GET_WAR_AFF(ch)-=2:GET_WAR_AFF(ch)+=2)
#define TOG_WAR_SNEAK(ch) \
        (AFF_WAR_SNEAK(ch)?GET_WAR_AFF(ch)-=4:GET_WAR_AFF(ch)+=4)


/* class related macros */

#define IS_MULTI(ch)    (GET_CLASS(ch) >= CLASS_2MULTI)
#define IS_2MULTI(ch)   (GET_CLASS(ch) == CLASS_2MULTI)
#define IS_3MULTI(ch)   (GET_CLASS(ch) == CLASS_3MULTI)
#define IS_DUAL(ch)     (GET_CLASS(ch) == CLASS_DUAL)

#define IS_WARRIOR(ch) (((GET_CLASS(ch)%4==0 && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==0\
  ||   GET_2CLASS(ch)%4==0\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==0)))))

#define IS_THIEF(ch) (((GET_CLASS(ch)%4==CLASS_THIEF && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_THIEF\
  ||   GET_2CLASS(ch)%4==CLASS_THIEF\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_THIEF)))))

#define IS_CLERIC(ch) (((GET_CLASS(ch)%4==CLASS_CLERIC && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_CLERIC\
  ||   GET_2CLASS(ch)%4==CLASS_CLERIC\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_CLERIC)))))

#define IS_MAGIC_USER(ch) (((GET_CLASS(ch)%4==CLASS_MAGIC_USER && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_MAGIC_USER\
  ||   GET_2CLASS(ch)%4==CLASS_MAGIC_USER\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_MAGIC_USER)))))

#define IS_ASSASSIN(ch) (((GET_CLASS(ch)%4==CLASS_ASSASSIN && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_ASSASSIN\
  ||   GET_2CLASS(ch)%4==CLASS_ASSASSIN\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_ASSASSIN)))))

#define IS_BARD(ch) (((GET_CLASS(ch)%4==CLASS_BARD && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_BARD\
  ||   GET_2CLASS(ch)%4==CLASS_BARD\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_BARD)))))

#define IS_NINJA(ch) (((GET_CLASS(ch)%4==CLASS_NINJA && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_NINJA\
  ||   GET_2CLASS(ch)%4==CLASS_NINJA\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_NINJA)))))

#define IS_RANGER(ch) (((GET_CLASS(ch)%4==CLASS_RANGER && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_RANGER\
  ||   GET_2CLASS(ch)%4==CLASS_RANGER\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_RANGER)))))

#define IS_MARINER(ch) (((GET_CLASS(ch)%4==CLASS_MARINER && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_MARINER\
  ||   GET_2CLASS(ch)%4==CLASS_MARINER\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_MARINER)))))

#define IS_CAVALIER(ch) (((GET_CLASS(ch)%4==CLASS_CAVALIER && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_CAVALIER\
  ||   GET_2CLASS(ch)%4==CLASS_CAVALIER\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_CAVALIER)))))

#define IS_KNIGHT(ch) (((GET_CLASS(ch)%4==CLASS_KNIGHT && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_KNIGHT\
  ||   GET_2CLASS(ch)%4==CLASS_KNIGHT\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_KNIGHT)))))

#define IS_MONK(ch) (((GET_CLASS(ch)%4==CLASS_MONK && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_MONK\
  ||   GET_2CLASS(ch)%4==CLASS_MONK\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_MONK)))))

#define IS_DRUID(ch) (((GET_CLASS(ch)%4==CLASS_DRUID && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_DRUID\
  ||   GET_2CLASS(ch)%4==CLASS_DRUID\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_DRUID)))))

#define IS_PALADIN(ch) (((GET_CLASS(ch)%4==CLASS_PALADIN && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_PALADIN\
  ||   GET_2CLASS(ch)%4==CLASS_PALADIN\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_PALADIN)))))

#define IS_WIZARD(ch) (((GET_CLASS(ch)%4==CLASS_WIZARD && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_WIZARD\
  ||   GET_2CLASS(ch)%4==CLASS_WIZARD\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_WIZARD)))))

#define IS_ILLUSIONIST(ch) (((GET_CLASS(ch)%4==CLASS_ILLUSIONIST && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_ILLUSIONIST\
  ||   GET_2CLASS(ch)%4==CLASS_ILLUSIONIST\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_ILLUSIONIST)))))

#define IS_PSIONICIST(ch) (((GET_CLASS(ch)%4==CLASS_PSIONICIST && GET_CLASS(ch)<=20)\
  || ((GET_CLASS(ch)>=CLASS_2MULTI) && (GET_1CLASS(ch)%4==CLASS_PSIONICIST\
  ||   GET_2CLASS(ch)%4==CLASS_PSIONICIST\
  || (GET_CLASS(ch)==CLASS_3MULTI && GET_3CLASS(ch)%4==CLASS_PSIONICIST)))))


#define LOWEST_CLASS(ch)   (IS_MULTI(ch) ? ((IS_3MULTI(ch) && GET_3LEVEL(ch)<GET_2LEVEL(ch)) ? GET_3CLASS(ch):(GET_2LEVEL(ch)<GET_1LEVEL(ch) ? GET_2CLASS(ch):GET_1CLASS(ch))):GET_CLASS(ch))

#define LOWEST_LEVEL(ch)   (IS_MULTI(ch) ? ((IS_3MULTI(ch) && GET_3LEVEL(ch)<GET_2LEVEL(ch)) ? GET_3LEVEL(ch):(GET_2LEVEL(ch)<GET_1LEVEL(ch) ? GET_2LEVEL(ch):GET_1LEVEL(ch))):GET_LEVEL(ch))

/* Skill/spell related macros */
#define SPELLS_TO_LEARN(ch) ((ch)->specials2.spells_to_learn)

#define GET_SKILL(ch, i) ((ch)->skills ? (((ch)->skills)[i]) : ((ch)->mobskills ? get_mob_skill(ch, i) : 0))

#define SET_SKILL(ch, i, pct) { if ((ch)->skills) (ch)->skills[i] = pct; }


/* Object related macros */
#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GET_LEVEL(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define MOB_CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)) && !IS_OBJ_STAT(obj, ITEM_NOSWEEP))


/* descriptor-based utils ************************************************/

#define STATE(d) ((d)->connected)

#define WAIT_STATE(ch, cycle) { \
        if ((ch)->desc) (ch)->desc->wait += (cycle); \
        else if (IS_NPC(ch)) GET_MOB_WAIT(ch) += (cycle); }

#define CHECK_WAIT(ch) (((ch)->desc) ? ((ch)->desc->wait > 1) : 0)

/* object utils **********************************************************/

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_ITEM_TYPE(obj)      ((obj)->obj_flags.type_flag)
#define GET_ITEM_VALUE(obj,i)   ((obj)->obj_flags.value[i])
#define GET_ITEM_LEVEL(obj)     ((obj)->obj_flags.level)
#define GET_ITEM_ANTICLASS(obj) ((obj)->obj_flags.anticlass)
#define GET_ITEM_HP(obj)        ((obj)->obj_flags.value[5])
#define GET_OBJ_WEIGHT(obj)     ((obj)->obj_flags.weight)

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))

#define CLANEQ(obj)      ((obj)->clan_eq)
#define CLANEQ_CLAN(obj) ((obj)->clan_eq->clan)
#define CLANEQ_EX(obj)   ((obj)->clan_eq->exchange)

/* compound utilities and other macros **********************************/

#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")
#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!IS_AFFECTED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || IS_AFFECTED((sub), AFF_INFRARED) \
			     || IS_AFFECTED((sub), AFF_LIGHT)))

#define INVIS_OK(sub, obj) \
 ((!IS_AFFECTED((obj),AFF_INVISIBLE) || IS_AFFECTED(sub,AFF_DETECT_INVIS)) && \
  (!IS_AFFECTED((obj), AFF_HIDE) || IS_AFFECTED(sub, AFF_SENSE_LIFE)))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || PRF_FLAGGED(sub, PRF_HOLYLIGHT))

#define SELF(sub, obj)  ((sub) == (obj))

#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= GET_INVIS_LEV(obj)) && IMM_CAN_SEE(sub, obj)))

/* end of CAN_SEE */ 

#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || IS_AFFECTED((sub), AFF_DETECT_INVIS))

#define MORT_CAN_SEE_OBJ(sub, obj) (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || PRF_FLAGGED((sub), PRF_HOLYLIGHT))


/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room]->room_flags,INDOORS))

#define EXIT(ch, door)  (world[(ch)->in_room]->dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			  (EXIT(ch,door)->to_room != NOWHERE) && \
			  !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

/* OS compatibility ******************************************************/

/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET           0
#define SEEK_CUR           1
#define SEEK_END	   2
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/*
 * NOCRYPT can be defined by an implementor manually in sysdep.h.
 * CIRCLE_CRYPT is a variable that the 'configure' script
 * automatically sets when it determines whether or not the system is
 * capable of encrypting.
 */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

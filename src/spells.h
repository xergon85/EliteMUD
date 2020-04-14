/* ************************************************************************
*   File: spells.h                                      Part of EliteMUD  *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */
#define NUM_OF_SPELLS 111


#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON            7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOUR_SPRAY           10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE       19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL      34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FLY                    45 /* Added by Rickard               */
#define SPELL_INFRAVISION            46 /* Needed to get innate abils     */
#define SPELL_CAT_EYES               47
#define SPELL_ARC_FIRE               48
#define SPELL_FREEZE_FOES            49
#define SPELL_WORD_OF_HEALING        50
#define SPELL_LEVITATION             51
#define SPELL_WARSTRIKE              52
#define SPELL_IDENTIFY               53
#define SPELL_ANIMATE_DEAD           54 
#define SPELL_FEAR                   55 /* EXAMPLE */
#define SPELL_FIRE_BREATH            56
#define SPELL_GAS_BREATH             57
#define SPELL_FROST_BREATH           58
#define SPELL_ACID_BREATH            59
#define SPELL_LIGHTNING_BREATH       60
#define SPELL_REGENERATION           61
#define SPELL_FLESH_RESTORE          62
#define SPELL_SHOCKING_SPHERE        63
#define SPELL_FLESH_ANEW             64
#define SPELL_INSTANT_WOLF           65
#define SPELL_INSTANT_SLAYER         66
#define SPELL_VORPAL_PLATING         67
#define SPELL_QUICK_FIX              68
#define SPELL_HOLY_WATER             69
#define SPELL_MAGE_GAUNTLETS         70
#define SPELL_MYSTIC_SHIELD          71
#define SPELL_STAR_FLARE             72
#define SPELL_SPECTRE_TOUCH          73
#define SPELL_PHASE_DOOR             74
#define SPELL_MYSTICAL_COAT          75
#define SPELL_DEATH_STRIKE           76
#define SPELL_ICE_STORM              77
#define SPELL_STONE_TO_FLESH         78
#define SPELL_MIND_JAB               79
#define SPELL_PHASE_BLUR             80
#define SPELL_TRUE_SIGHT             81
#define SPELL_WIND_WARRIOR           82
#define SPELL_WIND_OGRE              83
#define SPELL_WIND_DRAGON            84
#define SPELL_DISRUPT_ILLUSION       85
#define SPELL_MIND_BLADE             86
#define SPELL_RIMEFANG               87
#define SPELL_MAELSTROM              88
#define SPELL_ELEMENTAL_SUMMONING    89
#define SPELL_GRAVITY_FOCUS          90
#define SPELL_GROUP_HEAL             91
#define SPELL_GROUP_RECALL           92                   
#define SPELL_IMPROVED_BLESS         93
#define SPELL_PHASE_KNIFE            94
#define SPELL_FLAME_RAY              95
#define SPELL_PSYCHIC_SCREAM         96
#define SPELL_PROJECT_FORCE          97
#define SPELL_DETONATE               98
#define SPELL_PHASEFIRE              99
#define SPELL_DRAIN_LIFE            100
#define SPELL_SHADOW_KNIFE          101
#define SPELL_DISINTEGRATE          102
#define SPELL_DRAGON_BREATH         103
#define SPELL_TERRORWEAVE           104
#define SPELL_ELEMENTAL_CANNON      105
#define SPELL_UNLEASH_MIND          106
#define SPELL_PHANTASMAL_KILLER     107
#define SPELL_WATER_BREATHING       108
#define SPELL_CLAN_RECALL           109
#define SPELL_HOLY_WRATH            110
#define SPELL_LOCATE_PERSON         111

/* Incomplete Spells Below
#define SPELL_PRIME_SUMMONING        
#define SPELL_DEMON_BANE             
#define SPELL_FLAME_COLUMN           
#define SPELL_DISPOSSESS             
#define SPELL_SPELL_BIND             
#define SPELL_SOUL_WHIP              
#define SPELL_GREATER_SUMMONING     
#define SPELL_BEYOND_DEATH          
#define SPELL_WIZARD_BLAST          
#define SPELL_DEMON_STRIKE          
#define SPELL_BLUNDER               
#define SPELL_BATCH_SPELL           
#define SPELL_NIGHT_LANCE           
#define SPELL_IOS_MALLET            
#define SPELL_VITALITY              
#define SPELL_FATAL_FIST            
#define SPELL_FROST_FORCE           
#define SPELL_GOD_FIRE              
#define SPELL_ELECTRIC_STUN         
#define SPELL_LUCK_CHANT            
#define SPELL_FAR_DEATH             
#define SPELL_OIL_OF_OLAY           
#define SPELL_GRAVE_ROBBER          
#define SPELL_SHADOW_SHIELD         
#define SPELL_WITHER                
#define SPELL_EARTH_DAGGER          
#define SPELL_TREBUCHET             
#define SPELL_EARTH_ELEMENTAL       
#define SPELL_PETRIFY               
#define SPELL_DESERT_STRIKE         
#define SPELL_GLACIER_STRIKE        
#define SPELL_MAGMA_BLAST           
#define SPELL_JOLT_BOLT             
#define SPELL_EARTH_MAW             
#define SPELL_ATEAS_GILLS         
#define SPELL_DIVINE_INTERVENTION   
#define SPELL_GOTTERDAMURUNG        
*/

#define SPELL_FROM_ITEM             298  /* A bit set by wearing obj */

/* MAXIMUM SPELL NUMBER 299 */
#define SKILL_START                 300

#define SKILL_STAB                  300
#define SKILL_BLUDGEON              301
#define SKILL_SLASH                 302
#define SKILL_CHOP                  303
#define SKILL_2ATTACK               304
#define SKILL_COOK                  305
#define SKILL_SPELLCRAFT            306
#define SKILL_FLAY                  307
#define SKILL_SWIM                  308
#define SKILL_DIVE                  309
#define SKILL_CLIMB                 310
#define SKILL_FIRST_AID             311 
#define SKILL_SNEAK                 312
#define SKILL_HIDE                  313
#define SKILL_STEAL                 314
#define SKILL_BACKSTAB              315
#define SKILL_PICK_LOCK             316
#define SKILL_DUAL                  317
#define SKILL_DODGE                 318
#define SKILL_THROW                 319
#define SKILL_DISARM_TRAP           320
#define SKILL_MARTIAL_ARTS          321
#define SKILL_CRITICAL_HIT          322
#define SKILL_KICK                  323 
#define SKILL_BASH                  324 
#define SKILL_RESCUE                325  
#define SKILL_3ATTACK               326
#define SKILL_4ATTACK               327
#define SKILL_PARRY                 328
#define SKILL_BERZERK               329
#define SKILL_TWO_HANDED            330
#define SKILL_POISON_BLADE          331
#define SKILL_BATTLE_TACTICS        332
#define SKILL_TRACK                 333
#define SKILL_HUNT                  334
#define SKILL_ARCHERY               335
#define SKILL_SAIL                  336
#define SKILL_APPRAISE              337
#define SKILL_TUMBLE                338
#define SKILL_BLINDFIGHT            339
#define SKILL_DISBELIEVE            340
#define SKILL_JOUST                 341
#define SKILL_FEIGN_DEATH           342
#define SKILL_RIDING_LAND           343
#define SKILL_RIDING_AIR            344
#define SKILL_DISARM                345
#define SKILL_FISH                  346
#define SKILL_FORGERY               347
#define SKILL_STONEMASONRY          348
#define SKILL_HERBALISM             349
#define SKILL_DISQUISE              350
#define SKILL_JUMP                  351
#define SKILL_VENTRILOQUISM         352
#define SKILL_INTIMIDATE            353
#define SKILL_ANIMAL_HANDLING       354
#define SKILL_SET_SNARE             355
#define SKILL_HEROIC_RESCUE         356
#define SKILL_TAUNT                 357
#define SKILL_ESCAPE                358
#define SKILL_SCOUT                 359
#define SKILL_SPY                   360
#define SKILL_SEARCH                361
#define SKILL_TRIP                  362
#define SKILL_UNFAIR_FIGHT          363
#define SKILL_HEADBANG              364
#define SKILL_EXTRA_DAMAGE          365
#define SKILL_VITALIZE_MANA         366
#define SKILL_MEDITATE              367
#define SKILL_TRADING               368
#define SKILL_MOUNTED_BATTLE        369
#define SKILL_PATH_FIND             370
#define SKILL_NAVIGATE              371
#define SKILL_CIRCLE_AROUND         372
#define SKILL_PUGILISM              373
#define SKILL_VITALIZE_STAMINA      374
#define SKILL_PIERCE                375
#define SKILL_FIRST_TO_ATTACK       376
#define SKILL_TAIL_LASH             377
#define SKILL_HORN_BUTT             378
#define SKILL_BITE                  379
#define SKILL_KVACK_FU              380
#define SKILL_MEND_ARMOR            381
#define SKILL_REPAIR_WEAPON         382
#define SKILL_DET_TRAP              383
#define SKILL_CLAW                  384
#define SKILL_POUNCE                385
#define SKILL_NINJITSU              386
#define SKILL_ADV_MARTIAL_ARTS      387
/* MAXIMUM SKILL NUMBER 399 */

#define TYPE_START                   400

#define TYPE_HIT                     400
#define TYPE_BLUDGEON                401
#define TYPE_PIERCE                  402
#define TYPE_SLASH                   403
#define TYPE_BLAST		     404
#define TYPE_WHIP                    405  /* EXAMPLE */
#define TYPE_NO_BS_PIERCE	     406
#define TYPE_CLAW                    407
#define TYPE_BITE                    408
#define TYPE_STING                   409
#define TYPE_CRUSH                   410
#define TYPE_AMA                     411


#define TYPE_TRAP                    495
#define TYPE_DAMAGE                  496
#define TYPE_SUFFOCATE               497
#define TYPE_FALLING                 498
#define TYPE_SUFFERING               499
/* More anything but spells and weapontypes can be insterted here! */

#define DAMTYPE_START                500
#define DAMTYPE_NORMAL               500



#define MAX_TYPES 70

#define SAVING_PHYSICAL    1000
#define SAVING_MENTAL      1001
#define SAVING_MAGIC       1002
#define SAVING_POISON      1003
#define MAGIC_RESISTANCE   1004

/* Saving throw types - Helm */
#define SAVE_NONE     0
#define SAVE_HALF     1
#define SAVE_NEGATE   2
#define SAVE_SPECIAL  3


#define MAX_SPL_LIST	300


#define CAST_OFFENSIVE    1
#define CAST_AFF_SELF     2
#define CAST_AFF_VICT     3
#define CAST_HEAL         4
#define CAST_FLEE         5
#define CAST_AFF_OWNWPN   6
#define CAST_AFF_VICTWPN  7
#define CAST_CURE_SELF    8


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024


#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)


struct spell_info_type {
    int    type;              /* What type of spell is this? */
    byte   minimum_position;  /* Min position for caster */
    ubyte  min_usesmana;      /* Amount of mana used by a spell  */
    byte   beats;             /* Heartbeats until ready for next */
    byte   offensive;         /* Is the spell offensive is some way? */
    sh_int targets;           /* See below for use with TAR_XXX  */
    char   *wearoffmess;      /* The message to display when worn off */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
   char	*singular;
   char	*plural;
};


#define ASPELL(spellname) \
void	spellname(byte level, struct char_data *ch, char *arg,\
		  struct char_data *victim, struct obj_data *obj)

#define ACAST(castname) \
void	castname(byte level, struct char_data *ch, char *arg, int type, \
		 struct char_data *victim, struct obj_data *tar_obj)

void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		int spellnum, int casttype);

void mag_areas(byte level, struct char_data * ch, int spellnum, int savetype);

void mag_points(int level, struct char_data * ch, struct char_data * victim,
		int spellnum, int casttype);

void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		 int spellnum, int casttype);

void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		   int spellnum, int type);

void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		 int spellnum, int savetype);

ASPELL(spell_teleport);
ASPELL(spell_clone);
ASPELL(spell_control_weather);
ASPELL(spell_create_water);
ASPELL(spell_enchant_weapon);
ASPELL(spell_locate_object);
ASPELL(spell_locate_person);
ASPELL(spell_ventriloquate);
ASPELL(spell_word_of_recall);
ASPELL(spell_clan_recall);
ASPELL(spell_summon);
ASPELL(spell_charm_person);
ASPELL(spell_identify);


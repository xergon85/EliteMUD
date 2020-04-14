/* ************************************************************************
*   File: structs.h                                     Part of EliteMUD  *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  (C) 1998 Petya Vachranukunkiet                                         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
************************************************************************ */

#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/types.h>

#define NOWHERE    -1    /* nil reference for room-database    */
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/

#define SPECIAL(c) int (c)(struct char_data *ch, struct char_data *mob, struct obj_data *obj, int cmd, char *arg)


/* room-related defines *************************************************/

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_UNDERWATER      8   /* New - Petrus */
#define SECT_AIR             9
#define SECT_VOID           10
#define SECT_DESERT         11   /* New - Move cost + water drain */
#define SECT_F_WASTE        12
#define SECT_F_MOUNTAIN     13
#define SECT_ICY_UNDERWATER 14
#define SECT_F_WATER_NOSWIM 15

#define SECT_MAX            16

/* Bitvector For 'room_flags' */

#define DARK           (1 << 0)  /* a - Can't see wo/light */
#define DEATH          (1 << 1)  /* b - Death Trap */
#define NO_MOB         (1 << 2)  /* c - No mob chooses to enter */
#define INDOORS        (1 << 3)  /* d - No outdoor msg effects */
#define LAWFULL        (1 << 4)  /* e - No steal/agressing/summon */
#define NEUTRAL        (1 << 5)  /* f - !yet: Undefined */
#define CHAOTIC        (1 << 6)  /* g - Random exit reguardless of leaving */
#define NO_MAGIC       (1 << 7)  /* h - No spells castable */
#define TUNNEL         (1 << 8)  /* i - !yet: One person in at a time */
#define PRIVATE        (1 << 9)  /* j - If 2+ppl in, no scry/entry */
#define GODROOM        (1 << 10) /* k - No mortal entry wo/transing,
				        No track thru */
#define BFS_MARK       (1 << 11) /* l - Breath For sec mark (track) */
#define ZERO_MANA      (1 << 12) /* m - Removes all mana */
#define DISPELL        (1 << 13) /* n - Unaffects noninnate player affects */
#define SILENT         (1 << 14) /* o - Only qs, say, wizline comm
                                        works in here */
#define IN_AIR         (1 << 15) /* p - In air movement only (fly) */
#define OCS            (1 << 16) /* q - Changed in OCS mark */
#define PKOK           (1 << 17) /* r - PK allowed in the room
                                        wo/KILLER Flag */
#define ARENA          (1 << 18) /* s - Wargames Rooms */
#define REGEN          (1 << 19) /* t - Faster HP growth */
#define NO_TELEPORT    (1 << 20) /* u - No random teleporting into */
#define NO_SCRY        (1 << 21) /* v - !yet Can't see in by spell/skill */
#define NO_FLEE        (1 << 22) /* w - !yet:Can't wimp or flee
                                        in here while fight*/
#define DAMAGE         (1 << 23) /* x - Lose hitpoints while in room */
#define NO_TRACK       (1 << 24) /* y - Can't track into/thru room */
#define NO_SWEEP       (1 << 25) /* z - Zone sweeper/zpurge skips this room */
#define NO_SCOUT       (1 << 26) /* A - Can't scout in this room */
#define NO_SLEEP       (1 << 27) /* B - Can't sleep in this room */
#define NO_SUMMON      (1 << 28) /* C - Can't summon into/out of this room */
#define NO_QUIT        (1 << 29) /* D - Can't quit or idle-save in this room */
#define NO_DROP        (1 << 30) /* E - Can't drop items */

#define MAX_ROOM_FLAGS 31

/* For Exit info */

#define EX_ISDOOR      (1 <<  0) /* a - Door */
#define EX_CLOSED      (1 <<  1) /* b - Door is closed */
#define EX_LOCKED      (1 <<  2) /* c - Door is locked */
#define EX_RSCLOSED    (1 <<  3) /* d - Reset closed */
#define EX_RSLOCKED    (1 <<  4) /* e - Reset locked */
#define EX_PICKPROOF   (1 <<  5) /* f - Can't picklock */
#define EX_TRAP        (1 <<  6) /* g - Trap on exit, chance of dam */
#define EX_WALL        (1 <<  7) /* h - Exit is a wall */
#define EX_BASHPROOF   (1 <<  8) /* i - Cannot be bashed open */
#define EX_MAGICPROOF  (1 <<  9) /* j - Cannot be magiced open */
#define EX_PASSPROOF   (1 << 10) /* k - Cannot be passed thru when open */
#define EX_TRAPSET     (1 << 11) /* l - Trap is set for damage */
#define EX_SECRET      (1 << 12) /* m - Exit keyword is secret */
#define EX_BROKEN      (1 << 13) /* n - Door is broken, usually after being bashed */

#define MAX_EXIT_INFO 14

/* char and mob-related defines *****************************************/

/* Bitvector for 'affected_by' */
#define AFF_BLIND             (1 << 0)   /* a */
#define AFF_INVISIBLE         (1 << 1)   /* b */
#define AFF_DETECT_ALIGN      (1 << 2)   /* c */
#define AFF_DETECT_INVIS      (1 << 3)   /* d */
#define AFF_DETECT_MAGIC      (1 << 4)   /* e */
#define AFF_SENSE_LIFE        (1 << 5)   /* f */
#define AFF_HOLD              (1 << 6)   /* g - !USED */
#define AFF_SANCTUARY         (1 << 7)   /* h */
#define AFF_GROUP             (1 << 8)   /* i */
#define AFF_CURSE             (1 << 9)   /* j */
#define AFF_LIGHT             (1 << 10)  /* k */
#define AFF_POISON            (1 << 11)  /* l */
#define AFF_PROTECT_EVIL      (1 << 12)  /* m */
#define AFF_PARALYSIS         (1 << 13)  /* n - !yet */
#define AFF_INSANITY          (1 << 14)  /* o - !USED */
#define AFF_FL_WPN            (1 << 15)  /* p - !USED */
#define AFF_SLEEP             (1 << 16)  /* q */
#define AFF_DODGE             (1 << 17)  /* r - !USED */
#define AFF_SNEAK             (1 << 18)  /* s */
#define AFF_HIDE              (1 << 19)  /* t */
#define AFF_FEAR              (1 << 20)  /* u - !USED */
#define AFF_CHARM             (1 << 21)  /* v */
#define AFF_FEIGN_DEATH       (1 << 22)  /* w - !USED */
#define AFF_DISQUISE	      (1 << 23)  /* x - !USED */
#define AFF_INFRARED          (1 << 24)  /* y */
#define AFF_BERZERK           (1 << 25)  /* z */
#define AFF_HOVER             (1 << 26)  /* A */
#define AFF_FLY               (1 << 27)  /* B */
#define AFF_BREATH_WATER      (1 << 28)  /* C */
#define AFF_REGENERATION      (1 << 29)  /* D */
#define AFF_CHAOS             (1 << 30)  /* E - !USED */

#define AFF_MAX   31

/* Bitvector for affected_by2 */
#define AFF2_TEST             (1 << 0)   /* a */

#define AFF2_MAX  1


/* Bitvector for godlevel */
#define IMM_BASIC       (1 << 0)     /* Basic Immortal Powers */
#define IMM_STD         (1 << 1)     /* Standard Powers (Active) */
#define IMM_ADMIN       (1 << 2)     /* Admin level Powers */
#define IMM_SHUTDOWN    (1 << 3)     /* Reboot */
#define IMM_REMORT      (1 << 4)     /* Remort powers */
#define IMM_QUEST       (1 << 5)     /* Set quest points, load quest eq */
#define IMM_LOAD        (1 << 6)     /* Load non quest objs */
#define IMM_CLAN        (1 << 7)     /* Set Clan */
#define IMM_WORLD       (1 << 8)     /* Set world: [r/m/o]list */
#define IMM_WARGAME     (1 << 9)     /* Start/Run/End the Wargame */
#define IMM_QUESTOR     (1 << 10)    /* Running Quests */
#define IMM_OVERSEER    (1 << 11)    /* Overseer levle Powers*/
#define IMM_CODE        (1 << 12)    /* Code Head powers */
#define IMM_HELP        (1 << 13)    /* Tedit: Help File Access */
#define IMM_NEWS        (1 << 14)    /* Tedit: News File Access */
#define IMM_PK          (1 << 15)    /* PK Mud-wide System control */
#define IMM_CODER       (1 << 16)    /* Coder powers */
#define IMM_18          (1 << 17)    /* */
#define IMM_19          (1 << 18)    /* */
#define IMM_20          (1 << 19)    /* */
#define IMM_21          (1 << 20)    /* */
#define IMM_22          (1 << 21)    /* */
#define IMM_23          (1 << 22)    /* */
#define IMM_24          (1 << 23)    /* */
#define IMM_25          (1 << 24)    /* */
#define IMM_26          (1 << 25)    /* */
#define IMM_27          (1 << 26)    /* */
#define IMM_28          (1 << 27)    /* */
#define IMM_29          (1 << 28)    /* */
#define IMM_30          (1 << 29)    /* */
#define IMM_ALL         (1 << 30)    /* RESERVED */

/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_CHA               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PHYSICAL  20
#define APPLY_SAVING_MENTAL    21
#define APPLY_SAVING_MAGIC     22
#define APPLY_SAVING_POISON    23
#define APPLY_MAGIC_RESISTANCE 24
#define APPLY_JUMP             25
#define APPLY_STEAL            26
#define APPLY_SNEAK            27
#define APPLY_TRACK            28
#define APPLY_ARCHERY          29
#define APPLY_THROW            30
#define APPLY_SWIM             31
#define APPLY_DIVE             32
#define APPLY_BV2              33


/* 'class' for PC's  Changed by Petrus  Don't touch if you don't know what*/
/* Classes organized in groups of four, by archetype */
#define CLASS_MAGIC_USER   1  /* Magic User type */
#define CLASS_CLERIC       2  /* Cleric type     */
#define CLASS_THIEF        3  /* Thief type      */
#define CLASS_WARRIOR      4  /* Warrior type    */
#define CLASS_PSIONICIST   5
#define CLASS_MONK         6
#define CLASS_BARD         7
#define CLASS_KNIGHT       8
#define CLASS_WIZARD       9
#define CLASS_DRUID       10
#define CLASS_ASSASSIN    11
#define CLASS_RANGER      12
#define CLASS_ILLUSIONIST 13
#define CLASS_PALADIN     14
#define CLASS_MARINER     15
#define CLASS_CAVALIER    16
#define CLASS_NONEMU      17
#define CLASS_NONECL      18
#define CLASS_NINJA       19
#define CLASS_NONEWA      20
#define CLASS_DUAL        21
#define CLASS_2MULTI      22
#define CLASS_3MULTI      23
#define CLASS_MAX         24

/* THESE ARE USED ON SOME PLACES - DO NOT CONFUSE THIS WITH THOSE ABOVE -PETRUS */

#define MU_CLASS          (1 << 0)
#define CL_CLASS          (1 << 1)
#define TH_CLASS          (1 << 2)
#define WA_CLASS          (1 << 3)
#define NE_CLASS          (1 << 4)
#define MO_CLASS          (1 << 5)
#define BA_CLASS          (1 << 6)
#define KN_CLASS          (1 << 7)
#define WI_CLASS          (1 << 8)
#define DR_CLASS          (1 << 9)
#define AS_CLASS          (1 << 10)
#define RA_CLASS          (1 << 11)
#define IL_CLASS          (1 << 12)
#define PA_CLASS          (1 << 13)
#define MA_CLASS          (1 << 14)
#define CA_CLASS          (1 << 15)
#define N1_CLASS          (1 << 16)
#define N2_CLASS          (1 << 17)
#define NI_CLASS          (1 << 18)
#define N3_CLASS          (1 << 19)
#define DU_CLASS          (1 << 20)
#define M2_CLASS          (1 << 21)
#define M3_CLASS          (1 << 22)

#define  MAX_ACLASS_FLAGS  23


/* 'races' for PC's  -Petrus */
#define RACE_GOD          0
#define RACE_HUMAN        1
#define RACE_ELF          2
#define RACE_HALFELF      3
#define RACE_DWARF        4
#define RACE_GNOME        5
#define RACE_HALFLING     6
#define RACE_HALFTROLL    7
#define RACE_HALFORC      8
#define RACE_HALFOGRE     9
#define RACE_DUCK        10
#define RACE_FAIRY       11
#define RACE_MINOTAUR    12
#define RACE_RATMAN      13
#define RACE_DROW        14
#define RACE_LIZARDMAN   15
#define RACE_VAMPIRE     16
#define RACE_TROLL       17
#define RACE_DRACONIAN   18
#define RACE_AVATAR      19
#define RACE_WEREWOLF    20
#define RACE_DEMON       21
#define RACE_DRAGON      22
#define RACE_FELINE      23
#define RACE_ANGEL       24

#define RACE_MAX         25


/* 'race' for NPC's */
#define MOB_UNDEFINED    0 /*hit*/
#define MOB_HUMANOID     1 /*hit*/
#define MOB_UNDEAD       2 /*hit*/
#define MOB_CATBEAST     3 /*bite*/
#define MOB_HOUND        4 /*bite*/
#define MOB_BEARBEAST    5 /*claw*/
#define MOB_BIRD         6 /*bite*/
#define MOB_MOUNT        7 /*bite*/
#define MOB_GIANT        8 /*crush*/
#define MOB_DWARF        9 /*hit*/
#define MOB_ILLUSION    10 /*hit*/
#define MOB_MOUNT_FLY   11 /*bite*/
#define MOB_DEMON       12 /*hit*/
#define MOB_FLYBEAST    13 /*bite*/
#define MOB_FIRE        14 /*hit*/
#define MOB_WATER       15 /*hit*/
#define MOB_EARTH       16 /*hit*/
#define MOB_AIR         17 /*hit*/
#define MOB_DRAGON      18 /*whip*/
#define MOB_INSECT      19 /*sting*/

#define MOB_RACES       20

/* 'class' for NPC's */
#define MOB_CLASS_NORMAL       0
#define MOB_CLASS_MAGIC_USER   1
#define MOB_CLASS_CLERIC       2
#define MOB_CLASS_THIEF        3
#define MOB_CLASS_WARRIOR      4

#define MOB_CLASSES            5

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POS_DEAD       0
#define POS_MORTALLYW  1
#define POS_INCAP      2
#define POS_STUNNED    3
#define POS_SLEEPING   4
#define POS_RESTING    5
#define POS_SITTING    6
#define POS_FIGHTING   7
#define POS_STANDING   8

#define POS_SWIMMING  10   /* new - Petrus */
#define POS_DIVING    11
#define POS_HOOVERING 12
#define POS_FLYING    13


/* for mobile actions: specials.act */
#define MOB_SPEC         (1 << 0)  /* a - spec-proc to be called if exist    */
#define MOB_SENTINEL     (1 << 1)  /* b - this mobile not to be moved        */
#define MOB_SCAVENGER    (1 << 2)  /* c - pick up stuff lying around         */
#define MOB_ISNPC        (1 << 3)  /* d - for use with IS_NPC()              */
#define MOB_NICE_THIEF   (1 << 4)  /* e - Set if thief should NOT be killed  */
#define MOB_AGGRESSIVE   (1 << 5)  /* f - Set if automatic attack on NPC's   */
#define MOB_STAY_ZONE    (1 << 6)  /* g - MOB Must stay inside its own zone  */
#define MOB_WIMPY        (1 << 7)  /* h - MOB Will flee when injured, and if */
                                  /* aggressive only attack sleeping players */
/*
 * For MOB_AGGRESSIVE_XXX, you must also set MOB_AGGRESSIVE.
 * These switches can be combined, if none are selected, then
 * the mobile will attack any alignment (same as if all 3 were set)
 */
#define MOB_AGGRESSIVE_EVIL    (1 << 8) /* i - auto attack evil PC's only  */
#define MOB_AGGRESSIVE_GOOD    (1 << 9) /* j - auto attack good PC's only  */
#define MOB_AGGRESSIVE_NEUTRAL (1 << 10)/* k auto attack neutral PC's only */
#define MOB_MEMORY	       (1 << 11)/* l remembers if struck first     */
#define MOB_HELPER	       (1 << 12)/* m - mob helper */
#define MOB_SWITCHED           (1 << 13)/* n - mob switched */
#define MOB_BLINDER            (1 << 14)/* o - mob blinds PC if examined */
#define MOB_HIDDEN             (1 << 15)/* p - mob doesn't display on look room */
#define MOB_NOTRACK            (1 << 16)/* q - mob can't be tracked */


/* For players : specials.act */
#define PLR_KILLER         (1 << 0)
#define PLR_THIEF          (1 << 1)
#define PLR_FROZEN	   (1 << 2)
#define PLR_DONTSET        (1 << 3)   /* Dont EVER set (ISNPC bit) */
#define PLR_WRITING	   (1 << 4)
#define PLR_MAILING	   (1 << 5)
#define PLR_SAVEOBJS	   (1 << 6)
#define PLR_SITEOK	   (1 << 7)
#define PLR_MUTE	   (1 << 8)
#define PLR_NOTITLE	   (1 << 9)
#define PLR_DELETED	   (1 << 10)
#define PLR_LOADROOM	   (1 << 11)
#define PLR_NOWIZLIST	   (1 << 12)
#define PLR_NODELETE	   (1 << 13)
#define PLR_INVSTART	   (1 << 14)
#define PLR_CRYO           (1 << 15)
#define PLR_ARENA          (1 << 16)
#define PLR_NOCLANTITLE    (1 << 17)
#define PLR_NOCLANTELL     (1 << 18)
#define PLR_LINEWRAP       (1 << 19)
#define PLR_LOG            (1 << 20)
#define PLR_SAVECHR        (1 << 21)
#define PLR_SAVEALS        (1 << 22)
#define PLR_SAVESTR        (1 << 23)
#define PLR_AUTOASSIST	   (1 << 24)
#define PLR_NOWHO          (1 << 25)
#define PLR_TEST           (1 << 26)
#define PLR_DETAIL         (1 << 27)
#define PLR_PKOK           (1 << 28)
#define PLR_SAVEIGN        (1 << 29) /* Save the ignore list*/
#define PLR_NOPKSAY        (1 << 30) /* Deaf to PKOK channel */

#define MAX_PLR_FLAGS  31

/* for players: preference bits */
#define PRF_BRIEF          (1 << 0)  /* Brief room desc */
#define PRF_COMPACT        (1 << 1)  /* */
#define PRF_NONEWBIE	   (1 << 2)  /* Deaf to newbie (chan) */
#define PRF_NOTELL         (1 << 3)  /* Deaf to tells (chan) */
#define PRF_DISPHP	   (1 << 4)  /* */
#define PRF_DISPMANA	   (1 << 5)  /* */
#define PRF_DISPMOVE	   (1 << 6)  /* */
#define PRF_DISPVT	   (1 << 7)  /* */
#define PRF_NOHASSLE	   (1 << 8)  /* Imm Flag - Not aggressed by mob*/
#define PRF_QUEST	   (1 << 9)  /* Quest chan on (Chan) */
#define PRF_SUMMONABLE     (1 << 10) /* Able to be summoned thru spell */
#define PRF_NOREPEAT	   (1 << 11) /* */
#define PRF_HOLYLIGHT	   (1 << 12) /* Imm Flag - See in dark & Invis */
#define PRF_COLOR_1	   (1 << 13) /* Color mode 1 */
#define PRF_COLOR_2	   (1 << 14) /* Color mode 2 */
#define PRF_NOWIZ	   (1 << 15) /* Imm Chan - Wizline off (chan) */
#define PRF_LOG1	   (1 << 16) /* */
#define PRF_LOG2	   (1 << 17) /* */
#define PRF_NOAUCT	   (1 << 18) /* Deaf to auction (chan) */
#define PRF_NOGOSS	   (1 << 19) /* Deaf to gossip (chan) */
#define PRF_NOCHAT 	   (1 << 20) /* Deaf to chat (chan) */
#define PRF_ROOMFLAGS	   (1 << 21) /* Imm Flag - See the room flags */
#define PRF_DISPVIC        (1 << 22) /* */
#define PRF_DISPANSI       (1 << 23) /* */
#define PRF_NOSPDWLK       (1 << 24) /* Speed walk option off */
#define PRF_NOALIAS        (1 << 25) /* */
#define PRF_VERBATIM       (1 << 26) /* Verbatim entry only */
#define PRF_NOEXITS        (1 << 27) /* No exit directions displayed */
#define PRF_GAG            (1 << 28) /* No extraneous combat spam */
#define PRF_IBM_PC         (1 << 29) /* */
#define PRF_NOGRAT         (1 << 30) /* Deaf to Gratz (chan) */

#define MAX_PRF  31

/* modes of connectedness */

#define CON_PLYNG    0      /* Playing - Nominal state      */
#define CON_NME      1      /* By what name ..?             */
#define CON_NMECNF   2      /* Did I get that right, x?     */
#define CON_PWDNRM   3      /* Password:                    */
#define CON_PWDGET   4      /* Give me a password for x     */
#define CON_PWDCNF   5      /* Please retype password:      */
#define CON_QSEX     6      /* Sex?                         */
#define CON_RMOTD    7      /* PRESS RETURN after MOTD      */
#define CON_SLCT     8      /* Your choice: (main menu)     */
#define CON_EXDSCR   9      /* Enter a new description:     */
#define CON_QCLASS   10     /* Class?                       */
#define CON_LDEAD    11     /* !USED                        */
#define CON_PWDNQO   12     /* Changing passwd: get old     */
#define CON_PWDNEW   13     /* Changing passwd: get new     */ 
#define CON_PWDNCNF  14     /* Verify new password          */
#define CON_CLOSE    15     /* Disconnecting                */
#define CON_DELCNF1  16     /* Delete confirmation 1        */
#define CON_DELCNF2  17     /* Delete confirmation 2        */
#define CON_QRACE    18     /* Race?                        */
#define CON_DISPL    19     /* Screen mode: (display types) */
#define CON_DEADWAIT 20     /* Dead, waiting to resurrect   */
#define CON_TEXTED   21     /* In tedit                     */

/* ident state */
#define ID_NONE     0
#define ID_CONING   1
#define ID_CONED    2
#define ID_READING  3
#define ID_READ     4
#define ID_COMPLETE 5

/* OCS modes */

#define OCS_OFF               0
#define OCS_ROOM_PRINT        1
#define OCS_ROOM_MAIN         2
#define OCS_ROOM_GET_NAME     3
#define OCS_ROOM_GET_SECT     4
#define OCS_ROOM_GET_FLAGS    5
#define OCS_ROOM_GET_DESC     6
#define OCS_ROOM_GET_EMAIN    7
#define OCS_ROOM_GET_KEYWRD   8
#define OCS_ROOM_GET_EDESC    9
#define OCS_EXIT_PRINT       10
#define OCS_EXIT_MAIN        11
#define OCS_EXIT_GET_DESC    12
#define OCS_EXIT_GET_KEYWRD  13
#define OCS_EXIT_GET_INFO    14
#define OCS_EXIT_GET_KEY     15
#define OCS_EXIT_GET_TO      16
#define OCS_OBJ_PRINT        17
#define OCS_OBJ_MAIN         18
#define OCS_OBJ_GET_NAME     19
#define OCS_OBJ_GET_RDESC    20
#define OCS_OBJ_GET_IDESC    21
#define OCS_OBJ_GET_ADESC    22
#define OCS_MOB_PRINT        23
#define OCS_MOB_MAIN         24
#define OCS_MOB_GET_NLIST    25
#define OCS_MOB_GET_NAME     26
#define OCS_MOB_GET_RDESC    27
#define OCS_MOB_GET_LDESC    28
#define OCS_OBJ_GET_V0       29
#define OCS_OBJ_GET_V1       30
#define OCS_OBJ_GET_V2       31
#define OCS_OBJ_GET_V3       32
#define OCS_OBJ_GET_V4       33
#define OCS_OBJ_GET_V5       34
#define OCS_OBJ_GET_WEAR     35
#define OCS_OBJ_GET_XTRA     36
#define OCS_OBJ_GET_ACLASS   37
#define OCS_OBJ_GET_AFF      38


/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
/* Used by objsave */
#define IN_INVENTORY   18
#define IN_DEPTH1      19
#define IN_DEPTH2      20
#define IN_DEPTH3      21
#define IN_DEPTH4      22
#define IN_DEPTH5      23

#define IN_MAXDEPTH    23

/* object-related defines ********************************************/

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_FOUNTAIN  23
#define ITEM_BOMB      24   /* !yet: Exploding damage effect */
#define ITEM_RAWFOOD   25   /* !yet: Defined */
#define ITEM_PORTAL    26
#define ITEM_BOARD     27

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              (1 << 0)      /* a */
#define ITEM_WEAR_FINGER       (1 << 1)      /* b */
#define ITEM_WEAR_NECK         (1 << 2)      /* c */
#define ITEM_WEAR_BODY         (1 << 3)      /* d */
#define ITEM_WEAR_HEAD         (1 << 4)      /* e */
#define ITEM_WEAR_LEGS         (1 << 5)      /* f */
#define ITEM_WEAR_FEET         (1 << 6)      /* g */
#define ITEM_WEAR_HANDS        (1 << 7)      /* h */
#define ITEM_WEAR_ARMS         (1 << 8)      /* i */
#define ITEM_WEAR_SHIELD       (1 << 9)      /* j */
#define ITEM_WEAR_ABOUT        (1 << 10)     /* k */
#define ITEM_WEAR_WAIST        (1 << 11)     /* l */
#define ITEM_WEAR_WRIST        (1 << 12)     /* m */
#define ITEM_WIELD             (1 << 13)     /* n */
#define ITEM_HOLD              (1 << 14)     /* o */
#define ITEM_THROW             (1 << 15)     /* p */
#define ITEM_WIELD_2H          (1 << 16)     /* q */

#define MAX_WEAR_FLAGS  17

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            (1 << 0)  /* a - !yet: light source           */
#define ITEM_HUM             (1 << 1)  /* b - effect only                  */
#define ITEM_DARK            (1 << 2)  /* c - effect only                  */
#define ITEM_LOCK            (1 << 3)  /* d - effect only                  */
#define ITEM_EVIL	     (1 << 4)  /* e - dam Vs good 3d6              */
#define ITEM_INVISIBLE       (1 << 5)  /* f - makes object invisible       */
#define ITEM_MAGIC           (1 << 6)  /* g - object identifiable as magic */
#define ITEM_NODROP          (1 << 7)  /* h - not able to remove from inv  */
#define ITEM_BLESS           (1 << 8)  /* i - dam Vs Evil 3d6              */
#define ITEM_ANTI_GOOD       (1 << 9)  /* j - not usable by good people    */
#define ITEM_ANTI_EVIL       (1 << 10) /* k - not usable by evil people    */
#define ITEM_ANTI_NEUTRAL    (1 << 11) /* l - not usable by neutral people */
#define ITEM_NORENT	     (1 << 12) /* m - not allowed to rent          */
#define ITEM_NODONATE	     (1 << 13) /* n - not donate the item          */
#define ITEM_NOINVIS	     (1 << 14) /* o - not allowed to cast invis on */
#define ITEM_HIDDEN          (1 << 15) /* p - Hidden in room or in corpse  */
#define ITEM_BROKEN          (1 << 16) /* q - Item is broken               */
#define ITEM_CHAOTIC         (1 << 17) /* r - !yet: x2 dam, dam->ch.hp     */
#define ITEM_ARENA           (1 << 18) /* s - special item for arena only  */
#define ITEM_DONATED         (1 << 19) /* t - DONATED ITEM - can't be sold */
#define ITEM_FLAME           (1 << 20) /* u - Flame damage                 */
#define ITEM_NOLOCATE        (1 << 21) /* v - Can't locate object          */
#define ITEM_NOBREAK         (1 << 22) /* w - !yet: Can't break            */
#define ITEM_NOREMOVE        (1 << 23) /* x - !yet: Can't remove from wear */
#define ITEM_QUEST           (1 << 24) /* y - quest item                   */
#define ITEM_NOSWEEP         (1 << 25) /* z - sweeper doesn't destroy      */
#define ITEM_KILLER          (1 << 26) /* A - item sets killer bit         */

#define MAX_EXTRA_FLAGS  27


/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15
#define LIQ_CHAMPAGNE  16
#define LIQ_MEAD       17

/* for containers  - value[1] */

#define CONT_CLOSEABLE      (1 << 0)  /* a */
#define CONT_PICKPROOF      (1 << 1)  /* b */
#define CONT_CLOSED         (1 << 2)  /* c */
#define CONT_LOCKED         (1 << 3)  /* d */
#define CONT_TRAP           (1 << 4)  /* e */

/* For Portal Objects - value[1] */
#define PORTAL_CLOSED  (1 << 0) /* a - Portal is closed */
#define PORTAL_LOCKED  (1 << 1) /* b - Portal is locked */
#define PORTAL_RANDOM  (1 << 2) /* c - Portal has random destination */
#define PORTAL_EFFECT  (1 << 3) /* d - Portal has special effect */

#define PORTAL_MAX 4

/* other miscellaneous defines *******************************************/

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Affect duration -Petrus */
#define DURATION_PERMANENT    -1
#define DURATION_INNATE       -2

/* How much light is in the land ? */

#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET         3

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

/* For obj save system */
#define MAIN_FILE      1
#define BACKUP_FILE    2

#define MANUAL_SAVE    1
#define CRASH_SAVE     2
#define BACKUP_SAVE    3
#define DELETE_SAVE    4


#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5
#define RENT_SAVE       6

/* other #defined constants **********************************************/

#define LEVEL_IMPL      116
#define LEVEL_ADMIN     115
#define LEVEL_GREATER   114
#define LEVEL_LESSER    113
#define LEVEL_IMMORT    112
#define LEVEL_RETIRED   111
#define LEVEL_DEITY     110

#define LEVEL_BOARD	115

#define LEVEL_WIZNAME   100
#define LEVEL_WORSHIP   100

#define NUM_OF_DIRS	6

/* For Channel History */
#define CHAN_CHAR_MAX     7
#define CHAN_GLOBAL_MAX   6 /* local max is in structs.h */

#define LVL   -101
#define LVL2  -102
#define LVL3  -103

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       96          /* Max length of prompt        */
#define GARBAGE_SPACE		32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE		1024        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE	   (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define MAX_STRING_LENGTH	8192
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		500

#define MAX_NAME_LENGTH     15
#define MAX_HOSTNAME	    256
#define MAX_PWD_LENGTH	    10    /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LEN	    30   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS          400  /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MOB_SKILLS          100
#define MAX_WEAR            18
#define MAX_AFFECT          32   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT      4    /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000
#define MAX_MOB_RESISTANCES 10


#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define OPT_USEC	  100000                    /* time delay  */
#define PASSES_PER_SEC    (1000000 / OPT_USEC)
#define REAL_SEC          * PASSES_PER_SEC

#define PULSE_ZONE         (10 REAL_SEC)
#define PULSE_MOBILE       (8 REAL_SEC)
#define PULSE_GAIN         (6 REAL_SEC)
#define PULSE_VIOLENCE     (2 REAL_SEC) /* Change here for fight speed */
#define WAIT_SEC	   (1 REAL_SEC)
#define WAIT_ROUND	   (2 REAL_SEC)


/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char	       sbyte;
typedef unsigned char	       ubyte;
typedef signed short int       sh_int;
typedef unsigned short int     ush_int;
typedef char                   byte;
typedef char                   bool;

/* New -Petrus */
struct room_affect_type {
    int     type;          /* The type of spell that caused this      */
    sh_int  duration;      /* For how long its effects will last      */
    sh_int  room_num;      /* In which room - real num                */
    int	bitvector;         /* Tells which room flag to set            */

    struct room_affect_type *next;           /* Next affect in room   */
    struct room_affect_type *next_in_list;   /* Next in global list   */
};


struct exit_affect_type {
    int     type;           /* The type of spell that caused this      */
    sh_int  duration;
    char   *description;
};


struct room_direction_data {
   char	*general_description;  /* When look DIR.                  */

   char	*keyword;              /* for open/close                  */

   sh_int exit_info;           /* Exit info                       */
   sh_int key;		       /* Key's number (-1 for no key)    */
   sh_int to_room;             /* Where direction leeds (NOWHERE) */

/*   struct exit_affect_type affects;  For later use */
};


struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};

struct clan_obj_data {
     sh_int clan; /* clan of item */
     int exchange; /* vnum of item to exchange for */
};


struct obj_flag_data {
   int   value[6];	/* Values of the item (see list)    */
   byte  type_flag;	/* Type of item                     */
   int   wear_flags;	/* Where you can wear it            */
   long  extra_flags;	/* If it hums,glows etc             */
   sbyte level;         /* Min level to use                 */
   long  anticlass;     /* which class can't use            */
   int   weight;	/* Weight what else                 */
   int   cost;		/* Value when sold (gp.)            */
   int   cost_per_day;	/* Cost to keep pr. real day        */
   int   timer;	        /* Timer for object                 */
   long  bitvector;	/* To set chars bits                */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data {
   sh_int item_number;            /* Where in data-base               */
   sh_int in_room;                /* In what room -1 when conta/carr  */

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

   struct clan_obj_data *clan_eq;       /* Clan eq data if any */
   char	*name;                    /* Title of object :get etc.        */
   char	*description ;            /* When in room                     */
   char	*short_description;       /* when worn/carry/in cont.         */
   char	*action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *used_by;     /* NULL if !worn/wielded/held       */
   sh_int worn_on;                /* Worn where?                       */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
};
/* ====================================================================== */


struct room_prog_data {
  struct room_prog_data *next;
  int type;
  char *arglist;
  char *comlist;
};

#define ERROR_RPROG     -1
#define TRANS_RPROG      1
#define TTRANS_RPROG     2
#define ECHO_RPROG       3
#define PUSH_RPROG       4
#define PUSHALL_RPROG    5

typedef struct room_prog_data RPROG_DATA;


/* ========================= Structure for room ========================== */
struct room_data {
   sh_int number;                /* Rooms number                       */
   sh_int zone;                  /* Room zone (for resetting)          */
   byte   sector_type;           /* sector type (move/hide)  changed -Petrus */
   char   *name;                 /* Rooms name 'You are ...'           */
   char   *description;          /* Shown when entered                 */
   struct extra_descr_data *ex_description;        /* for examine/look */
   struct room_direction_data *dir_option[NUM_OF_DIRS];  /* Directions */
   long   room_flags;

   char   dir_from;              /* For the track/hunt system          */
   sh_int room_from;

   byte   light;                 /* Number of lightsources in room     */
   int    (*funct)();            /* special procedure                  */
  RPROG_DATA *rprogs;            /* special room prog                  */

/* struct room_affect_type *affects;   Affected by spells  NEW -Petrus */

   struct obj_data *contents;    /* List of items in room              */
   struct char_data *people;     /* List of NPC / PC in room           */
};
/* ====================================================================== */


/* This struct is for the new house-room eq crash save -Petrus */
struct room_list {
    sh_int number;
    struct room_list *next;
};


struct memory_rec_struct {
  long	id;
  struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;

/* MOBProgram foo */
struct mob_prog_act_list {
  struct mob_prog_act_list *next;
  char *buf;
  struct char_data *ch;
  struct obj_data *obj;
  void *vo;
};

typedef struct mob_prog_act_list MPROG_ACT_LIST;

struct mob_prog_data {
  struct mob_prog_data *next;
  int type;
  char *arglist;
  char *comlist;
};

typedef struct mob_prog_data MPROG_DATA;

extern bool MOBTrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024

/* end of MOBProg foo */


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
  int   hours, day, month;
  sh_int year;
};

/* These data contain information about a players time data */
struct time_data {
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  time_t last_logon; /* Time of the last save  */
  int	 played;   /* This is the total accumulated time played in secs */
};

struct char_player_data {
  char passwd[MAX_PWD_LENGTH+1]; /* character's password                 */
  char   *name;	                 /* PC / NPC s name (kill ...  )         */
  char   *short_descr;           /* for 'actions'                        */
  char   *long_descr;            /* for 'look'.. Only here for testing   */
  char   *description;           /* Extra descriptions                   */
  char   *plan;                  /* Player plan file/string              */
  char   *title;                 /* PC / NPC s title                     */
  byte   sex;                    /* PC / NPC s sex                       */
  ubyte  class;                  /* PC s class                           */
  ubyte  level;                  /* PC / NPC s level                     */
  int	 hometown;               /* PC s Hometown (zone)                 */
  struct time_data time;         /* PC s AGE in days                     */
  ubyte  weight;                 /* PC / NPC s weight                    */
  ubyte  height;                 /* PC / NPC s height                    */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
/* change these to ubyte asap -P */
struct char_ability_data {
  sbyte str;
  sbyte str_add;      /* 000 - 100 if strength 18             */
  sbyte intel;
  sbyte wis;
  sbyte dex;
  sbyte con;
  sbyte cha;
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data {
  sh_int mana;
  sh_int max_mana;     /* Max move for PC/NPC		          */
  sh_int hit;
  sh_int max_hit;      /* Max hit for PC/NPC                      */
  sh_int move;
  sh_int max_move;     /* Max move for PC/NPC                     */

  sh_int armor;        /* Internal -100..100, external -10..10 AC */
  int	 gold;         /* Money carried                           */
  int	 bank_gold;    /* Gold the char has in a bank account     */
  int	 exp;          /* The experience of the player            */

  sbyte  hitroll;      /* Any bonus or penalty to the hit roll    */
  sbyte  damroll;      /* Any bonus or penalty to the damage roll */
};

/* New struct for the generic card game system -Petrus */
struct card_list {
  struct card_list   *prev;
  sbyte              value;
  struct card_list   *next;
};


/* New struct for the alias command - Petrus*/
struct alias_list {
  char *alias;
  char *replace;
  struct alias_list *next;
};

/* New struct for ignore list */
struct ignore {
  char *name;
  struct ignore *next;
};

/* New structure for resistance */
struct resistance {
  int type;
  byte percentage;
};


struct mskill {
  int type;
  byte percentage;
};


#define MAX_MOB_ATTACKS 10
/* New structure for mob attack type */
struct attack_type {
  int   type;
  char  percent_of_use;
  byte  damodice;
  byte  damsizedice;
  sbyte damadd;
  int   damtype;
};

/* Specials used by NPCs, not PCs */
struct mob_special_data {
  byte last_direction;          /* The last direction the monster went     */
  byte default_pos;             /* Default position for NPC                */
  memory_rec *memory;	        /* List of attackers to remember           */
  struct attack_type *attacks;  /* The Attack Type Bitvector for NPC's     */
  struct resistance *resists;   /* NPC resistances                         */
  int wait_state;	        /* Wait state for bashed mobs	           */
};

/* for wargames */
struct warg_data {
   // int health;            /* hits taken            */
   int hits;              /* old hits taken - here for current compatibility */
   int ammo;              /* how much ammo         */
   int team;              /* player team           */
   int weapon;            /* weapon player using   */
   int afections;         /* used for special items*/
};


/* char_special_data's fields are fields which are needed while the game
   is running, but are not stored in the playerfile.  In other words,
   a struct of type char_special_data appears in char_data but NOT
   char_file_u.  If you want to add a piece of data which is automatically
   saved and loaded with the playerfile, add it to char_special2_data.
*/

struct char_special_data {
  struct char_data *fighting;    /* Opponent                                */
  struct char_data *hunting;     /* Hunting person..                        */
  struct char_data *mounting;    /* Mounted on                              */
  struct char_data *mounted_by;  /* Mounted by whom                         */
  struct char_data *protecting;  /* Protecting which player                 */
  struct char_data *protected_by;/* Protected by which player               */
  struct char_data *last_tell;   /* last char who telled you                */
  struct ignore    *ignore_list; /* Ignore list                             */
  struct warg_data wargame;      /* link to wargame info                    */
  long	 affected_by;            /* Bitvector for spells/skills affected by */
  long   affected_by2;           /* Temporary affects                       */

  int    worshippers;           /* For higher players: number of worshippers */
  int    power;                 /* The power of the worshippers              */

  byte   position;               /* Standing or ...                         */
  int    gain_count;             /* New gain routine  -Petrus               */
  sbyte  skillgain;              /* Copy this to specials2 later            */

  struct history_list *chan_hist[CHAN_CHAR_MAX]; /* character history */

  int	 carry_weight;           /* Carried weight                          */
  int    carry_items;            /* Number of items carried                 */
  int	 timer;                  /* Timer for update                        */
  sh_int was_in_room;            /* storage of location for linkdead people */

  struct card_list *deck_head;   /* GCS generic card system  -Petrus        */
  struct card_list *deck_tail;
  struct card_list *cards_played;
  struct card_list *cards_in_hand;

  char   *poofIn;	         /* Description on arrival of a god.	    */
  char   *poofOut; 	         /* Description upon a god's exit.	    */
  char   *transIn;               /* Description on arrival with trans cmd   */
  char   *transOut;              /* Description upon exit with trans cmd    */
  char   *wizname;               /* Wizname for players                     */
  char   *prename;               /* Prename for players                     */

  sh_int invis_level;            /* level of invisibility		    */

  struct alias_list *aliases;    /* new for alias commamd - Petrus          */
};


struct char_special2_data {
  long   idnum;		    /* player's idnum	                        */
  sh_int load_room;         /* Which room to place char in		*/
  ubyte  spells_to_learn;   /* How many can you learn yet this level    */
  int	 alignment;	    /* +-1000 for alignments                    */
  long   act;		    /* act flag for NPC's; player flag for PC's */
  long   pref;	            /* preference flags for PC's.	        */
  int	 wimp_level;	    /* Below this # of hit points, flee!        */
  byte   freeze_level;	    /* Level of god who froze char, if any      */
  ubyte  bad_pws;	    /* number of bad password attemps           */
  sh_int resistances[5];    /* Resistances against fire cold etc        */
  sbyte  conditions[3];     /* Drunk full etc.			        */

  long   worships;          /* IDnum of the player' that is worshipped  */

  int    quest;             /* Vnum of Quest that is joined in          */
  long   godlevel;          /* Setable Immortal Powers - Helm           */

  sh_int clan;
  ubyte  clanlevel;

  ubyte  race;
  ubyte  class1;
  ubyte  class2;
  ubyte  class3;
  ubyte  level1;
  ubyte  level2;
  ubyte  level3;
  ubyte  scrlen;

  ubyte  remorts;           /* number of times remorted                 */
  ubyte  questflags;        /* Quest progress flags  HACK - Helm        */

};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
  int type;              /* The type of spell that caused this      */
  sh_int duration;       /* For how long its effects will last      */
  sbyte modifier;        /* This is added to apropriate ability     */
  byte location;         /* Tells which ability to change(APPLY_XXX)*/
  long	bitvector;       /* Tells which bits to set (AFF_XXX)       */

  struct affected_type *next;
};

struct follow_type {
  struct char_data *follower;
  struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data {
  sh_int nr;                            /* monster nr (pos in file)      */
  sh_int in_room;                       /* Location                      */

  struct char_player_data player;       /* Normal data                   */
  struct char_ability_data abilities;   /* Abilities                     */
  struct char_ability_data tmpabilities;/* The abilities we will use     */
  struct char_point_data points;        /* Points                        */
  struct char_special_data specials;    /* Special plaing constants      */
  struct char_special2_data specials2;  /* Additional special constants  */
  byte *skills;			        /* dynam. alloc. array of skills */

  struct track_stack_data *trackdir;    /* stack of dirs of tracking     */

  struct affected_type *affected;       /* affected by what spells       */
  struct obj_data *equipment[MAX_WEAR]; /* Equipment array               */

  struct obj_data *carrying;            /* Head of list                  */
  struct descriptor_data *desc;         /* NULL for mobiles              */

  struct char_data *next_in_room;       /* For room->people - list       */
  struct char_data *next;               /* For either monster or ppl-list*/
  struct char_data *next_fighting;      /* For fighting list             */

  struct follow_type *followers;        /* List of chars followers       */
  struct char_data *master;             /* Who is char following?        */

  struct mob_special_data mob_specials; /* NPC specials                  */

  byte *mobskills;                      /* New: mobskills                */
  char *mobaction;                      /* New system for mob action -P  */
  char *nextact;                        /* Where in act list             */

  MPROG_ACT_LIST *mpact;
  int mpactnum;

  /* OCS SYSTEM */
  ubyte ocsmode;
  void *ocs_ptr1;
  void *ocs_ptr2;

};
/* ====================================================================== */

/* ============================  CLAN SYSTEM  ============================= */

struct  clan_data {
  sh_int vnum;              /* Virtual num of clan      */
  char *name;               /* Name of clan             */
  char *symbol;             /* Symbol of clan           */
  char *info;               /* Info aboub this clan     */
  sh_int donation;          /* Vnum of clan donation    */
  sh_int recall;            /* Vnum of clan recall room */
  int flags;                /* clan setable powers      */
  int pwr_demote;
  int pwr_enlist;
  int pwr_expel;
  int pwr_raise;
  int members;              /* Total members            */
  int gods;                 /* Total number of gods     */
  int remorts;              /* Total number of remorts  */
  int mortals;              /* Total number of mortals  */
  int level;                /* Total levels !incl immort*/
  int power;                /* Total levels             */
  long on_power;            /* Online member totals     */
  long on_power_rec;        /* Online member totals from file */
  long on_level;            /* Online level totals      */
  long wealth;              /* Total gold on and in bank*/
  char *ranknames[11][3];   /* Name of ranks            */
  char *roster[11];         /* Namelist of clan members */
};
/* ====================================================================== */

/* ============================  TRACK SYSTEM  ============================ */

struct track_stack_data {
  sh_int room;
  char dir;
  struct track_stack_data *next;
};

/* ====================================================================== */


/* element in monster and object index-tables   */
struct index_data {
   int	   virtual;        /* virtual number of this mob/obj            */
   int	   number;         /* number of existing units of this mob/obj  */
   int    progtypes;       /* program types for MOBProg		        */
   MPROG_DATA *mobprogs;   /* programs for MOBProg		        */
   int	  (*func)();       /* special procedure for this mob/obj        */
};



struct weather_data {
  int	pressure;    /* How is the pressure ( Mb ) */
  int	change;      /* How fast and what way does it change. */
  int	sky;	      /* How is the sky. */
  int	sunlight;    /* And how much sun. */
};


/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u {
  byte   sex;
  byte   class;
  ubyte  level;
  time_t  birth;     /* Time of birth of character     */
  int	  played;    /* Number of secs played in total */

  ubyte  weight;
  ubyte  height;

  sh_int hometown;

  struct char_ability_data abilities;

  struct char_point_data points;

  byte   skills[MAX_SKILLS];

  struct affected_type affected[MAX_AFFECT];

  struct char_special2_data specials2;

  time_t last_logon;		/* Time (in secs) of last logon */
  char   host[HOST_LEN+1];	/* host of last logon */

  /* char data */
  char	 name[20];
  char	 pwd[MAX_PWD_LENGTH+1];
};



/* ************************************************************************
*  file element for object file. BEWARE: Changing it will ruin rent files *
************************************************************************ */


struct obj_file_elem {
  sh_int item_number;

  char  pos;
  int	  value[6];
  int	  extra_flags;
  int	  weight;
  int	  timer;
  long  bitvector;
  struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


/* header block for rent files */
struct rent_info {
  int	time;
  int	rentcode;
  int	net_cost_per_diem;
  int	gold;
  int	account;
  int	nitems;
  int	spare0;
  int	spare1;
  int	spare2;
  int	spare3;
  int	spare4;
  int	spare5;
  int	spare6;
  int	spare7;
};


/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */

struct txt_block {
  char	*text;
  struct txt_block *next;
};

struct txt_q {
  struct txt_block *head;
  struct txt_block *tail;
};

struct descriptor_data {
  socket_t ident_sock;             /* socket for ident process             */
  socket_t descriptor;		   /* file descriptor for socket       	   */
  u_short peer_port;               /* port of peer                         */
  int    ident_state;              /* status of ident lookup               */
  char   ident_name[HOST_LEN+1];   /* name from ident lookup               */
  byte   ident_idle;               /* number of idle tics for ident        */
  char   *name;			   /* ptr to name for mail system     	   */
  char   host[HOST_LEN+1];	   /* hostname			           */
  char   pwd[MAX_PWD_LENGTH+1];    /* password                             */
  byte   bad_pws;		   /* number of bad pw attemps this login  */
  byte   idle_tics;                /*number of tics idle at passwd prompt  */
  int    pos;			   /* position in player-file	           */
  int	 connected;		   /* mode of 'connectedness'	           */
  int	 wait;		           /* wait for how many loops	           */
  int	 desc_num;		   /* unique num assigned to desc    	   */
  long   login_time;		   /* when the person connected		   */
  char   *showstr_head;	           /* for keeping track of an internal str */
  char   **showstr_vector;	   /* for paging through texts	           */
  int    showstr_count;		   /* number of pages to page through	   */
  int    showstr_page;		   /* which page are we currently showing? */
  char   **str;			   /* for the modify-str system        	   */
  char   *backstr;		   /* added for handling abort buffers     */
  int	 max_str;                  /*		-		           */
  int	 prompt_mode;	           /* control of prompt-printing       	   */
  char   inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input	           */
  char   last_input[MAX_RAW_INPUT_LENGTH]; /* the last input      	   */
  char   num_input[SMALL_BUFSIZE]; /* Numerical input and multiple cmds    */
  char   mult_input[SMALL_BUFSIZE]; /* input  -Petrus                      */
  char   small_outbuf[SMALL_BUFSIZE]; /* standard output bufer		   */
  char   *output;		   /* ptr to the current output buffer	   */
  int    bufptr;		   /* ptr to end of current output	   */
  int	 bufspace;		   /* space left in the output buffer      */
  struct txt_block *large_outbuf;  /* ptr to large buffer, if we need it   */
  struct txt_q input;		   /* q of unprocessed input	           */
  struct char_data *character;     /* linked to char		           */
  struct char_data *original;	   /* original char if switched		   */
  struct descriptor_data *snooping; /* Who is this char snooping           */
  struct descriptor_data *snoop_by; /* And who is snooping this char       */
  struct descriptor_data *next;    /* link to next descriptor	           */
  char   *storage;                 /* used for tedit                       */
};


struct msg_type {

  char	*attacker_msg;		/* message to attacker */
  char	*victim_msg;		/* message to victim   */
  char	*room_msg;		/* message to room     */
};

struct message_type {
  struct msg_type      die_msg;      /* messages when death   */
  struct msg_type      miss_msg;     /* messages when miss    */
  struct msg_type      hit_msg;      /* messages when hit     */
  struct message_type *next;	     /* to next messages of this kind.	*/
};

struct message_list {
  int	number_of_attacks;	/* How many attack messages to chose from. */
  struct message_type *msg;	/* List of messages.			*/
};

struct dex_app_type {
  sh_int reaction;
  sh_int miss_att;
  sh_int defensive;
};

struct str_app_type {
  sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
  sh_int todam;    /* Damage Bonus/Penalty                */
  sh_int carry_w;  /* Maximum weight that can be carrried */
  sh_int wield_w;  /* Maximum weight that can be wielded  */
  sh_int bash;     /* Bash % */
};

struct wis_app_type {
  byte bonus;       /* how many bonus skills a player can */
                    /* practice pr. level                 */
};

struct int_app_type {
  byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type {
  sh_int hitp;
  sh_int shock;
};

struct ama_info_type {
  char *sin;        /* Damage Message singular*/
  char *plu;        /* Damage Message plural*/
  sh_int dam;       /* DR bonus */
};


/* misc editor defines **************************************************/

/* format modes for format_text */
#define FORMAT_INDENT      (1 << 0)


#endif   /* STRUCT_H */



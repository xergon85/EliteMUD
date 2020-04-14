/* ************************************************************************
*   File: constants.c                                   Part of EliteMUD  *
*  Usage: Numeric and string contants used by the MUD                     *
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
#include "spells.h"

const char	elitemud_version[] = {
   "EliteMUD Version 6.4.122\r\nJuly 2003 - Code Group" };


const char      *class_3abb[] = {
   "-",
   "#MM#N",
   "#GC#N",
   "#YT#N",
   "#BW#N",
   "#RP#N",
   "#gM#N",
   "#YB#N",
   "#CK#N",
   "#mW#N",
   "#GD#N",
   "#rA#N",
   "#GR#N",
   "#yI#N",
   "#bP#N",
   "#BM#N",
   "#cC#N",
   "!",
   "!",
   "#wN#N",
   "!",
   "D",
   "2",
   "3",
   "?"
};

const char      *class_2abb[] = {
   "--",
   "#MMu#N",
   "#GCl#N",
   "#YTh#N",
   "#BWa#N",
   "#RPs#N",
   "#gMo#N",
   "#YBa#N",
   "#CKn#N",
   "#mWi#N",
   "#GDr#N",
   "#rAs#N",
   "#GRa#N",
   "#yIl#N",
   "#bPa#N",
   "#BMa#N",
   "#cCa#N",
   "!!",
   "!!",
   "#wNi#N",
   "!!",
   "Du",
   "2m",
   "3m",
   "??"
};

const char	*class_abbrevs[] = {
   "-- --",
   "#MMu#N   ",
   "#GCl#N   ",
   "#YTh#N   ",
   "#BWa#N   ",
   "#RPs#N   ",
   "#gMo#N   ",
   "#YBa#N   ",
   "#CKn#N   ",
   "#mWi#N   ",
   "#GDr#N   ",
   "#rAs#N   ",
   "#GRa#N   ",
   "#yIl#N   ",
   "#bPa#N   ",
   "#BMa#N   ",
   "#cCa#N   ",
   "!USED",
   "!USED",
   "#wNi#N   ",
   "!USED",
   "Dual ",
   "2Mult",
   "3Mult",
   "?????"
   };
/*
const char     *class_dual[4][4] = {
{ "Mu/Mu", "#MMu#N/#GCl#N", "#MMu#N/#YTh#N", "#MMu#N/#BWa#N"},
{ "#GCl#N/#MMu#N", "Cl/Cl", "#GCl#N/#YTh#N", "#GCl#N/#BWa#N"},
{ "#YTh#N/#MMu#N", "#YTh#N/#GCl#N", "Th/Th", "#YTh#N/#BWa#N"},
{ "#BWa#N/#MMu#N", "#BWa#N/#GCl#N", "#BWa#N/#YTh#N", "Wa/Wa"}};

const char     *class_multi[] =
{"#BW#N/#GC#N/#MM#N", "#BW#N/#YT#N/#MM#N"};
*/

const char     *race_table[]  =  {
    "god",
    "human",
    "elf",
    "half-elf",
    "dwarf",
    "gnome",
    "halfling",
    "half-troll",
    "half-orc",
    "half-ogre",
    "duck",
    "fairy",
    "minotaur",
    "ratman",
    "drow",
    "lizard",
    "vampire",
    "troll",
    "draconian",
    "avatar",
    "werewolf",
    "demon",
    "dragon",
    "feline",
    "angel",
    "\n"
    };

const char   *race_title_names[]  =  {
    "#rDivine#N",
    "#wHuman#N",
    "#bElven#N",
    "#cHalf-elven#N",
    "#RDwarven#N",
    "#gGnome#N",
    "#YHalfling#N",
    "#yHalf-troll#N",
    "#MHalf-orc#N",
    "#mHalf-ogre#N",
    "#YDuck#N",
    "#RFairy#N",
    "#RMinotaur#N",
    "#MRatman#N",
    "#RDrow#N",
    "#GLizard#N",
    "#mVampire#N",
    "#yTroll#N",
    "#GDraconian#N",
    "#rAvatar#N",
    "#mWerewolf#N",
    "#wDemon#N",
    "#GDragon#N",
    "#GFeline#N",
    "#wAngel#N",
    "\n"
    };

/* Race max            GOD  HU  EL  HE  DW  GN  HL  HT  HO  HO  DU  Fa  Mi  Ra  Dr  Li  Va  Tr Drc  Av  We  De  Dra Fe  An*/
const int str_max[]= {  25, 18, 18, 18, 20, 16, 16, 20, 19, 18, 16, 15, 20, 16, 18, 20, 21, 20, 20, 23, 20, 24, 21, 18, 20 };
const int con_max[]= {  25, 18, 17, 18, 20, 18, 18, 20, 18, 18, 18, 16, 20, 18, 18, 18, 21, 20, 19, 23, 20, 24, 21, 19, 20 };
const int dex_max[]= {  25, 18, 19, 18, 16, 20, 20, 16, 16, 18, 20, 20, 18, 20, 18, 18, 21, 16, 18, 23, 19, 24, 14, 24, 20 };
const int int_max[]= {  25, 18, 18, 18, 16, 20, 18, 14, 16, 18, 16, 20, 16, 16, 18, 16, 21,  9, 16, 23, 17, 24, 20, 16, 20 };
const int wis_max[]= {  25, 18, 18, 18, 18, 18, 18, 14, 16, 18, 16, 18, 16, 18, 18, 18, 21,  9, 15, 23, 16, 24, 20, 18, 20 };
const int cha_max[]= {  25, 18, 18, 18, 18, 18, 18, 12, 16, 18, 16, 18, 16, 16, 16, 16, 21, 12, 16, 23, 16, 24, 14, 19, 20 };

const int
age_average[]     =  {9999, 18, 90, 60, 80,100, 40, 50, 26, 80, 14,300, 40, 16, 26, 34, 500,50,100,300,30,1000,1000,15,100};
const int
height_average[]  =  { 300, 72, 64, 68, 48, 48, 42, 84, 84, 70, 72, 24,120, 60, 64, 78, 72,108, 84,78, 74, 84,180, 56, 84 };
const int
weight_average[]  =  {1000,150,115,135,135,115,100,180,170,250,130, 30,300,120,115,110,150,275,200,200,190,200,600,130,175 };

/* Race max            GOD  HU  EL  HE  DW  GN  HL  HT  HO  HO  DU  Fa  Mi  Ra  Dr  Li  Va  Tr Drc  Av  We  De  Dra Fe */
const int hp_start[]={ 999, 40, 30, 35, 55, 20, 20, 55, 50, 55, 30, 20, 60, 30, 30, 60, 45, 65, 40, 75, 65, 65, 70, 40, 70};
const int mn_start[]={ 999, 88,110, 90, 80,140, 88, 30, 40, 40, 60,120, 40, 70,110, 40, 90, 20, 80,150, 20,100,60,  90, 100};
const int mv_start[]={ 999, 90, 95, 90, 75, 75, 75, 85,100,100, 80, 70,120, 85, 95,110, 85,100,150,200,110,150,120, 150, 125};

const long  allowed_classes[] = {
    0 ,     /* god - noneclass */
    NE_CLASS | MO_CLASS | BA_CLASS | KN_CLASS | WI_CLASS |
    DR_CLASS | AS_CLASS | RA_CLASS | IL_CLASS | PA_CLASS |
    MA_CLASS | CA_CLASS | NI_CLASS,                       /* human - most */
    BA_CLASS | KN_CLASS | WI_CLASS | DR_CLASS | RA_CLASS |
    PA_CLASS | M2_CLASS | M3_CLASS,                       /* elf */
    BA_CLASS | KN_CLASS | WI_CLASS | DR_CLASS | RA_CLASS |
    PA_CLASS | NI_CLASS | M2_CLASS | M3_CLASS,            /* halfelf  */
    WA_CLASS | MO_CLASS | DR_CLASS | M2_CLASS | MA_CLASS, /* dwarf */
    DR_CLASS | IL_CLASS | NE_CLASS | RA_CLASS | MO_CLASS |
    M2_CLASS,                                             /* gnome */
    AS_CLASS | DR_CLASS | RA_CLASS | MO_CLASS | M2_CLASS, /* halfling */
    CL_CLASS | TH_CLASS | WA_CLASS | DR_CLASS | RA_CLASS |
    MA_CLASS,                                             /* half-troll */
    TH_CLASS | WA_CLASS | MA_CLASS | NE_CLASS,            /* halforc */
    TH_CLASS | WA_CLASS | MA_CLASS,                       /* half-ogre */
    MU_CLASS | CL_CLASS | TH_CLASS | WA_CLASS | BA_CLASS |
    AS_CLASS | MA_CLASS | M2_CLASS,                       /* duck */
    WI_CLASS | DR_CLASS | IL_CLASS | NE_CLASS | MO_CLASS, /* fairy */
    WI_CLASS | MO_CLASS | WA_CLASS | NE_CLASS | M2_CLASS |
    DR_CLASS,                                             /* minotaur */
    IL_CLASS | MO_CLASS | AS_CLASS | M2_CLASS,            /* ratman */
    AS_CLASS | BA_CLASS | KN_CLASS | WI_CLASS | DR_CLASS |
    RA_CLASS | M2_CLASS | M3_CLASS,                       /* drow  */
    MO_CLASS | DR_CLASS | WA_CLASS | AS_CLASS | M2_CLASS, /* lizard */
    0,     /* vampire - noneclass */
    BA_CLASS | WA_CLASS | RA_CLASS | DR_CLASS,            /* Troll */
    AS_CLASS | WA_CLASS | WI_CLASS | NE_CLASS | IL_CLASS |
    M2_CLASS,                                             /* Draconian */
    0,     /* Avatar - noneclass */
    0,     /* Werewolf - noneclass */
    0,     /* Demon - noneclass */
    0,     /* Dragon - noneclass */
    0      /* Feline - noneclass */
};


const char *skills[] = {
    "stab",    /* NO 300 */
    "bludgeon",
    "slash",
    "chop",
    "second attack",
    "cook",
    "spellcraft",
    "flay",
    "swim",
    "dive",
    "climb",
    "first aid",
    "sneak",
    "hide",
    "steal",
    "backstab",
    "pick lock",
    "dual weapons",
    "dodge",
    "throw",
    "disarm traps",
    "martial arts",
    "critical hit",
    "kick",
    "bash",
    "rescue",
    "third attack",
    "fourth attack",
    "parrying",
    "berzerk",
    "two-handed weapon",
    "poison blade",
    "battle tactics",
    "track",
    "hunt",
    "archery",
    "sail",
    "appraise",
    "tumble",
    "blindfight",
    "disbelieve illusion",
    "joust",
    "feign death",
    "riding landbased",
    "riding airborne",
    "disarm foe",
    "fish",
    "forgery",
    "stonemasonry",
    "herbalism",
    "disquise",
    "jump",
    "ventriloquism",
    "intimidate",
    "animal handling",
    "set snare",
    "heroic rescue",
    "taunt",
    "escape",
    "scout",
    "spy",
    "search",
    "trip",
    "unfair fight",
    "headbang",
    "extra damage",
    "vitalize mana",
    "meditate",
    "trade",
    "mounted battle",
    "find path",
    "navigate",
    "circle around",
    "pugilism",
    "vitalize stamina",
    "pierce",
    "first to attack",
    "tail lash",
    "horn butt",
    "bite",
    "kvack fu",
    "mend armor",
    "repair weapon",
    "detect traps",
    "claw",
    "pounce",
    "ninjitsu",
    "adv. martial arts",
    "\n"
};

#define XX  LEVEL_DEITY

const char skill_minlevel[88][20] =  {
/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*STAB */{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,XX,XX, 1,XX},
/*BLUD */{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,XX,XX, 1,XX},
/*SLAS */{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,XX,XX, 1,XX},
/*CHOP */{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,XX,XX, 1,XX},
/*2ATT */{50,30,10, 5,50,15, 8, 4,50,30,10, 6,50,16,10, 5,XX,XX,10,XX},
/*COOK */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*SPEL */{ 1, 1,XX,XX, 1,10,15,50, 1, 1,XX,XX, 1,10,XX,XX,XX,XX,XX,XX},
/*FLAY */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*SWIM */{ 8, 5, 5, 4, 8, 5, 5, 5, 1, 5, 5, 3, 8, 5, 1, 5,XX,XX, 5,XX},
/*DIVE */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*CLIM */{12, 8, 8, 8,12, 8, 8, 9,12, 8, 8, 6,12, 8, 8, 8,XX,XX, 8,XX},
/*1AID */{ 5, 3,XX,XX, 6, 9,XX,XX, 5, 5,XX,XX, 5, 9,XX,XX,XX,XX,XX,XX},
/*SNEA */{XX,XX, 3,XX,XX,XX, 9,XX,XX,XX, 2, 9,XX,XX, 9,XX,XX,XX, 2,XX},
/*HIDE */{XX,XX, 6,XX,XX,XX,18,XX,XX,XX, 3,18,XX,XX,36,XX,XX,XX, 3,XX},
/*STEA */{XX,XX,12,XX,XX,XX,36,XX,XX,XX,24,XX,XX,XX,36,XX,XX,XX,35,XX},
/*BACK */{XX,XX, 1,XX,XX,XX, 1,XX,XX,XX, 1,XX,XX,XX,XX,XX,XX,XX, 5,XX},
/*PICK */{XX,XX, 9,XX,XX,XX,27,XX,XX,XX,18,XX,XX,XX,XX,XX,XX,XX,18,XX},
/*DUAL */{XX,XX,30,XX,XX,XX,XX,XX,XX,XX,15,50,XX,XX,XX,XX,XX,XX,20,XX},
/*DODG */{XX,XX,15,XX,XX,30,45,XX,XX,XX,12,15,XX,XX,XX,XX,XX,XX,12,XX},
/*THRO */{XX,XX,18,XX,XX,XX,54,XX,XX,XX, 9,XX,XX,XX,42,XX,XX,XX, 9,XX},
/*DTRA */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*MART */{XX,XX,35,XX,XX,20,XX,XX,XX,XX,10,XX,XX,XX,XX,XX,XX,XX,11,XX},
/*CRIT */{XX,XX,60,XX,XX,XX,XX,XX,XX,XX,30,XX,XX,XX,XX,XX,XX,XX,70,XX},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*KICK */{XX,XX,XX,10,XX,14,30,12,XX,XX,22,14,XX,XX,10,10,XX,XX,22,XX},
/*BASH */{XX,XX,XX,12,XX,XX,36,14,XX,XX,XX,16,XX,45,12,12,XX,XX,XX,XX},
/*RESC */{XX,XX,XX,14,XX,XX,XX, 6,XX,XX,XX,18,XX,42,38,14,XX,XX,XX,XX},
/*3ATT */{XX,50,XX,16,55,50,48,15,XX,50,44,17,55,24,24,16,XX,XX,50,XX},
/*4ATT */{XX,XX,XX,24,XX,XX,XX,24,XX,XX,XX,26,XX,36,36,24,XX,XX,XX,XX},
/*PARR */{XX,XX,XX, 2,XX,XX, 6, 1,XX,XX,XX, 6,XX, 6, 8, 2,XX,XX,XX,XX},
/*BERZ */{XX,XX,XX,18,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,18,XX,XX,XX,XX},
/*2HAN */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*POIS */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,60,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*BATT */{XX,XX,XX,26,XX,XX,XX,16,XX,XX,XX,30,XX,XX,XX,20,XX,XX,XX,XX},
/*TRAC */{15,12,12, 6,15,12, 8, 6,15,12, 3, 1,15,12,15, 6,XX,XX, 7,XX},
/*HUNT */{XX,XX,XX,XX,XX,XX,XX,XX,XX,45,XX, 1,XX,XX,XX,XX,XX,XX,XX,XX},
/*ARCH */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*SAIL */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*APPR */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*TUMB */{XX,XX,28,XX,XX,XX,XX,XX,XX,XX,14,XX,XX,XX,XX,XX,XX,XX,19,XX},
/*BLIN */{XX,XX,XX,22,XX,24,XX,11,XX,XX,44,20,XX,30,44,22,XX,XX,44,XX},
/*DISB */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*JOUS */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*FEIG */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*RIDEL*/{15,12,10,10,15,12,10, 6,15,12,10, 8,15,10,15, 8,XX,XX,10,XX},
/*RIDEA*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*DIS_F*/{XX,XX,22,32,XX,XX,27,20,XX,XX,18,32,XX,52,32,32,XX,XX,25,XX},
/*FISH */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*FORGE*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*STONE*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*HERB */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*DISGU*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*JUMP */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*VENTR*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*INTIM*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ANIMH*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*SET S*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*HEROI*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*TAUNT*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ESCAP*/{XX,XX,24,XX,XX,XX,72,XX,XX,XX,15,XX,XX,XX,XX,50,XX,XX, 9,XX},
/*SCOUT*/{XX,XX,34,38,XX,XX,36,40,XX,XX,22,12,XX,38,36,38,XX,XX,28,XX},
/*SPY  */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*SEARC*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*TRIP */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*UNFAI*/{XX,XX, 1,XX,XX,XX, 3,XX,XX,XX, 1,XX,XX,XX,XX, 1,XX,XX, 3,XX},
/*HEADB*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*X_DAM*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*VMANA*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*MEDIT*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*TRADE*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*MOUNT*/{XX,XX,XX,25,XX,XX,34, 9,XX,XX,XX,14,XX,14,XX,28,XX,XX,XX,XX},
/*PATHF*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*NAVIG*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*CIRCL*/{XX,XX,60,XX,XX,XX,XX,XX,XX,XX,40,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*PUGIL*/{XX,XX,XX,36,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,30,30,XX,XX,XX,XX},
/*VSTAM*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*PIERC*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,XX,XX, 1,XX},
/*1 ATT*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*TAIL */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*HORN */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*BITE */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*KVACK*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*MEND */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*REPAI*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*DETTR*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*CLAW */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*POUNC*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*NINJI*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,21,XX},
/*ADVMA*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,30,XX}

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */
};


const char skill_max[88][20] =  {
/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*STAB */{95,50,95,95,95,50,95,95,50,50,95,95,50,95,95,95,95,95,95,95},
/*BLUDG*/{50,95,95,95,50,95,95,95,50,95,95,95,50,95,95,95,95,95,95,95},
/*SLASH*/{20,40,95,95,20,30,95,95,20,40,95,95,20,95,95,95,95,95,95,95},
/*CHOP */{10,20,50,95,10,20,95,95,10,20,50,95,10,95,95,95,95,95,95,95},
/*2ATTA*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*COOK */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SPELL*/{95,95,95,95,95,95,50,50,95,95,95,95,95,70,95,95,95,95,95,95},
/*FLAY */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SWIM */{80,90,95,95,80,90,95,80,80,90,95,95,80,85,95,90,95,95,95,95},
/*DIVE */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*CLIMB*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*1AID */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SNEAK*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*HIDE */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*STEAL*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*BACKS*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*PICKL*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*DUAL */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*DODGE*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*THROW*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*DTRAP*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*MARTS*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*CRITI*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*KICK */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*BASH */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*RESC */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*3ATT */{95,95,95,95,95,95,95,95,95,80,95,95,95,95,95,95,95,95,95,95},
/*4ATT */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*PARRY*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*BERZ */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*2HAND*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*POISB*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*BATTL*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*TRACK*/{30,50,50,50,30,50,50,50,50,50,75,95,50,50,50,50,95,95,95,95},
/*HUNT */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*ARCH */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SAIL */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*APPR */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*TUMB */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*BLINF*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*DISB */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*JOUST*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*FEIGN*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*RIDEL*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*RIDEA*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*DISAR*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*FISH */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*FORGE*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*STONE*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*HERB */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*DISGU*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*JUMP */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*VENTR*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*INTIM*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*ANIMH*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SET S*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*HEROI*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*TAUNT*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*XCAPE*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SCOUT*/{ 5, 5,95,50, 5, 5,95,50, 5, 5,95,95, 5,50,95,50,95,95,95,95},
/*SPY  */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*SEARC*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*TRIP */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*UNFAI*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*HEADB*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*X_DAM*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*VMANA*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*MEDIT*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*TRADE*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*MOUNT*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*PATHF*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*NAVIG*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*CIRCL*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*PUGIL*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*VSTAM*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*PIERC*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*1 ATT*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*TAIL */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*HORN */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*BITE */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*KVACK*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*MEND */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*REPAI*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*DETTR*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*CLAW */{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*POUNC*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*NINJI*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95},
/*ADVMA*/{95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95}

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */
};


const char skill_difficulty[88][20] =  {
/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*STAB */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BLUDG*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SLASH*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*CHOP */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*2ATTA*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*COOK */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SPELL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*FLAY */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SWIM */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DIVE */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*CLIMB*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*1AID */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SNEAK*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*HIDE */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*STEAL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BACKS*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*PICKL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DUAL */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DODGE*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*THROW*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DTRAP*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*MARTS*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*CRITI*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*KICK */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BASH */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*RESC */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*3ATT */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*4ATT */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*PARRY*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BERZ */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*2HAND*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*POISB*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BATTL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*TRACK*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*HUNT */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*ARCH */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SAIL */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*APPR */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*TUMB */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BLINF*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DISB */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*JOUST*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*FEIGN*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*RIDEL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*RIDEA*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*DISAR*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*FISH */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*FORGE*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*STONE*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*HERB */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DISGU*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*JUMP */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*VENTR*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*INTIM*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*ANIMH*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SET S*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*HEROI*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*TAUNT*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*XCAPE*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SCOUT*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SPY  */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*SEARC*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*TRIP */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*UNFAI*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*HEADB*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*X_DAM*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*VMANA*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*MEDIT*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*TRADE*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*MOUNT*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*PATHF*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*NAVIG*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*CIRCL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*PUGIL*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*VSTAM*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*PIERC*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*1 ATT*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*TAIL */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*HORN */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*BITE */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*KVACK*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*MEND */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*REPAI*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*DETTR*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*CLAW */{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*POUNC*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*NINJI*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
/*ADVMA*/{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}

/* Skill  MU CL TH WA PS MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */
};


const struct ama_info_type ama_info[78] = {
  /*              Singular                      Plural  DR */
  { "straight punch"           , "straight punches"        ,  2}, /* 30 */
  { "straight punch"           , "straight punches"        ,  2},
  { "straight punch"           , "straight punches"        ,  2},
  { "backhand strike"          , "backhand strikes"        ,  3}, /* 33 */
  { "backhand strike"          , "backhand strikes"        ,  3},
  { "double punch"             , "double punches"          ,  4}, /* 35 */
  { "double punch"             , "double punches"          ,  4},
  { "ridge-hand-strike"        , "ridge-hand strikes"      ,  5}, /* 37 */
  { "ridge-hand-strike"        , "ridge-hand strikes"      ,  5},
  { "ridge-hand-strike"        , "ridge-hand strikes"      ,  5},
  { "reverse punch"            , "reverse punches"         ,  6}, /* 40 */
  { "reverse punch"            , "reverse punches"         ,  6},
  { "scissors punch"           , "scissors punches"        ,  7}, /* 42 */
  { "scissors punch"           , "scissors punches"        ,  7},
  { "scissors punch"           , "scissors punches"        ,  7},
  { "fore-knuckle fist"        , "fore-knuckle fists"      ,  8}, /* 45 */
  { "fore-knuckle fist"        , "fore-knuckle fists"      ,  8},
  { "fore-knuckle fist"        , "fore-knuckle fists"      ,  8},
  { "snap kick"                , "snap kicks"              ,  9}, /* 48 */
  { "snap kick"                , "snap kicks"              ,  9},
  { "snap kick"                , "snap kicks"              ,  9},
  { "one-knuckle fist strike"  , "one-knuckle fist strikes", 10}, /* 51 */
  { "one-knuckle fist strike"  , "one-knuckle fist strikes", 10},
  { "parallel punch"           , "parallel punches"        , 11}, /* 53 */
  { "parallel punch"           , "parallel punches"        , 11},
  { "lunge punch"              , "lunge punches"           , 12}, /* 55 */
  { "lunge punch"              , "lunge punches"           , 12},
  { "lunge punch"              , "lunge punches"           , 12},
  { "side kick"                , "side kicks"              , 13}, /* 58 */
  { "side kick"                , "side kicks"              , 13},
  { "front kick"               , "front kicks"             , 14}, /* 60 */
  { "front kick"               , "front kicks"             , 14},
  { "vertical strike"          , "vertical strikes"        , 15}, /* 62 */
  { "vertical strike"          , "vertical strikes"        , 15},
  { "vertical strike"          , "vertical strikes"        , 15},
  { "elbow smash"              , "elbow smashes"           , 16}, /* 65 */
  { "elbow smash"              , "elbow smashes"           , 16},
  { "spear hand"               , "spear hands"             , 17}, /* 67 */
  { "spear hand"               , "spear hands"             , 17},
  { "roundhouse punch"         , "roundhouse punches"      , 18}, /* 69 */
  { "roundhouse punch"         , "roundhouse punches"      , 18},
  { "roundhouse kick"          , "roundhouse kicks"        , 19}, /* 71 */
  { "roundhouse kick"          , "roundhouse kicks"        , 19},
  { "roundhouse kick"          , "roundhouse kicks"        , 19},
  { "back-fist strike"         , "back-fist strikes"       , 20}, /* 74 */
  { "back-fist strike"         , "back-fist strikes"       , 20},
  { "back-fist strike"         , "back-fist strikes"       , 20},
  { "back-fist strike"         , "back-fist strike"        , 20},
  { "back kick"                , "back kicks"              , 21}, /* 78 */
  { "back kick"                , "back kicks"              , 21},
  { "thrust kick"              , "thrust kicks"            , 22}, /* 80 */
  { "thrust kick"              , "thrust kicks"            , 22},
  { "outside circular strike"  , "outside circular strikes", 23}, /* 82 */
  { "outside circular strike"  , "outside circular strikes", 23},
  { "outside circular strike"  , "outside circular strikes", 23},
  { "crescent kick"            , "crescent kicks"          , 24}, /* 85 */
  { "crescent kick"            , "crescent kicks"          , 24},
  { "jump kick"                , "jump kicks"              , 25}, /* 87 */
  { "jump kick"                , "jump kicks"              , 25},
  { "jump kick"                , "jump kicks"              , 25},
  { "upwards punch"            , "upwards punches"         , 26}, /* 90 */
  { "upwards punch"            , "upwards punches"         , 26},
  { "upwards punch"            , "upwards punches"         , 26},
  { "reverse crescent kick"    , "reverse crescent kicks"  , 27}, /* 93 */
  { "reverse crescent kick"    , "reverse crescent kicks"  , 27},
  { "eagle-hand strike"        , "eagle-hand strikes"      , 28}, /* 95 */
  { "eagle-hand strike"        , "eagle-hand strikes"      , 28},
  { "eagle-hand strike"        , "eagle-hand strikes"      , 28},
  { "knife-hand strike"        , "knife-hand strikes"      , 29}, /* 98 */
  { "knife-hand strike"        , "knife-hand strikes"      , 29},
  { "knee smash"               , "knee smashes"            , 30}, /* 100 */
  { "knee smash"               , "knee smashes"            , 30},
  { "knee smash"               , "knee smashes"            , 30},
  { "knee smash"               , "knee smashes"            , 30},
  { "knee smash"               , "knee smashes"            , 30},
  { "hammer-fist strike"       , "hammer-fist strikes"     , 38}, /* 105 */
  { "hammer-fist strike"       , "hammer-fist strikes"     , 38},
  { "mountain strike"          , "mountain strikes"        , 42}  /* 107 */
};

const char *attacktypes[] = {
    "Hit",
    "Bludgeon",
    "Pierce",
    "Slash",
    "Blast",
    "Whip",
    "Pierce no bs",
    "Claw",
    "Bite",
    "Sting",
    "Crush",
    "\n"
};


const char *damtypes[] = {
    "Normal",
    "\n"
};


const int	rev_dir[] =
{
   2,
   3,
   0,
   1,
   5,
   4
};


const int	movement_loss[] =
{
   1,  /* Inside     */
   2,  /* City       */
   2,  /* Field      */
   3,  /* Forest     */
   4,  /* Hills      */
   6,  /* Mountains  */
   4,  /* Swimming   */
   4,  /* Unswimable */
   9,  /* Underwater */
   4,  /* Air        */
   1,  /* Void       */
   5,  /* Desert     */
   3,  /* Frozen Waste */
   7,  /* Frozen Mountain */
   9,  /* Icy Underwater */
   9   /* Icy Underwater noswim */
};


const char	*dirs[] =  {
   "north",
   "east",
   "south",
   "west",
   "up",
   "down",
   "\n"
};

const char    *from_dir[] =  {
    "the north",
    "the east",
    "the south",
    "the west",
    "above",
    "below",
    "\n"
};

const char	*weekdays[7] = {
   "the Day of the Moon",
   "the Day of the Bull",
   "the Day of the Deception",
   "the Day of Thunder",
   "the Day of Freedom",
   "the Day of Rigel the Giant",
   "the Day of IO the Creator" };


const char	*month_name[17] = {
   "Month of Winter",           /* 0 */
   "Month of the Winter Wolf",
   "Month of the Frost Giant",
   "Month of the Old Forces",
   "Month of the Grand Struggle",
   "Month of the Spring",
   "Month of Nature",
   "Month of Futility",
   "Month of the Dragon",
   "Month of the Sun",
   "Month of the Heat",
   "Month of the Battle",
   "Month of the Dark Shades",
   "Month of the Shadows",
   "Month of the Long Shadows",
   "Month of the Ancient Darkness",
   "Month of the Great Evil"
};


const int	sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0 };  /* Pierce   */


const char	*where[] = {
   "<used as light>      ",
   "<worn on finger>     ",
   "<worn on finger>     ",
   "<worn around neck>   ",
   "<worn around neck>   ",
   "<worn on body>       ",
   "<worn on head>       ",
   "<worn on legs>       ",
   "<worn on feet>       ",
   "<worn on hands>      ",
   "<worn on arms>       ",
   "<worn as shield>     ",
   "<worn about body>    ",
   "<worn about waist>   ",
   "<worn around wrist>  ",
   "<worn around wrist>  ",
   "<wielded>            ",
   "<held>               ",
   /* Used by listrent */
   "<in inventory>       ",
   " <in container 1>    ",
   "  <in container 2>   ",
   "   <in container 3>  ",
   "    <in container 4> ",
   "     <in container 5>",
   "     <in container 6>"
};


const char	*drinks[] =
{
   "water",
   "beer",
   "wine",
   "ale",
   "dark ale",
   "vodka",
   "lemonade",
   "dwarven spirits",
   "local speciality",
   "slime mold juice",
   "milk",               /* 10 */
   "tea",
   "coffee",
   "blood",
   "salt water",
   "clear water",
   "champagne",
   "mead",
   "orange juice",
   "\n"
};


const char	*drinknames[] =
{
   "water",
   "beer",
   "wine",
   "ale",
   "ale",
   "vodka",
   "lemonade",
   "spirits",
   "local",
   "juice",
   "milk",         /* 10 */
   "tea",
   "coffee",
   "blood",
   "salt",
   "water",
   "champagne",
   "mead",
   "juice"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int	drink_aff[][3] = {
  /* Drunk Hunger Thirst */
   { 0, 1, 10 },
   { 3, 2, 5 },
   { 5, 2, 5 },
   { 2, 2, 5 },
   { 1, 2, 5 },
   { 6, 1, 4 },
   { 0, 1, 8 },
   { 10, 0, 0 },
   { 3, 3, 3 },
   { 0, 4, -8 },
   { 0, 3, 6 },     /* 10 */
   { 0, 1, 6 },
   { 0, 1, 6 },
   { 0, 2, -1 },
   { 0, 1, -2 },
   { 0, 0, 13 },
   { 4, 0, 4  },
   { 0, 3, 6 },
   { 0, 2, 8 }
};


const char	*color_liquid[] =
{
   "clear",
   "brown",
   "clear",
   "brown",
   "dark",
   "clear",
   "red",
   "green",
   "clear",
   "light green",
   "white",        /* 10 */
   "brown",
   "black",
   "red",
   "clear",
   "crystal clear",
   "yellowish",
   "dark",
   "orange"
};


const char	*fullness[] =
{
   "less than half ",
   "about half ",
   "more than half ",
   ""
};


const char	*genders[] =
{
   "neutral",
   "male",
   "female"
};


const struct title_type titles[20][40] = {
    /* Magic-User */
{ { "Commoner", "Commoner", 1 },
  { "Magic-apprentice", "Magic-apprentice", 2500 },
  { "Spell-student", "Spell-student", 5000 },
  { "Magic-scholar", "Magic-scholar", 10000 },
  { "Spell-delver", "Spell-delvress", 20000 },
  { "Magic-medium", "Magic-medium", 40000 },
  { "Magic-scribe", "Magic-scribress", 60000 },
  { "Seer", "Seeress", 90000 },
  { "Sage", "Sage", 135000 },
  { "Soothsayer", "Soothsayer", 250000 },
  { "Abjurer", "Abjuress", 375000 },
  { "Invoker", "Invoker", 750000 },
  { "Enchanter", "Enchantress", 1125000 },
  { "Conjurer", "Conjuress", 1500000 },
  { "Magician", "Witch", 1875000 },
  { "Creator", "Creator", 2250000 },
  { "Savant", "Savant", 2625000 },
  { "Magus", "Craftess", 3000000 },
  { "Wizard", "Wizard", 3375000 },
  { "Warlock", "War Witch", 3750000 },
  { "Sorcerer", "Sorceress", 4000000 },
  { "Necromancer", "Necromancress", 4300000 },
  { "Thaumaturge", "Thaumaturgess", 4600000 },
  { "Student of Occult", "Student of Occult", 4900000 },
  { "Disciple of Uncanny", "Disciple of Uncanny", 5200000 },
  { "Minor Elemental", "Minor Elementress", 5500000 },
  { "Greater Elemental", "Greater Elementress", 5950000 },
  { "Crafter of Magics", "Crafter of Magics", 6400000 },
  { "Shaman", "Shaman", 6850000 },
  { "Keeper of Talismans", "Keeper of Talismans", 7400000 },
  { "Archmage", "Archwitch", 8000000 },
  { "Immortal Warlock", "Immortal Enchantress", 9000000 },
  { "Avatar of Magic", "Empress of Magic", 9500000 },
  { "God of Magic", "Goddess of Magic", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},   /* End Magic-User */
     /* Cleric */
{ { "Commoner", "Commoner", 1 },
  { "Believer", "Believer", 1500 },
  { "Attendant", "Attendant", 3000 },
  { "Acolyte", "Acolyte", 6000 },
  { "Novice", "Novice", 13000 },
  { "Missionary", "Missionary", 27500 },
  { "Adept", "Adept", 55000 },
  { "Deacon", "Deaconess", 110000 },
  { "Vicar", "Vicaress", 225000 },
  { "Priest", "Priestess", 450000 },
  { "Minister", "Lady Minister", 675000 },
  { "Canon", "Canon", 900000 },
  { "Levite", "Levitess", 1125000 },
  { "Curate", "Curess", 1350000 },
  { "Churchman", "Churchwoman", 1575000 },
  { "Healer", "Healess", 1800000 },
  { "Chaplain", "Chaplain", 2100000 },
  { "Expositor", "Expositress", 2400000 },
  { "Bishop", "Bishop", 2700000 },
  { "Arch Bishop", "Arch Lady of Church", 3000000 },
  { "Patriarch", "Matriarch", 3250000 },
  { "Patriarch", "Matriarch", 3500000 },
  { "Patriarch", "Matriarch", 3800000 },
  { "Patriarch", "Matriarch", 4100000 },
  { "Patriarch", "Matriarch", 4400000 },
  { "Patriarch", "Matriarch", 4800000 },
  { "Patriarch", "Matriarch", 5200000 },
  { "Patriarch", "Matriarch", 5600000 },
  { "Patriarch", "Matriarch", 6000000 },
  { "Patriarch", "Matriarch", 6400000 },
  { "Pope", "Holy Mother", 7000000 },
  { "Immortal Cardinal", "Immortal Priestess", 9000000 },
  { "Inquisitor", "Inquisitress", 9500000 },
  { "God of good and evil", "Goddess of good and evil", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "OverLady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},  /* End Cleric */
    /* Thief  */
{ { "Commoner", "Commoner", 1 },
  { "Pilferer", "Pilferess", 1500 },
  { "Footpad", "Footpad", 3000 },
  { "Filcher", "Filcheress", 6000 },
  { "Pick-Pocket", "Pick-Pocket", 12000 },
  { "Sneak", "Sneak", 24000 },
  { "Pincher", "Pincheress", 48000 },
  { "Cut-Purse", "Cut-Purse", 96000 },
  { "Snatcher", "Snatcheress", 192000 },
  { "Sharper", "Sharpress", 384000 },
  { "Rogue", "Rogue", 600000 },
  { "Robber", "Robber", 800000 },
  { "Magsman", "Magswoman", 1000000 },
  { "Highwayman", "Highwaywoman", 1250000 },
  { "Burglar", "Burglaress", 1500000 },
  { "Thief", "Thief", 1750000 },
  { "Knifer", "Knifer", 2000000 },
  { "Shadow", "Shadow", 2250000 },
  { "Killer", "Murderess", 2500000 },
  { "Brigand", "Brigand", 2750000 },
  { "Switch-Blade", "Switch-Blade", 3100000 },
  { "Night-Blade", "Night-Blade", 3450000 },
  { "Quick-Blade", "Quick-Blade", 3800000 },
  { "Dark-Blade", "Dark-Blade", 4200000 },
  { "Silent-Blade", "Silent-Blade", 4600000 },
  { "Death-Blade", "Death-Blade", 5000000 },
  { "Master-Blade", "Master-Blade", 5400000 },
  { "Night-Killer", "Dark-Killer", 5800000 },
  { "Quick-Killer", "Quick-Killer", 6200000 },
  { "Silent-Killer", "Silent-Killer",  6600000 },
  { "Master-Killer", "Master-Killer", 7000000 },
  { "Immortal Assassin", "Immortal Assassin", 8000000 },
  { "Demi God of thieves", "Demi Goddess of thieves",9000000 },
  { "God of thieves and tradesmen", "Goddess of thieves and tradesmen", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},   /* End Thief */
     /* Warrior */
{ { "Commoner", "Commoner", 1 },
  { "Swordpupil", "Swordpupil", 2000 },
  { "Recruit", "Recruit", 4000 },
  { "Sentry", "Sentress", 8000 },
  { "Fighter", "Fighter", 16000 },
  { "Soldier", "Soldier", 32000 },
  { "Warrior", "Warrior", 64000 },
  { "Veteran", "Veteran", 125000 },
  { "Swordsman", "Swordswoman", 250000 },
  { "Fencer", "Fenceress", 500000 },
  { "Combatant", "Combatess", 750000 },
  { "Man-at-arms", "Woman-at-arms", 1000000 },
  { "Myrmidon", "Myrmidon", 1250000 },
  { "Swashbuckler", "Swashbuckleress", 1500000 },
  { "Mercenary", "Mercenaress", 1850000 },
  { "Swordmaster", "Swordmistress", 2200000 },
  { "Lieutenant", "Lieutenant", 2550000 },
  { "Champion", "Lady Champion", 2900000 },
  { "Dragoon", "Lady Dragoon", 3250000 },
  { "Sergeant", "Sergeant", 3600000 },
  { "Knight", "Lady Knight", 3900000 },
  { "Knight of Crown", "Lady Knight of Crown", 4200000 },
  { "Knight of Sword", "Lady Knight of Sword", 4500000 },
  { "Knight of Rose", "Lady Knight of Rose", 4800000 },
  { "Knight", "Lady Knight", 5150000 },
  { "Knight", "Lady Knight", 5500000 },
  { "Knight", "Lady Knight", 5950000 },
  { "Knight", "Lady Knight", 6400000 },
  { "Knight", "Lady Knight", 6850000 },
  { "Knight", "Lady Knight", 7400000 },
  { "Knight", "Lady Knight", 8000000 },
  { "Immortal Warlord", "Immortal Lady of War", 9000000 },
  { "Extirpator", "Queen of Destruction", 9500000 },
  { "God of war", "Goddess of war", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},  /* End Warrior */
    /* Psionicist */
{ { "Commoner", "Commoner", 1 },
  { "Apprentice of the Mind", "Apprentice of the Mind", 2500 },
  { "Scroll Bearer", "Scroll Bearer", 5000 },
  { "Wand Bearer", "Wand Bearer", 10000 },
  { "Seeker of Darkness", "Seeker of Darkness", 20000 },
  { "Spell Crafter", "Spell Craftess", 40000 },
  { "Minor Empath", "Minor Empath", 60000 },
  { "Seer of Thoughts", "Seeress of Thoughts", 90000 },
  { "Minor Clairvoyant", "Minor Clairvoyant", 135000 },
  { "Enchanter of Thoughts", "Enchantress of Thoughts", 250000 },
  { "Hypnotist", "Hypnotist", 375000 },
  { "Seeker of Auras", "Seeker of Auras", 750000 },
  { "Enchanter of Minds", "Enchantress of Minds", 1125000 },
  { "Conjurer of Thoughts", "Conjuress of Thoughts", 1500000 },
  { "Psychic", "Psychic", 1875000 },
  { "Telekeneticist", "Telekeneticist", 2250000 },
  { "Bringer of Dreams", "Bringer of Dreams", 2625000 },
  { "Astral Traveller", "Astral Traveller", 3000000 },
  { "Mind Master", "Mind Mistress", 3375000 },
  { "Empath", "Empath", 3750000 },
  { "Clairvoyant", "Clairvoyant", 4000000 },
  { "Controller of Thoughts", "Controller of Thoughts", 4300000 },
  { "Keeper of Portals", "Keeper of Portals", 4600000 },
  { "Keeper of the Mind's Eye", "Keeper of the Mind's Eye ", 4900000 },
  { "Disciple of Psionic Energy", "Disciple of Psionic Energy", 5200000 },
  { "Minor Etherealist", "Minor Etherialist", 5500000 },
  { "Greater Etherialist", "Greater Etherialist", 5950000 },
  { "Crafter of Dark Magic", "Crafter of Dark Magic", 6400000 },
  { "Keeper of Dreams", "Keeper of Dreams", 6850000 },
  { "Keeper of Psionic Energy", "Keeper of Psionic Energy", 7400000 },
  { "High Magus of Thought", "Archwitch of Thought", 8000000 },
  { "Immortal Psionicist", "Immortal Psionicist", 9000000 },
  { "Avatar of Psionic Energy", "Empress of Psionic Energy", 9500000 },
  { "God of the Mind", "Goddess of the Mind", 10000000 },
  { "Lord of Thought", "Lady of Thought", 15000000 },
  { "High Lord of Thought", "High Lady of Thought", 25000000 },
  { "Overlord of Psionic", "Overlady of Psionics", 30000000 },
  { "Overlord of Thought", "Overlady of Thought", 40000000 },
  { "Master of Psychic Energy", "Mistress of Psychic Energy", 40000000 },
  { "Eternal Master of the Mind", "Eternal Mistress of the Mind", 50000000}
},  /* End Psionicist */
    /* Monk */
{ { "Commoner", "Commoner", 1 },
  { "Believer", "Believer", 1500 },
  { "Attendant", "Attendant", 3000 },
  { "Acolyte", "Acolyte", 6000 },
  { "Novice", "Novice", 13000 },
  { "Missionary", "Missionary", 27500 },
  { "Adept", "Adept", 55000 },
  { "Deacon", "Deaconess", 110000 },
  { "Vicar", "Vicaress", 225000 },
  { "Priest", "Priestess", 450000 },
  { "Minister", "Lady Minister", 675000 },
  { "Canon", "Canon", 900000 },
  { "Levite", "Levitess", 1125000 },
  { "Curate", "Curess", 1350000 },
  { "Monk", "Nunne", 1575000 },
  { "Healer", "Healess", 1800000 },
  { "Chaplain", "Chaplain", 2100000 },
  { "Expositor", "Expositress", 2400000 },
  { "Bishop", "Bishop", 2700000 },
  { "Arch Bishop", "Arch Lady of Church", 3000000 },
  { "Patriarch", "Matriarch", 3250000 },
  { "Patriarch", "Matriarch", 3500000 },
  { "Patriarch", "Matriarch", 3800000 },
  { "Patriarch", "Matriarch", 4100000 },
  { "Patriarch", "Matriarch", 4400000 },
  { "Patriarch", "Matriarch", 4800000 },
  { "Patriarch", "Matriarch", 5200000 },
  { "Patriarch", "Matriarch", 5600000 },
  { "Patriarch", "Matriarch", 6000000 },
  { "Patriarch", "Matriarch", 6400000 },
  { "Patriarch", "Matriarch", 7000000 },
  { "Immortal Cardinal", "Immortal Priestess", 9000000 },
  { "Inquisitor", "Inquisitress", 9500000 },
  { "God of good and evil", "Goddess of good and evil", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
}, /* End Monk */
   /* Bard */
{ { "Commoner", "Commoner", 1 },
  { "Apprentice of Music", "Apprentice of Music", 1750 },
  { "Bearer of Music", "Bearer of Music", 3500 },
  { "Tone Deaf Musician", "Tone Deaf Musician", 7000 },
  { "Chanter", "Chanter", 14000 },
  { "Tuner of Instruments", "Tuner of Instruments", 28000 },
  { "Bearer of Mandolins", "Bearer of Mandolins", 56000 },
  { "Crafter of Rhyme", "Craftess of Rhyme", 112000 },
  { "Crafter of Music", "Craftess of Music", 224000 },
  { "Creator of Song", "Creator of Song", 400000 },
  { "Soother of Beasts", "Soother of Beasts", 650000 },
  { "Mandolin Maker", "Mandolin Maker", 900000 },
  { "Accomplished Musician", "Accomplished Musician", 1150000 },
  { "Crafter of Ballads", "Crafter of Ballads", 1400000 },
  { "Wandering Minstrel", "Wandering Minstrel", 1750000 },
  { "Keeper of Rhythm", "Keeper of Rhythm", 2000000 },
  { "Poet", "Poetess", 2350000 },
  { "Slayer of the Tone Deaf", "Slayer of the Tone Deaf", 2700000 },
  { "Street Musician", "Street Musician", 3050000 },
  { "Herald", "Heraldess", 3400000 },
  { "Master of Timbre", "Master of Timbre", 3800000 },
  { "Master of Meter", "Master of Meter", 4200000 },
  { "Master of Poets", "Mistress of Poets", 4600000 },
  { "Composer", "Composer", 5000000 },
  { "Master of Harmony", "Mistress of Harmony", 5400000 },
  { "Jongleur", "Muse", 5800000 },
  { "Conductor", "Conductor", 6200000 },
  { "High Poet", "High Poetess", 6600000 },
  { "Poet Laureate", "Poet Laureate", 7000000 },
  { "Prodigal Musician", "Prodigal Musician",  7400000 },
  { "Composer of Masterpieces", "Composer of Masterpieces", 7800000 },
  { "Grand Master of Music", "Grand Mistress of Music", 8500000 },
  { "Demi God of Musicians", "Demi Goddess of Musicians", 9000000 },
  { "God of Bards", "Goddess of Bards", 10000000 },
  { "Lord of Bards", "Lady of Bards", 15000000 },
  { "High Lord of Bards", "High Lady of Bards", 25000000 },
  { "Overlord of Bards", "Overlady of Bards", 30000000 },
  { "Overlord of Bards", "Overlady of Bards", 40000000 },
  { "God Lord of Bards", "God Lady of Bards", 40000000 },
  { "Eternal Minstrel", "Eternal Minstrel", 50000000 }
},  /* End Bard */
    /* Knight */
{ { "Commoner", "Commoner", 1 },
  { "Saddle Bearer", "Saddle Bearer", 2000 },
  { "Bringer of Horses", "Bringer of Horses", 4000 },
  { "Polisher of Armors", "Polisher of Armors", 8000 },
  { "Horsewasher", "Horsewasher", 16000 },
  { "Page", "Page", 32000 },
  { "Squire", "Squiress", 64000 },
  { "Horseman", "Horsewoman", 125000 },
  { "Sword Sharpener", "Sword Sharpener", 250000 },
  { "Sword Bearer", "Sword Bearer", 500000 },
  { "Swordsman", "Swordswoman", 750000 },
  { "Slayer of Small Animals", "Slayer of Small Animals", 1000000 },
  { "Hero", "Heroine", 1250000 },
  { "Swashbuckler", "Swashbuckleress", 1500000 },
  { "Slayer of Beasts", "Slayer of Beasts", 1850000 },
  { "Swordmaster", "Swordmistress", 2200000 },
  { "Lieutenant", "Lieutenant", 2550000 },
  { "Champion", "Lady Champion", 2900000 },
  { "Master of Squires", "Mistress of Squires", 3250000 },
  { "Knight", "Lady Knight", 3600000 },
  { "Knight", "Lady Knight", 3900000 },
  { "Knight", "Lady Knight", 4200000 },
  { "Knight", "Lady Knight", 4500000 },
  { "Knight of the Crown", "Lady Knight of the Crown", 4800000 },
  { "Knight of the Sword", "Lady Knight of the Sword", 5150000 },
  { "Knight of the Rose", "Lady Knight of the Rose", 5500000 },
  { "Slayer of Dragons", "Slayer of Dragons", 5950000 },
  { "Slayer of Dragons", "Slayer of Dragons", 6400000 },
  { "Knight of the Order", "Lady Knight of the Order", 6850000 },
  { "Knight of the Order", "Lady Knight Order", 7400000 },
  { "Knight of the High Order", "Lady Knight of the High Order", 8000000 },
  { "Immortal Knight", "Immortal Knight", 9000000 },
  { "King of Knights", "Queen of Knights", 9500000 },
  { "God of Knights", "Goddess of Knights", 10000000 },
  { "Lord of Knights", "Lady of Knights", 15000000 },
  { "High Lord of Knights", "High Lady of Knights", 25000000 },
  { "Overlord of Knights", "Overlady of Knights", 30000000 },
  { "Overlord of Knights", "Overlady of Knights", 40000000 },
  { "God Lord of Knights", "God Lady of Knights", 40000000 },
  { "Eternal Knight", "Eternal Knight", 50000000 }
},  /* End Knight */
    /* Wizard */
{ { "Commoner", "Commoner", 1 },
  { "Magic-apprentice", "Magic-apprentice", 2500 },
  { "Spell-student", "Spell-student", 5000 },
  { "Magic-scholar", "Magic-scholar", 10000 },
  { "Spell-delver", "Spell-delvress", 20000 },
  { "Magic-medium", "Magic-medium", 40000 },
  { "Magic-scribe", "Magic-scribress", 60000 },
  { "Seer", "Seeress", 90000 },
  { "Sage", "Sage", 135000 },
  { "Illusionist", "Illusionist", 250000 },
  { "Abjurer", "Abjuress", 375000 },
  { "Invoker", "Invoker", 750000 },
  { "Enchanter", "Enchantress", 1125000 },
  { "Conjurer", "Conjuress", 1500000 },
  { "Magician", "Witch", 1875000 },
  { "Creator", "Creator", 2250000 },
  { "Savant", "Savant", 2625000 },
  { "Magus", "Craftess", 3000000 },
  { "Wizard", "Wizard", 3375000 },
  { "Warlock", "War Witch", 3750000 },
  { "Sorcerer", "Sorceress", 4000000 },
  { "Necromancer", "Necromancress", 4300000 },
  { "Thaumaturge", "Thaumaturgess", 4600000 },
  { "Student of Occult", "Student of Occult", 4900000 },
  { "Disciple of Uncanny", "Disciple of Uncanny", 5200000 },
  { "Minor Elemental", "Minor Elementress", 5500000 },
  { "Greater Elemental", "Greater Elementress", 5950000 },
  { "Crafter of Magics", "Crafter of Magics", 6400000 },
  { "Shaman", "Shaman", 6850000 },
  { "Keeper of Talismans", "Keeper of Talismans", 7400000 },
  { "Archmage", "Archwitch", 8000000 },
  { "Immortal Warlock", "Immortal Enchantress", 9000000 },
  { "Avatar of Magic", "Empress of Magic", 9500000 },
  { "God of Magic", "Goddess of Magic", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000      }
},    /* End Wizard */
      /* Druid */
{ {"Dryw", "Dryw", 1 },
  {"Beth", "Beth", 1500 },
  {"Luis", "Luis", 3000 },
  {"Fearn", "Fearn", 6000 },
  {"Saille", "Saille", 13000 },
  {"Huathe", "Huathe", 27500 },
  {"Duir", "Duir", 55000 },
  {"Tinne", "Tinne", 110000 },
  {"Coll", "Coll", 225000 },
  {"Quert", "Quert", 450000 },
  {"Muin", "Muin", 675000 },
  {"Gort", "Gort", 900000 },
  {"Ngetal", "Ngetal", 1125000 },
  {"Straif", "Straif", 1350000 },
  {"Ruis", "Ruis", 1575000 },
  {"Ailim", "Ailim", 1800000 },
  {"Ohn", "Ohn", 2100000 },
  {"Ur", "Ur", 2400000 },
  {"Eadha", "Eadha", 2700000 },
  {"Ioho", "Ioho", 3000000 },
  {"Koad", "Koad", 3250000 },
  {"Oir", "Oir", 3500000 },
  {"Uilleand", "Uilleand", 3800000 },
  {"Phagos", "Phagos", 4100000 },
  {"Mor", "Mor", 4400000 },
  {"Alban Arthuan", "Alban Arthuan", 4800000 },
  {"Alban Eiler", "Alban Eiler", 5200000 },
  {"Alban Elved", "Alban Elved", 5600000 },
  {"Alban Heruin", "Alban Heruin", 6000000 },
  {"Glainnaider", "Glainnaider", 6400000 },
  {"Nadredd", "Nadredd", 7000000 },
  {"Imbas Forosnai", "Imbas Forosnai", 9000000 },
  {"Immrama", "Immrama", 9500000 },
  {"Eochra Ecsi", "Eochra Ecsi", 10000000 },
  {"Tienm Laida", "Tienm Laida", 15000000 },
  {"Ategenos", "Ategenos", 25000000 },
  {"Awen", "Awen", 30000000 },
  {"Awenyddion", "Awenyddion", 40000000 },
  {"Awenyddion", "Awenyddion", 40000000 },
  {"Implementor", "Implementress", 50000000 }
},  /* End Druid */
    /* Assassin */
{ { "Commoner", "Commoner", 1 },
  { "Pilferer", "Pilferess", 2000 },
  { "Footpad", "Footpad", 4000 },
  { "Filcher", "Filcheress", 8000 },
  { "Pick-Pocket", "Pick-Pocket", 16000 },
  { "Sneak", "Sneak", 32000 },
  { "Pincher", "Pincheress", 64000 },
  { "Cut-Purse", "Cut-Purse", 125000 },
  { "Snatcher", "Snatcheress", 250000 },
  { "Sharper", "Sharpress", 500000 },
  { "Rogue", "Rogue", 750000 },
  { "Robber", "Robber", 1000000 },
  { "Magsman", "Magswoman", 1250000 },
  { "Highwayman", "Highwaywoman", 1500000 },
  { "Burglar", "Burglaress", 1850000 },
  { "Thief", "Thief", 2200000 },
  { "Knifer", "Knifer", 2550000 },
  { "Shadow", "Shadow", 2900000 },
  { "Killer", "Murderess", 3250000 },
  { "Brigand", "Brigand", 3600000 },
  { "Switch-Blade", "Switch-Blade", 4000000 },
  { "Night-Blade", "Night-Blade", 4400000 },
  { "Quick-Blade", "Quick-Blade", 4800000 },
  { "Dark-Blade", "Dark-Blade", 5200000 },
  { "Silent-Blade", "Silent-Blade", 5600000 },
  { "Death-Blade", "Death-Blade", 6000000 },
  { "Master-Blade", "Master-Blade", 6400000 },
  { "Night-Killer", "Dark-Killer", 6800000 },
  { "Quick-Killer", "Quick-Killer", 7200000 },
  { "Silent-Killer", "Silent-Killer", 7600000 },
  { "Master-Killer", "Master-Killer", 8000000 },
  { "Immortal Assassin", "Immortal Assassin", 9000000 },
  { "Demi God of thieves", "Demi Goddess of thieves",9500000 },
  { "God of thieves and tradesmen", "Goddess of thieves and tradesmen", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},   /* End Assassin */
     /* Ranger */
{ { "Commoner", "Commoner", 1 },
  { "Swordpupil", "Swordpupil", 2000 },
  { "Recruit", "Recruit", 4000 },
  { "Sentry", "Sentress", 8000 },
  { "Fighter", "Fighter", 16000 },
  { "Soldier", "Soldier", 32000 },
  { "Warrior", "Warrior", 64000 },
  { "Veteran", "Veteran", 125000 },
  { "Swordsman", "Swordswoman", 250000 },
  { "Fencer", "Fenceress", 500000 },
  { "Combatant", "Combatess", 750000 },
  { "Hero", "Heroine", 1000000 },
  { "Myrmidon", "Myrmidon", 1250000 },
  { "Swashbuckler", "Swashbuckleress", 1500000 },
  { "Mercenary", "Mercenaress", 1850000 },
  { "Swordmaster", "Swordmistress", 2200000 },
  { "Lieutenant", "Lieutenant", 2550000 },
  { "Champion", "Lady Champion", 2900000 },
  { "Dragoon", "Lady Dragoon", 3250000 },
  { "Cavalier", "Cavalier", 3600000 },
  { "Knight", "Lady Knight", 3900000 },
  { "Knight of Crown", "Lady Knight of Crown", 4200000 },
  { "Knight of Sword", "Lady Knight of Sword", 4500000 },
  { "Knight of Rose", "Lady Knight of Rose", 4800000 },
  { "Knight", "Lady Knight", 5150000 },
  { "Knight", "Lady Knight", 5500000 },
  { "Knight", "Lady Knight", 5950000 },
  { "Knight", "Lady Knight", 6400000 },
  { "Knight", "Lady Knight", 6850000 },
  { "Knight", "Lady Knight", 7400000 },
  { "Knight", "Lady Knight", 8000000 },
  { "Immortal Warlord", "Immortal Lady of War", 9000000 },
  { "Extirpator", "Queen of Destruction", 9500000 },
  { "God of war", "Goddess of war", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},   /* End Ranger */
     /* Illusionist */
{ { "Commoner", "Commoner", 1 },
  { "Magic-apprentice", "Magic-apprentice", 2500 },
  { "Spell-student", "Spell-student", 5000 },
  { "Magic-scholar", "Magic-scholar", 10000 },
  { "Spell-delver", "Spell-delvress", 20000 },
  { "Magic-medium", "Magic-medium", 40000 },
  { "Magic-scribe", "Magic-scribress", 60000 },
  { "Seer", "Seeress", 90000 },
  { "Sage", "Sage", 135000 },
  { "Illusionist", "Illusionist", 250000 },
  { "Abjurer", "Abjuress", 375000 },
  { "Invoker", "Invoker", 750000 },
  { "Enchanter", "Enchantress", 1125000 },
  { "Conjurer", "Conjuress", 1500000 },
  { "Magician", "Witch", 1875000 },
  { "Creator", "Creator", 2250000 },
  { "Savant", "Savant", 2625000 },
  { "Magus", "Craftess", 3000000 },
  { "Wizard", "Wizard", 3375000 },
  { "Warlock", "War Witch", 3750000 },
  { "Sorcerer", "Sorceress", 4000000 },
  { "Necromancer", "Necromancress", 4300000 },
  { "Thaumaturge", "Thaumaturgess", 4600000 },
  { "Student of Occult", "Student of Occult", 4900000 },
  { "Disciple of Uncanny", "Disciple of Uncanny", 5200000 },
  { "Minor Elemental", "Minor Elementress", 5500000 },
  { "Greater Elemental", "Greater Elementress", 5950000 },
  { "Crafter of Magics", "Crafter of Magics", 6400000 },
  { "Shaman", "Shaman", 6850000 },
  { "Keeper of Talismans", "Keeper of Talismans", 7400000 },
  { "Archmage", "Archwitch", 8000000 },
  { "Immortal Warlock", "Immortal Enchantress", 9000000 },
  { "Avatar of Magic", "Empress of Magic", 9500000 },
  { "God of Magic", "Goddess of Magic", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000      }
},   /* End Illusionist */
     /* Paladin */
{ { "Commoner", "Commoner", 1 },
  { "Swordpupil", "Swordpupil", 2000 },
  { "Recruit", "Recruit", 4000 },
  { "Sentry", "Sentress", 8000 },
  { "Fighter", "Fighter", 16000 },
  { "Soldier", "Soldier", 32000 },
  { "Warrior", "Warrior", 64000 },
  { "Veteran", "Veteran", 125000 },
  { "Swordsman", "Swordswoman", 250000 },
  { "Fencer", "Fenceress", 500000 },
  { "Combatant", "Combatess", 750000 },
  { "Hero", "Heroine", 1000000 },
  { "Myrmidon", "Myrmidon", 1250000 },
  { "Swashbuckler", "Swashbuckleress", 1500000 },
  { "Mercenary", "Mercenaress", 1850000 },
  { "Swordmaster", "Swordmistress", 2200000 },
  { "Lieutenant", "Lieutenant", 2550000 },
  { "Champion", "Lady Champion", 2900000 },
  { "Dragoon", "Lady Dragoon", 3250000 },
  { "Cavalier", "Cavalier", 3600000 },
  { "Knight", "Lady Knight", 3900000 },
  { "Knight of Crown", "Lady Knight of Crown", 4200000 },
  { "Knight of Sword", "Lady Knight of Sword", 4500000 },
  { "Knight of Rose", "Lady Knight of Rose", 4800000 },
  { "Knight", "Lady Knight", 5150000 },
  { "Knight", "Lady Knight", 5500000 },
  { "Knight", "Lady Knight", 5950000 },
  { "Knight", "Lady Knight", 6400000 },
  { "Knight", "Lady Knight", 6850000 },
  { "Knight", "Lady Knight", 7400000 },
  { "Knight", "Lady Knight", 8000000 },
  { "Immortal Warlord", "Immortal Lady of War", 9000000 },
  { "Extirpator", "Queen of Destruction", 9500000 },
  { "God of war", "Goddess of war", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
},  /* End Paladin */
    /* Mariner */
{ { "Commoner", "Commoner", 1 },
  { "Dry-foot", "Dry-foot", 1750 },
  { "Bilge Water", "Bilge Water", 3500 },
  { "Head Cleaner", "Head Cleaner", 7000 },
  { "Fish Gutter", "Fish Gutter", 14000 },
  { "Barnacle Scraper", "Barnacle Scraper", 28000 },
  { "Apprentice of Knots", "Apprentice of Knots", 56000 },
  { "Docksman", "Dockswoman", 112000 },
  { "Galleyman", "Galleywoman", 224000 },
  { "Seasick Mate", "Seasick Mate", 400000 },
  { "Mate", "Mate", 650000 },
  { "Seaworthy Mate", "Seaworthy Mate", 900000 },
  { "Oarsman", "Oarswoman", 1150000 },
  { "Apprentice Helmsman", "Apprentice Helmswoman", 1400000 },
  { "Seafarer", "Seafarer", 1750000 },
  { "Keeper of Life Preservers", "Keeper of Life Preservers", 2000000 },
  { "Tackle Bearer", "Tackle Bearer", 2350000 },
  { "Knotsman", "Knotswoman", 2700000 },
  { "Master of Knots", "Mistress of Knots", 3050000 },
  { "Helmsman", "Helmswoman", 3400000 },
  { "Salty Mate", "Salty Mate", 3800000 },
  { "Salty Seafarer", "Salty Seafarer", 4200000 },
  { "Apprentice of Piracy", "Apprentice of Piracy", 4600000 },
  { "Pirate", "Pirate", 5000000 },
  { "Second Mate", "Second Mate", 5400000 },
  { "First Mate", "First Mate", 5800000 },
  { "Dread Pirate", "Dread Pirate", 6200000 },
  { "Shipmaster", "Shipmistress", 6600000 },
  { "Dockmaster", "Dockmistress", 7000000 },
  { "Ship's Captain", "Ship's Captain",  7400000 },
  { "Admiral", "Admiral", 7800000 },
  { "Immortal Seafarer", "Immortal Seafarer", 8500000 },
  { "Demi God of Mariners", "Demi Goddess of Mariners", 9000000 },
  { "God of Mariners", "Goddess of Mariners", 10000000 },
  { "Master of Pirates", "Mistress of Pirates", 15000000 },
  { "High Lord of Mariners", "High Lady of Mariners", 25000000 },
  { "Overlord of Pirates", "Overlady of Pirates", 30000000 },
  { "Overlord of Mariners", "Overlady Mariners", 40000000 },
  { "God Lord of Seafarers", "God Lady of Seafarers", 40000000 },
  { "Eternal Mariner", "Eternal Mariner", 50000000 }
},  /* End Mariner */
    /* Cavalier */
{ { "Commoner", "Commoner", 1 },
  { "Swordpupil", "Swordpupil", 2000 },
  { "Recruit", "Recruit", 4000 },
  { "Sentry", "Sentress", 8000 },
  { "Fighter", "Fighter", 16000 },
  { "Soldier", "Soldier", 32000 },
  { "Warrior", "Warrior", 64000 },
  { "Veteran", "Veteran", 125000 },
  { "Swordsman", "Swordswoman", 250000 },
  { "Fencer", "Fenceress", 500000 },
  { "Combatant", "Combatess", 750000 },
  { "Hero", "Heroine", 1000000 },
  { "Myrmidon", "Myrmidon", 1250000 },
  { "Swashbuckler", "Swashbuckleress", 1500000 },
  { "Mercenary", "Mercenaress", 1850000 },
  { "Swordmaster", "Swordmistress", 2200000 },
  { "Lieutenant", "Lieutenant", 2550000 },
  { "Champion", "Lady Champion", 2900000 },
  { "Dragoon", "Lady Dragoon", 3250000 },
  { "Cavalier", "Cavalier", 3600000 },
  { "Knight", "Lady Knight", 3900000 },
  { "Knight of Crown", "Lady Knight of Crown", 4200000 },
  { "Knight of Sword", "Lady Knight of Sword", 4500000 },
  { "Knight of Rose", "Lady Knight of Rose", 4800000 },
  { "Knight", "Lady Knight", 5150000 },
  { "Knight", "Lady Knight", 5500000 },
  { "Knight", "Lady Knight", 5950000 },
  { "Knight", "Lady Knight", 6400000 },
  { "Knight", "Lady Knight", 6850000 },
  { "Knight", "Lady Knight", 7400000 },
  { "Knight", "Lady Knight", 8000000 },
  { "Immortal Warlord", "Immortal Lady of War", 9000000 },
  { "Extirpator", "Queen of Destruction", 9500000 },
  { "God of war", "Goddess of war", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
}, /* End Cavalier */
    /* Unused MU*/
{ { "Commoner", "Commoner", 1 },
  { "Magic-apprentice", "Magic-apprentice", 2500 },
  { "Spell-student", "Spell-student", 5000 },
  { "Magic-scholar", "Magic-scholar", 10000 },
  { "Spell-delver", "Spell-delvress", 20000 },
  { "Magic-medium", "Magic-medium", 40000 },
  { "Magic-scribe", "Magic-scribress", 60000 },
  { "Seer", "Seeress", 90000 },
  { "Sage", "Sage", 135000 },
  { "Soothsayer", "Soothsayer", 250000 },
  { "Abjurer", "Abjuress", 375000 },
  { "Invoker", "Invoker", 750000 },
  { "Enchanter", "Enchantress", 1125000 },
  { "Conjurer", "Conjuress", 1500000 },
  { "Magician", "Witch", 1875000 },
  { "Creator", "Creator", 2250000 },
  { "Savant", "Savant", 2625000 },
  { "Magus", "Craftess", 3000000 },
  { "Wizard", "Wizard", 3375000 },
  { "Warlock", "War Witch", 3750000 },
  { "Sorcerer", "Sorceress", 4000000 },
  { "Necromancer", "Necromancress", 4300000 },
  { "Thaumaturge", "Thaumaturgess", 4600000 },
  { "Student of Occult", "Student of Occult", 4900000 },
  { "Disciple of Uncanny", "Disciple of Uncanny", 5200000 },
  { "Minor Elemental", "Minor Elementress", 5500000 },
  { "Greater Elemental", "Greater Elementress", 5950000 },
  { "Crafter of Magics", "Crafter of Magics", 6400000 },
  { "Shaman", "Shaman", 6850000 },
  { "Keeper of Talismans", "Keeper of Talismans", 7400000 },
  { "Archmage", "Archwitch", 8000000 },
  { "Immortal Warlock", "Immortal Enchantress", 9000000 },
  { "Avatar of Magic", "Empress of Magic", 9500000 },
  { "God of Magic", "Goddess of Magic", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "Overlady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
}, /* End Unused MU */
    /* Unused CL */
{ { "Commoner", "Commoner", 1 },
  { "Believer", "Believer", 1500 },
  { "Attendant", "Attendant", 3000 },
  { "Acolyte", "Acolyte", 6000 },
  { "Novice", "Novice", 13000 },
  { "Missionary", "Missionary", 27500 },
  { "Adept", "Adept", 55000 },
  { "Deacon", "Deaconess", 110000 },
  { "Vicar", "Vicaress", 225000 },
  { "Priest", "Priestess", 450000 },
  { "Minister", "Lady Minister", 675000 },
  { "Canon", "Canon", 900000 },
  { "Levite", "Levitess", 1125000 },
  { "Curate", "Curess", 1350000 },
  { "Churchman", "Churchwoman", 1575000 },
  { "Healer", "Healess", 1800000 },
  { "Chaplain", "Chaplain", 2100000 },
  { "Expositor", "Expositress", 2400000 },
  { "Bishop", "Bishop", 2700000 },
  { "Arch Bishop", "Arch Lady of Church", 3000000 },
  { "Patriarch", "Matriarch", 3250000 },
  { "Patriarch", "Matriarch", 3500000 },
  { "Patriarch", "Matriarch", 3800000 },
  { "Patriarch", "Matriarch", 4100000 },
  { "Patriarch", "Matriarch", 4400000 },
  { "Patriarch", "Matriarch", 4800000 },
  { "Patriarch", "Matriarch", 5200000 },
  { "Patriarch", "Matriarch", 5600000 },
  { "Patriarch", "Matriarch", 6000000 },
  { "Patriarch", "Matriarch", 6400000 },
  { "Pope", "Holy Mother", 7000000 },
  { "Immortal Cardinal", "Immortal Priestess", 9000000 },
  { "Inquisitor", "Inquisitress", 9500000 },
  { "God of good and evil", "Goddess of good and evil", 10000000 },
  { "Lord", "Lady", 15000000 },
  { "High Lord", "High Lady", 25000000 },
  { "Overlord", "OverLady", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 40000000 },
  { "Implementor", "Implementress", 50000000 }
}, /* End Unused CL */
    /* Ninja */
{ { "Commoner", "Commoner", 1},
  { "Kyoushuusei", "Kyoushuusei", 1500 },
  { "Daigensha", "Daigensha", 3000 },
  { "Sekkou", "Sekkou", 6000 },
  { "Kenkaku", "Kenkaku", 12000 },
  { "Youhei", "Youhei", 24000 },
  { "Ronin", "Ronin", 48000 },
  { "Hikyuu", "Hikyuu", 96000 },
  { "Kishi", "Kishi", 192000 },
  { "Gounomono", "Gounomono", 384000 },
  { "Keisotsu", "Keisotsu", 600000 },
  { "Joufu", "Joufu", 800000 },
  { "Koroshiya", "Koroshiya", 1000000 },
  { "Kagemusha", "Kagemusha", 1250000 },
  { "Toorima", "Toorima", 1500000 },
  { "Shoubu", "Shoubu", 1750000 },
  { "Deshi", "Deshi", 2000000 },
  { "Hachikyu", "Hachikyu", 2250000 },
  { "Shichikyu", "Shichikyu", 2500000 },
  { "Rokkyu", "Rokkyu", 2750000 },
  { "Gokyu", "Gokyu", 3100000 },
  { "Chikkyu", "Chikkyu", 3450000 },
  { "Sankyu", "Sankyu", 3800000 },
  { "Nikkyu", "Nikkyu", 4200000 },
  { "Ikkyu", "Ikkyu", 4600000 },
  { "Shodan", "Shodan", 5000000 },
  { "Shinobi", "Kunoichi", 5400000 },
  { "Genin", "Genin", 5800000 },
  { "Chunin", "Chunin", 6200000 },
  { "Jonin", "Jonin", 6600000 },
  { "Sensei", "Sensei", 7000000 },
  { "Immortal Shinobi", "Immortal Kunoichi", 8000000 },
  { "Demigod of Ninjitsu", "Demigoddess of Ninjitsu", 9000000 },
  { "God of Ninjitsu", "Goddess of Ninjitsu", 10000000 },
  { "Lord of Ninjitsu", "Lady of Ninjitsu", 15000000 },
  { "High Lord of Ninjitsu", "High Lady of Ninjitsu", 25000000 },
  { "Shu-gi", "Shu-gi", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 50000000 }
}, /* End Ninja */
    /* Unused WA */
{ { "Commoner", "Commoner", 1},
  { "Kyoushuusei", "Kyoushuusei", 2000 },
  { "Daigensha", "Daigensha", 4000 },
  { "Sekkou", "Sekkou", 8000 },
  { "Kenkaku", "Kenkaku", 16000 },
  { "Youhei", "Youhei", 32000 },
  { "Ronin", "Ronin", 64000 },
  { "Hikyuu", "Hikyuu", 125000 },
  { "Kishi", "Kishi", 250000 },
  { "Gounomono", "Gounomono", 500000 },
  { "Keisotsu", "Keisotsu", 750000 },
  { "Joufu", "Joufu", 1000000 },
  { "Koroshiya", "Koroshiya", 1250000 },
  { "Kagemusha", "Kagemusha", 1500000 },
  { "Toorima", "Toorima", 1850000 },
  { "Shoubu", "Shoubu", 2200000 },
  { "Deshi", "Deshi", 2550000 },
  { "Hachikyu", "Hachikyu", 2900000 },
  { "Shichikyu", "Shichikyu", 3250000 },
  { "Rokkyu", "Rokkyu", 3600000 },
  { "Gokyu", "Gokyu", 3900000 },
  { "Chikkyu", "Chikkyu", 4200000 },
  { "Sankyu", "Sankyu", 4500000 },
  { "Nikkyu", "Nikkyu", 4800000 },
  { "Ikkyu", "Ikkyu", 5150000 },
  { "Shodan", "Shodan", 5500000 },
  { "Shinobi", "Kunoichi", 5950000 },
  { "Genin", "Genin", 6400000 },
  { "Chunin", "Chunin", 6850000 },
  { "Jonin", "Jonin", 7400000 },
  { "Sensei", "Sensei", 8000000 },
  { "Immortal Shinobi", "Immortal Kunoichi", 9000000 },
  { "Demigod of Ninjutsu", "Demigoddess of Ninjutsu", 9500000 },
  { "God of Ninjutsu", "Goddess of Ninjutsu", 10000000 },
  { "Lord of Ninjustu", "Lady of Ninjustu", 15000000 },
  { "High Lord of Ninjutsu", "High Lady of Ninjutsu", 25000000 },
  { "Shu-gi", "Shu-gi", 30000000 },
  { "Overlord", "Overlady", 40000000 },
  { "Overlord", "Overlady", 40000000 },
  { "God Lord", "God Lady", 50000000 }
} /* End Unused WA */
};


const char	*item_types[] = {
   "UNDEFINED",
   "LIGHT",
   "SCROLL",
   "WAND",
   "STAFF",
   "WEAPON",
   "FIRE WEAPON",
   "MISSILE",
   "TREASURE",
   "ARMOR",
   "POTION",
   "WORN",
   "OTHER",
   "TRASH",
   "TRAP",
   "CONTAINER",
   "NOTE",
   "LIQUID CONTAINER",
   "KEY",
   "FOOD",
   "MONEY",
   "PEN",
   "BOAT",
   "FOUNTAIN",
   "BOMB",
   "RAWFOOD",
   "PORTAL",
   "BOARD",
   "\n"
};


const char	*wear_bits[] = {
   "TAKE",
   "FINGER",
   "NECK",
   "BODY",
   "HEAD",
   "LEGS",
   "FEET",
   "HANDS",
   "ARMS",
   "SHIELD",
   "ABOUT",
   "WAIST",
   "WRIST",
   "WIELD",
   "HOLD",
   "THROW",
   "WIELD-2H",
   "\n"
};


const char	*extra_bits[] = {
   "GLOW",
   "HUM",
   "DARK",
   "LOCK",
   "EVIL",
   "INVISIBLE",
   "MAGIC",
   "NODROP",
   "BLESS",
   "ANTI-GOOD",
   "ANTI-EVIL",
   "ANTI-NEUTRAL",
   "NORENT",
   "NODONATE",
   "NOINVIS",
   "HIDDEN",
   "BROKEN",
   "CHAOTIC",
   "ARENA",
   "DONATED",
   "FLAMING",
   "NOLOCATE",
   "NOBREAK",
   "NOREMOVE",
   "QUEST",
   "NOSWEEP",
   "KILLER",
   "\n"
};

const char      *portal_bits[] = {
  "CLOSED",
  "LOCKED",
  "RANDOM",
  "EFFECT",
  "\n"
};



const char	*room_bits[] = {
   "DARK",
   "DEATH",
   "!MOB",
   "INDOORS",
   "LAWFUL",
   "NEUTRAL",
   "CHAOTIC",
   "!MAGIC",
   "TUNNEL",
   "PRIVATE",
   "GODROOM",
   "*",
   "0-MANA",
   "DISPELL",
   "SILENT",
   "IN THE AIR",
   "OCS",
   "PKOK",
   "ARENA",
   "REGEN",
   "!TELEPORT",
   "!SCRY",
   "!FLEE",
   "DAMAGE",
   "!TRACK",
   "!SWEEP",
   "!SCOUT",
   "!SLEEP",
   "!SUMMON",
   "!QUIT",
   "!DROP",
   "\n"
};


const char	*exit_bits[] = {
   "DOOR",
   "CLOSED",
   "LOCKED",
   "RSCLOSED",
   "RSLOCKED",
   "PICKPROOF",
   "TRAP",
   "WALL",
   "BASHPROOF",
   "MAGICPROOF",
   "PASSPROOF",
   "TRAPSET",
   "SECRET",
   "BROKEN",
   "\n"
};


const char	*sector_types[] = {
   "Inside",
   "City",
   "Field",
   "Forest",
   "Hills",
   "Mountains",
   "Water Swim",
   "Water NoSwim",
   "Underwater",
   "Air",
   "Void",
   "Desert",
   "Frozen Waste",
   "Frozen Mountain",
   "Icy Underwater",
   "Frozen Water NoSwim",
   "\n"
};


const char	*equipment_types[] = {
   "Special",
   "Worn on right finger",
   "Worn on left finger",
   "First worn around Neck",
   "Second worn around Neck",
   "Worn on body",
   "Worn on head",
   "Worn on legs",
   "Worn on feet",
   "Worn on hands",
   "Worn on arms",
   "Worn as shield",
   "Worn about body",
   "Worn around waiste",
   "Worn around right wrist",
   "Worn around left wrist",
   "Wielded",
   "Held",
   "\n"
};



const char	*affected_bits[] =
{
   "BLIND",
   "INVIS",
   "DET-ALIGN",
   "DET-INV",
   "DET-MAGIC",
   "SENSE-LIFE",
   "RIGHTEOUS",
   "SANC",
   "GROUP",
   "CURSE",
   "LIGHT",
   "POISON",
   "PROT-EVIL",
   "!USED",
   "!USED",
   "!USED",
   "SLEEP",
   "DODGE",
   "SNEAK",
   "HIDE",
   "!USED",
   "CHARM",
   "!USED",
   "!USED",
   "INFRA",
   "BERZ",
   "HOVER",
   "FLY",
   "WATERBREATH",
   "REGEN",
   "!USED",
   "\n"
};

const char *affected_bits2[] =
{
  "TEST",
  "\n"
};

const char	*item_affects[] =
{
   "blindness",
   "invisibility",
   "detect alignment",
   "detect invisibility",
   "detect magic",
   "sense life",
   "ERROR: Please Report",
   "sanctuary",
   "ERROR: Please Report",
   "curse",
   "cat eyes",
   "poison",
   "protection from evil",
   "ERROR: Please Report",
   "ERROR: Please Report",
   "ERROR: Please Report",
   "sleep",
   "ERROR: Please Report",
   "sneak",
   "hide",
   "ERROR: Please Report",
   "charm person",
   "ERROR: Please Report",
   "ERROR: Please Report",
   "infravision",
   "berzerk",
   "levitation",
   "fly",
   "water breathing",
   "regeneration",
   "ERROR: Please Report",
   "\n"
};

const char *imm_powers[] =
{
  "BASIC",
  "STANDARD",
  "ADMIN",
  "SHUTDOWN",
  "REMORT",
  "QUEST",
  "LOAD",
  "CLAN",
  "WORLD",
  "WARGAME",
  "QUESTOR",
  "OVERSEER",
  "CODE",
  "HELP",
  "NEWS",
  "PK",
  "CODER",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "#rALL#N",
  "\n"
};


const char	*apply_types[] = {
   "NONE",
   "STR",
   "DEX",
   "INT",
   "WIS",
   "CON",
   "CHA",
   "CLASS",
   "LEVEL",
   "AGE",
   "CHAR_WEIGHT",
   "CHAR_HEIGHT",
   "MANA",
   "HIT",
   "MOVE",
   "GOLD",
   "EXP",
   "ARMOR",
   "HITROLL",
   "DAMROLL",
   "SAVING_PHYSICAL",
   "SAVING_MENTAL",
   "SAVING_MAGIC",
   "SAVING_POISON",
   "MAGIC_RESISTANCE",
   "JUMP",
   "STEAL",
   "SNEAK",
   "TRACK",
   "ARCHERY",
   "THROW",
   "SWIM",
   "DIVE",
   "BV2",
   "\n"
};


const char	*pc_class_types[] = {
   "UNDEFINED",
   "magic-user",
   "cleric",
   "thief",
   "warrior",
   "psionicist",
   "monk",
   "bard",
   "knight",
   "wizard",
   "druid",
   "assassin",
   "ranger",
   "illusionist",
   "paladin",
   "mariner",
   "cavalier",
   "DO_NOT_USE",
   "DO_NOT_USE",
   "ninja",
   "DO_NOT_USE",
   "dual-class",
   "2-multi-class",
   "3-multi-class",
   "\n"
};


const char      *npc_types[] = {
    "Normal",
    "Humanoid",
    "Undead",
    "Catbeast",
    "Hound",
    "Bearbeast",
    "Bird",
    "Mount",
    "Giant",
    "Dwarf",
    "Illusion",
    "Flying Mount",
    "Demon",
    "Flying Beast",
    "Fire Elemental",
    "Water Elemental",
    "Earth Elemental",
    "Air Elemental",
    "Dragon",
    "Insect",
    "\n"
};

/* terminate each npc-race with { 0, 0 } */
const struct resistance npc_resistance_table[]  =  {
/* Normal */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Humanoid */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Undead */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, { TYPE_PIERCE, 90}, {0, 0},
/* Catbeast */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Hound */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Bearbeast */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Bird */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Mount */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Giant */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Dwarf */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Illusion */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Flying Mount */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Demon */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, { SPELL_FEAR, 100 }, {0, 0},
/* Flying Beast */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Fire Elemental */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, { SPELL_FIREBALL, 95 }, {0, 0},
/* Water Elemental */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, { SPELL_FIREBALL, -95 }, {0, 0},
/* Earth Elemental */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Air Elemental */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/* Dragon */
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},
/*insect*/
{SPELL_SUMMON, LVL3}, {SPELL_CHARM_PERSON, LVL2}, {SPELL_SLEEP, LVL}, {0, 0},

{-1, 0}   /* end with this */

};


const char	*npc_class_types[] = {
   "Normal",
   "Magic-user",
   "Cleric",
   "Thief",
   "Warrior",
   "\n"
};


const struct mskill npc_skill_table[]  =  {
/* Normal */
{SKILL_2ATTACK, LVL3}, {SKILL_3ATTACK, LVL2}, {SKILL_4ATTACK, LVL},
{SKILL_DODGE, LVL2}, {SKILL_PARRY, LVL}, {SKILL_TUMBLE, LVL},
{SKILL_MARTIAL_ARTS, LVL}, {SKILL_SCOUT, 95}, {SKILL_TRACK, 95}, {0, 0},
/* Magic-user */
{SKILL_2ATTACK, LVL3}, {SKILL_3ATTACK, LVL2}, {SKILL_4ATTACK, LVL},
{SKILL_DODGE, LVL2}, {SKILL_PARRY, LVL}, {SKILL_TUMBLE, LVL},
{SKILL_MARTIAL_ARTS, LVL}, {SKILL_SCOUT, 95}, {SKILL_TRACK, 95}, {0, 0},
/* Cleric */
{SKILL_2ATTACK, LVL3}, {SKILL_3ATTACK, LVL2}, {SKILL_4ATTACK, LVL},
{SKILL_DODGE, LVL2}, {SKILL_PARRY, LVL}, {SKILL_TUMBLE, LVL},
{SKILL_MARTIAL_ARTS, LVL}, {SKILL_SCOUT, 95}, {SKILL_TRACK, 95}, {0, 0},
/* Thief */
{SKILL_2ATTACK, LVL}, {SKILL_TRACK, 95}, {SKILL_SCOUT, 95},
{SKILL_CIRCLE_AROUND, LVL},  {0, 0},
/* Warrior */
{SKILL_2ATTACK, LVL3}, {SKILL_3ATTACK, LVL2}, {SKILL_4ATTACK, LVL},
{SKILL_KICK, 60}, {SKILL_BASH, 60}, {SKILL_DISARM, 75},
{SKILL_SCOUT, 95}, {SKILL_TRACK, 95}, {0, 0},

{ -1, 0}  /* end with this */
};


const char      *clan_ranks[][3] = {
{"Non-Member", "Non-Member", "Non-Member"},
{"Applying", "Applying", "Applying"},
{"New Member", "New Member", "New Member"},
{"Member", "Member", "Member"},
{"Soldier", "Soldier", "Soldier"},
{"Adept", "Adept", "Adept"},
{"Advisor", "Advisor", "Advisor"},
{"Ruler", "Ruler", "Ruler"},
{"King", "King", "Queen"},
{"Deity", "Deity", "Deity"},
{"Overgod", "Overgod", "Overgoddess"},
{"\n", "\n", "\n"}
};


const char      *quest_bits[] = {
   "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13",
   "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25",
   "26", "27", "28", "29", "30", "31", "32", "\n"
};

const char	*action_bits[] = {
   "SPEC",
   "SENTINEL",
   "SCAVENGER",
   "ISNPC",
   "NICE-THIEF",
   "AGGR",
   "STAY-ZONE",
   "WIMPY",
   "AGG_EVIL",
   "AGG_GOOD",
   "AGG_NEUTRAL",
   "MEMORY",
   "HELPER",
   "SWITCHED",
   "BLINDER",
   "HIDDEN",
   "!TRACK",
   "\n"
};


const char	*player_bits[] = {
   "KILLER",
   "THIEF",
   "FROZEN",
   "DONTSET",
   "WRITING",
   "MAILING",
   "SOBJS",
   "SITEOK",
   "MUTE",
   "NOTITLE",
   "DELETED",
   "LOADRM",
   "NO-WIZL",
   "NO-DEL",
   "INVST",
   "CRYO",
   "ARENA",
   "!CLNTTL",
   "!CLANTLL",
   "WRAP",
   "", /* LOG */
   "SCHR",
   "SALS",
   "SSTR",
   "AUTO_ASSIST",
   "NOWHO",
   "T1",
   "T2",
   "PKOK",
   "SIGNO",
   "!PKSAY",
   "\n"
};


const char	*preference_bits[] = {
   "BRIEF",
   "COMPACT",
   "NONEWBIE",
   "NOTELL",
   "D_HP",
   "D_MN",
   "D_MV",
   "D_VT",
   "NOHASS",
   "QUEST",
   "SUMN",
   "NO-REP",
   "LIGHT",
   "C1",
   "C2",
   "NO-WIZ",
   "L1",
   "L2",
   "NOAUC",
   "NOGOS",
   "NOCHAT",
   "RMFLG",
   "D_VICT",
   "D_ANSI",
   "NOSPDWLK",
   "NOALIAS",
   "VERBATIM",
   "NOEXITS",
   "GAG",
   "IBM",
   "NOGRAT",
   "\n"
};


const char	*position_types[] = {
   "Dead",
   "Mortally wounded",
   "Incapacitated",
   "Stunned",
   "Sleeping",
   "Resting",
   "Sitting",
   "Fighting",
   "Standing",
   "\n"
};

const char *ident_types[] = {
   "Ident none",
   "Ident conning",
   "Ident conned",
   "Ident reading",
   "Ident read",
   "Ident complete",
   "\n"
};

const char	*connected_types[] = {
   "Playing",
   "Get name",
   "Cnf name",
   "Get pswd",
   "New PW",
   "Cnf NPW",
   "Sel sex",
   "ReadMOTD",
   "MainMenu",
   "Get dscr",
   "SELclass",
   "Linkless",
   "Ch PW 1",
   "Ch PW 2",
   "Ch PW 3",
   "Disconn",
   "SelfDel1",
   "SelfDel2",
   "Sel race",
   "Sel scr",
   "Dead Msg",
   "TextEdit",
   "\n"
};


const char	*ban_types[] = {
   "no",
   "new",
   "select",
   "all",
   "ERROR"
};

/* [ch] strength apply (all) */
const struct str_app_type str_app[35] = {
/*     HR  DR CARRY WIELD BASH      */
     { -5, -4,    0,   0,  0 },  /* 0 */
     { -5, -4,    3,   1,  1 },  /* 1 */
     { -3, -2,    3,   2,  2 },  /* 2 */
     { -3, -1,   10,   3,  3 },  /* 3 */
     { -2, -1,   25,   4,  4 },  /* 4 */
     { -2, -1,   55,   5,  5 },  /* 5 */
     { -1,  0,   80,   6,  6 },   /* 6 */
     { -1,  0,   90,   7,  6 },   /* 7 */
     {  0,  0,  100,   8,  8 },    /* 8 */
     {  0,  0,  100,   9,  8 },    /* 9 */
     {  0,  0,  115,  10, 10 },    /* 10 */
     {  0,  0,  115,  11, 10 },    /* 11 */
     {  0,  0,  140,  12, 12 },    /* 12 */
     {  0,  0,  140,  13, 12 },    /* 13 */
     {  0,  0,  170,  14, 14 },    /* 14 */
     {  0,  0,  170,  15, 14 },    /* 15 */
     {  0,  1,  195,  16, 16 },    /* 16 */
     {  1,  1,  220,  18, 18 },    /* 17 */
     {  1,  2,  255,  20, 20 },    /* 18 */
     {  3,  7,  640,  40, 50 },    /* 19 */
     {  3,  8,  700,  40, 55 },    /* 20 */
     {  4,  9,  810,  40, 60 },    /* 21 */
     {  4, 10,  970,  40, 65 },   /* 22 */
     {  5, 11, 1130,  40, 70 },  /* 23 */
     {  6, 12, 1440,  40, 75 },  /* 24 */
     {  7, 14, 1750,  40, 80 },  /* 25 */
     {  1,  3,  280,  22, 25 },  /* 26 - 18/01 to 18/50 */
     {  2,  3,  305,  24, 30 },  /* 27 - 18/51 to 18/75 */
     {  2,  4,  330,  26, 35 },  /* 28 - 18/76 to 18/90 */
     {  2,  5,  380,  28, 40 },  /* 29 - 18/91 to 18/99 */
     {  3,  6,  480,  30, 45 },  /* 30 - 18/00 */
};

/* ************************************************************************
*   File: spellconst.c                                  Part of EliteMUD  *
*  Usage: constants for the spells                                        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1994 Mr Wang at RIT                                      *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "limits.h"


const char	*spells[] =  {
    "armor",               /* 1 */
    "teleport",
    "bless",
    "blindness",
    "burning hands",
    "call lightning",
    "charm person",
    "chill touch",
    "clone",
    "color spray",
    "control weather",     /* 11 */
    "create food",
    "create water",
    "cure blind",
    "cure critic",
    "cure light",
    "curse",
    "detect alignment",
    "detect invisibility",
    "detect magic",
    "detect poison",       /* 21 */
    "dispel evil",
    "earthquake",
    "enchant weapon",
    "energy drain",
    "fireball",
    "harm",
    "heal",
    "invisibility",
    "lightning bolt",
    "locate object",      /* 31 */
    "magic missile",
    "poison",
    "protection from evil",
    "remove curse",
    "sanctuary",
    "shocking grasp",
    "sleep",
    "strength",
    "summon",
    "ventriloquate",      /* 41 */
    "word of recall",
    "remove poison",
    "sense life",         /* 44 */
    "fly",
    "infravision",
    "cat eyes",
    "arc fire",
    "freeze foes",
    "word of healing",
    "levitation",        /* 51 */
    "warstrike",
    "identify",
    "animate dead",
    "fear",
    "fire breath",
    "gas breath",
    "frost breath",
    "acid breath",
    "lightning breath",
    "regeneration",     /* 61 */

    "flesh restore",
    "shocking sphere",
    "flesh anew",
    "instant wolf",
    "instant slayer",
    "vorpal plating",
    "quick fix",
    "holy water",
    "mage gauntlets",
    "mystic shield",    /* 71 */
    "star flare",
    "spectre touch",
    "phase door",
    "mystical coat",
    "death strike",
    "ice storm",
    "stone to flesh",
    "mind jab",
    "phase blur",
    "true sight",      /* 81 */
    "wind warrior",
    "wind ogre",
    "wind dragon",
    "disrupt illusion",
    "mind blade",
    "rimefang",
    "maelstrom",
    "elemental summoning",
    "gravity focus",
    "group heal",      /* 91 */
    "group recall",
    "improved bless",
    "phase knife",
    "flame ray",
    "psychic scream",
    "project force",
    "detonate",
    "phasefire",
    "drain life",
    "shadow knife",    /* 101 */
    "disintegrate",
    "dragon breath",
    "terrorweave",
    "elemental cannon",
    "unleash mind",
    "phantasmal killer",
    "water breathing",
    "clan recall",
    "holy wrath",  /* 110 */
    "locate person",

/* Incomplete Spells Below
    "prime summoning",
    "demon bane",
    "flame column",
    "dispossess",
    "spell bind",
    "soul whip",
    "greater summoning",
    "beyond death",
    "wizard blast",
    "demon strike",
    "blunder",
    "batch spell",
    "night lance",
    "IO's mallet",
    "vitality",
    "fatal fist",
    "frost force",
    "god fire",
    "electric stun",
    "luck chant",
    "far death",
    "oil of olay",
    "grave robber",
    "shadow shield",
    "wither",
    "earth dagger",
    "trebuchet",
    "earth elemental",
    "petrify",
    "desert strike",
    "glacier strike",
    "magma blast",
    "jolt bolt",
    "earth maw",
    "Atea's gills",
    "divine intervention",
    "gotterdamurung",  */

    "\n"
};

#define XX  LEVEL_IMMORT

/* Put spells not given to mortals in all caps */

const char spell_minlevel[111][20] =  {
/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*Armor*/{XX, 1,XX,XX,XX, 2,10,XX,80, 1,XX,30,XX, 1,XX,XX,XX,XX,XX,XX},
/*telep*/{20,XX,XX,XX,14,XX,60,XX,10,XX,XX,XX,15,XX,XX,XX,XX,XX,XX,XX},
/*bless*/{XX,10,XX,XX,XX,15,XX,15,XX, 5,XX,55,XX,15,XX,XX,XX,XX,XX,XX},
/*blind*/{10,12,XX,XX, 8,20,18,XX, 6,12,XX,27,10,42,XX,XX,XX,XX,XX,XX},
/*brnhn*/{ 5,XX,XX,XX, 3,XX,20,XX, 4,XX,XX,XX, 8,XX,XX,XX,XX,XX,XX,XX},
/*ca li*/{XX,42,XX,XX,XX,50,XX,XX,XX,25,XX,50,XX,XX,XX,XX,XX,XX,XX,XX},
/*charm*/{40,70,XX,XX,30,80,15,XX,25,50,XX,XX,38,XX,XX,XX,XX,XX,XX,XX},
/*ch to*/{XX, 8,XX,XX, 5,XX,XX,XX,XX, 9,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*clone*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*co sp*/{ 8,XX,XX,XX,10,XX,24,XX, 7,XX,XX,XX,12,XX,XX,XX,XX,XX,XX,XX},
/*co we*/{35,17,XX,XX,XX,XX,27,XX,XX,15,XX,60,XX,XX,XX,XX,XX,XX,XX,XX},
/*cr fo*/{XX, 3,XX,XX,XX,10,30,XX,16, 4,XX,15,XX,47,XX,XX,XX,XX,XX,XX},
/*cr wa*/{XX, 4,XX,XX,XX, 6,25,XX,11, 3,XX,10,XX,39,XX,XX,XX,XX,XX,XX},
/*cu bl*/{XX,14,XX,XX,100,25,XX,80,XX,14,XX,XX,XX,30,XX,XX,XX,XX,XX,XX},
/*cu cr*/{XX,15,XX,XX,XX,21,45,60,90,16,XX,XX,XX,35,XX,XX,XX,XX,XX,XX},
/*cu li*/{XX, 2,XX,XX,XX, 4,XX,25,XX, 1,XX,25,XX,10,XX,XX,XX,XX,XX,XX},
/*curse*/{17,11,XX,XX,12,XX,32,XX,15,XX,XX,XX,17,27,XX,XX,XX,XX,XX,XX},
/*de al*/{XX, 7,XX,XX,11, 8,22,30,12, 8,XX,35, 9, 1,XX,XX,XX,XX,XX,XX},
/*de in*/{ 3, 9,XX,XX, 4,12,17,75, 3,10,XX,40, 3,20,XX,XX,XX,XX,XX,XX},
/*de ma*/{ 2, 5,XX,XX, 2, 5, 5,10, 2, 6,XX,20, 2, 7,XX,XX,XX,XX,XX,XX},
/*de pr*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*di ev*/{XX,36,XX,XX,25,30,XX,XX,20,18,XX,XX,20,XX,XX,XX,XX,XX,XX,XX},
/*equak*/{XX,13,XX,XX,XX,13,21,XX,10,11,XX,65,XX,XX,XX,XX,XX,XX,XX,XX},
/*en wp*/{16,38,XX,XX,XX,40,37,90, 9,28,XX,70,XX,70,XX,XX,XX,XX,XX,XX},
/*drain*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*fireb*/{75,XX,XX,XX,XX,XX,80,XX,70,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*harm */{XX,45,XX,XX,35,55,XX,XX,XX,35,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*heal */{XX,40,XX,XX,90,45,80,100,100,30,XX,90,XX,60,XX,XX,XX,XX,XX,XX},
/*invis*/{ 9,75,XX,XX, 6,90,27,XX, 5,85,XX,XX, 4,XX,XX,XX,XX,XX,XX,XX},
/*l bol*/{40,XX,XX,XX,XX,XX,50,XX,30,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*lo ob*/{24,50,XX,XX,27,35, 7,70,13,XX,XX,80,16,55,XX,XX,XX,XX,XX,XX},
/*ma mi*/{ 1,XX,XX,XX,XX,XX, 2,XX, 1,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*poiso*/{45,28,XX,XX,42,33,XX,XX,30,20,XX,17,XX,XX,XX,XX,XX,XX,XX,XX},
/*pr ev*/{XX,16,XX,XX,XX,12, 9,50,XX,13,XX,XX,XX,25,XX,XX,XX,XX,XX,XX},
/*re cu*/{XX,19,XX,XX,XX,22,35,45,XX,17,XX,XX,XX,32,XX,XX,XX,XX,XX,XX},
/*sanc */{XX,35,XX,XX,45,50,70,65,XX,29,XX,XX,XX,50,XX,XX,XX,XX,XX,XX},
/*sh gr*/{15,XX,XX,XX,13,XX,XX,XX,14,XX,XX,XX,14,XX,XX,XX,XX,XX,XX,XX},
/*sleep*/{30,90,XX,XX,19,70,14,XX,19,XX,XX,XX,19,XX,XX,XX,XX,XX,XX,XX},
/*stren*/{14,XX,XX,XX, 9,17,16,35, 8,23,XX,45,13,40,XX,XX,XX,XX,XX,XX},
/*summo*/{38,26,XX,XX,33,27,75,XX,27,27,XX,75,40,XX,XX,XX,XX,XX,XX,XX},
/*ventr*/{ 4,XX,XX,XX, 3,XX, 6,XX, 2,XX,XX,XX, 1,XX,XX,XX,XX,XX,XX,XX},
/*recal*/{60,48,XX,XX,70,51,55,95,40,26,XX,85,55,80,XX,XX,XX,XX,XX,XX},
/*re po*/{XX,21,XX,XX,62,23,XX,40,XX,21,XX,XX,XX,37,XX,XX,XX,XX,XX,XX},
/*se li*/{42,22,XX,XX,40,29,XX,55,32,19,XX,XX,30,45,XX,XX,XX,XX,XX,XX},
/*fly  */{28,55,XX,XX,22,60,65,XX,21,45,XX,95,25,90,XX,XX,XX,XX,XX,XX},
/*infra*/{13,53,XX,XX,15,62,28,XX,18,32,XX, 5,21,75,XX,XX,XX,XX,XX,XX},
/*ca ey*/{ 1,24,XX,XX, 7,XX, 4,85, 1, 7,XX, 1, 2,65,XX,XX,XX,XX,XX,XX},
/*ar fi*/{ 3,XX,XX,XX, 2,XX,XX,XX, 3,XX,XX,XX, 5,XX,XX,XX,XX,XX,XX,XX},
/*fr fo*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*wo he*/{XX, 6,XX,XX,XX,11,XX,XX,XX, 4,XX,XX,XX,22,XX,XX,XX,XX,XX,XX},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*levit*/{ 7,37,XX,XX,16,57,29,XX,12,38,XX,XX,11,XX,XX,XX,XX,XX,XX,XX},
/*warst*/{11,XX,XX,XX,XX,XX,19,XX,11,XX,XX,XX, 6,XX,XX,XX,XX,XX,XX,XX},
/*ident*/{73,60,XX,XX,80,85,50,XX,50,51,XX,100,60,XX,XX,XX,XX,XX,XX,XX},
/*anide*/{12,XX,XX,XX,XX,XX,XX,XX, 7,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*fear */{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*fi br*/{55,XX,XX,XX,60,XX,XX,XX,49,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ga br*/{53,XX,XX,XX,58,XX,XX,XX,47,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*fr br*/{51,XX,XX,XX,56,XX,XX,XX,45,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ac br*/{49,XX,XX,XX,54,XX,XX,XX,43,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*li br*/{47,95,XX,XX,52,XX,XX,XX,41,100,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*regen*/{XX,57,XX,XX,75,65,85,XX,60,37,XX,XX,XX,85,XX,XX,XX,XX,XX,XX},
/*fl re*/{100,XX,XX,XX,47,XX,XX,XX,XX,10,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*shock*/{XX,18,XX,XX,XX,18,XX,XX,XX,13,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*fl an*/{XX,27,XX,XX,XX,38,XX,XX,XX,24,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*in wo*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, 7,XX,XX,XX,XX,XX,XX,XX},
/*in sl*/{XX,XX,XX,XX,XX,XX,XX,XX,35,XX,XX,XX,22,XX,XX,XX,XX,XX,XX,XX},
/*vorpl*/{ 6,XX,XX,XX, 7,52,26,20, 4,70,XX,XX, 7,95,XX,XX,XX,XX,XX,XX},
/*qu fi*/{65,XX,XX,XX,22,XX,12,XX, 8,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*HOLYW*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*mageg*/{32,XX,XX,XX,31,XX,77,XX,23,95,XX,XX,24,XX,XX,XX,XX,XX,XX,XX},
/*myssh*/{85,80,XX,XX,73,75,90,XX,70,XX,XX,XX,70,100,XX,XX,XX,XX,XX,XX},
/*starf*/{XX,XX,XX,XX,XX,XX,XX,XX,25,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*sp to*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,25,XX,XX,XX,XX,XX,XX,XX},
/*PHSED*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*mysco*/{XX,XX,XX,XX,95,95,100,XX,95,60,XX,XX,50,XX,XX,XX,XX,XX,XX,XX},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*deths*/{75,XX,XX,XX,50,XX,95,XX,55,80,XX,XX,80,XX,XX,XX,XX,XX,XX,XX},
/*icest*/{33,XX,XX,XX,36,XX,52,XX,28,XX,XX,XX,35,XX,XX,XX,XX,XX,XX,XX},
/*STN2F*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*mindj*/{XX,XX,XX,XX, 1,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*phsbl*/{XX,XX,XX,XX,65,XX,XX,XX,53,XX,XX,XX,45,XX,XX,XX,XX,XX,XX,XX},
/*TRUES*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*WINWA*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*WINOG*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*WINDR*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*DISIL*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*MINBL*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*rimef*/{XX,XX,XX,XX,70,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*maels*/{80,XX,XX,XX,78,XX,82,XX,75,XX,XX,XX,85,XX,XX,XX,XX,XX,XX,XX},
/*ELSUM*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*gr fc*/{90,XX,XX,XX,85,XX,XX,XX,85,XX,XX,XX,95,XX,XX,XX,XX,XX,XX,XX},
/*GR HE*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*GR RE*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*IMPBL*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ph kn*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, 1,XX,XX,XX,XX,XX,XX,XX},
/*fl ra*/{XX,XX,XX,XX,XX,XX,XX,XX,15,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ps sc*/{XX,XX,XX,XX,20,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*pr fo*/{XX,XX,XX,XX,40,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*deton*/{XX,XX,XX,XX,XX,XX,XX,XX,50,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ph fi*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,50,XX,XX,XX,XX,XX,XX,XX},
/*dr li*/{105,XX,XX,XX,XX,XX,XX,XX,75,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*sh kn*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,80,XX,XX,XX,XX,XX,XX,XX},
/*disin*/{XX,XX,XX,XX,95,XX,XX,XX,90,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*dr br*/{XX,XX,XX,XX,XX,XX,XX,XX,95,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*terro*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,95,XX,XX,XX,XX,XX,XX,XX},
/*el ca*/{XX,XX,XX,XX,XX,XX,XX,XX,105,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*un mi*/{XX,XX,XX,XX,105,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ph ki*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,105,XX,XX,XX,XX,XX,XX,XX},
/*watwo*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,105,XX,XX,XX,XX,XX,XX,XX},
/*CL RE*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX},
/*ho wr*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,20,XX,XX,XX,XX,XX,XX},
/*lo pe*/{XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX}
/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */
};

const char spell_max[111][20] =  {
/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*armor*/{ 5,95, 5, 5, 5,90,75, 5, 5,95, 5,60, 5,70, 5, 5, 5, 5, 5, 5},
/*telep*/{80, 5, 5, 5,70, 5,60, 5,95, 5, 5,25, 5, 5, 5, 5, 5, 5, 5, 5},
/*bless*/{ 5,95, 5, 5, 5,70, 5,50, 5,95, 5,50, 5,50, 5, 5, 5, 5, 5, 5},
/*blind*/{75,95, 5, 5,95,75,95, 5,95,95, 5,60,75,30, 5, 5, 5, 5, 5, 5},
/*brnhn*/{95, 5, 5, 5,95, 5,95, 5,95, 5, 5,35,85, 5, 5, 5, 5, 5, 5, 5},
/*ca li*/{ 5,75, 5, 5, 5,70, 5, 5, 5,95, 5,75, 5, 5, 5, 5, 5, 5, 5, 5},
/*charm*/{75,75, 5, 5,75,70,95, 5,80,60, 5,30,75, 5, 5, 5, 5, 5, 5, 5},
/*ch to*/{ 5,85, 5, 5, 5, 5, 5, 5, 5,75, 5,40, 5, 5, 5, 5, 5, 5, 5, 5},
/*clone*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*co sp*/{75, 5, 5, 5,75, 5,85, 5,95, 5, 5,35,65, 5, 5, 5, 5, 5, 5, 5},
/*co we*/{50,75, 5, 5,50, 5,60, 5,75,95, 5,25,95, 5, 5, 5, 5, 5, 5, 5},
/*cr fo*/{ 5,90, 5, 5, 5,75,50, 5,35,95, 5,65,25,50, 5, 5, 5, 5, 5, 5},
/*cr wa*/{ 5,90, 5, 5, 5,75,50, 5,35,95, 5,65,25,50, 5, 5, 5, 5, 5, 5},
/*cu bl*/{ 5,95, 5, 5,50,75,40,50,65,95, 5,75,55,50, 5, 5, 5, 5, 5, 5},
/*cu cr*/{ 5,95, 5, 5, 5,90,75,50, 5,95, 5,75, 5,75, 5, 5, 5, 5, 5, 5},
/*cu li*/{ 5,95, 5, 5, 5,90, 5,75, 5,95, 5,75, 5,90, 5, 5, 5, 5, 5, 5},
/*curse*/{75,75, 5, 5,95,75,95, 5,95,85, 5,50,75,75, 5, 5, 5, 5, 5, 5},
/*de al*/{95,95, 5, 5,95,95,95,50,95,95, 5,95,95,75, 5, 5, 5, 5, 5, 5},
/*de in*/{95,95, 5, 5,95,95,95,50,95,95, 5,95,95,75, 5, 5, 5, 5, 5, 5},
/*de ma*/{95,95, 5, 5,95,95,95,50,95,95, 5,95,95,75, 5, 5, 5, 5, 5, 5},
/*de pr*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*di ev*/{ 5,80, 5, 5,75,75, 5, 5, 5,95, 5,50, 5, 5, 5, 5, 5, 5, 5, 5},
/*equak*/{ 5,75, 5, 5, 5,75,75, 5, 5,95, 5,50, 5, 5, 5, 5, 5, 5, 5, 5},
/*en wp*/{95,75, 5, 5,75,50,95,50,95,75, 5,75,75,50, 5, 5, 5, 5, 5, 5},
/*drain*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*fireb*/{85, 5, 5, 5, 5, 5,85, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*harm */{ 5,85, 5, 5,80,75, 5,25, 5,95, 5,50, 5,50, 5, 5, 5, 5, 5, 5},
/*heal */{ 5,90, 5, 5,60,90,50,40,40,95, 5,50, 5,60, 5, 5, 5, 5, 5, 5},
/*invis*/{95,75, 5, 5,75,75,75, 5,95,95, 5,50,95, 5, 5, 5, 5, 5, 5, 5},
/*l bol*/{85, 5, 5, 5, 5, 5,75, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*lo ob*/{75,50, 5, 5,50,50,95,50,95,75, 5,25,75,50, 5, 5, 5, 5, 5, 5},
/*ma mi*/{95, 5, 5, 5, 5, 5,95, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*poiso*/{50,75, 5, 5,75,75,50, 5,75,95, 5,95, 5, 5, 5, 5, 5, 5, 5, 5},
/*pr ev*/{ 5,75, 5, 5, 5,75,50,50, 5,95, 5,50, 5,75, 5, 5, 5, 5, 5, 5},
/*re cu*/{ 5,75, 5, 5, 5,95, 5,35, 5,95, 5,50, 5,75, 5, 5, 5, 5, 5, 5},
/*sanct*/{ 5,75, 5, 5,50,95,50,40, 5,95, 5,50, 5,50, 5, 5, 5, 5, 5, 5},
/*sh gr*/{85, 5, 5, 5,75, 5, 5, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*sleep*/{85,75, 5, 5,95,50,95, 5,95,75, 5,50,95, 5, 5, 5, 5, 5, 5, 5},
/*stren*/{50, 5, 5, 5,95,75,50,50,75,50, 5,25,75,50, 5, 5, 5, 5, 5, 5},
/*summo*/{75,75, 5, 5,50,50,50, 5,75,50, 5,25,95, 5, 5, 5, 5, 5, 5, 5},
/*ventr*/{95, 5, 5, 5,95, 5,95, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*recal*/{75,95, 5, 5, 5,75,85,50,95,95, 5,50,75,50, 5, 5, 5, 5, 5, 5},
/*re ps*/{ 5,75, 5, 5, 5,50, 5,35, 5,95, 5,95, 5,50, 5, 5, 5, 5, 5, 5},
/*se li*/{75,75, 5, 5,75,75,75,50,95,95, 5,95,95,75, 5, 5, 5, 5, 5, 5},
/*fly  */{95,50, 5, 5,75,50,75, 5,95,75, 5,75,75,50, 5, 5, 5, 5, 5, 5},
/*infra*/{75,75, 5, 5,95,50,75, 5,95,95, 5,95,75,50, 5, 5, 5, 5, 5, 5},
/*ca ey*/{75,50, 5, 5,75, 5,75,50,95,95, 5,95,75,50, 5, 5, 5, 5, 5, 5},
/*ar fi*/{85, 5, 5, 5,75, 5,75, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*fr fo*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*wohea*/{ 5,95, 5, 5, 5,85, 5, 5, 5,95, 5,75, 5,75, 5, 5, 5, 5, 5, 5},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*levit*/{75,50, 5, 5,50,75,75, 5,95, 5, 5, 5,75, 5, 5, 5, 5, 5, 5, 5},
/*warst*/{85, 5, 5, 5,75, 5,75, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*ident*/{75,50, 5, 5,50,75,95, 5,95,75, 5,50,95, 5, 5, 5, 5, 5, 5, 5},
/*an de*/{50, 5, 5, 5, 5, 5, 5, 5,75, 5, 5, 5,50, 5, 5, 5, 5, 5, 5, 5},
/*fear */{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*fi br*/{85, 5, 5, 5,80, 5,75, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*ga br*/{85, 5, 5, 5,80, 5,75, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*fr br*/{85, 5, 5, 5,80, 5,75, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*ac br*/{85, 5, 5, 5,80, 5,75, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*li br*/{85,75, 5, 5,80, 5,75, 5,95,95, 5,25,95, 5, 5, 5, 5, 5, 5, 5},
/*regen*/{ 5,75, 5, 5,50,75,40, 5, 5,95, 5,50, 5,35, 5, 5, 5, 5, 5, 5},
/*fl re*/{70, 5, 5, 5,75, 5, 5, 5, 5,95, 5,75, 5, 5, 5, 5, 5, 5, 5, 5},
/*sh sp*/{ 5,90, 5, 5, 5,75, 5, 5, 5,95, 5,50, 5, 5, 5, 5, 5, 5, 5, 5},
/*flnew*/{ 5,95, 5, 5, 5,95, 5,50, 5,95, 5,50, 5, 5, 5, 5, 5, 5, 5, 5},
/*in wo*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*in sl*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*vp pl*/{75, 5, 5, 5,50,50,75,50,95,50, 5,25,95,50, 5, 5, 5, 5, 5, 5},
/*qu fi*/{75, 5, 5, 5,95, 5,75, 5,75, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*HOLYW*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*mageg*/{50, 5, 5, 5,50, 5,40, 5,75,50, 5, 5,75, 5, 5, 5, 5, 5, 5, 5},
/*myssh*/{50,50, 5, 5,50,50,50, 5,75,75, 5, 5,75,50, 5, 5, 5, 5, 5, 5},
/*starf*/{ 5, 5, 5, 5, 5, 5, 5, 5,75, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*sptou*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,75, 5, 5, 5, 5, 5, 5, 5},
/*PHSED*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*mysco*/{ 5, 5, 5, 5,50,25,25, 5,75,75, 5,25,95, 5, 5, 5, 5, 5, 5, 5},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*deths*/{75, 5, 5, 5,75, 5,50, 5,95,75, 5,25,50, 5, 5, 5, 5, 5, 5, 5},
/*icest*/{85, 5, 5, 5,75, 5,85, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*STN2F*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*mindj*/{ 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*ph bl*/{50, 5, 5, 5,75, 5, 5, 5,75, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*TRUES*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*WI WA*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*WI OG*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*WI DR*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*DISIL*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*MINDB*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*rimef*/{ 5, 5, 5, 5,75, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*maels*/{85, 5, 5, 5,75, 5,85, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*EL_SU*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*gr fo*/{85, 5, 5, 5,75, 5, 5, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*GR HE*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*GR RE*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*IMPBL*/{ 5,95, 5, 5, 5,70, 5,50, 5,95, 5,50, 5,50, 5, 5, 5, 5, 5, 5},
/*ph kn*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*fl ra*/{ 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*ps sc*/{ 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*pr fo*/{ 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*deton*/{ 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*ph fi*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*dr li*/{95, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*sh kn*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*disin*/{ 5, 5, 5, 5,95, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*dr br*/{ 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*terro*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*el ca*/{ 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*un mi*/{ 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*ph ki*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,95, 5, 5, 5, 5, 5, 5, 5},
/*wa br*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*CL RE*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
/*ho wr*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,75, 5, 5, 5, 5, 5, 5},
/*lo pe*/{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */
};

const char spell_difficulty[111][20] =  {
/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*armor*/{ 1, 6, 1, 1, 1, 5, 4, 1, 1, 7, 1, 4, 1, 4, 1, 1, 1, 1, 1, 1},
/*telep*/{ 6, 1, 1, 1, 5, 1, 1, 1, 7, 1, 1, 4, 6, 1, 1, 1, 1, 1, 1, 1},
/*bless*/{ 1, 5, 1, 1, 1, 4, 1, 2, 1, 5, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
/*blind*/{ 4, 3, 1, 1, 5, 3, 3, 1, 5, 5, 1, 2, 4, 2, 1, 1, 1, 1, 1, 1},
/*brnhn*/{ 4, 1, 1, 1, 5, 1, 4, 1, 6, 1, 1, 1, 6, 1, 1, 1, 1, 1, 1, 1},
/*ca li*/{ 1, 3, 1, 1, 4, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*charm*/{ 3, 2, 1, 1, 3, 2, 7, 1, 5, 5, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},
/*ch to*/{ 1, 4, 1, 1, 4, 4, 3, 1, 4, 5, 1, 2, 5, 1, 1, 1, 1, 1, 1, 1},
/*clone*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*co sp*/{ 5, 1, 1, 1, 6, 1, 5, 1, 8, 1, 1, 3, 7, 1, 1, 1, 1, 1, 1, 1},
/*contw*/{ 4, 5, 1, 1, 3, 3, 4, 1, 4, 7, 1, 2, 4, 1, 1, 1, 1, 1, 1, 1},
/*cr fo*/{ 1, 6, 1, 1, 1, 4, 3, 1, 4, 7, 1, 3, 4, 2, 1, 1, 1, 1, 1, 1},
/*cr wa*/{ 1, 6, 1, 1, 1, 4, 3, 1, 4, 7, 1, 3, 4, 2, 1, 1, 1, 1, 1, 1},
/*cu bl*/{ 1, 5, 1, 1, 3, 4, 2, 1, 3, 6, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
/*cu cr*/{ 1, 4, 1, 1, 2, 4, 2, 1, 2, 5, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1},
/*cu lg*/{ 1, 6, 1, 1, 1, 5, 1, 2, 2, 7, 1, 4, 1, 4, 1, 1, 1, 1, 1, 1},
/*curse*/{ 3, 5, 1, 1, 6, 4, 4, 1, 4, 4, 1, 2, 4, 3, 1, 1, 1, 1, 1, 1},
/*de al*/{ 1, 6, 1, 1, 3, 4, 3, 2, 4, 4, 1, 2, 4, 3, 1, 1, 1, 1, 1, 1},
/*de in*/{ 6, 4, 1, 1, 5, 4, 5, 2, 7, 4, 1, 4, 5, 3, 1, 1, 1, 1, 1, 1},
/*de ma*/{ 7, 5, 1, 1, 3, 4, 5, 2, 8, 3, 1, 3, 6, 3, 1, 1, 1, 1, 1, 1},
/*de pr*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*di ev*/{ 1, 4, 1, 1, 3, 3, 2, 1, 1, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*equak*/{ 1, 3, 1, 1, 3, 3, 3, 1, 2, 4, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1},
/*en we*/{ 3, 3, 1, 1, 2, 3, 4, 2, 5, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1},
/*drain*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*fireb*/{ 6, 1, 1, 1, 5, 1, 4, 1, 7, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1},
/*harm */{ 1, 3, 1, 1, 3, 1, 1, 1, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1},
/*heal */{ 1, 4, 1, 1, 2, 3, 2, 1, 1, 5, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
/*invis*/{ 6, 3, 1, 1, 5, 1, 4, 1, 7, 1, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1},
/*li bo*/{ 5, 1, 1, 1, 4, 1, 3, 1, 6, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},
/*lo ob*/{ 4, 3, 1, 1, 3, 3, 5, 1, 6, 3, 1, 2, 5, 2, 1, 1, 1, 1, 1, 1},
/*ma mi*/{ 7, 1, 1, 1, 5, 1, 4, 1, 8, 1, 1, 3, 4, 1, 1, 1, 1, 1, 1, 1},
/*poiso*/{ 4, 6, 1, 1, 8, 5, 1, 1, 6, 4, 1, 2, 4, 1, 1, 1, 1, 1, 1, 1},
/*pr ev*/{ 1, 6, 1, 1, 4, 4, 3, 2, 3, 7, 1, 3, 3, 6, 1, 1, 1, 1, 1, 1},
/*re cu*/{ 1, 5, 1, 1, 4, 4, 5, 2, 5, 4, 1, 2, 4, 3, 1, 1, 1, 1, 1, 1},
/*sanct*/{ 1, 2, 1, 1, 2, 2, 1, 1, 2, 5, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
/*sh gr*/{ 6, 1, 1, 1, 7, 1, 1, 1, 8, 1, 1, 2, 4, 1, 1, 1, 1, 1, 1, 1},
/*sleep*/{ 3, 3, 1, 1, 4, 2, 6, 1, 4, 3, 1, 2, 6, 1, 1, 1, 1, 1, 1, 1},
/*stren*/{ 5, 1, 1, 1, 5, 3, 3, 2, 5, 3, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/*summo*/{ 3, 6, 1, 1, 3, 3, 4, 1, 8, 4, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1},
/*ventr*/{ 7, 1, 1, 1, 1, 1, 9, 1, 8, 1, 1, 3, 4, 1, 1, 1, 1, 1, 1, 1},
/*recal*/{ 3, 5, 1, 1, 1, 3, 3, 1, 4, 4, 1, 2, 6, 2, 1, 1, 1, 1, 1, 1},
/*re po*/{ 1, 7, 1, 1, 3, 5, 1, 1, 4,10, 1, 6, 3, 2, 1, 1, 1, 1, 1, 1},
/*se li*/{ 1, 5, 1, 1, 7, 3, 6, 2, 4, 5, 1, 7, 2, 2, 1, 1, 1, 1, 1, 1},
/*fly  */{ 4, 3, 1, 1, 3, 3, 3, 1, 7, 4, 1, 2, 8, 2, 1, 1, 1, 1, 1, 1},
/*infra*/{ 4, 3, 1, 1, 8, 3, 3, 2, 4, 6, 1, 5, 3, 2, 1, 1, 1, 1, 1, 1},
/*ca ey*/{ 5, 3, 1, 1, 8, 3, 4, 2, 5, 6, 1, 4,6 , 2, 1, 1, 1, 1, 1, 1},
/*ar fi*/{ 6, 1, 1, 1, 4, 2, 3, 1, 8, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},
/*fr fo*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*wo he*/{ 3, 6, 1, 1, 1, 4, 1, 1, 1, 5, 1, 2, 1, 3, 1, 1, 1, 1, 1, 1},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*levit*/{ 5, 3, 1, 1, 4, 2, 4, 1, 6, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1},
/*wastr*/{ 6, 1, 1, 1, 4, 1, 4, 1, 8, 1, 1, 2, 5, 1, 1, 1, 1, 1, 1, 1},
/*ident*/{ 4, 3, 1, 1, 3, 4, 5, 1, 6, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*an de*/{ 3, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*fear */{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*fi br*/{ 4, 1, 1, 1, 3, 1, 1, 1, 7, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*ga br*/{ 4, 1, 1, 1, 3, 1, 1, 1, 7, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*fr br*/{ 4, 1, 1, 1, 3, 1, 1, 1, 7, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*ac br*/{ 4, 1, 1, 1, 3, 1, 1, 1, 7, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*li br*/{ 4, 3, 1, 1, 3, 1, 1, 1, 7, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*regen*/{ 1, 3, 1, 1, 3, 4, 2, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*fl re*/{ 3, 1, 1, 1, 2, 1, 1, 1, 1, 4, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*sh sp*/{ 1, 4, 1, 1, 2, 3, 1, 1, 5, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*flesh*/{ 1, 4, 1, 1, 3, 4, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*in wo*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*in sl*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*vp pl*/{ 4, 1, 1, 1, 3, 1, 2, 2, 5, 2, 1, 2, 4, 1, 1, 1, 1, 1, 1, 1},
/*qu fi*/{ 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*HOLYW*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*mageg*/{ 4, 1, 1, 1, 3, 1, 2, 1, 4, 2, 1, 2, 7, 1, 1, 1, 1, 1, 1, 1},
/*myssh*/{ 3, 3, 1, 1, 3, 3, 1, 1, 4, 3, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},
/*starf*/{ 1, 4, 1, 1, 3, 3, 2, 2, 5, 4, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1},
/*sp to*/{ 1, 3, 1, 1, 5, 1, 3, 1, 7, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1},
/*PHSED*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*mysco*/{ 1, 1, 1, 1, 2, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*deths*/{ 3, 1, 1, 1, 3, 1, 2, 1, 4, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*icest*/{ 4, 1, 1, 1, 3, 1, 2, 1, 5, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*STN2F*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*mindj*/{ 1, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*ph bl*/{ 1, 1, 1, 1, 3, 1, 1, 1, 4, 1, 1, 1, 7, 1, 1, 1, 1, 1, 1, 1},
/*TRUES*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*WI WA*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*WI OG*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*WI DR*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*DISIL*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*MINDB*/{ 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*rimef*/{ 3, 1, 1, 1, 3, 1, 2, 1, 4, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*maels*/{ 3, 1, 1, 1, 3, 1, 2, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*EL_SU*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*gr fo*/{ 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*GR HE*/{ 1, 4, 1, 1, 2, 3, 2, 1, 1, 5, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
/*GR RE*/{ 3, 5, 1, 1, 1, 3, 3, 1, 4, 4, 1, 2, 6, 2, 1, 1, 1, 1, 1, 1},
/*IMPBL*/{ 1, 5, 1, 1, 1, 4, 1, 2, 1, 5, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
/*ph kn*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1},
/*fl ra*/{ 1, 1, 1, 1, 1, 1, 1, 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*ps sc*/{ 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*pr fo*/{ 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*deton*/{ 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*ph fi*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},

/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */

/*dr li*/{ 5, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*sh kn*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1},
/*disin*/{ 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*dr br*/{ 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*terro*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
/*el ca*/{ 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*un mi*/{ 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*ph ki*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1},
/*wa br*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*CL RE*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*HO WR*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1},
/*lo pe*/{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
/*Spell   MU CL TH WA Ps MO BA KN WI DR AS RA IL PA MA CA !! !! NI !! */
};


const char  *mob_cast_array[] = {
    "UNDEFINED",
    "OFFENSIVE",
    "AFFECT SELF",
    "AFFECT VICT",
    "HEAL SELF",
    "FLEE",
    "AFF OWN WPN",
    "AFF VICTIM'S WPN",
    "CURE SELF",
    "\n"
};
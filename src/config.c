/* ************************************************************************
*   File: config.c                                      Part of EliteMUD  *
*  Usage: Configuration of various aspects of EliteMUD operation          *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */
#define __CONFIG_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "functions.h"

#define YES	1
#define FALSE	0
#define NO	0



/*
Below are several constants which you can change to alter certain aspects
of the way EliteMUD acts.  Since this is a .c file, all you have to do
to change one of the constants (assuming you keep your object files around)
is change the constant in this file and type 'make'.  Make will recompile
this file and relink; you don't have to wait for the whole thing to recompile
as you do if you change a header file.

I realize that it would be slightly more efficient to have lots of
#defines strewn about, so that, for example, the autowiz code isn't
compiled at all if you don't want to use autowiz.  However, the actual
code for the various options is quite small, as is the computational time
in checking the option you've selected at run-time, so I've decided the
convenience of having all your options in this one file outweighs the
efficency of doing it the other way.
*/

/****************************************************************************/
/****************************************************************************/

struct elite_config {
int pk_allowed;
int pt_allowed;
int pkok_allowed;
int loot_allowed;
int max_recall_level;
int level_can_chat;
int holler_move_cost;
int max_bank_gold;
int max_npc_corpse_time;
int max_pc_corpse_time;
int dts_are_dumps;
int max_obj_save;
int newbie_clan;
int auto_save;
int autosave_time;
int use_autowiz;
int min_wizlist_lev;
} config;

void read_elite_config();
void write_elite_config();
void default_elite_config();

/* GAME PLAY OPTIONS */

/*
 * pk_allowed sets the tone of the entire game.  If pk_allowed is set to
 * NO, then players will not be allowed to kill, summon, charm, or sleep
 * other players, as well as a variety of other "asshole player" protections.
 * However, if you decide you want to have an all-out knock-down drag-out
 * PK Mud, just set pk_allowed to YES - and anything goes.
 */
int pk_allowed;

/* is playerthieving allowed? */
int pt_allowed;

/* Is PKOK Opt In Global PK System enabled? */
int pkok_allowed;

/* Are players allowed to loot other players corpses? */
int loot_allowed;

/* maximum level a player can be to use recall command */
int max_recall_level;

/* minimum level a player must be to chat/gossip/auction */
int level_can_chat;

/* number of movement points it costs to holler */
int holler_move_cost;

/* gold limits */
int max_bank_gold;    

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int max_npc_corpse_time;
int max_pc_corpse_time;

/* should items in death traps automatically be junked? */
int dts_are_dumps;


/* Newbie Clan */
int newbie_clan;


/* Channels Allowed */
sbyte channel_allowed[7];


/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/* maximum number of items players are allowed to rent */
int max_obj_save;

/* receptionist's surcharge on top of item costs */
int min_rent_cost = 100;

/* Should the game automatically save people?  (i.e., save player data
   every 4 kills (on average), and Crash-save as defined below. */
int auto_save;

/* if auto_save (above) is yes, how often (in minutes) should the MUD
   Crash-save people's objects? */
int autosave_time;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int crash_file_timeout = 10;

/* Lifetime of normal rent files in days */
int rent_file_timeout = 30;


/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that immorts should enter at by default */
sh_int immort_start_room = 1204;

/* virtual number of room that mortals should enter at */
sh_int mortal_start_room = 3001;
/* virtual number of room that newbies should enter at */
sh_int newbie_start_room = 2000;
/* virtual number of room that newbies corpses should appear at */
sh_int newbie_corpse_room = 3001;

/* virtual number of room that frozen players should enter at */
sh_int frozen_start_room = 4;

/* virtual number of wargames room */
sh_int wargame_start_room = 15800;

/* virtual numbers of donation rooms.  note: you must change code in
   do_drop of act.obj1.c if you change the number of non-NOWHERE
   donation rooms.
*/
sh_int donation_room_1 = 3063;
sh_int donation_room_2 = NOWHERE;
sh_int donation_room_3 = NOWHERE;


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/* default port the game should run on if no port given on command-line */
int DFLT_PORT = 4500;

/* default directory to use as data directory */
char *DFLT_DIR = "lib";

/* maximum number of players allowed before game starts to turn people away */
int MAX_PLAYERS = 100;

/* maximum number of files your system allows a process to have open at
   once -- not used if you define #TABLE_SIZE in comm.c or if your system
   defines OPEN_MAX.
*/
int MAX_DESCRIPTORS_AVAILABLE = 128;

/* Some nameservers (such as the one here at JHU) are slow and cause the
   game to lag terribly every time someone logs in.  The lag is caused by
   the gethostbyaddr() function -- the function which resolves a numeric
   IP address (such as 128.220.13.30) into an alphabetic name (such as
   circle.cs.jhu.edu).

   The nameserver at JHU can get so bad at times that the incredible lag
   caused by gethostbyaddr() isn't worth the luxury of having names
   instead of numbers for players' sitenames.

   If your nameserver is fast, set the variable below to NO.  If your
   nameserver is slow, of it you would simply prefer to have numbers
   instead of names for some other reason, set the variable to YES.

   You can experiment with the setting of nameserver_is_slow on-line using
   the SLOWNS command from within the MUD.
*/

int nameserver_is_slow = YES;
   
/* If YES, the mud will attempt to find the remote user name for each
   player connecting to the mud.  This can also be toggled in the game
   with the "ident" command. */
int ident = YES;


/* Should the game always save the last command entered?
 * Good if you are debugging.
 */
int save_last_command = NO;



char *MENU = 
"\033[H\033[J#N"
"                   ßb___                             _\r\n" 
"                  (  _) /  o _/_ _    /|/|       ___)\r\n"
"                  ßy_)   /  /  /  /_)  / ( |  / / /  /\r\n"
"                 ßb(____(__(__(__(____/    |_(_(_(__(\r\n"
"\r\n"
"                 #:bßr(0) Quit El#:yit#:beMud.                     #N\r\n"
"                 #:bßw(1) Adventu#:yßbreßw#:b in the Realm of EliteMud.#N\r\n"
"                 #:bßw(2) Enter d#:yßbesßw#:bcription.                 #N\r\n"
"                 #:yßb(3) Read background story.             #N\r\n"   
"                 #:bßw(4) Change #:yßbpaßw#:bssword.                   #N\r\n"
"                 #:bßr(5) Delete #:yth#:bis character.             #N\r\n"
"                 #:bßw(p) Change/#:yßbseßw#:bt your plan.              #N\r\n"
"\r\n"
"                 ßNThe option is yours: ";

char *PC_MENU =
"\033[0;37;40m\033[H\033[J"
"\033[0;1m\033[20C\033[47m‹\033[40m€€€€€€€€€€‹›\033[0mﬁ\033[1;47m±∞\033[0mﬂ‹ﬂ\033[1m‹€€€€€€ €€ﬂ\033[0mﬂ\033[8C€\033[1m‹€€€€€\r\n"
"\033[20C€€€€€€€€€€€€€ \033[0mﬁ›ﬁ›\033[1mﬁ€€€€€ﬂ‹ﬂ\033[11C\033[47m‹\033[40m€€€€€€\r\n"
"\033[21Cﬂﬂﬂ\033[47mﬂ\033[40m€€€€€€€€€‹\033[0mﬂ‹ﬂ‹\033[1mﬂ€ﬂﬂ\033[6C\033[31m˛\033[5C\033[0m‹\033[1m‹€€€\033[47mﬂ\033[40m€€€› \033[0m‹‹\r\n"
"\033[22C\033[1;47m∞\033[5C\033[0mﬂ\033[1mﬂﬂﬂﬂﬂ \033[0m‹ \033[1;30mﬂ  \033[37m‹‹‹\033[0m‹   \033[1m‹ ‹‹‹\033[47m‹\033[40mﬂ\033[47mﬂ\033[40mﬂ\033[47mﬂﬂ‹‹\033[40m€€€€ \033[47m≤≤±\033[40m\r\n"
"\033[21C\033[0mﬁ\033[1;47m±≤\033[0m‹\033[5C\033[31m˛   \033[37mﬁ\033[1;47m∞\033[0mﬂ\033[1m‹€€€€€€€€€€€‹‹‹‹€€€€\033[47mﬂ\033[40mﬂﬂ€€€€›ﬁ€\033[47m≤\033[40m\r\n"
"\033[21C\033[0mﬂ\033[1;47m≤\033[40m€€€\033[47m‹\033[40m‹\033[0m‹  ‹‹\033[1;47m±∞\033[0m‹\033[1mﬁ   ﬂﬂ€€€€\033[47m€€\033[40m€€\033[47m≤≤±∞\033[0mﬂ    \033[1mﬁ€€€€ﬂ €€\r\n"
"\033[23Cﬂ€€€€€\033[47m≤≤≤\033[0mﬂ\033[1;47m≤±\033[0m›\033[1m€  ‹  ‹\033[47m≤\033[40m€\033[47m€€\033[40m€€ﬂ€€\033[47m≤\033[40mﬂ ﬂ    ﬂﬂ  € \033[47m≤\033[40m€\r\n"
"\033[25C\033[0mﬂ\033[1mﬂﬂﬂﬂ   \033[47m≤≤≤\033[0m› \033[1m‹\033[47m≤≤≤±±ﬂ\033[40m€€\033[47m‹ﬂ\033[40mﬂ€ﬂ‹ €‹ \033[0mﬁ‹\033[1;47m∞∞\033[2C\033[40mﬁ›\033[0mﬁ\033[1;47m±≤\033[40m\r\n"
"\033[33Cﬁ\033[47m≤\033[40m€€\033[47m≤≤\033[40m€\033[47m±€€‹±ﬂ\033[40mﬂﬂ ‹ ﬂ€› ﬂ   \033[0mﬂ›  \033[1mﬂ \033[47m∞±\033[0mﬂ\r\n"
"\033[34C\033[1mﬂ€€€\033[47m±\033[40m€€\033[47m‹\033[40mﬂﬂ‹‹€‹ﬂ€€  ﬂ\033[10C\033[47m∞\033[0mﬂ\r\n"
"\033[36C\033[1mﬂﬂﬂ ‹ €€‹ﬂ€ﬂ\033[7C\033[30m∞\033[7C\033[0mﬂ \033[1;30;47m∞\033[40m\r\n"
"\033[7C\033[37m(\033[0;31m0\033[1;37m) Quit from EliteMud\033[10Cﬂ€€ ﬂ\033[10C\033[30m±±\033[6C\033[37m‹  \033[47m∞\033[40m\r\n"
"\033[8C(\033[0;31m1\033[1;37m) Journey Onward\033[26C\033[30m‹€±\033[6C\033[37m‹ ﬂ\033[0mﬁ\033[1;47m±\033[40m\r\n"
"\033[9C(\033[0;31m2\033[1;37m) Enter description\033[21C\033[30;47m±≤\033[40m±\033[5C\033[37m‹\033[47mﬂ\033[40m€€ \033[47m≤\033[0m›\r\n"
"\033[10C(\033[0;31m3\033[1;37m) Read background story\033[11C‹‹\033[1;30;47m∞±≤\033[40mﬂﬂ \033[37m‹ ‹ﬂ€€\033[0mﬂ\033[1m‹\033[47m≤≤\033[40m\r\n"
"\033[11C(\033[0;31m4\033[1;37m) Change password\033[16C\033[0mﬁ\033[1;47m±∞\033[0mﬂ\033[1m‹ ‹€›ﬂ€ ﬂ‹‹€€€\r\n"
"\033[12C(\033[0;31m5\033[1;37m) Delete this character\033[10C\033[0mﬂ‹\033[1mﬂ€€ ﬂﬂ‹‹€€≤±∞ﬂ\r\n"
"\033[48Cﬁ\033[47m≤\033[40m‹‹€€€€≤≤±∞ﬂ\r\n"
"\033[49Cﬂ€€€ﬂﬂﬂ\r\n"
" What is your choice adventurer: \033[0m";



char *GREETINGS =
"\033[H\033[J"
"\r\n\r\n"
"                     ____/    /       / __   __/   ____/\r\n"
"                    /        /       /      /     /\r\n"
"                   ___/     /       /      /     ___/\r\n"
"                  /        /       /      /     /\r\n"
"               _______/ ______/ __/    __/   _______/\r\n"     
"\r\n"
"                  The purpose is C/C++ development,\r\n"
"               Having fun while doing it is just a bonus.\r\n"
"\r\n"
"    Created by Petrus Wang (IO) and Richard Rosenberg (Rigel)\r\n"
"\r\n"
"                                  Based on DikuMUD (GAMMA 0.0)\r\n"
"    Special credits to            Created by Hans Staerfeldt,\r\n"
"    Jeremy Elson for              Sebastian Hammer, Katja Nyboe,\r\n"
"    CircleMUD 3.0 code            Tom Madsen and Michael Seifert\r\n"
"\r\n"
"    Implementor: Kestrel          Host: Britomartis\r\n\r\n";

char *WELC_MESSG =
"\r\n"
"Welcome to the land of ßGEliteßNMUD!  Stay awhile ... Stay ßRFOREVER!ßN"
"\r\n\r\n";

char *START_MESSG =
"Welcome to ßGEliteßNMud.  Stay awhile ... Stay ßRFOREVER!ßN\r\n"
"\r\n";

char *DEAD_MESSG =
"ßRYou have died. Press RETURN to go to main menu...ßN\r\n";

char *BYE_MESSG =
" THANK YOU FOR PLAYING ELITE MUD.  COME BACK SOON :)\r\n";

char *COLOR_TEST =
"These are the standard colors:\r\n"
"ßNNormal    ßRRed       ßGGreen     ßYYellow\r\n"
"ßBBlue      ßMMagenta   ßCCyan      ßeGrey\r\n"
"ßNNone      ßrLRed      ßgLGreen    ßyLYellowßN\r\n"
"ßbLBlue     ßmLMagenta  ßcLCyan     ßwWhite\r\n"
"ßNBNormal   ß:RBRed      ß:GBGreen    ß:YBYellow   #N\r\n"
"ß:BBlue      ß:MMagenta   ß:CBCyan     ß:WBWhite    #N\r\n";


/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/* Should the game automatically create a new wizlist/immlist every time
   someone immorts, or is promoted to a higher (or lower) god level? */
int use_autowiz;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead. */
int min_wizlist_lev;


/****************************************************************************/


/* CONFIG OPTIONS */

void default_elite_config() {

pk_allowed = config.pk_allowed = NO;
pt_allowed = config.pt_allowed = NO;
pkok_allowed = config.pkok_allowed = YES;
loot_allowed = config.loot_allowed = NO;
max_recall_level = config.max_recall_level = 10;
level_can_chat = config.level_can_chat = 10;
holler_move_cost = config.holler_move_cost = 0;
max_bank_gold = config.max_bank_gold = 1000000000;
max_npc_corpse_time = config.max_npc_corpse_time = 10;
max_pc_corpse_time = config.max_pc_corpse_time = 500;
dts_are_dumps = config.dts_are_dumps = NO;
max_obj_save = config.max_obj_save = 100;
newbie_clan = config.newbie_clan = 100;
auto_save = config.auto_save = YES;
autosave_time = config.autosave_time = 6;
use_autowiz = config.use_autowiz = NO;
min_wizlist_lev = config.min_wizlist_lev = LEVEL_DEITY;
}


void read_elite_config()
{
  FILE *fl;

  if (!(fl = fopen("misc/Elite.cfg", "rb"))) {
    if (errno != ENOENT) {   /* It's not that it just doesn't exist... */
      log("Error opening Elite config file - restoring defaults.");
      default_elite_config();
      return;
    } else { /* It doesn't exist, so create the default one */
      log("Creating Elite config file.");
      default_elite_config();
      write_elite_config();
      return;
    }
  }

  /* The file exists - it's open, so lets see what's inside */

  fread(&config, sizeof(struct elite_config), 1, fl);
  if (ferror(fl)) {
    log("Error reading Elite config file - restoring defaults.");
    fclose(fl);
    default_elite_config();
    return;
  }

  pk_allowed = config.pk_allowed;
  pt_allowed = config.pt_allowed;
  pkok_allowed = config.pkok_allowed;
  loot_allowed = config.loot_allowed;
  max_recall_level = config.max_recall_level;
  level_can_chat = config.level_can_chat;
  holler_move_cost = config.holler_move_cost;
  max_bank_gold = config.max_bank_gold;
  max_npc_corpse_time = config.max_npc_corpse_time;
  max_pc_corpse_time = config.max_pc_corpse_time;
  dts_are_dumps = config.dts_are_dumps;
  max_obj_save = config.max_obj_save;
  newbie_clan = config.newbie_clan;
  auto_save = config.auto_save;
  autosave_time = config.autosave_time;
  use_autowiz = config.use_autowiz;
  min_wizlist_lev = config.min_wizlist_lev;

  log("Elite configured from misc/Elite.cfg");
}


void write_elite_config()
{
  FILE *fl;

  if (!(fl = fopen("misc/Elite.cfg", "wb"))) {
    log("Unable to open Elite config file for writing.");
    return;
  }

  config.pk_allowed = pk_allowed;
  config.pt_allowed = pt_allowed;
  config.pkok_allowed = pkok_allowed;
  config.loot_allowed = loot_allowed;
  config.max_recall_level = max_recall_level;
  config.level_can_chat = level_can_chat;
  config.holler_move_cost = holler_move_cost;
  config.max_bank_gold = max_bank_gold;
  config.max_npc_corpse_time = max_npc_corpse_time;
  config.max_pc_corpse_time = max_pc_corpse_time;
  config.dts_are_dumps = dts_are_dumps;
  config.max_obj_save = max_obj_save;
  config.newbie_clan = newbie_clan;
  config.auto_save = auto_save;
  config.autosave_time = autosave_time;
  config.use_autowiz = use_autowiz;
  config.min_wizlist_lev = min_wizlist_lev;

  if (fwrite(&config, sizeof(struct elite_config), 1, fl) < 1) {
    perror("Error writing Elite config file.");
    fclose(fl);
    return;
  }

  fclose(fl);
  log("Elite config file updated.");

}


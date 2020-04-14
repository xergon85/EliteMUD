/* ************************************************************************
*   File: db.h                                          Part of CircleMUD *
*  Usage: header file for database handling                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD	0
#define DB_BOOT_MOB	1
#define DB_BOOT_OBJ	2
#define DB_BOOT_ZON	3
#define DB_BOOT_SHP	4
#define DB_BOOT_CLN     5

/* names of various files and directories */

#define INDEX_FILE	"index"		/* index of world files		*/
#define MINDEX_FILE	"index.mini"	/* ... and for mini-mud-mode	*/
#define WLD_PREFIX	"world/wld"	/* room definitions		*/
#define MOB_PREFIX	"world/mob"	/* monster prototypes		*/
#define OBJ_PREFIX	"world/obj"	/* object prototypes		*/
#define ZON_PREFIX	"world/zon"	/* zon defs & command tables	*/
#define SHP_PREFIX	"world/shp"	/* shop definitions		*/
#define CLN_PREFIX      "world/cln"     /* clan defenitions   -Petrus   */
#define MOB_DIR		"world/prg"	/* Programs                     */
#define INCLUDE_DIR     "world/prg"     /* Include files                */
#define CREDITS_FILE	"text/credits"	/* for the 'credits' command	*/
#define NEWS_FILE	"text/news"	/* for the 'news' command	*/
#define MOTD_FILE	"text/motd"	/* messages of the day / mortal	*/
#define IMOTD_FILE	"text/imotd"	/* messages of the day / immort	*/
#define IDEA_FILE	"text/ideas"	/* for the 'idea'-command	*/
#define TYPO_FILE	"text/typos"	/*         'typo'		*/
#define BUG_FILE	"text/bugs"	/*         'bug'		*/
#define HELP_KWRD_FILE	"text/help_table"/* for HELP <keywrd>		*/
#define HELP_PAGE_FILE	"text/help"	/* for HELP <CR>		*/
#define INFO_FILE	"text/info"	/* for INFO			*/
#define NEWBIE_FILE     "text/newbie"   /* newbie                       */
#define WIZLIST_FILE	"text/wizlist"	/* for WIZLIST			*/
#define IMMLIST_FILE	"text/immlist"	/* for IMMLIST			*/
#define REMLIST_FILE    "text/remlist"  /* for REMLIST                  */
#define BACKGROUND_FILE	"text/background" /* for the background story	*/
#define POLICIES_FILE	"text/policies"	/* player policies/rules	*/
#define HANDBOOK_FILE	"text/handbook"	/* handbook for new immorts	*/
#define IDEAS_FILE      "text/ideas"    /* ideas file                   */
#define BUGS_FILE       "text/bugs"     /* bugs file                    */
#define TYPOS_FILE      "text/typos"    /* typos file                   */

#define PLAYER_FILE	"misc/players"	/* the player database		*/
#define MESS_FILE	"misc/messages"	/* damage message		*/
#define SOCMESS_FILE	"misc/socials"	/* messgs for social acts	*/
#define BAN_FILE	"misc/badsites"	/* for the siteban system	*/
#define XNAME_FILE	"misc/xnames"	/* invalid name substrings	*/
#define CLAN_FILE       "misc/clanpower" /* clan power record           */
#define DISABLED_FILE   "misc/disabled.cmds" /* disabled commands       */


#define REAL 0
#define VIRTUAL 1

/* For special assign system */
#define  SPEC_ROOM     1
#define  SPEC_MOB      2
#define  SPEC_OBJ      4


/* structure for the reset commands */
struct reset_com {
   char	command;   /* current command                      */

   bool if_flag;   /* if TRUE: exe only if preceding exe'd */
   int	arg1;       /*                                      */
   int	arg2;       /* Arguments to the command             */
   int	arg3;       /*                                      */

   /* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*  'R': Random the room's exits *
   */
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data {
   char	*name;		    /* name of this zone                  */
   int	lifespan;           /* how long between resets (minutes)  */
   int	age;                /* current age of this zone (minutes) */
   int	top;                /* upper limit for rooms in this zone */

   int lowest;              /* real num of first room */
   int highest;             /* real num of last room*/

   int	reset_mode;         /* conditions for reset (see below)   */
   int	number;		    /* virtual number of this zone	  */
   struct reset_com *cmd;   /* command table for reset	          */

   /*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
   */
};


/* for queueing zones for update   */
struct reset_q_element {
   int	zone_to_reset;            /* ref to zone_data */
   struct reset_q_element *next;
};



/* structure for the update queue     */
struct reset_q_type {
   struct reset_q_element *head;
   struct reset_q_element *tail;
};



struct player_index_element {
   char	*name;
   sbyte level;
   long worships;
   long idnum;
};


struct help_index_element {
   char	*keyword;
   long	pos;
};

/* don't change these */
#define BAN_NOT 	0
#define BAN_NEW 	1
#define BAN_SELECT	2
#define BAN_ALL		3

#define BANNED_SITE_LENGTH    50
struct ban_list_element {
   char	site[BANNED_SITE_LENGTH+1];
   int	type;
   long	date;
   char	name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next;
};


/* for disabled commands */
#define END_MARKER	"END" /* for load_disabled() and save_disabled() */

typedef struct disabled_data DISABLED_DATA;

extern DISABLED_DATA *disabled_first; /* interpreter.c */

/* one disabled command */
struct disabled_data {
   DISABLED_DATA *next; 		/* pointer to next node 	*/
   struct command_info const *command;  /* pointer to the command struct*/
   char  *disabled_by; 		        /* name of disabler 		*/
   sh_int level; 			/* level of disabler 		*/
   int subcmd;				/* the subcmd, if any		*/
};


/* global buffering system */

#ifdef __DB_C__
char	buf[MAX_STRING_LENGTH];
char	buf1[MAX_STRING_LENGTH];
char	buf2[MAX_STRING_LENGTH];
char    err_buf[MAX_STRING_LENGTH];
char	arg[MAX_STRING_LENGTH];
#else
extern char	buf[MAX_STRING_LENGTH];
extern char	buf1[MAX_STRING_LENGTH];
extern char	buf2[MAX_STRING_LENGTH];
extern char	arg[MAX_STRING_LENGTH];
#endif


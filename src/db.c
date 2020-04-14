/*************************************************************************
*   File: db.c                                          Part of EliteMUD  *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#define __DB_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "history.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "ocs.h"
#include "functions.h"
#include "shop.h"


/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */
struct room_data **world = 0;         /* array of rooms			*/
int	top_of_world = 0;            /* ref to the top element of world	*/
struct room_list *room_crash_list = 0; /* List with room to be crash-saved */
struct room_affect_type *room_affect_list = 0;  /*list of rooms with magic */

/* FOR THE OCS SYSTEM */
int     ocs_rooms  = 0;
int     ocs_room_buffer = 0;
int     ocs_objs   = 0;
int     ocs_obj_buffer = 0;
int     ocs_mobs   = 0;
int     ocs_mob_buffer = 0;
int     ocs_zones  = 0;
int     ocs_zone_buffer = 0;
int     ocs_shops  = 0;
int     ocs_shop_buffer = 0;

/**********************/

struct char_data *character_list = 0; /* global linked list of chars	*/
struct char_data **mob_list = &character_list;   /* New Petrus */
struct index_data *mob_index;		/* index table for mobile file	*/
struct char_data *mob_proto;		/* prototypes for mobs		*/
int	top_of_mobt = 0;		/* top of mobile index table	*/

struct obj_data *object_list = 0;    /* the global linked list of objs	*/
struct index_data *obj_index;		/* index table for object file	*/
struct obj_data *obj_proto;		/* prototypes for objs		*/
int	top_of_objt = 0;		/* top of object index table	*/
struct obj_data *questeq_list = 0;   /* linked list of quest objs       */
struct obj_data *claneq_list = 0;    /* linked list of clan objs        */

struct zone_data *zone_table;	     /* zone table			*/
int	top_of_zone_table = 0;	     /* ref to top element of zone tab	*/

struct clan_data *clan_list = 0;     /* array of clans                  */
int     top_of_clan = 0;

struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages	*/

struct player_index_element *player_table = 0; /* index to player file	*/
FILE *player_fl = 0;			/* file desc of player file	*/
int	top_of_p_table = 0;		/* ref to top of table		*/
int	top_of_p_file = 0;		/* ref of size of p file	*/
long	top_idnum = 0;			/* highest idnum in use		*/

int	no_mail = 0;			/* mail disabled?		*/
int	mini_mud = 0;			/* mini-mud mode?		*/
int	no_rent_check = 0;		/* skip rent check on boot?	*/
long	boot_time = 0;			/* time of mud boot		*/
int	restrict = 0;			/* level of game restriction	*/
sh_int	r_mortal_start_room;		/* rnum of mortal start room	*/
sh_int	r_newbie_start_room;		/* rnum of newbie start room	*/
sh_int  r_newbie_corpse_room;            /* rnum of newbie corpse room   */
sh_int	r_immort_start_room;		/* rnum of immort start room	*/
sh_int	r_frozen_start_room;		/* rnum of frozen start room	*/
int     r_newbie_clan;                  /* rnum of newbie clan          */
sh_int  r_wargame_start_room;           /* rnum of wargame start room   */

char	*credits = 0;			/* game credits			*/
char	*news = 0;			/* mud news			*/
char	*motd = 0;			/* message of the day - mortals */
char	*imotd = 0;			/* message of the day - immorts */
char	*help = 0;			/* help screen			*/
char	*info = 0;			/* info page			*/
char    *newbie = 0;                    /* Instant Newbie help          */
char	*wizlist = 0;			/* list of higher gods		*/
char	*immlist = 0;			/* list of peon gods		*/
char    *remlist = 0;                   /* list of remorts              */
char	*background = 0;		/* background story		*/
char	*handbook = 0;			/* handbook for new immortals	*/
char	*policies = 0;			/* policies page		*/
char    *ideas = 0;                     /* ideas by players             */
char    *bugs = 0;                      /* bugs reported                */
char    *typos = 0;                     /* typos reported               */
extern struct resistance
npc_resistance_table[];                 /* New general mob resists */

FILE *help_fl = 0;			/* file for help text		*/
struct help_index_element *help_index = 0; /* the help table		*/
int	top_of_helpt;			/* top of help index table	*/

struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */

struct help_index_element *build_help_index(FILE *fl, int *num);

extern struct descriptor_data *descriptor_list;

extern int height_average[];
extern int weight_average[];

extern char     *clan_ranks[][3];

extern sh_int donation_room_1;

extern void read_elite_config();
extern void load_disabled(void);
extern sbyte channel_allowed[];


int check_pc(int theroom);
int update_rprog_check(struct room_data *room);

struct reset_q_type reset_q;
void rprog_read_programs(FILE *fp, struct room_data *room, char *name);

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* this is necessary for the autowiz system */
void	reboot_wizlists(void)
{
   file_to_string_alloc(WIZLIST_FILE, &wizlist);
   file_to_string_alloc(IMMLIST_FILE, &immlist);
}


ACMD(do_reboot)
{
   int	i;

   one_argument(argument, arg);

   if (!str_cmp(arg, "all") || *arg == '*') {
      file_to_string_alloc(NEWS_FILE, &news);
      file_to_string_alloc(CREDITS_FILE, &credits);
      file_to_string_alloc(MOTD_FILE, &motd);
      file_to_string_alloc(IMOTD_FILE, &imotd);
      file_to_string_alloc(HELP_PAGE_FILE, &help);
      file_to_string_alloc(INFO_FILE, &info);
      file_to_string_alloc(NEWBIE_FILE, &newbie);
      file_to_string_alloc(WIZLIST_FILE, &wizlist);
      file_to_string_alloc(IMMLIST_FILE, &immlist);
      file_to_string_alloc(REMLIST_FILE, &remlist);
      file_to_string_alloc(POLICIES_FILE, &policies);
      file_to_string_alloc(HANDBOOK_FILE, &handbook);
      file_to_string_alloc(BACKGROUND_FILE, &background);
  } else if (!str_cmp(arg, "wizlist"))
      file_to_string_alloc(WIZLIST_FILE, &wizlist);
   else if (!str_cmp(arg, "immlist"))
      file_to_string_alloc(IMMLIST_FILE, &immlist);
   else if (!str_cmp(arg, "remlist"))
      file_to_string_alloc(REMLIST_FILE, &remlist);
   else if (!str_cmp(arg, "news"))
      file_to_string_alloc(NEWS_FILE, &news);
   else if (!str_cmp(arg, "credits"))
      file_to_string_alloc(CREDITS_FILE, &credits);
   else if (!str_cmp(arg, "motd"))
      file_to_string_alloc(MOTD_FILE, &motd);
   else if (!str_cmp(arg, "imotd"))
      file_to_string_alloc(IMOTD_FILE, &imotd);
   else if (!str_cmp(arg, "help"))
      file_to_string_alloc(HELP_PAGE_FILE, &help);
   else if (!str_cmp(arg, "info"))
      file_to_string_alloc(INFO_FILE, &info);
   else if (!str_cmp(arg, "newbie"))
      file_to_string_alloc(NEWBIE_FILE, &newbie);
   else if (!str_cmp(arg, "policy"))
      file_to_string_alloc(POLICIES_FILE, &policies);
   else if (!str_cmp(arg, "handbook"))
      file_to_string_alloc(HANDBOOK_FILE, &handbook);
   else if (!str_cmp(arg, "background"))
      file_to_string_alloc(BACKGROUND_FILE, &background);
   else if (!str_cmp(arg, "xhelp")) {
      if (help_fl)
	 fclose(help_fl);
      if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
	 return;
      else {
	 for (i = 0; i < top_of_helpt; i++)
	    free(help_index[i].keyword);
	 free(help_index);
	 help_index = build_help_index(help_fl, &top_of_helpt);
      }
   } else {
      send_to_char("Unknown reboot option.\r\n", ch);
      return;
   }

   send_to_char("Okay.\r\n", ch);
}


/* body of the booting system */
void	boot_db(void)
{
   int	i;
   extern int	no_specials;
   extern int newbie_clan;

   log("Boot db -- BEGIN.");

   log("Resetting the game time:");
   reset_time();

   log("Reading news, credits, help, bground, info & motds.");
   file_to_string_alloc(NEWS_FILE, &news);
   file_to_string_alloc(CREDITS_FILE, &credits);
   file_to_string_alloc(MOTD_FILE, &motd);
   file_to_string_alloc(IMOTD_FILE, &imotd);
   file_to_string_alloc(HELP_PAGE_FILE, &help);
   file_to_string_alloc(INFO_FILE, &info);
   file_to_string_alloc(NEWBIE_FILE, &newbie);
   file_to_string_alloc(WIZLIST_FILE, &wizlist);
   file_to_string_alloc(IMMLIST_FILE, &immlist);
   file_to_string_alloc(REMLIST_FILE, &remlist);
   file_to_string_alloc(POLICIES_FILE, &policies);
   file_to_string_alloc(HANDBOOK_FILE, &handbook);
   file_to_string_alloc(BACKGROUND_FILE, &background);
   file_to_string_alloc(IDEAS_FILE, &ideas);
   file_to_string_alloc(BUGS_FILE, &bugs);
   file_to_string_alloc(TYPOS_FILE, &typos);

   log("Loading config settings.");
   read_elite_config();

   log("Loading disabled commands list.");
   load_disabled();

   log("Configuring Global Channels.");
   for (i = 0; i < 8; i++)
    channel_allowed[i] = 1; // Default = All Channels on - Bod
 
   log("Opening help file.");
   if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
     log("   Could not open help file.");
   else
     help_index = build_help_index(help_fl, &top_of_helpt);

   log("Loading zone table.");
   index_boot(DB_BOOT_ZON);

   log("Loading rooms.");
   index_boot(DB_BOOT_WLD);

   log("Arranging zones.");
   arrange_zones();

   log("Renumbering rooms.");
   renum_world();

   log("Checking start rooms.");
   check_start_rooms();

   log("Setup Crash Rooms.");
   setup_crashrooms();

   log("Loading mobs and generating index.");
   index_boot(DB_BOOT_MOB);

   log("Loading objs and generating index.");
   index_boot(DB_BOOT_OBJ);

   log("Setting up Quest eq list.");
   boot_questeq();

   log("Renumbering zone table.");
   renum_zone_table();

   log("Loading clans and generating index.");
   index_boot(DB_BOOT_CLN);

   boot_clanpower();

   if ((r_newbie_clan = real_clan(newbie_clan)) >= 0) {
     sprintf(buf, "Newbie Clan Enabled: %d [%d]", newbie_clan, r_newbie_clan);
     log(buf);
   } else {
     log("SYSERR: Newbie Clan Disabled");
   }

   log("Generating player index.");
   build_player_index();

   log("Loading fight messages.");
   load_messages();

   log("Loading social messages.");
   boot_social_messages();

   if (!no_specials) {
     log("Loading shops.");
     index_boot(DB_BOOT_SHP);
   }

   log("Assigning function pointers:");

   if (!no_specials) {
     log("   Shopkeepers.");
     assign_the_shopkeepers();
     log("   Mobiles.");
     assign_mobiles();
     log("   Objects.");
     assign_objects();
     log("   Rooms.");
     assign_rooms();
   }

   log("   Spells.");
   assign_spell_pointers();

   log("Sorting command list.");
   sort_commands();

   /*
      log("Booting mail system.");
      if (!scan_file()) {
	log("    Mail boot failed -- Mail system disabled");
	no_mail = 1;
      }*/

   log("Reading banned site and invalid-name list.");
   load_banned();
   Read_Invalid_List();

   log("Restoring crash files...");
   update_obj_file();

   for (i = 0; i <= top_of_zone_table; i++) {
     sprintf(buf2, "Resetting %s (rooms %d-%d).",
	     zone_table[i].name,
	     (i ? (zone_table[i - 1].top + 1) : 0),
	     zone_table[i].top);
     log(buf2);
     reset_zone(i, FALSE);
   }

   log("Retrieving Crash-saved objs to rooms.");
   Crash_load_rooms();

   reset_q.head = reset_q.tail = 0;

   boot_time = time(0);

   MOBTrigger = TRUE;

   log("Boot db -- DONE.");
 }


/* reset the time in the game from file */
void	reset_time(void)
{
  long	beginning_of_time = 650336715;

  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);


  switch (time_info.hours) {
  case 0 : case 1 : case 2 : case 3 : case 4 :
    weather_info.sunlight = SUN_DARK;
    break;
  case 5 :
    weather_info.sunlight = SUN_RISE;
    break;
  case 6 : case 7 : case 8 : case 9: case 10 : case 11 : case 12 : case 13 :
  case 14 : case 15 : case 16 : case 17 : case 18 : case 19 : case 20 :
    weather_info.sunlight = SUN_LIGHT;
    break;
  case 21 :
    weather_info.sunlight = SUN_SET;
    break;
  case 22 : case 23 :
  default :
    weather_info.sunlight = SUN_DARK;
    break;
  }

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.",
	  time_info.hours, time_info.day,
	  time_info.month, time_info.year);
  log(buf);

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12))
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.pressure <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.pressure <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;
}




/* generate index table for the player file */
void	build_player_index(void)
{
  int	nr = -1, i;
  long	size, recs;
  struct char_file_u dummy;
  char **ptr;

  if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
    perror("Error opening playerfile");
    exit(1);
  }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "WARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    sprintf(buf, "   %ld players in database.", recs);
    log(buf);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = 0;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl); ) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl))	/* new record */ {
      nr++;
      /*
       * Don't make it lower case - Charlene
       * CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
       * for (i = 0;
       *   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
       */
      /* fix bug with uppercase names - Charlene */
      player_table[nr].name = strdup(dummy.name);
      top_idnum = MAX(top_idnum, dummy.specials2.idnum);

      /* worships system -Petrus */
      if((player_table[nr].level = dummy.level) >= LEVEL_WORSHIP)
	player_table[nr].worships = dummy.specials2.idnum;
      else
	player_table[nr].worships = dummy.specials2.worships;

      /* Set idnum so we can get name of player easily from idnum - Charlene */
      player_table[nr].idnum = dummy.specials2.idnum;

      /* clan system -Petrus */
      if (dummy.specials2.clan >= 0 &&
	  dummy.specials2.clanlevel > 1 &&
	  (i = real_clan(dummy.specials2.clan)) != -1) {

	if (dummy.level >= LEVEL_DEITY) /* deity */
	  clan_list[i].gods++;
	else /* Disabled NODELETE flagged players not counting towards clan stats */
	   /*  if (!IS_SET(dummy.specials2.act, PLR_NODELETE)) { */
		  if (dummy.specials2.remorts != 0) { /* remort */
		       clan_list[i].remorts++;
		       clan_list[i].wealth += ((dummy.points.gold + dummy.points.bank_gold)/1000);

		       clan_list[i].level += dummy.level;
		       clan_list[i].power += (dummy.level + dummy.specials2.remorts *
					      109);
		       clan_list[i].members++;
		  }
		  else { /* mortal */
		       clan_list[i].mortals++;
		       clan_list[i].wealth += ((dummy.points.gold + dummy.points.bank_gold)/1000);

		       clan_list[i].level += dummy.level;
		       clan_list[i].power += dummy.level;
		       clan_list[i].members++;
		  }
	   /*  } */
	/* replaced leader list with roster */
	if (dummy.specials2.clanlevel > 0) {
	  ptr = &(clan_list[i].roster[dummy.specials2.clanlevel]);
	  if (!*ptr)
	    CREATE((*ptr), char, strlen(dummy.name) + 3);
	  else if (!((*ptr) =
		     (char *) realloc((*ptr), (strlen(*ptr) + strlen(dummy.name) + 3) * sizeof(char)))) {
	    perror("Couldn't realloc.  (build_player_index, db.c)");
	    exit(0);
	  }

	  sprintf(*ptr, "%s%s  ", *ptr, dummy.name);
	}

      }
    }
  }
  top_of_p_file = top_of_p_table = nr;
}


/* function to count how many hash-mark delimited records exist in a file */
int	count_hash_records(FILE *fl)
{
  char	buf[120];
  int	count = 0;

  while (fgets(buf, 120, fl))
    if (*buf == '#')
      count++;

  return (count - 1);
}



void	index_boot(int mode)
{
  char	*index_filename, *prefix;
  FILE * index, *db_file;
  int	rec_count = 0;

  switch (mode) {
  case DB_BOOT_WLD	: prefix = WLD_PREFIX; break;
  case DB_BOOT_MOB	: prefix = MOB_PREFIX; break;
  case DB_BOOT_OBJ	: prefix = OBJ_PREFIX; break;
  case DB_BOOT_ZON	: prefix = ZON_PREFIX; break;
  case DB_BOOT_SHP	: prefix = SHP_PREFIX; break;
  case DB_BOOT_CLN     : prefix = CLN_PREFIX; break;
  default:
    log("SYSERR: Unknown subcommand to index_boot!");
    exit(1);
    break;
  }

  if (mini_mud)
    index_filename = MINDEX_FILE;
  else
    index_filename = INDEX_FILE;

  sprintf(buf2, "%s/%s", prefix, index_filename);

  if (!(index = fopen(buf2, "r"))) {
    sprintf(buf1, "Error opening index file '%s'", buf2);
    perror(buf1);
    exit(1);
  }

  /* first, count the number of records in the file so we can malloc */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      exit(1);
    } else {
      if (mode == DB_BOOT_ZON)
	rec_count++;
      else
	rec_count += count_hash_records(db_file);
    }
    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  if (!rec_count) {
    log("SYSERR: boot error - 0 records counted");
    exit(1);
  }

  rec_count++;

  switch (mode) {
  case DB_BOOT_WLD :
    CREATE(world, struct room_data *, rec_count + OCS_ROOMS);
    break;
  case DB_BOOT_MOB :
    CREATE(mob_proto, struct char_data, rec_count);
    CREATE(mob_index, struct index_data, rec_count);
    break;
  case DB_BOOT_OBJ :
    CREATE(obj_proto, struct obj_data, rec_count);
    CREATE(obj_index, struct index_data, rec_count);
    break;
  case DB_BOOT_ZON :
    CREATE(zone_table, struct zone_data, rec_count);
    break;
  case DB_BOOT_CLN:
    CREATE(clan_list, struct clan_data, rec_count);
    break;
  case DB_BOOT_SHP:
    CREATE(shop_index, struct shop_data, rec_count);
    break;
  }


  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      exit(1);
    }

    switch (mode) {
    case DB_BOOT_WLD	: load_rooms(db_file); break;
    case DB_BOOT_OBJ	: load_objects(db_file); break;
    case DB_BOOT_MOB	: load_mobiles(db_file); break;
    case DB_BOOT_ZON	: load_zones(db_file); break;
    case DB_BOOT_SHP	: boot_the_shops(db_file, buf2); break;
    case DB_BOOT_CLN  : load_clans(db_file); break;
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }
}

void boot_questeq()
{
  int nr, current;
  struct obj_data *obj;

  for (current = (LEVEL_DEITY-1); current >= 0; current--)
    for (nr = 0; nr <= top_of_objt; nr++)
      if (obj_proto[nr].obj_flags.level == current)
	if (IS_SET(obj_proto[nr].obj_flags.extra_flags, ITEM_QUEST)) {
	  CREATE(obj, struct obj_data, 1);
	  *obj = obj_proto[nr];
	  obj->next = questeq_list;
	  questeq_list = obj;
	}
}

void boot_clanpower()
{
  int i;
  long j;
  FILE *clan_f;

  if (!(clan_f = fopen(CLAN_FILE, "r"))) {
    log("SYSERR: Unable to open clan power file");
    return;
  }

  for (;;) {
    i = 0; j = 0;
    if (fscanf(clan_f, "#%d A%ld\n", &i, &j) < 0) {
      log("SYSERR: Corrupt clanpower info file.");
      return;
    }
    if (i != 99999) {
      if ((i = real_clan(i)) >= 0)
	clan_list[i].on_power_rec = j;
    } else {
      fclose(clan_f);
      return;
    }
  }
}

/********************************************************
 * STRING SHARING UNIT - (C) 1994 Mr Wang RIT           *
 ********************************************************/

#define ROOM_NAME           1
#define ROOM_DESC           2
#define ROOM_EXIT_DESC      3
#define ROOM_EXIT_KEYWORDS  4
#define ROOM_EXTRA_DESC     5
#define ROOM_EXTRA_KEYWORDS 6

#define MOB_NAME            7
#define MOB_SHORT_DESC      8
#define MOB_LONG_DESC       9
#define MOB_DESC            10

#define OBJ_NAME            11
#define OBJ_SHORT_DESC      12
#define OBJ_DESC            13
#define OBJ_EXTRA_DESC      14
#define OBJ_EXTRA_KEYWORDS  15

char * fread_share_string(FILE *fl, char *errorbuf, int num, int type)
{
  char *str;
  int vnum, rnum, dir;

  str = fread_string(fl, errorbuf);

  if (!str || *str != '^')
    return str;

  dir = atoi(one_argument(str, buf));
  vnum = atoi(buf + 1);

  if (dir < 0 || dir > 5 || !vnum) {
    fprintf(stderr, "String sharing error at or near %s", errorbuf);
    exit(0);
  }

  free(str);

  switch (type) {
  case ROOM_NAME:case ROOM_DESC:case ROOM_EXIT_DESC:case ROOM_EXIT_KEYWORDS:
  case ROOM_EXTRA_DESC:case ROOM_EXTRA_KEYWORDS:
    for (rnum = num;rnum >= 0;rnum--)
      if (world[rnum]->number == vnum)
	break;
    break;
  case MOB_NAME:case MOB_SHORT_DESC:case MOB_LONG_DESC:case MOB_DESC:
    for (rnum = num;rnum >= 0;rnum--)
      if (mob_proto[rnum].nr == vnum)
	break;
    break;
  case OBJ_NAME:case OBJ_SHORT_DESC:case OBJ_DESC:
  case OBJ_EXTRA_DESC:case OBJ_EXTRA_KEYWORDS:
    for (rnum = num;rnum >= 0;rnum--)
      if (obj_index[rnum].virtual == vnum)
	break;
    break;
  default:
    fprintf(stderr, "String sharing error (Unknown type) at or near %s",
	    errorbuf);
    exit(0);
    break;
  }

  switch (type) {

  case ROOM_NAME:
    str = world[rnum]->name; break;
  case ROOM_DESC:
    str = world[rnum]->description; break;
  case ROOM_EXIT_DESC:
    str = world[rnum]->dir_option[dir]->general_description; break;
  case ROOM_EXIT_KEYWORDS:
    str = world[rnum]->dir_option[dir]->keyword; break;
  case ROOM_EXTRA_DESC:
    str = world[rnum]->ex_description->description; break;
  case ROOM_EXTRA_KEYWORDS:
    str = world[rnum]->ex_description->keyword; break;
  case MOB_NAME:
    str = mob_proto[rnum].player.name; break;
  case MOB_SHORT_DESC:
    str = mob_proto[rnum].player.short_descr; break;
  case MOB_LONG_DESC:
    str = mob_proto[rnum].player.long_descr; break;
  case MOB_DESC:
    str = mob_proto[rnum].player.description; break;
  case OBJ_NAME:
    str = obj_proto[rnum].name; break;
  case OBJ_SHORT_DESC:
    str = obj_proto[rnum].short_description; break;
  case OBJ_DESC:
    str = obj_proto[rnum].description; break;
  case OBJ_EXTRA_DESC:
    str = obj_proto[rnum].ex_description->description; break;
  case OBJ_EXTRA_KEYWORDS:
    str = obj_proto[rnum].ex_description->keyword; break;
  default:
    str = "String Sharing Error\r\n";
  }

  return str;
}

/*****************************************************************************/


void    arrange_zones()
{
  int i, zone = 0;

  for (i = 0; i < top_of_world;i++) {
    if (world[i]->number > zone_table[zone].top) {
      if (++zone > top_of_zone_table) {
	fprintf(stderr, "Room num %d is above any zone.\n",
		world[i]->number);
	exit(0);
      }
      if (world[i]->number < (zone?zone_table[zone-1].top:-1)) {
	fprintf(stderr, "Room num %d is below zone %d.\n",
		world[i]->number, zone);
	exit(0);
      }
    }

    if (world[i]->zone != zone) {
      fprintf(stderr, "Warning: Room num %d in zone %d has incorrect zone num (%d).\n", world[i]->number, zone, world[i]->zone);
    }

    if (i < zone_table[zone].lowest)
      zone_table[zone].lowest = i;
    if (i > zone_table[zone].highest)
      zone_table[zone].highest = i;
  }
}

/* load the rooms */
void	load_rooms(FILE *fl)
{
  static int	room_nr = 0, virtual_nr, flag, tmp;
  char	*temp, chk[50];
  struct extra_descr_data *new_descr;
  struct room_list *room_ls;
  struct room_data *room;

  do {
    fscanf(fl, " #%d\n", &virtual_nr);

    sprintf(buf2, "room #%d", virtual_nr);

    CREATE(room, struct room_data, 1);
    room->rprogs = NULL;

    temp = fread_string(fl, buf2);
    if ((flag = (*temp != '$'))) { /* a new record to be read */

      room->number = virtual_nr;
      room->name = temp;
      room->description = fread_string(fl, buf2);

      room->zone = real_zone(fread_number(fl, buf2));

      room->room_flags = fread_number(fl, buf2);
      room->sector_type = fread_number(fl, buf2);

      if (room->sector_type < 0 || room->sector_type >= SECT_MAX) {
        sprintf(buf, "Invalid Sector Type (%d) at [%d] set to 0",
                room->sector_type, room->number);
        room->sector_type = 0;
      }

      if (IS_SET(room->room_flags, OCS))
	ocs_rooms++;

      room->funct = 0;
      room->contents = 0;
      room->people = 0;
      room->light = 0;		/* Zero light sources */

      for (tmp = 0; tmp <= 5; tmp++)
	room->dir_option[tmp] = 0;

      room->ex_description = 0;

      for (; ; ) {
	fscanf(fl, " %s \n", chk);

	if (*chk == 'D')	/* direction field */
	  setup_dir(fl, room, room_nr, atoi(chk + 1));
	else if (*chk == 'E')	/* extra description field */ {
	  CREATE(new_descr, struct extra_descr_data, 1);
	  new_descr->keyword = fread_string(fl, buf2);
	  new_descr->description = fread_string(fl, buf2);
	  new_descr->next = room->ex_description;
	  room->ex_description = new_descr;
	} else if (*chk == 'C') {
	  CREATE(room_ls, struct room_list, 1);
	  room_ls->number = virtual_nr;
	  room_ls->next = room_crash_list;
	  room_crash_list = room_ls;
	} else if (*chk == 'P') {
	  special_assign(chk, SPEC_ROOM, &room->funct, buf2);
	} else if (*chk == '>') {
	  /* New Room Prog -Helm */
	  rprog_read_programs(fl, room, (chk+1));
	} else if (*chk == 'S')	/* end of current room */
	  break;
      }

      if (!room_nr)
	world[room_nr] = room;
      for (tmp = room_nr - 1; tmp >= 0; tmp--)
	if (world[tmp]->number < virtual_nr) {
	  move_roomblock(tmp+1, room_nr);
	  world[tmp + 1] = room;
	  break;
	} else if (world[tmp]->number == virtual_nr) {
	  free_room(world[tmp]);
	  world[tmp] = room;
	  room_nr--;
	  break;
	}

      room_nr++;

    } else
      free(room);

  } while (flag);

  free(temp);			/* cleanup the area containing the terminal $  */

  top_of_world = room_nr - 1;
}


/* setup crash_rooms */
void setup_crashrooms()
{
  struct room_list *room_ls;

  room_ls = room_crash_list;

  while (room_ls) {
    room_ls->number = real_room(room_ls->number);
    room_ls = room_ls->next;
  }
}



/* read direction data */
void	setup_dir(FILE *fl,struct room_data *room, int nr, int dir)
{
  int	tmp;

  sprintf(buf2, "Room #%d, direction D%d", room->number, dir);

  CREATE(room->dir_option[dir], struct room_direction_data , 1);

  room->dir_option[dir]->general_description =
    fread_string(fl, buf2);
  room->dir_option[dir]->keyword = fread_string(fl, buf2);

  tmp = fread_number(fl, buf2);
  if (tmp == 1)
    room->dir_option[dir]->exit_info = EX_ISDOOR;
  else if (tmp == 2)
    room->dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else if (tmp == 0)
    room->dir_option[dir]->exit_info = 0;
    else
    room->dir_option[dir]->exit_info = tmp;

  fscanf(fl, " %d ", &tmp);
  room->dir_option[dir]->key = tmp;

  fscanf(fl, " %d ", &tmp);
  room->dir_option[dir]->to_room = tmp;
}


void	check_start_rooms(void)
{
  extern sh_int mortal_start_room;
  extern sh_int newbie_start_room;
  extern sh_int newbie_corpse_room;
  extern sh_int immort_start_room;
  extern sh_int frozen_start_room;
  extern sh_int wargame_start_room;

  if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
    log("SYSERR:  Mortal start room does not exist.  Change in config.c.");
    exit(1);
  }
  if ((r_newbie_start_room = real_room(newbie_start_room)) < 0) {
    log("SYSERR:  Newbie start room does not exist.  Change in config.c. Setting to mortal start room.");
    r_newbie_start_room = r_mortal_start_room;
  }

  if ((r_newbie_corpse_room = real_room(newbie_corpse_room)) < 0) {
    log("SYSERR:  Newbie corpse room does not exist.  Change in config.c. Setting to mortal start room.");
    r_newbie_corpse_room = r_mortal_start_room;
  }

  if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
    r_immort_start_room = r_mortal_start_room;
  }

  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
    r_frozen_start_room = r_mortal_start_room;
  }

  if ((r_wargame_start_room = real_room(wargame_start_room)) < 0) {
    log("SYSERR: Warning: Wargames disabled! Start room does not exist.  Change in config.c");
  }
}



void	renum_world(void)
{
  register int	room, door;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door <= 5; door++)
      if (world[room]->dir_option[door])
	if (world[room]->dir_option[door]->to_room != NOWHERE)
	  world[room]->dir_option[door]->to_room =
	    real_room(world[room]->dir_option[door]->to_room);
}


void    check_reset_cmds(struct reset_com *ptr, char *errorbuf)
{
  int comm, a, b;

  for (comm = 0; ptr[comm].command != 'S'; comm++) {
    a = b = 0;
    switch (ptr[comm].command) {
    case 'M':
      a = ptr[comm].arg1 =
	real_mobile(ptr[comm].arg1);
      b = ptr[comm].arg3 =
	real_room(ptr[comm].arg3);
      break;
    case 'O':
      a = ptr[comm].arg1 =
	real_object(ptr[comm].arg1);
      if (ptr[comm].arg3 != NOWHERE)
	b = ptr[comm].arg3 =
	  real_room(ptr[comm].arg3);
      break;
    case 'G':
      a = ptr[comm].arg1 =
	real_object(ptr[comm].arg1);
      break;
    case 'E':
      a = ptr[comm].arg1 =
	real_object(ptr[comm].arg1);
      break;
    case 'P':
      a = ptr[comm].arg1 =
	real_object(ptr[comm].arg1);
      b = ptr[comm].arg3 =
	real_object(ptr[comm].arg3);
      break;
    case 'D':
      a = ptr[comm].arg1 =
	real_room(ptr[comm].arg1);
      break;
    case 'R':
      a = ptr[comm].arg1 =
	real_room(ptr[comm].arg1);
      break;
    case 'L':
      a = ptr[comm].arg1 =
	real_room(ptr[comm].arg1);
      b = ptr[comm].arg2 =
	real_object(ptr[comm].arg2);
      break;
    case 'A':
      a = ptr[comm].arg1 =
	real_room(ptr[comm].arg1);
      break;
    }
    if (a < 0 || b < 0) {
      if (!mini_mud)
	fprintf(stderr, "Invalid vnum in reset cmds, %s, cmd %d .. command disabled.\n", errorbuf, comm + 1);
      ptr[comm].command = '*';
    }
  }
}



void	renum_zone_table(void)
{
  int	zone;

  for (zone = 0; zone <= top_of_zone_table; zone++) {
    sprintf(buf, "zone #%d", zone_table[zone].number);
    check_reset_cmds(zone_table[zone].cmd, buf);
  }
}


/* A generic load reset cmds function.  To be used by load_zones and
 * load_quests.  -Petrus
 */
void  load_reset_cmds(FILE *fl, struct reset_com **ptr)
{
  int	cmd_no = 0, expand, tmp;
  char buf[81];

  for (expand = 1; ; ) {
    if (expand) {
      if (!cmd_no)
	CREATE((*ptr), struct reset_com, 1);
      else if (!((*ptr) =
		 (struct reset_com *) realloc((*ptr),
					      (cmd_no + 1) * sizeof(struct reset_com)))) {
	perror("Couldn't realloc.  (load_reset_cmds, db.c)");
	exit(0);
      }
    }
    expand = 1;

    fscanf(fl, " ");		/* skip blanks */
    fscanf(fl, "%c",
	   &((*ptr)[cmd_no].command));

    if ((*ptr)[cmd_no].command == 'S')
      break;

    if ((*ptr)[cmd_no].command == '*') {
      expand = 0;
      fgets(buf, 80, fl);	/* skip command */
      continue;
    }

    fscanf(fl, " %d %d %d",
	   &tmp,
	   &(*ptr)[cmd_no].arg1,
	   &(*ptr)[cmd_no].arg2);

    (*ptr)[cmd_no].if_flag = tmp;

    if ((*ptr)[cmd_no].command == 'M' ||
	(*ptr)[cmd_no].command == 'O' ||
	(*ptr)[cmd_no].command == 'E' ||
	(*ptr)[cmd_no].command == 'P' ||
	(*ptr)[cmd_no].command == 'L' ||
	(*ptr)[cmd_no].command == 'A' ||
	(*ptr)[cmd_no].command == 'D')
      fscanf(fl, " %d", &(*ptr)[cmd_no].arg3);

    fgets(buf, 80, fl);		/* read comment */

    cmd_no++;
  }
}


/* load the zone table and command tables */
void	load_zones(FILE *fl)
{
  static int	zon = 0;
  int tmp;
  char	*check;

  for (; ; ) {
    fscanf(fl, " #%d\n", &tmp);
    sprintf(buf2, "beginning of zone #%d", tmp);
    check = fread_string(fl, buf2);

    if (*check == '$')
      break;			/* end of file */

    zone_table[zon].number = tmp;
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

    zone_table[zon].lowest = 999999; /* -Petrus */
    zone_table[zon].highest = 0;

    /* read the command table */
    load_reset_cmds(fl, &(zone_table[zon].cmd));

    zon++;
  }
  top_of_zone_table = zon - 1;
  free(check);
}



/*************************************************************************
 *  procedures for resetting, both play-time and boot-time	 	 *
 *********************************************************************** */

/* create a new mobile from a prototype */
struct char_data *read_mobile(int nr, int type)
{
  int	i;
  struct char_data *mob;

  if (type == VIRTUAL) {
    if ((i = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
      return(0);
    }
  } else
    i = nr;

  CREATE(mob, struct char_data, 1);

  *mob = mob_proto[i];

  if (!mob->points.max_hit) {
    mob->points.max_hit = dice(mob->points.hit, mob->points.mana) +
      mob->points.move;
  } else
    mob->points.max_hit = number(mob->points.hit, mob->points.mana);

  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;


  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon  = time(0);
  mob->player.time.last_logon = time(0);

  /* insert in list.  New: change -Petrus */
  insert_to_char_list(mob);

  mob_index[i].number++;

  return mob;
}


/* set_character_abilities:
 * Given a pointer to a char, it'll set that char's abils depending on it's
 * level
 */
void set_character_abilities(struct char_data *ch)
{
  int level = GET_LEVEL(ch);

  ch->abilities.str   = 9 + level/10;
  ch->abilities.intel = 9 + level/12;
  ch->abilities.wis   = 9 + level/12;
  ch->abilities.dex   = 9 + level/8;
  ch->abilities.con   = 9 + level/8;
  ch->abilities.cha   = 9 + level/16;
}


/* General mob race resistance table -Petrus
 * And general mob skill table
 */
struct resistance *npc_resistances[MOB_RACES];
extern struct resistance npc_resistance_table[];
char   npc_skills[MOB_CLASSES][MOB_SKILLS];
extern struct mskill npc_skill_table[];


/* character_with_EEMMS:
 * Extra Easy Mob Making System: Given a pointer to a char with level set,
 * this function will set all other necessary data, all calculated by level
 * Note: SEX needs to be set too
 */
void character_with_EEMMS(struct char_data *ch)
{
  int level = GET_LEVEL(ch);
  int i;

  ch->points.hitroll = level/1.9;
  ch->points.armor   = 100 - 10*level/1.8;

  ch->points.max_mana = 100 + level * 10;
  ch->points.max_move = 80 + level * 15;


/* Helm - A-type mob hit points formula */
  ch->points.max_hit = 0;
/*  ch->points.hit = 5;      /* 5d(level)+.003*level^3-.25*level^2+20*level*/
/*  ch->points.mana = level;
  ch->points.move = .003*level*level*level - .25*level*level + 20*level;
*/

/* OLD EEMMS CODE : 1dLVL + LVL*15+25 */
  ch->points.hit = 1;
  ch->points.mana = level;
  ch->points.move = level*15 + 25;



  CREATE(ch->mob_specials.attacks, struct attack_type, 2);
  ch->mob_specials.attacks[1].type = 0;

  switch(ch->specials2.race) {
    case MOB_CATBEAST:
    case MOB_HOUND:
    case MOB_BIRD:
    case MOB_MOUNT_FLY:
    case MOB_FLYBEAST:
    case MOB_MOUNT:
      ch->mob_specials.attacks[0].type = TYPE_BITE;
      break;
    case MOB_BEARBEAST:
      ch->mob_specials.attacks[0].type = TYPE_CLAW;
      break;
    case MOB_GIANT:
      ch->mob_specials.attacks[0].type = TYPE_CRUSH;
      break;
    case MOB_DRAGON:
      ch->mob_specials.attacks[0].type = TYPE_WHIP;
      break;
    case MOB_INSECT:
      ch->mob_specials.attacks[0].type = TYPE_STING;
      break;
    default:
      ch->mob_specials.attacks[0].type = TYPE_HIT;
      break;
  }

  ch->mob_specials.attacks[0].damtype = 500;
  ch->mob_specials.attacks[0].percent_of_use = 100;
  ch->mob_specials.attacks[0].damodice = 1;
  ch->mob_specials.attacks[0].damsizedice = (level>12?level/2:6);
  ch->mob_specials.attacks[0].damadd = level/2+1;

  ch->points.damroll = level/2;   /* CHANGE HERE SOON */

  MAX(20 - level, 2);
  ch->mobskills =
    npc_skills[ch->player.class];

  ch->mob_specials.resists = npc_resistances[ch->specials2.race];

  GET_GOLD(ch) =  level*level;
  GET_EXP(ch)  =  exp_needed(ch)/10;

  ch->specials.position    = 8;
  ch->mob_specials.default_pos = 8;

  ch->player.weight = 200;
  ch->player.height = 198;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = -1;

  GET_COND(ch, DRUNK) = 0;  /* Mobs can get drunk -P */

  for (i = 0; i < 5; i++)
    ch->specials2.resistances[i] =
      MAX(20 - level, 2);
}



void	load_mobiles(FILE *mob_f)
{
  static int	i = 0;
  int	nr, j;
  long	tmp, tmp2, tmp3, tmp4, tmp5, tmp6;
  char	chk[256], *tmpptr;
  char	letter;

  /* resetting general mob resistances */
  j = 0;
  nr = 0;
  while (npc_resistance_table[j].type != -1) {
    if (npc_resistance_table[j].type) {
      npc_resistances[nr] = &npc_resistance_table[j];
      while (npc_resistance_table[j].type != 0)
	       j++;
    } else
      npc_resistances[nr] = 0;
    nr++;
    j++;
  }
  if (nr != MOB_RACES) {
    perror("Screwup while resetting mob resists - db.c");
    exit(0);
  }  /* end */

  /* resetting general mob skills */
  for (j = 0; j < MOB_CLASSES; j++)
    for (nr = 0; nr < MOB_SKILLS; nr++)
      npc_skills[j][nr] = 0;

  j = 0;
  nr = 0;
  while (npc_skill_table[j].type != -1) {
    while (npc_skill_table[j].type >= SKILL_START &&
	   npc_skill_table[j].type < SKILL_START + 99)
      {
	npc_skills[nr][npc_skill_table[j].type - SKILL_START] =
	  npc_skill_table[j].percentage;
	   j++;
      }
    nr++;
    j++;
  }
  if (nr != MOB_CLASSES) {
    perror("Screwup while resetting mob skills - db.c");
    exit(0);
  }  /* end */

  letter = fread_letter(mob_f);
  sprintf(buf2, "mobfile start");

  for (; ; ) {
    if (letter == '#') {
      fscanf(mob_f, "%d\n", &nr);
      if (nr >= 99999)
	break;

      mob_index[i].virtual = nr;
      mob_index[i].number  = 0;
      mob_index[i].func    = 0;

      clear_char(mob_proto + i);

      sprintf(buf2, "mob vnum %d", nr);

      /***** String data *** */
      mob_proto[i].player.name = fread_share_string(mob_f, buf2, i, MOB_NAME);
      tmpptr = mob_proto[i].player.short_descr = fread_share_string(mob_f, buf2, i, MOB_SHORT_DESC);
      if (tmpptr && *tmpptr)
	if (!str_cmp(fname(tmpptr), "a") ||
	    !str_cmp(fname(tmpptr), "an") ||
	    !str_cmp(fname(tmpptr), "the"))
	  *tmpptr = tolower(*tmpptr);
      mob_proto[i].player.long_descr = fread_share_string(mob_f, buf2, i, MOB_LONG_DESC);
      mob_proto[i].player.description = fread_share_string(mob_f, buf2, i, MOB_DESC);
	 mob_proto[i].player.title = NULL;;
      mob_proto[i].player.plan = NULL;

      /* *** Numeric data *** */
      mob_proto[i].specials2.race  =  fread_number(mob_f, buf2);
      mob_proto[i].player.class    =  fread_number(mob_f, buf2);

      MOB_FLAGS(mob_proto + i)     =  fread_number(mob_f, buf2);
      SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);

      QUEST_NUM((mob_proto + i)) = 0;

      mob_proto[i].specials.affected_by = fread_number(mob_f, buf2);
      mob_proto[i].specials.affected_by2 = 0;
      GET_ALIGNMENT(mob_proto + i)      = fread_number(mob_f, buf2);

      fscanf(mob_f, " %c \n", &letter);

      if (letter == 'S') {
	/* The new easy monsters */
	fscanf(mob_f, " %ld", &tmp);
	GET_LEVEL(mob_proto + i) = tmp;

	set_character_abilities(mob_proto + i);

	mob_proto[i].points.hitroll = 20 - fread_number(mob_f, buf2);
	mob_proto[i].points.armor   = 10 * fread_number(mob_f, buf2);

	mob_proto[i].points.max_mana = 100 + tmp * 10;
	mob_proto[i].points.max_move = 80 + tmp * 15;

	fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
	mob_proto[i].points.max_hit = 0;
	mob_proto[i].points.hit = tmp;
	mob_proto[i].points.mana = tmp2;
	mob_proto[i].points.move = tmp3;

	fscanf(mob_f, " %ld", &tmp);
	if (tmp != -1) {
	  CREATE(mob_proto[i].mob_specials.attacks, struct attack_type, MAX_MOB_ATTACKS + 1);
	  for (j = 0; j < MAX_MOB_ATTACKS; j++)
	    mob_proto[i].mob_specials.attacks[j].type = 0;
	}
	j = 0;
	while (tmp != -1 && j < MAX_MOB_ATTACKS) {
	  fscanf(mob_f, " %ld %ld %ldd%ld+%ld \n", &tmp2, &tmp3, &tmp4, &tmp5, &tmp6);

	  mob_proto[i].mob_specials.attacks[j].type = tmp;
	  mob_proto[i].mob_specials.attacks[j].damtype = tmp2;
	  mob_proto[i].mob_specials.attacks[j].percent_of_use = tmp3;
	  mob_proto[i].mob_specials.attacks[j].damodice = tmp4;
	  mob_proto[i].mob_specials.attacks[j].damsizedice = tmp5;
	  mob_proto[i].mob_specials.attacks[j].damadd = tmp6;
	  if (j == 0)
	    mob_proto[i].points.damroll = tmp6;/* CHANGE HERE SOON */

	  j++;
	  fscanf(mob_f, " %ld", &tmp);
	}

	     fscanf(mob_f, " %ld", &tmp);
	if (tmp != -1) {
	  CREATE(mob_proto[i].mobskills, byte, MOB_SKILLS);
	  for (j = 0; j < MOB_SKILLS; j++)
	    mob_proto[i].mobskills[j] =
	      npc_skills[mob_proto[i].player.class][j];
	} else {
	  mob_proto[i].mobskills =
	    npc_skills[mob_proto[i].player.class];
	}
	while (tmp != -1) {
	  fscanf(mob_f, " %ld", &tmp2);
	  assert(tmp >= SKILL_START && tmp < SKILL_START + 99);
	  mob_proto[i].mobskills[tmp - SKILL_START]  = tmp2;

		 fscanf(mob_f, " %ld", &tmp);
	}
	fscanf(mob_f, " \n");

	fscanf(mob_f, " %ld", &tmp);
	if (tmp != -1) {
	  CREATE(mob_proto[i].mob_specials.resists, struct resistance, MAX_MOB_RESISTANCES + 1);
	  for (j = 0; j < MAX_MOB_RESISTANCES; j++)
	    mob_proto[i].mob_specials.resists[j].type = 0;
	} else {
	  mob_proto[i].mob_specials.resists = npc_resistances[mob_proto[i].specials2.race];
	}
	j = 0;
	while (tmp != -1 && j < MAX_MOB_RESISTANCES) {
	  mob_proto[i].mob_specials.resists[j].type = tmp;
	  mob_proto[i].mob_specials.resists[j].percentage = fread_number(mob_f, buf2);
	  j++;
	  fscanf(mob_f, " %ld", &tmp);
	}
	fscanf(mob_f, " \n");

	GET_GOLD(mob_proto + i) =  fread_number(mob_f, buf2);
	GET_EXP(mob_proto + i)  =  fread_number(mob_f, buf2);

	mob_proto[i].specials.position    = fread_number(mob_f, buf2);
	mob_proto[i].mob_specials.default_pos = fread_number(mob_f, buf2);
	mob_proto[i].player.sex           = fread_number(mob_f, buf2);

	mob_proto[i].player.weight = 200;
	mob_proto[i].player.height = 198;

	for (j = 0; j < 3; j++)
	  GET_COND(mob_proto + i, j) = -1;

	GET_COND(mob_proto + i, DRUNK) = 0;  /* Mobs can get drunk -P */

      } else if (letter == 'A') {
	/* The new Automobs - even more easy monsters - Petrus*/
	fscanf(mob_f, " %ld", &tmp);
	GET_LEVEL(mob_proto + i) = tmp;

	set_character_abilities(mob_proto + i);
	mob_proto[i].player.sex           = fread_number(mob_f, buf2);
	character_with_EEMMS(mob_proto + i);

      } else {  /* The old monsters are down below here */
	mob_proto[i].abilities.str   = fread_number(mob_f, buf2);
	mob_proto[i].abilities.intel = fread_number(mob_f, buf2);
	mob_proto[i].abilities.wis   = fread_number(mob_f, buf2);
	mob_proto[i].abilities.dex   = fread_number(mob_f, buf2);
	mob_proto[i].abilities.con   = fread_number(mob_f, buf2);
	mob_proto[i].abilities.cha   = fread_number(mob_f, buf2);

	mob_proto[i].points.max_hit = 1;
	mob_proto[i].points.hit  = fread_number(mob_f, buf2);
	mob_proto[i].points.mana = fread_number(mob_f, buf2);

	mob_proto[i].points.armor = 10 * fread_number(mob_f, buf2);

	mob_proto[i].points.max_mana = fread_number(mob_f, buf2);
	mob_proto[i].points.max_move = fread_number(mob_f, buf2);
	mob_proto[i].points.gold     = fread_number(mob_f, buf2);
	GET_EXP(mob_proto + i)       = fread_number(mob_f, buf2);

	mob_proto[i].specials.position    = fread_number(mob_f, buf2);
	mob_proto[i].mob_specials.default_pos = fread_number(mob_f, buf2);
	mob_proto[i].player.sex           = fread_number(mob_f, buf2);
	GET_LEVEL(mob_proto + i)          = fread_number(mob_f, buf2);
	fread_number(mob_f, buf2); /* = AGE */

	mob_proto[i].player.weight = fread_number(mob_f, buf2);
	mob_proto[i].player.height = fread_number(mob_f, buf2);

	for (j = 0; j < 3; j++) {
	  GET_COND(mob_proto + i, j) = fread_number(mob_f, buf2);
	}

	fscanf(mob_f, " \n ");

	for (j = 0; j < 5; j++) {
	  mob_proto[i].specials2.resistances[j] = fread_number(mob_f, buf2);
	}

	fscanf(mob_f, " \n ");

	fscanf(mob_f, " %ld", &tmp);
	if (tmp != -1) {
	  CREATE(mob_proto[i].mob_specials.attacks, struct attack_type, MAX_MOB_ATTACKS + 1);
	  for (j = 0; j < MAX_MOB_ATTACKS; j++)
	    mob_proto[i].mob_specials.attacks[j].type = 0;
	}

	j = 0;
	while (tmp != -1 && j < MAX_MOB_ATTACKS) {
	  fscanf(mob_f, " %ld %ld %ldd%ld+%ld \n", &tmp2, &tmp3, &tmp4, &tmp5, &tmp6);

	  mob_proto[i].mob_specials.attacks[j].type = tmp;
	  mob_proto[i].mob_specials.attacks[j].damtype = tmp2;
	  mob_proto[i].mob_specials.attacks[j].percent_of_use = tmp3;
	  mob_proto[i].mob_specials.attacks[j].damodice = tmp4;
	  mob_proto[i].mob_specials.attacks[j].damsizedice = tmp5;
	  mob_proto[i].mob_specials.attacks[j].damadd = tmp6;
	  mob_proto[i].points.damroll = tmp6;   /* CHANGE HERE SOON */

	  j++;
	  fscanf(mob_f, " %ld", &tmp);
	}

	fscanf(mob_f, " %ld", &tmp);
	if (tmp != -1) {
	  CREATE(mob_proto[i].mobskills, byte, MOB_SKILLS);
	  for (j = 0; j < MOB_SKILLS; j++)
	    mob_proto[i].mobskills[j] =
	      npc_skills[mob_proto[i].player.class][j];
	} else {
	  mob_proto[i].mobskills =
	    npc_skills[mob_proto[i].player.class];
	}
	while (tmp != -1) {
	  fscanf(mob_f, " %ld", &tmp2);
	  assert(tmp >= SKILL_START && tmp < SKILL_START + 99);
	  mob_proto[i].mobskills[tmp - SKILL_START]  = tmp2;

	  fscanf(mob_f, " %ld", &tmp);
	}
	fscanf(mob_f, " \n");

	fscanf(mob_f, " %ld", &tmp);
	if (tmp != -1) {
	  CREATE(mob_proto[i].mob_specials.resists, struct resistance, MAX_MOB_RESISTANCES + 1);
	  for (j = 0; j < MAX_MOB_RESISTANCES; j++)
	    mob_proto[i].mob_specials.resists[j].type = 0;
	}
	j = 0;
	while (tmp != -1 && j < MAX_MOB_RESISTANCES) {
	  mob_proto[i].mob_specials.resists[j].type = tmp;
	  mob_proto[i].mob_specials.resists[j].percentage = fread_number(mob_f, buf2);
	  j++;
	  fscanf(mob_f, " %ld", &tmp);
	}
	fscanf(mob_f, " \n");

	/* Calculate THAC0 as a formular of Level */
	mob_proto[i].points.hitroll = MAX(1, GET_LEVEL(mob_proto + i) - 3);
      }

      mob_proto[i].tmpabilities = mob_proto[i].abilities;

      for (j = 0; j < MAX_WEAR; j++) /* Initialisering Ok */
	mob_proto[i].equipment[j] = 0;

      for (j = 0; j < 4; j++)
	mob_proto[i].specials2.resistances[j] = GET_LEVEL((mob_proto + i))/2;
      mob_proto[i].specials2.resistances[4] = 0; /* magic resistance */

      mob_proto[i].nr = i;
      mob_proto[i].desc = 0;

      letter = fread_letter(mob_f);

      if (letter == 'A') { /* New action string -Petrus */
	fread_letter(mob_f);
	mob_proto[i].mobaction = fread_string(mob_f, buf2);
	mob_proto[i].nextact = mob_proto[i].mobaction;
	letter = fread_letter(mob_f);
      }
      if (letter == 'P') { /* New special assign -Petrus */
	fscanf(mob_f, "%s\n", chk);
	special_assign(chk, SPEC_MOB, &mob_index[i].func, buf2);
	letter = fread_letter(mob_f);
      }
      if (letter == 'R') { /* New Mob Resistances -Helm */
        fscanf(mob_f, "%ld %ld %ld %ld %ld\n", &tmp, &tmp2, &tmp3, &tmp4, &tmp5);
	if (tmp >= 0) mob_proto[i].specials2.resistances[0] = MIN(tmp, 100);
	if (tmp2 >= 0) mob_proto[i].specials2.resistances[1] = MIN(tmp2, 100);
	if (tmp3 >= 0) mob_proto[i].specials2.resistances[2] = MIN(tmp3, 100);
	if (tmp4 >= 0) mob_proto[i].specials2.resistances[3] = MIN(tmp4, 100);
	if (tmp5 >= 0) mob_proto[i].specials2.resistances[4] = MIN(tmp5, 100);
      letter = fread_letter(mob_f);
      }
      if (letter == '>') { /* New Mob Prog -Petrus */
	ungetc(letter, mob_f);
	(void) mprog_read_programs(mob_f, &mob_index[i]);
	letter = fread_letter(mob_f);
      }

      i++;
    } else if (letter == '$') /* EOF */
      break;
    else {
      sprintf(buf2, "SYSERR: Format error in mob file near mob #%d", nr);
      log(buf2);
      exit(1);
    }
  }
  top_of_mobt = i - 1;
}


/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type)
{
   struct obj_data *obj;
   int	i;

   if (nr < 0) {
      log("SYSERR: trying to create obj with negative num!");
      return 0;
   }

   if (type == VIRTUAL) {
      if ((i = real_object(nr)) < 0) {
	 sprintf(buf, "Object (V) %d does not exist in database.", nr);
	 return(0);
      }
   } else
      i = nr;

   CREATE(obj, struct obj_data, 1);
   *obj = obj_proto[i];

   obj->next = object_list;
   object_list = obj;

   obj_index[i].number++;

   return (obj);
}



/* read all objects from obj file; generate index and prototypes */
void	load_objects(FILE *obj_f)
{
    static int	i = 0;
    long	tmp, tmp2, tmp3, j, nr;
    /* long tmp4, tmp5, tmp6; */
    char	chk[50], *tmpptr;
    struct extra_descr_data *new_descr;

    if (!fscanf(obj_f, "%s\n", chk)) {
	perror("load_objects");
	exit(1);
    }

    for (; ; ) {
	if (*chk == '#') {
	    sscanf(chk, "#%ld\n", &nr);
	    if (nr >= 99999)
		break;

	    obj_index[i].virtual = nr;
	    obj_index[i].number  = 0;
	    obj_index[i].func    = 0;

	    clear_object(obj_proto + i);

	    sprintf(buf2, "object #%ld", nr);

	    /* *** string data *** */

	    tmpptr = obj_proto[i].name = fread_share_string(obj_f, buf2, i, OBJ_NAME);
	    if (!tmpptr) {
		fprintf(stderr, "format error at or near %s\n", buf2);
		exit(1);
	    }

	    tmpptr = obj_proto[i].short_description = fread_share_string(obj_f, buf2, i, OBJ_SHORT_DESC);
	    if (*tmpptr)
		if (!str_cmp(fname(tmpptr), "a") ||
		    !str_cmp(fname(tmpptr), "an") ||
		    !str_cmp(fname(tmpptr), "the"))
		    *tmpptr = tolower(*tmpptr);
	    tmpptr = obj_proto[i].description = fread_share_string(obj_f, buf2, i, OBJ_DESC);
	    if (tmpptr && *tmpptr)
		*tmpptr = toupper(*tmpptr);
	    obj_proto[i].action_description = fread_string(obj_f, buf2);

	    /* *** numeric data *** */

	    obj_proto[i].obj_flags.type_flag   = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.level       = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.anticlass   = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.extra_flags = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.wear_flags  = fread_number(obj_f, buf2);

	    /*
	    fscanf(obj_f, " %d %d %d %d %d %d", &tmp, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6); */
	    obj_proto[i].obj_flags.value[0] = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.value[1] = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.value[2] = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.value[3] = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.value[4] = fread_number(obj_f, buf2);
	    obj_proto[i].obj_flags.value[5] = fread_number(obj_f, buf2);

	    fscanf(obj_f, " %ld %ld %ld", &tmp, &tmp2, &tmp3);
	    obj_proto[i].obj_flags.weight = tmp;
	    obj_proto[i].obj_flags.cost = tmp2;
	    obj_proto[i].obj_flags.cost_per_day = tmp3;

	    /* Check added for drink container weight */
	    if ((obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON) ||
		(obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN))
	      obj_proto[i].obj_flags.weight += obj_proto[i].obj_flags.value[1] / 10; /* add the weight of the drinks */

	    /* *** extra descriptions *** */

	    obj_proto[i].ex_description = 0;

	    sprintf(buf2, "%s - extra desc. section", buf2);

	    while (fscanf(obj_f, " %s \n", chk), *chk == 'E') {
		CREATE(new_descr, struct extra_descr_data, 1);
		new_descr->keyword = fread_share_string(obj_f, buf2, i, OBJ_EXTRA_KEYWORDS);
		new_descr->description = fread_share_string(obj_f, buf2, i, OBJ_EXTRA_DESC);
		new_descr->next = obj_proto[i].ex_description;
		obj_proto[i].ex_description = new_descr;
	    }

	    for (j = 0 ; (j < MAX_OBJ_AFFECT) && (*chk == 'A') ; j++) {
		fscanf(obj_f, " %ld %ld ", &tmp, &tmp2);
		obj_proto[i].affected[j].location = tmp;
		obj_proto[i].affected[j].modifier = tmp2;
		fscanf(obj_f, " %s \n", chk);
	    }

	    for (; (j < MAX_OBJ_AFFECT); j++) {
	      obj_proto[i].affected[j].location = APPLY_NONE;
	      obj_proto[i].affected[j].modifier = 0;
	    }

	    for (obj_proto[i].obj_flags.bitvector = 0; (*chk == 'B');) {
	      obj_proto[i].obj_flags.bitvector += fread_number(obj_f, buf2);
	      fscanf(obj_f, " %s \n", chk);
	    }

	    for (;(*chk == 'P');) {
	      special_assign(chk, SPEC_OBJ, &obj_index[i].func, buf2);
	      fscanf(obj_f, " %s \n", chk);
	    }

	    obj_proto[i].in_room = NOWHERE;
	    obj_proto[i].next_content = 0;
	    obj_proto[i].carried_by = 0;
            obj_proto[i].used_by = 0;
            obj_proto[i].worn_on = -1;
	    obj_proto[i].in_obj = 0;
	    obj_proto[i].contains = 0;
	    obj_proto[i].item_number = i;
	    obj_proto[i].clan_eq = NULL;

	    i++;
	} else if (*chk == '$') /* EOF */
	    break;
	else {
	    sprintf(buf2, "Format error in obj file at or near obj #%ld", nr);
	    log(buf2);
	    exit(1);
	}
    }
    top_of_objt = i - 1;
}


void set_key_timer(struct obj_data *obj)
{
   if (obj) {
      if (GET_ITEM_TYPE(obj) == ITEM_KEY)
	   GET_ITEM_VALUE(obj, 4) += 1;
      /* Set the Timer */

      set_key_timer(obj->contains);
      set_key_timer(obj->next_content);
   }
}


void key_update(void)
{
     struct obj_data *obj, *next_obj;

     for (obj = object_list; obj ; obj = next_obj) {
	  next_obj = obj->next; /* Go through each item */

	  if(GET_ITEM_TYPE(obj) == ITEM_KEY) {
	       if (GET_ITEM_VALUE(obj, 4) > 0)
		    ++(GET_ITEM_VALUE(obj, 4)); /* Timer count UP! */

	       if (GET_ITEM_VALUE(obj, 4) >= 999) GET_ITEM_VALUE(obj, 4) = 999;
	            /* Just in case Timer goes too high */

	       if (!GET_ITEM_VALUE(obj, 5)) GET_ITEM_VALUE(obj, 5) = 2;
	            /* Default if Value[5] is not set */

	       if (GET_ITEM_VALUE(obj, 4) >= GET_ITEM_VALUE(obj, 5)) {
		    /* Key Disappears */

                    if (obj->used_by)
                         act("$p disappears mysteriously.", FALSE, obj->used_by, obj, 0, TO_CHAR|TO_SLEEP);
		    if (obj->carried_by || obj->in_obj)
			 act("$p disappears mysteriously.", FALSE, obj->carried_by, obj, 0, TO_CHAR|TO_SLEEP);
		    else if ((obj->in_room != NOWHERE) && (world[obj->in_room]->people)) {
			 act("$p disappears mysteriously.",
			     TRUE, world[obj->in_room]->people, obj, 0, TO_ROOM);
			 act("$p disappears mysteriously.",
			     TRUE, world[obj->in_room]->people, obj, 0, TO_CHAR);
		    }

		    if ((obj->used_by) || (obj->carried_by) || (obj->in_obj) || (obj->in_room != NOWHERE))
			 extract_obj(obj);
	       }
	  }

     }
}


#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void	zone_update(void)
{
   int	i;
   struct reset_q_element *update_u, *temp;
   static int timer = 0;
   char buf[128];

   /* jelson 10/22/92 */
   if (((++timer * PULSE_ZONE) / 4) >= 60) { /* 4 comes from 4 passes/sec */
      /* one minute has passed */
      /* NOT accurate unless PULSE_ZONE is a multiple of 4 or a factor of 60 */

      key_update();
      timer = 0;

      /* since one minute has passed, increment zone ages */
      for (i = 0; i <= top_of_zone_table; i++) {
         if (zone_table[i].age < zone_table[i].lifespan &&
	     zone_table[i].reset_mode)
	       (zone_table[i].age)++;

         if (zone_table[i].age >= zone_table[i].lifespan &&
	     zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
                /* enqueue zone */

	        CREATE(update_u, struct reset_q_element, 1);

		update_u->zone_to_reset = i;
		update_u->next = 0;

		if (!reset_q.head)
		   reset_q.head = reset_q.tail = update_u;
		else {
		   reset_q.tail->next = update_u;
		   reset_q.tail = update_u;
		}

		zone_table[i].age = ZO_DEAD;
	 }
      }
   }

   /* dequeue zones (if possible) and reset */

   for (update_u = reset_q.head; update_u; update_u = update_u->next)
      if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
          is_empty(update_u->zone_to_reset)) {
	 reset_zone(update_u->zone_to_reset, FALSE);
	 sprintf(buf, "Auto zone reset: %s",
		 zone_table[update_u->zone_to_reset].name);
	 mudlog(buf, CMP, LEVEL_ADMIN, FALSE);
	 /* dequeue */
	 if (update_u == reset_q.head)
	    reset_q.head = reset_q.head->next;
	 else {
	    for (temp = reset_q.head; temp->next != update_u;
		 temp = temp->next) ;

	    if (!update_u->next)
	       reset_q.tail = temp;

	    temp->next = update_u->next;
	 }

	 free(update_u);
	 break;
      }

   /* execute rprogs */
   for (i = 0; i <= top_of_world; i++)
     if (world[i]->rprogs && world[i]->people)
       update_rprog_check(world[i]);
}


void execute_reset_cmds(struct reset_com *ptr, char* errorbuf, bool complete)
{
    int	cmd_no, last_cmd = 1;
    char	buf[256];
    struct char_data *mob = NULL;
    struct obj_data *obj, *obj_to;
    int i, j;
    struct room_direction_data *exit;;

    for (cmd_no = 0; ; cmd_no++) {
	if (ptr[cmd_no].command == 'S')
	    break;

	if (last_cmd || !ptr[cmd_no].if_flag)
	    switch (ptr[cmd_no].command) {
	    case '*': /* ignore command */
		break;

	    case 'M': /* read a mobile */
		if (mob_index[ptr[cmd_no].arg1].number < ptr[cmd_no].arg2) {
		    mob = read_mobile(ptr[cmd_no].arg1, REAL);
		    char_to_room(mob, ptr[cmd_no].arg3);
		    if (IS_SET(mob->specials2.act, MOB_SENTINEL))
		      mob->player.hometown = IN_ROOM(mob);   /* FOR TRACK */
		    last_cmd = 1;
		} else
		    last_cmd = 0;
		break;

	    case 'O': /* read an object */
		if (ptr[cmd_no].arg3 >= 0) {
		     if (!get_obj_in_list_num(ptr[cmd_no].arg1, world[ptr[cmd_no].arg3]->contents)) {
			  obj = read_object(ptr[cmd_no].arg1, REAL);
			  if (complete ||
			     (obj->obj_flags.type_flag == ITEM_KEY) ||
			     (ptr[cmd_no].arg2 >= number(1, 100))) {
			       obj_to_room(obj, ptr[cmd_no].arg3);
			       last_cmd = 1;
			  } else  {
			       extract_obj(obj);
			       last_cmd = 0;
			  }
  		     }
		} else {
		     obj = read_object(ptr[cmd_no].arg1, REAL);
		     obj->in_room = NOWHERE;
		     last_cmd = 1;
		}
		break;

	    case 'P': /* object to object */
		    obj = read_object(ptr[cmd_no].arg1, REAL);
		    if (!(obj_to = get_obj_num(ptr[cmd_no].arg3))) {
			log("SYSERR: error in reset cmds: target obj not found.");
			sprintf(buf, "SYSERR:   Offending cmd: \"P %d %d %d\" in %s (cmd %d)",
				obj_index[ptr[cmd_no].arg1].virtual, ptr[cmd_no].arg2,
				obj_index[ptr[cmd_no].arg3].virtual,
				errorbuf, cmd_no + 1);
			log(buf);
			extract_obj(obj);
			last_cmd = 0;
			break;
		    }
		    if (complete ||
		       (obj->obj_flags.type_flag == ITEM_KEY) ||
		       (ptr[cmd_no].arg2 >= number(1, 100)))
			    obj_to_obj(obj, obj_to);
		    else {
			 last_cmd = 1;
			 extract_obj(obj);
		         break;
		    }
		    last_cmd = 1;
		    break;

	    case 'G': /* obj_to_char */
		if (!mob) {
		    log("SYSERR: error in reset cmds: attempt to give obj to non-existant mob.");
		    sprintf(buf, "SYSERR: Offending cmd: \"G %d %d\" in %s (cmd %d)",
			    mob_index[ptr[cmd_no].arg1].virtual, ptr[cmd_no].arg2,
			    errorbuf, cmd_no + 1);
		    log(buf);
		    last_cmd = 0;
		    break;
		}
		obj = read_object(ptr[cmd_no].arg1, REAL);
		if (complete ||
		    (obj->obj_flags.type_flag == ITEM_KEY) ||
		    (ptr[cmd_no].arg2 >= number(1, 100))) {
			    obj_to_char(obj, mob);
		    last_cmd = 1;
		} else {
		  last_cmd = 1;
		  if(obj->obj_flags.type_flag == ITEM_CONTAINER)
			last_cmd = 0;
		  extract_obj(obj);
	 	}
		break;

	    case 'E': /* object to equipment list */
		if (!mob) {
		    log("SYSERR: error in reset cmds: trying to equip non-existant mob");
		    sprintf(buf, "SYSERR: Offending cmd: \"E %d %d %d\" in %s (cmd %d)",
			    obj_index[ptr[cmd_no].arg1].virtual, ptr[cmd_no].arg2,
			    ptr[cmd_no].arg3, errorbuf, cmd_no + 1);
		    log(buf);
		    last_cmd = 0;
		    break;
		}
		    if (ptr[cmd_no].arg3 < 0 || ptr[cmd_no].arg3 >= MAX_WEAR) {
			log("SYSERR: error in reset cmds: invalid equipment pos number");
			sprintf(buf, "SYSERR: Offending cmd: \"E %d %d %d\" in %s (cmd %d)",
				obj_index[ptr[cmd_no].arg1].virtual, ptr[cmd_no].arg2, ptr[cmd_no].arg3, errorbuf, cmd_no + 1);
			log(buf);
			last_cmd = 0;
		    } else {
			obj = read_object(ptr[cmd_no].arg1, REAL);
	 	    if (complete ||
			(obj->obj_flags.type_flag == ITEM_KEY) ||
			(ptr[cmd_no].arg2 >= number(1, 100))) {
			equip_char(mob, obj, ptr[cmd_no].arg3);
			last_cmd = 1;
		    } else {
			last_cmd = 1;
			if(obj->obj_flags.type_flag == ITEM_CONTAINER)
			   last_cmd = 0;
			extract_obj(obj);
		    }
		}
		break;

	    case 'D': /* set state of door */
	      if (!world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]) {
		log("SYSERR: error in reset cmds: non-existant direction");
		sprintf(buf, "SYSERR: Offending cmd: \"D %d %d %d\" in %s (cmd %d)", ptr[cmd_no].arg1, ptr[cmd_no].arg2, ptr[cmd_no].arg3, errorbuf, cmd_no + 1);
		log(buf);
		last_cmd = 0;
	      } else
		switch (ptr[cmd_no].arg3) {
		case 0:
		    REMOVE_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
			       EX_LOCKED);
		    REMOVE_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
			       EX_CLOSED);
		    break;
		case 1:
		    SET_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
			    EX_CLOSED);
		    REMOVE_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
			       EX_LOCKED);
		    break;
		case 2:
		    SET_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
			    EX_LOCKED);
		    SET_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
			    EX_CLOSED);
		    break;
		}
		last_cmd = 1;
                if (world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]) 
                  REMOVE_BIT(world[ptr[cmd_no].arg1]->dir_option[ptr[cmd_no].arg2]->exit_info,
                             EX_BROKEN); // fix door - Charlene
		break;

	    case 'R':
		for (i = 0; i < ptr[cmd_no].arg2 - 1; i++) {
		    j = number(i, ptr[cmd_no].arg2 - 1);
		    exit = world[ptr[cmd_no].arg1]->dir_option[i];
		    world[ptr[cmd_no].arg1]->dir_option[i] = world[ptr[cmd_no].arg1]->dir_option[j];
		    world[ptr[cmd_no].arg1]->dir_option[j] = exit;
		}
		break;

	    case 'L': /* Load/Reset a Portal */
	      /* arg1 - Room ; arg2 - Object vnum ; arg3 - Destination */
	      if ((ptr[cmd_no].arg1 >= 0) &&
		  (ptr[cmd_no].arg2 > 0) &&
		  (ptr[cmd_no].arg3 >= 0)) {
		i = 0;
	       for (obj = world[ptr[cmd_no].arg1]->contents; obj; obj = obj->next_content)
		 if ((GET_ITEM_TYPE(obj) == ITEM_PORTAL) &&
		     (obj->item_number == ptr[cmd_no].arg2)) {
		   GET_ITEM_VALUE(obj, 0) = ptr[cmd_no].arg3;
		   i = 1;
		   break;
		 }
	       if (i < 1) {
		 /* No portal found so we make one */
		 obj = read_object(ptr[cmd_no].arg2, REAL);
		 obj_to_room(obj, ptr[cmd_no].arg1);
		 GET_ITEM_VALUE(obj, 0) = ptr[cmd_no].arg3;
	       }
	       last_cmd = 1;
	      } else
		last_cmd = 0;
	      break;

	    case 'A': /* Activate a Portal */
	      /* arg1 - Room ; arg2 - Portal info ; arg3 - Lock Object */
	      for (obj = world[ptr[cmd_no].arg1]->contents; obj; obj = obj->next_content)
		if (GET_ITEM_TYPE(obj) == ITEM_PORTAL) {
		  if (ptr[cmd_no].arg2 > 0)
		    GET_ITEM_VALUE(obj, 1) = ptr[cmd_no].arg2;
		  else
		    GET_ITEM_VALUE(obj, 1) = 0;

		  if (ptr[cmd_no].arg3 > 0)
		    GET_ITEM_VALUE(obj, 2) = ptr[cmd_no].arg3;
		  else
		    GET_ITEM_VALUE(obj, 2) = -1;

		  last_cmd = 1;
		  break;
		} else
		  last_cmd = 0;
	      break;

	    default:
		sprintf(buf, "SYSERR:  Unknown cmd in reset table; %s cmd %d.",
			errorbuf, cmd_no + 1);
		mudlog(buf, NRM, LEVEL_ADMIN, TRUE);
		last_cmd = 0;
		break;
	    }
	else
	    last_cmd = 0;

    }
}

int check_pc(int theroom)
{
  int location, has_pc = 0;
  struct char_data *ch;

  location = theroom;
  for(ch = world[location]->people;ch;ch = ch->next_in_room) {
	if(!IS_NPC(ch))
		has_pc = 1;
  }
  return has_pc;
}


void clear_eq_zone(int zone)
{
     struct char_data *ch;
     struct obj_data *obj, *next_o;
     int location, i;
     int pc_inroom = 0;


     for (i = world[zone_table[zone].lowest]->number;
	  i <= world[zone_table[zone].highest]->number;
	  i++) {

	  if ((location = real_room(i)) < 0)
	       continue;

	  if (ROOM_FLAGGED(location, NO_SWEEP))
	       continue;

	  if(!(i == donation_room_1) && world[location]->contents) {
	       for(ch = world[location]->people; ch && !(pc_inroom); ch = ch->next_in_room) {
		    if(!IS_NPC(ch))
			 pc_inroom = 1;
	       }

	       if (!pc_inroom)
		    for(obj = world[location]->contents; obj; obj = next_o) {
			 next_o = obj->next_content;
			 if(!IS_OBJ_STAT(obj, ITEM_NOSWEEP))
			      extract_obj(obj);
		    }
	  }
     }
}



/* execute the reset command table of a given zone */
void	reset_zone(int zone, bool complete)
{
    sprintf(buf, "zone #%d", zone_table[zone].number);
    clear_eq_zone(zone);
    execute_reset_cmds(zone_table[zone].cmd, buf, complete);
    zone_table[zone].age = 0;
}




/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int	is_empty(int zone_nr)
{
   struct descriptor_data *i;

   for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	 if (world[i->character->in_room]->zone == zone_nr)
	    return(0);

   return(1);
}


void	load_clans(FILE *clan_f)
{
  static int	i = 0;
  int nr, j, k;
  char	chk[50], *tmpptr;
  struct room_list *room_ls;
  struct clan_obj_data *clan_eq;
  struct obj_data *obj;

  if (!fscanf(clan_f, "%s\n", chk)) {
    perror("load_clans");
    exit(1);
  }

  for (; ; ) {
    if (*chk == '#') {
      sscanf(chk, "#%d\n", &nr);
      if (nr >= 99999)
	break;

      clan_list[i].vnum = nr;

      sprintf(buf2, "clan #%d", nr);

      /* *** string data *** */

      tmpptr = clan_list[i].name = fread_string(clan_f, buf2);
      if (!tmpptr) {
	fprintf(stderr, "format error at or near %s\n", buf2);
	exit(1);
      }

      clan_list[i].symbol = fread_string(clan_f, buf2);
      clan_list[i].info   = fread_string(clan_f, buf2);

      for (j = 0; j <= 10; j++)
	for (k = 0; k <= 3; k++)
	  clan_list[i].ranknames[j][k] = clan_ranks[j][k];

      fscanf( clan_f, " %d\n", &j);
      while (j  >= 0) {
	if (j > 10) {
	  sprintf(buf2, "Format error in clan file at or near clan #%d - In rank names", nr);
	  log(buf2);
	  exit(1);
	}
	clan_list[i].ranknames[j][0] = fread_string(clan_f, buf2);
	clan_list[i].ranknames[j][1] = fread_string(clan_f, buf2);
	clan_list[i].ranknames[j][2] = fread_string(clan_f, buf2);

	fscanf( clan_f, " %d\n", &j);
      }

      /* *** numeric data *** */
      clan_list[i].flags  = fread_number(clan_f, buf2);

      if ((clan_list[i].donation =
	   real_room(fread_number(clan_f, buf2))) > 0) {
	CREATE(room_ls, struct room_list, 1);
	room_ls->number = clan_list[i].donation;
	room_ls->next = room_crash_list;
	room_crash_list = room_ls;
      }

      fscanf(clan_f, " %hd\n", &clan_list[i].recall);

/*  Can't use fread_number due to problems with -1 (NOWHERE) */

      clan_list[i].pwr_demote = fread_number(clan_f, buf2);
      clan_list[i].pwr_enlist = fread_number(clan_f, buf2);
      clan_list[i].pwr_expel  = fread_number(clan_f, buf2);
      clan_list[i].pwr_raise  = fread_number(clan_f, buf2);

      /* Clan EQ list */

      fscanf(clan_f, " %s \n", chk);
      for (;(*chk == 'E');) {
           fscanf(clan_f, " %d %d \n", &j, &k);
	   if ((j = real_object(j)) > -1) {
		/* flag the obj clan */
	     CREATE(clan_eq, struct clan_obj_data, 1);
	     clan_eq->clan = i;
	     clan_eq->exchange = k;
	     obj_proto[j].clan_eq = clan_eq;

	     /* create claneq list */
	     CREATE(obj, struct obj_data, 1);
	     *obj = obj_proto[j];
	     obj->next = claneq_list;
	     claneq_list = obj;
	}
	   fscanf(clan_f, " %s \n", chk);
      }

      /* Initialize online clan Information */
      clan_list[i].members = 0;
      clan_list[i].gods    = 0;
      clan_list[i].remorts = 0;
      clan_list[i].mortals = 0;
      clan_list[i].power   = 0;
      clan_list[i].wealth  = 0;
      clan_list[i].on_power= 0;

      for (j = 0; j < 11; j++)
	clan_list[i].roster[j]  = NULL;

      i++;

    } else if (*chk == '$')	/* EOF */
      break;
    else {
      sprintf(buf2, "Format error in clan file at or near clan #%d", nr);
      log(buf2);
      exit(1);
    }
  }
  top_of_clan = i - 1;
}



/*************************************************************************
*  Stuff related to the save/load player system				 *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int	load_char(char *name, struct char_file_u *char_element)
{
   int	player_i;

   int	find_name(char *name);

   if ((player_i = find_name(name)) >= 0) {
      fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
      fread(char_element, sizeof(struct char_file_u), 1, player_fl);
      return(player_i);
   } else
      return(-1);
}




/* copy data from the file structure to a char struct */
void	store_to_char(struct char_file_u *st, struct char_data *ch)
{
   int	i;

   GET_SEX(ch) = st->sex;
   GET_CLASS(ch) = st->class;
   GET_LEVEL(ch) = st->level;

   ch->player.short_descr = 0;
   ch->player.long_descr = 0;

   ch->player.hometown = st->hometown;

   ch->player.time.birth = st->birth;
   ch->player.time.played = st->played;
   ch->player.time.logon  = time(0);
   ch->player.time.last_logon = st->last_logon;

   ch->player.weight = st->weight;
   ch->player.height = st->height;

   ch->abilities = st->abilities;
   ch->tmpabilities = st->abilities;
   ch->points = st->points;
   ch->specials2 = st->specials2;

   if (CLAN(ch) >= 0)
     CLAN(ch) = real_clan(CLAN(ch));

   if (CLAN(ch) < 0)
     CLAN_LEVEL(ch) = 0;

   /* New dynamic skill system: only PCs have a skill array allocated. */
   CREATE(ch->skills, byte, MAX_SKILLS);
   for (i = 0; i < MAX_SKILLS; i++)
      SET_SKILL(ch, i, st->skills[i]);

   ch->specials.carry_weight = 0;
   ch->specials.carry_items  = 0;
   ch->points.armor          = 100;
   ch->points.hitroll        = 0;
   ch->points.damroll        = 0;

   CREATE(ch->player.name, char, strlen(st->name) + 1);
   strcpy(ch->player.name, st->name);
   strcpy(ch->player.passwd, st->pwd);

   /* Add all spell effects */
   for (i = 0; i < MAX_AFFECT; i++) {
      if (st->affected[i].type)
	 affect_to_char(ch, &st->affected[i]);
   }

   ch->in_room = real_room(GET_LOADROOM(ch));

   affect_total(ch);

   /* If you're not poisioned and you've been away for more than
      an hour, we'll set your HMV back to full */

   if (!IS_AFFECTED(ch, AFF_POISON) &&
       (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
   }

   REMOVE_BIT(PLR_FLAGS(ch), PLR_NOWHO);

   if (GET_LEVEL(ch) >= LEVEL_ADMIN)
     SET_BIT(GODLEVEL(ch), IMM_ALL);
   else
     REMOVE_BIT(GODLEVEL(ch), IMM_ALL);

   QUEST_FLAGS(ch) = 0;
} /* store_to_char */


/* copy vital data from a players char-structure to the file structure */
void	char_to_store(struct char_data *ch, struct char_file_u *st, bool u_time, bool u_logon)
{
   int	i;
   struct affected_type *af;
   struct obj_data *char_eq[MAX_WEAR];

   /* Unaffect everything a character can be affected by */

   for (i = 0; i < MAX_WEAR; i++) {
      if (ch->equipment[i])
	 char_eq[i] = unequip_char(ch, i);
      else
	 char_eq[i] = 0;
   }

   for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
      if (af && (af->type != SPELL_FROM_ITEM)) {
         st->affected[i] = *af;
         st->affected[i].next = 0;
	 af = af->next;
      } else {
	 st->affected[i].type = 0;  /* Zero signifies not used */
	 st->affected[i].duration = 0;
	 st->affected[i].modifier = 0;
	 st->affected[i].location = 0;
	 st->affected[i].bitvector = 0;
	 st->affected[i].next = 0;
      }
   }


   /* remove the affections so that the raw values are stored;
      otherwise the effects are doubled when the char logs back in. */

   while (ch->affected)
      affect_remove(ch, ch->affected);

   if ((i >= MAX_AFFECT) && af && af->next)
      log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

   ch->tmpabilities = ch->abilities;

   st->birth      = ch->player.time.birth;

   if (u_logon) {
     st->played     = ch->player.time.played;
     st->played    += (long) (time(0) - ch->player.time.logon);

     st->last_logon = time(0);
   } else {
     if (u_time) {
       st->played   = ch->player.time.played;
       st->played  += (long) (time(0) - ch->player.time.logon);
     } else
       st->played   = ch->player.time.played; /* no playtime update */

     st->last_logon = ch->player.time.last_logon; /* no last_logon update */
   }
   ch->player.time.played = st->played;
   ch->player.time.logon = time(0);


   st->hometown = ch->player.hometown;
   st->weight   = GET_WEIGHT(ch);
   st->height   = GET_HEIGHT(ch);
   st->sex      = GET_SEX(ch);
   st->class    = GET_CLASS(ch);
   st->level    = GET_LEVEL(ch);
   st->abilities = ch->abilities;
   st->points    = ch->points;
   st->specials2 = ch->specials2;

   if (CLAN(ch) >= 0)
     st->specials2.clan = clan_list[CLAN(ch)].vnum;  /* Save the clan vnum */

   st->points.armor   = 100;
   st->points.hitroll =  0;
   st->points.damroll =  0;

   for (i = 0; i < MAX_SKILLS; i++)
      st->skills[i] = GET_SKILL(ch, i);

   strcpy(st->name, GET_NAME(ch));
   strcpy(st->pwd, GET_PASSWD(ch));

   /* add spell and eq affections back in now */
   for (i = 0; i < MAX_AFFECT; i++) {
      if (st->affected[i].type)
	 affect_to_char(ch, &st->affected[i]);
   }

   for (i = 0; i < MAX_WEAR; i++) {
      if (char_eq[i])
	 equip_char(ch, char_eq[i], i);
   }

   affect_total(ch);
} /* Char to store */




/* create a new entry in the in-memory index table for the player file */
int	create_entry(char *name)
{
   /* int i; - Unused (Charlene) */

   if (top_of_p_table == -1) {
      CREATE(player_table, struct player_index_element, 1);
      top_of_p_table = 0;
   } else if (!(player_table = (struct player_index_element *)
       realloc(player_table, sizeof(struct player_index_element) *
       (++top_of_p_table + 1)))) {
      perror("create entry");
      exit(1);
   }
   /*
    * Don't copy lowercase equip of name to table field - Charlene
    * CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);
    *
    * copy lowercase equivalent of name to table field *
    * for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
    *   i++);
    */

   /* fix bug with uppercase names - Charlene */
   player_table[top_of_p_table].name = strdup(name);

   return (top_of_p_table);
}


/* write the vital data of a player to the player file */
void	save_char(struct char_data *ch, sh_int load_room, int mode)
{
  struct char_file_u st;

  if (IS_NPC(ch) || !ch->desc)
    return;

  switch(mode) {
  case 0 :
    /* Update nothing */
    char_to_store(ch, &st, FALSE, FALSE);
    break;
  case 1 :
    /* Update playtime but not login */
    char_to_store(ch, &st, FALSE, TRUE);
    break;
  case 2 :
    /* Update playtime and login */
    char_to_store(ch, &st, TRUE, TRUE);
    break;
  default :
    char_to_store(ch, &st, FALSE, FALSE);
    break;
  }

  strncpy(st.host, ch->desc->host, HOST_LEN);
  st.host[HOST_LEN] = '\0';

  st.specials2.load_room = load_room;

  fseek(player_fl, ch->desc->pos * sizeof(struct char_file_u), SEEK_SET);
  fwrite(&st, sizeof(struct char_file_u), 1, player_fl);

  REMOVE_BIT(PLR_FLAGS(ch), PLR_SAVECHR);
}


/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


char fread_letter(FILE *fp)
{
  char c;
  do {
    c = getc(fp);
  } while (isspace(c));
  return c;
}

/* read and allocate space for a '~'-terminated string from a given file */
char	*fread_string(FILE *fl, char *error)
{
    char	buf[MAX_STRING_LENGTH], tmp[512];
    char	*rslt;
    register char	*point;
    int	flag;

    memset((char *)buf, 0, (int)sizeof(buf));

    do {
	if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
	    fprintf(stderr, "fread_string: format error at or near %s\n", error);
	    exit(0);
	}

	if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH) {
	    log("SYSERR: fread_string: string too large (db.c)");
	    exit(0);
	} else
	    strcat(buf, tmp);

	for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
	     point--);

	if ((flag = (*point == '~')))
	    if (*(buf + strlen(buf) - 3) == '\n') {
		*(buf + strlen(buf) - 2) = '\r';
		*(buf + strlen(buf) - 1) = '\0';
	    } else
		*(buf + strlen(buf) - 2) = '\0';
	else {
	    *(buf + strlen(buf) + 1) = '\0';
	    *(buf + strlen(buf)) = '\r';
	}
    } while (!flag);

    /* do the allocate boogie  */

    if (strlen(buf) > 0) {
	CREATE(rslt, char, strlen(buf) + 1);
	strcpy(rslt, buf);
    } else
	rslt = 0;
    return(rslt);
}

/* Write a string to opened file, ending it with a ~ */
void fwrite_string(FILE *fl, char *str)
{
  char tempstr[LARGE_BUFSIZE];
  int  i, j, len;

  if(str != NULL) {
    len = strlen(str)+1;
    for(i = 0, j = 0; i <= len; i++) {
      if(str[i] != '\r') {
	if (str[i] == '~')
	  tempstr[j] = '^';
	else
	  tempstr[j] = str[i];
	j++;
      }
    }

    fprintf(fl, "%s~\n",tempstr);
  } else
    fprintf(fl, "~\n");
}

/* Read a number from file */
long  fread_number(FILE *fl, char *error)
{
    long number = 0;
    bool sign;
    char c;

    do {
	c = getc(fl);
    } while (isspace(c));

    sign = FALSE;

    if (c == '+') {
	c = getc(fl);
    } else if (c == '-') {
	sign = TRUE;
	c = getc(fl);
    }

    if (!isdigit(c)) {
      while (isalpha(c)) {
	if (islower(c))
	  number |= 1 << (c - 'a');
	else if (isupper(c))
	  number |= 1 << (26 + (c - 'A'));
	c = getc(fl);
      }

      if ( c != ' ' && c != '\n') {
	fprintf(stderr,"fread_number: Not digit error at or near %s\n", error);
	exit(0);
      }

      return number;
    }

    while (isdigit(c)) {
	number = number * 10 + c - '0';
	c      = getc(fl);
    }

    if (sign)
	number = 0 - number;

    if (c == '|')
	number += fread_number(fl, error);
    else if ( c != ' ' )
	ungetc(c, fl);

    return number;
}


/*  free alias_list  - Petrus  */
void free_alias_list(struct alias_list *ls)
{
    if (ls->next)
	free_alias_list(ls->next);

    free(ls->alias);
    free(ls->replace);
    free(ls);
}

/* release memory allocated for a char struct */
void	free_char(struct char_data *ch)
{
   int	i;
   extern void free_ignore_list(struct ignore *ls);

   if (ch->specials.poofIn)
      free(ch->specials.poofIn);
   if (ch->specials.poofOut)
      free(ch->specials.poofOut);
   if (ch->specials.transIn)
      free(ch->specials.transIn);
   if (ch->specials.transOut)
      free(ch->specials.transOut);
   if (ch->specials.wizname)            /* Free wizname  -Petrus */
       free(ch->specials.wizname);
   if (ch->specials.prename)            /* Free prename  -Petrus */
       free(ch->specials.prename);
   if (ch->specials.aliases)            /* Free aliases  -Petrus */
       free_alias_list(ch->specials.aliases);
   if (ch->specials.ignore_list)        /* Free ignore list      */
       free_ignore_list(ch->specials.ignore_list);

   if (ch->specials.deck_head)          /* Free cards    -Petrus */
       free_card_list(ch->specials.deck_head);
   if (ch->specials.cards_in_hand)
       free_card_list(ch->specials.cards_in_hand);
   if (ch->trackdir)
       free_stack(ch->trackdir);


   if (!IS_NPC(ch) || (IS_NPC(ch) && ch->nr == -1)) {
      if (GET_NAME(ch))
	 free(GET_NAME(ch));
      if (ch->player.title)
	 free(ch->player.title);
      if (ch->player.short_descr)
	 free(ch->player.short_descr);
      if (ch->player.long_descr)
	 free(ch->player.long_descr);
      if (ch->player.description)
	  free(ch->player.description);
      if (ch->player.plan)
	  free(ch->player.plan);

      clear_history(ch);

      if (ch->mobskills)  /* free mobskills for cloned chars */
	  free(ch->mobskills);
      if (ch->mob_specials.attacks)
	  free(ch->mob_specials.attacks);
      if (ch->mob_specials.resists)
	  free(ch->mob_specials.resists);


   } else if ((i = ch->nr) > -1) {
      if (ch->player.name && ch->player.name != mob_proto[i].player.name)
	 free(ch->player.name);
      if (ch->player.title && ch->player.title != mob_proto[i].player.title)
	 free(ch->player.title);
      if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
	 free(ch->player.short_descr);
      if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
	 free(ch->player.long_descr);
      if (ch->player.description && ch->player.description != mob_proto[i].player.description)
	 free(ch->player.description);
  }

   if (ch->skills) {
      free(ch->skills);
      if (IS_NPC(ch))
	 log("SYSERR: Mob had skills array allocated!");
   }

   while (ch->affected)
       affect_remove(ch, ch->affected);

   free(ch);
}




/* release memory allocated for an obj struct */
void	free_obj(struct obj_data *obj)
{
   int	nr;
   struct extra_descr_data *this, *next_one;

   if ((nr = obj->item_number) == -1) {
      if (obj->name)
	 free(obj->name);
      if (obj->description)
	 free(obj->description);
      if (obj->short_description)
	 free(obj->short_description);
      if (obj->action_description)
	 free(obj->action_description);
      if (obj->ex_description)
	 for (this = obj->ex_description; this; this = next_one) {
	    next_one = this->next;
	    if (this->keyword)
	       free(this->keyword);
	    if (this->description)
	       free(this->description);
	    free(this);
	 }
   } else {
      if (obj->name && obj->name != obj_proto[nr].name)
	 free(obj->name);
      if (obj->description && obj->description != obj_proto[nr].description)
	 free(obj->description);
      if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
	 free(obj->short_description);
      if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
	 free(obj->action_description);
      if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
	 for (this = obj->ex_description; this; this = next_one) {
	    next_one = this->next;
	    if (this->keyword)
	       free(this->keyword);
	    if (this->description)
	       free(this->description);
	    free(this);
	 }
   }

   free(obj);
}





/* read contets of a text file, alloc space, point buf to it */
int	file_to_string_alloc(char *name, char **buf)
{
   char	temp[MAX_STRING_LENGTH];

   if (file_to_string(name, temp) < 0)
      return -1;

   if (*buf)
      free(*buf);

   *buf = strdup(temp);

   return 0;
}




/* read contents of a text file, and place in buf */
int	file_to_string(char *name, char *buf)
{
   FILE * fl;
   char	tmp[100];

   *buf = '\0';

   if (!(fl = fopen(name, "r"))) {
      sprintf(tmp, "Error reading %s", name);
      perror(tmp);
      *buf = '\0';
      return(-1);
   }

   do {
      fgets(tmp, 99, fl);

      if (!feof(fl)) {
	 if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
	    log("SYSERR: fl->strng: string too big (db.c, file_to_string)");
	    *buf = '\0';
	    return(-1);
	 }

	 strcat(buf, tmp);
	 *(buf + strlen(buf) + 1) = '\0';
	 *(buf + strlen(buf)) = '\r';
      }
   } while (!feof(fl));

   fclose(fl);

   return(0);
}




/* clear some of the the working variables of a char */
void	reset_char(struct char_data *ch)
{
  int	i;

  for (i = 0; i < MAX_WEAR; i++) /* Initialisering */
    ch->equipment[i] = NULL;

  ch->followers = NULL;
  ch->master = NULL;
  /*	ch->in_room = NOWHERE; Used for start in room */
  ch->carrying = NULL;
  ch->next = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room = NULL;
  ch->specials.fighting = NULL;
  ch->specials.hunting = NULL;
  ch->specials.mounting = NULL;
  ch->specials.mounted_by = NULL;
  ch->specials.protecting = NULL;
  ch->specials.protected_by = NULL;
  ch->specials.last_tell = NULL;
  ch->specials.position = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;

  for (i = CHAN_GLOBAL_MAX; i < CHAN_MAX; i++)
       GET_HISTORY(ch, i) = NULL;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void	clear_char(struct char_data *ch)
{
   memset((char *)ch, 0, (int)sizeof(struct char_data));

   ch->in_room = NOWHERE;
   ch->specials.was_in_room = NOWHERE;
   ch->specials.last_tell = NULL;
   ch->trackdir = NULL;
   ch->specials.skillgain = 0;
   ch->specials.position = POS_STANDING;
   ch->mob_specials.default_pos = POS_STANDING;

   GET_AC(ch) = 100; /* Basic Armor */
}


void	clear_object(struct obj_data *obj)
{
   memset((char *)obj, 0, (int)sizeof(struct obj_data));

   obj->item_number = -1;
   obj->in_room	  = NOWHERE;
}




/* initialize a new character only if class is set */
void	init_char(struct char_data *ch)
{
  int	i, h, w;

  /* *** if this is our first player --- he be God *** */
  if (top_of_p_table < 0) {
    GET_LEVEL(ch) = LEVEL_IMPL;
    ch->points.max_hit = 9999;
    ch->points.max_mana = 9999;
    ch->points.max_move = 9999;
  }

  set_title(ch);

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  ch->player.description = 0;

  ch->player.hometown = number(1, 4);

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  GET_STR(ch) = 11;
  GET_INT(ch) = 11;
  GET_WIS(ch) = 11;
  GET_DEX(ch) = 11;
  GET_CON(ch) = 11;
  GET_CHA(ch) = 11;

  /* make favors for sex */
  h = height_average[GET_RACE(ch)];
  w = weight_average[GET_RACE(ch)];
  if (ch->player.sex == SEX_MALE) {
    ch->player.weight = number(w - 20 ,w + 20);
    ch->player.height = number(h - 30 ,h + 30);
  } else {
    ch->player.weight = number(w - 30, w + 10);
    ch->player.height = number(h - 40, h + 20);
  }

  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.armor = 100;

  ch->specials2.idnum = ++top_idnum;

  ch->specials2.worships = 0;	/* worships none */
  ch->specials2.clan = -1;
  ch->specials2.clanlevel = 0;
  ch->specials2.remorts = 0;
  ch->specials2.godlevel = 0;

  if (!ch->skills)
    CREATE(ch->skills, byte, MAX_SKILLS);

  for (i = 0; i < MAX_SKILLS; i++) {
    if (GET_LEVEL(ch) < LEVEL_IMPL)
      SET_SKILL(ch, i, 0)
	else
	  SET_SKILL(ch, i, 100);
  }

  ch->specials.affected_by = 0;
  ch->specials.affected_by2 = 0;
  ch->specials.wizname = 0;

  for (i = 0; i < 5; i++)
    ch->specials2.resistances[i] = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) == LEVEL_IMPL ? -1 : 24);

  if (r_newbie_clan >= 0) {
    CLAN(ch) = r_newbie_clan;
    CLAN_LEVEL(ch) = 2;
  }

  SET_BIT(PRF_FLAGS(ch), PRF_NOGOSS);
  SET_BIT(PLR_FLAGS(ch), PLR_NOPKSAY);
}


/* Returns the real zone number of the zone with given vnum */
int	real_zone(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_zone_table;

   /* perform binary search on world-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if (zone_table[mid].number == virtual)
	 return(mid);
      if (bot >= top) {
	  return(-1);
      }
      if (zone_table[mid].number > virtual)
	  top = mid - 1;
      else
	  bot = mid + 1;
   }
}


/* returns the real number of the room with given virtual number */
int	real_room(int vnum)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_world;

   /* perform binary search on world-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if (world[mid]->number == vnum)
	 return(mid);
      if (bot >= top)
 	 return(NOWHERE);
      if (world[mid]->number > vnum)
	 top = mid - 1;
      else
	 bot = mid + 1;
	 }
}






/* returns the real number of the monster with given virtual number */
int	real_mobile(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_mobt;

   /* perform binary search on mob-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if ((mob_index + mid)->virtual == virtual)
	 return(mid);
      if (bot >= top)
	 return(-1);
      if ((mob_index + mid)->virtual > virtual)
	 top = mid - 1;
      else
	 bot = mid + 1;
   }
}


/* returns the real number of the object with given virtual number */
int	real_object(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_objt;

   /* perform binary search on obj-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if ((obj_index + mid)->virtual == virtual)
	 return(mid);
      if (bot >= top)
	 return(-1);
      if ((obj_index + mid)->virtual > virtual)
	 top = mid - 1;
      else
	 bot = mid + 1;
   }
}


/* returns the real number of the clan with given virtual number */
int	real_clan(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_clan;

   /* perform binary search on obj-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if ((clan_list + mid)->vnum == virtual)
	 return(mid);
      if (bot >= top)
	 return(-1);
      if ((clan_list + mid)->vnum > virtual)
	 top = mid - 1;
      else
	 bot = mid + 1;
   }
}


/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

int mprog_name_to_type (char *name)
{
   if (!str_cmp(name, "in_file_prog"  ))    return IN_FILE_PROG;
   if (!str_cmp(name, "act_prog"      ))    return ACT_PROG;
   if (!str_cmp(name, "speech_prog"   ))    return SPEECH_PROG;
   if (!str_cmp(name, "rand_prog"     ))    return RAND_PROG;
   if (!str_cmp(name, "fight_prog"    ))    return FIGHT_PROG;
   if (!str_cmp(name, "hitprcnt_prog" ))    return HITPRCNT_PROG;
   if (!str_cmp(name, "death_prog"    ))    return DEATH_PROG;
   if (!str_cmp(name, "entry_prog"    ))    return ENTRY_PROG;
   if (!str_cmp(name, "greet_prog"    ))    return GREET_PROG;
   if (!str_cmp(name, "all_greet_prog"))    return ALL_GREET_PROG;
   if (!str_cmp(name, "give_prog"     ))    return GIVE_PROG;
   if (!str_cmp(name, "bribe_prog"    ))    return BRIBE_PROG;

   return(ERROR_PROG);
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp)
{
  char c;

  do {
    c = getc(fp);
  } while (c != '\n' && c != '\r');

  do {
    c = getc(fp);
  } while (c == '\n' || c == '\r');

  ungetc(c, fp);
  return;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp)
{
  static char word[MAX_INPUT_LENGTH];
  char *pword;
  char cEnd;

  do {
    cEnd = getc(fp);
  } while (isspace(cEnd));

  if (cEnd == '\'' || cEnd == '"') {
    pword   = word;
  } else {
    word[0] = cEnd;
    pword   = word+1;
    cEnd    = ' ';
  }

  for (; pword < word + MAX_INPUT_LENGTH; pword++) {
    *pword = getc(fp);
    if (cEnd == ' ' ? isspace(*pword) || *pword == '~' : *pword == cEnd) {
      if (cEnd == ' ' || cEnd == '~')
	ungetc(*pword, fp);
      *pword = '\0';
      return word;
    }
  }

  log("SYSERR: Fread_word: word too long.");
  exit(1);
  return NULL;
}


/* This routine reads in scripts of MOBprograms from a file */

MPROG_DATA* mprog_file_read(char *f, MPROG_DATA *mprg,
			    struct index_data *pMobIndex)
{
  char        MOBProgfile[MAX_INPUT_LENGTH];
  MPROG_DATA *mprg2;
  FILE       *progfile;
  char        letter;
  bool        done = FALSE;

  sprintf(MOBProgfile, "%s/%s", MOB_DIR, f);

  progfile = fopen(MOBProgfile, "r");
  if (!progfile) {
    sprintf(err_buf, "Mob: %d couldnt open mobprog file", pMobIndex->virtual);
    log(err_buf);
    exit(1);
  }

  mprg2 = mprg;
  switch (letter = fread_letter(progfile)) {
  case '>':
    break;
  case '|':
    log("empty mobprog file.");
    exit(1);
    break;
  default:
    log("in mobprog file syntax error.");
    exit(1);
    break;
  }

  while (!done) {
    mprg2->type = mprog_name_to_type(fread_word(progfile));
    switch (mprg2->type) {
    case ERROR_PROG:
      log("mobprog file type error");
      exit(1);
      break;
    case IN_FILE_PROG:
      log("mprog file contains a call to file.");
      exit(1);
      break;
    default:
      sprintf(buf2, "Error in file %s", f);
      pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
      mprg2->arglist       = fread_string(progfile,buf2);
      mprg2->comlist       = fread_string(progfile,buf2);
      switch (letter = fread_letter(progfile)) {
      case '>':
	mprg2->next = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
	mprg2       = mprg2->next;
	mprg2->next = NULL;
	break;
      case '|':
	done = TRUE;
	break;
      default:
	sprintf(err_buf,"in mobprog file %s syntax error.", f);
	log(err_buf);
	exit(1);
	break;
      }
      break;
    }
  }
  fclose(progfile);
  return mprg2;
}

struct index_data *get_obj_index (int vnum)
{
  int nr;
  for(nr = 0; nr <= top_of_objt; nr++) {
    if(obj_index[nr].virtual == vnum) return &obj_index[nr];
  }
  return NULL;
}

struct index_data *get_mob_index (int vnum)
{
  int nr;
  for(nr = 0; nr <= top_of_mobt; nr++) {
    if(mob_index[nr].virtual == vnum) return &mob_index[nr];
  }
  return NULL;
}

/* This procedure is responsible for reading any in_file MOBprograms.
 */

void mprog_read_programs(FILE *fp, struct index_data *pMobIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  pMobIndex->mobprogs = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
  mprg = pMobIndex->mobprogs;

  while (!done) {
    if ((letter = fread_letter(fp)) != '>') {
      sprintf(err_buf,"Load_mobiles: vnum %d MOBPROG char", pMobIndex->virtual);
      log(err_buf);
      exit(1);
    }
    mprg->type = mprog_name_to_type(fread_word(fp));
    switch (mprg->type) {
    case ERROR_PROG:
      sprintf(err_buf, "Load_mobiles: vnum %d MOBPROG type.", pMobIndex->virtual);
      log(err_buf);
      exit(1);
      break;
    case IN_FILE_PROG:
      sprintf(buf2, "Mobprog for mob #%d", pMobIndex->virtual);
      mprg = mprog_file_read(fread_word(fp), mprg,pMobIndex);
      fread_to_eol(fp);	/* need to strip off that silly ~*/
      switch (letter = fread_letter(fp)) {
      case '>':
	mprg->next = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
	mprg       = mprg->next;
	mprg->next = NULL;
	break;
      case '<':
      case '|':
	mprg->next = NULL;
	fread_to_eol(fp);
	done = TRUE;
	break;
      default:
	sprintf(err_buf, "Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->virtual);
	log(err_buf);
	exit(1);
	break;
      }
      break;
    default:
      sprintf(buf2, "Mobprog for mob #%d", pMobIndex->virtual);
      pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
      mprg->arglist        = fread_string(fp, buf2);
      mprg->comlist        = fread_string(fp, buf2);
      switch (letter = fread_letter(fp)) {
      case '>':
	mprg->next = (MPROG_DATA *)malloc(sizeof(MPROG_DATA));
	mprg       = mprg->next;
	mprg->next = NULL;
	break;
      case '<':
      case '|':
	mprg->next = NULL;
	fread_to_eol(fp);
	done = TRUE;
	break;
      default:
	sprintf(err_buf, "Load_mobiles: vnum %d bad MOBPROG (%c).", pMobIndex->virtual, letter);
	log(err_buf);
	exit(1);
	break;
      }
      break;
    }
  }

  return;
}


int rprog_name_to_type(char *name)
{
  if (!strcmp(name, "trans")) return TRANS_RPROG;
  if (!strcmp(name, "echo")) return ECHO_RPROG;
  if (!strcmp(name, "push")) return PUSH_RPROG;
  if (!strcmp(name, "ttrans")) return TTRANS_RPROG;
  if (!strcmp(name, "pushall")) return PUSHALL_RPROG;

  return ERROR_RPROG;
}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs(FILE *fp, struct room_data *room, char *name)
{
  RPROG_DATA *rprog;

  rprog = (RPROG_DATA *)malloc(sizeof(RPROG_DATA));
  rprog->type = rprog_name_to_type(name);

  if ((rprog->type) == ERROR_RPROG) {
    sprintf(err_buf, "Load Rooms: vnum %d bad ROOMPROG.", room->number);
    log(err_buf);
    exit(1);
  }

  rprog->arglist = fread_string(fp, buf2);
  rprog->comlist = fread_string(fp, buf2);
  rprog->next = room->rprogs;
  room->rprogs = rprog;
}


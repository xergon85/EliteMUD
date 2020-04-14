/* ************************************************************************
*   File: spec_assign.c                                 Part of EliteMUD  *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "functions.h"

extern struct room_data **world;
extern int top_of_world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

SPECIAL(postmaster);
SPECIAL(cityguard);
SPECIAL(guild_guard);
SPECIAL(mu_guild);
SPECIAL(cl_guild);
SPECIAL(th_guild);
SPECIAL(wa_guild);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(mayor);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(brass_dragon);
SPECIAL(drunk);
SPECIAL(priest);
SPECIAL(undertaker);
SPECIAL(roulette);
SPECIAL(questeq_shopkeeper);
SPECIAL(claneq_shopkeeper);
SPECIAL(cryo_shopkeeper);
SPECIAL(mob_block);
SPECIAL(tithe_cleric);
SPECIAL(goto_mayor);
SPECIAL(guard_caller);
SPECIAL(guildmaster);

/* From castle.c */
SPECIAL(CastleGuard);
SPECIAL(James);
SPECIAL(cleaning);
SPECIAL(DicknDavid);
SPECIAL(tim);
SPECIAL(tom);
SPECIAL(king_welmar);
SPECIAL(training_master);
SPECIAL(peter);
SPECIAL(jerry);



/* assign special procedures to mobiles */
void	assign_mobiles(void)
{
   void assign_kings_castle(void);

   assign_kings_castle();

   ASSIGNMOB(3095, cryo_shopkeeper);
   ASSIGNMOB(3098, questeq_shopkeeper);
   ASSIGNMOB(3097, claneq_shopkeeper);

   ASSIGNMOB(5005, brass_dragon);
   ASSIGNMOB(4061, cityguard);

   ASSIGNMOB(3059, cityguard);
   ASSIGNMOB(3060, cityguard);
   ASSIGNMOB(3067, cityguard);
   ASSIGNMOB(3061, janitor);
   ASSIGNMOB(3062, fido);
   ASSIGNMOB(3066, fido);
   ASSIGNMOB(3068, janitor);

   ASSIGNMOB(3010, postmaster);
   ASSIGNMOB(3070, postmaster);

   ASSIGNMOB(3015, priest);
   ASSIGNMOB(3016, undertaker);

   ASSIGNMOB(3020, mu_guild);
   ASSIGNMOB(3021, cl_guild);
   ASSIGNMOB(3022, th_guild);
   ASSIGNMOB(3023, wa_guild);

   ASSIGNMOB(3024, guild_guard);
   ASSIGNMOB(3025, guild_guard);
   ASSIGNMOB(3026, guild_guard);
   ASSIGNMOB(3027, guild_guard);

   ASSIGNMOB(3034, roulette);

   ASSIGNMOB(3064, drunk);   /*   Drunk */

   ASSIGNMOB(3143, mayor);

   ASSIGNMOB(7009, magic_user);
   ASSIGNMOB(6999, magic_user);

   /* MORIA */
   ASSIGNMOB(4000, snake);
   ASSIGNMOB(4001, snake);
   ASSIGNMOB(4053, snake);

   ASSIGNMOB(4103, thief);
   ASSIGNMOB(4100, magic_user);
   ASSIGNMOB(4102, snake);

   /* SEWERS */
   ASSIGNMOB(7006, snake);
   ASSIGNMOB(7200, magic_user);
   ASSIGNMOB(7201, magic_user);
   ASSIGNMOB(7240, magic_user);

   /* FOREST */
   ASSIGNMOB(6113, snake);

   /* ARACHNOS */
   ASSIGNMOB(6312, magic_user);
   ASSIGNMOB(6314, magic_user);
   ASSIGNMOB(6315, magic_user);
   ASSIGNMOB(6309, magic_user);
   ASSIGNMOB(6302, magic_user);

   /* DROW and THALOS */
   ASSIGNMOB(5104, magic_user);
   ASSIGNMOB(5014, magic_user);
   ASSIGNMOB(5107, magic_user);
   ASSIGNMOB(5108, magic_user);
   ASSIGNMOB(5200, magic_user);
   ASSIGNMOB(5201, magic_user);
   ASSIGNMOB(5004, magic_user);
   ASSIGNMOB(5103, magic_user);
   ASSIGNMOB(5352, magic_user);

   /* ROME */
   ASSIGNMOB(12018, cityguard);
   ASSIGNMOB(12021, cityguard);
   ASSIGNMOB(12009, magic_user);
   ASSIGNMOB(12025, magic_user);

   ASSIGNMOB(12020, magic_user);
   ASSIGNMOB(12025, magic_user);
   ASSIGNMOB(12030, magic_user);
   ASSIGNMOB(12031, magic_user);
   ASSIGNMOB(12032, magic_user);

   /* DWARVEN KINGDOM */
   ASSIGNMOB(6502, magic_user);
   ASSIGNMOB(6516, magic_user);
   ASSIGNMOB(6509, magic_user);
   ASSIGNMOB(6500, cityguard);

   /* NEW SPARTA */
   ASSIGNMOB(21068, guild_guard);
   ASSIGNMOB(21069, guild_guard);
   ASSIGNMOB(21070, guild_guard);
   ASSIGNMOB(21071, guild_guard);

   ASSIGNMOB(21072, mu_guild);
   ASSIGNMOB(21073, cl_guild);
   ASSIGNMOB(21074, th_guild);
   ASSIGNMOB(21075, wa_guild);
   ASSIGNMOB(21084, janitor);
   ASSIGNMOB(21085, fido);
   ASSIGNMOB(21057, magic_user);
   ASSIGNMOB(21054, magic_user);
   ASSIGNMOB(21011, magic_user);
   ASSIGNMOB(21003, cityguard);

   /* NEW THALOS */
   ASSIGNMOB(5700, mu_guild);
   ASSIGNMOB(5701, cl_guild);
   ASSIGNMOB(5702, wa_guild);
   ASSIGNMOB(5703, th_guild);
   ASSIGNMOB(5756, guild_guard);
   ASSIGNMOB(5757, guild_guard);
   ASSIGNMOB(5758, guild_guard);
   ASSIGNMOB(5759, guild_guard);

   /* Zone 131,132 */
   ASSIGNMOB(13138, mob_block);
   ASSIGNMOB(13139, mob_block);
   ASSIGNMOB(13280, tithe_cleric);
/*   ASSIGNMOB(13216, goto_mayor); */
   ASSIGNMOB(13149, guard_caller);
   ASSIGNMOB(13161, guard_caller);
   ASSIGNMOB(13160, guard_caller);
   ASSIGNMOB(13130, guard_caller);
   ASSIGNMOB(13128, guard_caller);
   ASSIGNMOB(13148, guard_caller);

   /* Zone 162 */
   ASSIGNMOB(16225, mu_guild);
   ASSIGNMOB(16223, cl_guild);
   ASSIGNMOB(16224, cl_guild);
   ASSIGNMOB(16208, wa_guild);
   ASSIGNMOB(16220, th_guild);
   ASSIGNMOB(16209, guild_guard);
   ASSIGNMOB(16221, guild_guard);
   ASSIGNMOB(16222, guild_guard);
   ASSIGNMOB(16226, guild_guard);
   ASSIGNMOB(16228, priest);

   /* Zone 169 */
   ASSIGNMOB(16931, postmaster);
   ASSIGNMOB(16946, guildmaster); 
   ASSIGNMOB(16906, mob_block);
   ASSIGNMOB(16942, mob_block);


}



SPECIAL(bank);
SPECIAL(gen_board);
SPECIAL(rigelskey);
SPECIAL(social_object);
SPECIAL(spraycan);
SPECIAL(guard_object);

/* assign special procedures to objects */
void	assign_objects(void)
{
   ASSIGNOBJ(2113, gen_board);  /* PK board */
   ASSIGNOBJ(3084, gen_board);  /* Code board */
   ASSIGNOBJ(3094, gen_board);  /* General board */
   ASSIGNOBJ(3095, gen_board);  /* quest board */
   ASSIGNOBJ(3096, gen_board);  /* social board */
   ASSIGNOBJ(3097, gen_board);  /* freeze board */
   ASSIGNOBJ(3098, gen_board);	/* immortal board */
   ASSIGNOBJ(3099, gen_board);  /* mortal board */
   ASSIGNOBJ( 899, gen_board);  /* chalumeau board */
   ASSIGNOBJ( 104, gen_board);  /* overseer board */
   ASSIGNOBJ( 118, gen_board);  /* IO's notebook */
   ASSIGNOBJ( 122, gen_board);  /* Glob's notebook */
   ASSIGNOBJ( 123, gen_board);  /* Articus's notebook */
   ASSIGNOBJ( 134, gen_board);  /* Bod's notebook */
   ASSIGNOBJ( 136, gen_board);  /* Drusian's notebook */
   ASSIGNOBJ( 137, gen_board);  /* Rahl's notebook */
   ASSIGNOBJ( 138, gen_board);  /* Israfel's notebook */
   ASSIGNOBJ( 139, gen_board);  /* Criz's notebook */
   ASSIGNOBJ( 148, gen_board);  /* Sapphyre's notebook */
   ASSIGNOBJ( 149, gen_board);  /* Kestrel's notebook */
   ASSIGNOBJ( 152, gen_board);  /* Phobos's notebook */
   ASSIGNOBJ( 156, gen_board);  /* Melody's notebook */
   ASSIGNOBJ( 157, gen_board);  /* Amok's notebook */
   ASSIGNOBJ( 158, gen_board);  /* Turf's notebook */
   ASSIGNOBJ( 159, gen_board);  /* Chani's notebook */
   ASSIGNOBJ( 160, gen_board);  /* Titan's notebook */
   ASSIGNOBJ( 161, gen_board);  /* Maerlyn's notebook */
   ASSIGNOBJ( 162, gen_board);  /* Erebos's notebook */
   ASSIGNOBJ( 163, gen_board);  /* Talon's notebook */
   ASSIGNOBJ( 164, gen_board);  /* Kurrelgyre's notebook */
   ASSIGNOBJ(3087, gen_board);  /* newbie board */
   ASSIGNOBJ(3088, gen_board);  /* area board */
   ASSIGNOBJ(3089, gen_board);  /* world board */
   ASSIGNOBJ(3090, gen_board);  /* object board */
   ASSIGNOBJ(3091, gen_board);  /* mob board */
   ASSIGNOBJ(3092, gen_board);  /* skill&spell board */
   ASSIGNOBJ(3093, gen_board);  /* remort board */
   ASSIGNOBJ(3082, gen_board);  /* arena board */
   ASSIGNOBJ( 135, gen_board);  /* Holy Rules Book */
   ASSIGNOBJ(   7, gen_board);  /* NT board 2 */
   ASSIGNOBJ(   8, gen_board);  /* Midnight Sun Board */
   ASSIGNOBJ(  15, gen_board);  /* ELITE board */
   ASSIGNOBJ(  16, gen_board);  /* cod board */
   ASSIGNOBJ(  17, gen_board);  /* ELYSIUM board */
   ASSIGNOBJ(  27, gen_board);  /* TRI board */
   ASSIGNOBJ(  29, gen_board);  /* SD board */
   ASSIGNOBJ(  30, gen_board);  /* tkk board */
   ASSIGNOBJ(  31, gen_board);  /* KAI board */
   ASSIGNOBJ(28202,gen_board);  /* KAI board 2 */
   ASSIGNOBJ(  32, gen_board);  /* Ntribe board */
   ASSIGNOBJ(  33, gen_board);  /* Malina board */
   ASSIGNOBJ(  34, gen_board);  /* Art board */
   ASSIGNOBJ(  36, gen_board);  /* moc board */
   ASSIGNOBJ(28000, gen_board); /* dragoon board */
   ASSIGNOBJ(28001, gen_board); /* dragoon board 2 */
   ASSIGNOBJ(  40, gen_board);  /* AG board */
   ASSIGNOBJ(28826,gen_board);  /* west wind board */
   ASSIGNOBJ(28825,gen_board);  /* west wind board 2 */
   ASSIGNOBJ(  44, gen_board);  /* norsca board */
   ASSIGNOBJ(  47, gen_board);  /* Ni board */
   ASSIGNOBJ(28102,gen_board);  /* tkk council board */
   ASSIGNOBJ(27903,gen_board);  /* Amber Circle  board 2 */
   ASSIGNOBJ(27509,gen_board);  /* Midnight Sun board 2 */

   ASSIGNOBJ(3034, bank);
   ASSIGNOBJ(6158, rigelskey);
   ASSIGNOBJ(114, social_object);
   ASSIGNOBJ(2418, social_object);
   ASSIGNOBJ(121, spraycan);

   ASSIGNOBJ(10, guard_object);
}


SPECIAL(newbie_only);
SPECIAL(dump);
SPECIAL(pet_shops);
SPECIAL(pray_for_items);
SPECIAL(poker);

/* assign special procedures to rooms */
void	assign_rooms(void)
{
   extern int dts_are_dumps;
   int i;

   ASSIGNROOM(2000, newbie_only);
   ASSIGNROOM(3030, dump);
   ASSIGNROOM(3031, pet_shops);
   ASSIGNROOM(21116, pet_shops);
   ASSIGNROOM(12196, pet_shops);
   ASSIGNROOM(13282, pet_shops);
   ASSIGNROOM(16274, pet_shops);

   ASSIGNROOM(3054, pray_for_items);

   ASSIGNROOM(   6, poker);
   ASSIGNROOM(3067, poker);
   ASSIGNROOM(3099, poker);

   if (dts_are_dumps)
      for (i = 0; i < top_of_world; i++)
         if (IS_SET(world[i]->room_flags, DEATH))
            world[i]->funct = dump;
}


struct special_info {
  char *specname;
  int  (*spec_pointer)(struct char_data *ch, struct char_data *mob, struct obj_data *obj, int cmd, char *arg);
  ubyte type;
} special_list[] = {
   { "postmaster"        , postmaster        , SPEC_MOB },
   { "cityguard"         , cityguard         , SPEC_MOB },
   { "guild_guard"       , guild_guard       , SPEC_MOB },
   { "mu_guild"          , mu_guild          , SPEC_MOB },
   { "cl_guild"          , cl_guild          , SPEC_MOB },
   { "th_guild"          , th_guild          , SPEC_MOB },
   { "wa_guild"          , wa_guild          , SPEC_MOB },
   { "fido"              , fido              , SPEC_MOB },
   { "janitor"           , janitor           , SPEC_MOB },
   { "mayor"             , mayor             , SPEC_MOB },
   { "snake"             , snake             , SPEC_MOB },
   { "thief"             , thief             , SPEC_MOB },
   { "magic_user"        , magic_user        , SPEC_MOB },
   { "brass_dragon"      , brass_dragon      , SPEC_MOB },
   { "drunk"             , drunk             , SPEC_MOB },
   { "priest"            , priest            , SPEC_MOB },
   { "undertaker"        , undertaker        , SPEC_MOB },
   { "questeq_shopkeeper", questeq_shopkeeper, SPEC_MOB },
   { "cryo_shopkeeper"   , cryo_shopkeeper   , SPEC_MOB },
   { "mob_block"         , mob_block         , SPEC_MOB },
   { "tithe_cleric"      , tithe_cleric      , SPEC_MOB },
   { "goto_mayor"        , goto_mayor        , SPEC_MOB },
   { "guard_caller"      , guard_caller      , SPEC_MOB },
   { "guildmaster"       , guildmaster       , SPEC_MOB },
   { "roulette"          , roulette          , SPEC_MOB },
   { "CastleGuard"       , CastleGuard       , SPEC_MOB },
   { "James"             , James             , SPEC_MOB },
   { "cleaning"          , cleaning          , SPEC_MOB },
   { "DicknDavid"        , DicknDavid        , SPEC_MOB },
   { "tim"               , tim               , SPEC_MOB },
   { "tom"               , tom               , SPEC_MOB },
   { "king_welmar"       , king_welmar       , SPEC_MOB },
   { "training_master"   , training_master   , SPEC_MOB },
   { "peter"             , peter             , SPEC_MOB },
   { "jerry"             , jerry             , SPEC_MOB },

   { "bank"              , bank              , SPEC_OBJ },
   { "gen_board"         , gen_board         , SPEC_OBJ },
   { "rigelskey"         , rigelskey         , SPEC_OBJ },
   { "social_object"     , social_object     , SPEC_OBJ },
   { "spraycan"          , spraycan          , SPEC_OBJ },
   { "guard_object"      , guard_object      , SPEC_OBJ },

   { "newbie_only"       , newbie_only       , SPEC_ROOM },
   { "dump"              , dump              , SPEC_ROOM },
   { "pet_shops"         , pet_shops         , SPEC_ROOM },
   { "pray_for_items"    , pray_for_items    , SPEC_ROOM },
   { "poker"             , poker             , SPEC_ROOM },

   { "\n"          , NULL        , 0 }
};



void special_assign(char* str, ubyte type, int  (**ptr)(struct char_data *ch, struct char_data *mob, struct obj_data *obj, int cmd, char *arg), char *errbuf)
{
  int i = 0;
  
  if (!ptr) {
    perror("special_assign: Spec_pointer to (null)");
    exit(0);
  }

  while (str && *str != '(') {
    if (*str == '\n' || *str == ')') {
      fprintf(stderr,"special_assign: Syntax error at or near %s\n", errbuf);
      exit(0);
    }
    str++;
  }
    
  str++;
  for (i = 0; (str + i); i++) {
    if (*(str + i) == ')') {
      *(str + i) = 0;
      break;
    }
  }

  for (i = 0; *special_list[i].specname != '\n'; i++)
    if (!strcmp(special_list[i].specname, str))
      break;
  
  if (*special_list[i].specname == '\n') {
    sprintf(buf, "special_assign: No such spec_proc '%s' at %s", str, errbuf);
    log(buf);
    return;
  }

  if (!IS_SET(special_list[i].type, type)) {
    sprintf(buf, "special_assign: spec_proc of wrong type at %s", errbuf);
    log(buf);
    return;
  }

  *ptr = special_list[i].spec_pointer;
}


char *
spec_proc_by_name(int  (*spec_pointer)(struct char_data *ch, struct char_data *mob, struct obj_data *obj, int cmd, char *arg))
{
  int i;
  
  if (!spec_pointer)
    return ("None");

  for (i = 0; *special_list[i].specname != '\n'; i++)
    if (spec_pointer == special_list[i].spec_pointer)
      return special_list[i].specname;
  
  return ("Unknown");
}

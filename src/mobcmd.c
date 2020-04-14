/***************************************************************************
 * MOBProgram ported for CircleMUD 3.0 by Mattias Larsson		   *
 * Traveller@AnotherWorld (ml@eniac.campus.luth.se 4000) 		   *
 **************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "interpreter.h"
#include "comm.h"
#include "functions.h"

extern struct index_data *mob_index;
extern struct room_data **world;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;

extern struct index_data *get_mob_index(int vnum);
extern struct index_data *get_obj_index(int vnum);

extern sh_int find_target_room(struct char_data * ch, char *rawroomstr);

#define bug(x, y) { sprintf(buf2, (x), (y)); log(buf2); }

/*
 * Local functions.
 */

char * mprog_type_to_name(int type);

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */

char *mprog_type_to_name(int type)
{
  switch (type) {
  case IN_FILE_PROG:          return "in_file_prog";
  case ACT_PROG:              return "act_prog";
  case SPEECH_PROG:           return "speech_prog";
  case RAND_PROG:             return "rand_prog";
  case FIGHT_PROG:            return "fight_prog";
  case HITPRCNT_PROG:         return "hitprcnt_prog";
  case DEATH_PROG:            return "death_prog";
  case ENTRY_PROG:            return "entry_prog";
  case GREET_PROG:            return "greet_prog";
  case ALL_GREET_PROG:        return "all_greet_prog";
  case GIVE_PROG:             return "give_prog";
  case BRIBE_PROG:            return "bribe_prog";
  default:                    return "ERROR_PROG";
  }
}

/* string prefix routine */
bool str_prefix(const char *astr, const char *bstr)
{
  if (!astr) {
    log("Strn_cmp: null astr.");
    return TRUE;
  }
  if (!bstr) {
    log("Strn_cmp: null astr.");
    return TRUE;
  }
  for(; *astr; astr++, bstr++) {
    if(LOWER(*astr) != LOWER(*bstr)) return TRUE;
  }
  return FALSE;
}


ACMD(do_mpstat)
{
  MPROG_DATA *mprg;
  struct char_data *victim;
  
  one_argument( argument, arg );
  
  if (arg[0] == '\0') {
    send_to_char( "MobProg stat whom?\r\n", ch );
    return;
  }

  if (!(victim = get_char_vis(ch, arg))) {
    send_to_char( "They aren't here.\r\n", ch );
    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char( "Only Mobiles can have Programs!\r\n", ch);
    return;
  }

  if (!(mob_index[victim->nr].progtypes)) {
    send_to_char( "That Mobile has no Programs set.\r\n", ch);
    return;
  }

  sprintf(buf, "Level #r%d#N - #c%s#N Vnum[#B%d#N]\r\n",
	  GET_LEVEL(victim), victim->player.name,
	  mob_index[victim->nr].virtual );
  
  for (mprg = mob_index[victim->nr].mobprogs; mprg; mprg = mprg->next) {
    sprintf(buf, "%s#r>%s#y%s#N\r\n%s\r\n", buf,
	    mprog_type_to_name(mprg->type),
	    mprg->arglist,
	    mprg->comlist );
  }
  page_string(ch->desc, buf, TRUE);
  return;
}

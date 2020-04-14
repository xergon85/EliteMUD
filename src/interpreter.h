/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (cmdnum >= 1 && cmdnum <= 6)

/* necessary for CMD_IS macro */
#ifndef __INTERPRETER_C__
extern struct command_info cmd_info[];
#endif

#define ACMD(c)  \
   void (c)(struct char_data *ch,char *argument,int cmd,int subcmd)


#define ASSIGNMOB(mob, fname) { if (real_mobile(mob) >= 0) \
				mob_index[real_mobile(mob)].func = fname; }

#define ASSIGNOBJ(obj, fname) { if (real_object(obj) >= 0) \
				obj_index[real_object(obj)].func = fname; }

#define ASSIGNROOM(room, fname) { if (real_room(room) >= 0) \
				world[real_room(room)]->funct = fname; }

struct command_info {
  char *command;
  byte minimum_position;
  void	(*command_pointer)
    (struct char_data *ch, char * argument, int cmd, int subcmd);
  sh_int minimum_level;
  int	subcmd;
};

struct immcommand_info {
  char *command;
  long godlevel;
};

/*
struct command_info {
   void	(*command_pointer)
   (struct char_data *ch, char * argument, int cmd, int subcmd);
   byte minimum_position;
   sh_int minimum_level;
   sh_int sort_pos;
   int	subcmd;
   byte is_social;
};
*/

#define SCMD_INFO       100
#define SCMD_HANDBOOK   101
#define SCMD_CREDITS    102
#define SCMD_NEWS       103
#define SCMD_WIZLIST    104
#define SCMD_POLICIES   105
#define SCMD_VERSION    106
#define SCMD_IMMLIST    107
#define SCMD_CLEAR	108
#define SCMD_WHOAMI	109
#define SCMD_NEWBIE     110
#define SCMD_MOTD       111
#define SCMD_IMOTD      112
#define SCMD_REMLIST    113
#define SCMD_MOBPROG    999

#define SCMD_TOG_BASE	201
#define SCMD_NOSUMMON   201
#define SCMD_NOHASSLE   202
#define SCMD_BRIEF      203
#define SCMD_COMPACT    204
#define SCMD_NOTELL	    205
#define SCMD_NOAUCTION	206
#define SCMD_NONEWBIE	207
#define SCMD_NOGOSSIP	208
#define SCMD_NOCHAT	    209
#define SCMD_NOWIZ	    210
#define SCMD_QUEST	    211
#define SCMD_ROOMFLAGS	212
#define SCMD_NOREPEAT	213
#define SCMD_HOLYLIGHT	214
#define SCMD_SLOWNS	    215
#define SCMD_NOSPDWLK   216
#define SCMD_NOALIAS    217
#define SCMD_VERBATIM   218
#define SCMD_EXITS      219
#define SCMD_GAG        220
#define SCMD_PC         221
#define SCMD_NOGRAT     222
#define SCMD_LASTCMD    223
#define SCMD_NOCLANTIT  224
#define SCMD_NOCLANTELL 225
#define SCMD_LINEWRAP   226
#define SCMD_AUTOASSIST 227
#define SCMD_NOWHO      228
#define SCMD_IDENT      229
#define SCMD_DETAIL	    230
#define SCMD_NOPKSAY    231

#define SCMD_PARDON     301
#define SCMD_NOTITLE    302
#define SCMD_SQUELCH    303
#define SCMD_FREEZE	304
#define SCMD_THAW	305
#define SCMD_UNAFFECT	306
#define SCMD_REROLL	307

/* do_gen_com */
#define SCMD_NEWBIEC	0
#define SCMD_YELL	1
#define SCMD_GOSSIP	2
#define SCMD_AUCTION	3
#define SCMD_CHAT       4
#define SCMD_PKSAY      5
#define SCMD_GRAT       6

/* do_shutdown */
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUIT	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2
#define SCMD_SACRIFICE  3

/* do_skillset */
#define SCMD_SKILLCLEAR  1

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2

/* do_qcomm */
#define SCMD_QECHO	1

/* do_wizname */
#define SCMD_WIZNAME    0
#define SCMD_PRENAME    1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_poofset */
#define SCMD_POOFIN	0
#define SCMD_POOFOUT	1
#define SCMD_POOFVIEW   2

/* do_hit */
#define SCMD_HIT	0
#define SCMD_MURDER	1

/* do_drink */
#define SCMD_EAT	0
#define SCMD_TASTE	1
#define SCMD_DRINK	2
#define SCMD_SIP	3

/* do_practice */
#define SCMD_SKILLS     1
#define SCMD_SPELLS     2

/* do_mount */
#define SCMD_DISMOUNT   1

/* do_qcomm */
#define SCMD_QSAY       0
#define SCMD_QECHO      1

/* do_use */
#define SCMD_RECITE     1
#define SCMD_QUAFF      2
#define SCMD_USE        3

/* do_worship */
#define SCMD_WORSHI     0
#define SCMD_WORSHIP    1

/* For restore_mortals */
#define SCMD_ALL        0
#define SCMD_ROOM 	1
#define SCMD_ZONE 	2

/* do_transset */
#define SCMD_TRANSIN    0
#define SCMD_TRANSOUT   1
#define SCMD_TRANSVIEW  2


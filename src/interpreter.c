/* ************************************************************************
*   File: interpreter.c                                 Part of EliteMUD  *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#define __INTERPRETER_C__

#include "crypt.h"
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limits.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "objsave.h"
#include "functions.h"
#include "ignore.h"


extern const struct title_type titles[20][40];
extern char	*motd;
extern char	*imotd;
extern char	*background;
extern char	*MENU;
extern char     *PC_MENU;
extern char	*WELC_MESSG;
extern char	*START_MESSG;
extern char 	*DEAD_MESSG;
extern char     *BYE_MESSG;
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int      save_last_command;
extern int	top_of_p_table;
extern int	restrict;
extern int      allowed_classes[];
extern char     *race_table[];
extern char     *pc_class_types[];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data **world;
extern char    *imm_powers[];

extern int     cmds_executed;        /* Commands executed since last */
extern int     plr_cmds_executed;    /* Commands by players */

/* local global variables */
DISABLED_DATA *disabled_first;

/* local functions */
int check_disabled(const struct command_info *command);
void load_disabled(void);
void save_disabled(void);



/* prototypes for all do_x functions. */
ACMD(do_move); ACMD(do_look); ACMD(do_read); ACMD(do_say); ACMD(do_exit);
ACMD(do_snoop); ACMD(do_insult); ACMD(do_quit); ACMD(do_help); ACMD(do_who);
ACMD(do_emote); ACMD(do_echo); ACMD(do_trans); ACMD(do_kill); ACMD(do_stand);
ACMD(do_sit); ACMD(do_rest); ACMD(do_sleep); ACMD(do_wake); ACMD(do_force);
ACMD(do_get); ACMD(do_drop); ACMD(do_score); ACMD(do_inventory);
ACMD(do_equipment); ACMD(do_not_here); ACMD(do_tell); ACMD(do_wear);
ACMD(do_wield); ACMD(do_grab); ACMD(do_remove); ACMD(do_put);
ACMD(do_shutdown); ACMD(do_save); ACMD(do_hit); ACMD(do_give); ACMD(do_stat);
ACMD(do_skillset); ACMD(do_set); ACMD(do_time); ACMD(do_weather);
ACMD(do_load); ACMD(do_vstat); ACMD(do_purge); ACMD(do_whisper); ACMD(do_cast);
ACMD(do_at); ACMD(do_goto); ACMD(do_ask); ACMD(do_drink); ACMD(do_eat);
ACMD(do_pour); ACMD(do_order); ACMD(do_follow); ACMD(do_rent); ACMD(do_offer);
ACMD(do_advance); ACMD(do_close); ACMD(do_open); ACMD(do_lock);
ACMD(do_unlock); ACMD(do_exits); ACMD(do_enter); ACMD(do_leave);
ACMD(do_write); ACMD(do_flee); ACMD(do_sneak); ACMD(do_hide);
ACMD(do_backstab); ACMD(do_pick); ACMD(do_steal); ACMD(do_bash);
ACMD(do_rescue);ACMD(do_kick); ACMD(do_examine); ACMD(do_info);
ACMD(do_users); ACMD(do_where); ACMD(do_levels); ACMD(do_consider);
ACMD(do_group); ACMD(do_restore); ACMD(do_return); ACMD(do_switch);
ACMD(do_use); ACMD(do_credits); ACMD(do_locate); ACMD(do_identname);
ACMD(do_display); ACMD(do_poofset); ACMD(do_teleport); ACMD(do_gecho);
ACMD(do_wiznet); ACMD(do_invis); ACMD(do_wimpy); ACMD(do_wizlock); ACMD(do_dc);
ACMD(do_gsay); ACMD(do_title); ACMD(do_visible); ACMD(do_assist);
ACMD(do_split); ACMD(do_toggle); ACMD(do_send); ACMD(do_vnum); ACMD(do_action);
ACMD(do_practice); ACMD(do_uptime); ACMD(do_commands); ACMD(do_ban);
ACMD(do_unban); ACMD(do_date); ACMD(do_zreset); ACMD(do_gen_write);
ACMD(do_gen_ps); ACMD(do_gen_tog); ACMD(do_gen_com); ACMD(do_wizutil);
ACMD(do_color); ACMD(do_syslog); ACMD(do_show); ACMD(do_ungroup);
ACMD(do_page); ACMD(do_diagnose); ACMD(do_qcomm); ACMD(do_reboot);
ACMD(do_last); ACMD(do_track); ACMD(do_wizname); ACMD(do_pstatus);
ACMD(do_alias); ACMD(do_throw); ACMD(do_flip); ACMD(do_aid); ACMD(do_finger);
ACMD(do_berzerk); ACMD(do_cards); ACMD(do_redraw); ACMD(do_resize);
ACMD(do_scout); ACMD(do_setinnate); ACMD(do_mount); ACMD(do_protect);
ACMD(do_disarm); ACMD(do_circle); ACMD(do_reply); ACMD(do_pray);
ACMD(do_worship); ACMD(do_redit); ACMD(do_clan); ACMD(do_land);
ACMD(do_clantell); ACMD(do_sitename); ACMD(do_ocsrsave); ACMD(do_eedit);
ACMD(do_pose); ACMD(do_temp); ACMD(do_oedit); ACMD(do_count);ACMD(do_claneq);
ACMD(do_medit); ACMD(do_mpstat); ACMD(do_pfunctions);ACMD(do_appraise);
ACMD(do_hunt); ACMD(do_skillstat); ACMD(do_recall); ACMD(do_fly);ACMD(do_peek);
ACMD(do_award); ACMD(do_quest); ACMD(do_activate); ACMD(do_questeq);
ACMD(do_duplicate);ACMD(do_immset);ACMD(do_immstat);ACMD(do_zpurge);
ACMD(do_wargame);ACMD(do_shoot);ACMD(do_join);ACMD(do_teams);ACMD(do_rpstat);
ACMD(do_log);ACMD(do_plog);ACMD(do_peace);ACMD(do_xlag);
ACMD(do_vlist); ACMD(do_tedit); ACMD(do_view); ACMD(do_unaffect); ACMD(do_config);
ACMD(do_transet);ACMD(do_ignore);ACMD(do_chanset);ACMD(do_disable);
ACMD(do_whereload); ACMD(do_findkey); ACMD(do_finddoor);

const struct command_info cmd_info[] = {
  { "RESERVED", 0, 0, 0, 0 },	/* this must be first -- for specprocs */
  { "north"      , POS_STANDING , do_move     , 0,           0 },
  { "east"       , POS_STANDING , do_move     , 0,           0 },
  { "south"      , POS_STANDING , do_move     , 0,           0 },
  { "west"       , POS_STANDING , do_move     , 0,           0 },
  { "up"         , POS_STANDING , do_move     , 0,           0 },
  { "down"       , POS_STANDING , do_move     , 0,           0 },
/* Anything below can be in any order, but movement must be in this order */

  { "emote"      , POS_SLEEPING , do_emote    , 1,           0 },
  { ":"          , POS_SLEEPING , do_emote    , 1,           0 },
  { "say"        , POS_RESTING  , do_say      , 0,           0 },
  { "'"          , POS_RESTING  , do_say      , 0,           0 },
  { "tell"       , POS_DEAD     , do_tell     , 0,           0 },
  { "reply"      , POS_DEAD     , do_reply    , 0,           0 },
  { "<"          , POS_DEAD     , do_reply    , 0,           0 },
  { "qui"        , POS_DEAD     , do_quit     , 0,           0 },
  { "quit"       , POS_DEAD     , do_quit     , 0,           SCMD_QUIT },
  { "save"       , POS_SLEEPING , do_save     , 0,           0 },
  { "alias"      , POS_DEAD     , do_alias    , 0,           0 },
  { "cast"       , POS_SITTING  , do_cast     , 1,           0 },
  { "kill"       , POS_FIGHTING , do_kill     , 0,           0 },
  { "assist"     , POS_FIGHTING , do_assist   , 1,           0 },
  { "who"        , POS_DEAD     , do_who      , 0,           0 },
  { "transfer"   , POS_SLEEPING , do_trans    , LEVEL_DEITY, 0 },
  { "transet"    , POS_DEAD     , do_transet  , LEVEL_DEITY, 0 },
  { "goto"       , POS_SLEEPING , do_goto     , LEVEL_DEITY, 0 },
  { "at"         , POS_DEAD     , do_at       , LEVEL_DEITY, 0 },
  { "set"        , POS_DEAD     , do_set      , LEVEL_DEITY, 0 },
  { "give"       , POS_RESTING  , do_give     , 0,           0 },
  { "look"       , POS_RESTING  , do_look     , 0,           0 },
  { "stats"      , POS_DEAD     , do_stat     , 0,           0 },
  { "score"      , POS_DEAD     , do_score    , 0,           0 },
  { "scout"      , POS_STANDING , do_scout    , 1,           0 },
  { "track"      , POS_STANDING , do_track    , 0,           0 },
  { "-"          , POS_DEAD     , do_wiznet   , LEVEL_DEITY, 0 },
  { "chat"       , POS_SLEEPING , do_gen_com  , 0,           SCMD_CHAT },
  { "."          , POS_SLEEPING , do_gen_com  , 0,           SCMD_CHAT },
  { "gossip"     , POS_SLEEPING , do_gen_com  , 0,           SCMD_GOSSIP },
  { ","          , POS_DEAD     , do_gen_com  , 0,           SCMD_GOSSIP },
  { "auction"    , POS_SLEEPING , do_gen_com  , 0,           SCMD_AUCTION },

  { "hit"        , POS_FIGHTING , do_hit      , 0,           SCMD_HIT },
  { "drop"       , POS_RESTING  , do_drop     , 0,           SCMD_DROP },
  { "get"        , POS_RESTING  , do_get      , 0,           0 },
  { "wear"       , POS_RESTING  , do_wear     , 0,           0 },
  { "wield"      , POS_RESTING  , do_wield    , 0,           0 },
  { "remove"     , POS_RESTING  , do_remove   , 0,           0 },
  { "inventory"  , POS_DEAD     , do_inventory, 0,           0 },
  { "ignore"     , POS_SLEEPING , do_ignore   , 0,           0 },
  { "equipment"  , POS_SLEEPING , do_equipment, 0,           0 },
  { "sacrifice"  , POS_RESTING  , do_drop     , 0,           SCMD_JUNK },
  { "count"      , POS_DEAD     , do_count    ,          0,  0 },
  { "drink"      , POS_RESTING  , do_drink    , 0,           SCMD_DRINK },
  { "eat"        , POS_RESTING  , do_eat      , 0,           SCMD_EAT },
  { "use"        , POS_SITTING  , do_use      , 1,           SCMD_USE },
  { "quaff"      , POS_RESTING  , do_use      , 0,           SCMD_QUAFF },
  { "recite"     , POS_RESTING  , do_use      , 0,           SCMD_RECITE },
  { "finger"     , POS_SLEEPING , do_finger   , 0,           0 },
  { "read"       , POS_RESTING  , do_read     , 0,           0 },
  { "write"      , POS_RESTING  , do_write    , 1,           0 },
  { "amend"      , POS_RESTING  , do_not_here , 1,           0 },
  { "exits"      , POS_RESTING  , do_exits    , 0,           0 },
  { "open"       , POS_SITTING  , do_open     , 0,           0 },
  { "close"      , POS_SITTING  , do_close    , 0,           0 },
  { "lock"       , POS_SITTING  , do_lock     , 0,           0 },
  { "unlock"     , POS_SITTING  , do_unlock   , 0,           0 },
  { "yell"       , POS_RESTING  , do_gen_com  , 0,           SCMD_YELL },
  { "grat"       , POS_SLEEPING , do_gen_com  , 0,           SCMD_GRAT },
  { "clan"       , POS_SLEEPING , do_clan     , 1,           0 },
  { "ctell"      , POS_SLEEPING , do_clantell , 1,           0 },
  { "fly"        , POS_STANDING , do_fly      , 0,           0 },
  { "land"       , POS_STANDING , do_land     , 0,           0 },
  { "group"      , POS_SLEEPING , do_group    , 1,           0 },
  { "gtell"      , POS_SLEEPING , do_gsay     , 0,           0 },
  { "pstatus"    , POS_DEAD     , do_pstatus  , 0,           0 },
  { "pksay"      , POS_SLEEPING , do_gen_com  , 0,           SCMD_PKSAY },
  { "help"       , POS_DEAD     , do_help     , 0,           0 },
  { "stand"      , POS_RESTING  , do_stand    , 0,           0 },
  { "sit"        , POS_RESTING  , do_sit      , 0,           0 },
  { "rest"       , POS_RESTING  , do_rest     , 0,           0 },
  { "sleep"      , POS_SLEEPING , do_sleep    , 0,           0 },
  { "wake"       , POS_SLEEPING , do_wake     , 0,           0 },
  { "aid"        , POS_STANDING , do_aid      , 1,           0 },
  { "news"       , POS_SLEEPING , do_gen_ps   , 0,           SCMD_NEWS },
  { "exchange"   , POS_STANDING , do_not_here , 0,           0 },
  { "buy"        , POS_STANDING , do_not_here , 0,           0 },
  { "sell"       , POS_STANDING , do_not_here , 0,           0 },
  { "value"      , POS_STANDING , do_not_here , 0,           0 },
  { "list"       , POS_STANDING , do_not_here , 0,           0 },
  { "weather"    , POS_RESTING  , do_weather  , 0,           0 },
  { "pour"       , POS_STANDING , do_pour     , 0,           SCMD_POUR },
  { "grab"       , POS_RESTING  , do_grab     , 0,           0 },
  { "put"        , POS_RESTING  , do_put      , 0,           0 },
  { "appraise"   , POS_RESTING  , do_appraise , 1,           0 },
  { "hunt"       , POS_STANDING , do_hunt     , 1,           0 },
  { "recall"     , POS_STANDING , do_recall   , 1,           0 },
  { "skills"     , POS_SLEEPING , do_practice , 1,           SCMD_SKILLS },
  { "spells"     , POS_SLEEPING , do_practice , 1,           SCMD_SPELLS },
  { "time"       , POS_DEAD     , do_time     , 0,           0 },
  { "whisper"    , POS_RESTING  , do_whisper  , 0,           0 },
  { "ask"        , POS_RESTING  , do_ask      , 0,           0 },
  { "order"      , POS_RESTING  , do_order    , 1,           0 },
  { "sip"        , POS_RESTING  , do_drink    , 0,           SCMD_SIP },
  { "taste"      , POS_RESTING  , do_eat      , 0,           SCMD_TASTE },
  { "follow"     , POS_RESTING  , do_follow   , 0,           0 },
  { "rent"       , POS_STANDING , do_not_here , 1,           0 },
  { "offer"      , POS_STANDING , do_not_here , 1,           0 },
  { "leave"      , POS_STANDING , do_leave    , 0,           0 },
  { "hold"       , POS_RESTING  , do_grab     , 1,           0 },
  { "flee"       , POS_FIGHTING , do_flee     , 1,           0 },
  { "sneak"      , POS_STANDING , do_sneak    , 1,           0 },
  { "hide"       , POS_RESTING  , do_hide     , 1,           0 },
  { "backstab"   , POS_STANDING , do_backstab , 1,           0 },
  { "circle"     , POS_FIGHTING , do_circle   , 0,           0 },
  { "pick"       , POS_STANDING , do_pick     , 1,           0 },
  { "peek"       , POS_STANDING , do_peek     , 1,           0 },
  { "steal"      , POS_STANDING , do_steal    , 1,           0 },
  { "bash"       , POS_FIGHTING , do_bash     , 1,           0 },
  { "rescue"     , POS_FIGHTING , do_rescue   , 1,           0 },
  { "kick"       , POS_FIGHTING , do_kick     , 1,           0 },
  { "practice"   , POS_RESTING  , do_practice , 1,           0 },
  { "examine"    , POS_SITTING  , do_examine  , 0,           0 },
  { "take"       , POS_RESTING  , do_get      , 0,           0 },
  { "info"       , POS_SLEEPING , do_gen_ps   , 0,           SCMD_INFO },
  { "throw"      , POS_FIGHTING , do_throw    , 1,           0 },
  { "where"      , POS_DEAD     , do_where    , 1,           0 },
  { "levels"     , POS_DEAD     , do_levels   , 0,           0 },
  { "pray"       , POS_SITTING  , do_pray     , 1,           0 },
  { "brief"      , POS_DEAD     , do_gen_tog  , 0,           SCMD_BRIEF },
  { "wizlist"    , POS_DEAD     , do_gen_ps   , 0,           SCMD_WIZLIST },
  { "consider"   , POS_RESTING  , do_consider , 0,           0 },
  { "immlist"    , POS_DEAD     , do_gen_ps   , 0,           SCMD_IMMLIST },
  { "remlist"    , POS_DEAD     , do_gen_ps   , 0,           SCMD_REMLIST },
  { "mount"      , POS_STANDING , do_mount    , 1,           0 },
  { "credits"    , POS_DEAD     , do_gen_ps   , 0,           SCMD_CREDITS },
  { "disarm"     , POS_FIGHTING , do_disarm   , 0,           0 },
  { "display"    , POS_DEAD     , do_display  , 0,           0 },
  { "ungroup"    , POS_DEAD     , do_ungroup  , 0,           0 },
  { "dismount"   , POS_STANDING , do_mount    , 1,           SCMD_DISMOUNT },
  { "murder"     , POS_FIGHTING , do_hit      , 1          , SCMD_MURDER},
  { "title"      , POS_DEAD     , do_title    , 0,           0 },
  { "balance"    , POS_STANDING , do_not_here , 1,           0 },
  { "deposit"    , POS_STANDING , do_not_here , 1,           0 },
  { "withdraw"   , POS_STANDING , do_not_here , 1,           0 },
  { "visible"    , POS_RESTING  , do_visible  , 1,           0 },
  { "protect"    , POS_STANDING , do_protect  , 1,           0 },
  { "split"      , POS_SITTING  , do_split    , 1,           0 },
  { "toggle"     , POS_DEAD     , do_toggle   , 0,           0 },
  { "mail"       , POS_RESTING  , do_not_here , 1,           0 },
  { "post"       , POS_RESTING  , do_not_here , 1,           0 },
  { "check"      , POS_RESTING  , do_not_here , 1,           0 },
  { "receive"    , POS_RESTING  , do_not_here , 1,           0 },
  { "fill"       , POS_STANDING , do_pour     , 0,           SCMD_FILL },
  { "commands"   , POS_DEAD     , do_commands , 0,           SCMD_COMMANDS },
  { "socials"    , POS_DEAD     , do_commands , 0,           SCMD_SOCIALS },
  { "date"       , POS_DEAD     , do_date     , 0,           0 },
  { "color"      , POS_DEAD     , do_color    , 0,           0 },
  { "prompt"     , POS_DEAD     , do_display  , 0,           0 },
  { "cls"        , POS_DEAD     , do_gen_ps   , 0,           SCMD_CLEAR },
  { "clear"      , POS_DEAD     , do_gen_ps   , 0,           SCMD_CLEAR },
  { "version"    , POS_DEAD     , do_gen_ps   , 0,           SCMD_VERSION },
  { "donate"     , POS_RESTING  , do_drop     , 0,           SCMD_DONATE },
  { "diagnose"   , POS_RESTING  , do_diagnose , 0,           0 },
  { "qsay"       , POS_SLEEPING , do_qcomm    , 0,           SCMD_QSAY },
  { "last"       , POS_DEAD     , do_last     , 0,           0 },
  { "whoami"     , POS_DEAD     , do_gen_ps   , 0,           SCMD_WHOAMI },
  { "enter"      , POS_STANDING , do_enter    , 0,           0 },
  { "activate"   , POS_STANDING , do_activate , 0,           0 },
  { "wimpy"      , POS_DEAD     , do_wimpy    , 0,           0 },
  { "berzerk"    , POS_FIGHTING , do_berzerk  , 0,           0 },
  { "cards"      , POS_RESTING  , do_cards    , 0,           0 },
  { "shuffle"    , POS_RESTING  , do_cards    , 0,           0 },
  { "deal"       , POS_RESTING  , do_cards    , 0,           0 },
  { "sort"       , POS_RESTING  , do_cards    , 0,           0 },
  { "redraw"     , POS_SLEEPING , do_redraw   , 1,           0 },
  { "resize"     , POS_SLEEPING , do_resize   , 1,           0 },
  { "newb"       , POS_SLEEPING , do_gen_ps   , 0,           SCMD_NEWBIE },
  { "newbie"     , POS_SLEEPING , do_gen_com  , 0,           SCMD_NEWBIEC},
  { "motd"       , POS_SLEEPING , do_gen_ps   , 0,           SCMD_MOTD },
  { "wargame"    , POS_DEAD     , do_wargame  , 1,           0 },
  { "join"       , POS_STANDING , do_join     , 1,           0 },
  { "shoot"      , POS_STANDING , do_shoot    , 1,           0 },
  { "teams"      , POS_STANDING , do_teams    , 1,           0 },
  { "policy"     , POS_DEAD     , do_gen_ps   , 0,           SCMD_POLICIES },
  { "idea"       , POS_DEAD     , do_gen_write, 0,           SCMD_IDEA },
  { "typo"       , POS_DEAD     , do_gen_write, 0,           SCMD_TYPO },
  { "bug"        , POS_DEAD     , do_gen_write, 0,           SCMD_BUG },
  { "invis"      , POS_DEAD     , do_invis    , LEVEL_DEITY, 0 },
  { "duplicate"  , POS_DEAD     , do_duplicate, LEVEL_DEITY, 0 },
  { "load"       , POS_DEAD     , do_load     , LEVEL_DEITY, 0 },
  { "purge"      , POS_DEAD     , do_purge    , LEVEL_DEITY, 0 },
  { "snoop"      , POS_DEAD     , do_snoop    , LEVEL_DEITY, 0 },
  { "advance"    , POS_DEAD     , do_advance  , LEVEL_DEITY, 0 },
  { "locate"     , POS_DEAD     , do_locate   , LEVEL_DEITY, 0 },
  { "restore"    , POS_DEAD     , do_restore  , LEVEL_DEITY, 0 },
  { "return"     , POS_DEAD     , do_return   , 0,           0 },
  { "switch"     , POS_DEAD     , do_switch   , LEVEL_DEITY, 0 },
  { "users"      , POS_DEAD     , do_users    , LEVEL_DEITY, 0 },
  { "show"       , POS_DEAD     , do_show     , LEVEL_DEITY, 0 },
  { "wizhelp"    , POS_DEAD     , do_commands , LEVEL_DEITY, SCMD_WIZHELP},
  { "teleport"   , POS_DEAD     , do_teleport , LEVEL_DEITY, 0 },
  { "gecho"      , POS_DEAD     , do_gecho    , LEVEL_DEITY, 0 },
  { "force"      , POS_DEAD     , do_force    , LEVEL_DEITY, 0 },
  { "wizlock"    , POS_DEAD     , do_wizlock  , LEVEL_DEITY, 0 },
  { "vnum"       , POS_DEAD     , do_vnum     , LEVEL_DEITY, 0 },
  { "vstat"      , POS_DEAD     , do_vstat    , LEVEL_DEITY, 0 },
  { "uptime"     , POS_DEAD     , do_uptime   , LEVEL_DEITY, 0 },
  { "qecho"      , POS_DEAD     , do_qcomm    , LEVEL_DEITY, SCMD_QECHO },
  { "quest"      , POS_DEAD     , do_quest    , LEVEL_DEITY, 0 },
  { "prename"    , POS_DEAD     , do_wizname  , 1, SCMD_PRENAME },
  { "wizname"    , POS_DEAD     , do_wizname  , 1, SCMD_WIZNAME },
  { "setinnate"  , POS_DEAD     , do_setinnate, LEVEL_DEITY, 0 },
  { "skillclear" , POS_DEAD     , do_skillset , LEVEL_DEITY, SCMD_SKILLCLEAR },
  { "skillset"   , POS_DEAD     , do_skillset , LEVEL_DEITY, 0 },
  { "stskill"    , POS_DEAD     , do_skillstat, LEVEL_DEITY, 0 },
/*  { "unaffect"   , POS_DEAD     , do_wizutil  , LEVEL_DEITY, SCMD_UNAFFECT },*/
  { "unaffect"   , POS_DEAD     , do_unaffect , LEVEL_DEITY, 0 },
  { "echo"       , POS_DEAD     , do_echo     , LEVEL_DEITY, 0 },
  { "send"       , POS_DEAD     , do_send     , LEVEL_DEITY, 0 },
  { "page"       , POS_DEAD     , do_page     , LEVEL_DEITY, 0 },
  { "immstat"    , POS_DEAD     , do_immstat  , LEVEL_DEITY, 0 },
  { "poof"       , POS_DEAD     , do_poofset  , LEVEL_DEITY, 0 },
  { "holylight"  , POS_DEAD     , do_gen_tog  , LEVEL_DEITY, SCMD_HOLYLIGHT},
  { "syslog"     , POS_DEAD     , do_syslog   , LEVEL_DEITY, 0 },
  { "handbook"   , POS_DEAD     , do_gen_ps   , LEVEL_DEITY, SCMD_HANDBOOK},
  { "nohassle"   , POS_DEAD     , do_gen_tog  , LEVEL_DEITY, SCMD_NOHASSLE },
  { "roomflags"  , POS_DEAD     , do_gen_tog  , LEVEL_DEITY, SCMD_ROOMFLAGS},
  { "mute"       , POS_DEAD     , do_wizutil  , LEVEL_DEITY, SCMD_SQUELCH},
  { "notitle"    , POS_DEAD     , do_wizutil  , LEVEL_DEITY, SCMD_NOTITLE},
  { "pardon"     , POS_DEAD     , do_wizutil  , LEVEL_DEITY, SCMD_PARDON},
  { "freeze"     , POS_DEAD     , do_wizutil  , LEVEL_DEITY, SCMD_FREEZE },
  { "thaw"       , POS_DEAD     , do_wizutil  , LEVEL_DEITY, SCMD_THAW },
  { "redit"      , POS_DEAD     , do_redit    , LEVEL_DEITY, 0 },
  { "eedit"      , POS_DEAD     , do_eedit    , LEVEL_DEITY, 0 },
  { "oedit"      , POS_DEAD     , do_oedit    , LEVEL_DEITY, 0 },
  { "medit"      , POS_DEAD     , do_medit    , LEVEL_DEITY, 0 },
  { "ocsrsave"   , POS_DEAD     , do_ocsrsave , LEVEL_DEITY, 0 },
  { "award"      , POS_DEAD     , do_award    , LEVEL_DEITY, 0 },
  { "vlist"      , POS_DEAD     , do_vlist    , LEVEL_DEITY, 0 },
  { "claneq"     , POS_DEAD     , do_claneq   , LEVEL_DEITY, 0 },
  { "questeq"    , POS_DEAD     , do_questeq  , LEVEL_DEITY, 0 },
  { "mpstat"     , POS_DEAD     , do_mpstat   , LEVEL_DEITY, 0 },
  { "log"        , POS_DEAD     , do_log      , LEVEL_DEITY, 0 },
  { "plog"       , POS_DEAD     , do_plog     , LEVEL_DEITY, 0 },
  { "tedit"      , POS_DEAD     , do_tedit    , LEVEL_DEITY, 0 },
  { "view"       , POS_DEAD     , do_view     , LEVEL_DEITY, 0 },
  { "peace"      , POS_DEAD     , do_peace    , LEVEL_DEITY, 0 },
  { "config"     , POS_DEAD     , do_config   , LEVEL_DEITY, 0 },
  { "chanset"    , POS_DEAD     , do_chanset  , LEVEL_DEITY, 0 },
  { "disable"    , POS_DEAD     , do_disable  , LEVEL_DEITY, 0 },
  { "finddoor"   , POS_DEAD     , do_finddoor , LEVEL_DEITY, 0 },
  { "findkey"    , POS_DEAD     , do_findkey  , LEVEL_DEITY, 0 },
  { "whereload"  , POS_DEAD     , do_whereload, LEVEL_DEITY, 0 },
  { "imotd"      , POS_SLEEPING , do_gen_ps   , LEVEL_DEITY, SCMD_IMOTD },

  { "dc"         , POS_DEAD     , do_dc       , LEVEL_DEITY, 0 },
  { "ban"        , POS_DEAD     , do_ban      , LEVEL_DEITY, 0 },
  { "zreset"     , POS_DEAD     , do_zreset   , LEVEL_DEITY, 0 },
  { "zpurge"     , POS_DEAD     , do_zpurge   , LEVEL_DEITY, 0 },
  { "unban"      , POS_DEAD     , do_unban    , LEVEL_DEITY, 0 },
  { "reboot"     , POS_DEAD     , do_reboot   , LEVEL_DEITY, 0 },
  { "shutdow"    , POS_DEAD     , do_shutdown , LEVEL_DEITY, 0 },
  { "shutdown"   , POS_DEAD     , do_shutdown , LEVEL_DEITY, SCMD_SHUTDOWN},
  { "afssave"    , POS_DEAD     , do_temp     , LEVEL_DEITY,  0 },
  { "ident"      , POS_DEAD     , do_gen_tog  , LEVEL_DEITY,  SCMD_IDENT },
  { "identname"  , POS_DEAD     , do_identname, LEVEL_DEITY,  0 },
  { "immset"     , POS_DEAD     , do_immset   , LEVEL_DEITY, 0 },
  { "nolastcmd"  , POS_DEAD     , do_gen_tog  , LEVEL_DEITY,  SCMD_LASTCMD },
  { "pfunctions" , POS_DEAD     , do_pfunctions,LEVEL_DEITY,  0 },
  { "reroll"     , POS_DEAD     , do_wizutil  , LEVEL_DEITY,  SCMD_REROLL },
  { "rpstat"     , POS_DEAD     , do_rpstat   , LEVEL_DEITY,  0 },
  { "sitename"   , POS_DEAD     , do_sitename , LEVEL_DEITY,  0 },
  { "slowns"     , POS_DEAD     , do_gen_tog  , LEVEL_DEITY,  SCMD_SLOWNS },
  { "xlag"       , POS_DEAD     , do_xlag     , LEVEL_DEITY,  0 },

  { "?"          , POS_RESTING  , do_action   ,          0,  0 },
  { "accuse"     , POS_RESTING  , do_action   ,          0,  0 },
  { "ack"        , POS_RESTING  , do_action   ,          0,  0 },
  { "adore"      , POS_RESTING  , do_action   ,          0,  0 },
  { "addict"     , POS_RESTING  , do_action   ,          0,  0 },
  { "admire"     , POS_RESTING  , do_action   ,          1,  0 },
  { "agree"      , POS_RESTING  , do_action   ,          0,  0 },
  { "ah"         , POS_RESTING  , do_action   ,          0,  0 },
  { "airguitar"  , POS_STANDING , do_action   ,          0,  0 },
  { "apologize"  , POS_RESTING  , do_action   ,          0,  0 },
  { "applaud"    , POS_RESTING  , do_action   ,          0,  0 },
  { "bark"       , POS_RESTING  , do_action   ,          0,  0 },
  { "bearhug"    , POS_STANDING , do_action   ,          0,  0 },
  { "beckon"     , POS_RESTING  , do_action   ,          0,  0 },
  { "beer"       , POS_RESTING  , do_action   ,          0,  0 },
  { "beg"        , POS_RESTING  , do_action   ,          0,  0 },
  { "bet"        , POS_RESTING  , do_action   ,          1,  0 },
  { "bite"       , POS_RESTING  , do_action   ,          0,  0 },
  { "bkiss"      , POS_RESTING  , do_action   ,          0,  0 },
  { "bleed"      , POS_RESTING  , do_action   ,          0,  0 },
  { "blink"      , POS_RESTING  , do_action   ,          0,  0 },
  { "blush"      , POS_RESTING  , do_action   ,          0,  0 },
  { "boast"      , POS_RESTING  , do_action   ,          1,  0 },
  { "boggle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "bonk"       , POS_RESTING  , do_action   ,          0,  0 },
  { "bounce"     , POS_RESTING  , do_action   ,          0,  0 },
  { "bow"        , POS_STANDING , do_action   ,          0,  0 },
  { "brb"        , POS_RESTING  , do_action   ,          0,  0 },
  { "breathe"    , POS_RESTING  , do_action   ,          0,  0 },
  { "bully"      , POS_RESTING  , do_action   ,          0,  0 },
  { "bungy"      , POS_RESTING  , do_action   ,          0,  0 },
  { "burp"       , POS_RESTING  , do_action   ,          0,  0 },
  { "bye"        , POS_RESTING  , do_action   ,          0,  0 },
  { "cackle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "camp"       , POS_RESTING  , do_action   ,          0,  0 },
  { "caress"     , POS_RESTING  , do_action   ,          0,  0 },
  { "catnap"     , POS_SLEEPING , do_sleep    ,          0,  1 },
  { "charge"     , POS_RESTING  , do_action   ,          0,  0 },
  { "cheer"      , POS_RESTING  , do_action   ,          0,  0 },
  { "choke"      , POS_RESTING  , do_action   ,          0,  0 },
  { "chortle"    , POS_RESTING  , do_action   ,          0,  0 },
  { "chuckle"    , POS_RESTING  , do_action   ,          0,  0 },
  { "clap"       , POS_RESTING  , do_action   ,          0,  0 },
  { "coke"       , POS_RESTING  , do_action   ,          0,  0 },
  { "comb"       , POS_RESTING  , do_action   ,          0,  0 },
  { "comfort"    , POS_RESTING  , do_action   ,          0,  0 },
  { "congrat"    , POS_RESTING  , do_action   ,          1,  0 },
  { "cough"      , POS_RESTING  , do_action   ,          0,  0 },
  { "cower"      , POS_RESTING  , do_action   ,          0,  0 },
  { "crack"      , POS_RESTING  , do_action   ,          1,  0 },
  { "cringe"     , POS_RESTING  , do_action   ,          0,  0 },
  { "cry"        , POS_RESTING  , do_action   ,          0,  0 },
  { "cuddle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "curse"      , POS_RESTING  , do_action   ,          1,  0 },
  { "curtsey"    , POS_RESTING  , do_action   ,          0,  0 },
  { "dance"      , POS_RESTING  , do_action   ,          0,  0 },
  { "daydream"   , POS_RESTING  , do_action   ,          0,  0 },
  { "disgust"    , POS_RESTING  , do_action   ,          0,  0 },
  { "dismember"  , POS_RESTING  , do_action   ,          0,  0 },
  { "dizzy"      , POS_RESTING  , do_action   ,          0,  0 },
  { "drool"      , POS_RESTING  , do_action   ,          0,  0 },
  { "duck"       , POS_RESTING  , do_action   ,          0,  0 },
  { "embrace"    , POS_RESTING  , do_action   ,          0,  0 },
  { "eyeball"    , POS_RESTING  , do_action   ,          0,  0 },
  { "faint"      , POS_RESTING  , do_action   ,          0,  0 },
  { "fart"       , POS_RESTING  , do_action   ,          0,  0 },
  { "fashion"    , POS_RESTING  , do_action   ,          1,  0 },
  { "flail"      , POS_RESTING  , do_action   ,          0,  0 },
  { "flex"       , POS_RESTING  , do_action   ,          0,  0 },
  { "flip"       , POS_RESTING  , do_flip     ,          0,  0 },
  { "flirt"      , POS_RESTING  , do_action   ,          0,  0 },
  { "flutter"    , POS_RESTING  , do_action   ,          0,  0 },
  { "fondle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "fps"        , POS_RESTING  , do_action   ,          0,  0 },
  { "french"     , POS_RESTING  , do_action   ,          0,  0 },
  { "frown"      , POS_RESTING  , do_action   ,          0,  0 },
  { "fume"       , POS_RESTING  , do_action   ,          0,  0 },
  { "gape"       , POS_RESTING  , do_action   ,          0,  0 },
  { "gasp"       , POS_RESTING  , do_action   ,          0,  0 },
  { "gaze"       , POS_RESTING  , do_action   ,          0,  0 },
  { "ghug"       , POS_RESTING  , do_action   ,          0,  0 },
  { "gibber"     , POS_RESTING  , do_action   ,          0,  0 },
  { "giggle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "girn"       , POS_RESTING  , do_action   ,          0,  0 },
  { "glare"      , POS_RESTING  , do_action   ,          0,  0 },
  { "gloat"      , POS_RESTING  , do_action   ,          0,  0 },
  { "greet"      , POS_RESTING  , do_action   ,          0,  0 },
  { "grimace"    , POS_RESTING  , do_action   ,          0,  0 },
  { "grin"       , POS_RESTING  , do_action   ,          0,  0 },
  { "groan"      , POS_RESTING  , do_action   ,          0,  0 },
  { "grope"      , POS_RESTING  , do_action   ,          0,  0 },
  { "grovel"     , POS_RESTING  , do_action   ,          0,  0 },
  { "growl"      , POS_RESTING  , do_action   ,          0,  0 },
  { "grumble"    , POS_RESTING  , do_action   ,          1,  0 },
  { "guffaw"     , POS_RESTING  , do_action   ,          1,  0 },
  { "gulp"       , POS_RESTING  , do_action   ,          1,  0 },
  { "hand"       , POS_RESTING  , do_action   ,          0,  0 },
  { "headache"   , POS_RESTING  , do_action   ,          1,  0 },
  { "hiccup"     , POS_RESTING  , do_action   ,          0,  0 },
  { "hidenseek"  , POS_RESTING  , do_action   ,          0,  0 },
  { "highfive"   , POS_RESTING  , do_action   ,          1,  0 },
  { "hkiss"      , POS_RESTING  , do_action   ,          0,  0 },
  { "hmm"        , POS_RESTING  , do_action   ,          0,  0 },
  { "holdhand"   , POS_RESTING  , do_action   ,          0,  0 },
  { "hop"        , POS_RESTING  , do_action   ,          0,  0 },
  { "howl"       , POS_RESTING  , do_action   ,          0,  0 },
  { "hug"        , POS_RESTING  , do_action   ,          0,  0 },
  { "hum"        , POS_RESTING  , do_action   ,          1,  0 },
  { "imitate"    , POS_RESTING  , do_action   ,          0,  0 },
  { "innocent"   , POS_RESTING  , do_action   ,          0,  0 },
  { "insult"     , POS_RESTING  , do_insult   ,          0,  0 },
  { "jump"       , POS_RESTING  , do_action   ,          1,  0 },
  { "keel"       , POS_RESTING  , do_action   ,          0,  0 },
  { "kiss"       , POS_RESTING  , do_action   ,          0,  0 },
  { "kvack"      , POS_RESTING  , do_action   ,          0,  0 },
  { "lag"        , POS_RESTING  , do_action   ,          0,  0 },
  { "laugh"      , POS_RESTING  , do_action   ,          0,  0 },
  { "leech"      , POS_RESTING  , do_action   ,          0,  0 },
  { "leer"       , POS_RESTING  , do_action   ,          0,  0 },
  { "lick"       , POS_RESTING  , do_action   ,          0,  0 },
  { "love"       , POS_RESTING  , do_action   ,          0,  0 },
  { "lust"       , POS_RESTING  , do_action   ,          0,  0 },
  { "makeup"     , POS_RESTING  , do_action   ,          1,  0 },
  { "massage"    , POS_RESTING  , do_action   ,          0,  0 },
  { "melt"       , POS_RESTING  , do_action   ,LEVEL_DEITY,  0 },
  { "moan"       , POS_RESTING  , do_action   ,          0,  0 },
  { "moo"        , POS_RESTING  , do_action   ,          0,  0 },
  { "moon"       , POS_RESTING  , do_action   ,          0,  0 },
  { "mumble"     , POS_RESTING  , do_action   ,          1,  0 },
  { "mutter"     , POS_RESTING  , do_action   ,          0,  0 },
  { "nibble"     , POS_RESTING  , do_action   ,          0,  0 },
  { "nod"        , POS_RESTING  , do_action   ,          0,  0 },
  { "nose"       , POS_RESTING  , do_action   ,          0,  0 },
  { "nudge"      , POS_RESTING  , do_action   ,          0,  0 },
  { "nuzzle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "ogle"       , POS_RESTING  , do_action   ,          0,  0 },
  { "oh"         , POS_RESTING  , do_action   ,          0,  0 },
  { "ohno"       , POS_RESTING  , do_action   ,          0,  0 },
  { "pace"       , POS_RESTING  , do_action   ,          1,  0 },
  { "paint"      , POS_RESTING  , do_action   ,          0,  0 },
  { "pale"       , POS_RESTING  , do_action   ,          1,  0 },
  { "pat"        , POS_RESTING  , do_action   ,          0,  0 },
  { "peer"       , POS_RESTING  , do_action   ,          0,  0 },
  { "phew"       , POS_RESTING  , do_action   ,          0,  0 },
  { "pillow"     , POS_RESTING  , do_action   ,          0,  0 },
  { "pinch"      , POS_RESTING  , do_action   ,          0,  0 },
  { "point"      , POS_RESTING  , do_action   ,          0,  0 },
  { "poke"       , POS_RESTING  , do_action   ,          0,  0 },
  { "ponder"     , POS_RESTING  , do_action   ,          0,  0 },
  { "pose"       , POS_RESTING  , do_pose     ,          1,  0 },
  { "pounce"     , POS_RESTING  , do_action   ,          0,  0 },
  { "pout"       , POS_RESTING  , do_action   ,          0,  0 },
  { "praise"     , POS_RESTING  , do_action   ,          0,  0 },
  { "propose"    , POS_RESTING  , do_action   ,          0,  0 },
  { "pucker"     , POS_RESTING  , do_action   ,          0,  0 },
  { "puke"       , POS_RESTING  , do_action   ,          0,  0 },
  { "punch"      , POS_RESTING  , do_action   ,          0,  0 },
  { "purr"       , POS_RESTING  , do_action   ,          0,  0 },
  { "raspberry"  , POS_RESTING  , do_action   ,          1,  0 },
  { "recoil"     , POS_RESTING  , do_action   ,          0,  0 },
  { "reprimand"  , POS_RESTING  , do_action   ,          0,  0 },
  { "roll"       , POS_RESTING  , do_action   ,          0,  0 },
  { "rub"        , POS_RESTING  , do_action   ,          0,  0 },
  { "ruffle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "runaway"    , POS_RESTING  , do_action   ,          0,  0 },
  { "scalp"      , POS_RESTING  , do_action   ,          0,  0 },
  { "scold"      , POS_RESTING  , do_action   ,          0,  0 },
  { "scratch"    , POS_RESTING  , do_action   ,          0,  0 },
  { "scream"     , POS_RESTING  , do_action   ,          0,  0 },
  { "shake"      , POS_RESTING  , do_action   ,          0,  0 },
  { "shark"      , POS_RESTING  , do_action   ,          0,  0 },
  { "shiver"     , POS_RESTING  , do_action   ,          0,  0 },
  { "shrug"      , POS_RESTING  , do_action   ,          0,  0 },
  { "shudder"    , POS_RESTING  , do_action   ,          0,  0 },
  { "sick"       , POS_RESTING  , do_action   ,          1,  0 },
  { "sigh"       , POS_RESTING  , do_action   ,          0,  0 },
  { "sing"       , POS_RESTING  , do_action   ,          0,  0 },
  { "slack"      , POS_RESTING  , do_action   ,          0,  0 },
  { "slam"       , POS_RESTING  , do_action   ,          0,  0 },
  { "slap"       , POS_RESTING  , do_action   ,          0,  0 },
  { "smile"      , POS_RESTING  , do_action   ,          0,  0 },
  { "smirk"      , POS_RESTING  , do_action   ,          0,  0 },
  { "smoke"      , POS_RESTING  , do_action   ,          0,  0 },
  { "smooch"     , POS_RESTING  , do_action   ,          1,  0 },
  { "snap"       , POS_RESTING  , do_action   ,          0,  0 },
  { "snarl"      , POS_RESTING  , do_action   ,          0,  0 },
  { "sneer"      , POS_RESTING  , do_action   ,          1,  0 },
  { "sneeze"     , POS_RESTING  , do_action   ,          0,  0 },
  { "snicker"    , POS_RESTING  , do_action   ,          0,  0 },
  { "sniff"      , POS_RESTING  , do_action   ,          0,  0 },
  { "snore"      , POS_SLEEPING , do_action   ,          0,  0 },
  { "snort"      , POS_RESTING  , do_action   ,          0,  0 },
  { "snowball"   , POS_RESTING  , do_action   ,LEVEL_DEITY,  0 },
  { "snuggle"    , POS_RESTING  , do_action   ,          0,  0 },
  { "sob"        , POS_RESTING  , do_action   ,          0,  0 },
  { "spank"      , POS_RESTING  , do_action   ,          0,  0 },
  { "spit"       , POS_RESTING  , do_action   ,          0,  0 },
  { "splat"      , POS_RESTING  , do_action   ,          0,  0 },
  { "squeak"     , POS_RESTING  , do_action   ,          0,  0 },
  { "squeeze"    , POS_RESTING  , do_action   ,          0,  0 },
  { "squirm"     , POS_RESTING  , do_action   ,          1,  0 },
  { "stare"      , POS_RESTING  , do_action   ,          0,  0 },
  { "steam"      , POS_RESTING  , do_action   ,          0,  0 },
  { "stretch"    , POS_RESTING  , do_action   ,          0,  0 },
  { "strip"      , POS_RESTING  , do_action   ,          0,  0 },
  { "stroke"     , POS_RESTING  , do_action   ,          0,  0 },
  { "strut"      , POS_RESTING  , do_action   ,          0,  0 },
  { "sulk"       , POS_RESTING  , do_action   ,          0,  0 },
  { "sway"       , POS_RESTING  , do_action   ,          1,  0 },
  { "swear"      , POS_RESTING  , do_action   ,          0,  0 },
  { "tackle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "tango"      , POS_RESTING  , do_action   ,          0,  0 },
  { "tantrum"    , POS_RESTING  , do_action   ,          0,  0 },
  { "tap"        , POS_RESTING  , do_action   ,          1,  0 },
  { "taunt"      , POS_RESTING  , do_action   ,          0,  0 },
  { "thank"      , POS_RESTING  , do_action   ,          0,  0 },
  { "think"      , POS_RESTING  , do_action   ,          0,  0 },
  { "threaten"   , POS_RESTING  , do_action   ,          0,  0 },
  { "throttle"   , POS_RESTING  , do_action   ,          0,  0 },
  { "tickle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "tilt"       , POS_RESTING  , do_action   ,          0,  0 },
  { "tongue"     , POS_RESTING  , do_action   ,          1,  0 },
  { "tug"        , POS_RESTING  , do_action   ,          0,  0 },
  { "tweak"      , POS_RESTING  , do_action   ,          0,  0 },
  { "twiddle"    , POS_RESTING  , do_action   ,          0,  0 },
  { "twirl"      , POS_RESTING  , do_action   ,          0,  0 },
  { "twitch"     , POS_RESTING  , do_action   ,          0,  0 },
  { "type"       , POS_RESTING  , do_action   ,          0,  0 },
  { "wave"       , POS_RESTING  , do_action   ,          0,  0 },
  { "wedgie"     , POS_RESTING  , do_action   ,          0,  0 },
  { "wetfish"    , POS_RESTING  , do_action   ,          1,  0 },
  { "whine"      , POS_RESTING  , do_action   ,          0,  0 },
  { "whinge"     , POS_RESTING  , do_action   ,          0,  0 },
  { "whistle"    , POS_RESTING  , do_action   ,          0,  0 },
  { "wiggle"     , POS_RESTING  , do_action   ,          0,  0 },
  { "wince"      , POS_RESTING  , do_action   ,          0,  0 },
  { "wink"       , POS_RESTING  , do_action   ,          0,  0 },
  { "wobble"     , POS_RESTING  , do_action   ,          0,  0 },
  { "worshi"     , POS_RESTING  , do_action   ,          0,  0 },
  { "worship"    , POS_RESTING  , do_worship  ,          1,  SCMD_WORSHIP},
  { "wrestle"    , POS_RESTING  , do_action   ,          1,  0 },
  { "yawn"       , POS_RESTING  , do_action   ,          0,  0 },
  { "yodel"      , POS_RESTING  , do_action   ,          0,  0 },

  { "\n", 0, 0, 0, 0 }	/* this must be last */
};


/* CEND: search for me when you're looking for the end of the cmd list! :) */

char	*fill[] =
{
   "in",
   "from",
   "with",
   "the",
   "on",
   "at",
   "to",
   "\n"
};

char *reserved[] =
{
  "self",
  "me",
  "all",
  "innate",
  "innates",
  "room",
  "someone",
  "something",
  "zone",
  "afk",
  "\n"
};


int	search_block(char *arg, char **list, bool exact)
{
  register int	i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return(i);
  } else {
    if (!l)
      l = 1; /* Avoid "" to match the first available string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return(i);
  }

  return(-1);
}


/* Speedwalk processor by  -Petrus  */

void   process_speedwalk(struct char_data *ch, char *comline)
{
  char tmp[128], *ptr, *com;
  int len = 0, step = 1;

  if (!ch->desc || PRF_FLAGGED(ch, PRF_NOSPDWLK) || PRF_FLAGGED(ch, PRF_VERBATIM))
    return;

  ptr = tmp;
  com = comline;
  while (*com != '\0') {
    if (len > 38)
      return;
    else if (*com == 'n' || *com == 'e' || *com == 'w' ||
	     *com == 's' || *com == 'u' || *com == 'd') {
      while(step--) {
	*ptr = *com;
	*(++ptr) = ';';
	ptr++;
	len++;
      }
      step = 1;
    } else if (*com > '0' && *com <= '9')
      step = *com - '0';
    else if (*com != ' ')
      return;

    com++;
  }
  *ptr = '\0';

  if (len < 39)
    strcpy(comline, tmp);
}


/* Numerical command interpreter  -Petrus */

int    process_numeric_command(struct char_data *ch, char **comline)
{
    char *ptr;

    if (!ch->desc || PRF_FLAGGED(ch, PRF_VERBATIM))
	return FALSE;

    *(ch->desc->num_input) = '\0';

    for (ptr = *comline; *ptr == ' '; ptr++);   /* first non-blank */

    if (*ptr != '#')
	return FALSE;

    if (*(ptr + 1) < '1' || *(ptr + 1) > '9')
	return FALSE;

    *comline = (ptr + 2);

    if (*(ptr + 1) > '1') {
	(*(ptr + 1))--;
	if (strlen(ptr) > 79)
	    send_to_char("***Numerical command too long.\r\n", ch);
	else {
	    strcpy(ch->desc->num_input, ptr);
	    return TRUE;
	}
    }
    return FALSE;
}

/* Multi-command interpreter  -Petrus */

void    process_multi_commands(struct char_data *ch, char *comline)
{
    char *ptr;

    if (!ch->desc || PRF_FLAGGED(ch, PRF_VERBATIM))
	return;

    *(ch->desc->mult_input) = '\0';

    for (ptr = comline; *ptr != '\0'; ptr++) {
	if (*ptr == ';') {
	    *ptr = '\0';
	    while (*(++ptr) == ';');
	    if (ptr != '\0') {
		if (strlen(ptr) > 79)
		    send_to_char("***Multi-command-line too long or too complex.\r\n", ch);
		else
		    strcpy(ch->desc->mult_input, ptr);
	    }
	    break;
	}
    }
}


/* Alias-interpreter by - Petrus  */

void    alias_interpreter(struct char_data *ch, char *comline)
{
  char newline[MAX_INPUT_LENGTH];
  char *point, *newpt, *alspt;
  int len, newlen = 0, begin, is_replaced;
  struct alias_list *al;

  if (!ch->specials.aliases)    /* No aliases - no need to go on */
    return;

  for (begin = 0 ; (*(comline + begin ) == ' ' ) ; begin++);
  point = comline + begin;      /* point = first char in line */

  is_replaced = 0;
  for (len = 0; (*(point + len) != ' ') &&
       (*(point + len) != '\0'); len++);   /* Get len of cmd */

  for (al = ch->specials.aliases; al;al = al->next) {

    if (!strncmp(al->alias, point, len) &&
	strlen(al->alias) == len)
      {
	/* Alias substitution -Petrus */
	alspt = al->replace;
	newpt = newline;
	newlen = 0;
	is_replaced = 1;

	while(*alspt != '\0') {
	  if (*alspt != '%') {
	    *newpt = *alspt;
	    newlen++;
	    newpt++;
	  } else if (*(point + len)) {
	    strcpy(newpt, (point + len + 1));
	    newlen += strlen((point + len + 1));
	    while (*newpt != '\0')
	      newpt++;;
	    is_replaced = 2;
	  }
	  alspt++;
	  if (newlen > MAX_INPUT_LENGTH - 1)
	    break;
	}
	*newpt = '\0';
	break;
      }
  }

  if (is_replaced) {
    if (is_replaced == 1)
      newlen += strlen(point + len);

    if (newlen > MAX_INPUT_LENGTH - 1) {
      send_to_char("***Command line too long - alias not replaced.\r\n", ch);
      return;
    } else if (is_replaced == 1) {
      strcat(newline, point + len);
      strcpy(comline, newline);
    } else if (is_replaced == 2)
      strcpy(comline, newline);
  }
}

/* New huh??? list - Petrus*/
static char     *huh_message[]  =  {
    "huh?!?\r\n",
    "que?!?\r\n",
    "va?!?\r\n",
    "wie bitte?!?\r\n",
    "again?!?\r\n",
    "?!? *sigh*\r\n"
    };


void	command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  extern int no_specials;
  char *line;
  bool is_num;
  struct char_data *original = 0, *switched = 0;

  REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  skip_spaces(&argument);

  alias_interpreter(ch, argument);

  process_speedwalk(ch, argument);
  is_num = process_numeric_command(ch, &argument);
  process_multi_commands(ch, argument);

  skip_spaces(&argument);
  if (!*argument)
    return;

  /* Special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha(*argument)) {
    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument+1;
  } else
    line = any_one_arg(argument, arg);

  /* Check if there is a switched immortal doing this: the flags
   * determines the godlevel that issue switched godcmds from their
   * immortal bodies */
  if (ch->desc && ch->desc->original &&
      (IS_SET(GODLEVEL(ch->desc->original), IMM_ALL) ||
       IS_SET(GODLEVEL(ch->desc->original), IMM_BASIC)) ) {
    original = ch->desc->original;
  }


  /* otherwise, find the command */
  for (length = strlen(arg), cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strncmp(cmd_info[cmd].command, arg, length))
      if ((GET_LEVEL(ch) >= cmd_info[cmd].minimum_level) ||
          (original && (GET_LEVEL(original) >= cmd_info[cmd].minimum_level)))
	break;

  if (*cmd_info[cmd].command == '\n') {
    send_to_char(huh_message[number(0,5)], ch);
    if (is_num && ch->desc)
      *(ch->desc->num_input) = '\0';
  } else if (check_disabled(&cmd_info[cmd]))    /* is it disabled? */
    send_to_char("This command has been temporarily disabled.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LEVEL_IMPL)
    send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
  else if (cmd_info[cmd].command_pointer == NULL)
    send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC(ch) && cmd_info[cmd].minimum_level >= LEVEL_DEITY &&
           cmd_info[cmd].subcmd != SCMD_MOBPROG && !(original))
    send_to_char("You can't use immortal commands while switched.\r\n", ch);
  else if (GET_POS(ch) < cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
      break;
    case POS_STUNNED:
      send_to_char("All you can do right now is think about the stars!\r\n", ch);
      break;
    case POS_SLEEPING:
      send_to_char("In your dreams, or what?\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("Maybe you should get on your feet first?\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
    }
  else if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_BERZERK) &&
           (GET_POS(ch) == POS_FIGHTING) &&
           (cmd_info[cmd].minimum_position > POS_SLEEPING || CMD_IS("wimpy")) &&
           !(CMD_IS("say") || CMD_IS("'") || CMD_IS("look") ||
             CMD_IS("yell") || CMD_IS("whisper") || CMD_IS("ask") ||
             CMD_IS("consider") || CMD_IS("diagnose") || CMD_IS("examine"))) {
    send_to_char("No way!  you'll fight to death!\r\n", ch);
  } else if (GET_MOVE(ch) < 1 &&
	     cmd_info[cmd].minimum_position > POS_SITTING) {
    send_to_char("You are too exhausted.\r\n", ch);
  } else {
    /* Reswitch to original character temporarily: this has to be
     * blocked for switch for obvious bad reasons....
     * Stat has been added as is an imm command that has restricted
       player functions and needs to parse through here to work for
       switched imms as intended */
    if (original && (((cmd_info[cmd].minimum_level >= LEVEL_DEITY) &&
        !CMD_IS("switch")) || CMD_IS("stats"))) {
      switched = ch;
      ch->desc->character = original;
      ch->desc->original = NULL;
      ch->desc->character->desc = ch->desc;
      ch->desc = NULL;
      ch = original;
    }
    if (save_last_command) {
      /* New crash test system -Petrus*/
      sprintf(buf, "%s: \"%s\" - %s", GET_NAME(ch), argument,
	      world[IN_ROOM(ch)]->name);
      write_last_command(buf);
    }
    if (PLR_FLAGGED(ch, PLR_LOG)) {
      sprintf(buf, "(LOG) %s: \"%s\" - %s", GET_NAME(ch), argument,
	      world[IN_ROOM(ch)]->name);
      log(buf);
    }

    cmds_executed++;
    if (!IS_NPC(ch))
      plr_cmds_executed++;

    /* NEW Immortal Command Imterpreter - Helm */
    if (cmd_info[cmd].minimum_level == LEVEL_DEITY) {
      if (!immortal_interpreter(ch, cmd_info[cmd].command))
	return;
    }

    if (no_specials || !special(ch, cmd, line))
      ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));

    /* Back to the switched character again as long as it is still valid */
    if (switched) {
      if (validate_char(switched)) {
        ch->desc->character = switched;
        ch->desc->original = ch;
        ch->desc->character->desc = ch->desc;
        ch->desc = NULL;
        ch = switched;
      }
    }
  }
}


/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
  char *read, *write;

  /* If the string has no dollar signs, return immediately */
  if ((write = strchr(string, '$')) == NULL)
    return string;

  /* Start from the location of the first dollar sign */
  read = write;


  while (*read)   /* Until we reach the end of the string... */
    if ((*(write++) = *(read++)) == '$') /* copy one char */
      if (*read == '$')
	read++; /* skip if we saw 2 $'s in a row */

  *write = '\0';

  return string;
}


int	is_number(char *str)
{
   int	look_at;

   if (!str || *str == '\0')
      return(0);

   for (look_at = 0; *(str + look_at) != '\0'; look_at++)
      if ((*(str + look_at) < '0') || (*(str + look_at) > '9'))
	 return(0);
   return(1);
}


int	fill_word(char *argument)
{
   return (search_block(argument, fill, TRUE) >= 0);
}

int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}

/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
    char *begin = first_arg;

    do {
	skip_spaces(&argument);

	first_arg = begin;
	while (*argument && !isspace(*argument)) {
	    *(first_arg++) = LOWER(*argument);
	    argument++;
	}

	*first_arg = '\0';
    } while (fill_word(begin));

    return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
    skip_spaces(&argument);

    while (*argument && !isspace(*argument)) {
	*(first_arg++) = LOWER(*argument);
	argument++;
    }

    *first_arg = '\0';

    return argument;
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg);	/* :-) */
}

/* determine if a given string is an abbreviation of another */
int	is_abbrev(char *arg1, char *arg2)
{
   if (!arg1 || !arg2 || !*arg1)
      return(0);

   for (; *arg1; arg1++, arg2++)
      if (LOWER(*arg1) != LOWER(*arg2))
	 return(0);

   return(1);
}



/* return first 'word' plus trailing substring of input string */
void	half_chop(char *string, char *arg1, char *arg2)
{
   for (; isspace(*string); string++);

   for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

   *arg1 = '\0';

   for (; isspace(*string); string++);

   for (; (*arg2 = *string); string++, arg2++);
}


/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
  int cmd;

  for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int	special(struct char_data *ch, int cmd, char *arg)
{
   register struct obj_data *i;
   register struct char_data *k;
   int	j;

   /* special in room? */
   if (world[ch->in_room]->funct)
      if ((*world[ch->in_room]->funct)(ch, NULL, NULL, cmd, arg))
	 return(1);

   /* special in mobile present? */
   for (k = world[ch->in_room]->people; k; k = k->next_in_room)
      if (IS_MOB(k))
	 if (mob_index[k->nr].func)
	    if ((*mob_index[k->nr].func)(ch, k, NULL, cmd, arg))
	       return(1);

   /* special in object present? */
   for (i = world[ch->in_room]->contents; i; i = i->next_content)
      if (i->item_number >= 0)
	 if (obj_index[i->item_number].func)
	    if ((*obj_index[i->item_number].func)(ch, NULL, i, cmd, arg))
	       return(1);

   /* special in equipment list? */
   for (j = 0; j <= (MAX_WEAR - 1); j++)
      if (ch->equipment[j] && ch->equipment[j]->item_number >= 0)
	 if (obj_index[ch->equipment[j]->item_number].func)
	    if ((*obj_index[ch->equipment[j]->item_number].func)
	        (ch, NULL, ch->equipment[j], cmd, arg))
	       return(1);

   /* special in inventory? */
   for (i = ch->carrying; i; i = i->next_content)
      if (i->item_number >= 0)
	 if (obj_index[i->item_number].func)
	    if ((*obj_index[i->item_number].func)(ch, NULL, i, cmd, arg))
	       return(1);

   return(0);
}


int mob_do_action(int type, struct char_data *ch, struct char_data *victim)
{
    if (!ch || !victim)
	return 0;

    if (!GET_SKILL(ch, type))
	return 0;

    switch (type) {
    case SKILL_KICK:
	do_kick(ch, "", 0, 0);
	return 1;
	break;
    case SKILL_BASH:
	if (ch->equipment[WEAR_SHIELD])
	    do_bash(ch, "", 0, 0);
	return 1;
	break;
    case SKILL_DISARM:
	if (victim->equipment[WIELD])
	    do_disarm(ch, "", 0, 0);
	return 1;
	break;
    case SKILL_CIRCLE_AROUND:
      if (ch->equipment[WIELD] && (ch->equipment[WIELD]->obj_flags.value[3] == 11)) {
	do_circle(ch, "", 0, 0);
	return 1;
      }
    default:
	return 0;
    }

    return 0;
}


/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int	find_name(char *name)
{
   int	i;

   if(!name || !*name)
       return -1;

   for (i = 0; i <= top_of_p_table; i++) {
      if (!str_cmp((player_table + i)->name, name))
	 return(i);
   }

   return(-1);
}


int	_parse_name(char *arg, char *name)
{
   int	i;

   /* skip whitespaces */
   for (; isspace(*arg); arg++);

   for (i = 0; (*name = *arg); arg++, i++, name++)
      if (!isalpha(*arg) || i > 15)
	 return(1);

   if (!i)
      return(1);

   return(0);
}


int get_player_lvl(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
	  if (!str_cmp(player_table[i].name, name))
	    return player_table[i].level;

  return 0;
}

char *get_name_idnum(long idnum)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
	  if (player_table[i].idnum == idnum)
	    return player_table[i].name;

  return "(null)";
}

sbyte get_level_idnum(long idnum)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
	  if (player_table[i].idnum == idnum)
	    return player_table[i].level;

  return -1;
}

/************************* N A N N Y *****************************/


void  count_worshippers(struct char_data *ch)
{
    int i;

    WORSHIPPERS(ch) = 0;
    POWER(ch)       = 0;

    for (i = 0; i <= top_of_p_table; i++) {
	if (GET_IDNUM(ch) == player_table[i].worships) {
	    WORSHIPPERS(ch)++;
	    POWER(ch) += player_table[i].level;
	}
    }
}


char * get_deity_name(struct char_data *ch)
{
    int i;

    for (i = 0; i <= top_of_p_table; i++)
	if (player_table[i].level >= LEVEL_WORSHIP &&
	    player_table[i].worships == WORSHIPS(ch))
	    return (player_table[i].name);

    return ("none");
}


void display_displays(struct descriptor_data *d)
{
  SEND_TO_Q("\nSelect Screen Mode:\r\n  [a] Normal Terminal\r\n  [b] VT100 Compatible\r\n  [c] VT & ANSI Color\r\n  [d] IBM/PC color&character set\r\n  [?] Help!\r\n", d);
  SEND_TO_Q("Screen Mode: ", d);
}


void display_races(struct descriptor_data *d)
{
  SEND_TO_Q("\nSelect a race:\r\n  [a] Human       [b] Troll       [c] Halfling\r\n  [d] Dwarf       [e] Gnome       [f] Elf\r\n  [g] Half-elf    [h] Fairy       [i] Minotaur\r\n  [j] Ratman      [k] Drow        [l] Lizardman\r\n  [m] Draconian", d);
  SEND_TO_Q("\r\nEnter capital letter to get info about race\r\n", d);
  SEND_TO_Q("Race: ", d);
}


void display_classes(struct descriptor_data *d)
{
  int i;

  SEND_TO_Q("\r\nSelect Class:\r\n", d);

  i = GET_RACE(d->character);

  if (IS_SET(allowed_classes[i],(1 << (CLASS_MAGIC_USER - 1))))
    SEND_TO_Q("  [a] Magic-user\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_CLERIC - 1))))
    SEND_TO_Q("  [b] Cleric\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_THIEF - 1))))
    SEND_TO_Q("  [c] Thief\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_WARRIOR - 1))))
    SEND_TO_Q("  [d] Warrior\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_PSIONICIST - 1))))
    SEND_TO_Q("  [e] Psionicist\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_MONK - 1))))
    SEND_TO_Q("  [f] Monk\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_BARD - 1))))
    SEND_TO_Q("  [g] Bard\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_KNIGHT - 1))))
    SEND_TO_Q("  [h] Knight\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_WIZARD - 1))))
    SEND_TO_Q("  [i] Wizard\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_DRUID - 1))))
    SEND_TO_Q("  [j] Druid\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_ASSASSIN - 1))))
    SEND_TO_Q("  [k] Assassin\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_RANGER - 1))))
    SEND_TO_Q("  [l] Ranger\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_ILLUSIONIST - 1))))
    SEND_TO_Q("  [m] Illusionist\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_PALADIN - 1))))
    SEND_TO_Q("  [n] Paladin\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_MARINER - 1))))
    SEND_TO_Q("  [o] Mariner\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_CAVALIER - 1))))
    SEND_TO_Q("  [p] Cavalier\r\n", d);
  if (IS_SET(allowed_classes[i],(1 << (CLASS_NINJA - 1))))
    SEND_TO_Q("  [s] Ninja\r\n", d);

  if (IS_SET(allowed_classes[i],(1 << (CLASS_2MULTI - 1)))) {
    SEND_TO_Q("  [u] Warrior/Thief\r\n", d);
    SEND_TO_Q("  [v] Warrior/Cleric\r\n", d);
    SEND_TO_Q("  [w] Warrior/Magic-user\r\n", d);
    SEND_TO_Q("  [x] Thief/Cleric\r\n", d);
    SEND_TO_Q("  [y] Thief/Magic-user\r\n", d);
    SEND_TO_Q("  [z] Cleric/Magic-user\r\n", d);
  }
  if (IS_SET(allowed_classes[i],(1 << (CLASS_3MULTI - 1)))) {
    SEND_TO_Q("  [1] Warrior/Thief/Magic-user\r\n", d);
    SEND_TO_Q("  [2] Warrior/Cleric/Magic-user\r\n", d);
  }

  SEND_TO_Q("  [-] Reselect Race\r\n", d);

  SEND_TO_Q("\r\nEnter capital letter to get info about class\r\n", d);
  SEND_TO_Q("Class: ", d);
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3

int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;
  extern struct descriptor_data *descriptor_list;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
      if (!target) {
	target = k->original;
	mode = UNSWITCH;
      }
      if (k->character)
	k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && (GET_IDNUM(k->character) == id)) {
      if (!target && STATE(k) == CON_PLYNG) {
	SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
	target = k->character;
	mode = USURP;
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
    }
  }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (IS_NPC(ch))
      continue;
    if (GET_IDNUM(ch) != id)
      continue;

    /* ignore chars with descriptors (already handled by above step) */
    if (ch->desc)
      continue;

    /* don't extract the target char we've found one already */
    if (ch == target)
      continue;

    /* we don't already have a target and found a candidate for switching */
    if (!target) {
      target = ch;
      mode = RECON;
      continue;
    }

    /* we've found a duplicate - blow him away, dumping his eq in limbo. */
    if (ch->in_room != NOWHERE)
      char_from_room(ch);
    char_to_room(ch, 1);
    extract_char(ch);
  }

  /* no target for swicthing into was found - allow login to continue */
  if (!target)
    return 0;

  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->specials.timer = 0;
  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
  STATE(d) = CON_PLYNG;

  switch (mode) {
  case RECON:
    SEND_TO_Q("Reconnecting.\r\n", d);
    act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    d->character->player.time.logon = time(0);
    break;
  case USURP:
    SEND_TO_Q("You take over your own body, already in use!\r\n", d);
    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	"$n's body has been taken over by a new spirit!",
	TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
	    GET_NAME(d->character));
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    d->character->player.time.logon = time(0);
    break;
  case UNSWITCH:
    SEND_TO_Q("Reconnecting to unswitched char.", d);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    d->character->player.time.logon = time(0);
    break;
  }

  return 1;
}



/* deal with newcomers and other non-playing sockets */
void	nanny(struct descriptor_data *d, char *arg)
{
  char	buf[100];
  int	player_i, load_result, i, helpmode = FALSE;
  char	tmp_name[20];
  struct char_file_u tmp_store;
  struct descriptor_data *k, *next;
  extern struct descriptor_data *descriptor_list;
  extern sh_int r_mortal_start_room;
  extern sh_int r_newbie_start_room;
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;
  extern int r_newbie_clan;
  sh_int load_room;
  FILE *fp;

  ACMD(do_look);

  switch (STATE(d)) {
  case CON_NME:	 /* After input of name */
    if (!d->character) {  /* Prepare for new character */
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      d->character->desc = d;
    }
    skip_spaces(&arg);

    if (!*arg)
      close_socket(d);
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) > MAX_NAME_LENGTH || strlen(tmp_name) < 2 || fill_word(strcpy(buf,tmp_name)) || reserved_word(buf))
	{
	  SEND_TO_Q("Invalid name, please try another.\r\n", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}

      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) { /*In database*/
	d->pos = player_i;
	store_to_char(&tmp_store, d->character);
	stringdata_load(d->character);
	d->character->nr = player_i;

	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
	  /* deleted char */
	  free_char(d->character);
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  d->character->desc = d;
	  d->character->nr = player_i;

	  /* clear the old eq file */
	  if (!get_filename(tmp_name, buf, OBJECT_FILE)) {
	       SEND_TO_Q("Player file error!!!\r\n", d);
	       STATE(d) = CON_CLOSE;
	       return;
	  }
	  if (!(fp = fopen(buf, "wb"))) {
	       SEND_TO_Q("Player file error!!!\r\n", d);
	       STATE(d) = CON_CLOSE;
	       return;;
	  }

	  fclose(fp);

	  for (i = 1;*(tmp_name + i) != '\0'; i++) /* lower name*/
	    *(tmp_name + i) = LOWER(*(tmp_name + i));

	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NMECNF;
	} else {  /* Normal char */

	  /* undo it just in case they are set */
	  REMOVE_BIT(PLR_FLAGS(d->character),
		     PLR_WRITING | PLR_MAILING | PLR_CRYO | PLR_ARENA);

	  SEND_TO_Q("Password: ", d);
	  echo_off(d->descriptor);

	  STATE(d) = CON_PWDNRM;
	}
      } else {  /* player unknown - new char */
	if (!Valid_Name(tmp_name)) {
	  SEND_TO_Q("Invalid name, please try another.\r\n", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}

	CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	strcpy(d->character->player.name, CAP(tmp_name));

	sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	SEND_TO_Q(buf, d);
	STATE(d) = CON_NMECNF;
      }
    }
    break;
  case CON_NMECNF:  /* after conf. of new name */
    skip_spaces(&arg);

    if (*arg == 'y' || *arg == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
	sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_IMMORT, TRUE);
	SEND_TO_Q("Sorry, new characters not allowed from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (restrict) {
	SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
	sprintf(buf, "Request for new char %s denied from %s (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_IMMORT, TRUE);
	STATE(d) = CON_CLOSE;
	return;
      }
      SEND_TO_Q("New character.\r\n", d);
      sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      echo_off(d->descriptor);
      STATE(d) = CON_PWDGET;
    } else if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Ok, what IS it, then? ", d);
      free(GET_NAME(d->character));
      d->character->player.name = 0;
      STATE(d) = CON_NME;
    } else {
      SEND_TO_Q("Please type Yes or No: ", d);
    }
    break;
  case CON_PWDNRM:  /* get pwd for known player */
    echo_on(d->descriptor);  /* turn echo back on */
    skip_spaces(&arg); /* skip whitespaces */


    if (!*arg)
      close_socket(d);
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
	sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_LEVEL(d->character)), TRUE);
	d->character->specials2.bad_pws++;
	save_char(d->character, IN_VROOM(d->character), 0);
	if (++(d->bad_pws) >= 3) { /* 3 strikes and you're out. */
	  SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
	  STATE(d) = CON_CLOSE;
	} else {
	  SEND_TO_Q("Wrong password.\r\nPassword: ", d);
	  echo_off(d->descriptor);
	}
	return;
      }

      load_result = d->character->specials2.bad_pws;
      d->character->specials2.bad_pws = 0;
      save_char(d->character, IN_VROOM(d->character), 0);

      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_IMMORT, TRUE);
	return;
      }

      if (GET_LEVEL(d->character) < restrict) {
	SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_IMMORT, TRUE);
	return;
      }

      if (perform_dupe_check(d))
	return;

      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      mudlog(buf, BRF, MAX(LEVEL_IMMORT, GET_LEVEL(d->character)), TRUE);

      if (GET_LEVEL(d->character) >= LEVEL_WORSHIP)
	count_worshippers(d->character);

      if (d->character && PRF_FLAGGED(d->character, PRF_DISPVT))
	SEND_TO_Q( VTCLS, d);
      if (GET_LEVEL(d->character) >= LEVEL_DEITY)
	SEND_TO_Q(imotd, d);
      else
	SEND_TO_Q(motd, d);
      if (load_result) {
	sprintf(buf, "\r\n\r\n\007\007\007"
		"§r%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.§N\r\n",
		load_result,
		(load_result > 1) ? "S" : "");
	SEND_TO_Q(buf, d);
      }

      SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_PWDGET:		/* get pwd for new player	*/
    skip_spaces(&arg); /* skip whitespaces */

    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }

    strncpy(GET_PASSWD(d->character), CRYPT(arg, d->character->player.name), MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    SEND_TO_Q("\r\nPlease retype password: ", d);
    STATE(d) = CON_PWDCNF;
    break;

  case CON_PWDCNF:		/* get confirmation of new pwd	*/
    skip_spaces(&arg);  /* skip whitespaces */

    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      STATE(d) = CON_PWDGET;
      return;
    }

    /* turn echo back on */
    echo_on(d->descriptor);

    display_displays(d);
    STATE(d) = CON_DISPL;
    break;

  case CON_DISPL:		/* Confirm display type for player */
    skip_spaces(&arg);  /* skip whitespaces */

    if (*arg == '?') {
      do_help(d->character, "screenmodes", 0, 0);
      display_displays(d);
      return;
    }

    *arg = LOWER(*arg);

    if (*arg < 'a' || *arg > 'd') {
      SEND_TO_Q("\r\nWrong option!\r\nScreen Mode: ", d);
      return;
    }

    switch (*arg) {
    case 'a':
      SET_BIT(PRF_FLAGS(d->character), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE | PRF_DISPVIC); break;
    case 'b':
      SET_BIT(PRF_FLAGS(d->character), PRF_DISPVT); break;
    case 'c':
      SET_BIT(PRF_FLAGS(d->character), PRF_DISPVT | PRF_DISPANSI | PRF_COLOR_1 | PRF_COLOR_2);
      break;
    case 'd':
      SET_BIT(PRF_FLAGS(d->character), PRF_DISPVT | PRF_DISPANSI | PRF_IBM_PC | PRF_COLOR_1 | PRF_COLOR_2);
      break;
    default:
      break;
    }

    if (d->character && PRF_FLAGGED(d->character, PRF_DISPVT))
      SEND_TO_Q( VTCLS, d);

    SEND_TO_Q("\r\nWhat is your sex (M/F)? ", d);
    STATE(d) = CON_QSEX;
    break;

  case CON_QSEX:  /* query sex of new user */
    skip_spaces(&arg);  /* skip whitespaces */
    switch (*arg) {
    case 'm':
    case 'M':
      /* sex MALE */
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      /* sex FEMALE */
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("That's not a sex..\r\n", d);
      SEND_TO_Q("What IS your sex? :", d);
      return;
      break;
    }

    if (d->character && PRF_FLAGGED(d->character, PRF_DISPVT))
      SEND_TO_Q( VTCLS, d);
    display_races(d);
    STATE(d) = CON_QRACE;
    break;

  case CON_QRACE:

    skip_spaces(&arg);  /* skip whitespaces */

    if (*arg >= 'A' && *arg <= 'O')
      helpmode = TRUE;

    *arg = LOWER(*arg);

    if (*arg < 'a' || *arg > 'm') {
      SEND_TO_Q("\r\nWrong option!\r\nRace:", d);
      return;
    }

    switch (*arg) {
    case 'a': GET_RACE(d->character) = RACE_HUMAN; break;
    case 'b': GET_RACE(d->character) = RACE_TROLL; break;
    case 'c': GET_RACE(d->character) = RACE_HALFLING; break;
    case 'd': GET_RACE(d->character) = RACE_DWARF; break;
    case 'e': GET_RACE(d->character) = RACE_GNOME; break;
    case 'f': GET_RACE(d->character) = RACE_ELF; break;
    case 'g': GET_RACE(d->character) = RACE_HALFELF; break;
    case 'h': GET_RACE(d->character) = RACE_FAIRY; break;
    case 'i': GET_RACE(d->character) = RACE_MINOTAUR; break;
    case 'j': GET_RACE(d->character) = RACE_RATMAN; break;
    case 'k': GET_RACE(d->character) = RACE_DROW; break;
    case 'l': GET_RACE(d->character) = RACE_LIZARDMAN; break;
    case 'm': GET_RACE(d->character) = RACE_DRACONIAN; break;
    }

    if (helpmode) {
      do_help(d->character, race_table[GET_RACE(d->character)], 0, 0);
      display_races(d);
      return;
    }

    if (d->character && PRF_FLAGGED(d->character, PRF_DISPVT))
      SEND_TO_Q( VTCLS, d);
    display_classes(d);
    STATE(d) = CON_QCLASS;
    break;

  case CON_QCLASS:
    skip_spaces(&arg); /* skip whitespaces */

    if (*arg == '-') {
      display_races(d);
      STATE(d) = CON_QRACE;
      return;
    }

    if (*arg == '*') {
      display_classes(d);
      return;
    }

    if (*arg >= 'A' && *arg <= 'Z')
      helpmode = TRUE;

    *arg = LOWER(*arg);

    if (((*arg < 'a') || (*arg > 'z')) && (*arg != '1') && (*arg != '2')) {
      SEND_TO_Q("\r\nWrong Option!\r\nClass: ", d);
      return;
    }

    if (((*arg >= 'a') && (*arg <= 't')) &&
	!IS_SET(allowed_classes[GET_RACE(d->character)], (1 << (*arg - 'a')))) {
      SEND_TO_Q("\r\nWrong Option!\r\nClass: ", d);
      return;
    } else if ((*arg > 't' && *arg <= 'z') &&
	       !IS_SET(allowed_classes[GET_RACE(d->character)], M2_CLASS)) {
      SEND_TO_Q("\r\nWrong Option!\r\nClass: ", d);
      return;
    } else if (((*arg == '1') || (*arg == '2')) &&
	       !IS_SET(allowed_classes[GET_RACE(d->character)], M3_CLASS)) {
      SEND_TO_Q("\r\nWrong Option!\r\nClass: ", d);
      return;
    }

    if ((*arg == 'q') || (*arg == 'r') || (*arg == 't')) {
      SEND_TO_Q("\r\nWrong Option!\r\nClass: ", d);
      return;
    }

    if ((*arg >= 'a') && (*arg <= 't')) {
      GET_CLASS(d->character) = (*arg - 'a' + 1);
    } else {
      switch (*arg) {
      case 'u': GET_CLASS(d->character) = CLASS_2MULTI;
	GET_1CLASS(d->character) = CLASS_WARRIOR;
	GET_2CLASS(d->character) = CLASS_THIEF; break;
      case 'v': GET_CLASS(d->character) = CLASS_2MULTI;
	GET_1CLASS(d->character) = CLASS_WARRIOR;
	GET_2CLASS(d->character) = CLASS_CLERIC; break;
      case 'w': GET_CLASS(d->character) = CLASS_2MULTI;
	GET_1CLASS(d->character) = CLASS_WARRIOR;
	GET_2CLASS(d->character) = CLASS_MAGIC_USER; break;
      case 'x': GET_CLASS(d->character) = CLASS_2MULTI;
	GET_1CLASS(d->character) = CLASS_THIEF;
	GET_2CLASS(d->character) = CLASS_CLERIC; break;
      case 'y': GET_CLASS(d->character) = CLASS_2MULTI;
	GET_1CLASS(d->character) = CLASS_THIEF;
	GET_2CLASS(d->character) = CLASS_MAGIC_USER; break;
      case 'z': GET_CLASS(d->character) = CLASS_2MULTI;
	GET_1CLASS(d->character) = CLASS_CLERIC;
	GET_2CLASS(d->character) = CLASS_MAGIC_USER; break;
      case '1': GET_CLASS(d->character) = CLASS_3MULTI;
	GET_1CLASS(d->character) = CLASS_WARRIOR;
	GET_2CLASS(d->character) = CLASS_THIEF;
	GET_3CLASS(d->character) = CLASS_MAGIC_USER; break;
      case '2': GET_CLASS(d->character) = CLASS_3MULTI;
	GET_1CLASS(d->character) = CLASS_WARRIOR;
	GET_2CLASS(d->character) = CLASS_CLERIC;
	GET_3CLASS(d->character) = CLASS_MAGIC_USER; break;
      default :
	SEND_TO_Q("\r\nWrong Option!\r\nClass: ", d);
	sprintf(buf, "SYSERR: Select Class Option %s", arg);
        log(buf);
	return;
      }
    }

    if (helpmode) {
      do_help(d->character, pc_class_types[GET_CLASS(d->character)], 0, 0);
      SEND_TO_Q("\r\n  [*] List classes\r\nClass: ", d);
      return;
    }

    init_char(d->character);
    if (d->pos < 0) {
      d->pos = create_entry(GET_NAME(d->character));
      d->character->nr = d->pos;
    }
    save_char(d->character, NOWHERE, 2);

    /* Fix so idnum is stored for new players - Charlene */
    player_table[top_of_p_table].idnum = GET_IDNUM(d->character);

    if (d->character && PRF_FLAGGED(d->character, PRF_DISPVT))
      SEND_TO_Q( VTCLS, d);
    SEND_TO_Q(motd, d);
    SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
    STATE(d) = CON_RMOTD;

    sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, LEVEL_IMMORT, TRUE);
    break;

  case CON_RMOTD:		/* read CR after printing motd	*/
    if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
      SEND_TO_Q(PC_MENU, d);
    else
      SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    break;

  case CON_SLCT:		/* get selection from main menu	*/

    skip_spaces(&arg); /* skip whitespaces */

    switch (*arg) {
    case '0':
      if (d->character && PRF_FLAGGED(d->character, PRF_DISPVT)) {
	sprintf(buf, VTCLS VTSCRREG,
		1, GET_SCRLEN(d->character) + 1);
	write_to_descriptor(d->descriptor, buf);
      }

      Crash_delete_file(GET_NAME(d->character), OBJECT_BACKUP);

      write_to_descriptor(d->descriptor, BYE_MESSG);
      close_socket(d);
      break;

    case '1':

      /* this code is to prevent people from multiply logging in */
      for (k = descriptor_list; k; k=next) {
	next = k->next;
	if (!k->connected && k->character &&
	    !str_cmp(GET_NAME(k->character), GET_NAME(d->character))){
	  SEND_TO_Q("Your character has been deleted.\r\n",d);
	  STATE(d) = CON_CLOSE;
	  return;
	}
      }
      reset_char(d->character);
      if ((load_result = Crash_load(d->character)))
	if (real_room(GET_LOADROOM(d->character)) != r_frozen_start_room)
	  GET_LOADROOM(d->character) = NOWHERE;
      save_char(d->character, NOWHERE, 0);

      insert_to_char_list(d->character);

      if (GET_LEVEL(d->character) >= LEVEL_DEITY) {
	if (!PLR_FLAGGED(d->character, PLR_LOADROOM)) {
	  if (GET_LOADROOM(d->character) == NOWHERE ||
	      (load_room = real_room(GET_LOADROOM(d->character))) < 0)
	    load_room = r_immort_start_room;
	} else
	  load_room = r_immort_start_room;
	if (PLR_FLAGGED(d->character, PLR_INVSTART))
	  GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
      } else {
	if (PLR_FLAGGED(d->character, PLR_FROZEN) ||
	    real_room(GET_LOADROOM(d->character)) == r_frozen_start_room)
	  load_room = r_frozen_start_room;
	else if (load_result || GET_LOADROOM(d->character) == NOWHERE)
	  if (GET_LEVEL(d->character) == 1)
	    load_room = r_newbie_start_room;
	  else
	    load_room = r_mortal_start_room;
	else if (((load_room = real_room(GET_LOADROOM(d->character))) < 0))
	  load_room = r_mortal_start_room;
      }

      char_to_room(d->character, load_room);
      act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

      STATE(d) = CON_PLYNG;

      if (!GET_LEVEL(d->character)) {
	do_start(d->character);
	send_to_char(START_MESSG, d->character);
      }

      if (PRF_FLAGGED(d->character, PRF_DISPVT))
	redraw_screen(d);

      send_to_char(WELC_MESSG, d->character);

      do_look(d->character, "", 0, 0);
      if (has_mail(GET_NAME(d->character)))
	send_to_char("You have mail waiting.\r\n", d->character);
      if (load_result == 2) {	/* rented items lost */
	send_to_char("\r\n\007You could not afford your rent!\r\n"
		     "Your possesions have been donated to the poor!\r\n", d->character);
      }
      alias_load(d->character);
      read_ignorefile(d->character);
      d->prompt_mode = 1;
      REMOVE_BIT(PRF_FLAGS(d->character), PRF_QUEST); /* Clear Quest channel */

      /* Autoexpel Newbie Power */
      if ((CLAN(d->character) == r_newbie_clan) &&
	  (CLAN_LEVEL(d->character) < 5) &&
	  (GET_LEVEL(d->character) >= 25)) {
	   send_to_char("You are no longer a newbie!!!\r\n", d->character);
	   CLAN(d->character) = -1;
	   CLAN_LEVEL(d->character) = 0;
      }

      if ((CLAN(d->character) >= 0) && !PLR_FLAGGED(d->character,PLR_INVSTART))
        send_to_clan(d-> character, "#m$n entering game.#N\r\n");

      if (GET_LEVEL(d->character) >= LEVEL_DEITY) {
	sprintbit((long) GODLEVEL(d->character), imm_powers, buf2);
	sprintf(buf, "%s Imm Powers: %s", GET_NAME(d->character), buf2);
	mudlog(buf, NRM, MAX(LEVEL_ADMIN, GET_INVIS_LEV(d->character)), TRUE);
      }

      break;

    case '2':
      SEND_TO_Q("Enter the text you'd like others to see when they look at you.\r\n", d);
      SEND_TO_Q("(/s saves /h for help)\r\n", d);
      if (d->character->player.description) {
	SEND_TO_Q("Current description:\r\n", d);
	SEND_TO_Q(d->character->player.description, d);
	d->backstr = strdup(d->character->player.description);
      }
      d->str = &d->character->player.description;
      d->max_str = 512;
      STATE(d) = CON_EXDSCR;
      SET_BIT(PLR_FLAGS(d->character), PLR_SAVESTR);
      break;

    case 'p':
      SEND_TO_Q("Enter the text for your plan.  Others will see it with the 'finger' command.\r\n", d);
      SEND_TO_Q("(/s saves /h for help)\r\n", d);
      if (d->character->player.plan) {
	SEND_TO_Q("Current plan:\r\n", d);
	SEND_TO_Q(d->character->player.plan, d);
	d->backstr = strdup(d->character->player.plan);
      }
      d->str = &d->character->player.plan;
      d->max_str = 1024;
      STATE(d) = CON_EXDSCR;
      SET_BIT(PLR_FLAGS(d->character), PLR_SAVESTR);
      break;

    case '3':
      SEND_TO_Q(background, d);
      SEND_TO_Q("\r\n\r\n*** PRESS RETURN:", d);
      STATE(d) = CON_RMOTD;
      break;

    case '4':
      SEND_TO_Q("Enter your old password: ", d);
      echo_off(d->descriptor);
      STATE(d) = CON_PWDNQO;
      break;

    case '5':
      SEND_TO_Q("\r\nEnter your password for verification: ", d);
      echo_off(d->descriptor);
      STATE(d) = CON_DELCNF1;
      break;

    default:
      SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
      if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
	SEND_TO_Q(PC_MENU, d);
      else
	SEND_TO_Q(MENU, d);
      break;
    }

    break;

  case CON_PWDNQO:
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    if (strncmp(CRYPT(arg,GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
	SEND_TO_Q(PC_MENU, d);
      else
	SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
      echo_on(d->descriptor);
      return;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_PWDNEW;
      return;
    }
    break;
  case CON_PWDNEW:

    /* skip whitespaces */
    for (; isspace(*arg); arg++)
      ;

    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }

    strncpy(GET_PASSWD(d->character), CRYPT(arg, d->character->player.name), MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    SEND_TO_Q("\r\nPlease retype password: ", d);
    STATE(d) = CON_PWDNCNF;
    break;

  case CON_PWDNCNF:
    /* skip whitespaces */
    for (; isspace(*arg); arg++)
      ;

    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      STATE(d) = CON_PWDNEW;
      return;
    }

    SEND_TO_Q("\r\nDone.  You must enter the game to make the change final.\r\n", d);
    if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
      SEND_TO_Q(PC_MENU, d);
    else
      SEND_TO_Q(MENU, d);
    echo_on(d->descriptor);
    STATE(d) = CON_SLCT;
    break;

  case CON_DELCNF1:
    echo_on(d->descriptor);
    for (; isspace(*arg); arg++);
    if (strncmp(CRYPT(arg,GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
	SEND_TO_Q(PC_MENU, d);
      else
	SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
    } else {
      SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
		"ARE YOU ABSOLUTELY SURE?\r\n\r\n"
		"Please type \"yes\" to confirm: ", d);
      STATE(d) = CON_DELCNF2;
    }
    break;

  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
	SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n",d);
	SEND_TO_Q("Character not deleted.\r\n\r\n",d);
	STATE(d) = CON_CLOSE;
	return;
      }

      if (GET_LEVEL(d->character) < LEVEL_IMPL && !PLR_FLAGGED(d->character, PLR_NODELETE)) {
	SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
        save_char(d->character, NOWHERE, 2);
        sprintf(buf, "Character '%s' deleted!\r\nGoodbye.\r\n", GET_NAME(d->character));
        SEND_TO_Q(buf, d);
        sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character), GET_LEVEL(d->character));
        mudlog(buf, NRM, LEVEL_IMMORT, TRUE);
        Crash_crashsave(d->character, MANUAL_SAVE);
        delete_alias_file(d->character);
        STATE(d) = CON_CLOSE;
        return;
      } else {
        SEND_TO_Q("You try to kill yourself, but a mystical force stops you.\r\n",d);
        SEND_TO_Q("Character not deleted.\r\n\r\n", d);
        if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
          SEND_TO_Q(PC_MENU, d);
        else
          SEND_TO_Q(MENU, d);
        STATE(d) = CON_SLCT;
      }
    } else {
      SEND_TO_Q("Character not deleted.\r\n\r\n", d);
      if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
	SEND_TO_Q(PC_MENU, d);
      else
	SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
    }
    break;

  case CON_DEADWAIT:
    if (!arg || !*arg) {        /* Press return to head to menu */
      if (d->character && PRF_FLAGGED(d->character, PRF_IBM_PC))
        SEND_TO_Q(PC_MENU, d);
      else
        SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
    }
    break;

  case CON_CLOSE :
    close_socket(d);
    break;

  default:
    log("SYSERR: Nanny: illegal state of con'ness");
    abort();
    break;
  }
}


/*
 * Code to disable or enable buggy commands on the run, saving
 * a list of disabled commands to disk. Originally created by
 * Erwin S. Andreasen (erwin@andreasen.org) for Merc. Ported to
 * CircleMUD by Alexei Svitkine (Myrdred), isvitkin@sympatico.ca.
 * Converted for Elite by Bod
 *
 * Syntax is:
 *   disable - shows disabled commands
 *   disable <command> - toggles disable status of command
 * 
 */

ACMD(do_disable)
{
  int i, length;
  DISABLED_DATA *p, *q;
	
  if (IS_NPC(ch)) {
    send_to_char("Monsters can't disable commands, silly.\r\n", ch);
    return;
  }
  
  skip_spaces(&argument);
        
  if (!*argument) { /* Nothing specified. Show disabled commands. */
    if (!disabled_first) { /* Any disabled at all ? */
      send_to_char("There are no disabled commands.\r\n", ch);
      return;
    }
    send_to_char("Commands that are currently disabled:\r\n\r\n"
                 " Command       Disabled by     Level\r\n"
                 "-----------   --------------  -------\r\n", ch);
    for (p = disabled_first; p; p = p->next) {
      sprintf(buf, " %-12s   %-12s    %2d\r\n", p->command->command, p->disabled_by, p->level);
      send_to_char(buf, ch);
    }
    return;
  }
	
  /* command given */
  /* First check if it is one of the disabled commands */
  for (length = strlen(argument), p = disabled_first; p ;  p = p->next)
    if (!strncmp(argument, p->command->command, length))
	break;
			
  if (p) { /* this command is disabled */
 
    /* Was it disabled by a higher level imm? */
    if (GET_LEVEL(ch) < p->level && !IS_SET(GODLEVEL(ch), IMM_ALL)) {
      send_to_char("This command was disabled by a higher power.\r\n", ch);
      return;
    }
		
    /* Remove */
		
    if (disabled_first == p) /* node to be removed == head ? */
      disabled_first = p->next;
    else { /* Find the node before this one */
      for (q = disabled_first; q->next != p; q = q->next); /* empty for */
        q->next = p->next;
    }
    
    sprintf(buf, "Command '%s' enabled.\n\r", p->command->command);
    send_to_char(buf, ch);
    free(p->disabled_by);
    free(p);
    save_disabled(); /* save to disk */
    
  } else { /* not a disabled command, check if that command exists */

    for (length = strlen(argument), i = 0; *cmd_info[i].command != '\n'; i++)
     if (!strncmp(cmd_info[i].command, argument, length))
       if (GET_LEVEL(ch) >= cmd_info[i].minimum_level)
         break;

    if (!strcmp(cmd_info[i].command, "disable")) {
      send_to_char ("You cannot disable the disable command.\r\n", ch);
      return;
    }

    /*  Found? 	*/			
    if (*cmd_info[i].command == '\n') {
      send_to_char("You don't know of any such command.\r\n", ch);
      return;
    }

   /* Disable the command */
   CREATE(p, struct disabled_data, 1);
   p->command = &cmd_info[i];
   p->disabled_by = strdup(GET_NAME(ch)); /* save name of disabler  */
   p->level = GET_LEVEL(ch); 		  /* save level of disabler */	
   p->subcmd = cmd_info[i].subcmd; 	  /* the subcommand if any  */	
   p->next = disabled_first;
   disabled_first = p; /* add before the current first element */
   sprintf(buf, "Command '%s' disabled.\n\r", p->command->command);
   send_to_char(buf, ch);
   save_disabled(); /* save to disk */
  }
}

/* check if a command is disabled */   
int check_disabled(const struct command_info *command) {

  DISABLED_DATA *p;
	
  for (p = disabled_first; p ; p = p->next)
    if (p->command->command_pointer == command->command_pointer)
      if (p->command->subcmd == command->subcmd)
        return TRUE;

  return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
  FILE *fp;
  DISABLED_DATA *p;
  int i;
  char line[READ_SIZE], name[MAX_INPUT_LENGTH], temp[MAX_INPUT_LENGTH];
  
  disabled_first = NULL;
	
  fp = fopen(DISABLED_FILE, "r");
	
  if (!fp) /* No disabled file.. no disabled commands. */
    return;

  get_line(fp, line);
  while (str_cmp(line, END_MARKER)) { /* as long as name is NOT END_MARKER :) */
    CREATE(p, struct disabled_data, 1);
    sscanf(line, "%s %d %hd %s", name, &(p->subcmd), &(p->level), temp);
    /* Find the command in the table */
    for (i = 0; cmd_info[i].command ; i++)
      if (!str_cmp(cmd_info[i].command, name))
        break;
    if (!cmd_info[i].command) { /* command does not exist? */
      log("SYSERR: Skipping unknown command in " DISABLED_FILE " file.");
      free(p);  
    } else { /* add new disabled command */
      p->disabled_by = strdup(temp);
      p->command = &cmd_info[i];
      p->next = disabled_first;
      disabled_first = p;
    }
    get_line(fp, line);
  }
  fclose (fp);		
}

/* Save disabled commands */
void save_disabled()
{
  FILE *fp;
  DISABLED_DATA *p;
	
  if (!disabled_first) {
    /* delete file if no commands are disabled */
    unlink(DISABLED_FILE);
    return;
   }
	
  fp = fopen (DISABLED_FILE, "w");
	
  if (!fp) {
    log("SYSERR: Could not open " DISABLED_FILE " for writing");
    return;
  }
	
  for (p = disabled_first; p ; p = p->next)
    fprintf (fp, "%s %d %d %s\n", p->command->command, p->subcmd, p->level, p->disabled_by);
  fprintf (fp, "%s\n", END_MARKER);
  fclose (fp);
}


/*
************************************************************************
*   File: boards.c                                      Part of EliteMUD  *
*  Usage: handling of multiple bulletin boards                            *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

/* FEATURES & INSTALLATION INSTRUCTIONS ***********************************

Written by Jeremy "Ras" Elson (jelson@server.cs.jhu.edu)

This board code has many improvements over the infamously buggy standard
Diku board code.  Features include:

- Arbitrary number of boards handled by one set of generalized routines.
  Adding a new board is as easy as adding another entry to an array.
- Bug-free operation -- no more mixed messages!
- Safe removal of messages while other messages are being written.
- Does not allow messages to be removed by someone of a level less than
  the poster's level.

To install:

0.  Edit your makefile so that boards.c is compiled and linked into the server.

1.  In spec_assign.c, declare the specproc "gen_board".  Give ALL boards
    the gen_board specproc.

2.  In boards.h, change the constants CMD_READ, CMD_WRITE, CMD_REMOVE, and
    CMD_LOOK to the correct command numbers for your mud's interpreter.

3.  In boards.h, change NUM_OF_BOARDS to reflect how many different types
    of boards you have.  Change MAX_BOARD_MESSAGES to the maximum number
    of messages postable before people start to get a 'board is full'
    message.

4.  Follow the instructions for adding a new board (below) to correctly
    define the board_info array (also below).  Make sure you define an
    entry in this array for each object that you gave the gen_board specproc
    in step 1.

Send comments, bug reports, help requests, etc. to Jeremy Elson
(jelson@server.cs.jhu.edu).  Enjoy!

************************************************************************/

/* TO ADD A NEW BOARD, simply follow our easy 3-step program:

1 - Create a new board object in the object files

2 - Increase the NUM_OF_BOARDS constant in board.h

3 - Add a new line to the board_info array below.  The fields, in order, are:

	Board's virtual number.
	Min level one must be to look at this board or read messages on it.
	Min level one must be to post a message to the board.
	Min level one must be to remove other people's messages from this
		board (but you can always remove your own message).
	Filename of this board, in quotes.
	Last field must always be 0.
*/


#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "boards.h"
#include "interpreter.h"
#include "handler.h"
#include "functions.h"

extern char *get_name_idnum(long idnum);
extern sbyte get_level_idnum(long idnum);

extern struct room_data **world;
extern struct descriptor_data *descriptor_list;
extern int real_clan(int virtual);
extern struct clan_data *clan_list;

int set_msg_flag(struct message_flag msg_flag, struct char_data *ch, int board_type, int ind);


/*
format:	vnum, flags, ref, read lvl, write lvl, remove lvl, filename, 0 at end
Be sure to also change NUM_OF_BOARDS in board.h
   { Vnum, READ, WRITE, REMOVE, "filename", 0 }
*/
struct board_info_type board_info[NUM_OF_BOARDS] = {
  {3099, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.mort" , 0 },
  {3098, 0, 0, LEVEL_DEITY, LEVEL_DEITY, LEVEL_LESSER, "boards/Board.immort" ,0},
  {3097, 0, 0, LEVEL_DEITY, LEVEL_DEITY, LEVEL_LESSER, "boards/Board.jail" ,0},
  {3096, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.social" , 0},
  {3095, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.quest" , 0},
  { 899, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.chalumeau" , 0 },
  { 104, 0, 0, LEVEL_ADMIN, 0, LEVEL_ADMIN, "boards/Board.overseer" , 0 },
  {3087, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.newbie" , 0 },
  {3088, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.area" , 0 },
  {3089, 0, 0, LEVEL_DEITY, LEVEL_DEITY, LEVEL_GREATER, "boards/Board.world" , 0},
  {3090, 0, 0, LEVEL_DEITY, LEVEL_DEITY, LEVEL_GREATER, "boards/Board.object" ,0},
  {3091, 0, 0, LEVEL_DEITY, LEVEL_DEITY, LEVEL_GREATER, "boards/Board.mobile" ,0},
  {3092, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.skill" , 0 },
  {3093, BOARD_REMORT, 0, 0, 0, LEVEL_DEITY, "boards/Board.remort" , 0 },
  {3094, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.general" , 0 },
  {3084, BOARD_CODE, 0, LEVEL_DEITY, LEVEL_DEITY, LEVEL_GREATER, "boards/Board.code", 0},
  {3082, 0, 0, 0, 0, LEVEL_GREATER, "boards/Board.arena", 0},
  {2113, 0, 0, 0, 0, LEVEL_DEITY, "boards/Board.pk", 0},
  { 135, 0, 0, LEVEL_DEITY, LEVEL_GREATER, LEVEL_GREATER, "boards/rulebook.holy" , 0 },
  {   7, BOARD_CLAN, 121, 0, 0, 10, "boards/Clan.ntribe2", 0},
  {   8, BOARD_CLAN, 110, 0, 0, 10, "boards/Clan.ms", 0},
  {  15, BOARD_CLAN, 110, 0, 0, 10, "boards/Clan.elite", 0},
  {27509,BOARD_CLAN, 110, 0, 0, 10, "boards/Clan.elite2", 0},
  {  27, BOARD_CLAN, 107, 0, 0, 9, "boards/Clan.tri", 0},
  {  16, BOARD_CLAN, 122, 0, 0, 10, "boards/Clan.cod", 0},
  {  29, BOARD_CLAN, 112, 0, 0, 10, "boards/Clan.sd", 0},
  {  30, BOARD_CLAN, 116, 2, 2, 10, "boards/Clan.tkk", 0},
  {28102,BOARD_CLAN, 116, 0, 0, 10, "boards/Clan.tkk2", 0},
  {  17, BOARD_CLAN, 118, 0, 0, 10, "boards/Clan.elysium", 0},
  {  31, BOARD_CLAN, 117, 0, 0, 9, "boards/Clan.kai", 0},
  {28202,BOARD_CLAN, 117, 0, 0, 9, "boards/Clan.kai2", 0},
  {  32, BOARD_CLAN, 121, 0, 0, 10, "boards/Clan.ntribe", 0},
  {  33, BOARD_CLAN, 122, 0, 0, 10, "boards/Clan.malina", 0},
  {  34, BOARD_CLAN,   0, 0, 0, 10, "boards/Clan.art", 0},
  {  36, BOARD_CLAN, 113, 0, 0, 10, "boards/Clan.moc", 0},
  {27903,BOARD_CLAN, 113, 0, 0, 10, "boards/Clan.moc2", 0},
  {28000,BOARD_CLAN, 114, 0, 0, 9, "boards/Clan.dragoon", 0},
  {28001,BOARD_CLAN, 114, 9, 2, 9, "boards/Clan.dragoon2", 0},
  {  40, BOARD_CLAN, 100, 0, 0, 9, "boards/Clan.ag", 0},
  {28826,BOARD_CLAN, 119, 0, 0, 10, "boards/Clan.goa", 0},
  {28825,BOARD_CLAN, 119, 0, 0, 10, "boards/Clan.wew", 0},
  {  44, BOARD_CLAN, 115, 0, 0, 10, "boards/Clan.norsca", 0},
  {  47, BOARD_CLAN, 111, 0, 0, 10, "boards/Clan.ni", 0}
};


char	*msg_storage[INDEX_SIZE];
int	msg_storage_taken[INDEX_SIZE];
int	num_of_msgs[NUM_OF_BOARDS];
int CMD_READ, CMD_LOOK, CMD_WRITE, CMD_EDIT, CMD_REMOVE, CMD_LOCK, CMD_UNLOCK;

struct board_msginfo msg_index[NUM_OF_BOARDS][MAX_BOARD_MESSAGES];


int	find_slot(void)
{
   int	i;

   for (i = 0; i < INDEX_SIZE; i++)
      if (!msg_storage_taken[i]) {
	 msg_storage_taken[i] = 1;
	 return i;
      }

   return - 1;
}


/* search the room ch is standing in to find which board he's looking at */
int	find_board(struct obj_data *obj)
{
   int	i;

   for (i = 0; i < NUM_OF_BOARDS; i++)
     if (RNUM(i) == obj->item_number)
       return i;

   return - 1;
}


void	init_boards(void)
{
   int	i, j, fatal_error = 0;
   char	buf[256];

   for (i = 0; i < INDEX_SIZE; i++) {
      msg_storage[i] = 0;
      msg_storage_taken[i] = 0;
   }

   for (i = 0; i < NUM_OF_BOARDS; i++) {
      if ((RNUM(i) = real_object(VNUM(i))) == -1) {
	 sprintf(buf, "SYSERR: Fatal board error: board vnum %ld does not exist!", VNUM(i));
	 log(buf);
/*	 fatal_error = 1; */
      }
      num_of_msgs[i] = 0;
      for (j = 0; j < MAX_BOARD_MESSAGES; j++) {
	 memset(&(msg_index[i][j]), 0, sizeof(struct board_msginfo ));
	 msg_index[i][j].slot_num = -1;
      }
      Board_load_board(i);
   }

   CMD_READ = find_command("read");
   CMD_WRITE = find_command("write");
   CMD_EDIT = find_command("amend");
   CMD_REMOVE = find_command("remove");
   CMD_LOOK = find_command("look");
   CMD_LOCK = find_command("lock");
   CMD_UNLOCK = find_command("unlock");

   if (fatal_error)
      exit(0);
}


SPECIAL(gen_board)
{
  int	board_type;
  static int	loaded = 0;

  if (!ch->desc)
    return 0;

  if (!loaded) {
    init_boards();
    loaded = 1;
  }

  if (cmd != CMD_WRITE && cmd != CMD_LOOK && cmd != CMD_READ &&
      cmd != CMD_REMOVE && cmd != CMD_EDIT && cmd != CMD_LOCK &&
      cmd != CMD_UNLOCK)
    return 0;

  if ((board_type = find_board(obj)) == -1) {
    log("SYSERR:  degenerate board!  (what the hell..)");
    return 0;
  }

  if ((cmd == CMD_READ || cmd == CMD_REMOVE || cmd == CMD_EDIT || cmd ==
CMD_LOCK || cmd == CMD_UNLOCK) && strstr(arg, "."))
   return 0;

  if (cmd == CMD_WRITE) {
    Board_write_message(board_type, ch, arg);
    return 1;
  }
  else
    if (cmd == CMD_LOOK)
    {
      Board_save_board(board_type);
      return Board_show_board(board_type, ch, obj, arg);
    }
	    else
	      if (cmd == CMD_READ)
          return Board_display_msg(board_type, ch, obj, arg);
        else
		      if (cmd == CMD_REMOVE)
            return Board_remove_msg(board_type, ch, arg);
          else
            if (cmd == CMD_EDIT)
              return Board_edit_msg(board_type, ch, arg);
            else
              if (cmd == CMD_LOCK)
                return Board_lock_msg(board_type, ch, arg);
              else
                if (cmd == CMD_UNLOCK)
                  return Board_unlock_msg(board_type, ch, arg);
                else
                  return 0;
}


void	Board_write_message(int board_type, struct char_data *ch, char *arg)
{
  char	*tmstr;
  int	len;
  long	ct;
  char	buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  if (BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("Unable to write on the board it is locked.\r\n", ch);
    return;
  }

  if (BOARD_FLAGGED(board_type, BOARD_CLAN))
  {
    if (WRITE_LVL(board_type) > 0 && CLAN(ch) != real_clan(BOARD_REF(board_type)) &&
        GET_LEVEL(ch) < LEVEL_BOARD)
    {
      sprintf(buf, "You are not in %s#N and are not allowed to write on this board.\r\n",
              clan_list[real_clan(BOARD_REF(board_type))].name);
      send_to_char(buf, ch);
      return;
    }
    else
      if (CLAN_LEVEL(ch) < WRITE_LVL(board_type) && GET_LEVEL(ch) < LEVEL_BOARD)
      {
        sprintf(buf, "You are not of sufficient rank in %s to write on this board.\r\n",
                clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return;
      }
  }
  else
    if (GET_LEVEL(ch) < WRITE_LVL(board_type))
    {
      send_to_char("You are not holy enough to write on this board.\r\n", ch);
      return;
    }

  if (num_of_msgs[board_type] >= MAX_BOARD_MESSAGES)
  {
    send_to_char("The board is full.\r\n", ch);
    return;
  }

  if ((NEW_MSG_INDEX(board_type).slot_num = find_slot()) == -1)
  {
    send_to_char("The board is malfunctioning - sorry.\r\n", ch);
    log("SYSERR: Board: failed to find empty slot on write.");
    return;
  }

  /* skip blanks */
  for (; isspace(*arg); arg++);

  delete_doubledollar(arg);

  if (!*arg)
  {
    send_to_char("We must have a headline!\r\n", ch);
    return;
  }

  ct = time(0);
  tmstr = (char *) asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  /* Truncate String at 80 characters */
  arg[81] = '\0';

  sprintf(buf2, "(%s)", GET_NAME(ch));
/* Fixed so LEVEL_ADMIN and LEVEL_IMPL write in the same color -
Charlene*/
  sprintf(buf, "#Y%6.10s #c%-12s#N :: %s%s#N", tmstr, buf2,
	  (GET_LEVEL(ch)>=LEVEL_ADMIN?"#r":"#g"), arg);
  len = strlen(buf) + 1;
  if (!(NEW_MSG_INDEX(board_type).heading = (char *)malloc(sizeof(char)*len)))
  {
    send_to_char("The board is malfunctioning - sorry.\r\n", ch);
    return;
  }

  strcpy(NEW_MSG_INDEX(board_type).heading, buf);
  NEW_MSG_INDEX(board_type).heading[len-1] = '\0';
  NEW_MSG_INDEX(board_type).heading_len = len;

  if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    if (CLAN(ch) != real_clan(BOARD_REF(board_type)))
      NEW_MSG_INDEX(board_type).level = 0;
    else
      NEW_MSG_INDEX(board_type).level = CLAN_LEVEL(ch);
  else
    NEW_MSG_INDEX(board_type).level = GET_LEVEL(ch);

  send_to_char("Write your message, (/s saves /h for help).", ch);
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

  if (!IS_NPC(ch))
    SET_BIT(PLR_FLAGS(ch), PLR_WRITING);

  ch->desc->str = &(msg_storage[NEW_MSG_INDEX(board_type).slot_num]);
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  num_of_msgs[board_type]++;
}


int	Board_show_board(int board_type, struct char_data *ch, struct obj_data *obj, char *arg)
{
  int	i;
  char	tmp[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];

  if (!ch->desc)
    return 0;

  one_argument(arg, tmp);

  if (!*tmp || !isname(tmp, obj->name))
    return 0;

  if (BOARD_FLAGGED(board_type, BOARD_CLAN)) {
    if (READ_LVL(board_type) > 0 && CLAN(ch) != real_clan(BOARD_REF(board_type)) &&
        GET_LEVEL(ch) < LEVEL_BOARD)  {
      sprintf(buf, "You are not in %s#N and are not allowed to read this board.\r\n",
                   clan_list[real_clan(BOARD_REF(board_type))].name);
      send_to_char(buf, ch);
      return 1;
    }

    if (CLAN_LEVEL(ch) < READ_LVL(board_type) && GET_LEVEL(ch) < LEVEL_BOARD) {
      sprintf(buf, "You are not of sufficient rank in %s#N to read this board.\r\n",
                   clan_list[real_clan(BOARD_REF(board_type))].name);
      send_to_char(buf, ch);
      return 1;
    }
  } else if (GET_LEVEL(ch) < READ_LVL(board_type)) {
    send_to_char("You try but fail to understand the holy words.\r\n", ch);
    return 1;
  }

  act("$n studies $p.", TRUE, ch, obj, 0, TO_ROOM);

  sprintf(buf, "This is %s.\r\nUsage: READ/AMEND/(UN)LOCK/REMOVE <messg #>, WRITE <header>.\r\n"
	  "You will need to look at the board to save your message.\r\n",
	  obj->short_description);
  if (!num_of_msgs[board_type])
    strcat(buf, "The board is empty.\r\n");
  else
    {
      sprintf(buf + strlen(buf), "There are #w%d#N messages.\r\n",
	      num_of_msgs[board_type]);
      for (i = num_of_msgs[board_type] - 1; i >= 0; i--) {
	if (MSG_HEADING(board_type, i))
	  sprintf(buf + strlen(buf), "#w%-2d#N : %s\r\n", num_of_msgs[board_type] - i, MSG_HEADING(board_type, i));
	else {
	  log("SYSERR: The board is fubar'd.");
	  send_to_char("Sorry, the board isn't working.\r\n", ch);
	  return 1;
	}
      }
    }
  page_string(ch->desc, buf, 1);

  return 1;
}

int	Board_display_msg(int board_type, struct char_data *ch, struct obj_data *obj, char *arg)
{
  char	number[MAX_INPUT_LENGTH], buffer[1024], buf[1024], buf2[1024];
  int	msg, ind, num_flags, i;
  long *flags;
  struct message_flag msg_flag;


  one_argument(arg, number);
  if (!*number)
    return 0;

  if (isname(number, obj->name))
    return(Board_show_board(board_type, ch, obj, arg));

  if (!(msg = atoi(number)))
    return 0;

  if (BOARD_FLAGGED(board_type, BOARD_CLAN))
  {
    if (READ_LVL(board_type) > 0 &&
        CLAN(ch) != real_clan(BOARD_REF(board_type)) &&
        GET_LEVEL(ch) < LEVEL_BOARD)
    {
      sprintf(buf, "You are not in %s#N and are not allowed to read this board.\r\n",
                   clan_list[real_clan(BOARD_REF(board_type))].name);
      send_to_char(buf, ch);
      return 1;
    }

    if (CLAN_LEVEL(ch) < READ_LVL(board_type) && GET_LEVEL(ch) < LEVEL_BOARD)
    {
      sprintf(buf, "You are not in %s#N and are not allowed to read this board.\r\n",
                   clan_list[real_clan(BOARD_REF(board_type))].name);
      send_to_char(buf, ch);
      return 1;
    }
  }
  else if (GET_LEVEL(ch) < READ_LVL(board_type))
  {
    send_to_char("You try but fail to understand the holy words.\r\n", ch);
    return 1;
  }

  if (!num_of_msgs[board_type])
  {
    act("$p is empty!", FALSE, ch, obj, 0, TO_CHAR);
    return(1);
  }
  if (msg < 1 || msg > num_of_msgs[board_type])
  {
    send_to_char("That message exists only in your imagination..\r\n",
		 ch);
    return(1);
  }

  ind = num_of_msgs[board_type] - msg;

  if (MSG_SLOTNUM(board_type, ind) < 0 ||
      MSG_SLOTNUM(board_type, ind) >= INDEX_SIZE)
  {
    send_to_char("Sorry, the board is not working.\r\n", ch);
    log("SYSERR: Board is screwed up.");
    return 1;
  }

  if (!(MSG_HEADING(board_type, ind)))
  {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }

  if (!(msg_storage[MSG_SLOTNUM(board_type, ind)]))
  {
    send_to_char("That message seems to be empty.\r\n", ch);
    return 1;
  }


  buf[0] = '\0';

  num_flags = MSG_HEADING_LEN(board_type, ind) - (strlen(MSG_HEADING(board_type, ind)) + 1);
  num_flags /= 3*sizeof(long);

  if (num_flags)
  {
    flags = (long *)(MSG_HEADING(board_type, ind) + strlen(MSG_HEADING(board_type, ind)) + 1);
    i = 0;
    sprintf(buf, "\r\n---\r\n");
    while (i < num_flags)
    {
      memcpy(msg_flag.flag, flags, sizeof(long));
      flags++;
      msg_flag.idnum = *flags;
      flags++;
      msg_flag.date = *flags;
      flags++;

      sprintf(buf2, "%s", get_name_idnum(msg_flag.idnum));
      CAP(buf2);

      if (msg_flag.flag[1] == MSG_EDITED)
      {
        sprintf(buffer, "#bEdited: #e%6.10s#b :: %s#N\r\n", asctime(localtime(&msg_flag.date)), buf2);
        strcat(buf, buffer);
      }
      else
        if (msg_flag.flag[1] == MSG_LOCKED)
        {
          sprintf(buffer, "#rLocked: #e%6.10s#r :: %s#N\r\n", asctime(localtime(&msg_flag.date)), buf2);
          strcat(buf, buffer);
        }
        else
          if (msg_flag.flag[1] == MSG_STICKY)
          {
            sprintf(buffer, "#gSticky: #e%6.10s#g :: %s#N\r\n", asctime(localtime(&msg_flag.date)), buf2);
            strcat(buf, buffer);
          }
          else
          {
            sprintf(buffer, "#wUnknown: #e%6.10s#w :: %s#N\r\n", asctime(localtime(&msg_flag.date)), buf2);
            strcat(buf, buffer);
          }

      i++;
    }
    strcat(buf, "---\r\n");
  }
  else
    if (num_flags)
    {
      // flag corrupted ignore it and set new length to cut it out next time a flag is added
      MSG_HEADING_LEN(board_type, ind) = (strlen(MSG_HEADING(board_type, ind)) + 1);
    }

  /*// Debug
  sprintf(buffer, "Message heading length:       %d\r\n"
                  "Message heading total length: %d\r\n"
                  "Total flag bytes:             %d\r\n"
                  "Total flags:                  %d\r\n"
                  "---\r\n",
                  strlen(MSG_HEADING(board_type, ind)) + 1,
                  MSG_HEADING_LEN(board_type, ind),
                  MSG_HEADING_LEN(board_type, ind) - (strlen(MSG_HEADING(board_type, ind)) + 1),
                  num_flags);
  send_to_char(buffer, ch);


  for (i=0;i<strlen(MSG_HEADING(board_type, ind));i++)
  {
    if (*(MSG_HEADING(board_type, ind) + i) == '#')
      sprintf(buffer, "%c ", *(MSG_HEADING(board_type, ind) + i));
    else
      sprintf(buffer, "%c", *(MSG_HEADING(board_type, ind) + i));
    send_to_char(buffer, ch);
  }

  send_to_char("\r\n---\r\n", ch);
  // End Debug */

  sprintf(buffer, "Message %d : %s\r\n%s\r\n",
          msg,
	      MSG_HEADING(board_type, ind),
          buf);

  send_to_char(buffer, ch);

  page_string(ch->desc, msg_storage[MSG_SLOTNUM(board_type, ind)], 1);

  send_to_char("\r\n", ch);

  return 1;
}


int	Board_remove_msg(int board_type, struct char_data *ch, char *arg)
{
  int	ind, msg, slot_num;
  char	number[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int remove_okay = 0;

  one_argument(arg, number);

  if (!*number || !isdigit(*number))
    return 0;
  if (!(msg = atoi(number)))
    return(0);

  if (!num_of_msgs[board_type])
  {
    send_to_char("The board is empty!\r\n", ch);
    return 1;
  }

  if (msg < 1 || msg > num_of_msgs[board_type])
  {
    send_to_char("That message exists only in your imagination..\r\n", ch);
    return 1;
  }

  ind = num_of_msgs[board_type] - msg;
  if (!MSG_HEADING(board_type, ind))
  {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }

  if (BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("Unable to remove message, the board is locked.\r\n", ch);
    return 1;
  }

  /* Locked messages has to be unlocked before removed */
  if (msg_locked(board_type, ind))
  {
    send_to_char("That message is locked.\r\n", ch);
    return 1;
  }

  sprintf(buf, "(%s)", GET_NAME(ch));

  if ((GET_LEVEL(ch) >= LEVEL_DEITY) && IS_SET(GODLEVEL(ch), IMM_ALL))
    remove_okay = 1;
  else
    if (BOARD_FLAGGED(board_type, BOARD_REMORT) &&
	     (GET_LEVEL(ch) >= LEVEL_DEITY) &&
	      IS_SET(GODLEVEL(ch), IMM_REMORT))
      remove_okay = 1;
    else
      if (BOARD_FLAGGED(board_type, BOARD_CODE) &&
         (GET_LEVEL(ch) >= LEVEL_DEITY) &&
         IS_SET(GODLEVEL(ch), IMM_CODE))
        remove_okay = 1;
      else
        if (BOARD_FLAGGED(board_type, BOARD_CLAN) &&
           (GET_LEVEL(ch) >= LEVEL_DEITY) &&
           IS_SET(GODLEVEL(ch), IMM_CLAN))
          remove_okay = 1;
        else
          if (BOARD_FLAGGED(board_type, BOARD_CLAN) &&
             (CLAN(ch) == real_clan(BOARD_REF(board_type))) &&
             (CLAN_LEVEL(ch) >= REMOVE_LVL(board_type)))
            remove_okay = 1;
          else
            remove_okay = 0;

  if (!remove_okay)
  {
    if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    {
      if (CLAN(ch) != real_clan(BOARD_REF(board_type)) &&
	      !(strstr(MSG_HEADING(board_type, ind), buf)))
      {
        sprintf(buf, "You can not remove other people's messages from a %s#N board.\r\n",
                     clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return 1;
      }

      if (CLAN_LEVEL(ch) < REMOVE_LVL(board_type) &&
	      !(strstr(MSG_HEADING(board_type, ind), buf)))
      {
        sprintf(buf, "You are not of sufficient rank in %s#N to remove other people's messages.\r\n",
                     clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return 1;
      }

      if ((CLAN_LEVEL(ch) < MSG_LEVEL(board_type, ind)) && (CLAN_LEVEL(ch) < 10))
      {
        send_to_char("You can't remove a message of higher clan rank "
                     "than yourself.\r\n", ch);
        return 1;
      }
    }
    else
    {
      if (GET_LEVEL(ch) < REMOVE_LVL(board_type) &&
  	    !(strstr(MSG_HEADING(board_type, ind), buf)))
      {
        send_to_char("You are not holy enough to remove other people's "
                     "messages.\r\n", ch);
        return 1;
      }

      if ((GET_LEVEL(ch) < MSG_LEVEL(board_type, ind)) &&
          (GET_LEVEL(ch) < LEVEL_ADMIN))
      {
        send_to_char("You can't remove a message holier than yourself.\r\n", ch);
        return 1;
      }
    }
  }

  slot_num = MSG_SLOTNUM(board_type, ind);

  if (slot_num < 0 || slot_num >= INDEX_SIZE)
  {
    log("SYSERR: The board is seriously screwed up.");
    send_to_char("That message is majorly screwed up.\r\n", ch);
    return 1;
  }

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected && d->str == &(msg_storage[slot_num]))
    {
      send_to_char("At least wait until the author is finished before removing it!\r\n", ch);
      return 1;
    }

  if (msg_storage[slot_num])
    free(msg_storage[slot_num]);

  msg_storage[slot_num] = 0;
  msg_storage_taken[slot_num] = 0;

  if (MSG_HEADING(board_type, ind))
    free(MSG_HEADING(board_type, ind));

  for (; ind < num_of_msgs[board_type] - 1; ind++)
  {
    MSG_HEADING(board_type, ind)     = MSG_HEADING(board_type, ind + 1);
    MSG_HEADING_LEN(board_type, ind) = MSG_HEADING_LEN(board_type, ind + 1);
    MSG_SLOTNUM(board_type, ind)     = MSG_SLOTNUM(board_type, ind + 1);
    MSG_LEVEL(board_type, ind)       = MSG_LEVEL(board_type, ind + 1);
  }

  num_of_msgs[board_type]--;

  send_to_char("Message removed.\r\n", ch);

  sprintf(buf, "$n just removed message %d.", msg);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  Board_save_board(board_type);

  return 1;
}


int	Board_edit_msg(int board_type, struct char_data *ch, char *arg)
{
  int	ind, msg, slot_num, level, imp, head_len;
  char *ptr;
  char number[MAX_INPUT_LENGTH], buf[1024];
  struct descriptor_data *d;
  struct message_flag msg_flag;

  one_argument(arg, number);

  if (!*number || !isdigit(*number))
  {
    send_to_char("You must specify a message number..\r\n", ch);
    return 1;
  }

  if (!(msg = atoi(number)))
  {
    send_to_char("You must specify a message number..\r\n", ch);
    return 1;
  }

  if (!num_of_msgs[board_type])
  {
    send_to_char("The board is empty!\r\n", ch);
    return 1;
  }

  if (msg < 1 || msg > num_of_msgs[board_type])
  {
    send_to_char("That message exists only in your imagination..\r\n", ch);
    return 1;
  }

  ind = num_of_msgs[board_type] - msg;
  if (!MSG_HEADING(board_type, ind))
  {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }

  if (BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("Unable to amend message, the board is locked.\r\n", ch);
    return 1;
  }

  /* Locked messages has to be unlocked before amended */
  if (msg_locked(board_type, ind))
  {
    send_to_char("That message is locked.\r\n", ch);
    return 1;
  }

/* Fixed to make LEVEL_IMPL and LEVEL_ADMIN amend message - Charlene */
  imp = (GET_LEVEL(ch) >= LEVEL_ADMIN ? 1 : 0);
  sprintf(buf, "(%s)", GET_NAME(ch));

  if (!BOARD_FLAGGED(board_type, BOARD_CLAN))
  {
/* Changed to use correct function - Charlene */
   if ((!strstr(MSG_HEADING(board_type, ind), buf)) && !imp)
    {
      send_to_char("You can't amend other people's messages.\r\n", ch);
      return 1;
    }
  }
  else
  {
    if (!(strstr(MSG_HEADING(board_type, ind), buf)) && !imp)
    {
      if (CLAN(ch) != real_clan(BOARD_REF(board_type)))
      {
        sprintf(buf, "You cant amend messages on a %s#N board.",
                clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return 1;
      }
      if (CLAN_LEVEL(ch) < MSG_LEVEL(board_type, ind) &&
          CLAN_LEVEL(ch) < 10 &&
          CLAN_LEVEL(ch) < REMOVE_LVL(board_type))
      {
        sprintf(buf, "Your rank in %s#N is not sufficient to remove other people messages.\r\n",
                clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return 1;
      }
    }
  }

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected && d->str == &(msg_storage[MSG_SLOTNUM(board_type, ind)]))
    {
      send_to_char("At least wait until the author is finished before amending it!\r\n", ch);
      return 1;
    }

  if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    msg_flag.flag[0] = CLAN_LEVEL(ch);
  else
    msg_flag.flag[0] = GET_LEVEL(ch);

  msg_flag.flag[1] = MSG_EDITED;
  msg_flag.idnum = GET_IDNUM(ch);
  msg_flag.date  = time(0);

  if (!set_msg_flag(msg_flag, ch, board_type, ind))
    return 1;

  if (ind < num_of_msgs[board_type] - 1)
  {
    ptr       = MSG_HEADING(board_type, ind);
    head_len  = MSG_HEADING_LEN(board_type, ind);
    slot_num  = MSG_SLOTNUM(board_type, ind);
    level     = MSG_LEVEL(board_type, ind);

    while (ind < num_of_msgs[board_type] - 1)
    {
      ind++;
      MSG_HEADING(board_type, ind - 1)     = MSG_HEADING(board_type, ind);
      MSG_HEADING_LEN(board_type, ind - 1) = MSG_HEADING_LEN(board_type, ind);
      MSG_SLOTNUM(board_type, ind - 1)     = MSG_SLOTNUM(board_type, ind);
      MSG_LEVEL(board_type, ind - 1)       = MSG_LEVEL(board_type, ind);
    }

    MSG_HEADING(board_type, ind)     = ptr;
    MSG_HEADING_LEN(board_type, ind) = head_len;
    MSG_SLOTNUM(board_type, ind)     = slot_num;
    MSG_LEVEL(board_type, ind)       = level;
  }

  if (!IS_NPC(ch))
    SET_BIT(PLR_FLAGS(ch), PLR_WRITING);

  sprintf(buf, "$n starts amending message %d.", msg);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  send_to_char("Amend your message (/s saves /h for help).\r\n\r\n", ch);

  page_string(ch->desc, msg_storage[MSG_SLOTNUM(board_type, ind)], 1);

  send_to_char("\r\n", ch);

  ch->desc->str = &(msg_storage[MSG_SLOTNUM(board_type, ind)]);
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  return 1;
}


int Board_lock_msg(int board_type, struct char_data *ch, char *arg)
{
  int	ind, msg;
  struct descriptor_data *d;
  struct message_flag msg_flag;
  char number[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

  one_argument(arg, number);

  if (!*number || !isdigit(*number))
  {
    if (is_abbrev(number, "board"))
    {
      lock_board(board_type, ch);
      return 1;
    }
    send_to_char("You must specify a message number..\r\n", ch);
    return 1;
  }

  if (!(msg = atoi(number)))
  {
    send_to_char("You must specify a message number..\r\n", ch);
    return 1;
  }

  if (!num_of_msgs[board_type])
  {
    send_to_char("The board is empty!\r\n", ch);
    return 1;
  }

  if (msg < 1 || msg > num_of_msgs[board_type])
  {
    send_to_char("That message exists only in your imagination..\r\n", ch);
    return 1;
  }

  ind = num_of_msgs[board_type] - msg;
  if (!MSG_HEADING(board_type, ind))
  {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }

  if (BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("Unable to lock message, the board is locked.\r\n", ch);
    return 1;
  }

  if (GET_LEVEL(ch) != LEVEL_ADMIN)
  {
    if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    {
      if (CLAN(ch) == real_clan(BOARD_REF(board_type)))
      {
        if (CLAN_LEVEL(ch) < MSG_LEVEL(board_type, ind) &&
            CLAN_LEVEL(ch) < REMOVE_LVL(board_type))
        {
          sprintf(buf, "Your rank in %s#N is not sufficient to lock other peoples messages.\r\n",
                  clan_list[real_clan(BOARD_REF(board_type))].name);
          send_to_char(buf, ch);
          return 1;
        }
      }
      else
      {
          sprintf(buf, "You can't lock messages on a %s#N board.\r\n",
                  clan_list[real_clan(BOARD_REF(board_type))].name);
          send_to_char(buf, ch);
          return 1;
      }
    }
    else
      if (GET_LEVEL(ch) < LEVEL_ADMIN)
      {
        send_to_char("You aren't powerful enough to lock a message.\r\n",
ch);
        return 1;
      }
      else
        if (GET_LEVEL(ch) < MSG_LEVEL(board_type, ind))
        {
          send_to_char("You can't lock a message holier than yourself.\r\n", ch);
          return 1;
        }
  }

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected && d->str == &(msg_storage[MSG_SLOTNUM(board_type, ind)]))
    {
      send_to_char("At least wait until the author is finished locking it!\r\n", ch);
      return 1;
    }

  if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    msg_flag.flag[0] = CLAN_LEVEL(ch);
  else
    msg_flag.flag[0] = GET_LEVEL(ch);

  msg_flag.flag[1] = MSG_LOCKED;
  msg_flag.idnum = GET_IDNUM(ch);
  msg_flag.date  = time(0);

  if (!set_msg_flag(msg_flag, ch, board_type, ind))
    return 1;

  *(MSG_HEADING(board_type, ind)+14) = 'C';

  sprintf(buf, "$n locks message %d.", msg);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  sprintf(buf, "You lock message %d.\r\n", msg);
  send_to_char(buf, ch);

  return 1;
}


int Board_unlock_msg(int board_type, struct char_data *ch, char *arg)
{
  int	ind, msg;
  char number[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

  one_argument(arg, number);

  if (!*number || !isdigit(*number))
  {
    if (is_abbrev(number, "board"))
    {
      unlock_board(board_type, ch);
      return 1;
    }
    send_to_char("You must specify a message number..\r\n", ch);
    return 1;
  }

  if (!(msg = atoi(number)))
  {
    send_to_char("You must specify a message number..\r\n", ch);
    return 1;
  }

  if (!num_of_msgs[board_type])
  {
    send_to_char("The board is empty!\r\n", ch);
    return 1;
  }

  if (msg < 1 || msg > num_of_msgs[board_type])
  {
    send_to_char("That message exists only in your imagination..\r\n", ch);
    return 1;
  }

  ind = num_of_msgs[board_type] - msg;
  if (!MSG_HEADING(board_type, ind))
  {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }

  if (BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("Unable to unlock message, the board is locked.\r\n", ch);
    return 1;
  }

  if (!msg_locked(board_type, ind))
  {
    send_to_char("That message isnt even locked.\r\n", ch);
    return 1;
  }

  if (!unlock_msg(board_type, ind, ch))
    send_to_char("Can't unlock a message holier than yourself.\r\n", ch);
  else
  {
    *(MSG_HEADING(board_type, ind)+14) = 'c';

    sprintf(buf, "$n unlocks message %d.", msg);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    sprintf(buf, "You unlock message %d.\r\n", msg);
    send_to_char(buf, ch);
  }

  return 1;
}


int msg_locked(int board_type, int ind)
{
  int	num_flags, i;
  long *flags;
  short flag[2];

  num_flags = MSG_HEADING_LEN(board_type, ind) - (strlen(MSG_HEADING(board_type, ind)) + 1);
  num_flags /= sizeof(long) * 3;

  if (num_flags)
  {
    flags = (long *)(MSG_HEADING(board_type, ind) + strlen(MSG_HEADING(board_type, ind)) + 1);

    i = 0;
    while (i < num_flags)
    {
      memcpy(flag, flags, sizeof(long));
      if (flag[1] == MSG_LOCKED)
        return 1;
      else
        flags += 3;
      i++;
    }
  }

  return 0;
}


int unlock_msg(int board_type, int ind, struct char_data *ch)
{
  long *flags;
  struct message_flag msg_flag;

  flags = (long *)(MSG_HEADING(board_type, ind) + MSG_HEADING_LEN(board_type, ind));
  flags -= 3;

  memcpy (msg_flag.flag, flags, sizeof(long));
  flags++;
  msg_flag.idnum = *flags;
  flags++;
  msg_flag.date = *flags;

  if (GET_LEVEL(ch) < LEVEL_ADMIN)
  {
    if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    {
      if (CLAN(ch) != real_clan(BOARD_REF(board_type)))
        return 0;
      if (CLAN_LEVEL(ch) < msg_flag.flag[0])
        return 0;
    }
    else
      if (GET_LEVEL(ch) < get_level_idnum(msg_flag.idnum))
        return 0;
  }

  MSG_HEADING_LEN(board_type, ind) = MSG_HEADING_LEN(board_type, ind) - 3*sizeof(long);
  MSG_HEADING(board_type, ind) = (char *) realloc(MSG_HEADING(board_type, ind), MSG_HEADING_LEN(board_type, ind));

  return 1;
}


int set_msg_flag(struct message_flag msg_flag, struct char_data *ch, int board_type, int ind)
{
  int	num_flags, i, flag_found;
  long *flags;
  short flag[2];

  num_flags = MSG_HEADING_LEN(board_type, ind) - (strlen(MSG_HEADING(board_type, ind)) + 1);
  flags = (long *)(MSG_HEADING(board_type, ind) + strlen(MSG_HEADING(board_type, ind)) + 1);

  if (num_flags)
  {
    num_flags /= sizeof(long) * 3;

    i = 0;
    flag_found = 0;
    while (i < num_flags)
    {
      memcpy(flag, flags, sizeof(long));
      if (flag[1] == msg_flag.flag[1])
      {
        flags++;
        // Edit flag found
        if (msg_flag.flag[1] == MSG_EDITED)
        {
          if (*flags == msg_flag.idnum)
          {
            flags++;
            *flags = msg_flag.date;
            flag_found = 1;
          }
          else
            flags++;
        }
        // Locked flag found
        if (msg_flag.flag[1] == MSG_LOCKED)
        {
          if (*flags == msg_flag.idnum)
          {
            send_to_char("Message already locked.\r\n", ch);
            return 0;
          }
          else
          {
            if (BOARD_FLAGGED(board_type, BOARD_CLAN))
            {
              // Clan Board
              if (CLAN_LEVEL(ch) <= msg_flag.flag[0])
              {
                send_to_char("Message already locked.\r\n", ch);
                return 0;
              }
              else
              {
                flags--;
                memcpy(flags, msg_flag.flag, sizeof(long));
                flags++;
                *flags = msg_flag.idnum;
                flag_found = 1;
              }
            }
            else
            {
              // Regular Board
              if (GET_LEVEL(ch) <= msg_flag.flag[0])
              {
                send_to_char("Message already locked.\r\n", ch);
                return 0;
              }
              else
              {
                flags--;
                memcpy(flags, msg_flag.flag, sizeof(long));
                flags++;
                *flags = msg_flag.idnum;
                flag_found = 1;
              }
            }
          }
        }

        // Add more flags here...
      }
      else
        flags += 2;

      flags++;
      i++;
    }

    if (!flag_found)
    {
      // Add new flag
      num_flags++;

      MSG_HEADING_LEN(board_type, ind) = num_flags*sizeof(long)*3 + strlen(MSG_HEADING(board_type, ind)) + 1;
      MSG_HEADING(board_type, ind) = (char *) realloc(MSG_HEADING(board_type, ind), MSG_HEADING_LEN(board_type, ind));

      flags = (long *)(MSG_HEADING(board_type, ind) + strlen(MSG_HEADING(board_type, ind)) + 1);

      flags += (num_flags-1)*3;

      memcpy(flags, msg_flag.flag, sizeof(long));
      flags++;
      *flags = msg_flag.idnum;
      flags++;
      *flags = msg_flag.date;
      flags++;
    }
  }
  else
  {
    // There was no flags at all, lets add one
    MSG_HEADING_LEN(board_type, ind) = 3*sizeof(long) + strlen(MSG_HEADING(board_type, ind)) + 1;
    MSG_HEADING(board_type, ind) = (char *) realloc(MSG_HEADING(board_type, ind), MSG_HEADING_LEN(board_type, ind));

    flags = (long *)(MSG_HEADING(board_type, ind) + strlen(MSG_HEADING(board_type, ind)) + 1);

    memcpy(flags, msg_flag.flag, sizeof(long));
    flags++;
    *flags = msg_flag.idnum;
    flags++;
    *flags = msg_flag.date;
    flags++;
  }

  return 1;
}


int lock_board(int board_type, struct char_data *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("The board is already locked.\r\n", ch);
    return 0;
  }

  if (GET_LEVEL(ch) < LEVEL_ADMIN)
  {
    if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    {
      if (CLAN(ch) != real_clan(BOARD_REF(board_type)))
      {
        sprintf(buf, "You can't lock a %s#N board, you arent even in the clan.\r\n",
                  clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return 0;
      }
      else
        if (CLAN_LEVEL(ch) < REMOVE_LVL(board_type))
        {
          sprintf(buf, "Your rank in %s#N is not sufficient to lock the clan board.\r\n",
                  clan_list[real_clan(BOARD_REF(board_type))].name);
          send_to_char(buf, ch);
          return 0;
        }
    }
    else
    {
      send_to_char("You arent godly enough to lock a board.\r\n", ch);
      return 0;
    }
  }

  BOARD_FLAG(board_type, BOARD_LOCKED);

  send_to_char("You lock the board.\r\n", ch);

  sprintf(buf, "$n locks the board.\r\n");
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  return 1;
}


int unlock_board(int board_type, struct char_data *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!BOARD_FLAGGED(board_type, BOARD_LOCKED))
  {
    send_to_char("Board isnt even locked.\r\n", ch);
    return 0;
  }

  if (GET_LEVEL(ch) < LEVEL_ADMIN)
  {
    if (BOARD_FLAGGED(board_type, BOARD_CLAN))
    {
      if (CLAN(ch) != real_clan(BOARD_REF(board_type)))
      {
        sprintf(buf, "You can't unlock a %s#N board, you arent even in the clan.\r\n",
                  clan_list[real_clan(BOARD_REF(board_type))].name);
        send_to_char(buf, ch);
        return 0;
      }
      else
        if (CLAN_LEVEL(ch) < REMOVE_LVL(board_type))
        {
          sprintf(buf, "Your rank in %s#N is not sufficient to unlock the clan board.\r\n",
                  clan_list[real_clan(BOARD_REF(board_type))].name);
          send_to_char(buf, ch);
          return 0;
        }
    }
    else
    {
      send_to_char("You arent godly enough to unlock a board.\r\n", ch);
      return 0;
    }
  }

  BOARD_UNFLAG(board_type, BOARD_LOCKED);

  send_to_char("You unlock the board.\r\n", ch);

  sprintf(buf, "$n unlocks the board.\r\n");
  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  return 1;
}


void	Board_save_board(int board_type)
{
  FILE * fl;
  int	i;
  char	*tmp1 = 0, *tmp2 = 0;

  if (!num_of_msgs[board_type])
  {
    unlink(FILENAME(board_type));
    return;
  }

  if (!(fl = fopen(FILENAME(board_type), "wb")))
  {
    perror("Error writing board");
    return;
  }

  fwrite(&(num_of_msgs[board_type]), sizeof(int), 1, fl);

  for (i = 0; i < num_of_msgs[board_type]; i++)
  {
    // heading_len assigned in write, used to keep track of
    // flags after the heading do NOT change this unless you
    // know what you are doing
    if (!(tmp1 = MSG_HEADING(board_type, i)))
      msg_index[board_type][i].heading_len = 0;

    if (MSG_SLOTNUM(board_type, i) < 0 ||
	      MSG_SLOTNUM(board_type, i) >= INDEX_SIZE ||
	   (!(tmp2 = msg_storage[MSG_SLOTNUM(board_type, i)])))
      msg_index[board_type][i].message_len = 0;
    else
      msg_index[board_type][i].message_len = strlen(tmp2) + 1;

    fwrite(&(msg_index[board_type][i]), sizeof(struct board_msginfo ), 1, fl);
    if (tmp1)
      fwrite(tmp1, sizeof(char), msg_index[board_type][i].heading_len, fl);
    if (tmp2)
      fwrite(tmp2, sizeof(char), msg_index[board_type][i].message_len, fl);
  }

  fclose(fl);
}


void	Board_load_board(int board_type)
{
  FILE * fl;
  int	i, len1 = 0, len2 = 0;
  char	*tmp1 = 0, *tmp2 = 0;


  if (!(fl = fopen(FILENAME(board_type), "rb")))
  {
    perror("Error reading board");
    return;
  }

  fread(&(num_of_msgs[board_type]), sizeof(int), 1, fl);
  if (num_of_msgs[board_type] < 1 || num_of_msgs[board_type] > MAX_BOARD_MESSAGES)
  {
    log("SYSERR: Board file corrupt.  Resetting.");
    Board_reset_board(board_type);
    return;
  }

  for (i = 0; i < num_of_msgs[board_type]; i++)
  {
    fread(&(msg_index[board_type][i]), sizeof(struct board_msginfo ), 1, fl);
    if (!(len1 = msg_index[board_type][i].heading_len))
    {
      log("SYSERR: Board file corrupt!  Resetting.");
      Board_reset_board(board_type);
      return;
    }

    if (!(tmp1 = (char *)malloc(sizeof(char) *len1)))
    {
      log("SYSERR: Error - malloc failed for board header");
      exit(1);
    }

    fread(tmp1, sizeof(char), len1, fl);
    MSG_HEADING(board_type, i) = tmp1;

    if ((len2 = msg_index[board_type][i].message_len))
    {
      if ((MSG_SLOTNUM(board_type, i) = find_slot()) == -1)
      {
	      log("SYSERR: Out of slots booting board!  Resetting..");
	      Board_reset_board(board_type);
	      return;
      }
      if (!(tmp2 = (char *)malloc(sizeof(char) *len2)))
      {
	      log("SYSERR: malloc failed for board text");
	      exit(1);
      }
      fread(tmp2, sizeof(char), len2, fl);
      msg_storage[MSG_SLOTNUM(board_type, i)] = tmp2;
    }
  }

  fclose(fl);
}


void	Board_reset_board(int board_type)
{
  int	i;

  for (i = 0; i < MAX_BOARD_MESSAGES; i++)
  {
    if (MSG_HEADING(board_type, i))
      free (MSG_HEADING(board_type, i));
    if (msg_storage[MSG_SLOTNUM(board_type, i)])
      free (msg_storage[MSG_SLOTNUM(board_type, i)]);
    msg_storage_taken[MSG_SLOTNUM(board_type, i)] = 0;
    memset(&(msg_index[board_type][i]), 0, sizeof(struct board_msginfo ));
    msg_index[board_type][i].slot_num = -1;
  }
  num_of_msgs[board_type] = 0;
  unlink(FILENAME(board_type));
}

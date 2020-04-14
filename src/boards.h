/* ************************************************************************
*   File: boards.h                                      Part of CircleMUD *
*  Usage: header file for bulletin boards                                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define NUM_OF_BOARDS		 0
#define MAX_BOARD_MESSAGES 	200     /* arbitrary */
#define MAX_MESSAGE_LENGTH     4096	 /* arbitrary */

#define INDEX_SIZE	   ((NUM_OF_BOARDS*MAX_BOARD_MESSAGES) + 5)


struct board_msginfo {
   int	slot_num;     /* pos of message in "master index" */
   char	*heading;     /* pointer to message's heading */
   int	level;        /* level of poster */
   int	heading_len;  /* size of header (for file write) */
   int	message_len;  /* size of message text (for file write) */
};

struct board_info_type {
   long	vnum;	           /* vnum of this board */
   int  board_flags;       /* board flags */
   int  reference;         /* vnum of clan for CLAN possible uses for other Types */
   int	read_lvl;	   /* min level to read messages on this board */
   int	write_lvl;         /* min level to write messages on this board */
   int	remove_lvl;        /* min level to remove messages from this board */
   char	filename[50];      /* file to save this board to */
   int	rnum;	           /* rnum of this board */
};

/* For board_flags */
#define BOARD_CLAN     (1 << 0)  /* Clan board, clanlevel 10 can remove */
#define BOARD_REMORT   (1 << 1)  /* Remort board, remort gods can remove */
#define BOARD_CODE     (1 << 2)  /* Code board, code gods can remove */
#define BOARD_LOCKED   (1 << 3)  /* Board is locked only for admin+ or remove for cb */

const char *board_flag_types[] = {
  "CLAN",
  "REMORT",
  "CODE",
  "LOCKED",
  "\n"
};

/* Message flags */
/* Flags are stored after the header as header flag flag etc.
   each flag is 3*sizeof(long) bytes. heading_len is used to
   calculate how many flags there are like this heading_len -
   strlen(header) - 1. so do not edit the header_len or you will
   lose all flags in the message or at the very least screw them
   up. NEVER add a flag after a MSG_LOCKED flag since when it unlocks
   the message it simply cuts out the last flag with realloc().
   What else, uhm yeah, read the code before you edit anything :P */
#define MSG_EDITED  (1 << 0)
#define MSG_LOCKED  (1 << 1)
#define MSG_STICKY  (1 << 2)

/* Message flag struct */
struct message_flag
{
  short flag[2];
  long idnum;
  long date;
};

#define VNUM(i) (board_info[i].vnum)
#define READ_LVL(i) (board_info[i].read_lvl)
#define WRITE_LVL(i) (board_info[i].write_lvl)
#define REMOVE_LVL(i) (board_info[i].remove_lvl)
#define FILENAME(i) (board_info[i].filename)
#define RNUM(i) (board_info[i].rnum)
#define BOARD_FLAGS(i) (board_info[i].board_flags)
#define BOARD_FLAGGED(i, flag) IS_SET(BOARD_FLAGS(i), flag)
#define BOARD_FLAG(i, flag) SET_BIT(BOARD_FLAGS(i), flag)
#define BOARD_UNFLAG(i, flag) REMOVE_BIT(BOARD_FLAGS(i), flag)
#define BOARD_REF(i) (board_info[i].reference)

#define NEW_MSG_INDEX(i) (msg_index[i][num_of_msgs[i]])
#define MSG_HEADING(i, j) (msg_index[i][j].heading)
#define MSG_HEADING_LEN(i, j) (msg_index[i][j].heading_len)
#define MSG_SLOTNUM(i, j) (msg_index[i][j].slot_num)
#define MSG_LEVEL(i, j) (msg_index[i][j].level)

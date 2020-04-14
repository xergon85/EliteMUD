/* ************************************************************************
*   File: mail.h                                        Part of EliteMUD  *
*  Usage: header file for mail system                                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

/******* MUD MAIL SYSTEM HEADER FILE ****************************
 ***     written by Jeremy Elson (jelson@server.cs.jhu.edu)   ***
 ****   compliments of CircleMUD (circle.cs.jhu.edu 4000)    ****
 ***************************************************************/

/* INSTALLATION INSTRUCTIONS in MAIL.C */

/* minimum level a player must be to send mail	*/
#define MIN_MAIL_LEVEL 2
#define MIN_POSTCARD_LEVEL 2

/* # of gold coins required to send mail	*/
#define STAMP_PRICE 350
#define POSTCARD_PRICE 600

/* Maximum size of mail in bytes (arbitrary)	*/
#define MAX_MAIL_SIZE 4000

/* Max size of player names			*/
#define NAME_SIZE  15

/* Path of the directory where you store the pictures */
#define POSTCARD_DIRECTORY "postcards"

/* Maximum size of postcard picture in bytes */
#define MAX_PICTURE_SIZE 4096

/* size of mail file allocation blocks		*/
#define BLOCK_SIZE 100

/* NOTE:  Make sure that your block size is big enough -- if not,
   HEADER_BLOCK_DATASIZE will end up negative.  This is a bad thing.
   Check the define below to make sure it is >0 when choosing values
   for NAME_SIZE and BLOCK_SIZE.  100 is a nice round number for
   BLOCK_SIZE and is the default ... why bother trying to change it
   anyway?

   The mail system will always allocate disk space in chunks of size
   BLOCK_SIZE.
*/

/* USER CHANGABLE DEFINES ABOVE **
***************************************************************************
**   DON'T TOUCH DEFINES BELOW  *
**   Yeah - Don't touch DEFINES BELOW -Petrus */

int	scan_file(void);
int	has_mail(char *recipient);
void	store_mail(char *to, char *from, char *message_pointer);
char	*read_delete(char *recipient, char *recipient_formatted);

#define DATA_BLOCK    1
#define LAST_BLOCK    0

struct mail_block_data {
   char	block_type;  	            /* 1 if data block, 0 if last data block */
   char	txt[BLOCK_SIZE+1];          /* the actual text		 */
};

typedef struct mail_block_data mail_block_data;

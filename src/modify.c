/* ************************************************************************
*   File: modify.c                                      Part of EliteMUD  *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "mail.h"
#include "screen.h"
#include "spells.h"
#include "functions.h"
#include "scrcol.h"

#define REBOOT_AT    8  /* 0-23, time of optional reboot if -e lib/reboot */


#define TP_MOB    0
#define TP_OBJ	  1
#define TP_ERROR  2

const struct scrcol_replace_data *scr_get_colcode(const char *str);

extern char *spells[];
extern char *skills[];

/* action modes for parse_action */
#define PARSE_FORMAT		0 
#define PARSE_REPLACE		1 
#define PARSE_HELP		2 
#define PARSE_DELETE		3
#define PARSE_INSERT		4
#define PARSE_LIST_NORM		5
#define PARSE_LIST_NUM		6
#define PARSE_EDIT		7


/*  handle some editor commands */
void parse_action(int command, char *string, struct descriptor_data *d) {
   int indent = 0, rep_all = 0, flags = 0, total_len, replaced;
   register int j = 0;
   int i, line_low, line_high;
   char *s, *t, temp;
   
   switch (command) { 
    case PARSE_HELP: 
      sprintf(buf, 
	      "Editor command formats: /<letter>\r\r\n\n"
	      "/a         -  aborts editor\r\n"
	      "/c         -  clears buffer\r\n"
	      "/d#        -  deletes a line #\r\n"
	      "/e# <text> -  changes the line at # with <text>\r\n"
	      "/f         -  formats text\r\n"
	      "/fi        -  indented formatting of text\r\n"
	      "/h         -  list text editor commands\r\n"
	      "/i# <text> -  inserts <text> before line #\r\n"
	      "/l         -  lists buffer\r\n"
	      "/n         -  lists buffer with line numbers\r\n"
	      "/r 'a' 'b' -  replace 1st occurance of text <a> in buffer with text <b>\r\n"
	      "/ra 'a' 'b'-  replace all occurances of text <a> within buffer with text <b>\r\n"
	      "              usage: /r[a] 'pattern' 'replacement'\r\n"
	      "/s         -  saves text\r\n");
      SEND_TO_Q(buf, d);
      break;
    case PARSE_FORMAT: 
      while (isalpha(string[j]) && j < 2) {
	 switch (string[j]) {
	  case 'i':
	    if (!indent) {
	       indent = 1;
	       flags += FORMAT_INDENT;
	    }             
	    break;
	  default:
	    break;
	 }     
	 j++;
      }
      format_text(d->str, flags, d, d->max_str);
      sprintf(buf, "Text formarted with%s indent.\r\n", (indent ? "" : "out")); 
      SEND_TO_Q(buf, d);
      break;    
    case PARSE_REPLACE: 
      while (isalpha(string[j]) && j < 2) {
	 switch (string[j]) {
	  case 'a':
	    if (!indent) {
	       rep_all = 1;
	    }             
	    break;
	  default:
	    break;
	 }     
	 j++;
      }
      s = strtok(string, "'");
      if (s == NULL) {
	 SEND_TO_Q("Invalid format.\r\n", d);
	 return;
      }
      s = strtok(NULL, "'");
      if (s == NULL) {
	 SEND_TO_Q("Target string must be enclosed in single quotes.\r\n", d);
	 return;
      }
      t = strtok(NULL, "'");
      if (t == NULL) {
	 SEND_TO_Q("No replacement string.\r\n", d);
	 return;
      }
      t = strtok(NULL, "'");
      if (t == NULL) {
	 SEND_TO_Q("Replacement string must be enclosed in single quotes.\r\n", d);
	 return;
      }
      total_len = ((strlen(t) - strlen(s)) + strlen(*d->str));
      if (total_len <= d->max_str) {
	 if ((replaced = replace_str(d->str, s, t, rep_all, d->max_str)) > 0) {
	    sprintf(buf, "Replaced %d occurance%sof '%s' with '%s'.\r\n", replaced, ((replaced != 1)?"s ":" "), s, t); 
	    SEND_TO_Q(buf, d);
	 }
	 else if (replaced == 0) {
	    sprintf(buf, "String '%s' not found.\r\n", s); 
	    SEND_TO_Q(buf, d);
	 }
	 else {
	    SEND_TO_Q("ERROR: Replacement string causes buffer overflow, aborted replace.\r\n", d);
	 }
      }
      else
	SEND_TO_Q("Not enough space left in buffer.\r\n", d);
      break;
    case PARSE_DELETE:
      switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
       case 0:
	 SEND_TO_Q("You must specify a line number or range to delete.\r\n", d);
	 return;
       case 1:
	 line_high = line_low;
	 break;
       case 2:
	 if (line_high < line_low) {
	    SEND_TO_Q("That range is invalid.\r\n", d);
	    return;
	 }
	 break;
      }
      
      i = 1;
      total_len = 1;
      if ((s = *d->str) == NULL) {
	 SEND_TO_Q("Buffer is empty.\r\n", d);
	 return;
      }
      if (line_low > 0) {
      	 while (s && (i < line_low))
	   if ((s = strchr(s, '\n')) != NULL) {
	      i++;
	      s++;
	   }
	 if ((i < line_low) || (s == NULL)) {
	    SEND_TO_Q("Line(s) out of range; not deleting.\r\n", d);
	    return;
	 }
	 
	 t = s;
	 while (s && (i < line_high))
	   if ((s = strchr(s, '\n')) != NULL) {
	      i++;
	      total_len++;
	      s++;
	   }
	 if ((s) && ((s = strchr(s, '\n')) != NULL)) {
	    s++;
	    while (*s != '\0') *(t++) = *(s++);
	 }
	 else total_len--;
	 *t = '\0';
	 RECREATE(*d->str, char, strlen(*d->str) + 3);
	 sprintf(buf, "%d line%sdeleted.\r\n", total_len,
		 ((total_len != 1)?"s ":" "));
	 SEND_TO_Q(buf, d);
      }
      else {
	 SEND_TO_Q("Invalid line numbers to delete must be higher than 0.\r\n", d);
	 return;
      }
      break;
    case PARSE_LIST_NORM:
      /* note: my buf,buf1,buf2 vars are defined at 32k sizes so they
       * are prolly ok fer what i want to do here. */
      *buf = '\0';
      if (*string != '\0')
	switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
	 case 0:
	   line_low = 1;
	   line_high = 999999;
	   break;
	 case 1:
	   line_high = line_low;
	   break;
	}
      else {
	 line_low = 1;
	 line_high = 999999;
      }
      
      if (line_low < 1) {
	 SEND_TO_Q("Line numbers must be greater than 0.\r\n", d);
	 return;
      }
      if (line_high < line_low) {
	 SEND_TO_Q("That range is invalid.\r\n", d);
	 return;
      }
      *buf = '\0';
      if ((line_high < 999999) || (line_low > 1)) {
	 sprintf(buf, "Current buffer range [%d - %d]:\r\n", line_low, line_high);
      }
      i = 1;
      total_len = 0;
      s = *d->str;
      while (s && (i < line_low))
	if ((s = strchr(s, '\n')) != NULL) {
	   i++;
	   s++;
	}
      if ((i < line_low) || (s == NULL)) {
	 SEND_TO_Q("Line(s) out of range; no buffer listing.\r\n", d);
	 return;
      }
      
      t = s;
      while (s && (i <= line_high))
	if ((s = strchr(s, '\n')) != NULL) {
	   i++;
	   total_len++;
	   s++;
	}
      if (s)	{
	 temp = *s;
	 *s = '\0';
	 strcat(buf, t);
	 *s = temp;
      }
      else strcat(buf, t);
      /* this is kind of annoying.. will have to take a poll and see..
      sprintf(buf, "%s\r\n%d line%sshown.\r\n", buf, total_len,
	      ((total_len != 1)?"s ":" "));
       */
      page_string(d, buf, TRUE);
      break;
    case PARSE_LIST_NUM:
      /* note: my buf,buf1,buf2 vars are defined at 32k sizes so they
       * are prolly ok fer what i want to do here. */
      *buf = '\0';
      if (*string != '\0')
	switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
	 case 0:
	   line_low = 1;
	   line_high = 999999;
	   break;
	 case 1:
	   line_high = line_low;
	   break;
	}
      else {
	 line_low = 1;
	 line_high = 999999;
      }
      
      if (line_low < 1) {
	 SEND_TO_Q("Line numbers must be greater than 0.\r\n", d);
	 return;
      }
      if (line_high < line_low) {
	 SEND_TO_Q("That range is invalid.\r\n", d);
	 return;
      }
      *buf = '\0';
      i = 1;
      total_len = 0;
      s = *d->str;
      while (s && (i < line_low))
	if ((s = strchr(s, '\n')) != NULL) {
	   i++;
	   s++;
	}
      if ((i < line_low) || (s == NULL)) {
	 SEND_TO_Q("Line(s) out of range; no buffer listing.\r\n", d);
	 return;
      }
      
      t = s;
      while (s && (i <= line_high))
	if ((s = strchr(s, '\n')) != NULL) {
	   i++;
	   total_len++;
	   s++;
	   temp = *s;
	   *s = '\0';
	   sprintf(buf, "%s%4d:\r\n", buf, (i-1));
	   strcat(buf, t);
	   *s = temp;
	   t = s;
	}
      if (s && t) {
	 temp = *s;
	 *s = '\0';
	 strcat(buf, t);
	 *s = temp;
      }
      else if (t) strcat(buf, t);
      /* this is kind of annoying .. seeing as the lines are #ed
      sprintf(buf, "%s\r\n%d numbered line%slisted.\r\n", buf, total_len,
	      ((total_len != 1)?"s ":" "));
       */
      page_string(d, buf, TRUE);
      break;

    case PARSE_INSERT:
      half_chop(string, buf, buf2);
      if (*buf == '\0') {
	 SEND_TO_Q("You must specify a line number before which to insert text.\r\n", d);
	 return;
      }
      line_low = atoi(buf);
      strcat(buf2, "\r\n");
      
      i = 1;
      *buf = '\0';
      if ((s = *d->str) == NULL) {
	 SEND_TO_Q("Buffer is empty, nowhere to insert.\r\n", d);
	 return;
      }
      if (line_low > 0) {
      	 while (s && (i < line_low))
	   if ((s = strchr(s, '\n')) != NULL) {
	      i++;
	      s++;
	   }
	 if ((i < line_low) || (s == NULL)) {
	    SEND_TO_Q("Line number out of range; insert aborted.\r\n", d);
	    return;
	 }
	 temp = *s;
	 *s = '\0';
	 if ((strlen(*d->str) + strlen(buf2) + strlen(s+1) + 3) > d->max_str) {
	    *s = temp;
	    SEND_TO_Q("Insert text pushes buffer over maximum size, insert aborted.\r\n", d);
	    return;
	 }
	 if (*d->str && (**d->str != '\0')) strcat(buf, *d->str);
	 *s = temp;
	 strcat(buf, buf2);
	 if (s && (*s != '\0')) strcat(buf, s);
	 RECREATE(*d->str, char, strlen(buf) + 3);
	 strcpy(*d->str, buf);
	 SEND_TO_Q("Line inserted.\r\n", d);
      }
      else {
	 SEND_TO_Q("Line number must be higher than 0.\r\n", d);
	 return;
      }
      break;

    case PARSE_EDIT:
      half_chop(string, buf, buf2);
      if (*buf == '\0') {
	 SEND_TO_Q("You must specify a line number at which to change text.\r\n", d);
	 return;
      }
      line_low = atoi(buf);
      strcat(buf2, "\r\n");
      
      i = 1;
      *buf = '\0';
      if ((s = *d->str) == NULL) {
	 SEND_TO_Q("Buffer is empty, nothing to change.\r\n", d);
	 return;
      }
      if (line_low > 0) {
	 /* loop through the text counting /n chars till we get to the line */
      	 while (s && (i < line_low))
	   if ((s = strchr(s, '\n')) != NULL) {
	      i++;
	      s++;
	   }
	 /* make sure that there was a THAT line in the text */
	 if ((i < line_low) || (s == NULL)) {
	    SEND_TO_Q("Line number out of range; change aborted.\r\n", d);
	    return;
	 }
	 /* if s is the same as *d->str that means im at the beginning of the
	  * message text and i dont need to put that into the changed buffer */
	 if (s != *d->str) {
	    /* first things first .. we get this part into buf. */
	    temp = *s;
	    *s = '\0';
	    /* put the first 'good' half of the text into storage */
	    strcat(buf, *d->str);
	    *s = temp;
	 }
	 /* put the new 'good' line into place. */
	 strcat(buf, buf2);
	 if ((s = strchr(s, '\n')) != NULL) {
	    /* this means that we are at the END of the line we want outta there. */
	    /* BUT we want s to point to the beginning of the line AFTER
	     * the line we want edited */
	    s++;
	    /* now put the last 'good' half of buffer into storage */
	    strcat(buf, s);
	 }
	 /* check for buffer overflow */
	 if (strlen(buf) > d->max_str) {
	    SEND_TO_Q("Change causes new length to exceed buffer maximum size, aborted.\r\n", d);
	    return;
	 }
	 /* change the size of the REAL buffer to fit the new text */
	 RECREATE(*d->str, char, strlen(buf) + 3);
	 strcpy(*d->str, buf);
	 SEND_TO_Q("Line changed.\r\n", d);
      }
      else {
	 SEND_TO_Q("Line number must be higher than 0.\r\n", d);
	 return;
      }
      break;
    default:
      SEND_TO_Q("Invalid option.\r\n", d);
      mudlog("SYSERR: invalid command passed to parse_action", BRF, LEVEL_ADMIN, TRUE);
      return;
   }
}



/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void	string_add(struct descriptor_data *d, char *str)
{
  int terminator = 0, action = 0;
  register int i = 2, j = 0;
  char actions[MAX_INPUT_LENGTH];
  extern char	*MENU;
  FILE *fl;

  delete_doubledollar(str);

   if ((action = (*str == '/'))) {
      while (str[i] != '\0') {
	 actions[j] = str[i];              
	 i++;
	 j++;
      }
      actions[j] = '\0';
    *str = '\0';
      switch (str[1]) {
       case 'a':
	 terminator = 2; /* working on an abort message */
	 break;
       case 'c':
	 if (*(d->str)) {
	    free(*d->str);
	    *(d->str) = NULL;
	    SEND_TO_Q("Current buffer cleared.\r\n", d);
	 }
	 else
	   SEND_TO_Q("Current buffer empty.\r\n", d);
	 break;
       case 'd':
	 parse_action(PARSE_DELETE, actions, d);
	 break;
       case 'e':
	 parse_action(PARSE_EDIT, actions, d);
	 break;
       case 'f': 
	 if (*(d->str))
	   parse_action(PARSE_FORMAT, actions, d);
	 else
	   SEND_TO_Q("Current buffer empty.\r\n", d);
	 break;
       case 'i':
	 if (*(d->str))
	   parse_action(PARSE_INSERT, actions, d);
	 else
	   SEND_TO_Q("Current buffer empty.\r\n", d);
	 break;
       case 'h': 
	 parse_action(PARSE_HELP, actions, d);
	 break;
       case 'l':
	 if (*d->str)
	   parse_action(PARSE_LIST_NORM, actions, d);
	 else SEND_TO_Q("Current buffer empty.\r\n", d);
	 break;
       case 'n':
	 if (*d->str)
	   parse_action(PARSE_LIST_NUM, actions, d);
	 else SEND_TO_Q("Current buffer empty.\r\n", d);
	 break;
       case 'r':   
	 parse_action(PARSE_REPLACE, actions, d);
	 break;
       case 's':
	 terminator = 1;
    *str = '\0';
	 break;
       default:
	 SEND_TO_Q("Invalid option.\r\n", d);
	 break;
      }
   }

  if (!(*d->str)) {
    if (strlen(str) > d->max_str) {
      send_to_char("String too long - Truncated.\r\n",
		   d->character);
      *(str + d->max_str) = '\0';
    }
    CREATE(*d->str, char, strlen(str) + 5);
    strcpy(*d->str, str);
  } else {
    if (strlen(str) + strlen(*d->str) > d->max_str) {
      send_to_char("String too long. Last line skipped.\r\n",
		   d->character);
      action = 1;
    } else {
      
      if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) + 
				       strlen(str) + 5))) {
	perror("string_add");
	exit(1);
      }
      strcat(*d->str, str);
    }
  }
  
  if (terminator) {
    strcat(*d->str, "§N");
    
    /* here we check for the abort option and reset the pointers */
    if ((terminator == 2) && (STATE(d) == CON_EXDSCR ||
                              STATE(d) == CON_TEXTED)) {
      free(*d->str);
      if (d->backstr) {
	*d->str = d->backstr;
      }
      else *d->str = NULL;
      d->backstr = NULL;
      d->str = NULL;
    }
    else if ((d->str) && (*d->str) && (**d->str == '\0')) {
      free(*d->str);
      *d->str = NULL;
    }
    
    if (!d->connected && (PLR_FLAGGED(d->character, PLR_MAILING))) {
      if ((terminator == 1) && *d->str) {
	store_mail(d->name, d->character->player.name, *d->str);
	SEND_TO_Q("Message sent!\r\n", d);
      }
      else SEND_TO_Q("Mail aborted.\r\n", d);
      d->name = 0;
      free(*d->str);
      free(d->str);
    }
    else if (!d->connected && (PLR_FLAGGED(d->character, PLR_WRITING))) {
      if (terminator == 2)
	SEND_TO_Q("Post not aborted, use REMOVE <Post #>.\r\n", d);
      
      d->name = 0;
    }
    else if (STATE(d) == CON_EXDSCR) {
      if (terminator != 1) SEND_TO_Q("Description aborted.\r\n", d);
      SEND_TO_Q(MENU, d);
      d->connected = CON_SLCT;
    }
    else if (STATE(d) == CON_TEXTED) {
      if (terminator == 1) {
        if (!(fl = fopen((char *)d->storage, "w"))) {
           sprintf(buf, "SYSERR: Can't write file '%s'.", d->storage);
           mudlog(buf, NRM, MIN(GET_LEVEL(d->character), LEVEL_GREATER), TRUE);
        }
        else {
           if (*d->str) {
              fputs(stripcr(buf1, *d->str), fl);
           }
           fclose(fl);
           sprintf(buf, "TEDIT: %s saves '%s'.", GET_NAME(d->character),
                   d->storage);
           mudlog(buf, NRM, MIN(GET_LEVEL(d->character), LEVEL_GREATER), TRUE);
           SEND_TO_Q("Saved.\r\n", d);
        }
      }
      else SEND_TO_Q("Edit aborted.\r\n", d);
      act("$n stops editing some scrolls.", TRUE, d->character, 0, 0, TO_ROOM);
      free(d->storage);
      d->storage = NULL;
      STATE(d) = CON_PLYNG;
    }
    else if (!d->connected && d->character && !IS_NPC(d->character)) {
      if (terminator == 1) {
	if (strlen(*d->str) == 0) {
	  free(*d->str);
	  *d->str = NULL;
	}
	else if ((d->str) && (*d->str) && (**d->str == '\0')) {
	     free(*d->str);
	     *d->str = NULL;
	}
      }
      else {
	free(*d->str);
	if (d->backstr) {
	  *d->str = d->backstr;
	}
	else *d->str = NULL;
	d->backstr = NULL;
	SEND_TO_Q("Message aborted.\r\n", d);
      }
    }
    
    if (OCSMODE(d->character) == OCS_ROOM_GET_DESC) { 
      print_room(d->character, (struct room_data*)(OCS1(d->character)));
      OCSMODE(d->character) = OCS_ROOM_MAIN;
    }
    if (OCSMODE(d->character) == OCS_EXIT_GET_DESC) {
      print_exit(d->character, (struct room_direction_data*)OCS1(d->character));
      OCSMODE(d->character) = OCS_EXIT_MAIN;
    }
    if (OCSMODE(d->character) == OCS_ROOM_GET_EDESC) {
      print_extra_desc(d->character, &(((struct room_data*)(OCS1(d->character)))->ex_description));
      OCSMODE(d->character) = OCS_ROOM_GET_EMAIN;
    }
    
    if (STATE(d) == CON_PLYNG && PRF_FLAGGED(d->character, PRF_DISPANSI)) {
      sprintf(buf, VTCURPOS VTDELEOS "\0337", GET_SCRLEN(d->character), 1);
      write_to_descriptor(d->descriptor, buf);
    }

    if (d->character && !IS_NPC(d->character)) {
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING | PLR_MAILING);
      act("$n has finished writing a message.", TRUE, d->character, 0, 0, TO_ROOM);
    }
    if (d->backstr) free(d->backstr);
    d->backstr = NULL;
    d->str = NULL;
  }
  else if (!action) strcat(*d->str, "\r\n");
}


ACMD(do_wizname)
{
  struct char_data *victim;
  
  for (; isspace(*argument); argument++);
  
  if(IS_NPC(ch))
    return;
  
  if(GET_LEVEL(ch) < LEVEL_DEITY) {
	skip_spaces(&argument);
  	delete_doubledollar(argument);
        sprintf(buf, argument);
        sprintf(arg, "%s", GET_NAME(ch));
  } else {
	half_chop(argument, arg, buf);
  }

  if (!*arg) {
    if (subcmd == SCMD_WIZNAME)
      send_to_char("Wizname who?\r\n", ch);
    if (subcmd == SCMD_PRENAME)
      send_to_char("Prename who?\r\n", ch);
    return;
  }
  
 
  if (!(victim = get_char_vis(ch, arg))) {
    send_to_char("No such person around.\r\n", ch);
    return;
  }

  if((GET_LEVEL(ch) < LEVEL_DEITY) &&  !(SELF(ch, victim))) {
    if (subcmd == SCMD_WIZNAME)
      send_to_char("You can only set your own wizname at your level.\r\n", ch);
    if (subcmd == SCMD_PRENAME)
      send_to_char("You can only set your own prename at your level.\r\n", ch);
    return;
  }

  if((GET_LEVEL(ch) < LEVEL_DEITY) && (subcmd == SCMD_WIZNAME) && (REMORT(ch) < 3)) {
       send_to_char("You must be at least a third remort before you can set your own wizname.\r\n", ch);
       return;
  }

  if((GET_LEVEL(ch) < 100) && (subcmd == SCMD_PRENAME) && (REMORT(ch) < 1)) {
       send_to_char("You must be at least level 100 or a remort before you can set your own prename.\r\n", ch);
       return;
  }

  if(SELF(ch, victim) && PLR_FLAGGED(ch, PLR_NOTITLE)) {
    send_to_char("You can't prename or wizname yourself -- you shouldn't have abused it!\r\n", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("Don't mess with the mobs!\r\n", ch);
    return;
  }
  
  if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
    send_to_char("Get a little higher first.\r\n", ch);
    return;
  }
  
  if ((subcmd == SCMD_WIZNAME && strchr(buf, '#'))) {
    send_to_char("Wiznames are automatically given color.\r\n", ch);
    return;
  }

  if (strstr(buf, "%")) {
    if (subcmd == SCMD_WIZNAME)
      send_to_char("Wiznames can't contain the % character.\r\n", ch);
    if (subcmd == SCMD_PRENAME)
      send_to_char("Prenames can't contain the % character.\r\n", ch);
    return;
  }

  if ((subcmd == SCMD_WIZNAME && strlen(buf) > 8) || 
      (subcmd == SCMD_PRENAME && strlen(buf) > 25)) {
    send_to_char("String too long - truncated.\r\n", ch);
    if (subcmd == SCMD_WIZNAME)
      *(buf + 8) = '\0';
    if (subcmd == SCMD_PRENAME)
      *(buf + 25) = '\0';
  }
  
  if (GET_WIZNAME(victim) && subcmd == SCMD_WIZNAME)
    free(GET_WIZNAME(victim));
  
  if (GET_PRENAME(victim) && subcmd == SCMD_PRENAME)
    free(GET_PRENAME(victim));

  if (!*buf) {
    if (subcmd == SCMD_WIZNAME)
      GET_WIZNAME(victim) = 0;
    if (subcmd == SCMD_PRENAME)
      GET_PRENAME(victim) = 0;
  } else {
    if (subcmd == SCMD_WIZNAME) {
      CREATE(GET_WIZNAME(victim), char, 9);
      strcpy(GET_WIZNAME(victim), buf);
    }
    if (subcmd == SCMD_PRENAME) {
      CREATE(GET_PRENAME(victim), char, 28);
      strcat(buf, "#N");   /* Add end of col */
      strcpy(GET_PRENAME(victim), buf);
    }
  }
  send_to_char("Ok.\r\n", ch);
  
  SET_BIT(PLR_FLAGS(victim), PLR_SAVESTR);
}

/* Function to change all '&' to ';'   -Petrus */
void process_alias_replace(char *ptr)
{
  while (*ptr != '\0') {
    if (*ptr == '&')
      *ptr = ';';
    ptr++;
  }
}

ACMD(do_alias)
{
  struct alias_list *ls, **prev;
  int i = 0;
    
  if(IS_NPC(ch))
    return;

  skip_spaces(&argument);
    
  half_chop(argument, arg, buf);    

  if (!*arg) {
    ls = ch->specials.aliases;
    if (!ls) {
      send_to_char("alias [SHORT] [REPLACE]  or  alias [load|save|clear]\r\n", ch);
      return;
    }
    else {
      for (; ls;ls = ls->next) {
	sprintf(buf2, "  %8s = [%s#N]\r\n", ls->alias, ls->replace);
	send_to_char(buf2, ch);
      }
      return;
    }
  }

  if (!str_cmp("load", arg)) {
    alias_load(ch);
    return;
  }

  if (!str_cmp("save", arg)) {
    if (PLR_FLAGGED(ch, PLR_SAVEALS))
      alias_save(ch);
    else
      send_to_char("No changes made to alias - no need to save.\r\n", ch);
    return;
  }

  if (!str_cmp("clear", arg)) {
    if (ch->specials.aliases)
      free_alias_list(ch->specials.aliases);
    ch->specials.aliases = 0;
    send_to_char("Removing all aliases.\r\n", ch);
    return;
  }

  if (strlen(arg) > 8 || strlen(buf) > 60) {
    send_to_char("Alias too long.\r\n", ch);
    return;
  }

  ls = ch->specials.aliases;
  prev = &ch->specials.aliases;
  while(ls && i < 32 && str_cmp(ls->alias, arg)) {
    prev = &ls->next;
    ls = ls->next;
    i++;
  }

  if (isname("at", buf))
       if (isname(arg, buf)) {
	    send_to_char("Recursive aliases are not allowed!\r\n", ch);
	    return;
       }
    
  if (i > 31) {
    send_to_char("Only 32 aliases possible.\r\n", ch);
    return;
  }

  if (!*buf) {
    if (!ls)
      send_to_char("No such alias to remove.\r\n", ch);
    else {	
      *prev = ls->next;
      send_to_char("Alias removed.\r\n", ch);
      if (ls->alias)
	free(ls->alias);
      if (ls->replace)
	free(ls->replace);
      if (ls)
	free(ls);
      SET_BIT(PLR_FLAGS(ch), PLR_SAVEALS);
    }
    return;
  }

  if (ls) {
    free(ls->alias);
    free(ls->replace);
  }
  else {
    CREATE(ls, struct alias_list, 1);
    *prev = ls;
  }
	
  CREATE(ls->alias, char, strlen(arg) + 1);
  CREATE(ls->replace, char, strlen(buf) + 1);
  strcpy(ls->alias, arg);
  delete_doubledollar(ls->replace);
  process_alias_replace(buf);
  strcpy(ls->replace, buf);
    
  sprintf(buf2, "Ok - %s = %s\r\n",arg, buf);
  send_to_char(buf2, ch);

  SET_BIT(PLR_FLAGS(ch), PLR_SAVEALS);
}
	

/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */


ACMD(do_skillset)
{
  extern struct room_data **world;
  extern FILE *player_fl;
  struct char_data *vict;
  int	skill, value, i;
  char *arg;
  struct char_data *cbuf = NULL;
  struct char_file_u tmp_store;
  char is_file = 0;
  int player_i = 0;

  arg = one_argument(argument, buf);

  if (!*buf) {
    send_to_char("skillset <PLAYER> \"<SKILL>\" <VALUE>\r\n", ch);
    return;
  }

  if(!(vict = get_player_vis_exact(ch, buf))) {
    send_to_char("Not found online... searching pfile.\r\n", ch);
    is_file = 1;
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);

    if((player_i = load_char(buf, &tmp_store)) > -1 ) {
      store_to_char(&tmp_store, cbuf);
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }

  if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
    send_to_char("Nice Try...\r\n", ch);

    if (is_file) 
      free(cbuf);

    return;
  }
  
  if (subcmd == SCMD_SKILLCLEAR) {
    for (i = 0; i < MAX_SKILLS; i++)
      SET_SKILL(vict, i, 0);
    
    send_to_char("All skills cleared!\r\n", ch);
    sprintf(buf, "(GC) %s: %s skillcleared", GET_NAME(ch), GET_NAME(vict));
    mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
  } else { /* End Skillclear */

    arg = one_word(arg, buf);
    if (!*buf) { 
      send_to_char("skillset <PLAYER> \"<SKILL>\" <VALUE>\r\n", ch);
      if (is_file)
	free(cbuf);
      return;
    }
    
    if (!str_cmp(buf, "all")) {	/* set all skills to max */
      for (i = 1; i < MAX_SKILLS;i++) {
	if (know_skill(vict, i) || GET_LEVEL(vict) >= LEVEL_IMMORT) {
	  SET_SKILL(vict, i, get_skill_max(vict, i));
	} else
	  SET_SKILL(vict, i, 0);
      }
      
      send_to_char("Ok Initiating skills&spells for player.\r\n", ch);
      
      sprintf(buf, "(GC) %s: %s's skills&spells set to ALL.", GET_NAME(ch), GET_NAME(vict));
      mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
      
    } else { /* End Skillset ALL */
    
      skill = search_block(buf, spells, FALSE);
      
      if (skill++ == -1) {
	skill = search_block(buf, skills, FALSE);
	if (skill == -1) {
	  send_to_char("No such skill is known. Try 'skills or spells' for list.\r\n", ch);
	  if (is_file)
	    free(cbuf);
	  return;
	}
	skill += SKILL_START;
      }  
      
      one_argument(arg, buf);
      value = atoi(buf);
      
      if (!*buf) {
	send_to_char("Value undefined.\r\n", ch);
	if (is_file)
	  free(cbuf);
	return;
      }
    
      value = MIN(value, 100);
      value = MAX(value, 0);
      
      SET_SKILL(vict, skill, value);
      
      sprintf(buf, "(GC) %s: %s's - %s - set to %d.", GET_NAME(ch), GET_NAME(vict), (skill < SKILL_START?spells[skill - 1]:skills[skill - SKILL_START]), value);
      mudlog(buf, BRF, GET_LEVEL(ch), TRUE);
    }
  } /* End Skillset */

  if(!is_file)
    save_char(vict, IN_VROOM(vict), 2);
  
  if(is_file) {
    char_to_store(vict, &tmp_store, FALSE, FALSE);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
  }
}




/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char	*one_word(char *argument, char *first_arg )
{
   int	found, begin, look_at;

   found = begin = 0;

   do {
      for ( ; isspace(*(argument + begin)); begin++)
	 ;

      if (*(argument + begin) == '\"') {  /* " is it a quote */

	 begin++;

	 for (look_at = 0; (*(argument + begin + look_at) >= ' ') && 
	     (*(argument + begin + look_at) != '\"') ; look_at++)
	    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

	 if (*(argument + begin + look_at) == '\"')     /* " */
	    begin++;

      } else {

	 for (look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)
	    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

      }

      *(first_arg + look_at) = '\0';
      begin += look_at;
   } while (fill_word(first_arg));

   return(argument + begin);
}


struct help_index_element *build_help_index(FILE *fl, int *num)
{
   int	nr = -1, issorted, i;
   struct help_index_element *list = 0, mem;
   char	buf[81], tmp[81], *scan;
   long	pos;

   for (; ; ) {
      pos = ftell(fl);
      fgets(buf, 81, fl);
      *(buf + strlen(buf) - 1) = '\0';
      scan = buf;
      for (; ; ) {
	 /* extract the keywords */
	 scan = one_word(scan, tmp);

	 if (!*tmp)
	    break;

	 if (!list) {
	    CREATE(list, struct help_index_element, 1);
	    nr = 0;
	 } else
	    RECREATE(list, struct help_index_element, ++nr + 1);

	 list[nr].pos = pos;
	 CREATE(list[nr].keyword, char, strlen(tmp) + 1);
	 strcpy(list[nr].keyword, tmp);
      }
      /* skip the text */
      do
	 fgets(buf, 81, fl);
      while (*buf != '#');
      if (*(buf + 1) == '~')
	 break;
   }
   /* we might as well sort the stuff */
   do {
      issorted = 1;
      for (i = 0; i < nr; i++)
	 if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
	    mem = list[i];
	    list[i] = list[i + 1];
	    list[i + 1] = mem;
	    issorted = 0;
	 }
   } while (!issorted);

   *num = nr;
   return(list);
}


void  return_ansi_cursor(struct descriptor_data *d)
{
  if (STATE(d) == CON_PLYNG && 
      PRF_FLAGGED(d->character, PRF_DISPANSI)) {
    sprintf(buf, VTCURPOS VTDELEOS "\0337", 
	    GET_SCRLEN(d->character), 1);
    write_to_descriptor(d->descriptor, buf);
  }
}


void	check_reboot(void)
{
   long	tc;
   struct tm *t_info;
   char	dummy;
   FILE * boot;

   extern int	elite_shutdown;

   tc = time(0);
   t_info = localtime(&tc);

   if ((t_info->tm_hour + 1) == REBOOT_AT && t_info->tm_min > 30)
      if ((boot = fopen("./reboot", "r"))) {
	 if (t_info->tm_min > 50) {
	    log("Reboot exists.");
	    fread(&dummy, sizeof(dummy), 1, boot);
	    if (!feof(boot))   /* the file is nonepty */ {
	       log("Reboot is nonempty.");
	       if (system("./reboot")) {
		  log("Reboot script terminated abnormally");
		  send_to_all("The reboot was cancelled.\r\n");
		  system("mv ./reboot reboot.FAILED");
		  fclose(boot);
		  return;
	       } else
		  system("mv ./reboot reboot.SUCCEEDED");
	    }

	    send_to_all("Automatic shutdown.\r\n");
	    elite_shutdown = 1;
	    system("touch ../log/.killscript");
	 } else if (t_info->tm_min > 40)
	    send_to_all("ATTENTION: EliteMUD will shutdown in 10 minutes.\r\n");
	 else if (t_info->tm_min > 30)
	    send_to_all(
	        "Warning: The game will close and shutdown in 20 minutes.\r\n");

	 fclose(boot);
      }
}

/* temp func -Petrus */
void write_last_command(char *arg)
{
  FILE *fp;
    
  if (!(fp = fopen("../log/.lastcommand", "wb"))) {
    return;
  }

  fputs(arg, fp);
  fputc('\n', fp);

  fclose(fp);
}


/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

#define PAGE_LENGTH     22
#define PAGE_WIDTH      80

/* Traverse down the string until the begining of the next page has been
 * reached.  Return NULL if this is the last page of the string.
 */
char *next_page(char *str, int page_length)
{
  int col = 1, line = 1, spec_code = FALSE;
  int pagelength = MAX(page_length, 20);
  const struct scrcol_replace_data *scrcode;

  for (;; str++) {
    /* If end of string, return NULL. */
    if (*str == '\0')
      return NULL;

    /* If we're at the start of the next page, return this fact. */
    else if (line > pagelength)
      return str;

    /* Check for the begining of an ANSI color code block. */
    else if (*str == '\x1B' && !spec_code)
      spec_code = TRUE;

    /* Check for the end of an ANSI color code block. */
    else if (*str == 'm' && spec_code)
      spec_code = FALSE;

    if (*str == '#') { /* for new color scrcol system -Helm */
      if((scrcode = scr_get_colcode(str + 1)))
	col -= strlen(scrcode->code);
    }

    /* Check for everything else. */
    else if (!spec_code) {
      /* Carriage return puts us in column one. */
      if (*str == '\r')
	col = 1;
      /* Newline puts us on the next line. */
      else if (*str == '\n')
	line++;

      /* We need to check here and see if we are over the page width,
       * and if so, compensate by going to the begining of the next line.
       */
      else if (col++ > PAGE_WIDTH) {
	col = 1;
	line++;
      }
    }
  }
}


/* Function that returns the number of pages in the string. */
int count_pages(char *str, int page_length)
{
  int pages;

  for (pages = 1; (str = next_page(str, page_length)); pages++);
  return pages;
}


/* This function assigns all the pointers for showstr_vector for the
 * page_string function, after showstr_vector has been allocated and
 * showstr_count set.
 */
void paginate_string(char *str, struct descriptor_data *d)
{
  int i;

  if (d->showstr_count)
    *(d->showstr_vector) = str;

  for (i = 1; i < d->showstr_count && str; i++)
    str = d->showstr_vector[i] = next_page(str, GET_SCRLEN(d->character)-2);

  d->showstr_page = 0;
}


/* The call that gets the paging ball rolling... */
void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  if (!d)
    return;

  if (!str || !*str) {
    send_to_char("", d->character);
    return;
  }
  CREATE(d->showstr_vector, char *, d->showstr_count = count_pages(str, GET_SCRLEN(d->character)-2));

  if (keep_internal) {
    d->showstr_head = strdup(str);
    paginate_string(d->showstr_head, d);
  } else
    paginate_string(str, d);

  show_string(d, "");
}


/* The call that displays the next page. */
void show_string(struct descriptor_data *d, char *input)
{
  char buffer[LARGE_BUFSIZE];
  int diff;

  one_argument(input, buf);

  /* Q is for quit. :) */
  if (LOWER(*buf) == 'q') {
    free(d->showstr_vector);
    d->showstr_count = 0;
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = 0;
    }
    return;
  }
  /* R is for refresh, so back up one page internally so we can display
   * it again.
   */
  else if (LOWER(*buf) == 'r')
    d->showstr_page = MAX(0, d->showstr_page - 1);

  /* B is for back, so back up two pages internally so we can display the
   * correct page here.
   */
  else if (LOWER(*buf) == 'b')
    d->showstr_page = MAX(0, d->showstr_page - 2);

  /* Feature to 'goto' a page.  Just type the number of the page and you
   * are there!
   */
  else if (isdigit(*buf))
    d->showstr_page = MAX(0, MIN(atoi(buf) - 1, d->showstr_count - 1));

  else if (*buf) {
    send_to_char(
		  "Valid commands while paging are RETURN, Q, R, B, or a numeric value.\r\n",
		  d->character);
    return;
  }
  /* If we're displaying the last page, just send it to the character, and
   * then free up the space we used.
   */
  if (d->showstr_page + 1 >= d->showstr_count) {
    send_to_char(d->showstr_vector[d->showstr_page], d->character);
    free(d->showstr_vector);
    d->showstr_count = 0;
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = NULL;
    }
  }
  /* Or if we have more to show.... */
  else {      
       strncpy(buffer, d->showstr_vector[d->showstr_page],
	       diff = ((int) d->showstr_vector[d->showstr_page + 1])
	       - ((int) d->showstr_vector[d->showstr_page]));
       buffer[diff] = '\0';
       send_to_char(buffer, d->character);
       d->showstr_page++;
  }
}

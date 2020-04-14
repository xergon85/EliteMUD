/* ************************************************************************
*   File: mail.c                                        Part of EliteMUD  *
*  Usage: Internal funcs and player spec-procs of mud-mail system         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  Written by Jeremy Elson (jelson@server.cs.jhu.edu)                     *
*                                                                         *
*  Updated/Changed by Petrus Wang at Royal Institute of Technology        *
*  Copyright (C) 1995 Mr.Wang at RIT                                      *
*                                                                         *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "mail.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "objsave.h"
#include "functions.h"


extern struct char_data *character_list;
extern struct room_data **world;
extern struct index_data *mob_index;
extern struct obj_data *object_list;
extern int	no_mail;


/* HAS_MAIL */
/* a simple little function which tells you if the guy has mail or not */
int has_mail(char *recipient)
{
  FILE *fp;
  char filename[128];
  
  get_filename(recipient, filename, MAIL_FILE);
  
  if ((fp = fopen(filename, "r"))) {
    fclose(fp);
    return 1;
  }
  
  return 0;
}


void	store_mail(char *to, char *from, char *message_pointer)
{
  FILE *mailfile;
  mail_block_data data;
  char	*msg_txt;
  char  adressee[32], filename[128], mail[LARGE_BUFSIZE];
  int	bytes_written;
  int	total_length;
  long  mailtime;
  struct char_data *k = NULL;

  if (!*from || !*to) {
    log("SYSERR: Mail system -- non-fatal error #5.");
    return;
  }
  if (!*message_pointer)
    return;
  
  mailtime = time(0);

  sprintf(mail, "§w * * * * Elite Mail System * * * *\r\n"
	  "Date: §y%s"
	  "§w  To: §y%s\r\n"
	  "§wFrom: §y%s§N\r\n\r\n"
	  "%s",
	  asctime(localtime(&mailtime)),
	  to, from, message_pointer);
  
  total_length = strlen(mail);

    for (k = character_list; k && !IS_NPC(k); k = k-> next)
      if (isname(k->player.name, to))
	send_to_char("#wYou have mail!#N\r\n", k);

  to = any_one_arg(to, adressee);
  while(adressee && *adressee) {
    msg_txt = mail;
    bytes_written = 0;
    if (get_filename(adressee, filename, MAIL_FILE)) {
      if ((mailfile = fopen(filename, "a+"))) {
	while (bytes_written < total_length) {
	  data.block_type = DATA_BLOCK;
	  strncpy(data.txt, msg_txt, BLOCK_SIZE);
	  data.txt[BLOCK_SIZE] = '\0';
	  
	  bytes_written += strlen(data.txt);
	  msg_txt += strlen(data.txt);

	  if (bytes_written >= total_length)
	    data.block_type = LAST_BLOCK;

	  fwrite(&data, sizeof(data), 1, mailfile);
	}
	fclose(mailfile);
      }
    }
    to = any_one_arg(to, adressee);
  }
} /* store mail */



char	*read_mail(FILE *fp)
{
  mail_block_data data;
  char *message;
  size_t string_size;

  if (!fp) {
    log("SYSERR: Mail system -- non-fatal error #6.");
    return 0;
  }
  message = strdup("");
  data.block_type = DATA_BLOCK;
  
  while (data.block_type != LAST_BLOCK && !feof(fp)) {
    fread(&data, sizeof(data), 1, fp);
    string_size = (sizeof(char) * (strlen(message) + strlen(data.txt) + 1));
    message = (char *)realloc(message, string_size);
    strcat(message, data.txt);
    message[string_size - 1] = '\0';
  }

  return message;
}


/*****************************************************************
** Below is the spec_proc for a postmaster using the above       **
** routines.  Written by Jeremy Elson (jelson@server.cs.jhu.edu) **
*****************************************************************/


int	mail_ok(struct char_data *ch)
{
  if (no_mail) {
    send_to_char("Sorry, the mail system is having technical difficulties.\r\n", ch);
    return 0;
  }
  
  return 1;
}


void	postmaster_send_mail(struct char_data *ch, struct char_data *mailman, int cmd, char *arg)
{
  char	name[MAX_INPUT_LENGTH], recipients[512], *tmp = NULL;
  int num = 0, price = STAMP_PRICE;
  

  if (GET_LEVEL(ch) < MIN_MAIL_LEVEL) {
    sprintf(buf, "$n tells you, 'Sorry, you have to be level %d to send mail!'", MIN_MAIL_LEVEL);
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }

  if (!*arg) { /* you'll get no argument from me! */
    act("$n tells you, 'You need to specify an addressee!'",
	FALSE, mailman, 0, ch, TO_VICT);
    return;
  }

  if (CMD_IS("post")) {
    price = POSTCARD_PRICE;
    arg = any_one_arg(arg, recipients); /* Get cardname */
    sprintf(name, "%s/%s.pic", POSTCARD_DIRECTORY, recipients);
    
    if (file_to_string_alloc(name, &tmp)) {
      act("$n tells you, 'Sorry, we don't have that card!'", FALSE, mailman, 0, ch, TO_VICT);
      return;
    }
  }

  if (GET_LEVEL(ch) >= LEVEL_DEITY)
    price = 0;

  recipients[0] = '\0';
  arg = any_one_arg(arg, name);
  
  while (name && *name) {

    if (find_name(CAP(name)) < 0) {
      sprintf(buf, "$n tells you, 'No one by the name '%s' is registered here!'", name);
      act(buf, FALSE, mailman, 0, ch, TO_VICT);
      return;
    }

    strcat(recipients, name);
    strcat(recipients, " ");
    if (++num > 9) {
      act("$n tells you, 'You can only ask for copies to maximum 9!'", FALSE, mailman, 0, ch, TO_VICT);
      return;
    }

    arg = any_one_arg(arg, name);
  }
  
  if (GET_GOLD(ch) < num * price) {
    sprintf(buf, "$n tells you, 'Stamps cost %d coins.'\r\n"
	    "$n tells you, '...which I see you can't afford.'",
	    num * price);
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  
  act("$n starts to write some mail.", TRUE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "$n tells you, 'I'll take %d coins for the stamp.'\r\n"
	  "$n tells you, 'Write your message, (/s saves /h for help).'",
	  num * price);
  act(buf, FALSE, mailman, 0, ch, TO_VICT);
  GET_GOLD(ch) -= num * price;
  SET_BIT(PLR_FLAGS(ch), PLR_MAILING | PLR_WRITING);
  
  ch->desc->name = (char *)strdup(recipients);
  ch->desc->str = (char **)malloc(sizeof(char *));
  if (tmp)
    *(ch->desc->str) = tmp;
  else
    *(ch->desc->str) = 0;

  ch->desc->max_str = MAX_MAIL_SIZE;
}


void	postmaster_check_mail(struct char_data *ch, struct char_data *mailman, int cmd, char *arg)
{
  char	buf[200], recipient[100];

  _parse_name(GET_NAME(ch), recipient);
  
  if (has_mail(recipient))
    sprintf(buf, "$n tells you, 'You have mail waiting.'");
  else
    sprintf(buf, "$n tells you, 'Sorry, you don't have any mail waiting.'");
  act(buf, FALSE, mailman, 0, ch, TO_VICT);
}


void	postmaster_receive_mail(struct char_data *ch, struct char_data *mailman, int cmd, char *arg)
{
  FILE *mailfile;
  char	buf[200], recipient[100], filename[128], *tmp;
  struct obj_data *tmp_obj;
  int number_of_mail = 0;

  _parse_name(GET_NAME(ch), recipient);

  if (!has_mail(recipient)) {
    sprintf(buf, "$n tells you, 'Sorry, you don't have any mail waiting.'");
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }

  if (!get_filename(recipient, filename, MAIL_FILE))
    return;
  
  if (!(mailfile = fopen(filename, "r+b"))) {
    if (errno != ENOENT) { /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING MAIL FILE %s", filename);
      perror(buf1);
    }
    sprintf(buf, "$n tells you, 'Sorry, you don't seem to have any mail waiting.'");
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }

  while (!feof(mailfile)) {
    tmp = read_mail(mailfile);
    if (!feof(mailfile)) {

      CREATE(tmp_obj, struct obj_data, 1);
      clear_object(tmp_obj);
      
      tmp_obj->name = strdup("mail paper letter");
      tmp_obj->short_description = strdup("a piece of mail");
      tmp_obj->description = strdup("Someone has left a piece of mail here.");
      
      tmp_obj->obj_flags.type_flag = ITEM_NOTE;
      tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
      tmp_obj->obj_flags.weight = 1;
      tmp_obj->obj_flags.cost = 30;
      tmp_obj->obj_flags.cost_per_day = 10;
      tmp_obj->action_description = tmp;
      
      if (!tmp_obj->action_description)
	tmp_obj->action_description = strdup("Mail system error - please report.  Error #11.\r\n");
      
      tmp_obj->next = object_list;
      object_list = tmp_obj;
      
      obj_to_char(tmp_obj, ch);
      number_of_mail++;
      tmp_obj->item_number = -1;
      
    }
  }
  fclose(mailfile);
  Crash_delete_file(recipient, MAIL_FILE);

  if (number_of_mail) {
    sprintf(buf, "$n gives you %d piece%s of mail.", number_of_mail,
	    (number_of_mail>1?"s":""));
    act(buf, FALSE, mailman, 0, ch, TO_VICT);  
    act("$N gives $n some mail.", FALSE, ch, 0, mailman, TO_ROOM);
  }
}


SPECIAL(postmaster)
{
  if (!ch->desc || IS_NPC(ch) || !mob)
    return 0;			/* so mobs don't get caught here */
  
  if (no_mail) {
    send_to_char("Sorry, the mail system is having technical difficulties.\r\n", ch);
    return 0;
  }
  
  if (CMD_IS("mail")) {
    postmaster_send_mail(ch, mob, cmd, arg);
    return 1;
  } else if (CMD_IS("check")) {
    postmaster_check_mail(ch, mob, cmd, arg);
    return 1;
  } else if (CMD_IS("receive")) {
    postmaster_receive_mail(ch, mob, cmd, arg);
    return 1;
  } else if (CMD_IS("list")) {
    postmaster_list_postcards(ch, mob, cmd, arg);
    return 1;
  } else if (CMD_IS("post")) {
    postmaster_send_mail(ch, mob, cmd, arg);
    return 1;
  } else if (CMD_IS("examine")) {
    postmaster_examine_postcard(ch, mob, cmd, arg);
    return 1;
  } else
    return 0;
}


void postmaster_list_postcards(struct char_data * ch,
			       struct char_data *mailman,
			       int cmd, char *arg)
{
  char buf[4096], buf2[200];
  
  sprintf(buf2, POSTCARD_DIRECTORY);
  strcat(buf2, "/index.pic");
  
  if (file_to_string(buf2, buf)) {            /* No index file. */
    log("SYSERR: Cannot find postcard index file.");
    act("$n tells you, 'Sorry, it seems I have lost my list.'", FALSE, mailman, 0, ch, TO_VICT);
  } else {
    page_string(ch->desc, buf, TRUE);
  }
}


void postmaster_examine_postcard(struct char_data * ch,
			     struct char_data *mailman,
			     int cmd, char *arg)
{
  char buf[MAX_PICTURE_SIZE+80];
  char buf2[256];
  
  one_argument(arg, buf);
  
  if (!*buf) {                  /* you'll get no argument from me! */
    act("$n tells you, 'You need to specify a postcard!'",
	FALSE, mailman, 0, ch, TO_VICT);
    return;
  }

  /* path and filename is in buf2 */
  sprintf(buf2, "%s/%s.pic", POSTCARD_DIRECTORY, buf);

  if (file_to_string(buf2, buf)) {                /* No such postcard. */
    act("$n tells you, 'Sorry, we haven't got that postcard here.'",
	FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  page_string(ch->desc, buf, TRUE);
}

char * get_mail_date(char *recepient)
{

  FILE * fp;
  char mailfile[50];
  static char buf[120];
  char buf2[120];

  get_filename(recepient, mailfile, MAIL_FILE);

  if((fp = fopen(mailfile, "r"))){
    fgets(buf2, 120, fp);
    fgets(buf2, 120, fp);
    sprintf(buf, "Has unread mail since %s", buf2 + 8);
    fclose(fp);
    return buf;
  } else {
    fclose(fp);
    return "";
  }
}



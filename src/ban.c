/* ************************************************************************
*   File: ban.c                                         Part of EliteMUD  *
*  Usage: banning/unbanning/checking sites and player names               *
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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "functions.h"

struct ban_list_element *ban_list = 0;
extern char	*ban_types[];
extern int nameserver_is_slow;

void	load_banned (void)
{
   FILE * fl;
   int	i, date;
   char	site_name[BANNED_SITE_LENGTH+1], ban_type[100];
   char	name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next_node;

   ban_list = 0;

   if (!(fl = fopen(BAN_FILE, "r"))) {
      perror("Unable to open banfile");
      return;
   }

   while (fscanf(fl, " %s %s %d %s ", ban_type, site_name, &date, name) == 4) {
      CREATE(next_node, struct ban_list_element, 1);
      strncpy(next_node->site, site_name, BANNED_SITE_LENGTH);
      next_node->site[BANNED_SITE_LENGTH] = '\0';
      strncpy(next_node->name, name, MAX_NAME_LENGTH);
      next_node->name[MAX_NAME_LENGTH] = '\0';
      next_node->date = date;

      for (i = BAN_NOT; i <= BAN_ALL; i++)
	 if (!strcmp(ban_type, ban_types[i]))
	    next_node->type = i;

      next_node->next = ban_list;
      ban_list = next_node;
   }

   fclose(fl);
}


int	isbanned(char *hostname)
{
   int	i;
   struct ban_list_element *banned_node;
   char	*nextchar;

   if (!hostname || !*hostname)
      return(0);

   i = 0;
   for (nextchar = hostname; *nextchar; nextchar++)
      *nextchar = tolower(*nextchar);

   for (banned_node = ban_list; banned_node; banned_node = banned_node->next)
     if (nameserver_is_slow) {
       if (is_abbrev(banned_node->site, hostname))
	 i = MAX(i, banned_node->type);
     } else
       if (strstr(hostname, banned_node->site))/* if hostname is a substring */
	 i = MAX(i, banned_node->type);

   return i;
}


void	_write_one_node(FILE *fp, struct ban_list_element *node)
{
   if (node) {
      _write_one_node(fp, node->next);
      fprintf(fp, "%s %s %ld %s\n", ban_types[node->type],
          node->site, node->date, node->name);
   }
}



void	write_ban_list(void)
{
   FILE * fl;

   if (!(fl = fopen(BAN_FILE, "w"))) {
      perror("write_ban_list");
      return;
   }

   _write_one_node(fl, ban_list);  /* recursively write from end to start */
   fclose(fl);
   return;
}


ACMD(do_ban)
{
   char	flag[80], site[80], format[50], *nextchar, *timestr;
   int	i;
   struct ban_list_element *ban_node;

   if (IS_NPC(ch)) {
      send_to_char("You Beast!!\r\n", ch);
      return;
   }

   if (!*argument || (!(IS_SET(GODLEVEL(ch), IMM_ALL) || \
                        IS_SET(GODLEVEL(ch), IMM_OVERSEER))) ) {
      if (!ban_list) {
	 send_to_char("No sites are banned.\r\n", ch);
	 return;
      }
      strcpy(format, "%-16.16s  %-8.8s  %-24.24s  %-16.16s\r\n");
      sprintf(buf, format,
          "Banned Site Name",
          "Ban Type",
          "Banned On",
          "Banned By");
      sprintf(buf2, format,
          "---------------------------------",
          "---------------------------------",
          "---------------------------------",
          "---------------------------------");
      strcat(buf, buf2);

      for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
	 if (ban_node->date) {
	    timestr = asctime(localtime(&(ban_node->date)));
	    *(timestr + 24) = 0;
	    strcpy(site, timestr);
	 } else
	    strcpy(site, "Unknown");
	 sprintf(buf2, format, ban_node->site, ban_types[ban_node->type], site,
	     ban_node->name);
	 if (strlen(buf) < MAX_STRING_LENGTH - 200)
	   strcat (buf, buf2);
      }
      if (strlen(buf) >= MAX_STRING_LENGTH - 200)
	strcat(buf, "String Length Exceeded\r\n");
      
      page_string(ch->desc, buf, TRUE);
      return;
   }

   half_chop(argument, flag, site);
   if (!*site || !*flag) {
      send_to_char("Usage: ban {all | select | new} site_name\r\n", ch);
      return;
   }

   if (!(!str_cmp(flag, "select") || !str_cmp(flag, "all") || !str_cmp(flag, "new"))) {
      send_to_char("Flag must be ALL, SELECT, or NEW.\r\n", ch);
      return;
   }

   for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
      if (!str_cmp(ban_node->site, site)) {
	 send_to_char(
	     "That site has already been banned -- unban it to change the ban type.\r\n", ch);
	 return;
      }
   }

   CREATE(ban_node, struct ban_list_element, 1);
   strncpy(ban_node->site, site, BANNED_SITE_LENGTH);
   for (nextchar = ban_node->site; *nextchar; nextchar++)
      *nextchar = tolower(*nextchar);
   ban_node->site[BANNED_SITE_LENGTH] = '\0';
   strncpy(ban_node->name, GET_NAME(ch), MAX_NAME_LENGTH);
   ban_node->name[MAX_NAME_LENGTH] = '\0';
   ban_node->date = time(0);

   for (i = BAN_NEW; i <= BAN_ALL; i++)
      if (!str_cmp(flag, ban_types[i]))
	 ban_node->type = i;

   ban_node->next = ban_list;
   ban_list = ban_node;

   sprintf(buf, "%s has banned %s for %s players.", GET_NAME(ch), site,
       ban_types[ban_node->type]);
   mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
   send_to_char("Site banned.\r\n", ch);
   write_ban_list();
}


ACMD(do_unban)
{
   char	site[80];
   struct ban_list_element *ban_node, *prev_node;
   int	found = 0;

   if (IS_NPC(ch)) {
      send_to_char("You are not godly enough for that!\r\n", ch);
   }

   one_argument(argument, site);
   if (!*site) {
      send_to_char("A site to unban might help.\r\n", ch);
      return;
   }

   ban_node = ban_list;
   while (ban_node && !found) {
      if (!str_cmp(ban_node->site, site))
	 found = 1;
      else
	 ban_node = ban_node->next;
   }

   if (!found) {
      send_to_char("That site is not currently banned.\r\n", ch);
      return;
   }

   /* first element in list */
   if (ban_node == ban_list)
      ban_list = ban_list->next;
   else {
      for (prev_node = ban_list; prev_node->next != ban_node; 
          prev_node = prev_node->next)
	 ;

      prev_node->next = ban_node->next;
   }

   send_to_char("Site unbanned.\r\n", ch);
   sprintf(buf, "%s removed the %s-player ban on %s.",
       GET_NAME(ch), ban_types[ban_node->type], ban_node->site);
   mudlog(buf, NRM, MAX(LEVEL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

   free(ban_node);
   write_ban_list();
}


/**************************************************************************
 *  Code to check for invalid names (i.e., profanity, etc.)		  *
 *  Written by Sharon P. Goza						  *
 **************************************************************************/

typedef char	namestring[MAX_NAME_LENGTH];

namestring *invalid_list = NULL;
int	num_invalid = 0;

int Valid_Name(char *newname)
{
  int i;
  struct descriptor_data *dt;
  char tempname[MAX_INPUT_LENGTH];
  extern struct descriptor_data *descriptor_list;

  /*
   * Make sure someone isn't trying to create this same name.  We want to
   * do a 'str_cmp' so people can't do 'Bob' and 'BoB'.  The creating login
   * will not have a character name yet and other people sitting at the
   * prompt won't have characters yet.
   */
  for (dt = descriptor_list; dt; dt = dt->next)
    if (dt->character && GET_NAME(dt->character) && !str_cmp(GET_NAME(dt->character), newname))
      return (STATE(dt) == CON_PLYNG);

  /* return valid if list doesn't exist */
  if (!invalid_list || num_invalid < 1)
    return (1);

  /* change to lowercase */
  strcpy(tempname, newname);
  for (i = 0; tempname[i]; i++)
    tempname[i] = LOWER(tempname[i]);

  /* Does the desired name contain a string in the invalid list? */
  for (i = 0; i < num_invalid; i++)
    if (strstr(tempname, invalid_list[i]))
      return (0);

  return (1);
}


void	Read_Invalid_List(void)
{
   FILE * fp;
   int	i = 0;
   char	string[80];

   if (!(fp = fopen(XNAME_FILE, "r"))) {
      perror("Unable to open invalid name file");
      return;
   }

   /* count how many records */
   while (!feof(fp)) {
      fscanf(fp, "%s", string);
      num_invalid++;
   }
   rewind(fp);

   CREATE(invalid_list, namestring, num_invalid);

   while (!feof(fp)) {
      fscanf(fp, "%s", invalid_list[i++]);
   }

   /* make sure there are no nulls in there */
   for (i = 0; i < num_invalid && *invalid_list[i]; ++i);
      num_invalid = i;

   fclose(fp);
}



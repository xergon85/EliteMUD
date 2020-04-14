/* ************************************************************************
*   File: utility.c                                     Part of EliteMUD  *
*  Usage: various internal functions of a utility nature                  *
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
#include "screen.h"
#include "spells.h"
#include "functions.h"

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif

extern struct time_data time_info;
extern const int age_average[];
extern char *class_abbrevs[];
extern char *class_2abb[];
extern char *class_3abb[];
unsigned long circle_random(void);

int	MIN(int a, int b)
{
   return a < b ? a : b;
}


int	MAX(int a, int b)
{
   return a > b ? a : b;
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((circle_random() % (to - from + 1)) + from);
}


/* simulates dice roll */
int dice(int number, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  while (number-- > 0)
    sum += ((circle_random() % size) + 1);

  return sum;
}


int  to_percentage(struct char_data *ch, int value)
{
    switch (value) {
    case LVL: return MIN(100, GET_LEVEL(ch));break;
    case LVL2:return MIN(100, 2 * GET_LEVEL(ch));break;
    case LVL3:return MIN(100, 3 * GET_LEVEL(ch));break;
    default:return value;
    }
}

/* Get a mob skill -Petrus */
int    get_mob_skill(struct char_data *mob, int nr)
{
    if (nr < SKILL_START || nr > SKILL_START + 99)
	return 0;

    if (!mob->mobskills) {
	perror("no allocated skillarray - get_mob_skill.");
	exit(0);
    }

    return to_percentage(mob, mob->mobskills[nr - SKILL_START]);
}

/* Create a duplicate of a string */
char	*str_dup(const char *source)
{
   char	*new;

   CREATE(new, char, strlen(source) + 1);
   return(strcpy(new, source));
}


/*
 * str_cmp: a case-insensitive version of strcmp().
 * Returns: 0 if equal, > 0 if arg1 > arg2, or < 0 if arg1 < arg2.
 *
 * Scan until strings are found different or we reach the end of both.
 */
int str_cmp(const char *arg1, const char *arg2)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    log("SYSERR: str_cmp() passed a NULL pointer");
    return (0);
  }

  for (i = 0; arg1[i] || arg2[i]; i++)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}


/*
 * strn_cmp: a case-insensitive version of strncmp().
 * Returns: 0 if equal, > 0 if arg1 > arg2, or < 0 if arg1 < arg2.
 *
 * Scan until strings are found different, the end of both, or n is reached.
 */
int strn_cmp(const char *arg1, const char *arg2, int n)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    log("SYSERR: strn_cmp() passed a NULL pointer");
    return (0);
  }

  for (i = 0; (arg1[i] || arg2[i]) && (n > 0); i++, n--)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}


/* log a death trap hit */
void	log_death_trap(struct char_data *ch)
{
   char	buf[150];
   extern struct room_data **world;

   sprintf(buf, "%s hit death trap #%d (%s)", GET_NAME(ch),
       world[ch->in_room]->number, world[ch->in_room]->name);
   mudlog(buf, BRF, LEVEL_DEITY, TRUE);
}


/* writes a string to the log */
void	log(char *str)
{
   long	ct;
   char	*tmstr;

   ct = time(0);
   tmstr = asctime(localtime(&ct));
   *(tmstr + strlen(tmstr) - 1) = '\0';
   fprintf(stderr, "%-19.19s :: %s\n", tmstr, str);
   fflush(stderr);
}


/* New PROC: syslog by Fen Jul 3, 1992 */

void mudlog(char *str, char type, sbyte level, byte file)

{
   char	buf[MAX_STRING_LENGTH];
   extern struct descriptor_data *descriptor_list;
   struct descriptor_data *i;
   char *tmp;
   long	ct;  
   char	tp;

   ct  = time(0);
   tmp = asctime(localtime(&ct));

   if (file)
      fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
   if (level<0)
      return;
  
    sprintf(buf, "§1G[ %s ]#N\r\n", str );

    for (i=descriptor_list; i; i = i->next) 
       if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING)) {
          tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
                (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

          if ((GET_LEVEL(i->character)>=level) && (tp >= type)) {
             send_to_char(buf, i->character);
	  }
       }
   return;
}

/* End of Modification */



void	sprintbit(long vektor, char *names[], char *result)
{
  long	nr;

  *result = '\0';

  if (vektor < 0) {
    strcpy(result, "SPRINTBIT ERROR!");
    return;
  }

  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(1, vektor)) {
      if (*names[nr] == '\0') {
	nr++;
        continue;
      }
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else
	strcat(result, "UNDEFINED ");
    }

    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcat(result, "-");
}

void	sprintflags(long vektor, char *result)
{
  long	nr;
  int i = 0;
  *result = '\0';

  if (vektor < 0) {
    return;
  }

  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(1, vektor)) {
      if (nr < 26)
	*(result+i++) = 'a' + nr;
      else
	*(result+i++) = 'A' + nr - 26;
    }
    nr++;
  }

  *(result+i++) = 0;

  if (!*result)
    strcat(result, "0");
}


void	sprinttype(int type, char *names[], char *result)
{
   int	nr;

   for (nr = 0; (*names[nr] != '\n'); nr++)
      ;
   if (type < nr)
      strcpy(result, names[type]);
   else
      strcpy(result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
   long	secs;
   struct time_info_data now;

   secs = (long) (t2 - t1);

   now.hours = (secs / SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
   secs -= SECS_PER_REAL_HOUR * now.hours;

   now.day = (secs / SECS_PER_REAL_DAY);          /* 0..34 days  */
   secs -= SECS_PER_REAL_DAY * now.day;

   now.month = -1;
   now.year  = -1;

   return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
   long	secs;
   struct time_info_data now;

   secs = (long) (t2 - t1);

   now.hours = (secs / SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
   secs -= SECS_PER_MUD_HOUR * now.hours;

   now.day = (secs / SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
   secs -= SECS_PER_MUD_DAY * now.day;

   now.month = (secs / SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
   secs -= SECS_PER_MUD_MONTH * now.month;

   now.year = (secs / SECS_PER_MUD_YEAR);        /* 0..XX? years */

   return now;
}



struct time_info_data age(struct char_data *ch)
{
   struct time_info_data player_age;

   player_age = mud_time_passed(time(0), ch->player.time.birth);

   player_age.year += age_average[GET_RACE(ch)];
   
   return player_age;
}




/*
** Turn off echoing (specific to telnet client)
*/

void	echo_off(int sock)
{
   char	off_string[] = 
    {
      (char) IAC,
      (char) WILL,
      (char) TELOPT_ECHO,
      (char)  0,
   };

   (void) write(sock, off_string, sizeof(off_string));
}


/*
** Turn on echoing (specific to telnet client)
*/


void	echo_on(int sock)
{
   char	off_string[] = 
    {
      (char) IAC,
      (char) WONT,
      (char) TELOPT_ECHO,
      (char) TELOPT_NAOFFD,
      (char) TELOPT_NAOCRD,
      (char)  0,
   };

   (void) write(sock, off_string, sizeof(off_string));
}


void CLASS_ABBR(struct char_data *ch, char *buf)
{
  if (!IS_NPC(ch)) {
    if ((GET_CLASS(ch) == CLASS_2MULTI) || (GET_CLASS(ch) == CLASS_DUAL))
      sprintf(buf, "%s/%s", class_2abb[(int)GET_1CLASS(ch)],
                            class_2abb[(int)GET_2CLASS(ch)]);
    else if (GET_CLASS(ch) == CLASS_3MULTI)
      sprintf(buf, "%s/%s/%s", class_3abb[(int)GET_1CLASS(ch)], 
                               class_3abb[(int)GET_2CLASS(ch)], 
                               class_3abb[(int)GET_3CLASS(ch)]); 
    else
      sprintf(buf, "%s", class_abbrevs[(int)GET_CLASS(ch)]);        
  } else
    sprintf(buf, "-- --");
  return;
}

/* string manipulation fucntion originally by Darren Wilson */
/* (wilson@shark.cc.cc.ca.us) improved and bug fixed by Chris (zero@cnw.com) */
/* completely re-written again by M. Scott 10/15/96 (scottm@workcommn.net), */
/* substitute appearances of 'pattern' with 'replacement' in string */
/* and return the # of replacements */
int replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size) {
   char *replace_buffer = NULL;
   char *flow, *jetsam, temp;
   int len, i;
   
   if ((strlen(*string) - strlen(pattern)) + strlen(replacement) > max_size)
     return -1;
   
   CREATE(replace_buffer, char, max_size);
   i = 0;
   jetsam = *string;
   flow = *string;
   *replace_buffer = '\0';
   if (rep_all) {
      while ((flow = (char *)strstr(flow, pattern)) != NULL) {
	 i++;
	 temp = *flow;
	 *flow = '\0';
	 if ((strlen(replace_buffer) + strlen(jetsam) + strlen(replacement)) > max_size) {
	    i = -1;
	    break;
	 }
	 strcat(replace_buffer, jetsam);
	 strcat(replace_buffer, replacement);
	 *flow = temp;
	 flow += strlen(pattern);
	 jetsam = flow;
      }
      strcat(replace_buffer, jetsam);
   }
   else {
      if ((flow = (char *)strstr(*string, pattern)) != NULL) {
	 i++;
	 flow += strlen(pattern);  
	 len = ((char *)flow - (char *)*string) - strlen(pattern);
   
	 strncpy(replace_buffer, *string, len);
	 strcat(replace_buffer, replacement);
	 strcat(replace_buffer, flow);
      }
   }
   if (i == 0) return 0;
   if (i > 0) {
      RECREATE(*string, char, strlen(replace_buffer) + 3);
      strcpy(*string, replace_buffer);
   }
   free(replace_buffer);
   return i;
}


/* re-formats message type formatted char * */
/* (for strings edited with d->str) (mostly olc and mail)     */
void format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen) {
   int total_chars, cap_next = TRUE, cap_next_next = FALSE;
   char *flow, *start = NULL, temp;
   /* warning: do not edit messages with max_str's of over this value */
   char formated[MAX_STRING_LENGTH];
   
   flow   = *ptr_string;
   if (!flow) return;

   if (IS_SET(mode, FORMAT_INDENT)) {
      strcpy(formated, "   ");
      total_chars = 3;
   }
   else {
      *formated = '\0';
      total_chars = 0;
   } 

   while (*flow != '\0') {
      while ((*flow == '\n') ||
	     (*flow == '\r') ||
	     (*flow == '\f') ||
	     (*flow == '\t') ||
	     (*flow == '\v') ||
	     (*flow == ' ')) flow++;

      if (*flow != '\0') {

	 start = flow++;
	 while ((*flow != '\0') &&
		(*flow != '\n') &&
		(*flow != '\r') &&
		(*flow != '\f') &&
		(*flow != '\t') &&
		(*flow != '\v') &&
		(*flow != ' ') &&
		(*flow != '.') &&
		(*flow != '?') &&
		(*flow != '!')) flow++;

	 if (cap_next_next) {
	    cap_next_next = FALSE;
	    cap_next = TRUE;
	 }

	 /* this is so that if we stopped on a sentance .. we move off the sentance delim. */
	 while ((*flow == '.') || (*flow == '!') || (*flow == '?')) {
	    cap_next_next = TRUE;
	    flow++;
	 }
	 
	 temp = *flow;
	 *flow = '\0';

	 if ((total_chars + strlen(start) + 1) > 79) {
	    strcat(formated, "\r\n");
	    total_chars = 0;
	 }

	 if (!cap_next) {
	    if (total_chars > 0) {
	       strcat(formated, " ");
	       total_chars++;
	    }
	 }
	 else {
	    cap_next = FALSE;
	    *start = UPPER(*start);
	 }

	 total_chars += strlen(start);
	 strcat(formated, start);

	 *flow = temp;
      }

      if (cap_next_next) {
	 if ((total_chars + 3) > 79) {
	    strcat(formated, "\r\n");
	    total_chars = 0;
	 }
	 else {
	    strcat(formated, "  ");
	    total_chars += 2;
	 }
      }
   }
   strcat(formated, "\r\n");

   if (strlen(formated) > maxlen) formated[maxlen] = '\0';
   RECREATE(*ptr_string, char, MIN(maxlen, strlen(formated)+3));
   strcpy(*ptr_string, formated);
}

/* strips \r's from line */
char *stripcr(char *dest, const char *src) {
   int i, length;
   char *temp;
 
   if (!dest || !src) return NULL;
   temp = &dest[0];
   length = strlen(src);
   for (i = 0; *src && (i < length); i++, src++)
     if (*src != '\r') *(temp++) = *src;
   *temp = '\0';
   return dest;
}


/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file. Buffer given must
 * be at least READ_SIZE (256) characters large.
 */
int get_line(FILE *fl, char *buf)
{
  char temp[READ_SIZE];
  int lines = 0;
  int sl;

  do {
    if (!fgets(temp, READ_SIZE, fl))
      return (0);
    lines++;
  } while (*temp == '*' || *temp == '\n' || *temp == '\r');

  /* Last line of file doesn't always have a \n, but it should. */
  sl = strlen(temp);
  while (sl > 0 && (temp[sl - 1] == '\n' || temp[sl - 1] == '\r'))
    temp[--sl] = '\0';

  strcpy(buf, temp); /* strcpy: OK, if buf >= READ_SIZE (256) */
  return (lines);
}



/**************************************************************************
*   File: ignore.c                                      Part of EliteMUD  *
*  Usage: funcs for ignoring players                                      *
*                                                                         *
*(C) 2003 Charlene Bordador                                               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "functions.h"
#include "objsave.h"

#define MAX_IGNORE 100
#define GET_IGNORE(ch)    ((ch)->specials.ignore_list)

void free_ignore_list(struct ignore *ls)
{
  if (ls->next)
    free_ignore_list(ls->next);

  free(ls->name);
  free(ls);
}

void add_ignore(struct char_data *ch, char *name)
{
  struct ignore *ignore_tmp, *last;

  if (!GET_IGNORE(ch))
  {
    CREATE(last, struct ignore, 1);

    last->name = strdup(name);

    GET_IGNORE(ch) = last;

  }
  else
  {
    for (ignore_tmp = GET_IGNORE(ch);ignore_tmp;ignore_tmp = ignore_tmp->next)
      last = ignore_tmp;

    CREATE(last->next, struct ignore, 1);

    last = last->next;

    last->name = strdup(name);
  }

  SET_BIT(PLR_FLAGS(ch), PLR_SAVEIGN);
}

void remove_ignore(struct char_data *ch, char *name)
{
  struct ignore *ignore_tmp, *temp;

  for (ignore_tmp = GET_IGNORE(ch);ignore_tmp;ignore_tmp = ignore_tmp->next)
    if (!str_cmp(ignore_tmp->name, name))
    {
      REMOVE_FROM_LIST(ignore_tmp, GET_IGNORE(ch), next);
      free(ignore_tmp->name);
      free(ignore_tmp);
      SET_BIT(PLR_FLAGS(ch), PLR_SAVEIGN);
      return;
    }
}

int is_ignoring(struct char_data *ch, char *name)
{
  struct ignore *ignore_tmp;

  for (ignore_tmp = GET_IGNORE(ch);ignore_tmp;ignore_tmp = ignore_tmp->next)
    if (!str_cmp(ignore_tmp->name, name))
      return 1;

  return 0;
}

int allow_ignore(struct char_data *ch)
{
  int num_ignore = 0;
  struct ignore *ignore_tmp;

  if (!GET_IGNORE(ch))
    return 1;

  for (ignore_tmp = GET_IGNORE(ch);ignore_tmp;ignore_tmp = ignore_tmp->next)
    num_ignore++;

  if (num_ignore >= MAX_IGNORE)
    return 0;
  else
    return 1;
}

void show_ignore(struct char_data *ch)
{
  struct ignore *ignore_tmp;
  char buf[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
  int  row_counter, tot_counter;

  if (!GET_IGNORE(ch))
  {
    send_to_char("You are currently not ignoring anyone.\r\n", ch);
    return;
  }

  sprintf(buf, "#rIgnore list:\r\n---\r\n#N");

  row_counter = 0;
  tot_counter = 0;
  for (ignore_tmp = GET_IGNORE(ch);ignore_tmp;ignore_tmp = ignore_tmp->next)
  {
    sprintf(name, "%-12.12s", CAP(ignore_tmp->name));
    strcat(buf, name);

    tot_counter++;
    row_counter++;

    if (row_counter > 4)
    {
      strcat(buf, "\r\n");
      row_counter = 0;
    }
  }

  if (!row_counter)
    strcat(buf, "\r\n");
  else
    strcat(buf, "\r\n\r\n");

  sprintf(name, "Number of players ignored: #r%d#N  Max allowed: #R%d#N\r\n\r\n",
          tot_counter,
          MAX_IGNORE);

  strcat(buf, name);

  page_string(ch->desc, buf, 1);
}

void write_ignorefile(struct char_data *ch)
{
  FILE *f;
  char  f_name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct ignore *ignore_tmp;

  get_filename(GET_NAME(ch), f_name, IGNORE_FILE);

  unlink(f_name);

  REMOVE_BIT(PLR_FLAGS(ch), PLR_SAVEIGN);

  if (!GET_IGNORE(ch))
    return;

  f = fopen(f_name, "wt");

  if (!f)
  {
    sprintf(buf, "Unable to open '%s' when trying to save ignorelist(%s).", f_name, CAP(GET_NAME(ch)));
    mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Unable to save ignorelist!\r\n", ch);
    return;
  }

  for (ignore_tmp = GET_IGNORE(ch);ignore_tmp;ignore_tmp = ignore_tmp->next)
    fprintf(f, "%s\n", ignore_tmp->name);

  fclose(f);
}

void read_ignorefile(struct char_data *ch)
{
  FILE *f;
  char  f_name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct ignore *ignore_tmp;

  get_filename(GET_NAME(ch), f_name, IGNORE_FILE);

  f = fopen(f_name, "r");

  if (!f)
    return;

  if (feof(f))
  {
    fclose(f);
    unlink(f_name);
    sprintf(buf, "Unable to open '%s' when trying to load ignorelist(%s) resetting.", f_name, CAP(GET_NAME(ch)));
    mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Unable to load ignorelist!\r\n", ch);
    return;
  }

  CREATE(GET_IGNORE(ch), struct ignore, 1);
  ignore_tmp = GET_IGNORE(ch);

  do
  {
    fscanf(f, "%s\n", buf);
    ignore_tmp->name = strdup(buf);

    if (!feof(f))
    {
      CREATE(ignore_tmp->next, struct ignore, 1);
      ignore_tmp = ignore_tmp->next;
    }
  } while (!feof(f));

  fclose(f);
}

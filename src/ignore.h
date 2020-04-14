/************************************************************************** 
*   File: ignore.h                                      Part of EliteMUD  *  
*  Usage: types for ignore.c                                              * 
*                                                                         * 
*(C) 2003 Charlene Bordador                                               *
**************************************************************************/

#ifndef _IGNORE_H_
#define _IGNORE_H_

extern void add_ignore(struct char_data *ch, char *name);
extern void remove_ignore(struct char_data *ch, char *name);
extern int is_ignoring(struct char_data *ch, char *name);
extern int allow_ignore(struct char_data *ch);
extern void read_ignorefile(struct char_data *ch);
extern void write_ignorefile(struct char_data *ch);
extern void show_ignore(struct char_data *ch);
extern void free_ignore_list(struct ignore *ls);

#endif

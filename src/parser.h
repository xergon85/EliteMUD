/* ****************************************************************************
*  File: parser.h                                           Part of EliteMUD  *
*  Usage: A realtime parser of EliteC                                         *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) Mr Wang 1995 EliteMud RIT                                    *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#ifndef __PARSER_H__
#define __PARSER_H__

#define	P_FNCT	  258
#define	P_VAR	  259
#define P_PROC    260

#define P_BYTE    270
#define P_SBYTE   271
#define P_UBYTE   272
#define P_SHINT   273
#define P_USHINT  274
#define P_NUMBER  275
#define	P_LONG    276
#define	P_REAL	  277
#define	P_REF	  278
#define	P_TYPE	  279
#define	P_STRING  280
#define	P_CHR	  281
#define	P_OBJ	  282
#define	P_ROOM	  283
#define P_ANY     284

#define	P_IF	  290
#define	P_ELSE	  291
#define	P_AND     292
#define	P_OR	  293
#define	P_NOT	  294
#define	P_INC	  295
#define	P_DEC	  296
#define	P_EQ	  297
#define	P_NE	  298
#define	P_GE	  299
#define	P_LE	  300
#define	P_NEG	  301
#define P_INCLUDE 302
#define P_LSWITCH 303
#define P_RSWITCH 304
#define P_RETURN  305

struct parse_data {
  int type; 
  union {
    int               number;
    long              lnum;
    float             real;
    char             *string;
    int               type;
    struct {
      int   type;
      void *ptr;
    }                 ref;                
    struct {
      int            type;
      struct varrec *argsptr;
      char          *prog;
    }                 proc;
    struct char_data *chr;
    struct obj_data  *obj;
    struct room_data *room;
    const struct symrec    *fptr;
    struct varrec    *vptr;
  } val;
};

struct symrec {
  const char *name;
  int  type;
  int  arg1type;
  int  arg2type;
  int  arg3type;
  int  (*fptr)();
};

struct varrec {
  char *name;
  struct parse_data data;
  struct varrec *next;
};

extern struct parse_data p_tmpval;
extern char parse_errorbuf[];

const struct symrec * parse_get_tokentype(const char *str);
void  parse_init_strings();
char *parse_alloc_string(int size);
char *parse_strdup(char *str);

void  parse_error(char *str);

#endif

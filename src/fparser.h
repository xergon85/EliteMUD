/******************************************************************************
*       ____/    /       / __   __/    ____/                                  *
*      /        /       /      /      /          /       /                    *
*     ___/     /       /      /      ___/   __  __/ __  __/                   *
*    /        /       /      /      /        __/     __/                      *
* _______/ ______/ __/    __/    _______/                                     *
*  File: fparser.h                                            Part of Elite++ *
*  Usage: A realtime file parser for load/save operations                     *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) Mr Wang 1995 EliteMud RIT                                    *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#ifndef __FPARSER_H__
#define __FPARSER_H__

#include <stdio.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE   1
#endif
#ifndef FALSE
#define FALSE  0
#endif

#define FPARSE_INPUT_LENGTH    256
#define FPARSE_LARGE_BUFSIZE 16384

#define FPARSE_COMMENT_CHAR    '%'
#define FPARSE_STRING_CHAR     '"'
#define FPARSE_ARGUMENT_S_CHAR '('  /* START */
#define FPARSE_ARGUMENT_E_CHAR ')'  /* END   */
#define FPARSE_DATA_S_CHAR     '['  /* START */
#define FPARSE_DATA_E_CHAR     ']'  /* END   */
#define FPARSE_FILE_CHAR       '~'

#define F_EOF        EOF
#define F_ERROR      258
#define F_STRTOKEN   259
#define F_NUMBER     260
#define F_REAL       261
#define F_STRING     262
#define F_ARGUMENT   263
#define F_DATA       264
#define F_FILE       265

struct fparse_data {
  int type; 
  union {
    long              number;
    float             real;
    char             *string;
  }   val;
};

/* Global variables */
extern struct fparse_data fp_val;
extern char   fparse_errorbuf[FPARSE_INPUT_LENGTH];
extern char   FP_ERROR;
int     fparse_error(const char *str);
#define FPARSE_ERROR(str, arg)  do {sprintf(fparse_errorbuf, (str), (arg)); FP_ERROR = TRUE; } while(0)

/* Functions */
int    fparse_token(FILE *fp);
int    fparse_unparse_token(FILE *fp);
int    fparse_write_string(FILE *fl, const char *str);
#define fparse_write_strtoken(fl, str)  fprintf(fl, "%s", str)
#define fparse_write_number(fl, num)    fprintf(fl, "%d ", num);
#define fparse_write_numberl(fl, num)   fprintf(fl, "%ld ", num);
#define fparse_putchar(fp, c)  fputc(c, fp)


#endif


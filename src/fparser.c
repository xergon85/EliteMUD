/******************************************************************************
*       ____/    /       / __   __/    ____/                                  *
*      /        /       /      /      /          /       /                    *
*     ___/     /       /      /      ___/   __  __/ __  __/                   *
*    /        /       /      /      /        __/     __/                      *
* _______/ ______/ __/    __/    _______/                                     *
*  File: fparser.c                                           Part of Elite++  *
*  Usage: A realtime parser for load/save operation                           *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) Mr Wang 1995 EliteMud RIT                                    *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#include "fparser.h"

#include "conf.h"
#include "sysdep.h"


#define MYGETC(fp)   fgetc(fp)
#define MYUNGETC(token, fp)   ungetc(token, fp)

static int  fparse_lasttoken = 0;

struct      fparse_data fp_val;
char        fparse_errorbuf[FPARSE_INPUT_LENGTH];
char        FP_ERROR = FALSE;


/* Function set the FPARSE_ERROR_FLAG to TRUE
 * and strcat's an given argument to the error buffer.
 */
int
fparse_error(const char *str)
{
  FP_ERROR = TRUE;
  strcpy(fparse_errorbuf, str);
  return F_ERROR;
}


/* Parses a INTERGER number from file */
static int
fparse_number(FILE *fp)
{
  long val = 0;
  int  pos = 1;
  int token = MYGETC(fp);
  
  if (token == '-') {
    pos = 0;
    token = MYGETC(fp);
  }
  
  while(isdigit(token)) {
    val *= 10;
    val += (token - '0');
    token = MYGETC(fp);
  } 
  MYUNGETC(token, fp);
  
  if (pos)
    fp_val.val.number = val;
  else
    fp_val.val.number = 0 - val;

  return (fp_val.type = F_NUMBER);
}


/* Reads and discarts everything to NEWLINE */
static int
fparse_comment(FILE *fp)
{
  int token = MYGETC(fp);
  
  while (token != EOF && token != '\n')
    token = MYGETC(fp);
  
  if (token == EOF)
    return FALSE;
  
  return MYGETC(fp);
}


static int
fparse_string(FILE *fp, char *buf)
{
  register int len = 0;
  int token = MYGETC(fp);
  
  while (token != EOF && token != FPARSE_STRING_CHAR &&
	 len < FPARSE_LARGE_BUFSIZE - 1) {
    switch(token) {
    case '\\':      
      switch (token = MYGETC(fp)) {
      case 'n': *(buf + len++) = '\n'; break;
      case 't': *(buf + len++) = '\t'; break;
      case 'r': *(buf + len++) = '\r'; break;
      case '"': *(buf + len++) = '"'; break;
      case '\\':  *(buf + len++) = '\\'; break;
      case '\n': case '\r': break;
      default:
	FPARSE_ERROR("invalid '\\' char '%c' in string", token);
	return FALSE;
	break;
      }
      break;
    case '\n':
      *(buf + len++) = '\n';
      *(buf + len++) = '\r';
      break;
    case '\r':
      break;
    default: 
      *(buf + len++) = token;
      break;
    }
    token = MYGETC(fp);
  }
  
  *(buf + len) = '\0';
  
  if (token == EOF || len >= FPARSE_LARGE_BUFSIZE - 1) {
    fparse_error("string too long or EOF reached in string");
    return FALSE;
  }
  
  return TRUE;
}

static int
fparse_argument(FILE *fp, char *buf)
{
  register int len = 0;
  int token = MYGETC(fp);
  
  while (token != EOF && token != FPARSE_ARGUMENT_E_CHAR &&
	 len < FPARSE_INPUT_LENGTH - 1) {
    *(buf + len++) = token;
    
    token = MYGETC(fp);
  }
  
  *(buf + len) = '\0';
   
  if (token == EOF || len >= FPARSE_INPUT_LENGTH - 1) {
    fparse_error("argument too long or EOF reached in argument");
    return FALSE;
  }
  
  return TRUE;
}

static int
fparse_data(FILE *fp, char *buf)
{
  register int len = 0;
  int token = MYGETC(fp);
  
  while (token != EOF && token != FPARSE_DATA_E_CHAR &&
	 len < FPARSE_INPUT_LENGTH - 1) {
    *(buf + len++) = token;
    
    token = MYGETC(fp);
  }
  
  *(buf + len) = '\0';
  
  if (token == EOF || len >= FPARSE_INPUT_LENGTH - 1) {
    fparse_error("data too long or EOF reached in data");
    return FALSE;
  }
  
  return TRUE;
}


static int
fparse_file(FILE *fp, char *buf)
{
  register int len = 0;
  int token = MYGETC(fp);
  
  while (token != EOF && token != FPARSE_FILE_CHAR &&
	 len < FPARSE_LARGE_BUFSIZE - 1) {
    switch(token) {
    case '\\':      
      switch (token = MYGETC(fp)) {
      case 'n': *(buf + len++) = '\n'; break;
      case 't': *(buf + len++) = '\t'; break;
      case 'r': *(buf + len++) = '\r'; break;
      case '"': *(buf + len++) = '"'; break;
      case '\\':  *(buf + len++) = '\\'; break;
      case '\n': case '\r': break;
      default:
	FPARSE_ERROR("invalid '\\' char '%c' in string", token);
	return FALSE;
	break;
      }
      break;
    case '\n':
      *(buf + len++) = '\n';
      *(buf + len++) = '\r';
      break;
    default: 
      *(buf + len++) = token;
      break;
    }    
    token = MYGETC(fp);
  }
  
  *(buf + len) = '\0';
  
  if (token == EOF || len >= FPARSE_LARGE_BUFSIZE - 1) {
    fparse_error("file too long or EOF reached in file");
    return FALSE;
  }
  
  return TRUE;
}


int
fparse_token(FILE *fp)
{
  static char buf[FPARSE_LARGE_BUFSIZE];
  char *ptr = buf;
  int token;
  
  token = MYGETC(fp);
  do {
    while(token != EOF && isspace(token))
      token = MYGETC(fp);
  } while (token == FPARSE_COMMENT_CHAR &&
           token != EOF &&
	   (token = fparse_comment(fp))); /* Comments */ 
  
  if (token == EOF)
    return F_EOF;
  
  fparse_lasttoken = token;
  
  /* if it's a number ... */
  if (isdigit(token) || token == '-') {
    MYUNGETC(token, fp);
    return fparse_number(fp);
  }  
  
  if (token == FPARSE_STRING_CHAR) { /* Start of string */
    if ((fparse_string(fp, buf)) == 0)
      return F_ERROR;
    fp_val.val.string = buf;
    return (fp_val.type = F_STRING);
  }
  
  if (token == FPARSE_ARGUMENT_S_CHAR) { /* Start of argument */
    if ((fparse_argument(fp, buf)) == 0)
      return F_ERROR;
    fp_val.val.string = buf;
    return (fp_val.type = F_ARGUMENT);
  }

  if (token == FPARSE_DATA_S_CHAR) { /* Start of data */
    if ((fparse_data(fp, buf)) == 0)
      return F_ERROR;
    fp_val.val.string = buf;
    return (fp_val.type = F_DATA);
  }

  if (token == FPARSE_FILE_CHAR) { /* Start of file */
    if ((fparse_file(fp, buf)) == 0)
      return F_ERROR;
    fp_val.val.string = buf;
    return (fp_val.type = F_FILE);
  }
  
  if (isalpha(token)) {
    while (token && (isalnum(token) || token == '_')) {
      *ptr++ = token;
      token = MYGETC(fp);
    }
    *ptr = '\0';
    
    MYUNGETC(token, fp);

    fp_val.val.string = buf;
    
    return fp_val.type = F_STRTOKEN;
  }
  
  return token;
}


int
fparse_unparse_token(FILE *fp)
{
  MYUNGETC(fparse_lasttoken, fp);
  return 1;
}


int
fparse_write_string(FILE *fl, const char *str)
{
  char tempstr[FPARSE_LARGE_BUFSIZE];
  register int  i = 0;
  
  if(str != NULL) {
    while (*str) {
      switch (*str) {
      case '\r': ++str; break;
      case '"':
	tempstr[i++] = '\\';
	tempstr[i++] = *str++;
	break;
      case '\\':
	tempstr[i++] = '\\';
	tempstr[i++] = *str++;
	break;
      default:
	tempstr[i++] = *str++;
      }
      
    }
    tempstr[i] = 0;
    
    fprintf(fl, "\"%s\"\n", tempstr);
  } else
    fprintf(fl, "\"\"\n");

  return 1;
}


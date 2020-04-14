/* ****************************************************************************
*  File: parser.c                                           Part of EliteMUD  *
*  Usage: A realtime parser of EliteC                                         *
*  All rights reserved.  See license.doc for complete information.            *
*                                                                             *
*  Copyright (C) Mr Wang 1995 EliteMud RIT                                    *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                    *
**************************************************************************** */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "functions.h"
#include "parser.h"
#include "db.h"
#include "interpreter.h"

extern struct room_data **world;

static char       *parse_tokenlist = 0;
static char       *parse_lasttoken = 0;
char              parse_errorbuf[MAX_INPUT_LENGTH];
char              parse_tmpbuf[MAX_INPUT_LENGTH];
static char       *parse_afterinclude = 0;
static char       *parse_includefile = 0;
static char       *parse_includefilename = 0;
static int        parse_includeline = 1;
static char       *parse_afterprocedure = 0;
static char       *parse_procedurename = 0;
static int        parse_procedureline = 1;
static int        parse_line = 1;
static bool       P_ERRORFLAG = FALSE;
static bool       P_INCLUDEFLAG = FALSE;
static bool       P_PROCEDUREFLAG = FALSE;
static struct parse_data p_val;
struct parse_data p_tmpval;


#define parse_same_type(a, b)  ((a).type == (b).type)
#define parse_is_type(arg, t)  ((arg).type == (t))
#define parse_free_string(arg) if ((arg).type == P_STRING && (arg).val.string) free((arg).val.string);
#define PARSE_ERROR(str, arg)  do {sprintf(parse_tmpbuf, (str), (arg)); parse_error(parse_tmpbuf);} while(0)

int parse_expression(int token);
int parse_ifcase(int token);
int parse_block(int token);
int parse_procedure(int token);
int parse_value(int token);
int parse_arglist(struct varrec **ptr);
int parse_read_program(char **prog);
int parse_if(struct parse_data pred);
int parse_include(int token);
int apply_function(const struct symrec *symptr, int args, struct parse_data arg1, struct parse_data arg2, struct parse_data arg3);
int parse_setref(struct parse_data ref, struct parse_data data);
int parse_getref(struct parse_data ref);
int parse_convert_to_type(struct parse_data *data, int type);
int parse_apply_procedure();
int perform_math(struct parse_data val1, int operator, struct parse_data val2);
int parse_make_same_type(struct parse_data *val1, struct parse_data *val2);


/* UTILS to handle a string allocation unit **********************************/

#define STRING_MEMORY_SIZE LARGE_BUFSIZE
static char p_string_memory[STRING_MEMORY_SIZE];
static char *p_string_next_free = p_string_memory;
static int  p_string_memory_size = 0;

void parse_init_strings()
{
  p_string_memory[STRING_MEMORY_SIZE - 1] = 0;
  p_string_next_free = p_string_memory;
  p_string_memory_size = 0;
}

char *parse_alloc_string(int size)
{
  char *ptr;

  if (p_string_memory_size + size >= STRING_MEMORY_SIZE) {
    parse_error("Unable to allocate more string mem");
    parse_init_strings();
  }
  ptr = p_string_next_free;
  p_string_next_free += size;
  p_string_memory_size += size;
  *(p_string_next_free - 1) = 0;
  
  return ptr;
}

void parse_keep_string()
{
  while(p_string_memory_size++ < STRING_MEMORY_SIZE &&
	*p_string_next_free++);
}

char *parse_strdup(char *str)
{
  char *ptr = parse_alloc_string(strlen(str) + 1);

  strcpy(ptr, str);

  return ptr;
}

/* END: UTILS to handle a string allocation unit *****************************/

/* Functions to handle variables ********************************************/ 

static struct varrec *parse_var_table = 0; 

struct varrec *putvar(char *name, struct varrec **varrecptr)
{
  struct varrec *ptr;
  CREATE(ptr, struct varrec, 1);
  ptr->name = strdup(name);
  ptr->data.type = P_NUMBER;
  ptr->data.val.number = 0;
  
  if (varrecptr) {
    ptr->next = *varrecptr;
    *varrecptr = ptr;
  } else {
    ptr->next = parse_var_table;
    parse_var_table = ptr;
  }
  
  return ptr;
}

struct varrec *getvar(char *name, struct varrec *varrecptr)
{
  struct varrec *ptr;
  for (ptr = varrecptr; ptr; ptr = ptr->next)
    if (!strcmp(ptr->name, name))
      return ptr;
  for (ptr = parse_var_table; ptr; ptr = ptr->next)
    if (!strcmp(ptr->name, name))
      return ptr;
  return 0;
}

void clearvar(struct varrec **varrecptr)
{
  struct varrec *ptr;
  
  if (!varrecptr)
    varrecptr = &parse_var_table; 

  while ((ptr = *varrecptr)) {
    *varrecptr = ptr->next;
    free(ptr->name);
    if (ptr->data.type == P_PROC)     /* Free Arglist */
      clearvar(&(ptr->data.val.proc.argsptr));
    free(ptr);
  }
}


void addvar(char *name, int type, void* data)
{
  struct varrec *ptr;

  ptr = putvar(name, 0);
  
  ptr->data.type = type;
  switch (type) {
  case P_NUMBER: ptr->data.val.number = *((int*)data)            ; break;
  case P_LONG  : ptr->data.val.number = *((long*)data)           ; break;
  case P_REAL  : ptr->data.val.real   = *((float*)data)          ; break;
  case P_STRING: ptr->data.val.string = parse_strdup((char*)(data)); break;
  case P_CHR   : ptr->data.val.chr    = (struct char_data*)(data); break;
  case P_OBJ   : ptr->data.val.obj    = (struct obj_data*)(data) ; break;
  case P_ROOM  : ptr->data.val.room   = (struct room_data*)(data); break;
  default: parse_error("wrong type in var initialization"); break;
  }
}


/* END: Functions to handle variables ***************************************/ 


/* FUNCTIONS for the Program parser ******************************************/

char *skip_one_statement(char *str)
{
  int bracers = 0;
  
  if (!str || !*str)
    return str;
  
  while (*str) {
    if (*str == '{')
      ++bracers;
    else if (*str == '}' && --bracers == 0)
      break;
    else if (*str == ';' && bracers == 0)
      break;
    if (bracers < 0)
      return str;
    if (*str == '\n')
      ++parse_line;        /* Inc line info */
    ++str;
  }
    
  if (*str)
    ++str;

  return str;
}

int parse_number(char **str)
{
  long val = 0, divider = 0;
  char *ptr = *str; 
  
  while (1) { 
    while(isdigit(*ptr)) {
      if (divider)
	divider *= 10;
      val *= 10;
      val += (*ptr - '0');
      ++ptr;
    } 
    if (*ptr == '.') {
      ++ptr;
      divider = 1;
    } else
      break;
  }
  
  *str = ptr;

  if (divider < 10) {
    p_val.type = P_NUMBER;        /* Check for long later? */
    p_val.val.number = val;
  } else {
    p_val.type = P_REAL;
    p_val.val.real = (float)val / (float)divider;
  }
  
  return p_val.type;
}

char *parse_comment(char *str)
{
  while (*str != '\n')
    ++str;
  
  return str;
}

char *parse_string(char *str, char *buf)
{
  int len = 0;
  
  while (*++str && *str != '"' && len < MAX_INPUT_LENGTH - 1) {
    switch(*str) {
    case '\\':      
      switch (*++str) {
      case 'n': *(buf + len++) = '\n'; break;
      case 't': *(buf + len++) = '\t'; break;
      case 'r': *(buf + len++) = '\r'; break;
      case '"': *(buf + len++) = '"'; break;
      case '\\':  *(buf + len++) = '\\'; break;
      default:
	PARSE_ERROR("invalid '\\' char '%c' in string", *str);
	return 0;
	break;
      }
      break;
    case '\n': ++parse_line;
    default: 
      *(buf + len++) = *str;
      break;
    }
  }
  
  *(buf + len) = '\0';
  
  if (*str)
    ++str;
  return str;
}

int parse_token()
{
  static char buf[MAX_INPUT_LENGTH];
  char *ptr = buf;
  int cmd;
  const struct symrec *rec;

  do {
    while(*parse_tokenlist && 
	  (*parse_tokenlist == ' '  || *parse_tokenlist == '\n' || *parse_tokenlist == '\r' || *parse_tokenlist == '\t')) {
      if (*parse_tokenlist == '\n')
	++parse_line;            /* Inc line info */
      ++parse_tokenlist;
    }
  } while (!strncmp(parse_tokenlist, "//", 2) &&
	   (parse_tokenlist = parse_comment(parse_tokenlist))); /* Comments */ 
  
  if (!*parse_tokenlist)
    return 0;
  
  parse_lasttoken = parse_tokenlist;

  /* if it's a number ... */
  if (isdigit(*parse_tokenlist))
    return parse_number(&parse_tokenlist);
  
  if (*parse_tokenlist == '"') {                /* Start of string */
    if ((parse_tokenlist = parse_string(parse_tokenlist, buf)) == 0)
      return 0;
    p_val.val.string = parse_strdup(buf);
    return (p_val.type = P_STRING);
  }
  
  if (isalpha(*parse_tokenlist)) {
    while (*parse_tokenlist &&
	   (isalnum(*parse_tokenlist) || *parse_tokenlist == '_'))
      *ptr++ = *parse_tokenlist++;
    *ptr = '\0';
    
    /* Find the command or function, it it exists */
    if (!(rec = parse_get_tokentype(buf))) {  /* Unknow identifier, try var */
      p_val.val.string = buf;
      return (p_val.type = P_VAR);
    }
    
    if (rec->fptr) {  /* Is a function */
      p_val.val.fptr = rec;
      return (p_val.type = P_FNCT);
    }
    if (rec->type == P_TYPE) {  /* Is type information */
      p_val.val.type = rec->arg1type;
      return (p_val.type = P_TYPE);
    }
    
    return rec->type;
  }
  
#define P_TRY(c,type) if(*parse_tokenlist == c) {++parse_tokenlist; return type;}
  
  switch ((cmd = *parse_tokenlist++)) {
  case '=': P_TRY('=', P_EQ); break;
  case '!': P_TRY('=', P_NE); break;
  case '<': P_TRY('=', P_LE); P_TRY('<', P_LSWITCH); break;
  case '>': P_TRY('=', P_GE); P_TRY('>', P_RSWITCH); break;
  case '&': P_TRY('&', P_AND); break;
  case '|': P_TRY('|', P_OR); break;
  default:
    return cmd;
  }
  
  return cmd;
}


int parse_unparse_token()
{
  if (parse_tokenlist == parse_lasttoken) {
    parse_error("unable to unparse token");
    return 0;
  }

  parse_tokenlist = parse_lasttoken;

  return 1;
}


int parse_program(int token)
{
  while(token && token != '}' && token != P_RETURN)
    token = parse_expression(token);
  
  return token;
}
  

int parse_expression(int token)
{
  switch(token) {
  case P_IF: token = parse_ifcase(token); break;
  case '{':  token = parse_block(token);  break;
  case ';':  token = parse_token();       break;
  case P_INCLUDE: token = parse_include(token); break;
  case P_TYPE: token = parse_procedure(token); break;
  case P_RETURN:
    token = parse_value(parse_token());
    if (token != ';')
      return token;
    else
      return P_RETURN;
    break;
  case 0: break;
  default:
    token = parse_value(token);
    if (token != ';') {
      parse_error("';' expected");
      return 0;
    }
    return (parse_token());    
    break;
  }

  if (P_ERRORFLAG) return 0;
  return token;
}


int parse_procedure(int token)
{
  int type;
  struct varrec *varptr;
  
  if (token != P_TYPE) {
    parse_error("expected procedure return type");
    return 0;
  }
  type = p_val.val.type;
  if (parse_token() != P_VAR) {
    parse_error("expecting procedure definition after type info");
    return 0;
  }
  if (!(varptr = getvar(p_val.val.string, 0)))
    varptr = putvar(p_val.val.string, 0);
  varptr->data.type = P_PROC;
  varptr->data.val.proc.type = type;
  varptr->data.val.proc.argsptr = 0;
  if (parse_token() != '(') {
    PARSE_ERROR("expecting '(' in definition of procedure '%s'", varptr->name);
    return 0;
  }
  token = parse_arglist(&(varptr->data.val.proc.argsptr));
  if (token != ')') {
    PARSE_ERROR("expecting ')' in definition of procedure '%s'", varptr->name);
    return 0;
  }
  return parse_read_program(&(varptr->data.val.proc.prog));
}


int parse_include(int token)
{
  if (token != P_INCLUDE) {
    parse_error("missing 'include'");
    return 0;
  }

  if (P_INCLUDEFLAG) {
    parse_error("cannot include from an include file");
    return 0;
  }

  if ((token = parse_token()) != P_STRING) {
    parse_error("missing include string file name");
    return 0;
  }
  
  parse_includefilename = parse_alloc_string(strlen(p_val.val.string) + strlen(INCLUDE_DIR) + 2);
  
  sprintf(parse_includefilename, "%s/%s", INCLUDE_DIR, p_val.val.string); 

  parse_includefile = parse_alloc_string(0);   /* Allocate space */
  if (file_to_string(parse_includefilename, parse_includefile) < 0) {
    parse_error("could not read include file");
    return 0;
  }
  parse_keep_string();                         /* Keep file */

  P_INCLUDEFLAG = TRUE;
  parse_afterinclude = parse_tokenlist;
  parse_tokenlist = parse_includefile;
  parse_includeline = parse_line;
  parse_line = 1;
  
  token = parse_program(parse_token());
  P_INCLUDEFLAG = FALSE;
  if (P_ERRORFLAG) return 0;

  parse_line = parse_includeline;
  parse_tokenlist = parse_afterinclude;

  if (token == P_RETURN)
    return P_RETURN;
  
  return parse_token();
}
  

int parse_block(int token)
{
  if (token != '{') {
    parse_error("missing starting bracer");
    return 0;
  }
  
  token = parse_program(parse_token());
  
  if (token == P_RETURN)
    return P_RETURN;

  if (token != '}') {
    parse_error("missing closing bracer");
    return 0;
  }
  
  return (parse_token());
}


int parse_ifcase(int token)
{
  if (token != P_IF) {
    parse_error("missing 'if' command");
    return 0;
  }
  
  token = parse_value(parse_token());
  
  if (!parse_if(p_tmpval)) {
    parse_unparse_token();
    parse_tokenlist = skip_one_statement(parse_tokenlist); /* Skip if case */
    token = parse_token();
    if (token == P_ELSE)
      token = parse_expression(parse_token());
  } else {
    token = parse_expression(token);
    if (token == P_ELSE) {
      parse_tokenlist = skip_one_statement(parse_tokenlist); /* Skip if case */
      token = parse_token();
    }
  }

  return token;
}


int parse_if(struct parse_data pred)
{
  switch (pred.type) {
  case P_NUMBER: return pred.val.number;   break;
  case P_REAL:   return pred.val.real;     break;
  case P_CHR:    return (int)pred.val.chr; break;
  case P_OBJ:    return (int)pred.val.obj; break;
  case P_ROOM:   return (int)pred.val.room;break;
  default: parse_error("Unrecognized type in ifcase"); return 0; break;
  }
  return 0;
}
    

bool parse_is_data(int token)
{
  return (token >= P_NUMBER && token <= P_ROOM);
}


int parse_value(int token)
{
  struct parse_data tmpval = p_val, arg1, arg2, arg3;
  struct varrec *varptr = 0;
  int operator, args = 0;

  switch(token) {

  case P_FNCT:
    if (parse_token() != '(') {
      parse_error("missing '(' for function");
      return 0;
    }
    if ((token = parse_token()) != ')') { /* Function has at least one arg */
      token = parse_value(token);
      arg1 = p_tmpval;
      ++args;
      switch (token) {
      case ')': break;
      case ',':                           /* Function has a second arg */
	token = parse_value(parse_token());
	arg2 = p_tmpval;
	++args;
	switch (token) {
	case ')': break;
	case ',':                         /* Function has a third arg */
	  token = parse_value(parse_token());
	  arg3 = p_tmpval;
	  ++args;
	  if (token != ')') {
	    parse_error("expected ')' in function call");
	    return 0;
	  }
	  break;
	default:
	  PARSE_ERROR("syntax error in function '%s' call",
		      (tmpval.val.fptr)->name);
	  return 0;
	}
	break;
      default:
	PARSE_ERROR("syntax error in function '%s' call",
		    (tmpval.val.fptr)->name);
	return 0;
      }
      if (!apply_function(tmpval.val.fptr, args, arg1, arg2, arg3))
	return 0;
    }
    p_val = p_tmpval;
    return parse_value(p_val.type);
    break;

  case P_VAR:
    varptr = getvar(p_val.val.string, 0);
    token = parse_token();
    if (token == '=') {      /* assign */
      if (!varptr)
	varptr = putvar(tmpval.val.string, 0);
      token = parse_value(parse_token());
      varptr->data = p_tmpval;
    } else if (!varptr) {
      PARSE_ERROR("unrecognized identifier '%s'", p_val.val.string);
      return 0;
    }
    if (!parse_unparse_token())
      return 0;
    p_val = p_tmpval = varptr->data;

    if (varptr->data.type == P_PROC) {    /* For parse_error - get proc name */
      parse_procedurename = parse_alloc_string(strlen(varptr->name) + 1);
      strcpy(parse_procedurename, varptr->name);
    } /* End */

    return parse_value(p_val.type);
    break;

  case P_REF:
    token = parse_token();
    if (token == '=') {      /* assign */
      token = parse_value(parse_token());
      parse_setref(tmpval, p_tmpval);
    } else
      parse_getref(tmpval);
    if (!parse_unparse_token())
      return 0;
    p_val = p_tmpval;
    return parse_value(p_val.type);
    break;

  case P_TYPE:   /* Type cast */
    tmpval = p_tmpval;
    token = parse_value(parse_token());
    if (!parse_convert_to_type(&p_tmpval, tmpval.val.type))
      return 0;
    return token;
    break;

  case P_PROC:
    if (!parse_apply_procedure())
      return 0;
    p_val = p_tmpval;
    return parse_value(p_val.type);
    break;
    
  case P_INC: break;
  case P_DEC: break;
  case P_NEG: break;
  case '!':
    token = parse_value(parse_token());
    tmpval.type = P_NUMBER;
    tmpval.val.number = !parse_if(p_tmpval);
    break;
  case '(':
    token = parse_value(parse_token());
    if (token != ')') {
      parse_error("missing ')'");
      return 0;
    }
    p_val = p_tmpval;
    return parse_value(p_val.type);
    break;
    
  default:
    if (!parse_is_data(token)) {
      parse_error("syntax error");
      return 0;
    }
    token = parse_token();
    switch((operator = token)) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case P_EQ:
    case '<':
    case P_GE:
    case '>':
    case P_LE:
    case P_OR:
    case P_AND:
    case P_NE:
    case '&':
    case '|':
    case P_LSWITCH:
    case P_RSWITCH:
      token = parse_value(parse_token());
      if (token)
	if (!perform_math(tmpval, operator, p_tmpval))
	  token = 0;
      return token;
      break;

    default: break;
    }
    break;
  }
  
  p_tmpval = tmpval;
  return token;
}


int perform_math(struct parse_data val1, int operator, struct parse_data val2)
{
  long tmp1, tmp2, tmp;

  if (!parse_same_type(val1, val2) &&
      !parse_make_same_type(&val1, &val2)) { /* Add type conversion */
    parse_error("type mismatch");
    return FALSE;
  }  
  
  switch (val1.type) {
  case P_NUMBER:
  case P_LONG:
    if (val1.type == P_NUMBER) {
      tmp1 = val1.val.number;
      tmp2 = val2.val.number;
    } else {
      tmp1 = val1.val.lnum;
      tmp2 = val2.val.lnum;
    }
    p_tmpval.type = val1.type;
    switch (operator) {
    case '+': tmp = tmp1 + tmp2;
      break;
    case '-': tmp = tmp1 - tmp2;
      break;
    case '*': tmp = tmp1 * tmp2;
      break;
    case '/':
      if (tmp2 == 0) {
	parse_error("division with zero");
	return FALSE;
      }
      tmp = tmp1 / tmp2;
      break;
    case '%':
      if (tmp2 == 0) {
	parse_error("modulo with zero");
	return FALSE;
      }
      tmp = tmp1 % tmp2;
      break;
    case P_EQ: p_tmpval.type = P_NUMBER; tmp = tmp1 == tmp2;
      break;
    case P_LE: p_tmpval.type = P_NUMBER; tmp = tmp1 <= tmp2;
      break;
    case '<': p_tmpval.type = P_NUMBER; tmp = tmp1 < tmp2;
      break;
    case P_GE: p_tmpval.type = P_NUMBER; tmp = tmp1 >= tmp2;
      break;
    case '>': p_tmpval.type = P_NUMBER; tmp = tmp1 > tmp2;
      break;
    case P_NE: p_tmpval.type = P_NUMBER; tmp = tmp1 != tmp2;
      break;
    case P_AND: p_tmpval.type = P_NUMBER; tmp = tmp1 && tmp2;
      break;
    case P_OR: p_tmpval.type = P_NUMBER; tmp = tmp1 || tmp2;
      break;
    case '&': p_tmpval.type = P_NUMBER; tmp = tmp1 & tmp2;
      break;
    case '|': p_tmpval.type = P_NUMBER; tmp = tmp1 | tmp2;
      break;
    case P_LSWITCH: p_tmpval.type = P_NUMBER; tmp = tmp1 << tmp2;
      break;
    case P_RSWITCH: p_tmpval.type = P_NUMBER; tmp = tmp1 >> tmp2;
      break;

    default:
      PARSE_ERROR("unrecognized operator '%c'", operator);
      return FALSE;
      break;
    }
    if (p_tmpval.type == P_NUMBER)
      p_tmpval.val.number = tmp;
    else
      p_tmpval.val.lnum = tmp;
    break;

  case P_REAL:
    p_tmpval.type = P_REAL;
    switch (operator) {
    case '+': p_tmpval.val.real = val1.val.real + val2.val.real;
      break;
    case '-': p_tmpval.val.real = val1.val.real - val2.val.real;
      break;
    case '*': p_tmpval.val.real = val1.val.real * val2.val.real;
      break;
    case '/':
      if (val2.val.real == 0.0) {
	parse_error("division with zero");
	return FALSE;
      }
      p_tmpval.val.real = val1.val.real / val2.val.real;
      break;
    case '%':
      parse_error("modulo with float");
      return FALSE;
      break;
    case P_EQ: p_tmpval.type = P_NUMBER; p_tmpval.val.number = val1.val.real == val2.val.real;
      break;
    case P_LE: p_tmpval.type = P_NUMBER; p_tmpval.val.number = val1.val.real <= val2.val.real;
      break;
    case '<': p_tmpval.type = P_NUMBER; p_tmpval.val.number = val1.val.real < val2.val.real;
      break;
    case P_GE: p_tmpval.type = P_NUMBER; p_tmpval.val.number = val1.val.real >= val2.val.real;
      break;
    case '>': p_tmpval.type = P_NUMBER; p_tmpval.val.number = val1.val.real > val2.val.real;
      break;
    case P_NE: p_tmpval.type = P_NUMBER; p_tmpval.val.number = val1.val.real != val2.val.real;
      break;

    default:
      PARSE_ERROR("no valid operator '%c'", operator);
      return FALSE;
      break;
    }
    break;
    
  case P_STRING:
    p_tmpval.type = P_STRING;
    switch (operator) {
    case '+': 
      p_tmpval.val.string = parse_alloc_string(strlen(val1.val.string) + strlen(val2.val.string) + 1);
      sprintf(p_tmpval.val.string, "%s%s", val1.val.string, val2.val.string);
      break;
    case P_EQ: p_tmpval.type = P_NUMBER;
      p_tmpval.val.number = strcmp(val1.val.string, val2.val.string) == 0;
      break;
    case P_NE: p_tmpval.type = P_NUMBER;
      p_tmpval.val.number = strcmp(val1.val.string, val2.val.string) != 0;
      break;
    case '<': p_tmpval.type = P_NUMBER;
      p_tmpval.val.number = strcmp(val1.val.string, val2.val.string) < 0;
      break;
    case '>': p_tmpval.type = P_NUMBER;
      p_tmpval.val.number = strcmp(val1.val.string, val2.val.string) > 0;
      break;
    case P_GE: p_tmpval.type = P_NUMBER;
      p_tmpval.val.number = strcmp(val1.val.string, val2.val.string) >= 0;
      break;
    case P_LE: p_tmpval.type = P_NUMBER;
      p_tmpval.val.number = strcmp(val1.val.string, val2.val.string) <= 0;
      break;
    default:
      PARSE_ERROR("cannot operate with '%c' on type string", operator);
      return FALSE;
      break;
    }
    break;

  case P_CHR:
  case P_OBJ:
  case P_ROOM:
    p_tmpval.type = P_NUMBER;
    switch (operator) {
    case P_EQ: p_tmpval.val.number = val1.val.chr == val2.val.chr; break;
    case P_NE: p_tmpval.val.number = val1.val.chr != val2.val.chr; break;
    default:
      PARSE_ERROR("cannot operate with '%c' on advanced types", operator);
      return FALSE;
      break;
    }
    
    break;
    
  default:
    parse_error("cannot operate on advanced types");
    return FALSE;
    break;
  }
  
  return TRUE;
}


int apply_function(const struct symrec *symptr, int args, struct parse_data arg1, struct parse_data arg2, struct parse_data arg3)
{
  if (!args) {
    if (symptr->arg1type) {
      PARSE_ERROR("wrong number of args in function '%s' call", symptr->name);
      return 0;
    }
    return symptr->fptr();
  } else {             /* one arg at least for function */
    if (!parse_is_type(arg1, symptr->arg1type) && symptr->arg1type != P_ANY) {
      if (!(symptr->arg1type)) {
	PARSE_ERROR("function '%s' not expecting any arg", symptr->name);
	return 0;  
      } else if (!parse_convert_to_type(&arg1, symptr->arg1type))
	PARSE_ERROR("type mismatch for first arg in function '%s' call",
		    symptr->name);
    }
    if (args > 1) {  /* Two args or more for function */
      if (!parse_is_type(arg2,symptr->arg2type) && symptr->arg2type!=P_ANY) {
	if (!(symptr->arg2type)) {
	  PARSE_ERROR("function '%s' not expecting a second arg",
		      symptr->name);
	  return 0;
	} else if (!parse_convert_to_type(&arg2, symptr->arg2type))
	  PARSE_ERROR("type mismatch for second arg in function '%s' call",
		      symptr->name);
      }
      if (args == 3) {  /* Three args for function */
	if (!parse_is_type(arg3,symptr->arg3type) && symptr->arg3type!=P_ANY) {
	  if (!(symptr->arg3type)) {
	    PARSE_ERROR("function '%s' not expecting a third arg",
			symptr->name);
	    return 0;  
	  } else if (!parse_convert_to_type(&arg3, symptr->arg3type))
	    PARSE_ERROR("type mismatch for third arg in function '%s' call",
			symptr->name);
	}
	return symptr->fptr(arg1, arg2, arg3);
	return 1;
      } else if (symptr->arg3type) { /* Check if function wants more args */
	PARSE_ERROR("function '%s' expecting a third arg",
		    symptr->name);
	return 0;
      }
      return symptr->fptr(arg1, arg2);
      return 1;
    } else if (symptr->arg2type) { /* Check if function wants more args */
      PARSE_ERROR("function '%s' expecting a second arg",
		  symptr->name);
      return 0;
    }
    
    return symptr->fptr(arg1);
  }
  
  return 1;
}
  

#define ALLOCATED_CHARS_FOR_NUMBERS     32

int parse_convert_to_type(struct parse_data *data, int type)
{
  char *tmpptr;

  if (type == data->type)
    return 1;

  switch (data->type) {
  case P_NUMBER:
    switch (type) {
    case P_LONG: data->val.lnum = data->val.number; break;
    case P_REAL: data->val.real = data->val.number; break; 
    case P_STRING:
      tmpptr = parse_alloc_string(ALLOCATED_CHARS_FOR_NUMBERS);
      sprintf(tmpptr, "%d", data->val.number);
      data->val.string = tmpptr;
      break;
    case P_BYTE: case P_UBYTE: case P_SBYTE: case P_SHINT: case P_USHINT:
      return 1; break;
    default:
      parse_error("cannot convert to type");
      return 0;
      break;
    }
    data->type = type;
    break;
  case P_LONG:
    switch (type) {
    case P_NUMBER: data->val.number = data->val.lnum; break;
    case P_REAL: data->val.real = data->val.lnum; break; 
    case P_STRING:
      tmpptr = parse_alloc_string(ALLOCATED_CHARS_FOR_NUMBERS);
      sprintf(tmpptr, "%ld", data->val.lnum);
      data->val.string = tmpptr;
      break;
    case P_BYTE: case P_UBYTE: case P_SBYTE: case P_SHINT: case P_USHINT:
      data->val.number = data->val.lnum;
      data->type = P_NUMBER;
      return 1; break;
    default:
      parse_error("cannot convert to type");
      return 0;
      break;
    }
    data->type = type;
    break;
  case P_REAL:
    switch (type) {
    case P_NUMBER: data->val.number = data->val.real; break;
    case P_LONG: data->val.lnum = data->val.real; break;
    case P_STRING:
      tmpptr = parse_alloc_string(ALLOCATED_CHARS_FOR_NUMBERS);
      sprintf(tmpptr, "%f", data->val.real);
      data->val.string = tmpptr;
      break;
    case P_BYTE: case P_UBYTE: case P_SBYTE: case P_SHINT: case P_USHINT:
      data->val.number = data->val.real;
      data->type = P_NUMBER;
      return 1; break;
    default:
      parse_error("cannot convert to type");
      return 0;
      break;
    }
    data->type = type;
    break;
  default:
    parse_error("type cannot be converted");
    return 0;
    break;
  }
  return 0;
}


int parse_make_same_type(struct parse_data *val1, struct parse_data *val2)
{
  if (val1->type == val2->type)
    return 1;

  switch (val1->type) {
  case P_NUMBER:
    switch (val2->type) {
    case P_LONG: break;
    case P_REAL: break;
    case P_STRING: break;
    default: return 0; break;
    }
    return parse_convert_to_type(val1, val2->type);
    break;
  case P_LONG:
    switch (val2->type) {
    case P_NUMBER: return parse_convert_to_type(val2, P_LONG); break;
    case P_REAL: break;
    case P_STRING: break;
    default: return 0; break;
    }
    return parse_convert_to_type(val1, val2->type);
    break;
  case P_REAL:
    switch (val2->type) {
    case P_NUMBER: return parse_convert_to_type(val2, P_REAL); break;
    case P_LONG: return parse_convert_to_type(val2, P_REAL); break;
    case P_STRING: return parse_convert_to_type(val1, P_STRING); break;
    default: return 0; break;
    }
    break;
  case P_STRING:
    switch (val2->type) {
    case P_NUMBER: break;
    case P_LONG: break;
    case P_REAL: break;
    default: return 0; break;
    }
    return parse_convert_to_type(val2, P_STRING);
    break;
  default: return 0; break;
  }

  return 0;
}


int parse_setref(struct parse_data ref, struct parse_data data)
{
  char **tmpptr;

  if (ref.type != P_REF) {
    parse_error("type is not REF in setref");
    return 0;
  }
  if (ref.val.ref.type != data.type &&
      !parse_convert_to_type(&data, ref.val.ref.type)) {
    parse_error("Type mismatch assigning referance");
    return 0;
  }
  
  switch (ref.val.ref.type) {
  case P_NUMBER: *(int*)(ref.val.ref.ptr) = data.val.number; break;
  case P_LONG: *(long*)(ref.val.ref.ptr) = data.val.lnum; break;
  case P_REAL: *(float*)(ref.val.ref.ptr) = data.val.real; break;
    /* OBS: Free's old string ... Better be careful with this */
  case P_STRING: 
    tmpptr = (char**)(ref.val.ref.ptr);
    if (*tmpptr) free(*tmpptr);
    *tmpptr = strdup(data.val.string);
    break;
  case P_SBYTE: *(sbyte*)(ref.val.ref.ptr) = (sbyte)data.val.number; break;  
  case P_BYTE:  *(byte*)(ref.val.ref.ptr) = (byte)data.val.number; break;    
  case P_UBYTE: *(ubyte*)(ref.val.ref.ptr) = (ubyte)data.val.number; break;  
  case P_SHINT: *(sh_int*)(ref.val.ref.ptr) = (sh_int)data.val.number; break;
  case P_USHINT:*(ush_int*)(ref.val.ref.ptr) = (ush_int)data.val.number; break;
  default:
    parse_error("Referance to complex types");
    return 0;
  }
  
  return 1;
}


int parse_getref(struct parse_data ref)
{
  char *tmpptr;

  if (ref.type != P_REF) {
    parse_error("type is not REF in getref");
    return 0;
  }

  p_tmpval.type = ref.val.ref.type;
  switch (ref.val.ref.type) {
  case P_NUMBER: p_tmpval.val.number = *(int*)(ref.val.ref.ptr); break;
  case P_LONG: p_tmpval.val.lnum = *(long*)(ref.val.ref.ptr); break;
  case P_REAL: p_tmpval.val.real = *(float*)(ref.val.ref.ptr); break;
  case P_STRING: 
    tmpptr = *(char**)(ref.val.ref.ptr);
    p_tmpval.val.string = parse_strdup(tmpptr);
    break;
  case P_SBYTE: p_tmpval.type = P_NUMBER; 
    p_tmpval.val.number = *(sbyte*)(ref.val.ref.ptr); break;  
  case P_BYTE: p_tmpval.type = P_NUMBER;
    p_tmpval.val.number = *(byte*)(ref.val.ref.ptr); break;    
  case P_UBYTE: p_tmpval.type = P_NUMBER;
    p_tmpval.val.number = *(ubyte*)(ref.val.ref.ptr); break;  
  case P_SHINT:  p_tmpval.type = P_NUMBER;
    p_tmpval.val.number = *(sh_int*)(ref.val.ref.ptr); break;
  case P_USHINT: p_tmpval.type = P_NUMBER;
    p_tmpval.val.number = *(ush_int*)(ref.val.ref.ptr); break;
  default:
    parse_error("Referance to complex types");
    return 0;
  }
  
  return 1;
}


int parse_arglist(struct varrec **ptr)
{
  int type_or_token;
  struct varrec *tmp, **ptmp = ptr;

  if (parse_token() != P_TYPE) {
    parse_error("type info expected in argument list");
    return 0;
  }
  type_or_token = p_val.val.type;
  if (parse_token() != P_VAR) {
    parse_error("identifier for argument name expected");
    return 0;
  }
  if (getvar(p_val.val.string, *ptr)) {
    parse_error("redefinition of argument identifier");
    return 0;
  }
  while (ptmp && *ptmp)
    ptmp = &((*ptmp)->next);
  tmp = putvar(p_val.val.string, ptmp);
  tmp->data.type = type_or_token;
  if ((type_or_token = parse_token()) == ',')
    return parse_arglist(ptr);

  return type_or_token;
}


int parse_read_program(char **prog)
{
  if (parse_token() != '{') {
    parse_error("expecting '{' before body of procedure definition");
    return 0;
  }
  *prog = parse_tokenlist;
  parse_unparse_token();  
  parse_tokenlist = skip_one_statement(parse_tokenlist);/*Skip the definition*/
  return parse_token();
}


int parse_apply_procedure()
{
  struct parse_data tmpval = p_val;
  struct varrec **ptr, *tmp;
  int token, argnum = 0;

  if (P_PROCEDUREFLAG) {
    parse_error("cannot call procedure within procedure call");
    return 0;
  }
  if (parse_token() != '(') {
    parse_error("'(' expected in procedure call");
    return 0;
  }
  ptr = &(tmpval.val.proc.argsptr);
  token = parse_token();
  while (token != ')') { 
    ++argnum;
    token = parse_value(token);
    if (!(*ptr)) {
      if (argnum == 1)
	parse_error("procedure not expecting any argument");
      else
	parse_error("procedure not expecting anymore arguments");
      return 0;
    }
    if ((*ptr)->data.type != p_val.type &&
	!parse_convert_to_type(&p_val, (*ptr)->data.type)) {
      PARSE_ERROR("type mismatch for argument number %d", argnum);
      return 0;
    }
    (*ptr)->data = p_val;
    ptr = &((*ptr)->next);
    
    if (token == ')') break;
    if (token == ',')
      token = parse_token();
    else {
      parse_error("expecting ',' between arguments in procedure call");
      return 0;
    }
  }
  if (*ptr) {
    parse_error("not enough arguments for procedure call");
    return 0;
  }
  
  P_PROCEDUREFLAG = TRUE;

  (*ptr) = parse_var_table;                /* Add new 'frame' for vars */
  parse_var_table = tmpval.val.proc.argsptr;
  parse_afterprocedure = parse_tokenlist;
  parse_tokenlist = tmpval.val.proc.prog;
  parse_procedureline = parse_line;
  parse_line = 1;
  
  parse_program(parse_token());

  /* HAVE TO FIX VAR TABLE - FREE LOCAL VARS */
  if (parse_var_table != tmpval.val.proc.argsptr) {  /*New local vars exists*/
    tmp = parse_var_table;
    while(tmp->next != tmpval.val.proc.argsptr)
      tmp = tmp->next;
    tmp->next = 0;
    clearvar(&parse_var_table);
  }
  
  parse_var_table = (*ptr);
  (*ptr) = 0;

  P_PROCEDUREFLAG = FALSE;
  if (P_ERRORFLAG) return 0;

  if (p_val.type != tmpval.val.proc.type) {
    parse_error("return type from procedure mismatch declaration");
    return 0;
  }

  parse_line = parse_procedureline;
  parse_tokenlist = parse_afterprocedure;
  
  return 1;
}


void parse_error(char *str)
{
  if (!P_ERRORFLAG) {
    if (P_INCLUDEFLAG)
      sprintf(parse_errorbuf, "%sin included file \"%s\": ",
	      parse_errorbuf, parse_includefilename);
    if (P_PROCEDUREFLAG)
      sprintf(parse_errorbuf, "%sin procedure \"%s\": ",
	      parse_errorbuf, parse_procedurename);

    strcat(parse_errorbuf, str);
    strcat(parse_errorbuf, ", ");
    P_ERRORFLAG = TRUE;
  }
  return;
}

void mprog_driver(char *com_list, struct char_data *mob, struct char_data *actor, struct obj_data *obj, void *vo)
{
  char tmpbuf[MAX_INPUT_LENGTH];
  parse_init_strings();
  P_ERRORFLAG = FALSE;
  parse_line = 1;

  if (mob) addvar("mob", P_CHR, (void*)mob);
  if (actor) addvar("ch", P_CHR, (void*)actor);
  if (obj) addvar("obj", P_OBJ, (void*)obj);

  parse_tokenlist = com_list;

  parse_program(parse_token());

  clearvar(0);

  if (P_ERRORFLAG) {
    sprintf(parse_errorbuf, "%sline %d", parse_errorbuf, parse_line);
    if (*parse_lasttoken) {
      strncpy(tmpbuf, parse_lasttoken, 16);
      *(tmpbuf + 16) = 0;
      if (*tmpbuf)
	sprintf(parse_errorbuf, "%s, before or at: '%s'",
		parse_errorbuf, tmpbuf);
      
    }
    mudlog(parse_errorbuf, BRF, LEVEL_DEITY, TRUE);
  }
}


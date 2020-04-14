/******************************************************************************
*       ____/    /       / __   __/    ____/                                  *
*      /        /       /      /      /          /       /                    *
*     ___/     /       /      /      ___/   __  __/ __  __/                   *
*    /        /       /      /      /        __/     __/                      *
* _______/ ______/ __/    __/    _______/                                     *
* File: scrcol.h                                             Part of Elite++  *
* Screen and color module.                                                    *
* Copyright (C) 1995 Petrus Wang at Royal Institute of Technology (KTH)       *
******************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "scrcol.h"

#define TRUE   1
#define FALSE  0

#define IS_SET(flg, bit)  ((flg) & (bit))

/* Table of screen codes and color codes *************************************/


#define CODE1   SCRCOL_ADDCODE1
#define CODE2   SCRCOL_ADDCODE2
#define CODEVT  SCRCOL_ADDCODEVT
#define CODEPC  SCRCOL_ADDCODEPC
#define KEEP    SCRCOL_KEEPCODE
#define SPECIAL SCRCOL_SPECIAL
#define CHAR    SCRCOL_CHAR


static int num_of_scr_codes = 0;

/* This table must be kept sorted so binary sort can be used */
static struct scrcol_replace_data
scr_replace_table[] = {
  { "<BLINK>", VT_BLINK, SPECIAL | CODEVT },
  { "<!BLINK>", VT_NOBLINK, SPECIAL | CODEVT },
  { "<UNDERLINE>", VT_ULINE, SPECIAL | CODEVT },
  { "<!UNDERLINE>", VT_NOULINE, SPECIAL | CODEVT },
  { "<REVERSE>", VT_REVERSE, SPECIAL | CODEVT },
  { "<!REVERSE>", VT_NOREVERSE, SPECIAL | CODEVT },
  { "<DIM>" , VT_DIM, SPECIAL | CODEVT },

  { "<BEEP>", VT_BEEP, SPECIAL },

  { "$"  , VT_CLS      , SPECIAL },

  { "#"  , "#"         , CHAR },
  { "§" , "§"    , SPECIAL | CHAR },

  { "0"  , VT_OFF      , SPECIAL },
  { "N"  , VT_COLBNRM VT_COLNRM , CODE1 },

  { ":B" , VT_COLBBLU  , CODE1 | CODE2 },
  { ":C" , VT_COLBCYN  , CODE1 | CODE2 },
  { ":G" , VT_COLBGRN  , CODE1 | CODE2 },
  { ":M" , VT_COLBMAG  , CODE1 | CODE2 },
  { ":N" , VT_COLBNRM  , CODE1 | CODE2 },
  { ":R" , VT_COLBRED  , CODE1 | CODE2 },
  { ":W" , VT_COLBWHT  , CODE1 | CODE2 },
  { ":Y" , VT_COLBYEL  , CODE1 | CODE2 },
  { ":b" , VT_COLBLBLU , CODE1 | CODE2 },
  { ":c" , VT_COLBLCYN , CODE1 | CODE2 },
  { ":g" , VT_COLBLGRN , CODE1 | CODE2 },
  { ":m" , VT_COLBLMAG , CODE1 | CODE2 },
  { ":r" , VT_COLBLRED , CODE1 | CODE2 },
  { ":w" , VT_COLBLWHT , CODE1 | CODE2 },
  { ":y" , VT_COLBLYEL , CODE1 | CODE2 },
  { "B"  , VT_COLBLU   , CODE1 | CODE2 },
  { "C"  , VT_COLCYN   , CODE1 | CODE2 },
  { "G"  , VT_COLGRN   , CODE1 | CODE2 },
  { "M"  , VT_COLMAG   , CODE1 | CODE2 },
  { "R"  , VT_COLRED   , CODE1 | CODE2 },
  { "W"  , VT_COLWHT   , CODE1 | CODE2 },
  { "Y"  , VT_COLYEL   , CODE1 | CODE2 },
  { "b"  , VT_COLLBLU  , CODE1 | CODE2 },
  { "c"  , VT_COLLCYN  , CODE1 | CODE2 },
  { "e"  , VT_COLLBLK  , CODE1 | CODE2 },
  { "g"  , VT_COLLGRN  , CODE1 | CODE2 },
  { "m"  , VT_COLLMAG  , CODE1 | CODE2 },
  { "r"  , VT_COLLRED  , CODE1 | CODE2 },
  { "w"  , VT_COLLWHT  , CODE1 | CODE2 },
  { "y"  , VT_COLLYEL  , CODE1 | CODE2 },

  { "2:B" , VT_COLBBLU  , CODE2 | SPECIAL },
  { "2:C" , VT_COLBCYN  , CODE2 | SPECIAL },
  { "2:G" , VT_COLBGRN  , CODE2 | SPECIAL },
  { "2:M" , VT_COLBMAG  , CODE2 | SPECIAL },
  { "2:N" , VT_COLBNRM  , CODE2 | SPECIAL },
  { "2:R" , VT_COLBRED  , CODE2 | SPECIAL },
  { "2:W" , VT_COLBWHT  , CODE2 | SPECIAL },
  { "2:Y" , VT_COLBYEL  , CODE2 | SPECIAL },
  { "2:b" , VT_COLBLBLU , CODE2 | SPECIAL },
  { "2:c" , VT_COLBLCYN , CODE2 | SPECIAL },
  { "2:g" , VT_COLBLGRN , CODE2 | SPECIAL },
  { "2:m" , VT_COLBLMAG , CODE2 | SPECIAL },
  { "2:r" , VT_COLBLRED , CODE2 | SPECIAL },
  { "2:w" , VT_COLBLWHT , CODE2 | SPECIAL },
  { "2:y" , VT_COLBLYEL , CODE2 | SPECIAL },
  { "2B"  , VT_COLBLU   , CODE2 | SPECIAL },
  { "2C"  , VT_COLCYN   , CODE2 | SPECIAL },
  { "2G"  , VT_COLGRN   , CODE2 | SPECIAL },
  { "2M"  , VT_COLMAG   , CODE2 | SPECIAL },
  { "2R"  , VT_COLRED   , CODE2 | SPECIAL },
  { "2W"  , VT_COLWHT   , CODE2 | SPECIAL },
  { "2Y"  , VT_COLYEL   , CODE2 | SPECIAL },
  { "2b"  , VT_COLLBLU  , CODE2 | SPECIAL },
  { "2c"  , VT_COLLCYN  , CODE2 | SPECIAL },
  { "2g"  , VT_COLLGRN  , CODE2 | SPECIAL },
  { "2m"  , VT_COLLMAG  , CODE2 | SPECIAL },
  { "2r"  , VT_COLLRED  , CODE2 | SPECIAL },
  { "2w"  , VT_COLLWHT  , CODE2 | SPECIAL },
  { "2y"  , VT_COLLYEL  , CODE2 | SPECIAL },

  { "1:B" , VT_COLBBLU  , CODE1 | SPECIAL },
  { "1:C" , VT_COLBCYN  , CODE1 | SPECIAL },
  { "1:G" , VT_COLBGRN  , CODE1 | SPECIAL },
  { "1:M" , VT_COLBMAG  , CODE1 | SPECIAL },
  { "1:N" , VT_COLBNRM  , CODE1 | SPECIAL },
  { "1:R" , VT_COLBRED  , CODE1 | SPECIAL },
  { "1:W" , VT_COLBWHT  , CODE1 | SPECIAL },
  { "1:Y" , VT_COLBYEL  , CODE1 | SPECIAL },
  { "1:b" , VT_COLBLBLU , CODE1 | SPECIAL },
  { "1:c" , VT_COLBLCYN , CODE1 | SPECIAL },
  { "1:g" , VT_COLBLGRN , CODE1 | SPECIAL },
  { "1:m" , VT_COLBLMAG , CODE1 | SPECIAL },
  { "1:r" , VT_COLBLRED , CODE1 | SPECIAL },
  { "1:w" , VT_COLBLWHT , CODE1 | SPECIAL },
  { "1:y" , VT_COLBLYEL , CODE1 | SPECIAL },
  { "1B"  , VT_COLBLU   , CODE1 | SPECIAL },
  { "1C"  , VT_COLCYN   , CODE1 | SPECIAL },
  { "1G"  , VT_COLGRN   , CODE1 | SPECIAL },
  { "1M"  , VT_COLMAG   , CODE1 | SPECIAL },
  { "1R"  , VT_COLRED   , CODE1 | SPECIAL },
  { "1W"  , VT_COLWHT   , CODE1 | SPECIAL },
  { "1Y"  , VT_COLYEL   , CODE1 | SPECIAL },
  { "1b"  , VT_COLLBLU  , CODE1 | SPECIAL },
  { "1c"  , VT_COLLCYN  , CODE1 | SPECIAL },
  { "1g"  , VT_COLLGRN  , CODE1 | SPECIAL },
  { "1m"  , VT_COLLMAG  , CODE1 | SPECIAL },
  { "1r"  , VT_COLLRED  , CODE1 | SPECIAL },
  { "1w"  , VT_COLLWHT  , CODE1 | SPECIAL },
  { "1y"  , VT_COLLYEL  , CODE1 | SPECIAL },

  { "<at>" , "@"       , CHAR },
  { "<tilde>" , "~"    , CHAR },
  { "<circumflex>" , "^", CHAR },
  { "<paragraph>" , "§", CHAR },
  { "<copyright>" , "©", CHAR },
  { "<registered>" , "®" , CHAR },
  { "<beta>" , "ß"  , CHAR },
  { "<1/2>" , "½"   , CHAR },
  { "<1/4>" , "¼"   , CHAR },
  { "<pound>" , "£" , CHAR },
  { "<3/4>" , "¾"   , CHAR },
  { "<aa>" , "å"    , CHAR },
  { "<ae>" , "ä"    , CHAR },
  { "<oe>" , "ö"    , CHAR },
  { "<AA>" , "Å"    , CHAR },
  { "<AE>" , "Ä"    , CHAR },
  { "<OE>" , "Ö"    , CHAR },

  { "\n"  , "", 0 } /* END*/

};


/* Sorts scr_codes table with bubble sort (preferably executed at runtime)
 * so that binary search can be used
 */
static void
scr_sort_codes()
{
  struct scrcol_replace_data val;
  int i, j;
  
  for (num_of_scr_codes = 0;  /* Count records */
       *scr_replace_table[num_of_scr_codes].code != '\n';
       ++num_of_scr_codes);
  
  for (j = num_of_scr_codes - 1; j > 0; --j)
for (i = 0; i <= j; ++i)
      if (strcmp(scr_replace_table[i].code, scr_replace_table[i+1].code) > 0) {
	val = scr_replace_table[i+1];
	scr_replace_table[i+1] = scr_replace_table[i];
	scr_replace_table[i] = val;
      }
}


/* Uses binary search - must have entries sorted -use scr_sort_codes */
const struct scrcol_replace_data *
scr_get_colcode(const char *str)
{
  int  bot = 0, top = num_of_scr_codes, mid, cmp;
  
  /* perform binary search on fsymrec-table */
  while (bot <= top) {
    mid = (bot + top) / 2;
    cmp = strncmp(str, scr_replace_table[mid].code, strlen(scr_replace_table[mid].code));
    if (!cmp)
      return (scr_replace_table + mid);
    if (cmp < 0)
      top = mid - 1;
    else
      bot = mid + 1;
  }
  
  return 0;
}

/* END: Code table ***********************************************************/


char *
scrcol_process(char **dest, const char **src, int flg, int maxlen)
{
  char *rptr = *dest;  /* Return pointer */
  register char *dptr = *dest, *kdptr = *dest;
  register const char* sptr = *src, *ksptr = *src;
  int len = 0, wordlen = 0;
  const struct scrcol_replace_data *scrcode;
  char full_word = FALSE;
  char copy_after = FALSE;

  if (!num_of_scr_codes)
    scr_sort_codes();

  if (maxlen < 25)       /* minimum maxlen */
    maxlen = 25;
  
  while (*sptr) {
    while (!full_word) {
      if (isalnum(*sptr) && *sptr != '§') {
	*dptr++ = *sptr++;
	++wordlen;
      } else 
	switch (*sptr) {
	case ' ': full_word = TRUE; copy_after = TRUE; break;
	  
	case '#':		/* Screen and color codes */
	case '§':
	  if (FLAGGED(SCRCOL_KEEPCODE)               ||
	      !(scrcode = scr_get_colcode(sptr + 1)) ||
	      (*sptr == '#' && IS_SET(scrcode->mode, SCRCOL_SPECIAL))) {
	    full_word = TRUE;
	    copy_after = TRUE;
	    /* A HACK WITH BITS BELOW -P */
	  } else if ((FLAGGED(CODE1 | CODE2) >= IS_SET(scrcode->mode, CODE1 | CODE2)) ||
		     (FLAGGED(IS_SET(scrcode->mode, CODEVT | CODEPC)))) {
	    /* Valid code */
	    strcpy(dptr, scrcode->replace);
	    sptr += strlen(scrcode->code) + 1;
	    dptr += strlen(scrcode->replace);
	    if (IS_SET(scrcode->mode, CHAR)) {
	      wordlen += strlen(scrcode->replace);
	      full_word = TRUE;
	    }
	  } else {
	    sptr += strlen(scrcode->code) + 1;
	  }
	  break;		/* End of screen and color codes */
	  
	case 0: full_word = TRUE; break;
	  
	case '\n': full_word = TRUE; break;
	  
	default:
	  *dptr++ = *sptr++;
	  ++wordlen;
	  break;
	}
      
      if (len + wordlen <= maxlen) {
	kdptr = dptr;
	ksptr = sptr;
      }
      
      if (wordlen >= maxlen)
	full_word = TRUE;	/* Force full_word for long words */
      
    } /* Found complete word */
    
    full_word = FALSE;
    
    if (wordlen >= maxlen) {
      *kdptr = 0;   /* EOS on dest */
      *src = ksptr;
      *dest = kdptr;
      return rptr;
    }
    
    if (len + wordlen >= maxlen) {
      **dest = 0;   /* EOS on dest */
      return rptr;
    }
    
    len += wordlen;
    wordlen = 0;
    *src = sptr;
    *dest = dptr;
    
    if (*sptr == '\n') {
      *src = sptr;
      **dest = 0;
      return rptr;
    }

    if (copy_after) {
      *dptr++ = *sptr++;
      ++wordlen;
      copy_after = FALSE;
    }
  }
  
  *src = sptr;
  *dest = dptr;
  **dest = 0; /* EOS on dest */
  return rptr;
}

char * scrcol_addcode_copy(char *dest, const char *src)
{
  const char * srcp = src;

  return scrcol_process(&dest, &srcp, CODE1 | CODE2, SCRCOL_MAX_STRING_SIZE);
}

char *
scrcol_copy(char *dest, const char *src, int flg, int maxlen)
{
  char *rptr = dest;
  
  if (!maxlen)
    maxlen = SCRCOL_MAX_STRING_SIZE;
  
  while (*src) {
    scrcol_process(&dest, &src, flg, maxlen);

    if (*src) {
      *dest++ = '\n';
      *dest++ = '\r';
    }
    if (*src == '\n') {
      if (*++src == '\r') ++src;
    } else
      for (; *src == ' '; ++src);
    
  }
  
  *dest = 0;

  return rptr;
}


const char *
scrcol_scrreg(int upper, int lower)
{
  static char scrcol_vt_scrreg[32];

  sprintf(scrcol_vt_scrreg, VT_SCRREG, upper, lower);

  return scrcol_vt_scrreg;
}


const char *
scrcol_curpos(int x, int y)
{
  static char scrcol_vt_curpos[32];

  sprintf(scrcol_vt_curpos, VT_CURPOS, y, x);

  return scrcol_vt_curpos;
}

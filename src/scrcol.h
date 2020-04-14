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

#ifndef __SCRCOL_H__
#define __SCRCOL_H__

struct scrcol_replace_data {
  const char *code;
  const char *replace;
  char mode;
};

#define SCRCOL_MAX_STRING_SIZE 16382

#define FLAGGED(bit)  ((flg) & (bit))

/* REMCODE IS ASSUMED IF NO ADD_XXX IS SET */
#define SCRCOL_REMCODE    0

#define SCRCOL_ADDCODE1   1
#define SCRCOL_ADDCODE2   2
#define SCRCOL_ADDCODEVT  4
#define SCRCOL_ADDCODEPC  8
#define SCRCOL_KEEPCODE  16

/* DO NOT SET THESE WHEN CALLING - USED INTERNALLY */
#define SCRCOL_SPECIAL   32
#define SCRCOL_CHAR      64

/* scrcol_process:
 * Lowlevel function - copies from src to dest, advancing the pointers
 * at the same time, adding, removing or keeping the screen and color codes
 * depending on colflg, counting the length of the copied words, and stopping
 * before the length exceeds maxlen.
 * Returns a pointer to the dest string
 */ 
char * scrcol_process(char **dest, const char **src, int flg, int maxlen);

/* scrcol_stripcode:
 * Given a string, this function removes all screen and color control codes
 * without processing them or replacing them.
 */
#ifdef __cplusplus
inline char *
scrcol_stripcode(char *str)
{ return scrcol_process(&str, &str, SCRCOL_REMCODE, SCRCOL_MAX_STRING_SIZE); }
#else
#define scrcol_stripcode(str) scrcol_process(&(str), &(str), SCRCOL_REMCODE, SCRCOL_MAX_STRING_SIZE)
#endif

/* scrcol_addcode_copy:
 * Given a string, this function copies from src to dest, adding all screen
 * and color control codes. Make sure dest is big enough since most color
 * codes are quite long.
 */
char * scrcol_addcode_copy(char *dest, const char *src);

/* scrcol_copy:
 * Generalised string copier, which adds screen and color codes while
 * copying, and wordwraps the string according to maxlen.
 * If maxlen is less than 25, it's set to 25. If maxlen is 0, no wrapping is
 * made.  Returns a pointer to dest.
 */
char * scrcol_copy(char *dest, const char *src, int flg, int maxlen);

/* scrcol_scrreg:
 * With given arguments x, y, this function will return a string with the
 * AT102 code to set scroll region for a screen.
 */
const char * scrcol_scrreg(int upper, int lower);

/* scrcol_curpos:
 * With given arguments x, y, this function will return a string with the
 * AT102 code to set the cursor position.
 */
const char * scrcol_curpos(int x, int y);

/* ANSI color codes */

/* TEXT COLOR - DARK */
#define VT_COLNRM  "\x1B[0;37m"
#define VT_COLRED  "\x1B[0;31m"
#define VT_COLGRN  "\x1B[0;32m"
#define VT_COLYEL  "\x1B[0;33m"
#define VT_COLBLU  "\x1B[0;34m"
#define VT_COLMAG  "\x1B[0;35m"
#define VT_COLCYN  "\x1B[0;36m"
#define VT_COLWHT  "\x1B[0;37m"

/* TEXT COLOR - LIGHT */ 
#define VT_COLLRED "\x1B[1;31m"
#define VT_COLLGRN "\x1B[1;32m"
#define VT_COLLYEL "\x1B[1;33m"
#define VT_COLLBLU "\x1B[1;34m"
#define VT_COLLMAG "\x1B[1;35m"
#define VT_COLLCYN "\x1B[1;36m"
#define VT_COLLWHT "\x1B[1;37m"
#define VT_COLLBLK "\x1B[1;30m"

/* TEXT BACKGROUND COLOR - DARK */
#define VT_COLBNRM  "\x1B[0;40m"
#define VT_COLBRED  "\x1B[0;41m"
#define VT_COLBGRN  "\x1B[0;42m"
#define VT_COLBYEL  "\x1B[0;43m"
#define VT_COLBBLU  "\x1B[0;44m"
#define VT_COLBMAG  "\x1B[0;45m"
#define VT_COLBCYN  "\x1B[0;46m"
#define VT_COLBWHT  "\x1B[0;47m"

/* TEXT BACKGROUND COLOR - LIGHT */
#define VT_COLBLNRM  "\x1B[1;40m"
#define VT_COLBLRED  "\x1B[1;41m"
#define VT_COLBLGRN  "\x1B[1;42m"
#define VT_COLBLYEL  "\x1B[1;43m"
#define VT_COLBLBLU  "\x1B[1;44m"
#define VT_COLBLMAG  "\x1B[1;45m"
#define VT_COLBLCYN  "\x1B[1;46m"
#define VT_COLBLWHT  "\x1B[1;47m"

#define VT_COLOFF    "\x1B[0;37m"
#define VT_COLNULL   ""

/* VT102 character codes - Petrus */

#define VT_CLS       "\033[H\033[J"  /* clearscreen */
#define VT_SCRREG    "\033[%d;%dr"   /* scroll region %d has to be replaced*/
#define VT_CURPOS    "\033[%d;%df"   /* curspos  y,x  %d need replacing */

#define VT_BOLD      "\033[1m"       /* Bold      */
#define VT_DIM       "\033[2m"       /* Low Intensity */
#define VT_ULINE     "\033[4m"       /* Underline */
#define VT_BLINK     "\033[5m"       /* Blink     */
#define VT_REVERSE   "\033[7m"       /* Reverse   */
#define VT_NOBOLD    "\033[21m"      /* Cancel bold      */
#define VT_NOULINE   "\033[24m"      /* Cancel underline */
#define VT_NOBLINK   "\033[25m"      /* Cancel blink     */
#define VT_NOREVERSE "\033[27m"      /* Cancel reverse   */
#define VT_OFF       "\033[0m"       /* Attributes off   */

#define VT_DELEOL    "\033[K"        /* Erase to End Of Line               */
#define VT_DELBOL    "\033[1K"       /* Erase to Beginning Of Line         */
#define VT_DELEOS    "\033[J"        /* Erase to End Of Screen             */
#define VT_DELBOS    "\033[1J"       /* Erase to Begginging Of Screen      */
#define VT_DELCHARS  "\033[%dP"      /* Del # chars at right - %d replaced */
#define VT_INSLINES  "\033[%dL"      /* Insert lines - %d need replacing   */
#define VT_DELLINES  "\033[%dM"      /* Delete lines - %d need replacing   */

#define VT_CURSON    "\033[025h"     /* Cursor on */
#define VT_CURSOFF   "\033[025l"     /* Cursor off */

/* SPECIAL */

#define VT_BEEP      "\007\007"      /* Send BEEP to vt terms */

#endif



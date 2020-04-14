/* ************************************************************************
*   File: screen.h                                      Part of EliteMUD  *
*  Usage: header file with ANSI color codes for online color              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#define KNRM  "\x1B[0;37m"
#define KRED  "\x1B[0;31m"
#define KGRN  "\x1B[0;32m"
#define KYEL  "\x1B[0;33m"
#define KBLU  "\x1B[0;34m"
#define KMAG  "\x1B[0;35m"
#define KCYN  "\x1B[0;36m"
#define KWHT  "\x1B[0;37m"

#define KLBLK  "\x1B[1;30m"
#define KLRED  "\x1B[1;31m"
#define KLGRN  "\x1B[1;32m"
#define KLYEL  "\x1B[1;33m"
#define KLBLU  "\x1B[1;34m"
#define KLMAG  "\x1B[1;35m"
#define KLCYN  "\x1B[1;36m"
#define KLWHT  "\x1B[1;37m"

#define KBNRM  "\x1B[40m"
#define KBRED  "\x1B[41m"
#define KBGRN  "\x1B[42m"
#define KBYEL  "\x1B[43m"
#define KBBLU  "\x1B[44m"
#define KBMAG  "\x1B[45m"
#define KBCYN  "\x1B[46m"
#define KBWHT  "\x1B[47m"

#define KNUL   ""
/* vt102 character codes - Petrus */

#define VTCLS       "\033[H\033[J"    /* clearscreen */
#define VTSCRREG    "\033[%d;%dr"     /* scroll region %d has to be replaced */
#define VTCURPOS    "\033[%d;%df"     /* curspos  y,x  %d need replacing */

#define VTBOLD      "\033[1m"
#define VTULINE     "\033[4m"       /* Underline */
#define VTBLINK     "\033[5m"       /* Blink     */
#define VTREVERSE   "\033[7m"       /* Reverse   */
#define VTNOULINE   "\033[24m"      /* Cancel underline */
#define VTNOBLINK   "\033[25m"      /* Cancel blink  */
#define VTNOREVERSE "\033[27m"      /* Cancel reverse */
#define VTOFF       "\033[0m"       /* Attributes off */

#define VTDELEOL    "\033[K"          /* Erase to End Of Line */
#define VTDELBOL    "\033[1K"         /* Erase to Beginning Of Line */
#define VTDELEOS    "\033[J"          /* Erase to End Of Screen */
#define VTDELBOS    "\033[1J"         /* Erase to Begginging Of Screen */
#define VTDELCHARS  "\033[%dP"        /* Del chars at right - %d replaced */
#define VTINSLINES  "\033[%dL"        /* Insert lines - %d need replacing */
#define VTDELLINES  "\033[%dM"        /* Delete lines - %d need replacing */

#define VTCURSON    "\033[025h"  /* Cursor on */
#define VTCURSOFF   "\033[025l"  /* Cursor off */

#define CCBLD(ch)    ((PRF_FLAGGED((ch), PRF_DISPVT))?VTBOLD:KNUL)
#define CCUDL(ch)    ((PRF_FLAGGED((ch), PRF_DISPVT))?VTULINE:KNUL)
#define CCREV(ch)    ((PRF_FLAGGED((ch), PRF_DISPVT))?VTREVERSE:KNUL)
#define CCBLK(ch)    ((PRF_FLAGGED((ch), PRF_IBM_PC))?VTBLINK:KNUL)

/* conditional color.  pass it a pointer to a char_data and a color level. */
#define C_OFF	0
#define C_SPR	1
#define C_NRM	2
#define C_CMP	3
#define _clrlevel(ch) ((PRF_FLAGGED((ch), PRF_COLOR_1) ? 1 : 0) + \
		       (PRF_FLAGGED((ch), PRF_COLOR_2) ? 2 : 0))
#define clr(ch,lvl) (_clrlevel(ch) >= (lvl))
#define CCNRM(ch,lvl)  (clr((ch),(lvl))?KNRM:KNUL)
#define CCRED(ch,lvl)  (clr((ch),(lvl))?KRED:KNUL)
#define CCGRN(ch,lvl)  (clr((ch),(lvl))?KGRN:KNUL)
#define CCYEL(ch,lvl)  (clr((ch),(lvl))?KYEL:KNUL)
#define CCBLU(ch,lvl)  (clr((ch),(lvl))?KBLU:KNUL)
#define CCMAG(ch,lvl)  (clr((ch),(lvl))?KMAG:KNUL)
#define CCCYN(ch,lvl)  (clr((ch),(lvl))?KCYN:KNUL)
#define CCWHT(ch,lvl)  (clr((ch),(lvl))?KWHT:KNUL)

#define CCLBLK(ch,lvl)  (clr((ch),(lvl))?KLBLK:NULL)
#define CCLRED(ch,lvl)  (clr((ch),(lvl))?KLRED:KNUL)
#define CCLGRN(ch,lvl)  (clr((ch),(lvl))?KLGRN:KNUL)
#define CCLYEL(ch,lvl)  (clr((ch),(lvl))?KLYEL:KNUL)
#define CCLBLU(ch,lvl)  (clr((ch),(lvl))?KLBLU:KNUL)
#define CCLMAG(ch,lvl)  (clr((ch),(lvl))?KLMAG:KNUL)
#define CCLCYN(ch,lvl)  (clr((ch),(lvl))?KLCYN:KNUL)
#define CCLWHT(ch,lvl)  (clr((ch),(lvl))?KLWHT:KNUL)

#define CCBNRM(ch,lvl)  (clr((ch),(lvl))?KBNRM:KNUL)
#define CCBRED(ch,lvl)  (clr((ch),(lvl))?KBRED:KNUL)
#define CCBGRN(ch,lvl)  (clr((ch),(lvl))?KBGRN:KNUL)
#define CCBYEL(ch,lvl)  (clr((ch),(lvl))?KBYEL:KNUL)
#define CCBBLU(ch,lvl)  (clr((ch),(lvl))?KBBLU:KNUL)
#define CCBMAG(ch,lvl)  (clr((ch),(lvl))?KBMAG:KNUL)
#define CCBCYN(ch,lvl)  (clr((ch),(lvl))?KBCYN:KNUL)
#define CCBWHT(ch,lvl)  (clr((ch),(lvl))?KBWHT:KNUL)

#define COLOR_LEV(ch) (_clrlevel(ch))

#define QNRM CCNRM(ch,C_SPR)
#define QRED CCRED(ch,C_SPR)
#define QGRN CCGRN(ch,C_SPR)
#define QYEL CCYEL(ch,C_SPR)
#define QBLU CCBLU(ch,C_SPR)
#define QMAG CCMAG(ch,C_SPR)
#define QCYN CCCYN(ch,C_SPR)
#define QWHT CCWHT(ch,C_SPR)


/* ************************************************************************
*  File: ident.h                              				  *
*                                                                         *
*  Usage: Header file containing stuff required for rfc 931/1413 ident    *
*         lookups                                                         *
*                                                                         *
*  Written by Eric Green (thrytis@imaxx.net)				  *
************************************************************************ */

void ident_start(struct descriptor_data *d, long addr);
void ident_check(struct descriptor_data *d);
int waiting_for_ident(struct descriptor_data *d);

extern int ident;

#define IDENT_STATE(d) ((d)->ident_state) 

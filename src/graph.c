/* ************************************************************************
*   File: graph.c                                       Part of EliteMUD  *
*  Usage: various graph algorithms                                        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */


#define TRACK_THROUGH_DOORS

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "functions.h"

/* Externals */
extern int top_of_world;
extern const char *dirs[];
extern struct room_data **world; 

struct bfs_queue_struct {
   sh_int room;
   char   dir;
   ubyte  steps;             /* num of steps to get here -Petrus */
   struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = 0, *queue_tail = 0;

/* Utility macros */
#define MARK(room) (SET_BIT(world[room]->room_flags, BFS_MARK))
#define UNMARK(room) (REMOVE_BIT(world[room]->room_flags, BFS_MARK))
#define IS_MARKED(room) (IS_SET(world[room]->room_flags, BFS_MARK))
#define TOROOM(x, y) (world[(x)]->dir_option[(y)]->to_room)
#define IS_CLOSED(x, y) (IS_SET(world[(x)]->dir_option[(y)]->exit_info, EX_CLOSED))


#define DIRFROMMARK(room, dir)  (world[(room)]->dir_from = (dir))
#define ROOMFROMMARK(room, from) (world[(room)]->room_from = (from))

#define FROMROOM(room)   (world[(room)]->room_from)

#ifdef TRACK_THROUGH_DOORS
#define VALID_EDGE(x, y) (world[(x)]->dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
                          (!IS_SET(world[TOROOM(x, y)]->room_flags, GODROOM | PKOK | ARENA | NO_TRACK)) && \
			  (!IS_MARKED(TOROOM(x, y))))
#else
#define VALID_EDGE(x, y) (world[(x)]->dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
                          (!IS_SET(world[TOROOM(x, y)]->room_flags, GODROOM | PKOK | ARENA | NO_TRACK)) && \
			  (!IS_CLOSED(x, y)) &&		\
			  (!IS_MARKED(TOROOM(x, y))))
#endif

void bfs_enqueue(sh_int room, char dir, ubyte steps)
{
   struct bfs_queue_struct *curr;

   CREATE(curr, struct bfs_queue_struct, 1);
   curr->room = room;
   curr->dir = dir;
   curr->steps = steps;
   curr->next = 0;

   if (queue_tail) {
      queue_tail->next = curr;
      queue_tail = curr;
   } else
      queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
   struct bfs_queue_struct *curr;

   curr = queue_head;

   if (!(queue_head = queue_head->next))
      queue_tail = 0;
   free(curr);
}


void bfs_clear_queue(void) 
{
   while (queue_head)
      bfs_dequeue();
}


void stack_push(struct track_stack_data **stk, byte dir, sh_int room)
{
  struct track_stack_data *tmp;
  
  CREATE(tmp, struct track_stack_data, 1);  

  tmp->dir = dir;
  tmp->room = room;
  tmp->next = *stk;

  *stk = tmp;
}

int stack_pop(struct track_stack_data **stk)
{
  struct track_stack_data *tmp;
  int dir;

  if (!(*stk))
    return -1;

  tmp = *stk;

  *stk = tmp->next;

  dir = tmp->dir;

  free(tmp);

  return dir;
}

void free_stack(struct track_stack_data *stk)
{
  if (stk) {
    if (stk->next)
      free_stack(stk->next);
    free(stk);
  }
}


struct track_stack_data *
build_track_stack(sh_int src, sh_int target)
{
  struct track_stack_data *stack;
  sh_int temp = target;

  stack = NULL;

  while (temp != src) {
    stack_push(&stack, world[temp]->dir_from, FROMROOM(temp));

    temp = FROMROOM(temp);
  }
  
  return stack;
}



struct track_stack_data *
make_track_stack(sh_int src, sh_int target, ubyte maxsteps)
{
  int curr_dir;
  sh_int curr_room;
  
  if (src < 0 || src > top_of_world || target < 0 || target > top_of_world) {
    log("Illegal value passed to find_first_step (graph.c)");
    return NULL;
  }
  
  if (src == target)
    return NULL;
  
  /* clear marks first */
  for (curr_room = 0; curr_room <= top_of_world; curr_room++)
    UNMARK(curr_room);
  
  MARK(src);
  
  /* first, enqueue the first steps, saving which direction we're going. */
  for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
    if (VALID_EDGE(src, curr_dir)) {
      MARK(TOROOM(src, curr_dir));
      DIRFROMMARK(TOROOM(src, curr_dir), curr_dir);
      ROOMFROMMARK(TOROOM(src, curr_dir), src);
      bfs_enqueue(TOROOM(src, curr_dir), curr_dir, 1);
    }
  
  /* now, do the classic BFS. */
  while (queue_head) {
    if (queue_head->steps > maxsteps) {
      bfs_clear_queue();
      return NULL;
    } else if (queue_head->room == target) {
      curr_dir = queue_head->dir;
      bfs_clear_queue();
      return build_track_stack(src, target);
    } else {
      for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
	if (VALID_EDGE(queue_head->room, curr_dir)) {
	  MARK(TOROOM(queue_head->room, curr_dir));
	  DIRFROMMARK(TOROOM(queue_head->room, curr_dir), curr_dir);
	  ROOMFROMMARK(TOROOM(queue_head->room, curr_dir), queue_head->room);
	  bfs_enqueue(TOROOM(queue_head->room, curr_dir),queue_head->dir, queue_head->steps + 1);
	}
      bfs_dequeue();
    }
  }
  
  return NULL;
}


/************************************************************************
*  Functions and Commands which use the above fns		        *
************************************************************************/

void track_check(struct char_data *ch)
{
  char buf[256];
  
  if (IS_NPC(ch) || !ch->trackdir)
    return;

  if (IN_ROOM(ch) == ch->trackdir->room) {
    sprintf(buf, "You sense a trail %s from here!\r\n",
	    dirs[stack_pop(&(ch->trackdir))]);
    send_to_char(buf, ch);
  }
}


int perform_track(struct char_data *ch, sh_int target, sbyte steps)
{
  if (!ch || IN_ROOM(ch) == NOWHERE)
    return 0;

  free_stack(ch->trackdir);

  ch->trackdir = make_track_stack(IN_ROOM(ch), target, steps);
  
  if (ch->trackdir)
    return 1;
  else
    return 0;
}


ACMD(do_track)
{
  struct char_data *vict;
  
  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("Whom are you trying to track?\r\n", ch);
    return;
  }
  
  if (!(vict = get_char_vis(ch, arg))) {
    /* send_to_char("No-one around by that name.\r\n", ch);  */
    send_to_char("You can't sense a trail from here.\r\n", ch);
    
    free_stack(ch->trackdir);
    ch->trackdir = NULL;

    return;
  }

  /* Can't track another PKOK character - might need to prevent
     normal players from tracking as well due to loophole - Bod */
  if (PLR_FLAGGED(vict, PLR_PKOK) && PLR_FLAGGED(ch, PLR_PKOK)) {
    send_to_char("You can't sense a trail from here.\r\n", ch);
    return;
  }

  if (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOTRACK)) {
    send_to_char("You can't sense a trail from here.\r\n", ch);
    return;
  }
  
  if (IN_ROOM(vict) == IN_ROOM(ch)) {
    act("$N is right here!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }

  perform_track(ch, IN_ROOM(vict), GET_SKILL(ch, SKILL_TRACK)/2);
  
  if (!ch->trackdir) {
/*
    sprintf(buf, "You can't sense a trail to %s from here.\r\n",
	    HMHR(vict));
*/

    sprintf(buf, "You can't sense a trail from here.\r\n");
    send_to_char(buf, ch);
    return;  
  }

  track_check(ch);
}


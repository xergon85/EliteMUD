/* ************************************************************************
*   File: comm.c                                        Part of EliteMUD  *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */

#define __COMM_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "limits.h"
#include "screen.h"
#include "functions.h"
#include "scrcol.h"
#include "ident.h"

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif

/* externs */
extern int	restrict;
extern int	mini_mud;
extern int	no_rent_check;
extern FILE	*player_fl;
extern int	DFLT_PORT;
extern char	*DFLT_DIR;
extern int	MAX_PLAYERS;
extern int	MAX_DESCRIPTORS_AVAILABLE;

extern struct room_data **world;	/* In db.c */
extern struct index_data *mob_index;
extern int	top_of_world;		/* In db.c */
extern struct time_info_data time_info;	/* In db.c */
extern char	help[];

/* local globals */
struct descriptor_data *descriptor_list, *next_to_process;
struct txt_block *bufpool = 0;	/* pool of large output buffers */
int	buf_largecount;		/* # of large buffers which exist */
int	buf_overflows;		/* # of overflows of output */
int	buf_switches;		/* # of switches from small to large buf */
int	elite_shutdown = 0;	/* clean shutdown */
int	elite_reboot = 0;	/* reboot the game after a shutdown */
int	no_specials = 0;	/* Suppress ass. of special routines */
int	last_desc = 0;		/* last unique num assigned to a desc. */
int	mother_desc = 0;	/* file desc of the mother connection */
int	maxdesc;		/* highest desc num used */
int	avail_descs;		/* max descriptors available */
int	tics = 0;		/* for extern checkpointing */
int     cmds_executed = 0;      /* Commands executed since last */
int     plr_cmds_executed = 0;  /* Commands by players */
int     plr_cmds_per_sec = 0;   /* Commands per sec by players */
int     cmds_per_sec = 0;       /* Commands per sec */
long    usec_spent_per_sec = 0; /* A measure of how much mud is calculating */
float   elite_efficiency = 0.0; /* The efficiency mud need to run at in % */
bool    MOBTrigger = TRUE;	/* For MOBProgs */
int     is_quest = 0;           /* For quest channel */

extern int	nameserver_is_slow;	/* see config.c */
extern int	auto_save;		/* see config.c */
extern int	autosave_time;		/* see config.c */
extern struct history_list *global_history[];


struct timeval null_time;
int port;

long mud_time[6] = {0, 0, 0, 0, 0, 0}; /* for internal mud info */

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

int	main(int argc, char **argv)
{
    char	buf[512];
    int	pos = 1;
    char	*dir;
    
    port = DFLT_PORT;
    dir = DFLT_DIR;
    
    while ((pos < argc) && (*(argv[pos]) == '-')) {
	switch (*(argv[pos] + 1)) {
	case 'd':
	    if (*(argv[pos] + 2))
		dir = argv[pos] + 2;
	    else if (++pos < argc)
		dir = argv[pos];
	    else {
		log("Directory arg expected after option -d.");
		exit(0);
	    }
	    break;
	case 'm':
	    mini_mud = 1;
	    no_rent_check = 1;
	    log("Running in minimized mode & with no rent check.");
	    break;
	case 'q':
	    no_rent_check = 1;
	    log("Quick boot mode -- rent check supressed.");
	    break;
	case 'r':
	    restrict = 1;
	    log("Restricting game -- no new players allowed.");
	    break;
	case 's':
	    no_specials = 1;
	    log("Suppressing assignment of special routines.");
	    break;
	default:
	    sprintf(buf, "SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
	    log(buf);
	    break;
	}
	pos++;
    }
    
    if (pos < argc) {
	if (!isdigit(*argv[pos])) {
	    fprintf(stderr, "Usage: %s [-m] [-q] [-r] [-s] [-d pathname] [ port # ]\n", argv[0]);
	    exit(0);
	}
	else if ((port = atoi(argv[pos])) <= 1024) {
	    printf("Illegal port #\n");
	    exit(0);
	}
    }
    
    sprintf(buf, "Running game on port %d.", port);
    log(buf);
    
    if (chdir(dir) < 0) {
	perror("Fatal error changing to data directory");
	exit(0);
    }
    
    sprintf(buf, "Using %s as data directory.", dir);
    log(buf);
    
    circle_srandom(time(0));
    run_the_game(port);
    return(0);
}


int get_avail_descs(void)
{
    int max_descs = 0;
    
    /*
     * First, we'll try using getrlimit/setrlimit.  This will probably work
     * on most systems.
     */
#if defined (RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#if !defined(RLIMIT_NOFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
{
    struct rlimit limit;
    
    getrlimit(RLIMIT_NOFILE, &limit);
    if (limit.rlim_max == RLIM_INFINITY)
	max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else
	max_descs = limit.rlim_max;
    
    limit.rlim_cur = max_descs;
    
    setrlimit(RLIMIT_NOFILE, &limit);
}
#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
  max_descs = OPEN_MAX;		/* Uh oh.. rlimit didn't work, but we have
				 * OPEN_MAX */
#else
  /*
   * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
   * use the POSIX sysconf() function.  (See Stevens' _Advanced Programming
   * in the UNIX Environment_).
   */
  errno = 0;
  if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
      if (errno == 0)
	  max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
      else {
	  perror("Error calling sysconf");
	  exit(1);
      }
  }
#endif

  max_descs = MIN(MAX_PLAYERS, max_descs - NUM_RESERVED_DESCS);

  if (max_descs <= 0) {
      log("Non-positive max player limit!");
      exit(1);
  }
  sprintf(buf, "Setting player limit to %d", max_descs);
  log(buf);
  return max_descs;
}


/* Init sockets, run game, and cleanup sockets */
void	run_the_game(int port)
{
    int	s, i;
    
    void	signal_setup(void);
    
    descriptor_list = NULL;
    
    log("Signal trapping.");
    signal_setup();
    
    boot_db();
    
    log("Opening mother connection.");
    mother_desc = s = init_socket(port);
    avail_descs = get_avail_descs();

    log("Entering game loop.");
    
    for (i = 0; i < CHAN_GLOBAL_MAX; i++)
	 global_history[i] = NULL;

    game_loop(s);

    log("Saving :: Players.");
    Crash_save_all();
    
    close_sockets(s);
    fclose(player_fl);
    
    if (elite_reboot) {
	log("Rebooting.");
	exit(52);            /* what's so great about HHGTTG, anyhow? */
    }
    
    log("Normal termination of game.");
}

long mud_time_spent(struct timeval time_from)
{
  struct timeval temp, timespent;
  long usec_per_sec;

  gettimeofday(&temp, (struct timezone *) 0);
  timespent = timediff(&temp, &time_from);
  usec_per_sec = timespent.tv_sec * 1000000 + timespent.tv_usec; 

  return usec_per_sec;
}

  
/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void	game_loop(int s)
{
  fd_set input_set, output_set, exc_set;
  struct timeval last_time, now, timespent, timeout, opt_time;
  char comm[MAX_INPUT_LENGTH];
  char prompt[MAX_INPUT_LENGTH];
  struct descriptor_data *point, *next_point;
  int	pulse = 0, mins_since_crashsave = 0;
  int	sockets_connected, sockets_playing;
  bool disp;
  char	buf[100];

  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  gettimeofday(&last_time, (struct timezone *) 0);
  gettimeofday(&now, (struct timezone *) 0);

  /* Main loop */
  while (!elite_shutdown) {
    /* Sleep if we don't have any connections */
    if (descriptor_list == NULL) {
      log("No connections.  Going to sleep. Zzzz");
      FD_ZERO(&input_set);
      FD_SET(s, &input_set);
      if(select(s + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0)
	{
	  if (errno == EINTR)
	    log("Waking up to process signal.");
	  else
	    perror("Select coma");
	} else
	  log("New connection waking me up.");
      gettimeofday(&last_time, (struct timezone *) 0);
      gettimeofday(&now, (struct timezone *) 0);
      usec_spent_per_sec = 0;
      mud_time[5] = 0;
    }
	
    /* Setup the input, output and exception sets for select() */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(s, &input_set);
    maxdesc = s;
    for (point = descriptor_list; point; point = point->next) {
      if(point->descriptor > maxdesc)
	maxdesc = point->descriptor;
      FD_SET(point->descriptor, &input_set);
      FD_SET(point->descriptor, &output_set);
      FD_SET(point->descriptor, &exc_set);
    }
       
    do {
      errno = 0;		/* clear error condition */
	    
      mud_time[5] += mud_time_spent(now);
      /* figure out for how long we have to sleep */
      gettimeofday(&now, (struct timezone *) 0);
      timespent = timediff(&now, &last_time);
      timeout = timediff(&opt_time, &timespent);
	    
      usec_spent_per_sec += timespent.tv_sec * 1000000 + timespent.tv_usec; 
      /* sleep until the next 0.1 second mark */
      if (select(0,(fd_set *) 0,(fd_set *) 0,(fd_set *) 0, &timeout) < 0)
	if (errno != EINTR) {
	  perror("Select sleep");
	  exit(1);
	}
    } while (errno);
	
    /* record the time for the next pass */
    gettimeofday(&last_time, (struct timezone *) 0);
	
    /* poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("Select poll");
      return;
    }

    /* Respond to whatever might be happening */
	
    /* New connection? */
    if (FD_ISSET(s, &input_set))
      if (new_descriptor(s) < 0)
	perror("New connection");
	
    /* kick out the freaky folks */
    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &exc_set)) {
	FD_CLR(point->descriptor, &input_set);
	FD_CLR(point->descriptor, &output_set);
	close_socket(point);
      }
    }
	
    /* Process descriptors with input pending */
    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &input_set))
	if (process_input(point) < 0)
	  close_socket(point);
    }

    /* Process descriptors with ident pending */
    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      
      if (waiting_for_ident(point))
	ident_check(point);
    }    

    /* process_commands; */
    for (point = descriptor_list; point; point = next_to_process) {
      next_to_process = point->next;
	    
      if ((--(point->wait) <= 0)) { 
	disp = TRUE;
	if (get_from_q(&point->input, comm)) {
	  point->wait = 1;
	  point->prompt_mode = 1;
	} else if (*point->mult_input && point->connected == CON_PLYNG) {
	  strcpy(comm, point->mult_input);
	  point->wait = 3;
	} else if (*point->num_input && point->connected == CON_PLYNG) {
	  strcpy(comm, point->num_input);
	  point->wait = 3;
	} else
	  disp = FALSE;
	if (disp) {
	  if (point->character && point->connected == CON_PLYNG && 
	      point->character->specials.was_in_room != NOWHERE) {
	    if (point->character->in_room != NOWHERE)
	      char_from_room(point->character);
	    char_to_room(point->character, 
			 point->character->specials.was_in_room);
	    point->character->specials.was_in_room = NOWHERE;
	    act("$n has returned.", 	TRUE, point->character, 0, 0, TO_ROOM);
	  }
		    
	  point->idle_tics = 0;
	  if (point->character)
	    point->character->specials.timer = 0;
	  
	  if (point->showstr_count)
	    show_string(point, comm);
	  else if (point->str)
	    string_add(point, comm);
	  else if (!point->connected) {
	    if (OCSMODE(point->character))
	      ocs_main(point->character, comm);
	    else {
	      if (PRF_FLAGGED(point->character, PRF_DISPANSI)) {
		sprintf(prompt, VTCURPOS VTDELEOS, 
			GET_SCRLEN(point->character), 1);
		write_to_descriptor(point->descriptor, prompt);
	      }
	      command_interpreter(point->character, comm);
	    }	    
	  } else
	    nanny(point, comm);
	} 
      }
    }
    
    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &output_set) && *(point->output)) {
	if (process_output(point) < 0)
	  close_socket(point);
	else
	  point->prompt_mode = 1;
      }
    }
	
    /* kick out the Phreaky Pholks II  -JE */
    for (point = descriptor_list; point; point = next_to_process) {
      next_to_process = point->next;
      if (STATE(point) == CON_CLOSE) 
	close_socket(point);
    }
	
    /* give the people some prompts */
    for (point = descriptor_list; point; point = point->next)
      if (point->prompt_mode) {
	if (point->showstr_count) {
	  if (PRF_FLAGGED(point->character, PRF_DISPVT)) {
	    sprintf(prompt, VTCURPOS, 
		    GET_SCRLEN(point->character)-4, 1);
	  } else 
	    *prompt = '\0';
	  sprintf(prompt, "\r[ #g(return) continue  #m(q)uit  #r(b)ack  #b(r)efresh  #y(##)Page %d of %d #N ] >", point->showstr_page, point->showstr_count);
	  write_to_descriptor(point->descriptor,
			      convert_to_color(point, prompt));
	} else   if (point->str) {
	  if (PRF_FLAGGED(point->character, PRF_DISPVT)) {
	    sprintf(prompt, VTCURPOS "] ", 
		    GET_SCRLEN(point->character)-4, 1);
	    write_to_descriptor(point->descriptor, prompt);
	  } else
	    write_to_descriptor(point->descriptor, "] ");
	} else if (!point->connected) {
	  if (PRF_FLAGGED(point->character, PRF_DISPVT)) {
	    stats_to_screen(point);
	    *prompt = '\0';
	    disp = 0;
	  } else if (GET_INVIS_LEV(point->character))
	    sprintf(prompt, "§ci%d>§N ", GET_INVIS_LEV(point->character));
	  else {
	    strcpy(prompt, "§y<");
	    disp = FALSE;
	    
	    if (PRF_FLAGGED(point->character, PRF_DISPHP)) {
	      sprintf(prompt, "%s §g%dHp", prompt, GET_HIT(point->character));
	      disp = TRUE;
	    }
	    
	    if (PRF_FLAGGED(point->character, PRF_DISPMANA)) {
	      sprintf(prompt, "%s §m%dMn", prompt, GET_MANA(point->character));
	      disp = TRUE;
	    }
	    
	    if (PRF_FLAGGED(point->character, PRF_DISPMOVE)) {
	      sprintf(prompt, "%s §c%dMv", prompt, GET_MOVE(point->character));
	disp = TRUE;
	    }
	    
	    if(((point->character)->specials.fighting) &&
	       PRF_FLAGGED(point->character, PRF_DISPVIC))
	      {
		sprintf(prompt, "%s #N(%s)",prompt,diag_to_prompt((point->character)->specials.fighting, 0));
		disp = TRUE;
	      }
	    
	    if (disp)
	      strcat(prompt, " §y>§N ");
	    else
	      strcpy(prompt, "§y>§N ");
	    
	  }
	  
	  write_to_descriptor(point->descriptor,
			      convert_to_color(point, prompt));
	}
	point->prompt_mode = 0;
      }
	
    /* handle heartbeat stuff */
    /* Note: pulse now changes every 1/8 sec  */
	
    mud_time[0] += mud_time_spent(last_time);

    /* check for negative pulse condition */
    if (pulse < 0) {
      sprintf(buf, "SYSERR: pulse is negative (%d).  Resetting.", pulse);
      log(buf);
      pulse = 1;
    } else 
      pulse++;

    if (!(pulse % PULSE_ZONE)) {
      gettimeofday(&now, (struct timezone *) 0);
      zone_update();
      mud_time[1] += mud_time_spent(now);
    }

    if (!(pulse % PULSE_MOBILE)) { 
      gettimeofday(&now, (struct timezone *) 0);
      mobile_activity();
      mud_time[2] += mud_time_spent(now);
    }

    if (!(pulse % PULSE_VIOLENCE)) {
      gettimeofday(&now, (struct timezone *) 0);
      perform_violence();
      mud_time[3] += mud_time_spent(now);
    }

    if (!(pulse % PULSE_GAIN)) {
      gettimeofday(&now, (struct timezone *) 0);
      check_gain();
      mud_time[5] += mud_time_spent(now);
    }
	
    if (!(pulse % (10 * PASSES_PER_SEC))) {
      plr_cmds_per_sec = plr_cmds_executed/10;
      cmds_per_sec = cmds_executed/10;
      cmds_executed = 0;
      plr_cmds_executed = 0;

      elite_efficiency = (float)usec_spent_per_sec/10000000.0;
      usec_spent_per_sec = 0;
    }

    if (!(pulse % (60 * PASSES_PER_SEC))) {
      sprintf(buf2, "Mud Efficiency (1 minute):\r\n#NPlayers: #C%5.2f%%  #NZones: #C%5.2f%%  #NMobiles: #C%5.2f%%\r\n#NViolence: #C%5.2f%%  #NTick: #C%5.2f%%  #NOther: #C%5.2f%%\r\n#G",  
	      (float)mud_time[0]/600000.0,
	      (float)mud_time[1]/600000.0,
	      (float)mud_time[2]/600000.0,
	      (float)mud_time[3]/600000.0,
	      (float)mud_time[4]/600000.0,
	      (float)mud_time[5]/600000.0);
      mudlog(buf2, CMP, LEVEL_ADMIN, FALSE);
      mud_time[0] = 0;
      mud_time[1] = 0;
      mud_time[2] = 0;
      mud_time[3] = 0;
      mud_time[4] = 0;
      mud_time[5] = 0;
    }

    if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) {
      gettimeofday(&now, (struct timezone *) 0);
      mudlog("Tick Tock" , CMP, LEVEL_DEITY, FALSE);
      weather_and_time(1);
      affect_update();
      point_update();
      if (fflush(player_fl))
	mudlog("SYSERR: Player file couldn't be updated", BRF, LEVEL_DEITY, TRUE);
      
      /* kick out the freaky (hung) folks */
      for (point = descriptor_list; point; point = next_point) {
	next_point = point->next;
	if (STATE(point) != CON_PLYNG) {
	  point->idle_tics++;
	  if (IDENT_STATE(point) != ID_NONE)
	    point->ident_idle++;
	  if (point->idle_tics > 4) {
	    sprintf(buf, "Desc num %d (%s) disconnected. (TIMEOUT)",
		    point->desc_num,
		    (point->character ? GET_NAME(point->character) : "<Unknown>"));
	    mudlog(buf, CMP, LEVEL_DEITY, FALSE);
	    close_socket(point);
	  }
	}
      }
     
      mud_time[4] += mud_time_spent(now);
    }

    gettimeofday(&now, (struct timezone *) 0);

    if (auto_save)
      if (!(pulse % (60 * PASSES_PER_SEC))) /* one minutes .*/
	if (++mins_since_crashsave >= autosave_time) {
	  mins_since_crashsave = 0;
	  Crash_save_all();
	  mudlog("Auto Crash Saving..." , CMP, LEVEL_DEITY, FALSE);
	}
	
    if (!(pulse % (600 * PASSES_PER_SEC))) { /* ten minutes */
      sockets_connected = sockets_playing = 0;
	    
      for (point = descriptor_list; point; point = next_point) {
	next_point = point->next;
	sockets_connected++;
	if (!point->connected)
	  sockets_playing++;
      }
	    
      sprintf(buf, "nusage: %-3d sockets connected, %-3d sockets playing",
	      sockets_connected,
	      sockets_playing);
      log(buf);
	    
#ifdef RUSAGE
      {
	struct rusage rusagedata;
	    
	getrusage(0, &rusagedata);
	sprintf(buf, "rusage: %d %d %d %d %d %d %d",
		rusagedata.ru_utime.tv_sec,
		rusagedata.ru_stime.tv_sec,
		rusagedata.ru_maxrss,
		rusagedata.ru_ixrss,
		rusagedata.ru_ismrss,
		rusagedata.ru_idrss,
		rusagedata.ru_isrss);
	log(buf);
      }
#endif
	    
    }
	
    if (pulse >= (30 * 60 * PASSES_PER_SEC)) {
      pulse = 0;
      afs_force_save();
      check_reboot();
    }
    mud_time[5] += mud_time_spent(now);
    tics++;			/* tics since last checkpoint signal */
  }
}


/* ******************************************************************
*  general utility stuff (for local use)			    *
****************************************************************** */

int	get_from_q(struct txt_q *queue, char *dest)
{
   struct txt_block *tmp;

   /* Q empty? */
   if (!queue->head)
      return(0);

   tmp = queue->head;
   strcpy(dest, queue->head->text);
   queue->head = queue->head->next;

   free(tmp->text);
   free(tmp);

   return(1);
}



/* Add a new string to a player's output queue */
void write_to_output(const char *txt, struct descriptor_data *t)
{
  int size;

  size = strlen(txt);

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufptr < 0)
    return;

  /* if we have enough space, just write to buffer and that's it! */
  if (t->bufspace >= size) {
    strcpy(t->output + t->bufptr, txt);
    t->bufspace -= size;
    t->bufptr += size;
    return;
  }
  /*
   * If the text is too big to fit into even a large buffer, chuck the
   * new text and switch to the overflow state.
   */
  if (size + t->bufptr > LARGE_BUFSIZE - 1) {
    t->bufptr = -1;
    buf_overflows++;
    return;
  }
  buf_switches++;

  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {                      /* else create a new one */
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output);     /* copy to big buffer */
  t->output = t->large_outbuf->text;    /* make big buffer primary */
  strcat(t->output, txt);       /* now add new text */

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;
}



void	write_to_q(char *txt, struct txt_q *queue)
{
  struct txt_block *new;
  
  CREATE(new, struct txt_block, 1);
  CREATE(new->text, char, strlen(txt) + 1);
  
  strcpy(new->text, txt);
  
  /* Q empty? */
  if (!queue->head) {
    new->next = NULL;
    queue->head = queue->tail = new;
  } else {
    queue->tail->next = new;
    queue->tail = new;
      new->next = NULL;
  }
}




struct timeval timediff(struct timeval *a, struct timeval *b)
{
  struct timeval rslt, tmp;
  
  tmp = *a;
  
  if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0) {
    rslt.tv_usec += 1000000;
    --(tmp.tv_sec);
  } else if (rslt.tv_usec >= 1000000) {
    rslt.tv_usec -= 1000000;
    ++(tmp.tv_sec);
  }
  if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0) {
    rslt.tv_usec = 0;
    rslt.tv_sec = 0;
  }
  return(rslt);
}





/* Empty the queues before closing connection */
void	flush_queues(struct descriptor_data *d)
{
   if (d->large_outbuf) {
      d->large_outbuf->next = bufpool;
      bufpool = d->large_outbuf;
   }

   while (get_from_q(&d->input, buf2)) 
      ;
}





/* ******************************************************************
*  socket handling						    *
****************************************************************** */


/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
int init_socket(int port)
{
  int s, opt;
  struct sockaddr_in sa;

  /*
   * Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so ths point is (hopefully) moot.
   */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Create socket");
    exit(1);
  }
#if defined(SO_SNDBUF)
  opt = LARGE_BUFSIZE + GARBAGE_SPACE;
  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt SNDBUF");
    exit(1);
  }
#endif

#if defined(SO_REUSEADDR)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }
#endif

#if defined(SO_REUSEPORT)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEPORT");
    exit(1);
  }
#endif

#if defined(SO_LINGER)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
      perror("setsockopt LINGER");
      exit(1);
    }
  }
#endif

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *) & sa, sizeof(sa)) < 0) {
    perror("bind");
    close(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5);
  return s;
}


void hostname(struct descriptor_data *desc, int slown)
{
  int size;
  struct sockaddr_in sock;
  struct hostent *from;
  
  if (*desc->host == '\0') {
    size = sizeof(sock);
    if (getpeername(desc->descriptor, (struct sockaddr *) & sock, &size) < 0) {
      perror("getpeername");
    } else if (slown || !(from = gethostbyaddr((char *)&sock.sin_addr,sizeof(sock.sin_addr), AF_INET)))
      {    
	/* find the numeric site address */
	strncpy(desc->host, (char *)inet_ntoa(sock.sin_addr), HOST_LEN);
	*(desc->host + HOST_LEN) = '\0';
      } else {
	strncpy(desc->host, from->h_name, HOST_LEN);
	*(desc->host + HOST_LEN) = '\0';
      }
    
  }
}


int	new_descriptor(int s)
{
   socket_t desc;
   struct descriptor_data *newd, *point, *next_point;
   int	sockets_connected, sockets_playing;
   extern char *GREETINGS;
   int i;
   struct sockaddr_in peer;

   /* accept the new connection */
   i = sizeof(peer);
   if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET) {
      perror("Accept");
      return(-1);
   }
   nonblock(desc);

   sockets_connected = sockets_playing = 0;

   for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      sockets_connected++;
      if (point->ident_sock != -1)
	sockets_connected++;
      if (!point->connected)
	 sockets_playing++;
   }

   /*	if ((maxdesc + 1) >= avail_descs) */
   if (sockets_connected >= avail_descs) {
      write_to_descriptor(desc, "Sorry, EliteMUD is full right now... try again later!  :-)\r\n");
      close(desc);
      return(0);
   } else if (desc > maxdesc)
      maxdesc = desc;

   CREATE(newd, struct descriptor_data, 1);

   /* find info */
   *newd->host = '\0';
   newd->descriptor = desc;
   hostname(newd, nameserver_is_slow);

   if (isbanned(newd->host) == BAN_ALL) {
      close(desc);
      sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
      mudlog(buf2, CMP, LEVEL_IMMORT, TRUE);
      free(newd);
      return(0);
   }


   /* init desc data */
   newd->connected = CON_NME;
   newd->peer_port = peer.sin_port;
   newd->ident_name[0] = '\0';
   newd->bad_pws = 0;
   newd->idle_tics = 0;
   newd->ident_idle = 0;
   newd->pos = -1;
   newd->wait = 1;
   newd->prompt_mode = 0;
   *newd->inbuf = '\0';
   newd->str = 0;
   newd->showstr_count = 0;
   newd->showstr_head = 0;
   *newd->last_input = '\0';
   *newd->num_input = '\0';
   *newd->mult_input = '\0';
   newd->output = newd->small_outbuf;
   *(newd->output) = '\0';
   newd->bufspace = SMALL_BUFSIZE-1;
   newd->large_outbuf = NULL;
   newd->input.head = NULL;
   newd->next = descriptor_list;
   newd->character = 0;
   newd->original = 0;
   newd->snooping = 0;
   newd->snoop_by = 0;
   newd->login_time = time(0);

   if (++last_desc == 1000)
      last_desc = 1;
   newd->desc_num = last_desc;

   /* prepend to list */

   descriptor_list = newd;

   SEND_TO_Q(GREETINGS, newd);
   SEND_TO_Q("By what name do you wish to be known? ", newd);

   ident_start(newd, peer.sin_addr.s_addr);

   return(0);
}


/* New system to handle color -Petrus */
void  str_color_cat(struct descriptor_data *d, char *to, char *from)
{
  int colorcode = SCRCOL_REMCODE;
  register char * tptr = to;

  while (*tptr != '\0')
    tptr++;

  if (d && d->character) {
    if (PRF_FLAGGED((d->character), PRF_COLOR_1))
      colorcode |= SCRCOL_ADDCODE1;
    if (PRF_FLAGGED((d->character), PRF_COLOR_2))
      colorcode |= SCRCOL_ADDCODE2;
    if (PRF_FLAGGED((d->character), PRF_DISPVT))
      colorcode |= SCRCOL_ADDCODEVT;
    if (PRF_FLAGGED((d->character), PRF_IBM_PC))
      colorcode |= SCRCOL_ADDCODEPC;
  }

  /* Word wrap does work properly yet due to different color levels */
  scrcol_copy(tptr, from, colorcode, 80);
}


int	process_output(struct descriptor_data *t)
{
  static char	i[LARGE_BUFSIZE + 20];
    
  if (!t->prompt_mode && !t->connected) {
    strcpy(i, "\r\n");
  } else if (STATE(t) == CON_PLYNG &&
	     PRF_FLAGGED(t->character, PRF_DISPVT) && 
	     !PRF_FLAGGED(t->character, PRF_COMPACT)) {
    strcpy(i, "\r\n");
  } else
    *i = '\0';
    
  str_color_cat(t, i, t->output);
  /*    strcat(i, t->output);  */
    
  if (t->bufptr < 0)
    strcat(i, "**OVERFLOW**");
    
  if (!t->connected && 
      !(t->character && !IS_NPC(t->character) && PRF_FLAGGED(t->character, PRF_COMPACT)) &&
      !PRF_FLAGGED(t->character, PRF_DISPVT))
    {
      strcat(i, "\r\n");
    }
    
  if (STATE(t) == CON_PLYNG && PRF_FLAGGED(t->character, PRF_DISPVT)) {
    if (write_to_pos(t, 1, GET_SCRLEN(t->character) - 4, i, 0) < 0)
      return -1;
  } else if (write_to_descriptor(t->descriptor, i) < 0) 
    return -1;
    
  if (t->snoop_by) {
    SEND_TO_Q("%\r\n", t->snoop_by);
    SEND_TO_Q(t->output, t->snoop_by);
  }
    
  /*
     if (t->snoop_by) {
     i[0] = '%';
     i[1] = ' ';
     strcpy(i+2, t->output);
     SEND_TO_Q(i, t->snoop_by->desc);
     }
     */

  /* if we were using a large buffer, put the large buffer on the buffer
     pool and switch back to the small one */
  if (t->large_outbuf) {
    t->large_outbuf->next = bufpool;
    bufpool = t->large_outbuf;
    t->large_outbuf = NULL;
    t->output = t->small_outbuf;
  }

  /* reset total bufspace back to that of a small buffer */
  t->bufspace = SMALL_BUFSIZE-1;
  t->bufptr = 0;
  *(t->output) = '\0';

  return 1;
}



int	write_to_descriptor(socket_t desc, char *txt)
{
   int	sofar, thisround, total;

   total = strlen(txt);
   sofar = 0;

   do {
      thisround = write(desc, txt + sofar, total - sofar);
      if (thisround < 0) {
	 perror("Write to socket");
	 return(-1);
      }
      sofar += thisround;
   } while (sofar < total);

   return(0);
}


/*
 * ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 */
int process_input(struct descriptor_data *t)
{
  int buf_length, bytes_read, space_left, failed_subst;
  char *ptr, *read_point, *write_point, *nl_pos = NULL;
  char tmp[MAX_INPUT_LENGTH + 8];

  /* first, find the point where we left off reading data */
  buf_length = strlen(t->inbuf);
  read_point = t->inbuf + buf_length;
  space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

  do {
    if (space_left <= 0) {
      log("process_input: about to close connection: input overflow");
      return -1;
    }
#ifdef CIRCLE_WINDOWS
    if ((bytes_read = recv(t->descriptor, read_point, space_left, 0)) < 0) {
      if (WSAGetLastError() != WSAEWOULDBLOCK) {
#else
    if ((bytes_read = read(t->descriptor, read_point, space_left)) < 0) {
#ifdef EWOULDBLOCK
      if (errno == EWOULDBLOCK)
	errno = EAGAIN;
#endif /* EWOULDBLOCK */
      if (errno != EAGAIN && errno != EINTR) {
#endif /* CIRCLE_WINDOWS */
	perror("process_input: about to lose connection");
	return -1;		/* some error condition was encountered on
				 * read */
      } else
	return 0;		/* the read would have blocked: just means no
				 * data there but everything's okay */
    } else if (bytes_read == 0) {
      log("EOF on socket read (connection broken by peer)");
      return -1;
    }
    /* at this point, we know we got some data from the read */

    *(read_point + bytes_read) = '\0';	/* terminate the string */

    /* search for a newline in the data we just read */
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;

    read_point += bytes_read;
    space_left -= bytes_read;

/*
 * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
 * causing the MUD to hang when it encounters input not terminated by a
 * newline.  This was causing hangs at the Password: prompt, for example.
 * I attempt to compensate by always returning after the _first_ read, instead
 * of looping forever until a read returns -1.  This simulates non-blocking
 * I/O because the result is we never call read unless we know from select()
 * that data is ready (process_input is only called if select indicates that
 * this descriptor is in the read set).  JE 2/23/95.
 */
#if !defined(POSIX_NONBLOCK_BROKEN)
  } while (nl_pos == NULL);
#else
  } while (0);

  if (nl_pos == NULL)
    return 0;
#endif /* POSIX_NONBLOCK_BROKEN */

  /*
   * okay, at this point we have at least one newline in the string; now we
   * can copy the formatted data to a new array for further processing.
   */

  read_point = t->inbuf;

  while (nl_pos != NULL) {
    write_point = tmp;
    space_left = MAX_INPUT_LENGTH - 1;

    for (ptr = read_point; (space_left > 0) && (ptr < nl_pos); ptr++) {
      if (*ptr == '\b') {	/* handle backspacing */
	if (write_point > tmp) {
	  if (*(--write_point) == '$') {
	    write_point--;
	    space_left += 2;
	  } else
	    space_left++;
	}
      } else if (isascii(*ptr) && isprint(*ptr)) {
	if ((*(write_point++) = *ptr) == '$') {		/* copy one character */
	  *(write_point++) = '$';	/* if it's a $, double it */
	  space_left -= 2;
	} else
	  space_left--;
      }
    }

    *write_point = '\0';

    if ((space_left <= 0) && (ptr < nl_pos)) {
      char buffer[MAX_INPUT_LENGTH + 64];
      
      sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
      if (write_to_descriptor(t->descriptor, buffer) < 0)
	return -1;
    }

    if (t->snoop_by) {
      SEND_TO_Q("% ", t->snoop_by);
      SEND_TO_Q(tmp, t->snoop_by);
      SEND_TO_Q("\r\n", t->snoop_by);
    }
    failed_subst = 0;

    if (*tmp == '!')
      strcpy(tmp, t->last_input);
    else if (*tmp == '^') {
      if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
	strcpy(t->last_input, tmp);
    } else
      strcpy(t->last_input, tmp);

    if (!failed_subst)
      write_to_q(tmp, &t->input);

    /* find the end of this line */
    while (ISNEWL(*nl_pos))
      nl_pos++;

    /* see if there's another newline in the input buffer */
    read_point = ptr = nl_pos;
    for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;
  }

  /* now move the rest of the buffer up to the beginning for the next pass */
  write_point = t->inbuf;
  while (*read_point)
    *(write_point++) = *(read_point++);
  *write_point = '\0';

  return 1;
}

/* perform substitution for the '^..^' csh-esque syntax orig is the
 * orig string, i.e. the one being modified.  subst contains the
 * substition string, i.e. "^telm^tell"
 */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
  char new[MAX_INPUT_LENGTH + 5];

  char *first, *second, *strpos;

  /*
   * first is the position of the beginning of the first string (the one
   * to be replaced
   */
  first = subst + 1;

  /* now find the second '^' */
  if (!(second = strchr(first, '^'))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* terminate "first" at the position of the '^' and make 'second' point
   * to the beginning of the second string */
  *(second++) = '\0';

  /* now, see if the contents of the first string appear in the original */
  if (!(strpos = strstr(orig, first))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* now, we construct the new string for output. */

  /* first, everything in the original, up to the string to be replaced */
  strncpy(new, orig, (strpos - orig));
  new[(strpos - orig)] = '\0';

  /* now, the replacement string */
  strncat(new, second, (MAX_INPUT_LENGTH - strlen(new) - 1));

  /* now, if there's anything left in the original after the string to
   * replaced, copy that too. */
  if (((strpos - orig) + strlen(first)) < strlen(orig))
    strncat(new, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(new) - 1));

  /* terminate the string in case of an overflow from strncat */
  new[MAX_INPUT_LENGTH - 1] = '\0';
  strcpy(subst, new);

  return 0;
}



void	close_sockets(int s)
{
   log("Closing all sockets.");

   while (descriptor_list)
      close_socket(descriptor_list);

   close(s);
}




void	close_socket(struct descriptor_data *d)
{
   struct descriptor_data *tmp;
   char	buf[100];

   close(d->descriptor);
   flush_queues(d);
   if (d->descriptor == maxdesc)
      --maxdesc;

   if (d->ident_sock != INVALID_SOCKET)
     close(d->ident_sock);

   /* Forget snooping */
   if (d->snooping)
      d->snooping->snoop_by = 0;

   if (d->snoop_by) {
      SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
      d->snoop_by->snooping = 0;
   }

   if (d->character)
      if (d->connected == CON_PLYNG) {
	 save_char(d->character, IN_VROOM(d->character), 2);
	 act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
	 sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
	 mudlog(buf, NRM, MAX(LEVEL_DEITY, GET_INVIS_LEV(d->character)), TRUE);
	 d->character->desc = 0;
      }
      else {
	 sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
	 mudlog(buf, CMP, MAX(LEVEL_DEITY, GET_LEVEL(d->character)), TRUE);
	 free_char(d->character);
      }
   else
      mudlog("Losing descriptor without char.", CMP, LEVEL_IMMORT, TRUE);

   if (next_to_process == d)  /* to avoid crashing the process loop */
      next_to_process = next_to_process->next;

   if (d == descriptor_list) /* this is the head of the list */
      descriptor_list = descriptor_list->next;
   else {  /* This is somewhere inside the list */
      /* Locate the previous element */
      for (tmp = descriptor_list; (tmp->next != d) && tmp; tmp = tmp->next)
	 ;
      tmp->next = d->next;
   }

   if (d->showstr_head)
      free(d->showstr_head);
   if (d->showstr_count)
      free(d->showstr_vector);
   if (d->storage)
      free(d->storage);

   free(d);
}

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

#if defined(SVR4) || defined(LINUX)

void	nonblock(socket_t s)
{
   int	flags;
   flags = fcntl(s, F_GETFL);
   flags |= O_NONBLOCK;
   if (fcntl(s, F_SETFL, flags) < 0) {
      perror("Fatal error executing nonblock (comm.c)");
      exit(1);
   }
}

#else

void	nonblock(int s)
{
   if (fcntl(s, F_SETFL, FNDELAY) == -1) {
      perror("Fatal error executing nonblock (comm.c)");
      exit(1);
   }
}

#endif



/* ****************************************************************
*	Public routines for system-to-player-communication	  *
*******************************************************************/



void	send_to_char(char *messg, struct char_data *ch)
{
   if (ch->desc && messg)
      SEND_TO_Q(messg, ch->desc);
}




void	send_to_all(char *messg)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
	 if (!i->connected)
	    SEND_TO_Q(messg, i);
}


void	send_to_outdoor(char *messg)
{
  struct descriptor_data *i;
  
  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	if (OUTSIDE(i->character) && !PLR_FLAGGED(i->character, PLR_WRITING))
	  SEND_TO_Q(messg, i);
}


void	send_to_except(char *messg, struct char_data *ch)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
	 if (ch->desc != i && !i->connected)
	    SEND_TO_Q(messg, i);
}



void	send_to_room(char *messg, int room)
{
   struct char_data *i;

   if (messg)
      for (i = world[room]->people; i; i = i->next_in_room)
	 if (i->desc)
	    SEND_TO_Q(messg, i->desc);
}




void	send_to_room_except(char *messg, int room, struct char_data *ch)
{
   struct char_data *i;

   if (messg)
      for (i = world[room]->people; i; i = i->next_in_room)
	 if (i != ch && i->desc)
	    SEND_TO_Q(messg, i->desc);
}


void	send_to_room_except_two
(char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
   struct char_data *i;

   if (messg)
      for (i = world[room]->people; i; i = i->next_in_room)
	 if (i != ch1 && i != ch2 && i->desc)
	    SEND_TO_Q(messg, i->desc);
}


/* Advanced Random Social System, ARSS, - Petrus */

/* Count # of ARSS-'slots' */
int arss_counter(char *str)
{
  int num = 1, bracers = 0;
  
  while (str && *str) {
    if (*str == '{')
      bracers++;
    else if (*str == '^' && bracers == 1)
      num++;
    else if (*str == '}')
      bracers--;
    
    if (bracers == 0)
      break;
    
    str++;
  }
  
  return num;
}

/* Fast forward to a given 'slot' */
char *arss_forward(char *str, int num)
{
  int bracers = 0;
  
  while (str && *str) {
    if (*str == '{') {
      bracers++;
      if (bracers == 1)
	num--;
    } else if (*str == '^' && bracers == 1)
      num--;
    else if (*str == '}')
      bracers--;
    
    str++;
    
    if (num == 0)
      break;
  }
  
  return str;
}

/* Find end of ARSS-string */
char *arss_findend(char *str)
{
  int bracers = 0;

  while (str && *str) {
    if (*str == '}')
      bracers--;
    else if (*str == '{')
      bracers++;

    str++;
    
    if (bracers < 0)
      break;
  }
  
  return str;
}

/* Returns a pointer to a random 'slot' */
char *arss_randompointer(char *str)
{
    return arss_forward(str, number(1, arss_counter(str)));
}


/* higher-level communication */

char *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) i = ACTNULL; else i = (expression);


#define SENDOK(ch) ((((ch)->desc) || \
		     (IS_NPC(to) && \
		      (mob_index[to->nr].progtypes & ACT_PROG))) && \
		    (AWAKE(to) || IS_SET(type, TO_SLEEP)) && \
		    !PLR_FLAGGED((ch), PLR_WRITING) && \
		    (!IS_SET(type, ACT_GAG) || !PRF_FLAGGED(to, PRF_GAG)))

/* New act() - PV 97/11/8  */


void perform_act(char *orig, struct char_data *ch, struct obj_data *obj,
		 void *vict_obj, struct char_data *to)
{
  register char *i = NULL, *buf;
  static char lbuf[MAX_STRING_LENGTH];
  bool ignore = FALSE;

  buf = lbuf;

  for (;;) {
    
    if (*orig == '{') {
      orig = arss_randompointer(orig);
    } else if ((*orig == '^' || *orig == '}')) {
      orig = arss_findend(orig);
    } else if (*orig == '$' && (!ignore || *(orig+1) == '+')) {
      switch (*(++orig)) {
      case 'n':
	i = PERS(ch, to);
	break;
      case 'N':
	CHECK_NULL(vict_obj, PERS((struct char_data *) vict_obj, to));
	break;
      case 'm':
	i = HMHR(ch);
	break;
      case 'M':
	CHECK_NULL(vict_obj, HMHR((struct char_data *) vict_obj));
	break;
      case 's':
	i = HSHR(ch);
	break;
      case 'S':
	CHECK_NULL(vict_obj, HSHR((struct char_data *) vict_obj));
	break;
      case 'e':
	i = HSSH(ch);
	break;
      case 'E':
	CHECK_NULL(vict_obj, HSSH((struct char_data *) vict_obj));
	break;
      case 'o':
	CHECK_NULL(obj, OBJN(obj, to));
	break;
      case 'O':
	CHECK_NULL(vict_obj, OBJN((struct obj_data *) vict_obj, to));
	break;
      case 'p':
	CHECK_NULL(obj, OBJS(obj, to));
	break;
      case 'P':
	CHECK_NULL(vict_obj, OBJS((struct obj_data *) vict_obj, to));
	break;
      case 'a':
	CHECK_NULL(obj, SANA(obj));
	break;
      case 'A':
	CHECK_NULL(vict_obj, SANA((struct obj_data *) vict_obj));
	break;
      case 't':
	CHECK_NULL(obj, (char *) obj);
	break;
      case 'T':
	CHECK_NULL(vict_obj, (char *) vict_obj);
	break;
      case 'F':
	CHECK_NULL(vict_obj, fname((char *) vict_obj));
	break;
      case '-':
	ignore = TRUE; i = "";
	break;
      case '+':
	ignore = FALSE; i = "";
	break;
      case '$':
	i = "$";
	break;
      default:
	log("SYSERR: Illegal $-code to act():");
	strcpy(buf1, "SYSERR: ");
	strcat(buf1, orig);
	log(buf1);
	break;
      }
      while ((*buf = *(i++)))
	buf++;
      orig++;
    } else if (!(*(buf++) = *(orig++)))
      break;
  }

  *(--buf) = '\r';
  *(++buf) = '\n';
  *(++buf) = '\0';

  if ((*lbuf == '§' || *lbuf == '#') && *(lbuf + 1) != '\0') {
	  if (isdigit(*(lbuf + 1)))
	    CAP((lbuf + 3));
	  else
	    CAP((lbuf + 2));
	} else
	  CAP(lbuf);

  if (to->desc)
    SEND_TO_Q(lbuf, to->desc);

  if (MOBTrigger)
    mprog_act_trigger(buf, to, ch, obj, vict_obj);

}


void act(char *str, int hide_invisible, struct char_data *ch, 
	 struct obj_data *obj, void *vict_obj, int type)
{
  struct char_data *to = NULL;
    
  if (!str || !*str) {
    MOBTrigger = TRUE;
    return;
  }
  
  if (IS_SET(type, TO_CHAR)) {
    to = ch;
    if (ch && SENDOK(ch))
      perform_act(str, ch, obj, vict_obj, to);
    return;
  }
  
  if (IS_SET(type, TO_VICT)) {
    if ((to = (struct char_data *) vict_obj) && ch != to && SENDOK(to))
      perform_act(str, ch, obj, vict_obj, to);
    return;
  }

  /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

  if (ch && ch->in_room != NOWHERE)
    to = world[ch->in_room]->people;
  else if (obj && obj->in_room != NOWHERE)
    to = world[obj->in_room]->people;
  else {
    log("SYSERR: no valid target to act()!");
    return;
  }
  
  for (; to; to = to->next_in_room) {
    if (!SENDOK(to) || (to == ch))
      continue;
    if (hide_invisible && ch && !CAN_SEE(to, ch))
      continue;
    if (type != TO_ROOM && to == vict_obj)
      continue;
    perform_act(str, ch, obj, vict_obj, to);  
  }        

  MOBTrigger = TRUE;
} /* New act() - PV 97/11/9 */



/* New gain routine by -Petrus */
void   check_gain(void)
{
    struct descriptor_data *point, *next_point;

    for (point = descriptor_list; point; point = next_point) {
	next_point = point->next;
	
	if (point->character) {
	    switch(GET_POS(point->character)) {
	    case POS_SLEEPING:
		point->character->specials.gain_count += 4;
		break;
	    case POS_RESTING:
		point->character->specials.gain_count += 3;
		break;
	    case POS_SITTING:
		point->character->specials.gain_count += 2;
		break;
	    case POS_STANDING:
		point->character->specials.gain_count++;
		break;
	    }
	}
    }
}


int is_host_by_name(char *name)
{
  while (name && *name != '\0') {
    if (isalpha(*name))
      return TRUE;
    name++;
  }

  return FALSE;
}
   

ACMD(do_sitename)
{
  struct char_data *tch;

  if (!ch || !ch->desc)
    return;

  skip_spaces(&argument);
  
  if (!*argument) {
    send_to_char("Sitename on what player?\r\n", ch);
    return;
  }

  if (!(tch = get_player_vis(ch, argument))) {
    send_to_char("No such player.\r\n", ch);
    return;
  }

  if (!tch->desc) {
    send_to_char("No descriptor on that char.\r\n", ch);
    return;
  }

  if (!is_host_by_name(tch->desc->host)) {
    *tch->desc->host = '\0';
    hostname(tch->desc, FALSE);
  }

  sprintf(buf, "%s is logged on from [%s].\r\n",
	  GET_NAME(tch), tch->desc->host);
  send_to_char(buf, ch);
}


ACMD(do_identname)
{
  int size;
  struct sockaddr_in sock;
  struct char_data *tch;

  if (!ch || !ch->desc)
    return;

  skip_spaces(&argument);
  
  if (!*argument) {
    send_to_char("Sitename on what player?\r\n", ch);
    return;
  }

  if (!(tch = get_player_vis(ch, argument))) {
    send_to_char("No such player.\r\n", ch);
    return;
  }

  if (!tch->desc) {
    send_to_char("No descriptor on that char.\r\n", ch);
    return;
  }  

  size = sizeof(sock);
  if (getpeername(tch->desc->descriptor, (struct sockaddr *) & sock, &size) < 0) {
    perror("getpeername");
  } else {
    ident_start(tch->desc, sock.sin_addr.s_addr);
  }
}


/* ******************************************************************
*  signal-handling functions (formerly signals.c).  UNIX only.      *
****************************************************************** */


#ifdef CIRCLE_UNIX

RETSIGTYPE checkpointing(int sig)
{
  if (!tics) {
    log("SYSERR: CHECKPOINT shutdown: tics not updated");
    abort();
  } else
    tics = 0;
}


RETSIGTYPE reread_wizlists(int sig)
{
  void reboot_wizlists(void);

  mudlog("Signal received - rereading wizlists.", CMP, LEVEL_DEITY, TRUE);
  reboot_wizlists();
}


RETSIGTYPE unrestrict_game(int sig)
{
  extern struct ban_list_element *ban_list;
  extern int num_invalid;

  mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
	 BRF, LEVEL_DEITY, TRUE);
  ban_list = NULL;
  restrict = 0;
  num_invalid = 0;
}


RETSIGTYPE hupsig(int sig)
{
  log("SYSERR: Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
  exit(0);			/* perhaps something more elegant should
				 * substituted */
}


/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 *
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 *
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
 */

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc *my_signal(int signo, sigfunc * func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif

  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
#endif				/* POSIX */


void signal_setup(void)
{
  struct itimerval itime;
  struct timeval interval;

  /* user signal 1: reread wizlists.  Used by autowiz system. */
  my_signal(SIGUSR1, reread_wizlists);

  /*
   * user signal 2: unrestrict game.  Used for emergencies if you lock
   * yourself out of the MUD somehow.  (Duh...)
   */
  my_signal(SIGUSR2, unrestrict_game);

  /*
   * set up the deadlock-protection so that the MUD aborts itself if it gets
   * caught in an infinite loop for more than 3 minutes.
   */
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  my_signal(SIGVTALRM, checkpointing);

  /* just to be on the safe side: */
  my_signal(SIGHUP, hupsig);
  my_signal(SIGINT, hupsig);
  my_signal(SIGTERM, hupsig);
  my_signal(SIGPIPE, SIG_IGN);
  my_signal(SIGALRM, SIG_IGN);
}

#endif				/* CIRCLE_UNIX */

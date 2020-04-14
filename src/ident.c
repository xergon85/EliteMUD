/* ************************************************************************
*  Part of EliteMud
*  Ported by Helm
*
*  File: ident.c                                                          *
*                                                                         *
*  Usage: Functions for handling rfc 931/1413 ident lookups               *
*                                                                         *
*  Written by Eric Green (thrytis@imaxx.net)				  *
*									  *
*  Changes:								  *
*      10/9/96 ejg:   Added compatibility with win95/winNT		  *
*      10/25/97 ejg:  Updated email address, fixed close socket bug,      *
*                     buffer overrun bug, and used extra hostlength space *
*                     if available                                        *
*      12/8/97 ejg:   Updated headers for patch level 12.                 *
************************************************************************ */

#define __IDENT_C__

#include "conf.h"
#include "sysdep.h"

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "ident.h"


/* max time in seconds until ident gives up */
#define IDENT_TIMEOUT 60

#define IDENT_PORT    113


#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

void    log(char *str);

/* log the errno/WSALastError() message */
void logerror(const char *msg)
{
  perror(msg);
}

/* start the process of looking up remote username */
void ident_start(struct descriptor_data *d, long addr)
{
  socket_t sock;
  struct sockaddr_in sa;

  void nonblock(socket_t s);

  if (!ident) {
    IDENT_STATE(d) = ID_COMPLETE;
    d->ident_sock = INVALID_SOCKET;
    return;
  }

  d->ident_idle = 0;

  /*
   * create a nonblocking socket, and start
   * the connection to the remote machine
   */

  if((sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    logerror("socket");
    d->ident_sock = INVALID_SOCKET;
    IDENT_STATE(d) = ID_COMPLETE;
    return;
  }

  sa.sin_family = AF_INET;
  sa.sin_port = ntohs(IDENT_PORT);
  sa.sin_addr.s_addr = addr;

  nonblock(sock);
  d->ident_sock = sock;

  errno = 0;

  if (connect(sock, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
    if (errno == EINPROGRESS ||	errno == EWOULDBLOCK) {
      /* connection in progress */
      IDENT_STATE(d) = ID_CONING;
      return;
    }

    /* connection failed */
    else if (errno != ECONNREFUSED)
      logerror("ident connect");

    IDENT_STATE(d) = ID_COMPLETE;
  }

  else    /* connection completed */
    IDENT_STATE(d) = ID_CONED;
}


void ident_check(struct descriptor_data *d)
{
  fd_set fd, efd;
  int rc, rmt_port, our_port, len;
  char user[256], *p;

  extern struct timeval null_time;
  extern int port;

  /*
   * Each pulse, this checks if the ident is ready to proceed to the
   * next state, by calling select to see if the socket is writeable
   * (connected) or readable (response waiting).  
   */

  switch (IDENT_STATE(d)) {
  case ID_CONING:
    /* waiting for connect() to finish */

    if (d->ident_sock != INVALID_SOCKET) {
      FD_ZERO(&fd);
      FD_ZERO(&efd);
      FD_SET(d->ident_sock, &fd);
      FD_SET(d->ident_sock, &efd);
    }

    if ((rc = select(d->ident_sock + 1, (fd_set *) 0, &fd,
		     &efd, &null_time)) == 0)
      break;

    else if (rc < 0) {
      logerror("ident check select (conning)");
      IDENT_STATE(d) = ID_COMPLETE;
      break;
    }

    if (FD_ISSET(d->ident_sock, &efd)) {
      /* exception, such as failure to connect */
      IDENT_STATE(d) = ID_COMPLETE;
      break;
    }
    
    IDENT_STATE(d) = ID_CONED;

  case ID_CONED:
    /* connected, write request */
	
    sprintf(buf, "%d, %d\n\r", ntohs(d->peer_port), port);
	
    len = strlen(buf);
    if (write(d->ident_sock, buf, len) != len) {
      if (errno != EPIPE)	/* read end closed (no remote identd) */
	logerror("ident check write (conned)");

      IDENT_STATE(d) = ID_COMPLETE;
      break;
    }

    IDENT_STATE(d) = ID_READING;
	
  case ID_READING:
    /* waiting to read */
	
    if (d->ident_sock != INVALID_SOCKET) {
      FD_ZERO(&fd);
      FD_ZERO(&efd);
      FD_SET(d->ident_sock, &fd);
      FD_SET(d->ident_sock, &efd);
    }

    if ((rc = select(d->ident_sock+1, &fd, (fd_set *) 0,
		     &efd, &null_time)) == 0)
      break;

    else if (rc < 0) {
      logerror("ident check select (reading)");
      IDENT_STATE(d) = ID_COMPLETE;
      break;
    }

    if (FD_ISSET(d->ident_sock, &efd)) {
     IDENT_STATE(d) = ID_COMPLETE;
      break;
    }
      
    IDENT_STATE(d) = ID_READ;
	
  case ID_READ:
    /* read ready, get the info */
    if ((len = read(d->ident_sock, buf, sizeof(buf) - 1)) < 0)
      logerror("ident check read (read)");

    else {
      buf[len] = '\0';
      if (sscanf(buf, "%u , %u : USERID :%*[^:]:%255s",
		 &rmt_port, &our_port, user) != 3) {

	/* check if error or malformed */
	if (sscanf(buf, "%u , %u : ERROR : %255s",
		   &rmt_port, &our_port, user) == 3) {
	  sprintf(buf2, "Ident error from %s: \"%s\"", d->host, user);
	  log(buf2);
	}
	else {
	  /* strip off trailing newline */
	  for (p = buf + len - 1; p > buf && ISNEWL(*p); p--);
	  p[1] = '\0';

	  sprintf(buf2, "Malformed ident response from %s: \"%s\"",
		  d->host, buf);
	  log(buf2);
	}
      }
      else {
	  strncpy(d->ident_name, user, HOST_LEN);
	  d->ident_name[HOST_LEN] = '\0';
      }
    }
	
    IDENT_STATE(d) =ID_COMPLETE;
	
  case ID_COMPLETE:
    /* close up the ident socket, if one is opened. */
    if (d->ident_sock != INVALID_SOCKET) {
      CLOSE_SOCKET(d->ident_sock);
      d->ident_sock = INVALID_SOCKET;
    }
    d->ident_idle = 0;

    IDENT_STATE(d) = ID_NONE;
    return;

  default:
    return;
  }

  if (d->ident_idle++ >= IDENT_TIMEOUT)
    IDENT_STATE(d) = ID_COMPLETE;
}


/* returns 1 if waiting for ident to complete, else 0 */
int waiting_for_ident(struct descriptor_data *d)
{
  switch (IDENT_STATE(d)) {
  case ID_CONING:
  case ID_CONED:
  case ID_READING:
  case ID_READ:
  case ID_COMPLETE:
    return 1;
      
  default:
    return 0;
  }

  return 0;
}

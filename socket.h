/*
  Copyright (C) MOXA Inc. All rights reserved.
  This software is distributed under the terms of the
  MOXA License.  See the file COPYING-MOXA for details.
*/

/*---------------------------------------------------------------------------*/
/**
  @file		socket.h
  @brief	Socket API header file

  TCP socket utility functions, it provides simple functions that helps
  to build TCP client/server.


  History:
  Date		Author			Comment
  08-01-2005	AceLan Kao.		Create it.

  @author AceLan Kao.(acelan_kao@moxa.com.tw)
 */
/*---------------------------------------------------------------------------*/

#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <strings.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <fcntl.h>

#define MAX_CONNECTION				20

int	TCPServerInit( int port, int *serverfd);
int	TCPServerWaitConnection( int serverfd, int *clientfd, char *clientaddr);
int     TCPServerSelect( int* serverfdlist, int num, int *clientfd, char *clientaddr);
int	TCPClientInit( int *clientfd);
int	TCPClientConnect( const int clientfd, const char *addr, int port);
int	TCPNonBlockRead( int clientfd, char* buf, int size);
int     TCPBlockRead( int clientfd, char* buf, int size);
int	TCPWrite( int clientfd, char* buf, int size);
void	TCPClientClose( int sockfd);
void	TCPServerClose( int sockfd);

#endif


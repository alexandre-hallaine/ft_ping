#ifndef _FT_PING_H_
# define _FT_PING_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <signal.h>
#include <stdlib.h>

typedef struct s_ping
{
	char *hostname;
	char ip[INET6_ADDRSTRLEN];
	struct sockaddr addr;
	int socket;
	struct icmphdr icmp;
	int done;
	int received;
	int sent;
	int reply;
	struct addrinfo *res;
} t_ping;

extern t_ping g_ping;

#endif
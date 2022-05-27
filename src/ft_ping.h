#ifndef _FT_PING_H_
#define _FT_PING_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

typedef union s_recv
{
	struct
	{
		struct iphdr ip;
		struct icmphdr icmp;
		char data[1];
	} v4;
	struct
	{
		struct icmphdr icmp;
		char data[1];
	} v6;
} t_recv;

typedef struct s_ping
{
	char *hostname;
	char ip[INET6_ADDRSTRLEN];

	struct addrinfo *res;
	int socket;
	struct icmphdr icmp;

	int done;

	int received;
	int sent;
	int reply;

	int verbose;
	int audible;
	long long int count;
	int quiet;
	int debug;

	int len;
	int ttl_reply;
	int ttl;

	char *cmd;

	struct timeval begin;
	struct timeval last;

	double min;
	double max;
	double sum;
	double msum;
} t_ping;

extern t_ping g_ping;

#endif
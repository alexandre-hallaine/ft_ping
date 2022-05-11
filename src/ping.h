#ifndef PING_H
#define PING_H

#include <netinet/in.h>
#include <stdbool.h>
#include <netinet/ip_icmp.h>

typedef struct s_ping_t
{
	char *hostname;
	char ip[INET_ADDRSTRLEN];

	bool verbose;

	int fd;
	struct sockaddr_in dest;
	struct icmphdr packet;

	size_t sent;
	size_t received;

	struct timeval first;
	struct timeval last;

	double min;
	double max;
	double sum;
	double msum;

	bool loss;
	bool finished;
} t_ping;

t_ping g_ping;

void check_args(int argc, char *argv[]);

void init_ping(void);

void sigint_handler();
void sigalrm_handler();

void reply_handler();

#endif

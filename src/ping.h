#ifndef PING_H
#define PING_H

#include <netinet/in.h>
#include <stdbool.h>
#include <netinet/ip_icmp.h>

typedef struct s_hdr
{
	struct iphdr ip;
	struct icmphdr icmp;
} t_hdr;

typedef struct s_ping
{
	char *hostname;
	char ip[INET_ADDRSTRLEN];

	bool verbose;

	int fd;
	struct sockaddr_in dest;
	t_hdr packet;

	size_t sent;
	size_t received;

	struct timeval first;
	struct timeval last;

	double min;
	double max;
	double sum;
	double msum;

	bool replied;
	bool finished;
} t_ping;

t_ping g_ping;

void check_args(int argc, char *argv[]);

unsigned short checksum(void *address, size_t len);
void fill_ip_header(struct iphdr *ip, uint16_t size, uint8_t ttl, uint32_t dest);
void fill_icmp_header(struct icmphdr *icmp);
void display_header(void *address);

void init_ping(void);

void sigint_handler();
void sigalrm_handler();

void reply_handler();
void print_stats();

#endif

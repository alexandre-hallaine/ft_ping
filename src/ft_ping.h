#ifndef FT_PING_H
#define FT_PING_H

#include <stdio.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>

typedef union
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

typedef struct
{
	size_t send;
	size_t received;
	size_t errors;

	double min;
	double max;
	double sum;
	double msum;
} t_stats;

typedef struct
{
	bool audible;
	size_t count;
	size_t interval;
	int ttl;
	bool timestamp;
	bool quiet;
	bool verbose;
	bool debug;
	bool ipv4;
	bool ipv6;
} t_options;

typedef struct
{
	bool replied;
	bool running;

	struct timeval begin;
	struct timeval last;
} t_utils;

typedef struct
{
	char *hostname;
	char ip[INET6_ADDRSTRLEN];
	char *cmd;

	struct addrinfo *res;
	int socket;
	struct icmphdr icmp;

	t_options options;
	t_stats stats;
	t_utils utils;
} t_ping;

extern t_ping g_ping;

// ft_ping
void sigint_handler();

// parse
void check_args(char **av);

// recv
void recv_msg();

// utils
void ft_exit(char *cmd, char *msg);
int is_digit(char *str);
unsigned short checksum(unsigned short *address, size_t len);
double seconds(struct timeval start);
void update_stats(unsigned short len, unsigned char ttl);

// verbose
void display_header_iphdr(struct iphdr *tmp, char *prefix);
void display_header_ip6hdr(struct ip6_hdr *header, char *prefix);
void display_header_icmp(struct icmphdr *icmp, char *prefix);

#endif
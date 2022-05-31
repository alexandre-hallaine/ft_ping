#include "ft_ping.h"

#include <limits.h>
#include <stdlib.h>
#include <sys/time.h>

void ft_exit(char *cmd, char *msg)
{
	printf("%s: %s: %s\n", g_ping.cmd, cmd, msg);
	exit(EXIT_FAILURE);
}

int is_digit(char *str)
{
	while ((*str >= '0' && *str <= '9'))
		str++;
	return !*str;
}

int ft_atoi(const char *nptr)
{
	ssize_t nbr = 0;
	while (*nptr >= '0' && *nptr <= '9')
		nbr = nbr * 10 + (*nptr++ - '0');
	if (nbr < INT_MIN || nbr > INT_MAX)
		return (0);
	return (nbr);
}

unsigned short checksum(unsigned short *address, size_t len)
{
	unsigned short sum = 0;
	while (len -= sizeof(short))
		sum += *address++;
	return (~sum);
}

double seconds(struct timeval start)
{
	struct timeval end;
	gettimeofday(&end, NULL);
	return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000.0;
}

#define ABS(x) ((x) < 0 ? -(x) : (x))

void update_stats(unsigned short len, unsigned char ttl)
{
	double delta = seconds(g_ping.utils.last);
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
		   len, g_ping.ip, g_ping.icmp.un.echo.sequence, ttl, delta);

	if (delta < g_ping.stats.min || g_ping.stats.received == 1)
		g_ping.stats.min = delta;
	if (delta > g_ping.stats.max)
		g_ping.stats.max = delta;
	g_ping.stats.sum += delta;
	g_ping.stats.msum += ABS(g_ping.stats.sum / g_ping.stats.received - delta);
}
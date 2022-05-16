#include "ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

bool is_digit(char *str)
{
	while (*str)
	{
		if (!ft_isdigit(*str))
			return (false);
		str++;
	}
	return (true);
}

int ft_atoi(const char *nptr)
{
	char sign;
	ssize_t nbr;

	while (*nptr == ' ' || (*nptr >= 9 && *nptr <= 13))
		nptr++;
	sign = 1;
	if (*nptr == '+' || *nptr == '-')
		if (*nptr++ == '-')
			sign = -1;
	nbr = 0;
	while (*nptr >= '0' && *nptr <= '9')
	{
		nbr = nbr * 10 + (*nptr++ - '0');
		if (nbr < 0)
		{
			if (sign == 1)
				return (-1);
			return (0);
		}
	}
	return (sign * nbr);
}

void usage(char *cmd)
{
	printf("Usage: %s [-h] [-t ttl] [-v] host\n", cmd);
	exit(1);
}

void options(char **argv[], char *cmd)
{
	while (*++**argv)
		switch (***argv)
		{
		case 'v':
			g_ping.verbose = 1;
			break;
		/*case 't':
		{
			if (*(**argv + 1) != '\0')
			{
				printf("%s: invalid argument: '%s'\n", cmd, **argv + 1);
				exit(1);
			}
			int ttl = 0;
			if (*++*argv && is_digit(**argv))
				ttl = ft_atoi(**argv);
			if (ttl <= 0 || ttl > 255)
			{
				printf("%s: cannot set unicast time-to-live: Invalid argument\n", cmd);
				exit(1);
			}
			g_ping.packet.ip.ttl = ttl;
			return;
		}*/
		default:
			usage(cmd);
		}
}

void init_host(char *hostname, char *cmd)
{
	g_ping.hostname = hostname;

	struct addrinfo hints;
	bzero(&hints, sizeof(hints));
	//hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	struct addrinfo *res;
	if (getaddrinfo(g_ping.hostname, NULL, &hints, &res) != 0)
	{
		printf("%s: %s: No address associated with hostname\n", cmd, g_ping.hostname);
		exit(1);
	}

	g_ping.family = res->ai_family;
	memcpy(&g_ping.dest, res->ai_addr, res->ai_addrlen);

	if (g_ping.family == AF_INET)
		inet_ntop(g_ping.family, &((struct sockaddr_in *)&g_ping.dest)->sin_addr, g_ping.ip, sizeof(g_ping.ip));
	else
		inet_ntop(g_ping.family, &((struct sockaddr_in6 *)&g_ping.dest)->sin6_addr, g_ping.ip, sizeof(g_ping.ip));

	//print ip
	printf("IP: %s\n", g_ping.ip);
	freeaddrinfo(res);
}

void check_args(char *argv[])
{
	char *cmd = argv[0];

	while (*++argv)
		if (**argv == '-')
			options(&argv, cmd);
		else if (!g_ping.hostname && !argv[1])
			init_host(*argv, cmd);
		else
		{
			printf("%s: usage error: Extranous argument found (%s)\n", cmd, argv[1]);
			exit(1);
		}
	if (!g_ping.hostname)
	{
		printf("%s: usage error: Destination address required\n", cmd);
		exit(1);
	}
}
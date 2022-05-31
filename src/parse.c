#include "ft_ping.h"

#include <limits.h>

void options(char ***av)
{
	while (*++**av)
		switch (***av)
		{
		case 'v':
			g_ping.options.verbose = true;
			break;
		case 't':
			if (*(**av + 1) != '\0')
				ft_exit("usage error", "Invalid argument for -t");
			int ttl = 0;
			if (*++*av && is_digit(**av))
				ttl = ft_atoi(**av);
			if (ttl <= 0 || ttl > UCHAR_MAX)
				ft_exit("usage error", "Invalid TTL");
			g_ping.options.ttl = ttl;
			return;
		case 'a':
			g_ping.options.audible = true;
			break;
		case 'c':
			if (*(**av + 1) != '\0')
				ft_exit("usage error", "Invalid argument for -c");
			int count = 0;
			if (*++*av && is_digit(**av))
				count = ft_atoi(**av);
			if (count <= 0)
				ft_exit("usage error", "Value out of range");
			g_ping.options.count = count;
			return;
		case 'q':
			g_ping.options.quiet = true;
			break;
		case 'V':
			g_ping.options.debug = true;
			g_ping.options.verbose = true;
			break;
		case 'D':
			g_ping.options.timestamp = true;
			break;
		case '4':
			g_ping.options.ipv4 = true;
			break;
		case '6':
			g_ping.options.ipv6 = true;
			break;
		default:
			printf("Usage: ft_ping [-h] [-t ttl] [-v] [hostname]\n");
			exit(1);
		}
}

void init_host(char *hostname)
{
	g_ping.hostname = hostname;
	struct addrinfo hints = {0};
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_socktype = SOCK_RAW;

	if (g_ping.options.ipv4)
		hints.ai_family = AF_INET;
	if (g_ping.options.ipv6)
		hints.ai_family = AF_INET6;

	struct addrinfo *res;
	if (getaddrinfo(g_ping.hostname, NULL, &hints, &res))
		ft_exit("getaddrinfo", "Could not resolve hostname");
	g_ping.res = res;
}

void check_args(char **av)
{
	if (getuid() != 0)
		ft_exit("usage error", "You must be root to run this program");

	while (*++av)
		if (**av == '-')
			options(&av);
		else if (!g_ping.hostname && !av[1])
			init_host(*av);
		else
			ft_exit("usage error", "Extranous argument found");
	if (!g_ping.hostname)
		ft_exit("usage error", "No hostname specified");
}
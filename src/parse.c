#include "ft_ping.h"

void options(char ***av)
{
	while (*++**av)
		switch (***av)
		{
		case 'v':
			g_ping.verbose = 1;
			break;
		case 't':
			if (*(**av + 1) != '\0')
				ft_exit("usage error", "Invalid argument for -t");
			if (*++*av && is_digit(**av))
				g_ping.ttl = ft_atoi(**av);
			if (g_ping.ttl <= 0 || g_ping.ttl > 255)
				ft_exit("usage error", "Invalid TTL");
			return;
		case 'a':
			g_ping.audible = 1;
			break;
		case 'c':
			if (*(**av + 1) != '\0')
				ft_exit("usage error", "Invalid argument for -c");
			if (*++*av && is_digit(**av))
				g_ping.count = ft_atoi(**av);
			if (g_ping.count <= 0 || g_ping.count > 9223372036854775807)
				ft_exit("usage error", "Value out of range");
			return;
		case 'q':
			g_ping.quiet = 1;
			break;
		case 'V':
			g_ping.debug = 1;
			g_ping.verbose = 1;
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
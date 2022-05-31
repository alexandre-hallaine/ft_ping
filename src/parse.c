#include "ft_ping.h"

#include <limits.h>

int get_number(char ***av, int max)
{

	if (*(**av + 1) != '\0')
	{
		char buffer[1 << 10];
		sprintf(buffer, "Invalid argument for -%c", ***av);
		ft_exit("usage error", buffer);
	}

	int value = 0;
	if (*++*av && is_digit(**av))
		value = ft_atoi(**av);

	if (value <= 0 || value >= max)
		ft_exit("usage error", "Value out of range");
	return (value);
}

void options(char ***av)
{
	while (*++**av)
		switch (***av)
		{
		case 'v':
			g_ping.options.verbose = true;
			break;
		case 't':
			g_ping.options.ttl = get_number(av, UCHAR_MAX);
			return;
		case 'a':
			g_ping.options.audible = true;
			break;
		case 'c':
			g_ping.options.count = get_number(av, INT_MAX);
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
		case 'i':
			g_ping.options.interval = get_number(av, INT_MAX);
			return;
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
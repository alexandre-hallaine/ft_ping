#include "ft_ping.h"

#include <limits.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

void print_help()
{
	// usage
	printf("Usage\n  %s [options] <destination>\n\n", g_ping.cmd);

	// options
	printf("Options:\n");
	printf("  <destination>      dns name or ip address\n");
	printf("  -a                 use audible ping\n");
	printf("  -c <count>         stop after <count> replies\n");
	printf("  -D                 print timestamps\n");
	printf("  -i <interval>      seconds between sending each packet\n");
	printf("  -q                 quiet output\n");
	printf("  -t <ttl>           define time to live\n");
	printf("  -v                 verbose output\n");
	printf("  -V                 debug and verbose output\n\n");

	// IPv4
	printf("IPv4 options:\n");
	printf("  -4                 use IPv4\n\n");

	// IPv6
	printf("IPv6 options:\n");
	printf("  -6                 use IPv6\n\n");

	//bg
	printf("Made with ♥ by hallainea and Assxios\n");
}

size_t get_number(char ***av, size_t max)
{
	if (*(**av + 1) != '\0')
	{
		char buffer[1 << 10];
		sprintf(buffer, "Invalid argument for -%c", ***av);
		ft_exit("usage error", buffer);
	}

	size_t nbr = 0;
	if (is_digit(*++*av))
		while (***av >= '0' && ***av <= '9')
			nbr = nbr * 10 + *(**av)++ - '0';

	if (nbr == 0 || nbr > max)
		ft_exit("usage error", "Value out of range");
	return nbr;
}

void options(char ***av)
{
	while (*++**av)
		switch (***av)
		{
		case 'a':
			g_ping.options.audible = true;
			break;
		case 'c':
			g_ping.options.count = get_number(av, LLONG_MAX);
			return;
		case 'D':
			g_ping.options.timestamp = true;
			break;
		case 'i':
			g_ping.options.interval = get_number(av, INT_MAX / 1000);
			return;
		case 'q':
			g_ping.options.quiet = true;
			break;
		case 't':
			g_ping.options.ttl = (int)get_number(av, UCHAR_MAX);
			return;
		case 'v':
			g_ping.options.verbose = true;
			break;
		case 'V':
			g_ping.options.debug = true;
			g_ping.options.verbose = true;
			break;
		case '4':
			if (g_ping.options.ipv6 == true)
				ft_exit("usage error", "Only one -4 or -6 option may be specified");
			g_ping.options.ipv4 = true;
			break;
		case '6':
			if (g_ping.options.ipv4 == true)
				ft_exit("usage error", "Only one -4 or -6 option may be specified");
			g_ping.options.ipv6 = true;
			break;
		default:
			print_help();
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

	if (getaddrinfo(g_ping.hostname, NULL, &hints, &g_ping.res))
		ft_exit("getaddrinfo", "Could not resolve hostname");
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
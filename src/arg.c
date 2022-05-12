#include "ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

void usage()
{
	printf("Usage: ping [-v] host\n");
	exit(1);
}

void init_host(char *hostname, char *cmd)
{
	g_ping.hostname = hostname;

	struct addrinfo hints;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	struct addrinfo *res;
	if (getaddrinfo(g_ping.hostname, NULL, &hints, &res) != 0)
	{
		printf("%s: %s: No address associated with hostname\n", cmd, g_ping.hostname);
		exit(1);
	}

	g_ping.dest.sin_family = AF_INET;
	g_ping.dest.sin_port = 0;
	g_ping.dest.sin_addr.s_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;

	inet_ntop(AF_INET, &g_ping.dest.sin_addr, g_ping.ip, INET_ADDRSTRLEN);

	freeaddrinfo(res);
}

void check_args(int argc, char *argv[])
{
	char *cmd = argv[0];

	if (argc < 2)
	{
		printf("%s: usage error: Destination address required\n", cmd);
		exit(1);
	}

	while (*++argv)
		if (**argv == '-')
			switch (*++*argv)
			{
			case 'h':
				usage();
				break;
			case 'v':
				g_ping.verbose = 1;
				break;
			}
		else if (!g_ping.hostname)
			init_host(*argv, cmd);
}
#include "ping.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

void sigint_handler()
{
	g_ping.finished = true;
}

void sigalrm_handler()
{
	if (!g_ping.replied && g_ping.sent > 0 && g_ping.verbose)
		printf("No reply from %s\n", g_ping.ip);

	if (sendto(g_ping.fd, &g_ping.packet.icmp, sizeof(g_ping.packet.icmp), 0, &g_ping.dest, g_ping.family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)) < 0)
		printf("sendto() failed: %s\n", strerror(errno));
	else
	{
		++g_ping.packet.icmp.un.echo.sequence;
		--g_ping.packet.icmp.checksum;
		++g_ping.sent;

		g_ping.replied = false;
		gettimeofday(&g_ping.last, NULL);
	}
	alarm(1);
}
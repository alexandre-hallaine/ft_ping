#include "ping.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

void sigint_handler()
{
	g_ping.finished = true;
}

void sigalrm_handler()
{
	if (!g_ping.replied && g_ping.sent > 0 && g_ping.verbose)
		printf("No reply from %s\n", g_ping.ip);

	if (sendto(g_ping.fd, &g_ping.packet, sizeof(g_ping.packet), 0, (struct sockaddr *)&g_ping.dest, sizeof(g_ping.dest)) < 0)
		printf("Unable to send packet.\n");
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
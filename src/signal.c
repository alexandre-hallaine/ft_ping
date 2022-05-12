#include "ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

void sigint_handler()
{
	g_ping.finished = true;
}

void sigalrm_handler()
{
	if (!g_ping.replied && g_ping.verbose)
		printf("Failed to ping %s\n", g_ping.hostname);

	if (sendto(g_ping.fd, &g_ping.packet, sizeof(g_ping.packet), 0, (struct sockaddr *)&g_ping.dest, sizeof(g_ping.dest)) < 0)
	{
		perror("sendto failed");
		exit(1);
	}
	++g_ping.packet.icmp.un.echo.sequence;
	++g_ping.sent;
	--g_ping.packet.icmp.checksum;

	gettimeofday(&g_ping.last, NULL);
	alarm(1);

	g_ping.replied = false;
}
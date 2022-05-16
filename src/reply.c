#include "ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

bool check_reply(t_hdr header)
{
	if (header.icmp.type == ICMP_TIME_EXCEEDED)
	{
		if (g_ping.verbose)
			printf("%d bytes from %s: Time to live excceeded\n", header.ip.tot_len / 32 / 8, g_ping.ip);
		return false;
	}
	else if (header.icmp.type != ICMP_ECHOREPLY)
	{
		if (g_ping.verbose)
			printf("%d bytes from %s: Received unexpected reply\n", header.ip.tot_len / 32 / 8, g_ping.ip);
		return false;
	}
	return ++g_ping.received;
}

#define ABS(x) ((x) < 0 ? -(x) : (x))

void update_stats(t_hdr header)
{
	struct timeval now;
	gettimeofday(&now, NULL);

	double delta = (now.tv_sec - g_ping.last.tv_sec) * 1000.0 + (now.tv_usec - g_ping.last.tv_usec) / 1000.0;
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
		   header.ip.tot_len / 32 / 8, g_ping.ip, g_ping.packet.icmp.un.echo.sequence, header.ip.ttl, delta);

	if (delta < g_ping.min || g_ping.received == 1)
		g_ping.min = delta;
	if (delta > g_ping.max)
		g_ping.max = delta;
	g_ping.sum += delta;
	g_ping.msum += ABS(g_ping.sum / g_ping.received - delta);
}

void reply_handler()
{
	t_hdr header;
	struct iovec iov = {.iov_base = &header, .iov_len = sizeof(header)};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	if (recvmsg(g_ping.fd, &msg, 0) < 0)
		return;
	g_ping.replied = true;

	if (!check_reply(header) || header.icmp.un.echo.id != g_ping.packet.icmp.un.echo.id)
		return;

	update_stats(header);
}

void print_stats()
{
	struct timeval now;
	gettimeofday(&now, NULL);

	if (g_ping.sent != 0)
	{
		printf("\n--- %s ping statistics ---\n", g_ping.hostname);
		printf("%ld packets transmitted, %ld received, %ld%% packet loss, time %ldms\n",
			g_ping.sent, g_ping.received, (g_ping.sent - g_ping.received) * 100 / g_ping.sent,
			(now.tv_sec - g_ping.start.tv_sec) * 1000 + (now.tv_usec - g_ping.start.tv_usec) / 1000);

		if (g_ping.received > 0)
			printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
				g_ping.min, g_ping.sum / g_ping.received, g_ping.max, g_ping.msum / g_ping.received);
	}
	else
		printf("\n");
}

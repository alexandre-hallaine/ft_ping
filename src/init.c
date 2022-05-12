#include "ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define TTL 60

void init_ping(void)
{
	g_ping.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_ping.fd < 0)
	{
		perror("socket failed");
		exit(1);
	}

	struct timeval timeout = {1, 0};
	if (setsockopt(g_ping.fd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0 ||
		setsockopt(g_ping.fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout)) < 0)
	{
		perror("setsockopt failed");
		exit(1);
	}

	fill_ip_header(&g_ping.packet.ip, sizeof(g_ping.packet), TTL, g_ping.dest.sin_addr.s_addr);
	fill_icmp_header(&g_ping.packet.icmp);
	g_ping.packet.icmp.checksum = checksum(&g_ping.packet.icmp, sizeof(g_ping.packet.icmp));

	struct timeval now;
	gettimeofday(&now, NULL);
	g_ping.first = now;

	if (g_ping.verbose)
		display_header((void *)&g_ping.packet);

	printf("PING %s (%s) %ld bytes of data.\n", g_ping.hostname, g_ping.ip, sizeof(g_ping.packet.icmp));
}
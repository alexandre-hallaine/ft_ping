#include "ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned short checksum(void *address, size_t len)
{
	unsigned short *src;
	unsigned long sum;

	src = (unsigned short *)address;
	sum = 0;
	while (len > 1)
	{
		sum += *src;
		src++;
		len -= sizeof(short);
	}
	if (len)
		sum += *(unsigned char *)src;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ((unsigned short)~sum);
}

void init_ping(void)
{
	g_ping.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_ping.fd < 0)
	{
		perror("socket failed");
		exit(1);
	}

	g_ping.packet.type = ICMP_ECHO;
	g_ping.packet.code = 0;
	g_ping.packet.un.echo.id = getpid();
	g_ping.packet.un.echo.sequence = 0;
	g_ping.packet.checksum = checksum(&g_ping.packet, sizeof(g_ping.packet));

	printf("PING %s (%s) %ld bytes of data.\n", g_ping.hostname, g_ping.ip, sizeof(g_ping.packet));
}
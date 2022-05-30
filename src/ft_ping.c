#include "ft_ping.h"

t_ping g_ping = {0};

void ping()
{
	if (g_ping.done == 1)
		return;
	if (sendto(g_ping.socket, &g_ping.icmp, sizeof(g_ping.icmp), 0, g_ping.res->ai_addr, g_ping.res->ai_addrlen) < 0)
		ft_exit("sendto", "Could not send packet");
	if (g_ping.debug)
		display_header_icmp(&g_ping.icmp, "ICMP Header sent");
	gettimeofday(&g_ping.last, NULL);
	g_ping.sent++;
	g_ping.reply = 0;
}

void sigint_handler()
{
	g_ping.done = 1;
	freeaddrinfo(g_ping.res);

	if (g_ping.sent > 0)
		printf("\n--- %s ping statistics ---\n%d packets transmitted, %d received, %d%% packet loss, time %.0fms\n", g_ping.hostname, g_ping.sent, g_ping.received, (int)((g_ping.sent - g_ping.received) * 100 / g_ping.sent), seconds(g_ping.begin));

	if (g_ping.received > 0)
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			   g_ping.min, g_ping.sum / g_ping.received, g_ping.max, g_ping.msum / g_ping.received);
}

void sigalrm_handler()
{
	if (g_ping.reply == 0 && g_ping.sent > 0 && g_ping.verbose)
		printf("No reply from %s\n", g_ping.hostname);

	++g_ping.icmp.un.echo.sequence;
	--g_ping.icmp.checksum;

	if (g_ping.count > 0 && g_ping.sent >= g_ping.count)
		sigint_handler();
	ping();
	alarm(1);
}

void socket_init()
{
	int on = 1;
	g_ping.socket = socket(g_ping.res->ai_family, SOCK_RAW, g_ping.res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6);
	if (g_ping.socket < 0)
		ft_exit("socket", "Could not create socket");

	struct timeval timeout = {1, 0};
	if (setsockopt(g_ping.socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
		ft_exit("setsockopt", "Could not set receive timeout");

	if (g_ping.res->ai_family == AF_INET)
	{
		if (setsockopt(g_ping.socket, IPPROTO_IP, IP_TTL, &g_ping.ttl, sizeof(g_ping.ttl)) < 0)
			ft_exit("setsockopt", "Could not set TTL");
	}
	else if (setsockopt(g_ping.socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &g_ping.ttl, sizeof(g_ping.ttl)) < 0)
		ft_exit("setsockopt", "Could not set hop limit");
	else if (setsockopt(g_ping.socket, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on)) < 0)
		ft_exit("setsockopt", "Could not set receive hop limit");
}

int main(int ac, char **av)
{
	(void)ac;
	g_ping.cmd = av[0];

	check_args(av);
	g_ping.ttl = g_ping.ttl == 0 ? 64 : g_ping.ttl;
	g_ping.count = g_ping.count == 0 ? -1 : g_ping.count;
	socket_init();

	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);

	g_ping.icmp.un.echo.id = getpid();
	g_ping.icmp.type = g_ping.res->ai_family == AF_INET ? ICMP_ECHO : ICMP6_ECHO_REQUEST;
	g_ping.icmp.checksum = g_ping.res->ai_family == AF_INET ? checksum((unsigned short *)&g_ping.icmp, sizeof(g_ping.icmp)) : g_ping.icmp.checksum;
	inet_ntop(g_ping.res->ai_family, g_ping.res->ai_family == AF_INET ? (void *)&((struct sockaddr_in *)g_ping.res->ai_addr)->sin_addr : (void *)&((struct sockaddr_in6 *)g_ping.res->ai_addr)->sin6_addr, g_ping.ip, sizeof(g_ping.ip));

	printf("PING %s (%s) %ld bytes of data.\n", g_ping.hostname, g_ping.ip, sizeof(struct icmphdr) + sizeof(struct iphdr));
	gettimeofday(&g_ping.begin, NULL);
	sigalrm_handler();

	while (g_ping.done == 0)
		recv_msg();

	return (0);
}

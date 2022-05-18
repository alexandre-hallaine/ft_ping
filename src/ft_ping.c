#include "ft_ping.h"

t_ping g_ping = {0};

// https://www.alpharithms.com/internet-checksum-calculation-steps-044921/
unsigned short checksum(unsigned short *address, size_t len)
{
	unsigned short sum = 0;
	while (len -= sizeof(short))
		sum += *address++;
	return (~sum);
}

void sigint_handler()
{
	g_ping.done = 1;
	freeaddrinfo(g_ping.res);
	if (g_ping.sent > 0)
	{
		printf("--- %s ping statistics ---\n", g_ping.hostname);
		printf("%d packets transmitted, %d received, %d%% packet loss\n", g_ping.sent, g_ping.received, (int)((g_ping.sent - g_ping.received) * 100 / g_ping.sent));
	}
	exit(1);
}

void ping()
{
	if (sendto(g_ping.socket, &g_ping.icmp, sizeof(g_ping.icmp), 0, g_ping.res->ai_addr, g_ping.res->ai_addrlen) < 0)
	{
		printf("sendto: %s\n", strerror(errno));
		return;
	}
	g_ping.sent++;
}

void *ancillary_data(struct msghdr msg, int len, int level)
{
	for (struct cmsghdr *cmsgptr = CMSG_FIRSTHDR(&msg);
		 cmsgptr != NULL;
		 cmsgptr = CMSG_NXTHDR(&msg, cmsgptr))
	{
		if (cmsgptr->cmsg_len == 0)
			break;

		if (cmsgptr->cmsg_level == len && cmsgptr->cmsg_type == level)
			return CMSG_DATA(cmsgptr);
	}
	return NULL;
}

void recv_msg()
{
	if (g_ping.reply == 1)
		return;

	t_recv buffer;
	struct iovec iov = {.iov_base = &buffer, .iov_len = sizeof(buffer)};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	if (g_ping.res->ai_family == AF_INET6)
	{
		char data[64];
		msg.msg_control = data;
		msg.msg_controllen = sizeof(data);
	}

	if (recvmsg(g_ping.socket, &msg, 0) < 0)
		return;

	unsigned short len;
	unsigned char ttl;
	if (g_ping.res->ai_family == AF_INET)
	{
		if (buffer.v4.ip.saddr != ((struct sockaddr_in *)g_ping.res->ai_addr)->sin_addr.s_addr)
			return;
		len = buffer.v4.ip.tot_len / 32 / 8;
		ttl = buffer.v4.ip.ttl;
	}
	else
	{
		// if (memcmp(buffer.v6.ip.ip6_src.s6_addr, ((struct sockaddr_in6 *)g_ping.res->ai_addr)->sin6_addr.s6_addr, sizeof(buffer.v6.ip.ip6_src.s6_addr)) == 0)
		// 	return;
		len = sizeof(buffer.v6.icmp);
		ttl = *(unsigned char *)ancillary_data(msg, IPPROTO_IPV6, IPV6_HOPLIMIT);
	}
	printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=42.42 ms\n", len, g_ping.hostname, g_ping.ip, g_ping.icmp.un.echo.sequence, ttl);
	g_ping.reply = 1;
	g_ping.received++;
}

void sigalrm_handler()
{
	if (g_ping.reply == 0 && g_ping.sent > 0)
		printf("No reply from %s\n", g_ping.hostname);
	++g_ping.icmp.un.echo.sequence;
	--g_ping.icmp.checksum;
	ping();
	g_ping.reply = 0;
	alarm(1);
}

int main(int ac, char **av)
{
	if (getuid() != 0)
	{
		printf("You must be root to run this program\n");
		return (1);
	}
	if (ac != 2)
	{
		printf("usage: ./ft_ping [hostname]\n");
		return (1);
	}

	struct addrinfo hints = {0};
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_socktype = SOCK_RAW;

	struct addrinfo *res;
	if (getaddrinfo(av[1], NULL, &hints, &res))
	{
		printf("getaddrinfo: %s\n", strerror(errno));
		return (1);
	}
	g_ping.res = res;
	g_ping.hostname = av[1];

	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);

	if ((g_ping.socket = socket(g_ping.res->ai_family, g_ping.res->ai_socktype, g_ping.res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6)) < 0)
	{
		printf("socket: %s\n", strerror(errno));
		return (1);
	}

	int on = 1;
	if (setsockopt(g_ping.socket, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on)) < 0 ||
		setsockopt(g_ping.socket, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on)) < 0)
	{
		printf("setsockopt: %s\n", strerror(errno));
		return (1);
	}

	g_ping.icmp.un.echo.id = getpid();
	g_ping.icmp.type = g_ping.res->ai_family == AF_INET ? ICMP_ECHO : ICMP6_ECHO_REQUEST;
	g_ping.icmp.checksum = g_ping.res->ai_family == AF_INET ? checksum((unsigned short *)&g_ping.icmp, sizeof(g_ping.icmp)) : g_ping.icmp.checksum;

	inet_ntop(g_ping.res->ai_family, g_ping.res->ai_family == AF_INET ? (void *)&((struct sockaddr_in *)g_ping.res->ai_addr)->sin_addr : (void *)&((struct sockaddr_in6 *)g_ping.res->ai_addr)->sin6_addr, g_ping.ip, sizeof(g_ping.ip));

	printf("PING %s (%s) %ld bytes of data.\n", g_ping.hostname, g_ping.ip, sizeof(struct icmphdr) + sizeof(struct iphdr));
	sigalrm_handler();

	while (g_ping.done == 0)
		recv_msg();

	return (0);
}
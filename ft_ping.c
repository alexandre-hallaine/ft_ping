#include "ft_ping.h"

t_ping g_ping = {0};

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

void sigint_handler()
{
	g_ping.done= 1;
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
		return ;
	}
	g_ping.sent++;
}

void recv_msg()
{
	char buffer[512];
	struct iovec iov = {.iov_base = &buffer, .iov_len = sizeof(buffer)};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	if (recvmsg(g_ping.socket, &msg, 0) < 0)
		return ;

	printf("64 bytes from %s (%s): icmp_seq=%d\n", g_ping.hostname, g_ping.ip, g_ping.icmp.un.echo.sequence);
	g_ping.reply = 1;
	g_ping.received++;
}

void sigalrm_handler()
{
	if (g_ping.reply == 0 && g_ping.sent > 0)
		printf("No reply from %s\n", g_ping.hostname);
	ping();
	++g_ping.icmp.un.echo.sequence;
	--g_ping.icmp.checksum;
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

	g_ping.icmp.type = g_ping.res->ai_family == AF_INET ? ICMP_ECHO : ICMP6_ECHO_REQUEST;
	g_ping.icmp.checksum = g_ping.res->ai_family == AF_INET ?  checksum((unsigned short *)&g_ping.icmp, sizeof(g_ping.icmp)) : g_ping.icmp.checksum;

	inet_ntop(g_ping.res->ai_family, g_ping.res->ai_family == AF_INET ? (void *)&((struct sockaddr_in *)g_ping.res->ai_addr)->sin_addr : (void *)&((struct sockaddr_in6 *)g_ping.res->ai_addr)->sin6_addr, g_ping.ip, sizeof(g_ping.ip));

	printf("PING %s (%s) %ld bytes of data.\n", g_ping.hostname,  g_ping.ip, sizeof(struct icmphdr) + sizeof (struct iphdr));
	sigalrm_handler();
	
	while (g_ping.done == 0)
		recv_msg();

	return (0);
}
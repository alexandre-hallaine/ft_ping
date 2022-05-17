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

void recv_msg()
{
	if (g_ping.reply == 1)
		return;

	t_hdr buffer;
	struct iovec iov = {.iov_base = &buffer, .iov_len = sizeof(buffer)};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	if (recvmsg(g_ping.socket, &msg, 0) < 0)
		return;

	unsigned short len;
	unsigned char ttl;
	if (g_ping.res->ai_family == AF_INET)
	{
		if (buffer.ip.v4.saddr != ((struct sockaddr_in *)g_ping.res->ai_addr)->sin_addr.s_addr)
			return;
		len = buffer.ip.v4.tot_len;
		ttl = buffer.ip.v4.ttl;
	}
	else
	{
		if (memcmp(buffer.ip.v6.ip6_src.s6_addr, ((struct sockaddr_in6 *)g_ping.res->ai_addr)->sin6_addr.s6_addr, sizeof(buffer.ip.v6.ip6_src.s6_addr)) == 0)
			return;
		len = buffer.ip.v6.ip6_plen;
		ttl = buffer.ip.v6.ip6_hlim;
	}
	printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=42.42 ms\n", len / 32 / 8, g_ping.hostname, g_ping.ip, g_ping.icmp.un.echo.sequence, ttl);
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

void init_host(char *hostname)
{
	g_ping.hostname = hostname;
	struct addrinfo hints = {0};
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_socktype = SOCK_RAW;

	struct addrinfo *res;
	if (getaddrinfo(g_ping.hostname, NULL, &hints, &res))
	{
		printf("getaddrinfo: %s\n", strerror(errno));
		exit(1);
	}
	g_ping.res = res;
}

void options(char **argv[])
{
	while (*++**argv)
		switch (***argv)
		{
		case 'v':
			g_ping.verbose = 1;
			break;
		/*case 't':
		{
			if (*(**argv + 1) != '\0')
			{
				printf("%s: invalid argument: '%s'\n", cmd, **argv + 1);
				exit(1);
			}
			int ttl = 0;
			if (*++*argv && is_digit(**argv))
				ttl = ft_atoi(**argv);
			if (ttl <= 0 || ttl > 255)
			{
				printf("%s: cannot set unicast time-to-live: Invalid argument\n", cmd);
				exit(1);
			}
			g_ping.packet.ip.ttl = ttl;
			return;
		}*/
		default:
			printf("Usage: ft_ping [-h] [-t ttl] [-v] [hostname]\n");
			exit(1);
		}
}

void check_args(char *argv[])
{
	while (*++argv)
		if (**argv == '-')
			options(&argv);
		else if (!g_ping.hostname && !argv[1])
			init_host(*argv);
		else
		{
			printf("ft_ping: usage error: Extranous argument found (%s)\n", argv[1]);
			exit(1);
		}
	if (!g_ping.hostname)
	{
		printf("ft_ping: usage error: Destination address required\n");
		exit(1);
	}
}

int main(int ac, char **av)
{
	(void)ac;
	if (getuid() != 0)
	{
		printf("You must be root to run this program\n");
		return (1);
	}
	check_args(av);

	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);

	if ((g_ping.socket = socket(g_ping.res->ai_family, g_ping.res->ai_socktype, g_ping.res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6)) < 0)
	{
		printf("socket: %s\n", strerror(errno));
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
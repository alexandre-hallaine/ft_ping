#include "ft_ping.h"

t_ping g_ping = {0};

void display_header_iphdr(struct iphdr *tmp, char *prefix)
{
	printf("\n%s\n", prefix);
	printf("  |-IP Version       : %d\n", tmp->version);
	printf("  |-IP Header Length : %d DWORDS or %d Bytes\n", tmp->ihl, (tmp->ihl * 4));
	printf("  |-Type Of Service  : %d\n", tmp->tos);
	printf("  |-IP Total Length  : %d (Size of Packet)\n", tmp->tot_len / 32 / 8);
	printf("  |-Identification   : %d\n", tmp->id);
	printf("  |-TTL              : %d\n", tmp->ttl);
	printf("  |-Protocol         : %d\n", tmp->protocol);
	printf("  |-Checksum         : %d\n", tmp->check);
	struct in_addr addr = {.s_addr = tmp->saddr};
	printf("  |-Source IP        : %s", inet_ntoa(addr));
	addr.s_addr = tmp->daddr;
}

void display_header_ip6hdr(struct ip6_hdr *header, char *prefix)
{
    printf("\n%s\n", prefix);
    printf("  |-IPv6 Version     : %d\n", (header->ip6_vfc & 0xf0) >> 4);
    printf("  |-Traffic Class    : %d\n", (header->ip6_flow & 0x0ff00000) >> 20);
    printf("  |-Flow Label       : %d\n", (header->ip6_flow & 0x000fffff));
    printf("  |-Payload Length   : %d\n", ntohs(header->ip6_plen));
    printf("  |-Next Header      : %d\n", header->ip6_nxt);
    printf("  |-Hop Limit        : %d\n", header->ip6_hlim);
    struct in_addr addr = {.s_addr = header->ip6_src.s6_addr32[3]};
    printf("  |-Source Address   : %s\n", inet_ntoa(addr));
    addr.s_addr = header->ip6_dst.s6_addr32[3];
    printf("  |-Destination      : %s", inet_ntoa(addr));
}

void display_header_icmp(struct icmphdr *icmp, char *prefix)
{
	printf("\n%s\n", prefix);
	printf("  |-Type             : %d\n", icmp->type);
	printf("  |-Code             : %d\n", icmp->code);
	printf("  |-Checksum         : %d\n", icmp->checksum);
	printf("  |-Identifier       : %d\n", icmp->un.echo.id);
	printf("  |-Sequence Number  : %d\n\n", icmp->un.echo.sequence);
}

void ft_exit(char *cmd, char *msg)
{
	printf("%s: %s: %s\n", g_ping.cmd, cmd, msg);
	exit(EXIT_FAILURE);
}

int is_digit(char *str)
{
	while ((*str >= '0' && *str <= '9'))
		str++;
	return !*str;
}

int ft_atoi(const char *nptr)
{
	ssize_t nbr = 0;
	while (*nptr >= '0' && *nptr <= '9')
		nbr = nbr * 10 + (*nptr++ - '0');
	return (nbr);
}

unsigned short checksum(unsigned short *address, size_t len)
{
	unsigned short sum = 0;
	while (len -= sizeof(short))
		sum += *address++;
	return (~sum);
}

double seconds(struct timeval start)
{
	struct timeval end;
	gettimeofday(&end, NULL);
	return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000.0;
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

void options(char ***av)
{
	while (*++**av)
		switch (***av)
		{
		case 'v':
			g_ping.verbose = 1;
			break;
		case 't':
			if (*(**av + 1) != '\0')
				ft_exit("usage error", "Invalid argument for -t");
			if (*++*av && is_digit(**av))
				g_ping.ttl = ft_atoi(**av);
			if (g_ping.ttl <= 0 || g_ping.ttl > 255)
				ft_exit("usage error", "Invalid TTL");
			return;
		case 'a':
			g_ping.audible = 1;
			break;
		case 'c':
			if (*(**av + 1) != '\0')
				ft_exit("usage error", "Invalid argument for -c");
			if (*++*av && is_digit(**av))
				g_ping.count = ft_atoi(**av);
			if (g_ping.count <= 0 || g_ping.count > 9223372036854775807)
				ft_exit("usage error", "Value out of range");
			return;
		case 'q':
			g_ping.quiet = 1;
			break;
		case 'V':
			g_ping.debug = 1;
			g_ping.verbose = 1;
			break;
		default:
			printf("Usage: ft_ping [-h] [-t ttl] [-v] [hostname]\n");
			exit(1);
		}
}

void init_host(char *hostname)
{
	g_ping.hostname = hostname;
	struct addrinfo hints = {0};
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_socktype = SOCK_RAW;

	struct addrinfo *res;
	if (getaddrinfo(g_ping.hostname, NULL, &hints, &res))
		ft_exit("getaddrinfo", "Could not resolve hostname");
	g_ping.res = res;
}

void check_args(char **av)
{
	if (getuid() != 0)
		ft_exit("usage error", "You must be root to run this program");

	while (*++av)
		if (**av == '-')
			options(&av);
		else if (!g_ping.hostname && !av[1])
			init_host(*av);
		else
			ft_exit("usage error", "Extranous argument found");
	if (!g_ping.hostname)
		ft_exit("usage error", "No hostname specified");
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

#define ABS(x) ((x) < 0 ? -(x) : (x))

void update_stats(unsigned short len, unsigned char ttl)
{
	double delta = seconds(g_ping.last);
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
		   len, g_ping.ip, g_ping.icmp.un.echo.sequence, ttl, delta);

	if (delta < g_ping.min || g_ping.received == 1)
		g_ping.min = delta;
	if (delta > g_ping.max)
		g_ping.max = delta;
	g_ping.sum += delta;
	g_ping.msum += ABS(g_ping.sum / g_ping.received - delta);
}

void recv_msg()
{
	if (g_ping.reply == 1)
		return;

	t_recv buffer;
	struct iovec iov = {.iov_base = &buffer, .iov_len = sizeof(buffer) * 2};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	if (g_ping.res->ai_family == AF_INET6)
	{
		char data[64];
		msg.msg_control = data;
		msg.msg_controllen = sizeof(data);
	}

	if ((g_ping.len = recvmsg(g_ping.socket, &msg, 0)) < 0)
		return;

	if (buffer.v4.icmp.type == ICMP_TIME_EXCEEDED || buffer.v6.icmp.type == ICMP6_TIME_EXCEEDED)
	{
		unsigned short id = g_ping.res->ai_family == AF_INET
								? ((t_recv *)buffer.v4.data)->v4.icmp.un.echo.id
								: ((struct icmphdr *)(&buffer.v6.data + sizeof(struct ip6_hdr)))->un.echo.id;
		if (id != g_ping.icmp.un.echo.id)
			return;
	}
	else if (g_ping.icmp.un.echo.id != buffer.v4.icmp.un.echo.id && g_ping.icmp.un.echo.id != buffer.v6.icmp.un.echo.id)
		return;

	if (g_ping.debug)
	{
		if (g_ping.res->ai_family == AF_INET)
			display_header_iphdr(&buffer.v4.ip, "IP Header received");
		g_ping.res->ai_family == AF_INET ? display_header_icmp(&buffer.v4.icmp, "ICMP Header received") : display_header_icmp(&buffer.v6.icmp, "ICMP Header received");
		if (g_ping.len == (sizeof(struct iphdr) + sizeof(struct icmphdr)) * 2)
		{
			if (g_ping.res->ai_family == AF_INET)
			{
				display_header_iphdr((void *)&buffer.v4 + (sizeof(struct iphdr) + sizeof(struct icmphdr)), "OLD IP Header sent");
				display_header_icmp((void *)&buffer.v4 + ((sizeof(struct iphdr) * 2) + sizeof(struct icmphdr)), "OLD ICMP Header sent");
			}
			else
			{
				display_header_ip6hdr((void *)&buffer.v6 + sizeof(struct icmphdr), "OLD IP6 Header sent");
				display_header_icmp((void *)&buffer.v6 + (sizeof(struct ip6_hdr) + sizeof(struct icmphdr)), "OLD ICMP6 Header sent");
			}
		}
	}

	g_ping.reply = 1;
	g_ping.ttl_reply = g_ping.res->ai_family == AF_INET ? buffer.v4.ip.ttl : *(unsigned char *)ancillary_data(msg, IPPROTO_IPV6, IPV6_HOPLIMIT);

	if (buffer.v4.icmp.type == ICMP_TIME_EXCEEDED || buffer.v6.icmp.type == ICMP6_TIME_EXCEEDED)
	{
		if (g_ping.verbose)
			printf("From %s (%s): Time to live excceeded\n", g_ping.hostname, g_ping.ip);
		return;
	}
	else if (buffer.v4.icmp.type != ICMP_ECHOREPLY && buffer.v6.icmp.type != ICMP6_ECHO_REPLY)
	{
		if (g_ping.verbose)
			printf("From %s (%s): Unknow ICMP type\n", g_ping.hostname, g_ping.ip);
		return;
	}

	g_ping.received++;
	if (g_ping.quiet)
		return;
	if (g_ping.audible)
		printf("\a");

	update_stats(g_ping.len, g_ping.ttl_reply);
	if (g_ping.count > 0 && g_ping.sent >= g_ping.count)
		sigint_handler();
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

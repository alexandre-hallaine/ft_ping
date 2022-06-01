#include "ft_ping.h"

#include <netdb.h>
#include <netinet/icmp6.h>

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

bool validate_id(t_recv *buffer)
{
	if (g_ping.utils.replied)
		return false;
	if (buffer->v4.icmp.type == ICMP_TIME_EXCEEDED || buffer->v6.icmp.type == ICMP6_TIME_EXCEEDED)
	{
		unsigned short id = g_ping.res->ai_family == AF_INET
								? ((t_recv *)buffer->v4.data)->v4.icmp.un.echo.id
								: ((struct icmphdr *)(&buffer->v6.data + sizeof(struct ip6_hdr)))->un.echo.id;
		if (id != g_ping.icmp.un.echo.id)
			return false;
	}
	else if (g_ping.icmp.un.echo.id != buffer->v4.icmp.un.echo.id && g_ping.icmp.un.echo.id != buffer->v6.icmp.un.echo.id)
		return false;
	return (g_ping.utils.replied = true);
}

void debug(t_recv buffer)
{
	if (g_ping.res->ai_family == AF_INET)
		display_header_iphdr(&buffer.v4.ip, "IP Header received");
	g_ping.res->ai_family == AF_INET ? display_header_icmp(&buffer.v4.icmp, "ICMP Header received") : display_header_icmp(&buffer.v6.icmp, "ICMP6 Header received");
	if (buffer.v4.icmp.type == ICMP_TIME_EXCEEDED || buffer.v6.icmp.type == ICMP6_TIME_EXCEEDED)
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

bool check_error(t_recv buffer)
{
	if (buffer.v4.icmp.type == ICMP_TIME_EXCEEDED || buffer.v6.icmp.type == ICMP6_TIME_EXCEEDED)
	{
		if (g_ping.options.verbose)
			printf("From %s (%s): Time to live excceeded\n", g_ping.hostname, g_ping.ip);
		return true;
	}
	else if (buffer.v4.icmp.type != ICMP_ECHOREPLY && buffer.v6.icmp.type != ICMP6_ECHO_REPLY)
	{
		if (g_ping.options.verbose)
			printf("From %s (%s): Unknow ICMP type\n", g_ping.hostname, g_ping.ip);
		return true;
	}
	return false;
}

void recv_msg()
{
	t_recv buffer;
	struct iovec iov = {.iov_base = &buffer, .iov_len = sizeof(buffer) * 2};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	if (g_ping.res->ai_family == AF_INET6)
	{
		char data[64];
		msg.msg_control = data;
		msg.msg_controllen = sizeof(data);
	}

	ssize_t len = recvmsg(g_ping.socket, &msg, 0);
	if (len < 0 || !validate_id(&buffer))
		return;

	if (g_ping.options.debug)
		debug(buffer);
	if (check_error(buffer))
		return;

	g_ping.stats.received++;
	if (g_ping.options.quiet)
		return;
	if (g_ping.options.audible)
		printf("\a");
	if (g_ping.options.timestamp)
		printf("[%f] ", seconds((struct timeval){0, 0}) / 1000.0);

	update_stats(len, g_ping.res->ai_family == AF_INET ? buffer.v4.ip.ttl : *(unsigned char *)ancillary_data(msg, IPPROTO_IPV6, IPV6_HOPLIMIT));
	if (g_ping.options.count > 0 && g_ping.stats.send >= g_ping.options.count)
		sigint_handler();
}
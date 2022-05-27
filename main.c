#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <stdbool.h>
#include <sys/time.h>

typedef union
{
	struct
	{
		struct iphdr ip;
		struct icmphdr icmp;
	} v4;
	struct
	{
		struct icmphdr icmp;
	} v6;
} t_data;

unsigned short checksum(unsigned short *address, size_t len)
{
	unsigned short sum = 0;
	while (len -= sizeof(short))
		sum += *address++;
	return (~sum);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <hostname>\n", argv[0]);
		return (1);
	}

	struct addrinfo hints = {0};
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP | IPPROTO_ICMPV6;

	struct addrinfo *res;
	if (getaddrinfo(argv[1], NULL, &hints, &res) != 0)
	{
		printf("getaddrinfo: %s\n", gai_strerror(errno));
		return (1);
	}

	int fd = socket(res->ai_family, SOCK_RAW, res->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6);
	if (fd < 0)
	{
		printf("socket: %s\n", strerror(errno));
		return (1);
	}

	struct icmphdr icmp = {0};
	icmp.type = res->ai_family == AF_INET ? ICMP_ECHO : ICMP6_ECHO_REQUEST;
	icmp.checksum = checksum((void *)&icmp, sizeof(struct icmphdr));

	printf("Sending ICMP echo request...\n");

	if (sendto(fd, &icmp, sizeof(icmp), 0, res->ai_addr, res->ai_addrlen) < 0)
	{
		printf("sendto: %s\n", strerror(errno));
		return (1);
	}

	struct timeval send;
	gettimeofday(&send, NULL);

	t_data data;
	struct iovec iov = {.iov_base = &data, .iov_len = sizeof(data)};
	struct msghdr msg = {.msg_iov = &iov, .msg_iovlen = 1};

	bool received = false;
	while (!received)
	{
		ssize_t len = recvmsg(fd, &msg, 0);
		if (len < 0)
		{
			printf("recvmsg: %s\n", strerror(errno));
			return (1);
		}

		if (data.v4.icmp.type == ICMP_ECHOREPLY || data.v6.icmp.type == ICMP6_ECHO_REPLY)
		{
			struct timeval recv;
			gettimeofday(&recv, NULL);
			printf("Received ICMP echo reply (%zd bytes, %.3f ms)\n", len, (recv.tv_sec - send.tv_sec) * 1000.0 + (recv.tv_usec - send.tv_usec) / 1000.0);
			received = true;
		}
	}

	freeaddrinfo(res);
	return 0;
}

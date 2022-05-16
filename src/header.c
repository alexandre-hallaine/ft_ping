#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <netinet/ip_icmp.h>

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

void fill_ip_header(struct iphdr *ip, uint16_t size, uint32_t dest)
{
	ip->version = 4;					// 4
	ip->ihl = sizeof(struct iphdr) / 4; // Internet header length in 32-bit words.

	ip->tos = 0;		// 0
	ip->tot_len = size; // Length of internet header and data in octets.

	// Used in fragmentation https://datatracker.ietf.org/doc/html/rfc792#ref-1
	ip->id = 0;
	ip->frag_off = 0;

	// Time to live in seconds; as this field is decremented at each
	//   machine in which the datagram is processed, the value in this
	//   field should be at least as great as the number of gateways which
	//   this datagram will traverse.
	//ip->ttl = 60; //set by default

	ip->protocol = IPPROTO_ICMP; // ICMP = 1

	// The 16 bit one's complement of the one's complement sum of all 16
	//   bit words in the header.  For computing the checksum, the checksum
	//   field should be zero.  This checksum may be replaced in the
	//   future.
	ip->check = 0;

	//  The address of the gateway or host that composes the ICMP message.
	//   Unless otherwise noted, this can be any of a gateway's addresses.
	ip->saddr = INADDR_ANY; // any address
	// The address of the gateway or host to which the message should be
	//   sent.
	ip->daddr = dest;
}

void fill_icmp_header(struct icmphdr *icmp, uint16_t type)
{
	icmp->type = type; // 8 for echo message;
	icmp->code = 0;			// 0

	// If code = 0, an identifier to aid in matching echos and replies,
	//   may be zero.
	icmp->un.echo.id = getpid();

	// If code = 0, a sequence number to aid in matching echos and
	//   replies, may be zero.
	icmp->un.echo.sequence = 0;

	// The checksum is the 16-bit ones's complement of the one's
	//   complement sum of the ICMP message starting with the ICMP Type.
	//   For computing the checksum , the checksum field should be zero.
	//   If the total length is odd, the received data is padded with one
	//   octet of zeros for computing the checksum.  This checksum may be
	//   replaced in the future
	icmp->checksum = 0;
}

void display_header(void *address)
{
	struct iphdr *tmp = (void *)address;
	// display the IP header
	printf("\nIP header:\n");
	printf("  |-IP Version       : %d\n", (unsigned int)tmp->version);
	printf("  |-IP Header Length : %d DWORDS or %d Bytes\n", (unsigned int)tmp->ihl,
		   (unsigned int)(tmp->ihl * 4));
	printf("  |-Type Of Service  : %d\n", (unsigned int)tmp->tos);
	printf("  |-IP Total Length  : %d (Size of Packet)\n",
		   (unsigned int)tmp->tot_len);
	printf("  |-Identification   : %d\n", (unsigned int)tmp->id);
	printf("  |-TTL              : %d\n", (unsigned int)tmp->ttl);
	printf("  |-Protocol         : %d\n", (unsigned int)tmp->protocol);
	printf("  |-Checksum         : %d\n", (unsigned int)tmp->check);
	struct in_addr addr = {.s_addr = tmp->saddr};
	printf("  |-Source IP        : %s\n", inet_ntoa(addr));
	addr.s_addr = tmp->daddr;
	printf("  |-Destination IP   : %s\n", inet_ntoa(addr));

	struct icmphdr *icmp = (void *)address + sizeof(struct iphdr);
	// display the ICMP header
	printf("\nICMP header:\n");
	printf("  |-Type             : %d\n", (unsigned int)icmp->type);
	printf("  |-Code             : %d\n", (unsigned int)icmp->code);
	printf("  |-Checksum         : %d\n", (unsigned int)icmp->checksum);
	printf("  |-Identifier       : %d\n", (unsigned int)icmp->un.echo.id);
	printf("  |-Sequence Number  : %d\n", (unsigned int)icmp->un.echo.sequence);

	printf("\n");
}
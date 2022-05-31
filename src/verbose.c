#include "ft_ping.h"

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
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(tmp->saddr), ip, INET_ADDRSTRLEN);
	printf("  |-Source IP        : %s\n", ip);
	inet_ntop(AF_INET, &(tmp->daddr), ip, INET_ADDRSTRLEN);
	printf("  |-Destination IP   : %s\n", ip);
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
	char ip[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &(header->ip6_src), ip, INET6_ADDRSTRLEN);
	printf("  |-Source IP        : %s\n", ip);
	inet_ntop(AF_INET6, &(header->ip6_dst), ip, INET6_ADDRSTRLEN);
	printf("  |-Destination IP   : %s\n", ip);
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

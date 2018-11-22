/**
 * @file icmpResolve.c
 * @provides imcpResolve
 */

#include <cstdlib>
#include <xinu.h>
#include "icmp.h"
#include "../arp/arp.h"

 /**
  * Generate a sequence of ARP requests.
  *
  * @param *ipaddr pointer to the IP address
  */
process echoRequest(int dev, uchar* ipaddr, struct icmp_t* ping)
{
	uchar packet[PKTSZ];
	struct ethergram *ether = (struct ethergram*) packet;
	struct icmp_t *icmp		= (struct icmp_t*) ether->data;
	struct ipgram *dgram	= (struct ipgram*) icmp->ip_header;
	struct icmp_header_t *icmp_header = (struct icmp_header_t*) icmp->icmp_header;
	int i;

	// Send ARP Request
	uchar macaddr[ETH_ADDR_LEN];
	if (OK != arpResolve(ipaddr, macaddr))
	{
		fprintf(stdout, "Could not resolve %s\n", ipaddr);
		return 1;
	}

	// Construct the echoRequest
	bzero(packet, PKTSZ);

	// Construct Ethernet Header
	for (i = 0; i < ETH_ADDR_LEN; i++)	ether->dst[i] = 0xFF;
	control(dev, ETH_CTRL_GET_MAC, (ulong)(ether->src), 0);
	ether->type = htons(ETYPE_IPv4);

	// Construct IP Header
	dgram->ver_ihl = (IPv4_VERSION << 4) | (IPv4_HDR_LEN >> 2);
	dgram->tos = IPv4_TOS_ROUTINE; /*Type 0*/
	dgram->len = 0; /* Set Checksum and Length later */
	dgram->id = htons(currpid);
	dgram->flags_froff = 0;
	dgram->ttl = 63;
	dgram->proto = IPv4_PROTO_ICMP; /*Protocol 1*/
	dgram->chksum = 0; /* Set Checksum and Length later */
	for (i = 0; i < IPv4_ADDR_LEN; i++) dgram->src[i] = 0x00;
	for (i = 0; i < IPv4_ADDR_LEN; i++) dgram->dst[i] = 0xFF;
	
	// Contrust ICMP Header
	icmp_header->type = ECHO_REQUEST;
	icmp_header->code = 0; /* see https://tools.ietf.org/html/rfc792 */
	icmp_header->identifier = htons(currpid);
	icmp_header->sequence_number = 0;
	icmp_header->checksum = checksum((uchar *)icmp_header,
		(4 * (dgram->ver_ihl & IPv4_IHL)));

	// Set IP Header Checksum and Length
	dgram->len = (sizeof(struct ipgram) + sizeof(struct icmp_header_t))/ sizeof(uchar); // calculate size in bytes
	dgram->chksum = checksum((uchar *)dgram,
		(4 * (dgram->ver_ihl & IPv4_IHL)));

	// Send the echoRequest (ping)
	size_t icmp_size = (sizeof(struct ipgram) + sizeof(struct icmp_header_t));
	write(dev, (uchar *)packet, icmp_size);

	sleep(1000);

	return OK;
}

int icmpResolve(uchar* ipaddr, struct icmp_t* ping)
{
	size_t icmp_size = (sizeof(struct ipgram) + sizeof(struct icmp_header_t));
	struct icmp_t* my_ping = (struct icmp_t*) malloc(icmp_size);
	memcpy(my_ping, 0, icmp_size); // Clear out icmp packet 

	message m;

	// Spawn a process to send ping request.
	ready(create
	((void *)echoRequest, INITSTK,
		proctab[currpid].priority + 1,
		"ECHO requester", 3,
		 ETH0, ipaddr, my_ping), RESCHED_NO);

	m = receive();
	if (TIMEOUT == m)
	{
		return SYSERR;
	}

	if (0 == memcmp(my_ping->ip_header->dst, ipaddr, IP_ADDR_LEN))
	{
		memcpy(ping, my_ping, sizeof(struct icmp_t));
		return OK;
	}

	free(my_ping);

	return SYSERR;
}

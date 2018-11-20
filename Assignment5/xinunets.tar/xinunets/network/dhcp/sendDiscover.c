/**
 * @file sendDiscover.c
 * @provides sendDiscover.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "dhcp.h"

void sendDiscover(int dev)
{
	uchar packet[PKTSZ];
	uchar *opt = NULL;
	struct ethergram *egram = (struct ethergram *)packet;
	struct ipgram    *dgram = (struct ipgram    *)egram->data;
	struct udpgram   *ugram = (struct udpgram   *)dgram->opts;
	struct dhcpgram  *dhcp  = (struct dhcpgram  *)ugram->data;
	int i;

	bzero(packet, PKTSZ);

	for (i = 0; i < ETH_ADDR_LEN; i++)	egram->dst[i] = 0xFF;
	control(dev, ETH_CTRL_GET_MAC, (ulong)(egram->src), 0);
	egram->type = htons(ETYPE_IPv4);

	dgram->ver_ihl = (IPv4_VERSION << 4) | (IPv4_HDR_LEN >> 2);
	dgram->tos = IPv4_TOS_ROUTINE;
	dgram->len = 0; /* Update later */
	dgram->id  = htons(currpid);
	dgram->flags_froff = 0;
	dgram->ttl = 63;
	dgram->proto = IPv4_PROTO_UDP;
	dgram->chksum = 0; /* FIXME */
	for (i = 0; i < IPv4_ADDR_LEN; i++) dgram->src[i] = 0x00;
	for (i = 0; i < IPv4_ADDR_LEN; i++) dgram->dst[i] = 0xFF;

	ugram->srcPort = htons(BOOTPC);
	ugram->dstPort = htons(BOOTPS);
	ugram->len     = 0; /* Update later */
	ugram->chksum  = 0;

	dhcp->opcode   = DHCP_OPCODE_REQUEST;
	dhcp->htype    = 1; // Ethernet
	dhcp->hlen     = 6; // MAC addresses
	dhcp->hops     = 0;
	i = htonl(currpid);
	memcpy(&(dhcp->id), &i, sizeof(dhcp->id));
	dhcp->elapsed  = clocktime;
	dhcp->flags    = 0;
	for (i = 0; i < IPv4_ADDR_LEN; i++)
	{
		dhcp->client[i] = 0x00;
		dhcp->yourIP[i] = 0x00;
		dhcp->server[i] = 0x00;
		dhcp->router[i] = 0x00;
	}
	control(dev, ETH_CTRL_GET_MAC, (ulong)(dhcp->hwaddr), 0);

	opt = &dhcp->opts[0];
	*opt++ = (DHCP_MAGIC_COOKIE >> 24) & 0xFF;
	*opt++ = (DHCP_MAGIC_COOKIE >> 16) & 0xFF;
	*opt++ = (DHCP_MAGIC_COOKIE >>  8) & 0xFF;
	*opt++ = (DHCP_MAGIC_COOKIE >>  0) & 0xFF;

	*opt++ = DHCP_OPTIONS_MESSAGE;
	*opt++ = 1;
	*opt++ = DHCP_MESSAGE_DISCOVER;
	
	*opt++ = DHCP_OPTIONS_END;

	ugram->len = htons((opt - (uchar *)ugram));
	dgram->len = htons((opt - (uchar *)dgram));
	dgram->chksum = checksum((uchar *)dgram, 
							 (4 * (dgram->ver_ihl & IPv4_IHL)));

//	for (i = 0; i < ((ulong)opt - (ulong)packet); i++)
//	{
//		printf(" %02X", packet[i]);
//		if ((i % 16) == 15) { printf("\n"); }
//	}
//	printf("\n");
	write(dev, packet, opt - packet);
}


/**
 * @file netDaemon.c
 * @provides netDaemon.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "arp/arp.h"
#include "dhcp/dhcp.h"

#define MAXBUFS 32

/**
 * Initialize network interface.
 */
void netDaemon(int dev, struct netif *nif)
{
	uchar bcast[ETH_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uchar *packet = NULL;
	struct ethergram *egram = NULL;
	struct ipgram    *dgram = NULL;
	int dhcpd = 0;
	int len   = 0;
	int bufpoolID = 0;

	control(dev, ETH_CTRL_GET_MAC, (ulong)nif->mac, 0);

	dhcpd = create((void *)dhcpclient, INITSTK, INITPRIO*2, "DHCPC", 2,
				   ETH0, nif);
	ready(dhcpd, RESCHED_YES);

	bufpoolID = bfpalloc(PKTSZ, MAXBUFS);
	if (SYSERR == bufpoolID)
	{
		printf("NetDaemon ERROR: Could not allocate buffer pool!\n");
		return;
	}
	
	while (1)
	{
		packet = bufget(bufpoolID);
		if (SYSERR == (ulong)packet)
		{
			printf("NetDaemon ERROR: Could not allocate buffer!\n");
			return;
		}
		len = read(dev, packet, PKTSZ);
		if (len <= 0)
		{
			printf("NetDaemon ERROR: Could not read packet!\n");
			return;	
		}

		egram = (struct ethergram *)packet;

		if ((0 != memcmp(egram->dst, nif->mac, ETH_ADDR_LEN))
			&& (0 != memcmp(egram->dst, bcast, ETH_ADDR_LEN)))
			continue;

		switch (ntohs(egram->type))
		{
		case ETYPE_ARP:
			arpReceive(dev, packet);
			break;

		case ETYPE_IPv4:
			dgram = (struct ipgram    *)egram->data;
			if (IPv4_PROTO_UDP == dgram->proto)
			{
				send(dhcpd, (ulong)packet);
			}
			break;

		default:
			printf("Packet for me!  Unknown type 0x%04X\n",
				   ntohs(egram->type));
		}
	}
	return;
}

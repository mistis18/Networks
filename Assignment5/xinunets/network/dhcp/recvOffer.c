/**
 * @file recvOffer.c
 * @provides recvOffer.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "dhcp.h"

bool recvOffer(int dev, struct netif *nif, uchar *packet)
{
	uchar *opt = NULL;
	uchar myMac[ETH_ADDR_LEN];
	struct ethergram *egram = (struct ethergram *)packet;
	struct ipgram    *dgram = (struct ipgram    *)egram->data;
	struct udpgram   *ugram = (struct udpgram   *)dgram->opts;
	struct dhcpgram  *dhcp  = (struct dhcpgram  *)ugram->data;
	int len = 0, option = 0;
	int i = 0;

	control(dev, ETH_CTRL_GET_MAC, (ulong)myMac, 0);
	/* for (i = 0; i < ETH_ADDR_LEN; i++)	 */
	/* 	if (egram->dst[i] != myMac[i]) return FALSE; */
	/* if (egram->type != htons(ETYPE_IPv4)) */
	/* 	return FALSE; */

	if (dgram->ver_ihl != ((IPv4_VERSION << 4) | (IPv4_HDR_LEN >> 2)))
		return FALSE;

	if (dgram->proto != IPv4_PROTO_UDP) return FALSE;

	if (ugram->srcPort != htons(BOOTPS)) return FALSE;
	if (ugram->dstPort != htons(BOOTPC)) return FALSE;

	if (dhcp->opcode   != DHCP_OPCODE_REPLY) return FALSE;
	if (dhcp->htype    != 1) return FALSE;
	if (dhcp->hlen     != 6) return FALSE;
	memcpy(&i, &(dhcp->id), sizeof(i));
	if (ntohl(i) != currpid)
	{
		printf("DHCP recvOffer: id not for me! %d vs %d.\n",
			   ntohs(dgram->id), currpid);
		return FALSE;
	}
	memcpy(nif->ip, dhcp->yourIP, IPv4_ADDR_LEN);

	/* printf("DHCP Client = "); */
	/* for (i = 0; i < IPv4_ADDR_LEN; i++) */
	/* { */
	/* 	printf("%d", dhcp->client[i]); */
	/* 	if (i < (IPv4_ADDR_LEN - 1)) printf("."); else printf("\n"); */
	/* } */
	/* printf("DHCP YourIP = "); */
	/* for (i = 0; i < IPv4_ADDR_LEN; i++) */
	/* { */
	/* 	printf("%d", dhcp->yourIP[i]); */
	/* 	if (i < (IPv4_ADDR_LEN - 1)) printf("."); else printf("\n"); */
	/* } */
	/* printf("DHCP Server = "); */
	/* for (i = 0; i < IPv4_ADDR_LEN; i++) */
	/* { */
	/* 	printf("%d", dhcp->server[i]); */
	/* 	if (i < (IPv4_ADDR_LEN - 1)) printf("."); else printf("\n"); */
	/* } */
	/* printf("DHCP Router = "); */
	/* for (i = 0; i < IPv4_ADDR_LEN; i++) */
	/* { */
	/* 	printf("%d", dhcp->router[i]); */
	/* 	if (i < (IPv4_ADDR_LEN - 1)) printf("."); else printf("\n"); */
	/* } */

	opt = &dhcp->opts[0];
	if (*opt++ != ((DHCP_MAGIC_COOKIE >> 24) & 0xFF)) return FALSE;
	if (*opt++ != ((DHCP_MAGIC_COOKIE >> 16) & 0xFF)) return FALSE;
	if (*opt++ != ((DHCP_MAGIC_COOKIE >>  8) & 0xFF)) return FALSE;
	if (*opt++ != ((DHCP_MAGIC_COOKIE >>  0) & 0xFF)) return FALSE;


	while (*opt != DHCP_OPTIONS_END)
	{
		option = opt[0];
		len = opt[1];
		opt += 2;

		switch (option)
		{
		case DHCP_OPTIONS_SUBNET_MASK:
			memcpy(nif->mask, opt, IPv4_ADDR_LEN);
			opt += len;
			break;

		case DHCP_OPTIONS_ROUTER:
			memcpy(nif->router, opt, IPv4_ADDR_LEN);
			opt += len;
			break;

		case DHCP_OPTIONS_DNS_SERVER:
			memcpy(nif->dns, opt, 
				   ((sizeof(nif->dns) > len) ? (sizeof(nif->dns)) : len));
			opt += len;
			break;

		default:
			opt += len;
			break;
		}
	}

	nif->state = NETIF_STATE_BOUND;
	return TRUE;

}


/**
 * @file arpReply.c
 * @provides arpReply
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "arp.h"

/**
 * Swap and complete an arp request.
 *
 * @param *buf pointer to the ethernet pkt
 */
int arpReply(int dev, uchar *buf)
{
	struct ethergram *ether = (struct ethergram *)buf;
    struct arpgram   *gram  = (struct arpgram   *)ether->data;
    uchar enet[ETH_ADDR_LEN];
    uchar ip[IP_ADDR_LEN];

    if (SYSERR == getmac(dev, enet))
    {
		buffree(buf);
        return SYSERR;
    }

    if (SYSERR == getip(dev, ip))
    {
		buffree(buf);
        return SYSERR;
    }

	// Check if request is for me.
    if (0 == memcmp(ip, gram->tpa, IP_ADDR_LEN))
    {
		memcpy(ether->dst, ether->src, ETH_ADDR_LEN);
		memcpy(ether->src, enet, ETH_ADDR_LEN);

        gram->oper = htons(ARP_REPLY);

        memcpy(gram->tha, gram->sha, ETH_ADDR_LEN);
        memcpy(gram->tpa, gram->spa, IP_ADDR_LEN);
        memcpy(gram->sha, enet, ETH_ADDR_LEN);
        memcpy(gram->spa, ip, IP_ADDR_LEN);

		write(dev, (uchar *)buf, 
		  sizeof(struct ethergram) + sizeof(struct arpgram));
    }
    else
    {
		buffree(buf);
        return SYSERR;
    }
	buffree(buf);	
    return OK;
}

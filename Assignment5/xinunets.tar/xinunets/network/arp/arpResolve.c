/**
 * @file arpResolve.c
 * @provides arpResolve
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "arp.h"

local arpAlloc(void)
{
	int index = 0;
	int oldindex = 0;
	ulong oldtime = 0;

	if (SYSERR == wait(arplock))
		return SYSERR;

	// Look for a free entry.
	for (index = 0; index < NARP; index++)
	{
		if (ARPFREE == arptab[index].state)
		{
			arptab[index].state = ARPPENDING;
			signal(arplock);
			return index;
		}
	}
	// Look for oldest entry in use.
	oldindex = -1;
	oldtime  = 0xFFFFFFFF;
	for (index = 0; index < NARP; index++)
	{
		if ((ARPRESOLVED == arptab[index].state)
			&& (arptab[index].timestamp < oldtime))
		{
			oldindex = index;
			oldtime  = arptab[index].timestamp;
		}

	}
	if ((oldindex >= 0) && (oldindex < NARP))
	{
		bzero(&arptab[oldindex], sizeof(struct arpentry));
		arptab[oldindex].state = ARPPENDING;
		signal(arplock);
		return oldindex;
	}

	signal(arplock);
	return SYSERR;
}

/**
 * Generate a sequence of ARP requests.
 *
 * @param *ipaddr pointer to the IP address
 */
process arpRequest(int dev, int arpindex, uchar *ipaddr)
{
	uchar buffer[PKTSZ];
	struct ethergram *ether = (struct ethergram *)buffer;
	struct arpgram   *arp   = (struct arpgram *)ether->data;
	int i;

	// Construct an ARP request.
	for (i = 0; i < ETH_ADDR_LEN; i++)
	{ ether->dst[i] = 0xFF; }
	getmac(dev, ether->src);
	ether->type = htons(ETYPE_ARP);
	arp->htype = htons(1);
	arp->ptype = htons(ETYPE_IPv4);
	arp->hlen  = ETH_ADDR_LEN;
	arp->plen  = IP_ADDR_LEN;
	arp->oper  = htons(ARP_REQUEST);
	getmac(dev, arp->sha);
	getip(dev, arp->spa);
    bzero(arp->tha, ETH_ADDR_LEN);
    memcpy(arp->tpa, ipaddr, IP_ADDR_LEN);

	write(dev, (uchar *)buffer, 
		  sizeof(struct ethergram) + sizeof(struct arpgram));

	sleep(1000);

	if (ARPPENDING != arptab[arpindex].state) { return OK; }

	write(dev, (uchar *)buffer, 
		  sizeof(struct ethergram) + sizeof(struct arpgram));
	
	sleep(1000);	
	
	if (ARPPENDING != arptab[arpindex].state) { return OK; }

	write(dev, (uchar *)buffer, 
		  sizeof(struct ethergram) + sizeof(struct arpgram));
	
	sleep(1000);	
	
	if (ARPPENDING != arptab[arpindex].state) { return OK; }

	send(arptab[arpindex].waitingpid, TIMEOUT);
		
    return OK;
}

/**
 * Generate an ARP request and wait for a reply.
 *
 * @param *ipaddr pointer to the IP address
 */
int arpResolve(uchar *ipaddr, uchar *mac)
{
	int arpindex;
	struct arpentry myarp;
	message m;

	// Check ARP cache for an entry.
	for (arpindex = 0; arpindex < NARP; arpindex++)
	{
		if (0 == memcmp(arptab[arpindex].ip, ipaddr, IP_ADDR_LEN))
		{
			wait(arplock);
			if ((ARPRESOLVED == arptab[arpindex].state)
				&& (0 == memcmp(arptab[arpindex].ip, ipaddr, IP_ADDR_LEN)))
			{
				memcpy(mac, arptab[arpindex].mac, ETH_ADDR_LEN);
				signal(arplock);
				return OK;
			}
			signal(arplock);
		}
	}
	// No entry.  Create a fresh entry.
	if (SYSERR == (arpindex = arpAlloc()))
	{
		return SYSERR;
	}
	memcpy(arptab[arpindex].ip, ipaddr, IP_ADDR_LEN);
	// Note that this process awaits resolution.
	arptab[arpindex].waitingpid = currpid;
	// Spawn a process to send requests.
	ready(create
          ((void *)arpRequest, INITSTK, 
		   proctab[currpid].priority + 1, 
		   "ARP requester", 3,
           ETH0, arpindex, ipaddr), RESCHED_NO);

	m = receive();
	if ((TIMEOUT == m) && (ARPPENDING == arptab[arpindex].state))
	{
		arptab[arpindex].state = ARPFREE;
		return SYSERR;
	}

	wait(arplock);
	memcpy((char *)&myarp, &arptab[arpindex], sizeof(myarp));
	signal(arplock);

	if ((ARPRESOLVED == myarp.state) &&
		(0 == memcmp(myarp.ip, ipaddr, IP_ADDR_LEN)))
	{
		memcpy(mac, myarp.mac, ETH_ADDR_LEN);
		return OK;
	}
	return SYSERR;
}

/**
 * @file netInit.c
 * @provides netInit.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "arp/arp.h"


struct netif nif0;

void netDaemon(int dev, struct netif *nif);

/**
 * Initialize network interface.
 */
void netInit(void)
{
	open(ETH0);
	bzero(&nif0, sizeof(struct netif));

	arpInit();

	ready(create((void *)netDaemon, INITSTK, INITPRIO*2, "NetD", 2, 
				 ETH0, &nif0),
		  RESCHED_NO);

	return;
}

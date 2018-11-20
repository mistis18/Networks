/**
 * @file netif.h
 */
/* Embedded Xinu, Copyright (C) 2012.  All rights reserved. */

#define NETIF_STATE_DOWN    0
#define NETIF_STATE_BINDING 1
#define NETIF_STATE_BOUND   2

struct netif
{
	uchar state;
	uchar mac[ETH_ADDR_LEN];
	uchar ip[IPv4_ADDR_LEN];
	uchar mask[IPv4_ADDR_LEN];
	uchar router[IPv4_ADDR_LEN];
	uchar dns[IPv4_ADDR_LEN*3];
};

extern struct netif nif0;

int getmac(int dev, uchar *dst);
int getip(int dev, uchar *dst);

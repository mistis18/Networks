/**
 * @file netUtils.c
 * @provides getmac, getip
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>

int getmac(int dev, uchar *dst)
{
	if (ETH0 == dev)
	{
		memcpy(dst, nif0.mac, ETH_ADDR_LEN);
		return OK;
	}
	return SYSERR;
}

int getip(int dev, uchar *dst)
{
	if (ETH0 == dev)
	{
		if (nif0.state != NETIF_STATE_BOUND) return SYSERR;
		memcpy(dst, nif0.ip, IPv4_ADDR_LEN);
		return OK;
	}
	return SYSERR;
}

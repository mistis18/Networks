/**
 * @file     xsh_netstat.c
 * @provides xsh_netstat
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>

/**
 * Shell command (netstat) displays status of net interface status
 * @param nargs count of arguments in args
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_netstat(int nargs, char *args[])
{
	struct netif *nif = &nif0;
	int i = 0;

	printf("ETH0 State = ");
	switch (nif->state)
	{
	case NETIF_STATE_DOWN:		printf("[DOWN]\n");    break;
	case NETIF_STATE_BINDING:   printf("[BINDING]\n"); break;
	case NETIF_STATE_BOUND:     printf("[BOUND]\n");   break;
	default:                    printf("????\n");      break;
	}

	if (NETIF_STATE_BOUND != nif->state) return OK;
	printf("\tMAC    = ");
	for (i = 0; i < ETH_ADDR_LEN; i++)
	{ 
		printf("%02X", nif->mac[i]); 
		if (i < ETH_ADDR_LEN - 1) { printf(":"); } else { printf("\n"); }
	}
	printf("\tIP     = %u.%u.%u.%u\n",
		   nif->ip[0], nif->ip[1], nif->ip[2], nif->ip[3]);
	printf("\tMask   = %u.%u.%u.%u\n",
		   nif->mask[0], nif->mask[1], nif->mask[2], nif->mask[3]);
	printf("\tRouter = %u.%u.%u.%u\n",
		   nif->router[0], nif->router[1], nif->router[2], nif->router[3]);
	printf("\tDNS    = ");
	for (i = 0; i < sizeof(nif->dns); i++)
	{ 
		printf("%u", nif->dns[i]); 
		if (i >= (sizeof(nif->dns) - 1))
		{
			printf("\n");
			continue;
		}
		if ((i > 0) && ((i % IPv4_ADDR_LEN) == (IPv4_ADDR_LEN - 1)))
		{ printf(", "); } else { printf("."); }
	}

	return OK;
}

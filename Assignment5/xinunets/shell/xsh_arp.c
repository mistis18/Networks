/* Embedded Xinu, Copyright (C) 2010.  All rights reserved. */

#include <xinu.h>
#include "../network/arp/arp.h"

#define MAXBUF 1500

/**
 * Displays an ARP table line.
 * @return OK.
 */
local arpLine(struct arpentry *arpptr)
{
	char workstr[20];
	int j;

	for (j = 0; j < ETH_ADDR_LEN; j++)
	{
		printf("%02X", arpptr->mac[j]);
		if (j < (ETH_ADDR_LEN - 1))
		{ printf(":"); }
	}
			
	sprintf(workstr, "%d.%d.%d.%d",
			arpptr->ip[0],
			arpptr->ip[1],
			arpptr->ip[2],
			arpptr->ip[3]);
	
	for (j = strnlen(workstr, 20); j < (4 * IP_ADDR_LEN); j++)
	{
		printf(" ");
	}
	printf("%s ", workstr);
	printf("%10u ", arpptr->timestamp);
	switch (arpptr->state)
	{
	case ARPRESOLVED:
		printf("Resolved\n");
		break;
	case ARPPENDING:
		printf("Pending");
		if (arpptr->waitingpid)
		{ printf(" (Process %d waiting)", arpptr->waitingpid); }
		printf("\n");
		break;
	default:
		printf("?\n");
	}
	return OK;
}

/**
 * Displays the ARP table.
 * @return OK.
 */
local arpDisplay(void)
{
	int i;
	struct arpentry arpcopy[NARP];

	wait(arplock);
	memcpy(arpcopy, arptab, NARP * sizeof(struct arpentry));
	signal(arplock);

	printf("ARP  Hardware Address   Proto Address  Timestamp Status\n");
	printf("--- ----------------- --------------- ---------- ------\n");
	for (i = 0; i < NARP; i++)
	{
		if (arpcopy[i].state != ARPFREE)
		{
			printf("%3d ",i);
			arpLine(&arpcopy[i]);
		}
	}
	printf("\n");
    return OK;
}

/**
 * Shell command (arp) displays the ARP table.
 * @param nargs number of arguments in args array
 * @param args  array of arguments
 * @return non-zero value on error
 */
command xsh_arp(int nargs, char *args[])
{
	uchar ipaddr[IP_ADDR_LEN];
	uchar macaddr[ETH_ADDR_LEN];
	int i;

	/* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strncmp(args[1], "--help", 6) == 0)
    {
        fprintf(stdout, "Displays and manipulates ARP table.\n");
        fprintf(stdout, "\t--help\t display this help and exit\n");
        fprintf(stdout, "\t<IP>\t resolve requested IP address\n");
        fprintf(stdout, "\tWithout arguments, displays current ARP table\n");
        return 0;
    }

	if (nargs < 2)
	{
		arpDisplay();
		return 0;
	}

	if (nargs > 2)
    {
        fprintf(stderr, "arp: too many arguments\n");
        fprintf(stderr, "Try 'kill --help' for more information\n");
        return 1;
    }

	if (OK != dot2ip(args[1], ipaddr))
	{
		fprintf(stderr, "Could not parse IP address \"%s\"\n", args[1]);
		return 1;
	}

	if (OK != arpResolve(ipaddr, macaddr))
	{
		fprintf(stdout, "Could not resolve %s\n", args[1]);
		return 1;
	}

	fprintf(stdout, "IP %s has hardware address ", args[1]);

	for (i = 0; i < ETH_ADDR_LEN; i++)
	{
		fprintf(stdout, "%02X", macaddr[i]);
		if (i < ETH_ADDR_LEN -1) { fprintf(stdout, ":"); }
	}
	fprintf(stdout, ".\n");
	
	return 0;
}

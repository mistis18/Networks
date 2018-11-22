/* Embedded Xinu, Copyright (C) 2010.  All rights reserved. */

/* PING Utility */
/* Author: Tyler Dicks, Misti Stevens 
*  MSCS 5300
*/

#include <xinu.h>
#include "../network/icmp/icmp.h"


/**
* Shell command (ping) sends ICMP ping message to commanded client
* @param nargs number of arguments in args array
* @param args  array of arguments
* @return non-zero value on error
*/
command xsh_ping(int nargs, char *args[])
{
	uchar ipaddr[IP_ADDR_LEN];
	ushort icmp_size = (sizeof(struct ipgram) + sizeof(struct icmp_header_t));
	struct icmp_t* ping = (struct icmp_t*) malloc(icmp_size);
	memcpy(ping, 0, icmp_size); // Clear out icmp packet 

	/* Check arguments */
	if(nargs < 2)
	{
		fprintf(stderr, "Error: No Arguments Provided. \n");
		return 1;
	}
	else if (nargs > 2)
	{
		fprintf(stderr, "Error: Too Many Arguments Provided. \n");
		return 1;
	}

	/* Check Validity of IP Address*/
	if (OK != dot2ip(args[1], ipaddr)) 
	{
		fprintf(stderr, "Could not parse IP address \"%s\"\n", args[1]);
		return 1;
	}

	/* Print Command to Console */
	fprintf(stdout, "PING %s \n", ipaddr);

	/* Call icmpResolve */
	if (OK != icmpResolve(ipaddr, ping))
	{
		fprintf(stdout, "Could not resolve %s\n", args[1]);
		return 1;
	}

	return 0;
}







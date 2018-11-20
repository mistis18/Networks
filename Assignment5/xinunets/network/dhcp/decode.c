/**
 * @file     xsh_dhcpsnoop.c
 * @provides xsh_dhcpsnoop
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>

void printDHCPopts(uchar *ppkt)
{
	int i = 0;
	int len = 0, option = 0;

	while (*ppkt != DHCP_OPTIONS_END)
	{
		option = ppkt[0];
		len = ppkt[1];
		printf("\t  Option %3d, length %2d: ", option, len);
		ppkt += 2;

		switch (option)
		{
		case DHCP_OPTIONS_MESSAGE:
			switch (*ppkt)
			{
			case DHCP_MESSAGE_DISCOVER: printf("DISCOVER"); break;
			case DHCP_MESSAGE_OFFER   : printf("OFFER   "); break;
			case DHCP_MESSAGE_REQUEST : printf("REQUEST "); break;
			case DHCP_MESSAGE_DECLINE : printf("DECLINE "); break;
			case DHCP_MESSAGE_ACK     : printf("ACK     "); break;
			case DHCP_MESSAGE_NACK    : printf("NACK    "); break;
			case DHCP_MESSAGE_RELEASE : printf("RELEASE "); break;
			case DHCP_MESSAGE_INFORM  : printf("INFORM  "); break;
			}
			ppkt += len;
			break;

		case DHCP_OPTIONS_REQUESTED_IP:
			printf("Requested IP ");
			for (i = 0; i < IPv4_ADDR_LEN; i++)		
			{
				printf("%d", *ppkt);
				if ((i % IPv4_ADDR_LEN) < IPv4_ADDR_LEN - 1)
				{   printf(".");   }
				else 
				{   printf(" ");   }
				ppkt++;
			}
			break;

		case DHCP_OPTIONS_SERVER_ID:
			printf("Server ID ");
			for (i = 0; i < IPv4_ADDR_LEN; i++)		
			{
				printf("%d", *ppkt);
				if ((i % IPv4_ADDR_LEN) < IPv4_ADDR_LEN - 1)
				{   printf(".");   }
				else 
				{   printf(" ");   }
				ppkt++;
			}
			break;



		default:
			for (i = 0; i < len; i++)
			{
				printf("%02X ", *ppkt);
				ppkt++;
			}
		}
		printf("\n");
	}
}


void filterDHCPpackets(uchar *packet)
{
	uchar *ppkt = packet;
	struct ethergram *egram;
	struct ipgram    *dgram;
	struct udpgram   *ugram;
	struct dhcpgram  *dhcp;
	int i = 0, len = 0;
	ulong magic = 0;

	egram = (struct ethergram *)packet;
	// Ignore packets that aren't IPv4
	if (ntohs(egram->type) != ETYPE_IPv4) return;
	
	dgram = (struct ipgram *)egram->data;
	// Ignore packets that aren't UDP
	if (dgram->proto != IPv4_PROTO_UDP) return;
	
	ugram = (struct udpgram *)dgram->opts;
	// Ignore packets that aren't BOOTPS or BOOTPC
	if ((ntohs(ugram->dstPort) != 67)
		&& (ntohs(ugram->dstPort) != 68)) return;

	dhcp = (struct dhcpgram  *)ugram->data;

	for (i = 0; i < 2 * ETH_ADDR_LEN; i++)		
	{
		printf("%02X", ppkt[i]);
		if ((i % ETH_ADDR_LEN) < ETH_ADDR_LEN - 1)
		{   printf(":");   }
		else 
		{   printf(" ");   }
	}

	for (i = 0; i < IPv4_ADDR_LEN; i++)		
	{
		printf("%d", dgram->dst[i]);
		if ((i % IPv4_ADDR_LEN) < IPv4_ADDR_LEN - 1)
		{   printf(".");   }
		else 
		{   printf(" ");   }
	}

	for (i = 0; i < IPv4_ADDR_LEN; i++)		
	{
		printf("%d", dgram->src[i]);
		if ((i % IPv4_ADDR_LEN) < IPv4_ADDR_LEN - 1)
		{   printf(".");   }
		else 
		{   printf(" ");   }
	}

	printf("%d %d ", ntohs(ugram->dstPort), ntohs(ugram->srcPort));
	printf("\n");

	if (dhcp->opcode == DHCP_OPCODE_REQUEST)
		printf("\tRequest, ");
	else
		printf("\tReply,   ");

	printf("Client %d.%d.%d.%d, ", dhcp->client[0], dhcp->client[1],
		   dhcp->client[2], dhcp->client[3]);
	printf("YourIP %d.%d.%d.%d, ", dhcp->yourIP[0], dhcp->yourIP[1],
		   dhcp->yourIP[2], dhcp->yourIP[3]);
	printf("Server %d.%d.%d.%d,\n", dhcp->server[0], dhcp->server[1],
		   dhcp->server[2], dhcp->server[3]);
	printf("\tGateway %d.%d.%d.%d, ", dhcp->router[0], dhcp->router[1],
		   dhcp->router[2], dhcp->router[3]);

	printf("Hardware ");
	for (i = 0; i < dhcp->hlen; i++)		
	{
		printf("%02X", dhcp->hwaddr[i]);
		if (i < dhcp->hlen - 1)
		{   printf(":");   }
		else 
		{   printf("\n");   }
	}

	printf("\tServer name \"%s\"\n", dhcp->servname);
	printf("\tBootfile name \"%s\"\n", dhcp->bootfile);

	ppkt = dhcp->opts;
	magic = (ppkt[0] << 24) | (ppkt[1] << 16) | (ppkt[2] << 8) | ppkt[3];
	printf("\tMagic Cookie 0x%08X\n", magic);
	if (magic == DHCP_MAGIC_COOKIE)
	{
		printDHCPopts(ppkt + 4);
	}
	
	printf("\n");
}

/**
 * Monitor DHCP traffic.
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_dhcpsnoop(int nargs, char *args[])
{
	uchar packet[PKTSZ];
	uchar *ppkt = packet;
	int i = 0, len = 0;

	// Zero out the packet buffer.
	bzero(packet, PKTSZ);

	printf("Destination        Source\n");

	while ((len = read(ETH0, packet, PKTSZ)) > 0)
	{
		filterDHCPpackets(packet);
	}

    return OK;
}

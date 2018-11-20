/* arpReceive.c - arpReceive */

#include <xinu.h>
#include "arp.h"

/*------------------------------------------------------------------------
 * arpReceive - Demultiplex incoming ARP packets
 *------------------------------------------------------------------------
 */
void arpReceive(int dev, uchar *packet)
{
	struct ethergram *ether = (struct ethergram *)packet;
	struct arpgram   *arp   = (struct arpgram *)ether->data;
	int i;
	
	switch (ntohs(arp->oper))
	{
	case ARP_REQUEST:
		arpReply(dev, packet);
		break;
	case ARP_REPLY:
		for (i = 0; i < NARP; i++)
		{
			if ((ARPPENDING == arptab[i].state)
				&& (0 == memcmp(arp->spa, arptab[i].ip, IP_ADDR_LEN)))
			{
				memcpy(arptab[i].mac, arp->sha, ETH_ADDR_LEN);
				arptab[i].timestamp = clocktime;
				arptab[i].state = ARPRESOLVED;
				send(arptab[i].waitingpid, OK);
			}
		}
		// fprintf(stderr, "Gratuitous ARP reply.\n");
		buffree(packet);
		break;
	default:
		fprintf(stderr, "Unknown ARP gram!\n");
		buffree(packet);
	}

}

/**
 * @file client.c
 * @provides dhcpclient.
 *
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#include <xinu.h>
#include "dhcp.h"

enum dhcp_state state = DHCP_INIT;

/**
 * DHCP client daemon.
 */
void dhcpclient(int dev, struct netif *nif)
{
	int mesg = 0;
	int missed  = 0;
	uchar *packet = NULL;

	nif->state = NETIF_STATE_BINDING;
	while (1)
	{
		switch (state)
		{
		case DHCP_INIT:
			// send DHCPDISCOVER
			sendDiscover(dev);
			state = DHCP_SELECTING;
			break;

		case DHCP_SELECTING:
			// Read an offer
			mesg = recvtime(SELECT_WAIT * (missed + 1));
			if (TIMEOUT == mesg)
			{
				missed++;
				sendDiscover(dev);
				break;
			}
			// Select an offer
			// send DHCPREQUEST
			packet = (uchar *)mesg;
			if(recvOffer(dev, nif, packet))
			{
				state = DHCP_REQUESTING;
			}
			buffree(packet);
			break;

		case DHCP_REQUESTING:
			// Read a NACK or an ACK
			if (1)
			{
				state = DHCP_BOUND;
				// set a timer
				sleep(10000);
			}
			// NACK
			state = DHCP_INIT;
			continue;
			break;

		case DHCP_BOUND:
			// Timer expired
			// send DHCPREQUEST
			state = DHCP_RENEWING;
			// sleep for a while
			break;

		case DHCP_RENEWING:
			// Send DHCPREQUEST
			// If ACK
			if (1)
			{
				state = DHCP_BOUND;
				// set a timer
				continue;
			}
			// If NACK
			if (0)
			{
				state = DHCP_INIT;
				continue;
			}
			// If 75% expired
			state = DHCP_REBINDING;
			break;

		case DHCP_REBINDING:
			// If ACK
			if (1)
			{
				state = DHCP_BOUND;
				// set a timer
				continue;
			}
			// If expired
			state = DHCP_INIT;
			break;

		default:
			break;
		}
	}

	return;
}

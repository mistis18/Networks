/**
 * @file dhcp.h
 */
/* Embedded Xinu, Copyright (C) 2008.  All rights reserved. */

#define BOOTPS 67
#define BOOTPC 68

#define SELECT_WAIT 1000

enum dhcp_state {  DHCP_INIT = 0,
				   DHCP_SELECTING,
				   DHCP_REQUESTING,
				   DHCP_BOUND,
				   DHCP_RENEWING,
				   DHCP_REBINDING };

enum dhcp_option { DHCPDISCOVER = 1,
				   DHCPOFFER,
				   DHCPREQUEST,
				   DHCPDECLINE,
				   DHCPACK,
				   DHCPNACK,
				   DHCPRELEASE,
				   DHCPINFORM };

extern enum dhcp_state state;

bool recvOffer(int dev, struct netif *nif, uchar *packet);
void sendDiscover(int dev);
void dhcpclient(int dev, struct netif *nif);
void printDHCPopts(uchar *ppkt);

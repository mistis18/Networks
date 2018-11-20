/**
 * @file arp.h
 *
 */
/* Embedded Xinu, Copyright (C) 2011.  All rights reserved. */

#define NARP 10

/* ARP Packet Definitions */
#define ARP_REQUEST 0x0001
#define ARP_REPLY   0x0002

/* ARP Header Size */
#define ARP_SIZE (8 + ETH_ADDR_LEN * 2 + ENET_IPv4 * 2)

/* ARP Packet Structure */
struct arpgram
{
    ushort htype;
    ushort ptype;
    uchar hlen;
    uchar plen;
    ushort oper;
    uchar sha[ETH_ADDR_LEN];
    uchar spa[IP_ADDR_LEN];
    uchar tha[ETH_ADDR_LEN];
    uchar tpa[IP_ADDR_LEN];
};

struct arpentry
{
	uchar state;
	ulong timestamp;
	uchar mac[ETH_ADDR_LEN];
	uchar ip[IP_ADDR_LEN];
	int   waitingpid;
};

extern struct arpentry arptab[];
extern int arplock;

#define ARPFREE     0
#define ARPRESOLVED 1
#define ARPPENDING  2

void arpInit(void);
int arpResolve(uchar *ipaddr, uchar *mac);
int arpReply(int dev, uchar *buf);
void arpReceive(int dev, uchar *buf);

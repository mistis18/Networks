/**
 * @file icmpResolve.c
 * @provides imcpResolve
 */

#include <xinu.h>
#include "icmp.h"
#include "../arp/arp.h"

int waitingPID;
ulong startTime;

 /**
  * Get the PID from the echoRequest.
  *
  */
const int getWaitingPID(void) 
{
	return waitingPID;
}

void printResponse(const struct ipgram* dgram)
{
	ulong elapsed = clocktime - startTime;
	
	fprintf(stdout, "Recieved reply from %d.%d.%d.%d: Bytes:%d Time:%d seconds TTL:%d\n", 
		dgram->src[0], dgram->src[1], dgram->src[2], dgram->src[3], 
		dgram->len, elapsed, dgram->ttl);
}


 /**
  * Generate an echo request.
  *
  * @param *ipaddr pointer to the IP address
  */
process echoRequest(int dev, uchar* ipaddr)
{
	uchar packet[PKTSZ];

	struct ethergram *ether = (struct ethergram*) packet;
	struct ipgram *dgram = (struct ipgram*) ether->data;
	struct icmp_header_t *icmp_header = (struct icmp_header_t*) dgram->opts;

	int i;

	// Send ARP Request
	uchar macaddr[ETH_ADDR_LEN];
	if (OK != arpResolve(ipaddr, macaddr))
	{
		return 1;
	}

	// Construct the echoRequest
	bzero(packet, PKTSZ);

	// Construct Ethernet Header
	bzero(ether->src, ETH_ADDR_LEN);
	getmac(dev, ether->src);
	bzero(ether->dst, ETH_ADDR_LEN);
	memcpy(ether->dst, macaddr, ETH_ADDR_LEN);

	ether->type = htons(ETYPE_IPv4);

	// Construct IP Header
	dgram->ver_ihl = (IPv4_VERSION << 4) | (IPv4_HDR_LEN >> 2);
	dgram->tos = IPv4_TOS_ROUTINE; /*Type 0*/
	dgram->len = 0; /* Set Checksum and Length later */

	i = htonl(currpid);
	memcpy(&(dgram->id), &i, sizeof(dgram->id));

	dgram->flags_froff = 0;
	dgram->ttl = 63;
	dgram->proto = IPv4_PROTO_ICMP; /*Protocol 1*/
	dgram->chksum = 0; /* Set Checksum and Length later */

	bzero(dgram->src, IP_ADDR_LEN);
	getip(dev, dgram->src);

	bzero(dgram->dst, IP_ADDR_LEN);
	memcpy(dgram->dst, ipaddr, IP_ADDR_LEN);
		
	// Contrust ICMP Header
	icmp_header->type = ECHO_REQUEST;
	icmp_header->code = 0; /* see https://tools.ietf.org/html/rfc792 */
	i = htonl(currpid);
	memcpy(&(icmp_header->identifier), &i, sizeof(icmp_header->identifier));

	icmp_header->sequence_number = 0;
	icmp_header->checksum = checksum((uchar*) icmp_header, sizeof(struct icmp_header_t));

	// Set IP Header Checksum and Length
	dgram->len = (sizeof(struct ipgram) + sizeof(struct icmp_header_t)); 
	dgram->chksum = checksum((uchar*) dgram, dgram->len);

	// Send the echoRequest (ping)
	write(dev, (uchar *)packet,
		sizeof(struct ethergram) + sizeof(struct ipgram) + sizeof(struct icmp_header_t));

	sleep(1000);

	return OK;
}

 /**
  * Generate an echo response.
  *
  * @param *egram the incoming ethergram that contains the PING request
  */
process echoReply(int dev, struct ethergram* egram)
{
	//Outgoing Packet
	uchar packet[PKTSZ];
	struct ethergram *ether = (struct ethergram*) packet;
	struct ipgram *dgram = (struct ipgram*) ether->data;
	struct icmp_header_t *icmp_header = (struct icmp_header_t*) dgram->opts;

	//Incoming ICMP Info Pointers
	struct  ipgram *dgram_in = (struct ipgram*) egram->data;
	struct icmp_header_t *icmp_header_in = (struct icmp_header_t*) dgram_in->opts;

	int i;

	// Construct the echoRequest
	bzero(packet, PKTSZ);

	// Construct Ethernet Header
	//Add the current device MAC address
	getmac(dev, ether->src);
	//use the MAC Address from the incoming ethergram as DST
	memcpy(ether->dst, egram->src, ETH_ADDR_LEN); 
	ether->type = htons(ETYPE_IPv4);

	// Construct IP Header
	dgram->ver_ihl = (IPv4_VERSION << 4) | (IPv4_HDR_LEN >> 2);
	dgram->tos = IPv4_TOS_ROUTINE; /*Type 0*/
	dgram->len = 0; /* Set Checksum and Length later */
	dgram->id = htons(currpid);
	dgram->flags_froff = 0;
	dgram->ttl = 63;
	dgram->proto = IPv4_PROTO_ICMP; /*Protocol 1*/
	dgram->chksum = 0; /* Set Checksum and Length later */
	getip(dev, dgram->src);
	//Copy the incoming dgram src as the outgoing dst
	memcpy(dgram->dst, dgram_in->src, IP_ADDR_LEN);
		
	// Contrust ICMP Header
	icmp_header->type = ECHO_REPLY;
	icmp_header->code = 0; /* see https://tools.ietf.org/html/rfc792 */
	icmp_header->identifier = htons(currpid);
	icmp_header->sequence_number = 0;
	icmp_header->checksum = checksum((uchar *)icmp_header,
		(4 * (dgram->ver_ihl & IPv4_IHL)));

	// Set IP Header Checksum and Length
	dgram->len = (sizeof(struct ipgram) + sizeof(struct icmp_header_t)); 
	dgram->chksum = checksum((uchar *)dgram,
		(4 * (dgram->ver_ihl & IPv4_IHL)));

	write(dev, (uchar *)packet,
		sizeof(struct ethergram) + sizeof(struct ipgram) + sizeof(struct icmp_header_t));

	return OK;
}

int icmpReply(struct ethergram* egram)
{
	ready(create
	((void *)echoReply, INITSTK,
		proctab[currpid].priority + 1,
		"ECHO responder", 2,
		 ETH0, egram), RESCHED_NO);

	return OK;
}


int icmpResolve(uchar* ipaddr)
{
	waitingPID = currpid;
	startTime = clocktime;
		
	struct ipgram ip;
	struct icmp_header_t icmp;
	message m;

	// Spawn a process to send ping request.
	ready(create
	((void *)echoRequest, INITSTK,
		proctab[currpid].priority + 1,
		"ECHO requester", 3,
		 ETH0, ipaddr, &ip, &icmp), RESCHED_NO);

	m = receive();
	if (TIMEOUT == m)
	{
		return SYSERR;
	}
	
	return OK;
}

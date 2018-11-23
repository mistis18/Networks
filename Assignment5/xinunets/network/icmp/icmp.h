/**
 * @file icmp.h
 *
 */

/* ICMP Types - 8 bits*/
#define ECHO_REPLY   0x00
#define ECHO_REQUEST 0x08
 /* ICMP Errors - 8 bits */
#define DESTINATION_UNREACHABLE   0x03
#define REDIRECT 0x05
#define TIME_EXCEEDED 0x0A
#define PARAMETER_PROBLEM 0x0B

/*
 * ICMP HEADER
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |		Type	   |		Code	    |		Checksum	   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         Identifier		       |		  Sequence Number      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct icmp_header_t
{
	uchar type; /*8 bits*/ 
	uchar code; /*8 bits*/
	ushort checksum; /*16 bits*/
	ushort identifier; /*16 bits*/
	ushort sequence_number; /*16 bits*/
};

/* ICMP Packet Structure				  */
/* This will be sent in and IP datagram   */
/* with a protocol number of 1 and a type */
/* service of 0						      */

/*
 * ICMP Packet
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |					IP V4 Header (20 bytes)                    |
 * | ...                                                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    ICMP Header (8 byets)			           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    ICMP Payload (optional, not used)	       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/*
struct icmp_t
{
	struct ipgram* ip_header;
	struct icmp_header_t* icmp_header;
};
*/ 

int icmpResolve(uchar* ipaddr);
process echoReply(int dev, struct ethergram* egram);


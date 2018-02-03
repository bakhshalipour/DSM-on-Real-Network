#ifndef SR_PROTOCOL_H
#define SR_PROTOCOL_H

#ifdef _LINUX_
#include <stdint.h>
#endif /* _LINUX_ */

#include <sys/types.h>
#include <arpa/inet.h>

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1
#endif
#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 2
#endif
#ifdef _CYGWIN_
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#endif
#ifdef _LINUX_
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#endif
#ifdef _SOLARIS_
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __BIG_ENDIAN
#endif
#endif

/*
 * Fix for Mac OS X
 */
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

/*
 * Structure of an Internet Protocol header, naked of options.
 */
struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int ip_hl:4;		/* header length */
	unsigned int ip_v:4;		/* version */
#elif __BYTE_ORDER == __BIG_ENDIAN
	unsigned int ip_v:4;		/* version */
	unsigned int ip_hl:4;		/* header length */
#else
#error "Byte ordering not specified "
#endif
	uint8_t ip_tos;			/* type of service */
	uint16_t ip_len;			/* total length */
	uint16_t ip_id;			/* identification */
	uint16_t ip_off;			/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	uint8_t ip_ttl;			/* time to live */
	uint8_t ip_p;			/* protocol */
	uint16_t ip_sum;			/* checksum */
	struct in_addr ip_src, ip_dst;	/* source and dest address */
} __attribute__ ((packed)) ;

#ifndef IPPROTO_TCP
#define IPPROTO_TCP            0x0006  /* TCP protocol */
#endif

#ifndef IPPROTO_UDP
#define IPPROTO_UDP            0x0011  /* UDP protocol */
#endif

/*
 *  Ethernet packet header prototype.  Too many O/S's define this differently.
 *  Easy enough to solve that and define it here.
 */
struct sr_ethernet_hdr
{
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif
	uint8_t  ether_dhost[ETHER_ADDR_LEN];    /* destination ethernet address */
	uint8_t  ether_shost[ETHER_ADDR_LEN];    /* source ethernet address */
	uint16_t ether_type;                     /* packet type ID */
} __attribute__ ((packed)) ;

#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP            0x0800  /* IP protocol */
#endif

#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP           0x0806  /* ARP protocol */
#endif


struct sr_tcp
{
	uint16_t port_src; /* source port */
	uint16_t port_dst; /* destination port */
	uint32_t tcp_seq; /* sequence number */
	uint32_t tcp_ack; /* acknowledgement number */
#define TCP_FIN 0x1 /* FIN flag */
#define TCP_SYN 0x2 /* SYN flag */
#define TCP_RST 0x4 /* RST flag */
#define TCP_PSH 0x8 /* PSH flag */
#define TCP_ACK 0x10 /* ACK flag */
#define TCP_URG 0x20 /* URG flag */
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int tcp_ecn_1:1; /* ecn, the first bit */
	unsigned int tcp_reserved:3; /* reserved */
	unsigned int tcp_doff:4; /* data offset */
	unsigned int tcp_flags:6; /* flags */
	unsigned int tcp_ecn_2:2; /* ecn, the last 2 bits */
#elif __BYTE_ORDER == __BIG_ENDIAN
	unsigned int tcp_doff:4; /* data offset */
	unsigned int tcp_reserved:3; /* reserved */
	unsigned int tcp_ecn_1:1; /* ecn, the first bit */
	unsigned int tcp_ecn_2:2; /* ecn, the last 2 bits */
	unsigned int tcp_flags:6; /* flags */
#else
#error "Byte ordering not specified "
#endif
	uint16_t tcp_window; /* window size */
	uint16_t tcp_sum; /* checksum */
	uint16_t tcp_ptr; /* urgent pointer */
} __attribute__ ((packed)) ;
/* for using: struct sr_tcp * tcp=(struct sr_tcp *)((void *)ip_hdr+ip_hdr->ip_hl*4); */

struct sr_udp
{
	uint16_t port_src; /* source port */
	uint16_t port_dst; /* destination port */
	uint16_t length; /* length of udp header and data in bytes */
	uint16_t udp_sum; /* checksum */
} __attribute__ ((packed)) ;


struct sr_arp {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t oper;
	uint8_t sha[ETHER_ADDR_LEN];
	uint32_t spa;
	uint8_t tha[ETHER_ADDR_LEN];
	uint32_t tpa;
} __attribute__ ((packed)) ;

#ifndef ARP_REQUEST 
#define ARP_REQUEST         1  /* ARP Request */
#endif

#ifndef ARP_REPLY
#define ARP_REPLY           2  /* ARP Reply */
#endif

struct sr_flit
{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	unsigned int mode:6;
	unsigned int data_valid:1;
	unsigned int message_type:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
	unsigned int message_type:1; 		//request = 0, response = 1
	unsigned int data_valid:1;			//invalid = 0, valid = 1
	unsigned int mode:6;				//In this PA, assume 0
#else
#error "Byte ordering not specified "
#endif
	uint16_t address;
	uint64_t data;
} __attribute__ ((packed)) ;

#endif /* -- SR_PROTOCOL_H -- */


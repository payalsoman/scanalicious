/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 *
 * IP, Arp and UDP functions.
 *
 * Title: Microchip ENC28J60 Ethernet Interface Driver
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/
#include <avr/io.h>
#include "net.h"
#include "enc28j60.h"

static uint8_t macaddr[6];
static uint8_t ipaddr[4];
static uint8_t pcip[4];// = {10,0,0,2};
static uint8_t pcmac[6];// = {0x54,0x55,0x58,0x10,0x00,0x24};

// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we 
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// You must set the upd checksum field to zero before you start
// the calculation.
// len for udp is: 8 + 8 + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
uint16_t checksum(uint8_t *buf, uint16_t len,uint8_t type){
        // type 0=ip 
        //      1=udp
        //      2=tcp
        uint32_t sum = 0;

        if(type==1){
                sum+=IP_PROTO_UDP_V; // protocol udp
                // the length here is the length of udp (data+header len)
                // =length - IP addr length
                sum+=len-8; // = real udp len
        }
        if(type==2){
                sum+=IP_PROTO_TCP_V; 
                // the length here is the length of tcp (data+header len)
                // =length - IP addr length
                sum+=len-8; // = real tcp len
        }
        // build the sum of 16bit words
        while(len >1){
                sum += 0xFFFF & (*buf<<8|*(buf+1));
                buf+=2;
                len-=2;
        }
        // if there is a byte left then add it (padded with zero)
        if (len){
                sum += (0xFF & *buf)<<8;
        }
        // now calculate the sum over the bytes in the sum
        // until the result is only 16bit long
        while (sum>>16){
                sum = (sum & 0xFFFF)+(sum >> 16);
        }
        // build 1's complement:
        return( (uint16_t) sum ^ 0xFFFF);
}

// you must call this function once before you use any of the other functions:
void init_ip_arp_udp(uint8_t *mymac,uint8_t *myip, uint8_t *mypcip, uint8_t *mypcmac){
        uint8_t i=0;
        while(i<4){
                ipaddr[i]=myip[i];
				pcip[i] = mypcip[i];
                i++;
        }
        i=0;
        while(i<6){
                macaddr[i]=mymac[i];
				pcmac[i] = mypcmac[i];
                i++;
        }
}

uint8_t eth_type_is_arp_and_my_ip(uint8_t *buf,uint8_t len){
        uint8_t i=0;
        //
        if (len<41){
                return(0);
        }
        if(buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V || 
           buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V){
                return(0);
        }
        while(i<4){
                if(buf[ETH_ARP_DST_IP_P+i] != ipaddr[i]){
                        return(0);
                }
                i++;
        }
        return(1);
}

uint8_t eth_type_is_ip_and_my_ip(uint8_t *buf,uint8_t len){
        uint8_t i=0;
        //eth+ip+udp header is 42
        if (len<42){
                return(0);
        }
        if(buf[ETH_TYPE_H_P]!=ETHTYPE_IP_H_V || 
           buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V){
                return(0);
        }
        while(i<4){
                if(buf[IP_DST_P+i]!=ipaddr[i]){
                        return(0);
                }
                i++;
        }
        return(1);
}
// make a return eth header from a received eth packet
void make_eth(uint8_t *buf)
{
        uint8_t i=0;
        //
        //copy the destination mac from the source and fill my mac into src
        while(i<6){
                buf[ETH_DST_MAC +i]=buf[ETH_SRC_MAC +i];
                buf[ETH_SRC_MAC +i]=macaddr[i];
                i++;
        }
}

// make a return eth header from known pc addresses
void make_send_eth(uint8_t *buf)
{
        uint8_t i=0;
        //
        //copy the destination mac from the source and fill my mac into src
        while(i<6){
                buf[ETH_DST_MAC +i]=pcmac[i];
                buf[ETH_SRC_MAC +i]=macaddr[i];
                i++;
        }
}


// make a return ip header from a received ip packet
void make_ip(uint8_t *buf)
{
        uint8_t i=0;
        uint16_t ck;
        while(i<4){
                buf[IP_DST_P+i]=buf[IP_SRC_P+i];
                buf[IP_SRC_P+i]=ipaddr[i];
                i++;
        }
        // clear the 2 byte checksum
        buf[IP_CHECKSUM_P]=0;
        buf[IP_CHECKSUM_P+1]=0;
        buf[IP_FLAGS_P]=0x40; // don't fragment
        buf[IP_FLAGS_P+1]=0;  // fragement offset
        buf[IP_TTL_P]=64; // ttl
        // calculate the checksum:
        ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
        buf[IP_CHECKSUM_P]=ck>>8;
        buf[IP_CHECKSUM_P+1]=ck& 0xff;
}

//sets up the ip to send to our workstation
void make_send_ip(uint8_t *buf)
{
        uint8_t i=0;
        uint16_t ck;
        while(i<4){
				buf[IP_DST_P+i]=pcip[i];
                buf[IP_SRC_P+i]=ipaddr[i];
                i++;
        }
        // clear the 2 byte checksum
          buf[IP_CHECKSUM_P]=0;
          buf[IP_CHECKSUM_P+1]=0;
          buf[IP_TTL_P]=64; // ttl
       
	      //calculate the checksum:
          ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
          buf[IP_CHECKSUM_P]=ck>>8;
          buf[IP_CHECKSUM_P+1]=ck& 0xff;
}


void make_arp_answer_from_request(uint8_t *buf,uint8_t len)
{
        uint8_t i=0;
        //
        make_eth(buf);
        buf[ETH_ARP_OPCODE_H_P]=ETH_ARP_OPCODE_REPLY_H_V;
        buf[ETH_ARP_OPCODE_L_P]=ETH_ARP_OPCODE_REPLY_L_V;
        // fill the mac addresses:
        while(i<6){
                buf[ETH_ARP_DST_MAC_P+i]=buf[ETH_ARP_SRC_MAC_P+i];
                buf[ETH_ARP_SRC_MAC_P+i]=macaddr[i];
                i++;
        }
        i=0;
        while(i<4){
                buf[ETH_ARP_DST_IP_P+i]=buf[ETH_ARP_SRC_IP_P+i];
                buf[ETH_ARP_SRC_IP_P+i]=ipaddr[i];
                i++;
        }
        // eth+arp is 42 bytes:
        enc28j60PacketSend(42,buf); 
}

void make_echo_reply_from_request(uint8_t *buf,uint8_t len)
{
        make_eth(buf);
        make_ip(buf);
        buf[ICMP_TYPE_P]=ICMP_TYPE_ECHOREPLY_V;
        // we changed only the icmp.type field from request(=8) to reply(=0).
        // we can therefore easily correct the checksum:
        if (buf[ICMP_CHECKSUM_P] > (0xff-0x08)){
                buf[ICMP_CHECKSUM_P+1]++;
        }
        buf[ICMP_CHECKSUM_P]+=0x08;
        //
        enc28j60PacketSend(len,buf);
}

// you can send a max of 220 bytes of data
void make_udp_reply_from_request(uint8_t *buf,uint8_t *data,uint8_t datalen,uint16_t port)
{
        uint8_t i=0;
        uint16_t ck;
        make_eth(buf);
        if (datalen>220){
                datalen=220;
        }
        // total length field in the IP header must be set:
        buf[IP_TOTLEN_H_P]=0;
        buf[IP_TOTLEN_L_P]=IP_HEADER_LEN+UDP_HEADER_LEN+datalen;
        make_ip(buf);
        // send to port:
        //buf[UDP_DST_PORT_H_P]=port>>8;
        //buf[UDP_DST_PORT_L_P]=port & 0xff;
        // sent to port of sender and use "port" as own source:
        buf[UDP_DST_PORT_H_P]=buf[UDP_SRC_PORT_H_P];
        buf[UDP_DST_PORT_L_P]= buf[UDP_SRC_PORT_L_P];
        buf[UDP_SRC_PORT_H_P]=port>>8;
        buf[UDP_SRC_PORT_L_P]=port & 0xff;
        // source port does not matter and is what the sender used.
        // calculte the udp length:
        buf[UDP_LEN_H_P]=0;
        buf[UDP_LEN_L_P]=UDP_HEADER_LEN+datalen;
        // zero the checksum
        buf[UDP_CHECKSUM_H_P]=0;
        buf[UDP_CHECKSUM_L_P]=0;
        // copy the data:
        while(i<datalen){
                buf[UDP_DATA_P+i]=data[i];
                i++;
        }
        ck=checksum(&buf[IP_SRC_P], 16 + datalen,1);
        buf[UDP_CHECKSUM_H_P]=ck>>8;
        buf[UDP_CHECKSUM_L_P]=ck& 0xff;
        enc28j60PacketSend(UDP_HEADER_LEN+IP_HEADER_LEN+ETH_HEADER_LEN+datalen,buf);
}

//can send a maximum of 240 bytes
void send_udp_packet(uint8_t *buf,uint8_t *data, uint16_t datalen, uint16_t lineNum, uint8_t seq)
{
		uint16_t port = 1200;
		uint16_t unique = 17098;
		uint16_t ck;
		uint16_t i= 0;
		
		datalen += 3;
	
	/**first construct the eth header**/

	//load in mac addresses
		make_send_eth(buf);
		//specify the etherent type
		buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;
		buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;

	/**second build the IP Header**/
		
		buf[IP_P] = 0x45;
		buf[IP_P+1] = 0;
		buf[IP_TOTLEN_H_P] = (IP_HEADER_LEN+UDP_HEADER_LEN+datalen)>>8;
		buf[IP_TOTLEN_L_P] = (IP_HEADER_LEN+UDP_HEADER_LEN+datalen) & 0xff;
		buf[18] = unique>>8;	//unique identification 18,19
		buf[19] = unique&0xff;
		buf[IP_FLAGS_P] = 0;
		buf[IP_FLAGS_P + 1] = 0;
		buf[IP_TTL_P] = 0x80;	//ttl = 128
		buf[IP_PROTO_P] = 0x11;	//protocol = UDP
		buf[IP_CHECKSUM_P] = 0;
		buf[IP_CHECKSUM_P+1] = 0;

		make_send_ip( buf ); //load IP and make checksum
		
	/**third build the UDP Header**/

		buf[UDP_SRC_PORT_H_P] = port >> 8;
		buf[UDP_SRC_PORT_L_P] = port & 0xff;

		buf[UDP_DST_PORT_H_P] = port >> 8;
		buf[UDP_DST_PORT_L_P] = port & 0xff;

		buf[UDP_LEN_H_P] = (UDP_HEADER_LEN+datalen) >> 8;
		buf[UDP_LEN_L_P]= (UDP_HEADER_LEN+datalen) & 0xff;

		buf[UDP_CHECKSUM_H_P]=0;	//clear UDP checksum
        buf[UDP_CHECKSUM_L_P]=0;
		
		//load in the line number and sequence
		buf[UDP_DATA_P+0] = lineNum >> 8;
		buf[UDP_DATA_P+1] = lineNum & 0xff;
		buf[UDP_DATA_P+2] = seq;

		i = 3;

		//copy in the data
		while(i<datalen){
                //buf[UDP_DATA_P+i]=data[i-3];
		     	buf[UDP_DATA_P+i]=data[i];
                i++;
        }

		ck=checksum(&buf[IP_SRC_P], 16 + datalen,1);
        buf[UDP_CHECKSUM_H_P]=ck>>8;
        buf[UDP_CHECKSUM_L_P]=ck & 0xff;
        
		//send packet
		enc28j60PacketSend(UDP_HEADER_LEN+IP_HEADER_LEN+ETH_HEADER_LEN+datalen,buf);
}

/* end of ip_arp_udp.c */

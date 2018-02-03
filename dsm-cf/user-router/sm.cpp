#include "sm.h"

#include "sr_protocol.h"
#include "interface.h"
#include "frame.h"

#include <netinet/in.h>
#include <stdlib.h>

#define EAST_INTERFACE_INDEX	0
#define NORTH_INTERFACE_INDEX	1
#define WEST_INTERFACE_INDEX	2
#define SOUTH_INTERFACE_INDEX	3
#define EJECTION_INTERFACE_INDEX	4
#define SRC_PORT	4000
#define DST_PORT	5000

using namespace std;

unsigned int my_core_id, my_core_x, my_core_y;

string interpret_direction (unsigned int direct)
{
	switch (direct)
	{
		case 0:
			return "east";
		case 1:
			return "north";
		case 2:
			return "west";
		case 3:
			return "south";
		case 4:
			return "processor";
		default:
			return "error";
	}
}

SimulatedMachine::SimulatedMachine (const ClientFramework *cf, int count) :
	Machine (cf, count) 
{

}

SimulatedMachine::~SimulatedMachine () 
{

}

void SimulatedMachine::initialize () 
{
	my_core_id = ((iface[EJECTION_INTERFACE_INDEX].getIp() >> 4) & 3) * 4 + ((iface[EJECTION_INTERFACE_INDEX].getIp() >> 6) & 3);
	my_core_x = my_core_id % 4;
	my_core_y = my_core_id / 4;
}

void SimulatedMachine::processFrame (Frame frame, int ifaceIndex) 
{

	if (frame.length < sizeof(struct sr_ethernet_hdr) + sizeof(struct ip))	//NoC doesn't interfere in transport and above layer(s).
		return; //No message

	struct sr_ethernet_hdr *eth = (sr_ethernet_hdr *)frame.data;
	bool broadcast = true;

	for (int i = 0; i < ETHER_ADDR_LEN; i++)
		if (eth->ether_dhost[i] != 0xFF)
			broadcast = false;

	if (!broadcast)
		for (int i = 0; i < ETHER_ADDR_LEN; i++)
			if (eth->ether_dhost[i] != iface[ifaceIndex].mac[i])
			{
				cout << "A packet dropped at link layer" << endl;
				return;
			}

	if (htons(eth->ether_type) != ETHERTYPE_IP)
	{
		cout << "A packet dropped at link layer" << endl;
		return;
	}

	struct ip *ip_packet = (struct ip *)(frame.data + sizeof(sr_ethernet_hdr));

	if (ip_packet->ip_hl < 5)
	{
		cout << "A packet dropped at network layer" << endl;
		return;
	}
	int ip_header_length_byte = ip_packet->ip_hl * 4;

	if (ip_packet->ip_v != 4)
	{
		cout << "A packet dropped at network layer" << endl;
		return;
	}

	if (ntohs(ip_packet->ip_len) > frame.length - sizeof(struct sr_ethernet_hdr))
	{
		cout << "A packet dropped at network layer" << endl;
		return;
	}

	if (ip_packet->ip_ttl == 1)
	{
		cout << "A packet dropped at network layer" << endl;
		return;
	}

	if (ip_packet->ip_p != 17)
	{
		cout << "A packet dropped at network layer" << endl;
		return;
	}

	unsigned int my_checksum = 0;
	unsigned short *my_ip_packet = (unsigned short *)(frame.data + sizeof(sr_ethernet_hdr));
	while (ip_header_length_byte)
	{
		my_checksum += ntohs (*my_ip_packet);
		my_ip_packet++;
		ip_header_length_byte -= 2;
	}
	while (my_checksum >> 16)
		my_checksum = (my_checksum & 0xFFFF) + (my_checksum >> 16);
	my_checksum = ~my_checksum;
	if ((my_checksum & 0xFFFF) != 0)
	{
		cout << "CHECKSUM ERROR -- I am router " << my_core_id << endl;
		cout << "A packet dropped at network layer" << endl;
		return;
	}

	ip_header_length_byte = ip_packet->ip_hl * 4;

	unsigned int packet_x = (ntohl(ip_packet->ip_dst.s_addr) >> 6) & 3;
	unsigned int packet_y = (ntohl(ip_packet->ip_dst.s_addr) >> 4) & 3;
	
	unsigned int destination_interface_index;
	unsigned int distance;

	if (packet_x != my_core_x)
	{
		if (my_core_x > packet_x)
		{
			distance = my_core_x - packet_x;
			if (distance < 3)
				destination_interface_index = WEST_INTERFACE_INDEX;
			else
				destination_interface_index = EAST_INTERFACE_INDEX;
		}
		else
		{
			distance = packet_x - my_core_x;
			if (distance < 3)
				destination_interface_index = EAST_INTERFACE_INDEX;
			else
				destination_interface_index = WEST_INTERFACE_INDEX;
		}

	}
	else if (packet_y != my_core_y)
	{
		if (my_core_y > packet_y)
		{
			distance = my_core_y - packet_y;
			if (distance < 3)
				destination_interface_index = SOUTH_INTERFACE_INDEX;
			else
				destination_interface_index = NORTH_INTERFACE_INDEX;
		}
		else
		{
			distance = packet_y - my_core_y;
			if (distance < 3)
				destination_interface_index = NORTH_INTERFACE_INDEX;
			else
				destination_interface_index = SOUTH_INTERFACE_INDEX;
		}
	}
	else
		destination_interface_index = EJECTION_INTERFACE_INDEX;

	memcpy(eth->ether_shost, iface[destination_interface_index].mac, ETHER_ADDR_LEN);
	memset(eth->ether_dhost, 0xFF, ETHER_ADDR_LEN);
	ip_packet->ip_ttl = ip_packet->ip_ttl - 1;

	ip_packet->ip_sum = 0;
	unsigned int new_checksum = 0;
	int new_ip_header_len_in_byte = ip_packet->ip_hl * 4;
	unsigned short *new_ip_packet = (unsigned short *)(frame.data + sizeof(struct sr_ethernet_hdr));
	while (new_ip_header_len_in_byte)
	{
		new_checksum += ntohs(*new_ip_packet);
		new_ip_packet++;
		new_ip_header_len_in_byte -= 2;
	}
	while(new_checksum >> 16)
		new_checksum = (new_checksum & 0xFFFF) + (new_checksum >> 16);
	ip_packet->ip_sum = htons(~new_checksum);

	sendFrame(frame, destination_interface_index);

	cout << "Packet was entered from the " << interpret_direction(ifaceIndex) << " and directed to the " << interpret_direction(destination_interface_index) << endl;
}

void SimulatedMachine::run () 
{
	
}

void SimulatedMachine::parseArguments (int argc, char *argv[]) 
{

}


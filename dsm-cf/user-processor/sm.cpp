#include "sm.h"

#include "sr_protocol.h"
#include "interface.h"
#include "frame.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <map>

#define EJECTION_INTERFACE_INDEX	0
#define REQ_SRC_PORT	4000
#define REQ_DST_PORT	5000
#define RESP_SRC_PORT	REQ_DST_PORT
#define RESP_DST_PORT	REQ_SRC_PORT

using namespace std;

/*For get rid of rand()*/
unsigned int my_core_id1;
unsigned int my_core_id2;
unsigned int number_of_sent1;
unsigned int number_of_sent2;

/*Global variables*/
map<uint16_t, uint64_t> cache_memory;
pthread_mutex_t print_mutex;
pthread_mutex_t cache_memory_access;

SimulatedMachine::SimulatedMachine (const ClientFramework *cf, int count) :
	Machine (cf, count) 
{

}

SimulatedMachine::~SimulatedMachine () 
{

}

void SimulatedMachine::initialize () 
{
	my_core_id1 = ((iface[EJECTION_INTERFACE_INDEX].getIp() >> 4) & 3) * 4 + ((iface[EJECTION_INTERFACE_INDEX].getIp() >> 6) & 3);
	my_core_id2 = my_core_id1;
	number_of_sent1 = 0;
	number_of_sent2 = 0;
	pthread_mutex_init(&print_mutex, 0);
	pthread_mutex_init(&cache_memory_access, 0);
}

void SimulatedMachine::processFrame (Frame frame, int ifaceIndex) 
{
	
	if (frame.length < sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct sr_udp) + sizeof(struct sr_flit))
		return;	//No message

	struct sr_ethernet_hdr *eth = (sr_ethernet_hdr *)frame.data;
	bool broadcast = true;

	for (int i = 0; i < ETHER_ADDR_LEN; i++)
		if (eth->ether_dhost[i] != 0xFF)
			broadcast = false;

	if (!broadcast)
		for (int i = 0; i < ETHER_ADDR_LEN; i++)
			if (eth->ether_dhost[i] != iface[EJECTION_INTERFACE_INDEX].mac[i])
			{
				pthread_mutex_lock(&print_mutex);
				cout << "A packet dropped at link layer" << endl;
				pthread_mutex_unlock(&print_mutex);
				return;
			}

	if (htons(eth->ether_type) != ETHERTYPE_IP)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at link layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	struct ip *ip_packet = (struct ip *)(frame.data + sizeof(sr_ethernet_hdr));

	if (ip_packet->ip_hl < 5)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at network layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}
	int ip_header_length_byte = ip_packet->ip_hl * 4;

	if (ip_packet->ip_v != 4)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at network layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	if (ntohs(ip_packet->ip_len) > frame.length - sizeof(struct sr_ethernet_hdr))
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at network layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	if (ip_packet->ip_p != 17)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at network layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	if (ntohl(ip_packet->ip_dst.s_addr) != iface[EJECTION_INTERFACE_INDEX].getIp())
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at network layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	unsigned int source_x = (ntohl(ip_packet->ip_src.s_addr) >> 6) & 3;
	unsigned int source_y = (ntohl(ip_packet->ip_src.s_addr) >> 4) & 3;

	unsigned int my_checksum = 0;
	unsigned short *my_ip_packet = (unsigned short *)(frame.data + sizeof(sr_ethernet_hdr));
	while (ip_header_length_byte)
	{
		my_checksum += ntohs(*my_ip_packet);
		my_ip_packet++;
		ip_header_length_byte -= 2;
	}
	while (my_checksum >> 16)
		my_checksum = (my_checksum & 0xFFFF) + (my_checksum >> 16);
	my_checksum = ~my_checksum;
	if ((my_checksum & 0xFFFF) != 0)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at network layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	ip_header_length_byte = ip_packet->ip_hl * 4;

	struct sr_udp *udp = (struct sr_udp *)(frame.data + sizeof(struct sr_ethernet_hdr) + ip_header_length_byte);

	bool is_request;

	if (ntohs(udp->port_src) != REQ_SRC_PORT && ntohs(udp->port_src) != RESP_SRC_PORT)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at transport layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	if (ntohs(udp->port_dst) != REQ_DST_PORT && ntohs(udp->port_dst) != RESP_DST_PORT)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at transport layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	if (ntohs(udp->port_dst) == REQ_DST_PORT)
		is_request = true;
	else
		is_request = false;

	if (ntohs(udp->length) > frame.length - sizeof(struct sr_ethernet_hdr) - ip_header_length_byte)
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at transport layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	struct sr_flit *flit = (struct sr_flit *)(frame.data + sizeof(struct sr_ethernet_hdr) + ip_header_length_byte + sizeof(sr_udp));

	if ((is_request & flit->message_type) || (!is_request & !flit->message_type))
	{
		pthread_mutex_lock(&print_mutex);
		cout << "A packet dropped at transport layer" << endl;
		pthread_mutex_unlock(&print_mutex);
		return;
	}

	if (flit->message_type)	//Response
	{
		if (flit->data_valid) //Source processor sent the data of specified address
		{
			pthread_mutex_lock(&print_mutex);
			cout << "The data of " << std::hex << ntohs(flit->address) << " was taken from ";
			cout << std::dec << (source_y * 4 + source_x) << endl;
			pthread_mutex_unlock(&print_mutex);

			uint64_t data = be64toh(flit->data);

			pthread_mutex_lock(&cache_memory_access);
			cache_memory.insert(make_pair(ntohs(flit->address), data)); 
			pthread_mutex_unlock(&cache_memory_access);
		}
		else //Source processor doesn't contain the data of specified address
		{
			pthread_mutex_lock(&print_mutex);
			cout << std::dec << (source_y * 4 + source_x) << " hasn't the data of ";
			cout << std::hex << ntohs(flit->address) << endl;
			pthread_mutex_unlock(&print_mutex);
		}
	}
	else //Request
	{
		if (flit->data_valid)
		{
			pthread_mutex_lock(&print_mutex);
			cout << "A packet dropped at application layer" << endl;
			pthread_mutex_unlock(&print_mutex);
			return;
		}

		pthread_mutex_lock(&print_mutex);
		cout << std::dec << (source_y * 4 + source_x) << " requested the data of ";
		cout << std::hex << ntohs(flit->address) << endl;
		pthread_mutex_unlock(&print_mutex);

		const int response_frame_length = sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct sr_udp) + sizeof(struct sr_flit);
		byte *response_data = new byte[response_frame_length];
		struct sr_ethernet_hdr *response_eth = (struct sr_ethernet_hdr *)response_data;
		memset(response_eth->ether_dhost, 0xFF, ETHER_ADDR_LEN);
		memcpy(response_eth->ether_shost, iface[EJECTION_INTERFACE_INDEX].mac, ETHER_ADDR_LEN);
		response_eth->ether_type = htons(ETHERTYPE_IP);

		struct ip *response_ip_packet = (struct ip *)(response_data + sizeof(struct sr_ethernet_hdr));
		response_ip_packet->ip_hl = sizeof(struct ip) / 4;
		response_ip_packet->ip_v = 4;
		response_ip_packet->ip_tos = 0;
		response_ip_packet->ip_len = htons(sizeof(struct ip) + sizeof(struct sr_udp) + sizeof(struct sr_flit));
		response_ip_packet->ip_id = 10000 * my_core_id2 + (3 * number_of_sent2++);
		response_ip_packet->ip_off = 0;
		response_ip_packet->ip_ttl = 64;
		response_ip_packet->ip_p = 17;
		response_ip_packet->ip_sum = 0;
		response_ip_packet->ip_src.s_addr = htonl(iface[EJECTION_INTERFACE_INDEX].getIp());
		response_ip_packet->ip_dst.s_addr = ip_packet->ip_src.s_addr;

		unsigned int response_my_checksum = 0;
		int response_my_ip_header_len = sizeof(struct ip);
		unsigned short *response_my_ip_packet = (unsigned short *)(response_data + sizeof(struct sr_ethernet_hdr));
		while (response_my_ip_header_len)
		{
			response_my_checksum += ntohs (*response_my_ip_packet);
			response_my_ip_packet++;
			response_my_ip_header_len -= 2;
		}
		while (response_my_checksum >> 16)
			response_my_checksum = (response_my_checksum & 0xFFFF) + (response_my_checksum >> 16);

		response_ip_packet->ip_sum = htons(~response_my_checksum);

		struct sr_udp *response_udp = (sr_udp *)(response_data + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
		response_udp->port_src = htons(RESP_SRC_PORT);
		response_udp->port_dst = htons(RESP_DST_PORT);
		response_udp->length = htons(sizeof(struct sr_udp) + sizeof(struct sr_flit));
		response_udp->udp_sum = 0;

		struct sr_flit *response_flit = (sr_flit *)(response_data + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct sr_udp));

		pthread_mutex_lock(&cache_memory_access);
		if (cache_memory.find(ntohs(flit->address)) != cache_memory.end()) //We have data
		{
			uint64_t data = cache_memory[ntohs(flit->address)];
			pthread_mutex_unlock(&cache_memory_access);
			response_flit->message_type = 1;
			response_flit->data_valid = 1;
			response_flit->mode = 0;
			response_flit->address = flit->address;
			
			response_flit->data = htobe64(data);

			pthread_mutex_lock(&print_mutex);
			cout << "Requested data was sent to " << std::dec << (source_y * 4 + source_x) << endl;
			pthread_mutex_unlock(&print_mutex);
		}
		else //We don't have data
		{
			pthread_mutex_unlock(&cache_memory_access);
			response_flit->message_type = 1;
			response_flit->data_valid = 0;
			response_flit->mode = 0;
			response_flit->address = flit->address;
			response_flit->data = 0;

			pthread_mutex_lock(&print_mutex);
			cout << "Empty response message was sent to " << std::dec << (source_y * 4 + source_x) << endl;
			pthread_mutex_unlock(&print_mutex);
		}

		Frame response_frame(response_frame_length, response_data);
		sendFrame(response_frame, EJECTION_INTERFACE_INDEX);
		delete[] response_data;
	}
}

void SimulatedMachine::run () 
{
	string command;

	while (cin >> command)
	{
		if (command == "load")
		{
			uint16_t address;
			uint64_t data;
			cin >> std::hex >> address;
			data = readMainMemory(address);

			pthread_mutex_lock(&cache_memory_access);
			cache_memory.insert(make_pair(address, data));
			pthread_mutex_unlock(&cache_memory_access);
		}
		else if (command == "print")
		{
			uint16_t address;
			cin >> std::hex >> address;
			pthread_mutex_lock(&cache_memory_access);
			if (cache_memory.find(address) != cache_memory.end())
			{
				pthread_mutex_lock(&print_mutex);
				cout << "The data of requested address is " << std::hex << cache_memory[address] << endl;
				pthread_mutex_unlock(&print_mutex);
				pthread_mutex_unlock(&cache_memory_access);
			}
			else
			{
				pthread_mutex_unlock(&cache_memory_access);
				pthread_mutex_lock(&print_mutex);
				cout << "The data of requested address isn't in the cache memory" << endl;
				pthread_mutex_unlock(&print_mutex);
			}
		}
		else if (command == "request")
		{
			unsigned int dest_core_id;
			uint16_t address;
			cin >> std::dec >> dest_core_id;
			cin >> std::hex >> address;

			const int frame_length = sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct sr_udp) + sizeof(struct sr_flit);
			byte *data = new byte[frame_length];
			struct sr_ethernet_hdr *eth = (struct sr_ethernet_hdr *)data;
			memset(eth->ether_dhost, 0xFF, ETHER_ADDR_LEN);
			memcpy(eth->ether_shost, iface[EJECTION_INTERFACE_INDEX].mac, ETHER_ADDR_LEN);
			eth->ether_type = htons(ETHERTYPE_IP);

			struct ip *ip_packet = (struct ip *)(data + sizeof(struct sr_ethernet_hdr));
			ip_packet->ip_hl = sizeof(struct ip) / 4;
			ip_packet->ip_v = 4;
			ip_packet->ip_tos = 0;
			ip_packet->ip_len = htons(sizeof(struct ip) + sizeof(struct sr_udp) + sizeof(struct sr_flit));
			ip_packet->ip_id = 10000 * my_core_id1 + (3 * number_of_sent1++);
			ip_packet->ip_off = 0;
			ip_packet->ip_ttl = 64;
			ip_packet->ip_p = 17;
			ip_packet->ip_sum = 0;
			ip_packet->ip_src.s_addr = htonl(iface[EJECTION_INTERFACE_INDEX].getIp());
			
			unsigned int dest_x = dest_core_id % 4;
			unsigned int dest_y = dest_core_id / 4;
			ip_packet->ip_dst.s_addr = htonl((192 << 24) + (168 << 16) + (0 << 8) + ((dest_x << 6) + (dest_y << 4) + (2 << 2) + (1)));
			
			unsigned int my_checksum = 0;
			int my_ip_header_len = sizeof (struct ip);
			unsigned short *my_ip_packet = (unsigned short *)(data + sizeof(struct sr_ethernet_hdr));
			while (my_ip_header_len)
			{
				my_checksum += ntohs (*my_ip_packet);
				my_ip_packet++;
				my_ip_header_len -= 2;
			}	
			while (my_checksum >> 16)
				my_checksum = (my_checksum & 0xFFFF) + (my_checksum >> 16);

			ip_packet->ip_sum = htons (~my_checksum);

			struct sr_udp *udp = (sr_udp *)(data + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
			udp->port_src = htons(REQ_SRC_PORT);
			udp->port_dst = htons(REQ_DST_PORT);
			udp->length = htons(sizeof(struct sr_udp) + sizeof(struct sr_flit));
			udp->udp_sum = 0;

			struct sr_flit *flit = (sr_flit *)(data + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct sr_udp));
			flit->message_type = 0;
			flit->data_valid = 0;
			flit->mode = 0;
			flit->address = htons(address);
			flit->data = 0;

			pthread_mutex_lock(&print_mutex);
			cout << "Request message for the data at " << std::hex << address << " was sent to ";
			cout << std::dec << dest_core_id << endl;
			pthread_mutex_unlock(&print_mutex);

			Frame frame(frame_length, data);
			sendFrame(frame, EJECTION_INTERFACE_INDEX);
			delete[] data;
		} 
	}
}

void SimulatedMachine::parseArguments (int argc, char *argv[]) 
{

}


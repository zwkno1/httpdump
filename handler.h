#pragma once

#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <assert.h>

#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>

class Handler
{
public:
    void handle(timeval ts, const uint8_t * data, uint32_t size)
    {
		uint32_t hl = sizeof(ether_header);
		if(hl > size)
		  return;

		ts_ = ts;
        auto eh = reinterpret_cast<const ether_header *>(data);
        if(ntohs(eh->ether_type) == ETHERTYPE_IP)
        {
			handle_ip(data + hl, size -hl);
		}
	}

    void handle_ip(const uint8_t * data, uint32_t size)
    {
		auto iph = reinterpret_cast<const iphdr *>(data);
		boost::asio::ip::address src, dst;
		char buf[64];
		if(iph->version == 4)
		{
			uint32_t hl = iph->ihl * 4;
			if(hl < size)
			{
				auto src = boost::asio::ip::address::from_string(inet_ntop(AF_INET, &iph->saddr, buf, sizeof(buf)));
				auto dst = boost::asio::ip::address::from_string(inet_ntop(AF_INET, &iph->daddr, buf, sizeof(buf)));
				if(iph->protocol == IPPROTO_TCP)
				{
					handle_tcp(data + hl, size - hl, src, dst);
				}
			}
		}
		else if(iph->version == 6)
		{
			auto ip6h = reinterpret_cast<const ip6_hdr *>(data);

			// compute head len
			uint8_t nxt = ip6h->ip6_ctlun.ip6_un1.ip6_un1_nxt;
			uint8_t len = sizeof(ip6_hdr);
			uint32_t hl = len;
			const static uint8_t ext_type[] = {0, 60, 43, 44, 51, 50, 60, 135, 139, 140, 253, 254 }; 
			while((hl > size) && std::find(std::begin(ext_type), std::end(ext_type), nxt) != std::end(ext_type))
			{
				auto ip6eh = reinterpret_cast<const ip6_ext *>(data + hl);
				nxt = ip6eh->ip6e_nxt;
				len = ip6eh->ip6e_len;
				hl += len;
			}

			if(hl < size)
			{
				std::cout << "compute ipv6 len error" << std::endl;
				return;
			}

			auto src = boost::asio::ip::address::from_string(inet_ntop(AF_INET6, &ip6h->ip6_src, buf, sizeof(buf)));
			auto dst = boost::asio::ip::address::from_string(inet_ntop(AF_INET6, &ip6h->ip6_dst, buf, sizeof(buf)));
			if(nxt == IPPROTO_TCP)
			{
				if(iph->protocol == IPPROTO_TCP)
				{
					handle_tcp(data + hl, size - hl, src, dst);
				}
			}
        }
		else
		{
			std::cout << "unsupported version: " << iph->version << std::endl;
			return;
		}
    }

    void handle_tcp(const uint8_t * data, uint32_t size, boost::asio::ip::address & src_addr, boost::asio::ip::address & dst_addr)
    {
		auto tcph = reinterpret_cast<const tcphdr *>(data);
		boost::asio::ip::tcp::endpoint src(src_addr, ntohs(tcph->th_sport));
		boost::asio::ip::tcp::endpoint dst(dst_addr, ntohs(tcph->th_dport));
		
		uint32_t hl = tcph->th_off * 4;
		if(hl > size)
		{
			std::cout << "tcp size error" << hl << "," << size << std::endl;
			return;
		}

		if((src.port() == 80) || (dst.port() == 80))
		{
			std::tm tm = *std::localtime(&ts_.tv_sec);
			uint32_t dl = size - hl;
			std::cout << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.") << ts_.tv_usec << "] ";
			std::cout << "[" << tcph->seq << " ";
			
			if(tcph->fin) std::cout << " FIN";
			if(tcph->rst) std::cout << " RST";
			if(tcph->syn) std::cout << " SYN";
			if(tcph->ack) std::cout << " ACK";

			std::cout << "[" << src << " -> " << dst << "]\n" << std::string((const char *)data + hl, size - hl) << std::endl;
		}
    }

private:
	timeval ts_;
};

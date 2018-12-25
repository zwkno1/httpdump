#pragma once

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "libnids.h"

inline std::ostream & operator<< (std::ostream & os, const struct tuple4 & t)
{
	os << "[ " << inet_ntoa(in_addr{t.saddr}) << ":" << t.source 
		<< "\t" << inet_ntoa(in_addr{t.daddr}) << ":" << t.dest << " ]";
	return os;
}

class Handler
{
public:
	void handle_tcp(tcp_stream * ts, void **)
	{
		std::cout << "handle tcp" <<std::endl;
		if (ts->nids_state == NIDS_JUST_EST)
		{
			handle_established(ts);
		}
		else if (ts->nids_state == NIDS_CLOSE)
		{
			handle_close(ts);
		}
		else if (ts->nids_state == NIDS_RESET)
		{
			handle_reset(ts);
		}
		else if (ts->nids_state == NIDS_DATA)
		{
			handle_data(ts);
		}
	}

	void handle_data(tcp_stream * ts)
	{
		if(ts->client.count_new_urg)
		{
			std::cout << ts->addr << " URG <- ";
			std::cout.write((char *)&ts->client.urgdata, 1);
			std::cout << std::endl;
		}

		if (ts->server.count_new_urg)
		{
			std::cout << ts->addr << " URG -> ";
			std::cout.write((char *)&ts->server.urgdata, 1);
			std::cout << std::endl;
		}

		if (ts->client.count_new)
		{
			std::cout << ts->addr << " -> ";
			std::cout.write(ts->client.data, ts->client.count);
			std::cout << std::endl;
		}
		else
		{
			std::cout << ts->addr << " -> ";
			std::cout.write(ts->server.data, ts->server.count);
			std::cout << std::endl;
		}
	}

	void handle_established(tcp_stream * ts)
	{
		// connection described by ts is established
		// here we decide, if we wish to follow this stream
		// sample condition: if (ts->addr.dest!=23) return;
		// in this simple app we follow each stream, so..
		ts->client.collect++; // we want data received by a client
		ts->server.collect++; // and by a server, too
		ts->client.collect_urg++; // we want urgent data received by a client
		ts->server.collect_urg++; // we want urgent data received by a server
		std::cout << ts->addr << " ESTABLISHED" << std::endl;
	}

	void handle_close(tcp_stream * ts)
	{
		// connection has been closed normally
		std::cout << ts->addr << " CLOSING" << std::endl;
	}

	void handle_reset(tcp_stream * ts)
	{
		// connection has been closed by RST
		std::cout << ts->addr << " RESET" << std::endl;
	}
};

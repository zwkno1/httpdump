#include <string>
#include <iostream>
#include <stdexcept>
#include <boost/regex.hpp>
#include <tins/tcp_ip/stream_follower.h>
#include <tins/sniffer.h>

#include "handler.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <interface> filter" << std::endl;
        return 1;
    }

    std::stringstream ss;
    for(int i = 2; i < argc; ++ i)
    {
        ss << argv[i] << " ";
    }

    try
    {
        // Construct the sniffer configuration object
        Tins::SnifferConfiguration config;
        // Get packets as quickly as possible
        config.set_immediate_mode(true);
        // Only capture TCP traffic sent from/to port 80
        config.set_filter(ss.str());
        // Construct the sniffer we'll use
        Tins::Sniffer sniffer(argv[1], config);

        std::cout << "Starting capture on interface " << argv[1] << std::endl;

        Handler h;

        // Now construct the stream follower
        Tins::TCPIP::StreamFollower follower;
        // We just need to specify the callback to be executed when a new
        // stream is captured. In this stream, you should define which callbacks
        // will be executed whenever new data is sent on that stream
        // (see on_new_connection)
        follower.new_stream_callback(std::bind(&Handler::handleTcpEstablished, &h, std::placeholders::_1));
        follower.stream_termination_callback(std::bind(&Handler::handleTcpClosed, &h, std::placeholders::_1));
        // Now start capturing. Every time there's a new packet, call
        // follower.process_packet

        sniffer.sniff_loop([&](Tins::PDU& packet)
        {
            follower.process_packet(packet);
            return true;
        });
    }
    catch (std::exception & ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}

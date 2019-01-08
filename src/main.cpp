#include <string>
#include <iostream>
#include <stdexcept>
#include <boost/regex.hpp>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <tins/tcp_ip/stream_follower.h>
#include <tins/sniffer.h>

#include "tcphandler.h"
#include "httphandler.h"

typedef HttpHandlerPtr (createHttpHandlerFunc)();
typedef boost::dll::detail::import_type<createHttpHandlerFunc>::type SlotFactory;

std::vector<HttpHandlerPtr> loadHttpHandlers(const std::string & configFile)
{
	std::vector<HttpHandlerPtr> handlers;
    try
    {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(configFile, tree);

		std::string workDir = tree.get<std::string>("workDir");

        for(auto const & i : tree.get_child("httpHandlers"))
        {
			std::string handlerName = i.second.get_value<std::string>();

			std::cout << "[PLUGIN]" << handlerName << std::endl;
			boost::filesystem::path libPath(workDir);
			libPath /= handlerName;
			auto factory = boost::dll::import<createHttpHandlerFunc>(
					libPath,
					"createHttpHandler",
					boost::dll::load_mode::append_decorations
					);

			auto h = factory();
			if(h)
			{
				handlers.push_back(h);
			}
		}
    }
    catch(boost::property_tree::ptree_error & err)
    {
		std::cout << "parse config " << configFile << " failed: " << err.what() << std::endl;
    }
	catch(std::exception & e)
	{
		std::cout << "load config" << configFile << " failed: " << e.what() << std::endl;
	}

	return handlers;
}

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

        std::cout << "[CAPTURE]" << argv[1] << std::endl;

        TcpHandler h;
		std::vector<HttpHandlerPtr> httpHandlers = loadHttpHandlers("./plugin.json");
		for(auto & i : httpHandlers)
		{
			h.registHandler(i);
		}

        // Now construct the stream follower
        Tins::TCPIP::StreamFollower follower;
        // We just need to specify the callback to be executed when a new
        // stream is captured. In this stream, you should define which callbacks
        // will be executed whenever new data is sent on that stream
        // (see on_new_connection)
        follower.new_stream_callback(std::bind(&TcpHandler::handleTcpEstablished, &h, std::placeholders::_1));
        follower.stream_termination_callback(std::bind(&TcpHandler::handleTcpClosed, &h, std::placeholders::_1));
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
        std::cerr << "[ERROR]" << ex.what() << std::endl;
        return 1;
    }
}

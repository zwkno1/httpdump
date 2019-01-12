#include <string>
#include <iostream>
#include <stdexcept>
#include <boost/regex.hpp>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

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

struct Config
{
	std::string interface;
	std::string filter;
	std::string pluginConfigFile;


	bool parse(int argc, char * argv[])
	{
		try
		{
			boost::program_options::options_description desc("Allowed options");
			desc.add_options()
				("help,h", "print help message")
				("interface,i", boost::program_options::value<std::string>()->default_value("any"), "interface")
				("plugin-config,c", boost::program_options::value<std::string>()->default_value("./plugin.json"), "plugin config file")
				("filter,f", boost::program_options::value< std::vector<std::string> >(), "filter")
				;
			boost::program_options::positional_options_description p;
			p.add("filter", -1);

			boost::program_options::variables_map vm;
			boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
			boost::program_options::notify(vm);    

			if(vm.count("help"))
			{
				std::cout << "Usage: httpdump [options]\n" << desc << std::endl;
				return false;
			}

			if (vm.count("interface")) 
			{
				interface = vm["interface"].as<std::string>();
			}

			if(vm.count("plugin-config"))
			{
				pluginConfigFile = vm["plugin-config"].as<std::string>();
			}

			if(vm.count("filter"))
			{
				for(auto & s : vm["filter"].as< std::vector<std::string> >())
				{
					filter.append(s);
					filter.append(" ");
				}
			}
			return true;
		}
		catch(std::exception & e)
		{
			std::cout << e.what() << std::endl;
			return false;
		}
	}
};

class HttpHandlerDispatcher : public HttpHandler
{
public:
    void registHandler(HttpHandlerPtr & h)
    {
        handlers_.push_back(h);
    }

    bool handleHttpRequest(const Tuple4 & tuple4, HttpRequest & message) override
    {
        std::cout << message << std::endl;
        for(auto & h : handlers_)
        {
            if(h->handleHttpRequest(tuple4, message))
            {
                return true;
            }
        }
        return false;
    }

    bool handleHttpResponse(const Tuple4 & tuple4, HttpResponse & message) override
    {
        std::cout << message << std::endl;
        for(auto & h : handlers_)
        {
            if(h->handleHttpResponse(tuple4, message))
            {
                return true;
            }
        }
        return false;
    }

private:
    std::vector<HttpHandlerPtr> handlers_;
};

int main(int argc, char* argv[])
{
	Config config;
	if(!config.parse(argc, argv))
	{
		return 1;
	}

	try
	{
		// Construct the sniffer configuration object
		Tins::SnifferConfiguration snifferConfig;
		// Get packets as quickly as possible
		snifferConfig.set_immediate_mode(true);
		// set filter
		if(!config.filter.empty())
		  snifferConfig.set_filter(config.filter);
		// Construct the sniffer we'll use
		Tins::Sniffer sniffer(config.interface, snifferConfig);

		std::cout << "[CAPTURE]" << config.interface << std::endl;

        auto d = std::make_shared<HttpHandlerDispatcher>();
		std::vector<HttpHandlerPtr> httpHandlers = loadHttpHandlers(config.pluginConfigFile);
		for(auto & i : httpHandlers)
		{
            d->registHandler(i);
        }

        TcpHandler h{d};

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

		return 0;
	}
	catch (std::exception & ex)
	{
		std::cerr << "[ERROR]" << ex.what() << std::endl;
		return 1;
	}
}

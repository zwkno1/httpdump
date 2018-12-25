#include <pcap.h>

#include <iostream>

#include "handler.h"

#define PROMISCUOUS 1
#define NONPROMISCUOUS 0

void list_device()
{
    char err[PCAP_ERRBUF_SIZE] = {0};
    pcap_if_t * dev = nullptr;
    if(pcap_findalldevs(&dev, err) != 0)
    {
        std::cout << err << std::endl;
        return;
    }

    for(pcap_if_t * d = dev; d; d = d->next)
    {
        std::cout << d->name << "\t" << (d->description ? d->description : "") << std::endl;
        for(pcap_addr * addr = d->addresses; addr; addr = addr->next)
        {
            char net[128] = {0};
            if(addr->addr->sa_family == AF_INET)
            {
                inet_ntop(addr->addr->sa_family, &((sockaddr_in *)addr->addr)->sin_addr, net, sizeof(net));
            }
            else if(addr->addr->sa_family == AF_INET)
            {
                inet_ntop(addr->addr->sa_family, &((sockaddr_in6 *)addr->addr)->sin6_addr, net, sizeof(net));
            }

            if(*net)
                std::cout << net << std::endl;
        }
    }

    pcap_freealldevs(dev);

}

void loop_callback(u_char * arg, const struct pcap_pkthdr *pkthdr, const u_char * data)
{
	// check truncated
	if(pkthdr->len != pkthdr->caplen)
	{
		std::cout << "packet trucated" << std::endl;
		return;
	}

    reinterpret_cast<Handler *>(arg)->handle(pkthdr->ts, data, pkthdr->len);
}

int main(int argc, char *argv[])
{
    list_device();
	if(argc < 2)
	{
		std::cout << "usage: " << argv[0] << " device" << std::endl;
		return 1; 
	}

    const char * deviceName = argv[1];
    char err[PCAP_ERRBUF_SIZE] = {0};
    struct bpf_program fp;

    pcap_t * cap = pcap_open_live(deviceName, 65535, 0, -1, err);
    if(!cap)
    {
        std::cout << err << std::endl;
        return 1;
    }

    bpf_u_int32 net, mask;
    if (pcap_lookupnet(deviceName, &net, &mask, err) == -1)
    {
        std::cout << err << std::endl;
        return 1;
    }

    // Set compile option.
    if (pcap_compile(cap, &fp, "tcp", 0, net) == -1)
    {
        fprintf(stderr, "compile error\n");
        return 1;
    }

    // Set packet filter role by compile option.
    if (pcap_setfilter(cap, &fp) == -1)
    {
        fprintf(stderr, "set filter error\n");
        return 1;
    }

    Handler h;
    pcap_loop(cap, 0, loop_callback, (u_char *)&h);
    return 0;
}

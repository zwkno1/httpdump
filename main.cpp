
#include <iostream>
#include <fstream>
#include <stdio.h>

#include "libnids.h"
#include "handler.h"

Handler h;

void tcp_callback(struct tcp_stream * s, void ** x)
{
	// never callback
	std::cout << "tcp callback xxx" << std::endl;
	std::fstream f("./a.txt");
	f << "asdasd";
	f.close();
	
	// ok
	printf("tcp callback\n");
	h.handle_tcp(s, x);
}

int main()
{
	// here we can alter libnids params, for instance:
	// nids_params.n_hosts=256;
	//char str[] = "wlp3s0";
	//nids_params.device = str;
	if (!nids_init ())
	{
		std::cout << "nids init error: " << nids_errbuf << std::endl;
		return 1;
	}
	std::cout << "init ok" << std::endl;
	std::cout << nids_params.device << std::endl;
	std::cout << nids_params.filename << std::endl;
	nids_register_tcp ((void *)tcp_callback);
	nids_run ();
	return 0;
}


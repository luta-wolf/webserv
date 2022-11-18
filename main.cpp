#include <iostream>
#include <string>
#include "TCPlistener.hpp"
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "RequestParser.hpp"
#include "Response.hpp"
#include "Request.hpp"

using namespace std;


//int main(int ac, char** av)
//{
//	Config config;
//	fd_set fdRead;
//	fd_set fdWrite;
//	int _numSelect = 0;
//	TCPlistener listeners[10];
//	FD_ZERO(&fdRead);
//	FD_ZERO(&fdWrite);
//
//	if(av[1])
//		config.Parse(av[1]);
//	else
//		config.Parse("av[1]");
//
//	for (int i = 0; i < config.GetSize(); i++)
//	{
//		cout << config.GetPorts(i);
//		listeners[i].Init(config.GetPorts(i));
//		int rez = listeners[i].Socket(fdRead);
//		_numSelect = rez + 1;
//		FD_SET(rez, &fdRead);
//	}
//	while(1)
//	{
//		fd_set FdRead = fdRead;
//		fd_set FdWrite = fdWrite;
//		if(select(_numSelect, &FdRead, &FdWrite, NULL, NULL) < 0)
//			continue;
//		for (int i = 0; i < config.GetSize(); i++)
//		{
//			listeners[i].Listen(fdRead, fdWrite, FdRead, FdWrite, _numSelect);
//		}
//	}
//}

int main(int argc, char **argv, char **ev)
{

	fd_set fdRead;
	fd_set fdWrite;
	int _numSelect = 0;
	TCPlistener listeners[10];
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);

    std::string a = "default.conf";
    if (argc == 2)
        a =	argv[1];
    Config config(a);
    try
    {
        config.main_read();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        exit(1);
    }

    std::map<int, std::string>  servers = config.getBinders();
    std::map<int, std::string>::iterator it = servers.begin();
    std::map<int, std::string>::iterator end = servers.end();

    int i = 0;

	for (; it != end; ++it, i++)
	{
		std::cout << "Binding to " << it->second << " on port " << it->first << std::endl;
		listeners[i].Init(it->first, it->second);
		int rez = listeners[i].Socket();
		_numSelect = rez + 1;
		FD_SET(rez, &fdRead);
	}
	while (1)
	{
		fd_set FdRead = fdRead;
		fd_set FdWrite = fdWrite;
		if (select(_numSelect, &FdRead, &FdWrite, NULL, NULL) < 0)
			continue;
		for (unsigned long i = 0; i < config.getBinders().size(); i++)
			listeners[i].Listen(fdRead, fdWrite, FdRead, FdWrite, std::make_pair(&_numSelect, config), ev); // передаю pair из-за ограничения по кол-ву передаваемых в функцию аргументов
	}
}


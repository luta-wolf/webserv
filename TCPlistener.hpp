


#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include "Request.hpp"
#include "RequestParser.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <ctime>

#include <cstring>
#include <sstream>
#include <map>
#include <vector>
#include <sys/select.h>


class Request;
class RequestParser;

using namespace std;

class TCPlistener
        {
private:
	int						__port;
	int						__socketFd;
	struct sockaddr_in		__servAddr;
	map<int, Request>		__connections;
    std::string             __ip;
public:
	TCPlistener(int port);
	TCPlistener();
	~TCPlistener();
	void Init(int port, std::string &ip);
	int Socket();
	void Listen(fd_set &_fdRead, fd_set &_fdWrite,fd_set &fdRead, fd_set &fdWrite, std::pair<int *, Config> pair, char **Envp);
};


#endif

#include "TCPlistener.hpp"

TCPlistener::TCPlistener() : __port(80), __socketFd(0), __ip("127.0.0.1") {}

TCPlistener::TCPlistener(int port) : __port(port), __socketFd(0), __ip("127.0.0.1") {}

TCPlistener::~TCPlistener() {}

void TCPlistener::Init(int port, std::string &ip)
{
	__port = port;
    __ip = ip;
}

int  TCPlistener::Socket()
{
	if((__socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	__servAddr.sin_family = AF_INET;
	__servAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	__servAddr.sin_port = htons(__port);

	if ((bind(__socketFd, (struct sockaddr*)&__servAddr, sizeof(__servAddr))) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (listen(__socketFd, 32))
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	int opt;
	setsockopt(__socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	cout << "Socket OK\n";
	return __socketFd;
}

void TCPlistener::Listen(fd_set &_fdRead, fd_set &_fdWrite,fd_set &fdRead, fd_set &fdWrite, std::pair<int *, Config> pair, char **Envp)
{
	for (int i = 0; i <= *(pair.first); i++)
	{
		Request *con;
		if (FD_ISSET(i, &fdRead) > 0)
		{
			if (i == __socketFd)
			{
				socklen_t size = sizeof(__servAddr);
				int conFd = accept(__socketFd, (struct sockaddr*)&__servAddr, &size);
				if (conFd < 0)
				{
					perror("conFd");
					exit(1);
				}
				if (conFd >= *pair.first)
					*(pair.first) = conFd + 1;
				fcntl(conFd, F_SETFL, O_NONBLOCK);
				FD_SET(conFd, &_fdRead);
				__connections.insert(make_pair(conFd, Request(conFd)));
			}
			else if(__connections.find(i) != __connections.end())
			{
				con = &(__connections.find(i)->second);
				char buf[1024];
				int red = recv(i, buf, 1024, 0);
				if (red == 0 && con->getRequest() == "")
				{
					FD_CLR(i, &_fdRead);
					return;
				}
				else if (con->getRequest() == "" && (!(con->PreParsing(buf))))
				{
					FD_CLR(i, &_fdRead);
					FD_SET(i, &_fdWrite);
					return;
				}
				else if (red == 0)
				{
					FD_CLR(i, &_fdRead);
					FD_SET(i, &_fdWrite);
					return;
				}

				if (red < 1024)
				{
					string s = buf;
					con->setRequest(s);
					FD_CLR(i, &_fdRead);
					FD_SET(i, &_fdWrite);
				}
				else if (red == 1024)
				{
					string s = buf;
					con->setRequest(s);
				}
				return;
			}
		}
		if (FD_ISSET(i, &fdWrite) > 0 && (__connections.end() != __connections.find(i)))
		{
			con = &(__connections.find(i)->second);
			if (con->getResponse() == "")
			{
				con->Parse(pair.second, Envp);
			}
			else
			{
				string str2 = con->getResponse();
				if (str2.size() > 1024)
				{
					send(i, str2.data(), 1024, 0); //нужно защитить send и при ошибке удалить клиента(чек-лист)
					con->setResponse(str2.data() + 1024);
				}
				else
				{
					send(i, str2.data(), str2.size(), 0);
					FD_CLR(i, &_fdWrite);
					close(i);
					__connections.erase(i);
//                    if(con)
//                        delete con;
				}
			}
		}
	}
	return;
}

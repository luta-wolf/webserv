#ifndef REQUEST_HPP
# define REQUEST_HPP

#define SIZE_BUFFER	16384

#include <iostream>
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
#include "RequestParser.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "CGI.hpp"

class Config;
#pragma once
class RequestParser;
//class CGI;
//class Response;

class Request
{

private:
		std::string __request;
		std::string __response;
		int			__fd;
		RequestParser   request_class;
		Response        response_class;
        Location        location;

        std::pair<std::string, bool>		body_inf;
        std::ifstream						file;
        std::string							buffer;


	public:
		Request();
		Request(int fd);
		Request(const Request &x);
		Request &operator=(const Request &x);
		~Request();
		int getFd() const;
		void setRequest(const std::string &request);
		void setResponse(const std::string &response);
		std::string getResponse() const;
		std::string getRequest() const;
		bool PreParsing(const std::string &preRequest);
		void Parse(Config &config, char **Envp);
        void write_response();
};

#endif

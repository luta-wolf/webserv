
#ifndef CGI_HPP
# define CGI_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include "RequestParser.hpp"
//#include "Config.hpp"
//#include "Server.hpp"
//#include "Location.hpp"
//#include <vector>
//#include <iterator>
#include <ctime>
#include <fcntl.h>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>


#pragma once
class RequestParser;
//class Location;

class Request;

class CGI
{
//	int									_pid;
//	std::string							_status;
//	bool								finished;
	std::string getDateHeader();
private:
//	std::map<std::string, std::string>	_env;
	char								fileName[20];
	int						    		_fd;
    int                                _cgi;
    int                                 pip[2];
    std::string                         _status;
public:
	CGI();
	~CGI();
	void cgi(RequestParser request, std::string cgiPath, std::string fullpath);
	void craft_response(Request &client);
	bool is_finished(Request &client);

    void cgi_error(Request &client);
};


#endif



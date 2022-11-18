
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <fstream>
#include "Server.hpp"
class Server;


class Config
{
    protected:
        std::string file_name;

    private:
        std::string full_file;
        std::vector<std::string> lines;
        std::vector<Server> servers;
        std::map<int, std::string> _binders;


    public :
        static std::map<std::string, std::string>	statusCodeMessages;
        static std::map<std::string, std::string> 	mimetypeMap;

    class fileErrorException : public std::exception
    {
        const char * what() const throw()
        {
            return " : File Not Found";
        }
    };

    class NotclosedBrackets : public std::exception
    {
        const char * what() const throw()
        {
            return "Brackets are not closed";
        }
    };

    class NoServerFoundException : public std::exception
    {
        const char * what() const throw()
        {
            return "No Server found";
        }
    };

    class MultpiteServerName : public std::exception
    {
        const char * what() const throw()
        {
            return "Multiple servers with same server name";
        }
    };

    Config();
    Config(std::string file);
    ~Config();
    Config(Config const& c);
    Config &operator=(Config const& c);

    std::vector<Server> getServers() const;
    std::string getFullFile() const;
    std::vector<std::string> getLines() const;
    std::string getLine(std::string &) const;
    Server getServer(int port, std::string &) const;
    void getServer2(int port );
    void setfile(std::string&);
    std::map<int, std::string > getBinders() const;

    void main_read();
    void read_file();
    void split_line();
    void split_servers();
    void parse_server();
    void parse_bind_map();
    void checkServerNamess();
    void main_error_check();
    void check_brekets();
    void init_statusCodeMessages();
    void init_mimetype();
};

//#include "CGI.h"

#endif
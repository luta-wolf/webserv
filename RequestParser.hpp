
#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "CGI.hpp"
#pragma once
class CGI;
class Config;
//class Server;

class RequestParser
{
        private:
            std::map<std::string, std::string>  headerMap;
            std::string                         method;
            std::string                         url;
            std::string                         tmp_url;
            std::string                         version;
            std::string                         query;
            std::string                         host_ip;
            int                                 host_port;
            std::string                         content_type;
            long long                           content_lenght;
            std::string                         body;
            Server                              server;
            Location                            location;
            std::string							filepath;
            bool 								parsed;
			char								**envp;

        public:
            CGI		*cgi_ptr;
            RequestParser();
			void setEnvp(char **Envp);
			char **getEnvp();
            ~RequestParser();

            std::map<std::string, std::string> get_headerMap() const { return headerMap; }
            std::string get_method() const { return method; }
            std::string get_url() const { return url; }
            std::string get_version() const { return version; }
            std::string get_query() const { return query; }
            std::string get_hostIp() const { return host_ip; }
            std::string get_filepath() const { return filepath; }
            int get_hostPort() const { return host_port; }
            long long get_content_lenght() const { return content_lenght; }
            Location getLocation() const { return location; }
            Server getServer() const { return server; }
            bool parse_chunks(const std::string &str, Config config);
            void clear();
            std::string create_file();
            int get_location();

        private:
            void parse_header(const std::string &str, int *parse);
            void parse_first_line(const std::string &str);
            void check_fl_data();
            void parse_header_lines(Config &config);
            void get_server_location(Config &config);
            void write_body();
};

#endif

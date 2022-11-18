

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include "RequestParser.hpp"
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include <vector>
#include <iterator>
#include <ctime>
#include <fcntl.h>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>


class RequestParser;
class Location;

class Response
{
    public:
        Response();
        ~Response();

    private:
        bool                                response_tmp;
        std::map<std::string, std::string>  headers_list;
        Location                            location;
        std::string                         status_code;
        struct stat 						file_stat;
        std::string                         body_path;

    public:
        std::string getHeaders(RequestParser &request, Location &location, std::string &status);
        std::pair<std::string, bool> getBody();
        void setHeaders(std::string key, std::string value);

    private:
        void buildResponse(RequestParser &request, int status_int);
        std::string build_full_path(std::string &path_page);
        bool permission(std::string path, std::string str);
        void set_content_type(std::string &path);
        std::string ltrim(const std::string &str);
        std::string build_location_path(std::string &path, RequestParser &request);
        void write_default_body(std::string &path);
        void choose_location(RequestParser &request);
        void search_for_location(RequestParser &request);
        void check_redirection(RequestParser &request);
        void check_methods(RequestParser &request);
        void methods(RequestParser &request);
        void method_get(RequestParser &request);
        void method_post(RequestParser &request);
        void method_delete(RequestParser &request);
        void validity_check(std::string path, RequestParser &request);
        void directory_end_slash(RequestParser &request);
        void check_index(RequestParser &request);
        void check_root(RequestParser &request);
        void autoindex_handler(RequestParser &request);
        void auto_index_template(std::multimap<std::string, std::pair<struct stat, long long> > &files, std::string tmp_path);
        std::string make_time(struct stat str);
        void check_default_index(std::string &path, RequestParser &request);
        void check_cgi(std::string &path, RequestParser &request);
        void end_slash_delete(RequestParser &request);
        int remove_dir(std::string &path);
        void set_date_header();
        void set_content_length(std::string &body_path);
        std::string make_header();
        std::string make_msg(std::string &status_code);
        long long file_size(std::string &body_path);
};

#endif

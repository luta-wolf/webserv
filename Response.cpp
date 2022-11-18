
#include "Response.hpp"

Response::Response()
{
    setHeaders("Server", "Webserver");
    status_code = "";
    body_path = "";
    response_tmp = false;
}

Response::~Response() {
//    std::cout << "deleteResponse\n";

}

void Response::setHeaders(std::string key, std::string value)
{
    this->headers_list[key] = value;
}

std::pair<std::string, bool> Response::getBody()
{
    return (std::pair<std::string, bool>(body_path, response_tmp));
}

std::string Response::build_full_path(std::string &path_page)
{
    std::string root = location.getRoot() + "/";
    std::string path = path_page.replace(0, location.getLocation_match().length(), root);
    return (path);
}

bool Response::permission(std::string path, std::string str)
{
    if (stat(path.c_str(), &file_stat) < 0)
        return false;
    if (str == "r")
        return (file_stat.st_mode & S_IRUSR);
    if (str == "w")
        return (file_stat.st_mode & S_IWUSR);
    if (str == "x")
        return (file_stat.st_mode & S_IXUSR);
    return (false);
}

void Response::set_content_type(std::string &path)
{
    std::string ext;
    int pos = path.find_last_of('.');
    pos++;
    ext = path.substr(pos, path.size() - pos);
    if (Config::mimetypeMap.find(ext) != Config::mimetypeMap.end())
        setHeaders("Content-Type", Config::mimetypeMap[ext]);
}

std::string Response::ltrim(const std::string &str)
{
    size_t pos = str.find_first_not_of(" \r\n\t\f\v./");
    return ((pos == std::string::npos) ? "" : str.substr(pos));
}

std::string Response::build_location_path(std::string &path, RequestParser &request)
{
    std::string location_path;
    if (strncasecmp(path.c_str(), "http://", 7) == 0)
        location_path = path;
    else
        location_path = "http://" + request.getServer().getIp() + ":" + std::to_string(request.get_hostPort()) + "/" + path;
    return (location_path);
}

void Response::write_default_body(std::string &path)
{
    std::string code = Config::statusCodeMessages.find(status_code) != Config::statusCodeMessages.end() ? Config::statusCodeMessages.find(status_code)->second : "";
    std::string response (
            "<html>"
                "<head>"
                    "<title>" + status_code + " " + code + "</title>"
                "</head>"
                "<body>"
                     "<center>"
                         "<h1>" + status_code + " " + code + "</h1>"
                     "</center>"
                     "<hr>"
                     "<center>webserv/1.1</center>"
               "</body>"
            "</html>\r\n\r\n");
    std::fstream file;
    file.open(path, std::fstream::app);
    file << response;
    file.close();
}

void Response::buildResponse(RequestParser &request, int status_int)
{
    bool have_page = false;
    std::string error_page;

    if (location.getErrorPages().size() > 0 && (location.getErrorPages().find(status_int) != location.getErrorPages().end()))
    {
        error_page = location.getErrorPages().find(status_int)->second;
        if (error_page.at(0) == '/')
        {
            std::string full_error_page = build_full_path(error_page);
            if (stat(full_error_page.c_str(), &file_stat) == 0)
            {
                if (file_stat.st_mode & S_IFREG)
                {
                    if (!permission(full_error_page, "r"))
                    {
                        status_code = "403";
                        have_page = false;
                    }
                    else
                    {
                        body_path = full_error_page;
                        set_content_type(full_error_page);
                        have_page = true;
                        throw "Error page received successfully";
                    }
                }
                else
                {
                    status_code = "501";
                    have_page = false;
                }
            }
            else
            {
                status_code = "404";
                have_page = false;
            }
        }
        else
        {
            status_code = "302";
            error_page = ltrim(error_page);
            error_page = build_location_path(error_page, request);
            setHeaders("Location", error_page);
            throw "Redirection";
        }
    }
    if (!have_page)
    {
        std::string default_error_page;
        default_error_page = request.create_file();
        write_default_body(default_error_page);
        body_path = default_error_page;
        setHeaders("Content-Type", "text/html");
        response_tmp = true;
    }
}

void Response::search_for_location(RequestParser &request)
{
    Server server = request.getServer();
    std::vector<Location> locations = server.getLocation();
    std::vector<Location>::iterator it;
    size_t length = 0;
    Location l;
    for (it = locations.begin(); it != locations.end(); ++it)
    {
        l = *it;
        if (length <= l.getLocation_match().length() && strncasecmp(l.getLocation_match().c_str(), request.get_url().c_str(), l.getLocation_match().length()) == 0)
        {
            length = l.getLocation_match().length();
            location = *it;
        }
    }
}

void Response::choose_location(RequestParser &request)
{
    if (request.get_location() == 0)
        search_for_location(request);
}


void Response::check_redirection(RequestParser &request)
{
    if (location.getRedirectUri().size() > 0)
    {
        int code = location.getRedirectUri().begin()->first;
        status_code = std::to_string(code);
        std::string path = location.getRedirectUri().begin()->second;

        if (code >= 300 && code < 400)
        {
            path = ltrim(path);
            path = build_location_path(path, request);
            setHeaders("Location", path);
            buildResponse(request, code);
            throw "Redirection";
        }
    }
}

void Response::check_root(RequestParser &request)
{
    if (location.getRoot().empty())
    {
        status_code = "404";
        buildResponse(request, 404);
        throw "No root";
    }
    else
    {
        std::string root = location.getRoot();
        struct stat buf;
		if (root == "./uploads" && stat("./uploads", &buf) == -1)
			mkdir("./uploads", 0777);
        else if (stat(root.c_str(), &buf) == -1)
        {
            status_code = "404";
            buildResponse(request, 404);
            throw "Root does not exist";
        }
        if (!permission(root, "x"))
        {
            status_code = "403";
            buildResponse(request, 403);
            throw "Root does not have permission";
        }
    }
}

void Response::check_methods(RequestParser &request)
{
    std::vector<std::string> methods = location.getAllowedMethods();

    if ((find(methods.begin(), methods.end(), request.get_method())) != methods.end())
        check_root(request);
    else
    {
        status_code = "405";
        buildResponse(request, 405);
        throw "Method not allowed";
    }
}

void Response::validity_check(std::string path, RequestParser &request)
{
    struct stat buf;

    if (stat(path.c_str(), &buf) == -1)
    {
        status_code = "404";
        buildResponse(request, 404);
        throw "Resource does not exist";
    }
}

void Response::directory_end_slash(RequestParser &request)
{
	int end = request.get_url().length() - 1;
	if (request.get_url().at(end) != '/')
	{
		status_code = "301";
		std::string path = ltrim(request.get_url());
		path = build_location_path(path, request);
		setHeaders("Location", path + "/");
		buildResponse(request, 301);
		throw "Redirection";
	}
}

std::string Response::make_time(struct stat str)
{
    std::stringstream date;
    file_stat = str;
    std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    struct tm *tm;
    tm = std::gmtime(&file_stat.st_mtime);
    date << std::setw(2) << std::setfill('0') << tm->tm_mday;
    date << "-" << months[tm->tm_mon].c_str();
    date << "-" << tm->tm_year + 1900 << " ";
    date << std::setw(2) << std::setfill('0') << (tm->tm_hour + 3) % 24 << ":" << std::setw(2) << std::setfill('0') << tm->tm_min;
    return (date.str());
}

void Response::auto_index_template(std::multimap<std::string, std::pair<struct stat, long long> > &files, std::string tmp_path)
{
    std::multimap<std::string, std::pair<struct stat, long long> >::iterator it;
    std::stringstream index;
    index <<	"<html><head>"
				"<title>Index of "+ location.getLocation_match() +"</title></head>"
				"<body>"
				"<h1>Index of "+ location.getLocation_match() +"</h1>"
				"<hr><pre>";

    for (it = files.begin(); it != files.end(); ++it)
    {
        index << "<a href='"+it->first+"'>"+it->first+"</a>";
        index << std::setw(40 - it->first.length()) << make_time(it->second.first);
        index << std::setw(20) << std::to_string(it->second.second) << std::endl;
    }

    index <<	"</pre><hr></body></html>";
    std::fstream file;
    file.open(tmp_path, std::fstream::app);
    file << index.str();
    file.close();
}


void Response::autoindex_handler(RequestParser &request)
{
    std::multimap<std::string, std::pair<struct stat, long long> > files;
    std::string path = request.get_url().replace(0, location.getLocation_match().length(), location.getRoot() + "/");
    DIR *dir;
    std::string slash;
    struct dirent *entry;
    if ((dir = opendir(path.c_str())) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            std::string filename = path + "/" + std::string(entry->d_name);
            stat(filename.c_str(), &file_stat);
			if (file_stat.st_mode &S_IFDIR)
                slash = "/";
			else
				slash = "";
            files.insert(std::make_pair(std::string(entry->d_name) + slash, std::make_pair(file_stat, file_stat.st_size)));
        }
        closedir(dir);
    }
    else
        throw "Сan not open directory";
    std::string tmp_path = request.create_file();
    auto_index_template(files, tmp_path);
    response_tmp = true;
    status_code = "200";
    body_path = tmp_path;
    setHeaders("Content-Type", "text/html");
    throw "Autoindex was received successfully";
}

void Response::check_index(RequestParser &request)
{
    std::vector<std::string> index_list = location.getIndex();
    for (size_t i = 0; i < index_list.size(); i++)
    {
        std::string path;
        struct stat buf;

        path = request.get_url().replace(0, location.getLocation_match().length(), location.getRoot() + "/") + index_list[i];
        if (stat(path.c_str(), &buf) == 0)
        {
            if (buf.st_mode &S_IFDIR)
            {
                status_code = "501";
                buildResponse(request, 501);
                throw "Index is directory";
            }
            else if (buf.st_mode &S_IFREG)
            {
                if (permission(path, "r"))
                {
                    if (!(location.getCgiPath().empty()))
                    {
//                        std::cout << "1 - const CGI\n";
                        request.cgi_ptr = new CGI();
                        request.cgi_ptr->cgi(request, location.getCgiPath().c_str(), path.c_str());
                        throw "Find cgi";
                    }
                    status_code = "200";
                    set_content_type(path);
                    body_path = path;
                    throw "Index was received successfully";
                }
            }
        }
    }
    if (location.getAutoIndex())
        autoindex_handler(request);
    else
    {
        status_code = "403";
        buildResponse(request, 403);
        throw "Index have an issue";
    }
}

void Response::check_default_index(std::string &path, RequestParser &request)
{
    std::string index_path = path + "index.html";
    struct stat buf;
    if (stat(index_path.c_str(), &buf) == 0)
    {
        if (!permission(index_path, "r"))
        {
            status_code = "403";
            buildResponse(request, 403);
            throw "Default index.html have no permissions";
        }
        status_code = "200";
        body_path = index_path;
        setHeaders("Content-Type","text/html");
        throw "Default index was received successfully";
    }
}

void Response::method_get(RequestParser &request)
{
    std::string root = location.getRoot() + "/";
    std::string full_path = request.get_url().replace(0, location.getLocation_match().length(), root);
    try
    {
        validity_check(full_path, request);
    }
    catch (const char *msg)
    {
        throw msg;
    }
    stat(full_path.c_str(), &file_stat);
    if (file_stat.st_mode &S_IFDIR)
    {
        if (!permission(full_path, "x"))
        {
            status_code = "403";
            buildResponse(request, 403);
            throw "Resource have no permissions";
        }
		directory_end_slash(request);
        search_for_location(request);
        if (location.getIndex().size() > 0)
        {
            try
            {
                check_index(request);
            }
            catch (const char *msg)
            {
                throw msg;
            }
        }
        else
        {
            check_default_index(full_path, request);
            if (location.getAutoIndex())
                autoindex_handler(request);
            else
            {
                status_code = "403";
                buildResponse(request, 403);
                throw "Index have an issue";
            }
        }
    }
    else if (file_stat.st_mode &S_IFREG)
    {
        if (!permission(full_path, "r"))
        {
            status_code = "403";
            buildResponse(request, 403);
            throw "Resource have no permissions";
        }
        if(!location.getCgiPath().empty())
        {
            request.cgi_ptr = new CGI();
//            std::cout << "2 - const CGI\n";
            request.cgi_ptr->cgi(request, location.getCgiPath().c_str(), full_path.c_str());
            throw "Calling cgi";
        }
        status_code = "200";
        body_path = full_path;
        set_content_type(full_path);
        throw "Resource was received successfully";
    }
}

void Response::check_cgi(std::string &path, RequestParser &request)
{
    std::string full_path = path;
    struct stat buf;
    if (stat(full_path.c_str(), &buf) == 0)
    {
        if (buf.st_mode &S_IFDIR)
        {
            if (location.getIndex().size() > 0)
            {
                std::vector <std::string> index_list = location.getIndex();
                for (size_t i = 0; i < index_list.size(); i++)
                {
                    std::string pat;
                    struct stat buf;

                    pat = request.get_url().replace(0, location.getLocation_match().length(),
                                                    location.getRoot() + "/") + index_list[i];
                    if (stat(pat.c_str(), &buf) == 0)
                    {
                        if (buf.st_mode &S_IFREG)
                        {
                            if (!location.getCgiPath().empty())
                                full_path = pat;
                        }
                    }
                }
            }
        }
        else
        {
            struct stat buf;
            std::string default_path = request.get_url().replace(0, location.getLocation_match().length(),
                                                                 location.getRoot() + "/") + "index.html";
            if (stat(default_path.c_str(), &buf) == 0)
                full_path = default_path;
        }
    }
    request.cgi_ptr = new CGI();
//    std::cout << "3 - const CGI\n";
    request.cgi_ptr->cgi(request, location.getCgiPath().c_str(), full_path.c_str()); //auf
    throw "Calling cgi";
}

void Response::method_post(RequestParser &request)
{
    std::string root = location.getRoot() + "/";
    std::string full_path = request.get_url().replace(0, location.getLocation_match().length(), root);
    if (!location.getCgiPath().empty())
        check_cgi(full_path, request);
    if (!location.getUploadPath().empty()) //else if
    {
        int end = request.get_url().length() - 1;
        struct stat buf;
        if ((request.get_url().at(end) == '/') || ((stat(full_path.c_str(), &buf) == 0) && (buf.st_mode & S_IFDIR)))
        {
            status_code = "500";
            buildResponse(request, 500);
            throw "Internal server error";
        }
        full_path = location.getUploadPath() + request.get_url().replace(0, location.getLocation_match().length(), "");
        if (stat(full_path.c_str(), &buf) == 0)
        {
            status_code = "409";
            buildResponse(request, 409);
            throw "Can not upload this resource already exist";
        }
        else
        {
            size_t pos = full_path.find_last_of('/');
            std::string dir = full_path.substr(0, pos);
            int stat_res = stat(dir.c_str(), &file_stat);
            std::string file_tmp = request.get_filepath();
            int ret = 1;

            if (stat_res != 0)
            {
                int flag = 0;
                for (size_t i = 0; i < dir.length(); i++)
                {
                    if (dir[i] == '/')
                    {
                        dir[i] = '\0';
                        if (mkdir(dir.c_str(), 0777) != 0)
                            flag = 1;
                        else
                            flag = 0;
                        dir[i] = '/';
                    }
                }
                if (mkdir(dir.c_str(), 0777) != 0)
                    flag = 1;
                else
                    flag = 0;
                if (flag)
                    ret = 0;
            }
            else if (stat_res == 0)
            {
                if (file_stat.st_mode & S_IFDIR)
                {
                    if (file_stat.st_mode & S_IWUSR)
                        ret = 1;
                    else
                        ret = 0;
                }
                else
                    ret = 0;
            }
            if (ret == 1)
            {
                int err = rename(file_tmp.c_str(), full_path.c_str());
                if (err)
                {
                    status_code = "500";
                    buildResponse(request, 500);
                    throw "Internal server error rename failed";
                }
                else
                {
                    status_code = "201";
                    body_path.clear();
                    setHeaders("Content-Lenght", "0");
                    response_tmp = false;
                }
            }
            else
            {
                status_code = "500";
                buildResponse(request, 500);
                throw "Internal server error";
            }
        }
    }
    else
    {
        status_code = "403";
        buildResponse(request, 403);
        throw "Сan not upload";
    }
}

void Response::end_slash_delete(RequestParser &request)
{
    int end = request.get_url().length() - 1;
    if (request.get_url().at(end) != '/')
    {
        status_code = "409";
        buildResponse(request, 409);
        throw "Conflict";
    }
}

int Response::remove_dir(std::string &path)
{
    DIR *dir = opendir(path.c_str());
    int r1 = -1;

    if (dir)
    {
        struct dirent *file;
        r1 = 0;
        while ((file = readdir(dir)) && !r1)
        {
            std::string buf;
            int r2 = -1;

            if (!strcmp(file->d_name, "./") || !strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
                continue;
            struct stat statbuf;
            buf = path + "/" + std::string(file->d_name);
            if (stat(buf.c_str(), &statbuf) == 0)
            {
                if (statbuf.st_mode &S_IFDIR)
                    r2 = remove_dir(buf);
                else
                    r2 = unlink(buf.c_str());
            }
            r1 = r2;
        }
        closedir(dir);
    }
    if (!r1)
        r1 = rmdir(path.c_str());
    return (r1);
}

void Response::method_delete(RequestParser &request)
{
    std::string root = location.getRoot() + "/";
    std::string full_path = request.get_url().replace(0, location.getLocation_match().length(), root);
    try
    {
        validity_check(full_path, request);
    }
    catch (const char *msg)
    {
        throw msg;
    }
	stat(full_path.c_str(), &file_stat);
    if (file_stat.st_mode &S_IFDIR)
    {
        if (!permission(full_path, "x"))
        {
            status_code = "403";
            buildResponse(request, 403);
            throw "Resource have no permissions";
        }
        end_slash_delete(request);
        search_for_location(request);
        if(!location.getCgiPath().empty())
        {
            check_cgi(full_path, request);
        }
        else
		{
            if (!(remove_dir(full_path)))
            {
                status_code = "204";
                body_path.clear();
                throw "Deleted successfully";
            }
            else
            {
                if (!permission(full_path, "x") || !permission(full_path, "w"))
                {
                    status_code = "403";
                    buildResponse(request, 403);
                    throw "Resource have no permissions";
                }
                else
                {
                    status_code = "500";
                    buildResponse(request, 500);
                    throw "Internal server error remove failed";
                }
            }
        }
    }
    else if (file_stat.st_mode &S_IFREG)
    {
        if(!location.getCgiPath().empty())
        {
//            std::cout << "3 - const CGI\n";
            request.cgi_ptr = new CGI();
            request.cgi_ptr->cgi(request, location.getCgiPath().c_str(), full_path.c_str());
            throw "Calling cgi";
        }
        else
        {
            if (!(remove(full_path.c_str())))
            {
                status_code = "204";
                body_path.clear();
                throw "Deleted successfully";
            }
            else
            {
                status_code = "500";
                buildResponse(request, 500);
                throw "Internal server error remove failed";
            }
        }
    }
}

void Response::methods(RequestParser &request)
{
    std::string request_method = request.get_method();
    if (request_method == "GET")
        method_get(request);
    else if (request_method == "POST")
        method_post(request);
    else if (request_method == "DELETE")
        method_delete(request);
}

void Response::set_date_header()
{
    time_t time_now;
    struct tm *tm;
    std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    std::string day[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    std::stringstream date;
    time(&time_now);
    tm = gmtime(&time_now);

    date << day[tm->tm_wday].c_str() << ", " << std::setw(2) << std::setfill('0') << tm->tm_mday << " ";
    date << months[tm->tm_mon].c_str() << " " << tm->tm_year + 1900 << " ";
    date << std::setw(2) << std::setfill('0') << (tm->tm_hour + 3) % 24 << ":";
    date << std::setw(2) << std::setfill('0') << tm->tm_min << ":";
    date << std::setw(2) << std::setfill('0') << tm->tm_sec << " GMT+3";
    setHeaders("Date", date.str());
}

long long Response::file_size(std::string &body_path)
{
    if (!body_path.empty() && body_path.length() > 0)
    {
        struct stat buf;
        stat(body_path.c_str(), &buf);
        return (buf.st_size);
    }
    return (0);
}

void Response::set_content_length(std::string &body_path)
{
    if (!body_path.empty() && body_path.length() > 0)
        setHeaders("Content-Length", std::to_string(file_size(body_path)));
}

std::string Response::make_msg(std::string &status_code)
{
    if (Config::statusCodeMessages.find(status_code) != Config::statusCodeMessages.end())
        return (Config::statusCodeMessages.find(status_code)->second);
    return "";
}

std::string Response::make_header()
{
    std::stringstream header;
    std::map<std::string, std::string>::iterator it;

    header << "HTTP/1.1 " << status_code << " " << make_msg(status_code) << "\r\n";
    for (it = headers_list.begin(); it != headers_list.end(); ++it)
        header << it->first << ": " << it->second << "\r\n";
    header << "\r\n";

    return (header.str());
}

std::string Response::getHeaders(RequestParser &request, Location &location, std::string &status)
{
    int status_int;
    this->location = location;
    status_code = status;

    std::istringstream(status) >> status_int;
    if (status_int >= 400 && status != "-1")
    {
        buildResponse(request, status_int);
    }
    else
    {
        try
        {
            choose_location(request);
            check_redirection(request);
            check_methods(request);
            methods(request);
        }
        catch (const char *msg)
        {
            std::cout << "Log : ["<< msg <<"]" <<std::endl;
        }
    }
    set_date_header();
    set_content_length(body_path);
    return (make_header());
}




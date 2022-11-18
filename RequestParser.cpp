
#include "RequestParser.hpp"

RequestParser::RequestParser()
{
	method = "";
	url = "";
	tmp_url = "";
	version = "HTTP/1.1";
	query = "";
	host_ip = "127.0.0.1";
	host_port = 0;
	content_type = "";
	content_lenght = 0;
	body = "";
	filepath = "";
	parsed = false;
    cgi_ptr = nullptr;
}

RequestParser::~RequestParser() {
//    std::cout << "deleteReqPar\n";
//    delete cgi_ptr;
}

void RequestParser::check_fl_data()
{
    if (method != "GET" && method != "POST" && method != "DELETE")
        throw "405";
    std::string right_url = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.~!*'();:@&=+$,/?%#[]";
    for (size_t i = 0; i < url.size(); i++)
    {
        if (right_url.find(url[i]) == std::string::npos)
            throw "409";
        if (url.size() > 2048)
            throw "414";
    }
    if (version != "HTTP/1.1")
        throw "505";
}

void RequestParser::parse_first_line(const std::string &str)
{
    std::string line = str;
    size_t pos;

    pos = line.find(" ");
    method = line.substr(0, pos);
    line.erase(0, pos + 1);
    pos = line.find(" ");
    url = line.substr(0, pos);
    line.erase(0, pos + 1);
    if (size_t pos = url.find("?") != std::string::npos)
    {
        query = url.substr(url.find("?") + 1, url.size());
        url.erase(url.find("?"), url.size());
    }
    tmp_url = url;
    pos = line.find(" ");
    version = line.substr(0, pos);
    line.erase(0, pos + 1);
    check_fl_data();
}

void RequestParser::parse_header(const std::string &str, int *parse)
{
    size_t pos;
    size_t pos_for_map;
    int first = 0;
    std::string line = str;
    std::string tmp;

    while ((pos = line.find("\r\n")) != std::string::npos)
    {
        tmp = line.substr(0, pos);
        if (!(first))
        {
            first = 1;
            parse_first_line(tmp);
        }
        else
        {
            if (tmp == "\0")
            {
                *parse = 1;
                line.erase(0, pos + 2);
                body = line;
                break;
            }
            if ((pos_for_map = tmp.find(":")) != std::string::npos)
                headerMap.insert(std::make_pair(tmp.substr(0, pos_for_map), tmp.substr(pos_for_map + 2, tmp.size())));
        }
        line.erase(0, pos + 2);
    }
}

int RequestParser::get_location()
{
    size_t pos;
    std::string str = tmp_url;
    int num = server.getLocation().size();
    for (int i = 0; i < num; ++i)
    {
        if (server.getLocation()[i].getLocation_match() == tmp_url)
        {
            location = server.getLocation()[i];
            return (1);
        }
    }
    pos = str.find_last_of("/");
    if (pos != std::string::npos)
        str.erase(pos, std::string::npos);
    tmp_url = str;
    if (pos != 0 && pos != std::string::npos)
        get_location();
    return (0);
}

void RequestParser::get_server_location(Config &config)
{
    try
    {
        server = config.getServer(host_port, host_ip);
    }
    catch (std::exception &error)
    {
        std::cout << error.what() << std::endl;
    }
    get_location();
}

std::string RequestParser::create_file()
{
	struct stat buf;
	struct stat str;
	if (stat("./tmp", &buf) == -1)
		mkdir("./tmp", 0777);
	char filename[] = "./tmp/tmpXXXXXXX";
	std::string new_filename(mktemp(filename));
	while (stat(new_filename.c_str(), &str) == 0)
		new_filename = mktemp(filename);
	std::fstream file(new_filename, std::fstream::in | std::fstream::out | std::fstream::app);
	file.close();
	return (new_filename);
}

void RequestParser::parse_header_lines(Config &config)
{
    if (headerMap.find("Host") == headerMap.end())
        throw "400";
    host_ip = headerMap["Host"].substr(0, headerMap["Host"].find(":"));
    if (headerMap["Host"].find(":") != std::string::npos)
        host_port = atoi(headerMap["Host"].substr(headerMap["Host"].find(":") + 1, headerMap["Host"].size()).c_str());
    content_lenght = atoll(headerMap["Content-Length"].c_str());
	if (method == "GET" || method == "DELETE")
		content_lenght = 0;
    get_server_location(config);
    if (headerMap.find("Transfer-Encoding") != headerMap.end() && headerMap["Transfer-Encoding"] != "chunked")
        throw "501";
    if ((headerMap.find("Transfer-Encoding") == headerMap.end()) && (headerMap.find("Content-Length") == headerMap.end()) && method == "POST")
        throw "411";
	if ((content_lenght > (long long)location.getClientMaxBodySize() && method != "GET") || content_lenght < 0)
		throw "413";
    if (headerMap.find("Content-Length") != headerMap.end() && content_lenght != 0)
    	filepath = create_file();
	parsed = true;
}

void RequestParser::write_body()
{
	if (body.size() > 0 && method == "POST")
	{
		std::fstream file;

		file.open(filepath, std::fstream::app);
		file << body;
		file.close();
		body = "";
	}
}

bool RequestParser::parse_chunks(const std::string &str, Config config)
{
    int parse = 0;

    parse_header(str, &parse);
    if (parse == 1)
        parse_header_lines(config);
	if (parsed)
		write_body();
	if (parsed)
		return (true);
	return (false);
}

void RequestParser::clear()
{
    headerMap.clear();
    method = "";
    url = "";
    tmp_url = "";
    version = "";
    query = "";
    host_ip = "127.0.0.1";
    host_port = 0;
    content_type = "";
    body = "";
    parsed = false;
    remove(filepath.c_str());
	filepath = "";
}

void RequestParser::setEnvp(char **Envp)
{
	envp = Envp;
}

char **RequestParser::getEnvp() {
	return envp;
}



#include "Request.hpp"

Request::Request(int fd)
{
    __fd = fd;
    __request = "";
    __response = "";
}

Request::Request()
{
    __fd = 0;
    __request = "";
    __response = "";
}

Request::Request(const Request &x) {*this = x;}

Request &Request::operator=(const Request &x)
{
    if (this != &x)
    {
        __fd = x.__fd;
        __request = x.__request;
        __response = x.__response;
    }
    return (*this);
}

Request::~Request(){}

int Request::getFd() const {return __fd;}

void Request::setRequest(const std::string &request)	{__request = __request + request;}

std::string Request::getResponse()	const {return __response;}

std::string Request::getRequest() const
{
    return __request;
}

void Request::setResponse(const std::string &response)	{__response = response;}

bool Request::PreParsing(const std::string &preRequest) //обсудим
{
	if (preRequest.empty())
		return false;
    return true;
}

void Request::Parse(Config &config, char **Envp)
{
	request_class.setEnvp(Envp);
    bool check;
    std::string code_error;
    std::string	status = "-1";
    try
    {
        check = request_class.parse_chunks(__request, config);
    }
    catch (const char *code)
    {
		status = std::string(code);
		remove(request_class.get_filepath().c_str());
        check = true;
    }
    if (check)
    {
        location = request_class.getLocation();
        buffer = response_class.getHeaders(request_class, location, status);
        body_inf = response_class.getBody();
        if (body_inf.first.size() > 0)
            file.open(body_inf.first);
//        std::cout << "D1";
        write_response();
    }
}

void Request::write_response()
{
    int	    bytes_read;
    char    buf[SIZE_BUFFER];
//    std::cout << "C0";
    if (request_class.cgi_ptr != nullptr)
    {
//        std::cout << "C1";
        if(!request_class.cgi_ptr->is_finished(*this))
        {
//            std::cout << "C2";
            delete request_class.cgi_ptr;
            request_class.cgi_ptr = nullptr;
            return;
        }
    }
    if (file.is_open())
	{
		while (!(file.eof()))
		{
			file.read(buf, SIZE_BUFFER);
			bytes_read = file.gcount();
			buffer += std::string(buf, bytes_read);
		}
	}
	__response = buffer;
	file.close();
	if (body_inf.second)
	{
		if (remove(body_inf.first.c_str()))
			perror("remove() failed: ");
	}
	__request.clear();
	buffer.clear();
	request_class.clear();
}

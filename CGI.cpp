
#include "CGI.hpp"
#include "Request.hpp"
#include <stdlib.h>

CGI::CGI()
{
//    fileName = "test_cgi";
    _cgi = -1;
    _fd = -1;
    _status = "200";
//    std::cout << "constr CGI\n";
}

CGI::~CGI()
{
    char temp[] = "/tmp/testXXXXXX";
//    unlink(temp);
//    std::cout << "delete CGI\n";
//	fclose(file);
}

void CGI::cgi(RequestParser request, std::string cgiPath, std::string script_path)
{
//    std::cout << "B1\n";

    char *args[3];
	args[0] = (char *)cgiPath.data();
	args[1] = (char *)script_path.data();
	args[2] = NULL;

//	FILE *temp = tmpfile();
//    char temp[] = "/tmp/testXXXXXX";
//    _fd = mkstemp(temp);
//    _fd = open("FILE_CGI.txt", O_RDONLY | O_CREAT | O_TRUNC);
//    int pip[2];
//    if(!pipe(pip))
//        exit(333);
    pipe(pip);
	pid_t pid;
	if((pid = fork()) == -1)
        exit(22);
	if(pid == 0)
	{
//        std::cout << "B2\n";
        dup2(pip[1], 1);
        close(pip[1]);
        close(pip[0]);
//        write()
//        std::cout << args[0] << " ar 0 \n";
		if (execve(args[0], args, request.getEnvp()) == -1)
            exit(111);
	}
	else
	{
        _cgi = 1;
        int res;
        waitpid(pid, &res, 0); //check res on error return 500;
        if(WIFEXITED(res) != 0)
            _status = "500"; //return 500 error

	}
}

void CGI::cgi_error(Request &client)
{
    std::string response = "<html><head><title> 500 </title></head><body><center><h1> 500 Internal Server Error:( </h1></center><hr><center>Webserv/1.1</center></body></html>";
    client.setResponse(response);
}

void CGI::craft_response(Request &client)
{
    if(_status == "500")
    {
        cgi_error(client);
        return;
    }

    char s1[1000];
    size_t readBytes = read(pip[0], s1, 1000);
    close(pip[1]);
    close(pip[0]);
	std::string response (
			"HTTP/1.1 200 OK\r\n"
//            "Content-Length: 50"
            "\r\n\r\n"
			"<html>"
			"<head>"
			"</head>"
			"<body>");
			response += std::string(s1, readBytes);
			response +="</body></html>";
	client.setResponse(response);
}

std::string CGI::getDateHeader()
{
	time_t curr_time;
	struct tm *tm;
	std::string days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	std::string months[] = {"Jan", "Feb", "MAar", "Apr", "May", "Jun",
							"Jul", "Aug", "Sep",  "Oct", "Nov", "Ded"};
	// Mon, 06 Jun 2022 03:48:42 GMT
	time(&curr_time);
	tm = gmtime(&curr_time);
	std::stringstream date;
	date << days[tm->tm_wday].c_str() << ", " << std::setw(2) << std::setfill('0')
		 << tm->tm_mday;
	date << " " << months[tm->tm_mon].c_str() << " " << tm->tm_year + 1900 << " ";
	date << std::setw(2) << std::setfill('0') << (tm->tm_hour + 1) % 24 << ":"
		 << std::setw(2) << std::setfill('0') << tm->tm_min << ":" << std::setw(2)
		 << std::setfill('0') << tm->tm_sec << " GMT+1";
	return ("Date: " + date.str() + "\r\n");
}

bool CGI::is_finished(Request &client) {
    if(_cgi == 1)
    {
        craft_response(client);
        _cgi = -1;
        return false;
    }
    else
    {
        return true;
    }
//    delete this;
//	return true;
}

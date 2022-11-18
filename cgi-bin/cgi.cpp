/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhalidy <akhalidy@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/14 14:45:00 by akhalidy          #+#    #+#             */
/*   Updated: 2022/06/30 15:15:12 by akhalidy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/cgi.hpp"
#include <cstdio>
#include <string>
#include <unistd.h>
#include <vector>

CGI::CGI(void) : finished(false) {
  char filename[] = "/tmp/tmp_cgi_XXXXXX";
  file = mktemp(filename);
//   std::cout << file << std::endl;
}

void CGI::set_env_map(const Request &request, const char *script_path) {

  std::string method = request.getMethod();

  if (request.getContentLenth() && method != "GET") {
	_env["CONTENT_LENGTH"] = request.getHeaderMap()["Content-Length"];
	_env["CONTENT_TYPE"] = request.getHeaderMap()["Content-Type"];
  }
  _env["GATEWAY_INTERFACE"] = "CGI/1.1";
  _env["QUERY_STRING"] = request.getQuery();
  _env["REQUEST_METHOD"] = request.getMethod();
  _env["SCRIPT_FILENAME"] = script_path; //* script_path is the path of the file to be executed.
  _env["SERVER_SOFTWARE"] = "WEBSERV";
  _env["SERVER_PROTOCOL"] = request.getVersion();
  _env["REDIRECT_STATUS"] = "true";
}

char **CGI::set_envp(const std::vector<std::string> &cookies) {
	char **envp;
	int i = 0;
	int len = _env.size();
	std::string str;
	std::map<std::string, std::string>::iterator	it = _env.begin();
	std::vector<std::string>::const_iterator		it_vec = cookies.begin();
  
  envp = new char *[len + 1 + cookies.size()];
  while (it != _env.end()) {
	str = it->first + "=" + it->second;
	envp[i] = strdup(str.c_str());
	i++;
	it++;
  }
  while (it_vec != cookies.end())
  {
	str = std::string("HTTP_COOKIE") + "=" + *it_vec;
	// std::cerr << "cookie : " << str << std::endl;
	envp[i] = strdup(str.c_str());
	i++;
	it_vec++;
  }
  envp[i] = NULL;
  return (envp);
}

bool CGI::execute(char **args, const Request &request) {
  int out;
  int in = 0;
  const char *post_body = request.getFilePath().c_str();
  _pid = fork();
  if (_pid == -1) {
	std::cerr << "fork failed !" << std::endl;
	_pid = 1;
	return false;
  }
  if (_pid == 0) {
	if (post_body && *post_body)
	  in = open(post_body, O_RDONLY);
	out = open(file.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (in) {
	  dup2(in, 0);
	  close(in);
	}
	dup2(out, 1);
	set_env_map(request, args[1]);
	if (execve(args[0], args, set_envp(request.getCookies())) == -1) {
	  std::cerr << "execve failed !" << std::endl;
	  exit(111);
	}
  }
  return true;
}

int CGI::cgi(const Request &request, const char *cgi_path,
			 const char *script_path) {
	char *args[3];

	args[0] = (char *)cgi_path;
	args[1] = (char *)script_path;
	args[2] = NULL;
  //TODO
//   std::cerr << GREEN << "Make it here! " << RESET << std::endl;
	if (execute(args, request))
		return true;
	return false;
}

void cgi_internal_error(Client &client, std::string status) {
  ws::Response response;

//   delete client.request.cgi_ptr;
//   client.request.cgi_ptr = NULL;
	client.file.close();
  Location location = client.request.getLocation();
  client.buffer = response.getHeaders(client.request, location, status);
  client.body_inf = response.getbody();
  if (client.body_inf.first.size() > 0)
	client.file.open(client.body_inf.first);
}

bool	CGI::is_finished(Client &client) {
	int pid;
	int status;
	if (finished)
		return true;
	pid = waitpid(_pid, &status, WNOHANG);
	if (pid == 0)
		return false;
	if (pid > 0)
	{
		if (WIFSIGNALED(status))
		{
			if (WTERMSIG(status) == SIGKILL)
			{
				finished = true;
				cgi_internal_error(client, "504");
				return true;
			}
		}
	}
	if (pid == -1 || !WIFEXITED(status) || WEXITSTATUS(status) == 111) {
		finished = true;
		cgi_internal_error(client, "502");
		return true;
	}
	finished = true;
	// call a function that will open the tmp file as client.file and read all the
	// headers and craft a header response and put it in the
	//  header response and put it in the client.buffer
	//   std::cerr << "Crafting response\n";
	craft_response(client);
	return true;
}

void CGI::craft_response(Client &client)
{
	std::string line;

	client.buffer = "Server: WebServ/1.0\r\n" + getDateHeader();
	_status = "200";
	client.file.open(file);

	for (; std::getline(client.file, line);)
	{
		if (line.empty() || line == "\r")
			break;
		if (strncasecmp("Status:", line.c_str(), 7) == 0)
		{
			std::string::iterator it = line.begin() + 7;
			while (*it == ' ')
				++it;
			_status = std::string(it, std::find(it, line.end(), ' '));
		}
		else
		{
			client.buffer += line + "\n";
		}
	}
	if (line.empty())
	{
		cgi_internal_error(client, "502");
		return;
	}
	//* I may need to check if I have content type header or not here!
	struct stat st;
	if (stat(this->file.c_str(), &st) == -1)
		return cgi_internal_error(client, "500");
	//*   std::cerr << st.st_size - client.file.tellg() << std::endl;
	std::stringstream header;
	//1- first line : HTTP/1.1 status msg
	header << "HTTP/1.1 " << _status << " " << ws::Response::getMessage(_status)
			<< "\r\n";
	//2- Server: WebServ/1.0\r\n + date + cgi header.
	header << client.buffer;
	//   std::cerr << GREEN << "Buffer : " << client.buffer << "\n Is python : " << is_python << RESET << std::endl;
	//   	header << "Content-Type: text/html" << "\r\n";
	//2- Content-Length.
	header << "Content-Length: " << st.st_size - client.file.tellg() << "\r\n";
	header << "\r\n";
	//TODO
	//   std::cerr << RED << " The headers : \n" <<  header.str() << RESET << std::endl;
	client.buffer = header.str();
}

std::string CGI::getDateHeader() {
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

int		CGI::get_pid()
{
	return (_pid);
}

CGI::~CGI(void)
{
	unlink(file.c_str());
}

//* staus cgi response :
// ?https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#server_error_responses
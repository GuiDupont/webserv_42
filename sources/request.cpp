/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gdupont <gdupont@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/13 14:06:41 by gdupont           #+#    #+#             */
/*   Updated: 2021/10/06 19:50:54 by gdupont          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"


std::ostream & operator<<(std::ostream & o,const request & r)
{
	o << "Methode: " << r.method << std::endl;
	o << "Request_target: " << r.request_target << std::endl;
	std::map<std::string, std::string>::const_iterator it = r.header_fields.begin();
	for (; it != r.header_fields.end(); it++)
	{
		o << "Header field : -" << it->first << "-" << std::endl;
		o << "Field content: -" << it->second << std::endl;
	}
	o << "Body: -" << r.body_request << "-" << std::endl;
	return (o);
}


request::request() : stage(0), next_chunk(-1), nb_trailer_to_received(0), code_to_send(0), close_csock(false), conf(NULL), resp(NULL), cgi(NULL), header_is_sent(false), body_is_sent(false), body_fd(0) {}

request::request(int csock) :	stage(0), csock(csock), next_chunk(-1), nb_trailer_to_received(0), 
								code_to_send(0), close_csock(false), conf(NULL), resp(NULL), cgi(NULL),
								header_is_sent(false), body_is_sent(false), body_fd(0) {}

request::request(int csock, std::string left) : stage(0), csock(csock), left(left), next_chunk(-1),
												nb_trailer_to_received(0), code_to_send(0), close_csock(false),
												conf(NULL), resp(NULL), cgi(NULL), header_is_sent(false), body_is_sent(false), body_fd(0) {}


void request::param_trailer(std::string str) {

	std::string::iterator it = str.begin();
	std::string::iterator it2;
	std::string::iterator it3;
	std::string substr;

	if (str.size() == 0)
		return ;
	for (; it != str.end(); it++) {
		for (; it != str.end() && std::isspace(*it) != 0; it++) {}
		if (str.find(',', it - str.begin()) != std::string::npos)
			it2 = str.begin() + str.find(',', it - str.begin());
		else
			it2 = str.end();
		substr = str.substr(it - str.begin(), it2 - it);
		it3 = it2 - 1;
		for (; it3 - it >= 0 && std::isspace(*it3) != 0; it3--) {}
		substr = substr.substr(0, it3 - it + 1);
		this->trailer.push_back(substr);
		it = (it2 == str.end() ? it2 - 1 : it2);
	}
	this->trailer.unique();
	this->nb_trailer_to_received = this->trailer.size();
	return ;
}

bool request::is_valid_chunk_size(std::string &str) {

	regex_t	regex;
	int 	reti;

	reti = regcomp(&regex, "^[0-9a-fA-F]\\{1,8\\}$", 0);
    reti = regexec(&regex, str.c_str(), 0, NULL, 0);
	if (reti) {
    	regfree(&regex);
		return (0);
    }
    regfree(&regex);
	return (1);
}

bool request::find_trailer_in_list(std::string str) {

	for (std::list<std::string>::iterator it = this->trailer.begin(); it != this->trailer.end(); it++) {
		if (str == *it)
			return (1);
	}
	return (0);
}

void	request::send_header(int csock, std::string & header) {
	send(csock, header.c_str(), header.size(), 0);
	std::cout << "Header sent: -" << header.c_str() << "-" << std::endl;
	g_logger.fd << g_logger.get_timestamp() << "We just sent an header" << std::endl;
}

void	request::send_body() {
	if (body_response.empty() == false)
		send_body_from_str();
	else if (method != "POST")
		send_body_from_file();
	else
		body_is_sent = true;
}

void	request::send_body_from_str() {
	int amount_sent;
	int to_send;
	for (int i = 10; i != 0; i--) {
		to_send = SEND_SPEED < body_response.size() ? SEND_SPEED : body_response.size();
		amount_sent = send(csock, body_response.c_str(), to_send, 0);
		std::cout << " We are sending from string : " << body_response.c_str() << std::endl;
		if (amount_sent == -1) {
			g_logger.fd << g_logger.get_timestamp() << "Issue while sending body on csock " << csock << ". Error: " << strerror(errno) << std::endl;
			body_is_sent = true;
			close_csock = true;
			return ;
		}
		else if (amount_sent != SEND_SPEED) {
			body_is_sent = true;
			break;
		}
		body_response = body_response.substr(to_send, body_response.size() - to_send);
	}
}

void	request::send_body_from_file() {
	int amount_read;
	int amount_sent;

	char buff[SEND_SPEED + 1];
	
	if (body_fd == 0)
		body_fd = open(conf->path_to_target.c_str(), O_RDONLY);
	if (body_fd == -1) {
		g_logger.fd << g_logger.get_timestamp() << "Issue while opening file on csock " << csock << ". Error: " << strerror(errno) << std::endl; // end special cases ?
		body_is_sent = true; // a check ce qu'on fait
		close_csock = true;
		return ;
	}
	g_logger.fd << g_logger.get_timestamp() << "We are sending : " << conf->path_to_target << "to csock : " << csock << std::endl;
	for (int i = 4; i != 0; i--) {
		amount_read = read(body_fd, buff, SEND_SPEED);
		buff[amount_read] = '\0';
		amount_sent = send(csock, buff, amount_read, 0);
		std::cout << " We are sending from file: " << buff << std::endl;
		if (amount_read != SEND_SPEED) {
			body_is_sent = true;
			g_logger.fd << g_logger.get_timestamp() << "We are done sending : " << conf->path_to_target << "to csock : " << csock << std::endl;
			break;
		}
	}
}

static int		unlink_or_rmdir(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
	std::string path(fpath);
	int status;

	if (is_directory(path))
		status = rmdir(fpath);
	else
		status = unlink(fpath);
	if (status != 0)
		g_logger.fd << g_logger.get_timestamp() << "An error occured while deleting -" << path << " errno: " << errno << " " << strerror(errno) << std::endl;
	return (status);
}
				
void	request::delete_directory(std::string & path, request & req) {
	std::cout << " We are about to delete " << path << std::endl;
	nftw(path.c_str(), unlink_or_rmdir, 30, FTW_DEPTH);
	req.code_to_send = 204;
	req.body_response = response::generate_error_body(g_webserv.status_code.find(req.code_to_send)->second);
	req.conf->local_actions_done = true;
}

void	request::delete_file(std::string & path, request & req) {
	std::cout << " We are about to delete " << path << std::endl;
	int status = unlink(path.c_str());
	
	req.code_to_send = 204;
	req.body_response = response::generate_error_body(g_webserv.status_code.find(req.code_to_send)->second);
	req.conf->local_actions_done = true;
}

request::~request() {
	delete conf;
	conf = NULL;
	delete resp;
	conf = NULL;
	delete cgi;
	cgi = NULL;
}

void request::initiate_CGI() {

	if (cgi == NULL) {
		cgi = new CGI();
		cgi->setCgi_stage("WRITEIN");
		cgi->param_CONTENT_LENGTH(*this);
		cgi->param_CONTENT_TYPE(*this);
		cgi->param_GATEWAY_INTERFACE();
		cgi->param_PATH_INFO(*this);
		cgi->param_PATH_TRANSLATED(*this);
		cgi->param_QUERY_STRING(*this);
		cgi->param_REMOTE_ADDR();
		cgi->param_REQUEST_METHOD(*this);
		cgi->param_SCRIPT_NAME(*this);
		cgi->param_SERVER_NAME();
		cgi->param_SERVER_PORT();
		cgi->param_SERVER_PROTOCOL();
		cgi->param_SERVER_SOFTWARE();
		if (method == "GET") {
			int ret = pipe(cgi->pipefd);
			if (ret == -1) {
				code_to_send = 500; // a voir/tester avec suite du programme
				conf->local_actions_done = true;
				return ;
			}
			cgi->pid = fork();
			if (cgi->pid == -1) {
				code_to_send = 500; // a voir/tester avec suite du programme
				conf->local_actions_done = true;
				return ;
			}
			if (cgi->pid == 0) //(fils)
			{
				close(cgi->pipefd[0]);
				dup2(cgi->pipefd[1], 1);
				close(cgi->pipefd[1]);
				char *arg2[] = {(char *)conf->_cgi_dir.c_str(), (char *)conf->path_to_target.c_str(), NULL};
				ret = execve(conf->_cgi_dir.c_str(), arg2, cgi->getenv());
				if (ret == -1)
					exit(1);
			}
			if (cgi->pid != 0) //(daron)
			{
				waitpid(cgi->pid, &cgi->pid_status, WNOHANG);
				if (WEXITSTATUS(cgi->pid_status) != 0) {
					code_to_send = 500; // a voir/tester avec suite du programme
					conf->local_actions_done = true;
					return ;
				}
				conf->local_actions_done = true;
				cgi->setCgi_stage("readfrom");
				return ;
			}
		}
		if (method == "POST") {

	}		
	return ;
}

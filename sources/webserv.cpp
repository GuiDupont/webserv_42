/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gdupont <gdupont@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 14:15:08 by gdupont           #+#    #+#             */
/*   Updated: 2021/09/06 18:18:36 by gdupont          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"
#include <string>
#include <fstream>
#include "../includes/exceptions.hpp"
#include <iostream>


webserv::webserv(const std::string & path_config) {
	std::ifstream	config_file;
	std::string		all_file;
	
	config_file.open(path_config, std::ios_base::in);
	if (!config_file.is_open())
		throw (config_file_not_open());
	std::getline(config_file, all_file, '\0');
	// if (!this->check_brackets(all_file))
		
	this->set_config(config_file);
}

void	webserv::set_config(std::ifstream & config_file) {
	std::string		line;
	std::string		first_word;
	
	config_file.clear();
	config_file.seekg(0);
		std::cout << "dalut" <<  std::endl;

	while (!config_file.eof())
	{
		std::getline(config_file, line, '\n');
		int i = 0;
		while (isspace(line[i])) 
			i++;
		first_word = line.substr(i, line.find_first_of(" \t\n\v\f\r", i) - i);
		if (first_word == "server")
			;
		else if (first_word == "client_max_body_size" || first_word == "error_page")
			;
		else if (first_word == "}")
			;
		else
		{
			if (first_word == "{")
				throw (wrong_brackets_conf());
			else
				throw (bad_directive());
		}
	}
}

/* 
	client_max_body_size
	error_page
	*/

bool	webserv::check_brackets(const std::string & config) {
	size_t 	position = 0;
	while (1) {
		position = config.find('{', position);
		if (position == std::string::npos)
			return (true);
		position = get_next_bracket(config, position + 1);
		if (position == std::string::npos)
			return (false);
	}
	return (true);
}

size_t	webserv::get_next_bracket(const std::string & config, size_t position) {
	size_t next_open_bracket = config.find('{', position);
	size_t next_closing_bracket = config.find('}', position);
	
	if (next_closing_bracket == std::string::npos)
		return (std::string::npos);

	
	if (next_open_bracket < next_closing_bracket)
	{
		position = get_next_bracket(config, next_open_bracket + 1);
		position = get_next_bracket(config, position + 1);
		if (position == std::string::npos)
			return (std::string::npos);
	}
	return (next_closing_bracket);
}

webserv::webserv(void) {
	
}

webserv::~webserv(void) {
	
}

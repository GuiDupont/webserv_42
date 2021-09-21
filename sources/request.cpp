/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gdupont <gdupont@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/13 14:06:41 by gdupont           #+#    #+#             */
/*   Updated: 2021/09/21 18:40:01 by gdupont          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"


std::ostream & operator<<(std::ostream & o,const request & r)
{
	o << "Methode: " << r._method << std::endl;
	o << "Request_target: " << r._request_target << std::endl;
	std::map<std::string, std::string>::const_iterator it = r._header_fields.begin();
	for (; it != r._header_fields.end(); it++)
	{
		std::cout << "Header field : -" << it->first << "-" << std::endl;
		std::cout << "Field content: -" << it->second << std::endl;
	}
	std::cout << "Body: -" << r._body << "-" << std::endl;
	return (o);
}

request::request() : stage(0) {}

request::request(int csock) : stage(0), _csock(csock) {}

request::request(int csock, std::string left) : stage(0), _csock(csock), _left(left) {}

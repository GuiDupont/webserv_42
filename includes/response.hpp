/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gdupont <gdupont@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/22 14:51:32 by gdupont           #+#    #+#             */
/*   Updated: 2021/09/24 18:30:44 by gdupont          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ANSWER_HPP
# define ANSWER_HPP

# include <map>
# include <string>
# include <sys/stat.h>
# include <dirent.h> 

# include "request.hpp"


class request;

class response {
    public:
        response(request & request);
    


    public:
        std::string     status_line;

    
    public:
    
        static std::string					get_allowed_functions_header(size_t & disabled_method);
        static std::string					get_last_time_modified_header(std::string path);
		static std::string					get_date_header();
		static std::string         			get_server_header();
		static std::string					get_content_length_header(int content_length);
		static std::string					get_return_location_header(std::string & return_location);

		static std::string					generate_header(request & request);
		static std::string					generate_error_body(std::string & message);
		static std::string					generate_autoindex_body(std::string path);
		static std::string         			add_special_header(request & req, std::string & header);

};

# endif
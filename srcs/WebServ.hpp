/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:39 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:40 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <sys/epoll.h>

# define DEFAULT_LOG_PATH "~/goinfre/Server.log"

# define DEB 1
# if DEB
#  define DEBUG(x) do {std::cerr << YELLOW << "DEBUG: " <<x<< RESET << std::flush;} while (0)
# else
#  define DEBUG(x) do {} while(0)
# endif

# define RESET "\033[0m"
# define RED "\033[0;31m"
# define GREEN "\033[0;32m"
# define YELLOW "\033[0;33m"
# define BLUE "\033[0;34m"
# define MAGENTA "\033[0;35m"
# define CYAN "\033[0;36m"
# define WHITE "\033[0;37m"
# define ORANGE "\033[0;91m"
# define PURPLE "\033[0;95m"
# define DARK_ORANGE "\033[0;33m"
# define PINK "\033[0;95m"
# define TEAL "\033[0;96m"
# define GRAY "\033[0;90m"


#define MAX_EXEC_TIME 5

typedef unsigned int t_uint;
typedef unsigned long t_ulong;

union CONFIG_FLAGS {
    uint8_t   flags;
    struct __attribute__((packed))
        {                                         // Bits  Description
        uint8_t  GET_METHOD		: 1;
        uint8_t  POST_METHOD	: 1;
        uint8_t  DELETE_METHOD	: 1;
        } flag;
};

#define SERV_RULE_NB 14

enum HttpRuleError
{
	NO_HTTP_RULE_ERROR	  = 0,
	NO_ROOT_PATH		  = 1,
	ROOT_NOT_DIR		  = 2,
};

enum HttpMethods
{
	GET		= 0,
	POST	= 1,
	DELETE	= 2
};


int		check_options(int ac, char **av);

#endif

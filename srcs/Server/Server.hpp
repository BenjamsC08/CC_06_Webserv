/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:25 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:26 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "Client.hpp"
#include "ServConf.hpp"

#define MAX_EVENTS 128

class Server{

    public:
        Server(const ServConf, int efd, std::map<int, Server *> &list_server);
        Server(const Server & src);
        ~Server();

        Server & operator=(const Server & src);

        void    socket_init(int efd, std::map<int, Server *> &list_server);
        void    newConnect(int epollFd, int servFD, std::map<int, Client *> & list_client);

        class SocketInitException : public std::exception {
		public:
			virtual const char *what() const throw();
	    };

        std::vector<std::string>        getServerNames();
        std::string                     getHostname();
        sockaddr_in                     getAddress();
        std::string                     getRoot();
        size_t                          getClientMaxBodySize();
        std::vector<int>                getListenFds();
        int                             getFdIndex(int index);
        std::vector<int>                getPorts();
        std::map<std::string, HttpRule>	&getHttpRules();
		const ServConf					&getConf() const;
        std::map<int, std::string>		getHttpErrors()	const;

    private:
        std::vector<std::string>            _serverNames;
        std::string                         _hostname;
        sockaddr_in                         _address;
        std::string                         _root;
        size_t                              _clientMaxBodySize;
        std::vector<int>                    _listenFds;
        std::vector<int>                    _portsVect;
        unsigned int                        _addrlen;
        ServConf                            _conf;
    	std::map<std::string, HttpRule>		_httpRules;
        std::map<int, std::string>          _httpErrors;

};

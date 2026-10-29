/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:21 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:56:54 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const ServConf conf, int efd, std::map<int, Server *> &list_server): _address(), _clientMaxBodySize(conf.getMaxBodySize()),
    _portsVect(conf.getPorts()), _conf(conf), _httpRules(conf.getHttpRules()), _httpErrors(conf.getErrors())
{
    this->socket_init(efd, list_server);
}

Server::Server(const Server & src):_serverNames(src._serverNames),
        _hostname(src._hostname),_address(src._address),_root(src._root),_clientMaxBodySize(src._clientMaxBodySize),
        _listenFds(src._listenFds),_portsVect(src._portsVect), _addrlen(src._addrlen), _conf(src._conf), _httpRules(src._httpRules), _httpErrors(src._httpErrors)
{}

Server & Server::operator=(const Server & src)
{
    if (this != &src)
    {
        _serverNames = src._serverNames;
        _hostname = src._hostname;
        _address = src._address;
        _root = src._root;
        _clientMaxBodySize = src._clientMaxBodySize;
        _listenFds = src._listenFds;
        _portsVect = src._portsVect;
        _addrlen = src._addrlen;
        _conf = src._conf;
		_httpRules = src._httpRules;
        _httpErrors = src._httpErrors;
    }
    return (*this);
}

Server::~Server()
{
}

void    Server::socket_init(int efd, std::map<int, Server *> &list_server)
{
    for (size_t i = 0; i < _portsVect.size(); i++)
    {
        int to_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (to_listen_fd < 0)
        {
            DEBUG("Error: Failed to create Socket for Port: " << _portsVect[i] << "\n");
            continue;
        }

        int optval = 1;
        setsockopt(to_listen_fd,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        if (fcntl(to_listen_fd, F_SETFL, O_NONBLOCK) < 0)
        {
            DEBUG("Error: Couldnt put in Non Block\n");
            close(to_listen_fd);
            continue;
        }

        _address.sin_family = AF_INET;
        _address.sin_addr.s_addr = INADDR_ANY;
        _address.sin_port = htons(_portsVect[i]);

        _addrlen = sizeof(sockaddr);


        if (bind(to_listen_fd,(struct sockaddr*) &_address, sizeof(_address)) < 0)
        {
            DEBUG("Error: Failed to create Bind\nPort " << _portsVect[i] << " Bind Errod: " << strerror(errno) << "\n");
            close(to_listen_fd);
            continue;
        }

        if (listen(to_listen_fd, 10) < 0)
        {
            DEBUG("Error: Failed to listen the Socket\nPort " << _portsVect[i] << "\n");
            close(to_listen_fd);
            continue;
        }

        _listenFds.push_back(to_listen_fd);
        DEBUG("Succesfully listening on Port: " << _portsVect[i] << "\n");

		epoll_event temp_event;
        memset(&temp_event, 0, sizeof(temp_event));
        temp_event.events = EPOLLIN;
        temp_event.data.fd = to_listen_fd;
        epoll_ctl(efd, EPOLL_CTL_ADD, to_listen_fd, &temp_event);
        list_server[to_listen_fd] = this;
    }
    if (_listenFds.empty())
        throw SocketInitException();
}

void    Server::newConnect(int epollFd, int servFD, std::map<int, Client *> & list_client)
{
    sockaddr_in tempAddr;
    memset(&tempAddr, 0, sizeof(tempAddr));

    unsigned int addrlen = sizeof(sockaddr);
    int connection = accept(servFD, (struct sockaddr*) &tempAddr, &addrlen);

    if (connection > -1)
    {
        Client *customer = new Client(*this);

        customer->setFd(connection);
        customer->setIp(inet_ntoa(tempAddr.sin_addr));
        if (fcntl(customer->getFd(), F_SETFL, O_NONBLOCK) < 0)
        {
            g_Log << "[w] Could not put Client in Non Block\n";
            close(connection);
            return;
        }

        epoll_event temp_event;
        temp_event.events = EPOLLIN;
        temp_event.data.fd = customer->getFd();
        epoll_ctl(epollFd, EPOLL_CTL_ADD, customer->getFd(), &temp_event);
        list_client[customer->getFd()] = customer;
    }
    else
        g_Log << "[w] Connection failed\n";
}



std::vector<std::string>    Server::getServerNames()
{
    return(_serverNames);
}

std::string                 Server::getHostname()
{
    return(_hostname);
}

sockaddr_in                 Server::getAddress()
{
    return(_address);
}

std::string                 Server::getRoot()
{
    return(_root);
}

size_t                      Server::getClientMaxBodySize()
{
    return(_clientMaxBodySize);
}

std::vector<int>            Server::getListenFds()
{
    return(_listenFds);
}

std::vector<int>            Server::getPorts()
{
    return(_portsVect);
}
int                         Server::getFdIndex(int index)
{
    return(_listenFds[index]);
}

std::map<std::string, HttpRule>	&Server::getHttpRules()
{
	return(_httpRules);
}

const ServConf	&Server::getConf() const
{
	return (this->_conf);
}

 
std::map<int, std::string>		Server::getHttpErrors()	const
{
    return (_httpErrors);
}

const char* Server::SocketInitException::what() const throw() 
{
    return  ("Socket initialization failed");
}

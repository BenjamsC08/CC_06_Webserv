/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:27 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:58:02 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "CgiProcess.hpp"
#include "Config.hpp"
#include "Server.hpp"

void	signal_handling(void);
void	multiplex_loop (std::map<int, Server *> &list_server, 
		std::map<int, Client *> &list_client, epoll_event event[MAX_EVENTS],
		int res, int &efd, t_CgiFdMap &list_cgi);
void	cgi_timeout_loop(std::map<int, Client *> &list_client, int efd,
		t_CgiFdMap &list_cgi);
void    client_timeout_loop(std::map<int, Client *> &list_client, int efd,
		t_CgiFdMap &list_cgi);
void	cgi_progress_loop(std::map<int, Client *> &list_client, int efd,
		t_CgiFdMap &list_cgi);
void	clean_listServ(std::map<int, Server *> &ls, int &efd);
void	clean_listClient(std::map<int, Client *> &lc, int &efd,
		t_CgiFdMap &cgiMap);

int main(int ac, char **av)
{
	std::string	configFile;

	if (ac == 1)
		return (1);
	else if (ac > 2)
	{
		if (check_options(ac, av) == EXIT_FAILURE)
			return (EXIT_FAILURE);
		for (int i=0; i < ac; i++)
		{
			std::string tmp = av[i];
			if (tmp.find(".config") != std::string::npos)
				configFile = tmp;
		}
	}
	else
		configFile = av[1];

	std::map<int, Client *> list_client;
	std::map<int, Server *> list_server;
	t_CgiFdMap				list_cgi;
	int efd = -1;
	std::vector<Server *> vectServ;

    try
	{
		Config conf(configFile);
		if (!conf.canRun())
		{
			std::cerr << "Config was not correct\n";
			return (1);
		}
		efd = epoll_create1(0);
        signal_handling();

        epoll_event event[MAX_EVENTS];

        for (size_t i = 0; i < conf.getAllServConf().size(); i++)
        {
            Server* serv = new Server(conf.getServConf(i), efd, list_server);
	    	vectServ.push_back(serv);
        }

        while (42)
        {
            int res = 0;

            res = epoll_wait(efd, event, MAX_EVENTS, 1000);
			multiplex_loop(list_server, list_client, event, res, efd, list_cgi);
			cgi_progress_loop(list_client, efd, list_cgi);
			cgi_timeout_loop(list_client, efd, list_cgi);
            client_timeout_loop(list_client, efd, list_cgi);
        }
    }
	catch (std::exception &err)
	{
		std::cerr << err.what() << '\n';
		if (!list_server.empty())
			clean_listServ(list_server, efd);
		if (!list_client.empty())
			clean_listClient(list_client, efd, list_cgi);
		if (!vectServ.empty())
		{
			for (size_t i = 0; i < vectServ.size(); i++)
				delete vectServ[i];
		}
		if (efd != -1)
			close(efd);
        return (1);
	}
    catch(int value)
    {
		if (value == 42)
		{
			clean_listServ(list_server, efd);
			clean_listClient(list_client, efd, list_cgi);
			for (size_t i = 0; i < vectServ.size(); i++)
				delete vectServ[i];
			close(efd);
		}
    }
	catch (t_CgiException &childErr)
	{ return (EXIT_FAILURE); }
	catch ( ... )
	{
		std::cerr << "Something thrown but unknown.\nJust in case server didn't start" << '\n';
        return (1);
	}
    return (0);
}


void	clean_listServ(std::map<int, Server *> &ls, int &efd)
{
	for (std::map<int, Server *>::iterator it = ls.begin(); it != ls.end(); it++)
	{
		epoll_ctl(efd, EPOLL_CTL_DEL, it->first, NULL);
		close(it->first);
	}
}

void	clean_listClient(std::map<int, Client *> &lc, int &efd,
		t_CgiFdMap &cgiMap)
{
	for (std::map<int, Client *>::iterator it = lc.begin(); it != lc.end(); it++)
	{
		it->second->cleanupCgi(efd, cgiMap);
		epoll_ctl(efd, EPOLL_CTL_DEL, it->first, NULL);
		close(it->first);
        delete it->second;
	}
}

void	multiplex_loop(std::map<int, Server *> &list_server,
		std::map<int, Client *> &list_client, epoll_event event[MAX_EVENTS],
		int res, int &efd, t_CgiFdMap &list_cgi)
{
	for (int i = 0; i < res; i++)
	{
		int	fd;

		fd = event[i].data.fd;

		std::map<int, Server *>::iterator serv_it;
		serv_it = list_server.find(fd);
		if (serv_it != list_server.end())
		{
			serv_it->second->newConnect(efd, serv_it->first, list_client);
			continue ;
		}

		std::map<int, t_CgiFdBinding>::iterator cgi_it;
		cgi_it = list_cgi.find(fd);
		if (cgi_it != list_cgi.end())
		{
			if (cgi_it->second.client)
				cgi_it->second.client->handleCgiEvent(efd, fd,
					event[i].events, cgi_it->second.role, list_cgi);
			continue ;
		}

		std::map<int, Client *>::iterator client_it;
		client_it = list_client.find(fd);
		if (client_it != list_client.end())
		{
			client_it->second->handleClientEvent(efd, event[i].events,
				list_client, list_cgi);

			std::map<int, Client *>::iterator check_it;
			check_it = list_client.find(fd);
			if (check_it != list_client.end()
				&& check_it->second->getKillSwitch())
				removeClient(efd, fd, list_client, list_cgi);
		}
	}
}

void client_timeout_loop(std::map<int, Client *> &list_client, int efd,
				t_CgiFdMap &list_cgi)
{
    std::map<int, Client *>::iterator iter;
    for (iter = list_client.begin(); iter != list_client.end();)
    {
        if (iter->second->isProcessingCgi())
		{
			iter++;
			continue ;
		}
		if ((time(NULL) - iter->second->getTimeoutCount() > 25) ||
			(iter->second->getRequest().getState() != FINISHED &&
			time(NULL) - iter->second->getRequest().getParsingTimer() > 30))
        {
            g_Log << "[i] Client " << iter->first << " has been inactive "
						"for too long.\n";
            std::map<int, Client *>::iterator it = iter++;
			it->second->timeoutClient(efd);
            (void) list_cgi;
        }
        else
            iter++;
    }
}

void	cgi_timeout_loop(std::map<int, Client *> &list_client, int efd,
			t_CgiFdMap &list_cgi)
{
	std::map<int, Client *>::iterator	it;
	Client								*client;

	it = list_client.begin();
	while (it != list_client.end())
	{
		client = it->second;
		++it;
		if (!client->isProcessingCgi())
			continue ;
		if (time(NULL) - client->getCgiStartTime() > MAX_EXEC_TIME)
		{
			g_Log << "[e] CGI execution exceeded " << MAX_EXEC_TIME
				<< " seconds. Returning 504.\n";
			client->timeoutCgi(efd, list_cgi);
		}
	}
}

void	cgi_progress_loop(std::map<int, Client *> &list_client, int efd,
						t_CgiFdMap &list_cgi)
{
	std::map<int, Client *>::iterator	it;

	it = list_client.begin();
	while (it != list_client.end())
	{
		Client	*client;

		client = it->second;
		++it;
		client->pollCgiProgress(efd, list_cgi);
	}
}

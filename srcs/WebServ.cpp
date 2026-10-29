/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:36 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:37 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "Logging.hpp"
#include "WebServ.hpp"

int usage_opt(char *prog)
{
	std::cerr << "Option usage: "<< prog << " [-v] [-L <path_to_log>]\n";
	return (EXIT_FAILURE);
}


int	check_options(int ac, char **av)
{
	int opt;
	while ((opt = getopt(ac, av, "L:v")) != -1)
	{
		switch (opt) {

			case 'v':
				g_Log.setConsole(false);
				break ;
			case 'L':
				if (*optarg == '-')
					return (usage_opt(av[0]));
				if (!g_Log.setFile(optarg))
				{
					g_Log << "[e] Cannot open `" << optarg << " log to ";
					g_Log << DEFAULT_LOG_PATH << '\n';
				}
				break ;
			default:
				return (usage_opt(av[0]));
		}
	}
	return (EXIT_SUCCESS);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiEpoll.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 03:37:20 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 02:00:56 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>

class Client;

enum t_CgiFdRole
{
				CGI_FD_STDIN,
				CGI_FD_STDOUT,
				CGI_FD_EXEC
};

typedef struct s_CgiFdBinding
{
	Client		*client;
	t_CgiFdRole	role;
}				t_CgiFdBinding;

typedef std::map<int, t_CgiFdBinding>	t_CgiFdMap;

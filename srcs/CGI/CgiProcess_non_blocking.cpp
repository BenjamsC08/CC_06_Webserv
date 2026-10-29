/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_non_blocking.cpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/03 08:43:54 by mkerrien          #+#    #+#             */
/*   Updated: 2026/04/25 19:36:32 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"
#include <fcntl.h>

bool	CgiProcess::setNonBlocking(int fd, t_CgiError error)
{
	int	flags = fcntl(fd, F_GETFL);

	if (flags == -1)
		return (this->setError(error));
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return (this->setError(error));
	return (true);
}

bool	CgiProcess::epollChildExit(void)
{
	pid_t	res = 0;
	int		status = 0;

	if (this->_runState.child_status_collected)
		return (this->updateCompletionState(), true);
	if (this->_runState.pid <= 0)
		return (false);
	
	res = waitpid(this->_runState.pid, &status, WNOHANG);
	if (res == 0)
		return (this->updateCompletionState(), true);
	if (res == this->_runState.pid)
	{
		this->decodeWaitStatus(status);
		this->updateCompletionState();
		return (true);
	}
	return (setError(CGI_ERR_WAITPID));
}

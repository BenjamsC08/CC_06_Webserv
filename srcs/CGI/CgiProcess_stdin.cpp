/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_stdin.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 04:49:45 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 01:59:38 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"
#include <unistd.h>

bool	CgiProcess::createStdinPipe(int (&pipe_fd)[2])
{
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	if (pipe(pipe_fd) == -1)
		return (this->setError(CGI_ERR_PIPE_STDIN));
	return (true);
}

bool	CgiProcess::writeChildStdin(void)
{
	ssize_t		res = 0;
	size_t		remaining = 0;
	const char	*data_ptr = NULL;

	if (this->_runState.stdin_closed)
		return (true);
	if (this->_runState.stdin_fd_parent == -1)
		return (this->setError(CGI_ERR_WRITE_STDIN));
	if (this->_launchSpec.stdin_data.empty()
		|| this->_runState.stdin_sent == this->_launchSpec.stdin_data.size())
	{
		this->closeIfOpen(this->_runState.stdin_fd_parent);
		this->_runState.stdin_closed = true;
		this->updateCompletionState();
		return (true);
	}

	remaining = this->_launchSpec.stdin_data.size()
		- this->_runState.stdin_sent;
	data_ptr = this->_launchSpec.stdin_data.c_str()
		+ this->_runState.stdin_sent;
	res = write(this->_runState.stdin_fd_parent, data_ptr, remaining);

	if (res > 0)
		this->_runState.stdin_sent += static_cast<size_t>(res);
	else if (res < 0)
	{
		this->closeIfOpen(this->_runState.stdin_fd_parent);
		return (this->setError(CGI_ERR_WRITE_STDIN));
	}
	if (this->_runState.stdin_sent == this->_launchSpec.stdin_data.size())
	{
		this->closeIfOpen(this->_runState.stdin_fd_parent);
		this->_runState.stdin_closed = true;
	}
	this->updateCompletionState();
	return (true);
}

bool	CgiProcess::closeChildStdin(void)
{
	if (this->_runState.stdin_closed)
		return (true);
	this->closeIfOpen(this->_runState.stdin_fd_parent);
	this->_runState.stdin_closed = true;
	this->updateCompletionState();
	return (true);
}

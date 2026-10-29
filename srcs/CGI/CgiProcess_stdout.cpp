/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_stdout.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 02:55:07 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 01:59:51 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"

bool	CgiProcess::createStdoutPipe(int (&pipe_fd)[2])
{
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	if (pipe(pipe_fd) == -1)
		return (this->setError(CGI_ERR_PIPE_STDOUT));
	return (true);
}

bool	CgiProcess::readChildStdout(void)
{
	char	buffer[4096];
	ssize_t	res = 0;

	if (this->_runState.stdout_eof)
		return (true);
	if (this->_runState.stdout_fd_parent == -1)
		return (this->setError(CGI_ERR_READ_STDOUT));
	res = read(this->_runState.stdout_fd_parent, buffer, sizeof(buffer));
	if (res > 0)
	{
		this->_runState.stdout_buffer.append(buffer, res);
		this->updateCompletionState();
		return (true);
	}
	if (res == 0)
	{
		this->_runState.stdout_eof = true;
		this->closeIfOpen(this->_runState.stdout_fd_parent);
		this->updateCompletionState();
		return (true);
	}
	this->closeIfOpen(this->_runState.stdout_fd_parent);
	return (this->setError(CGI_ERR_READ_STDOUT));
}

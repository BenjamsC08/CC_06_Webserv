/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_exec_fail_pipe.cpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/29 22:22:30 by mkerrien          #+#    #+#             */
/*   Updated: 2026/04/03 17:34:03 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

bool	CgiProcess::createExecFailurePipe(int (&pipe_fd)[2])
{
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	if (pipe(pipe_fd) == -1)
		return (this->setError(CGI_ERR_PIPE_EXEC));
	if (!this->setCloseOnExec(pipe_fd[1]))
	{
		this->closePipePair(pipe_fd);
		return (false);
	}
	return (true);
}

bool	CgiProcess::setCloseOnExec(int fd)
{
	int	flags = fcntl(fd, F_GETFD);

	if (flags == -1)
		return (this->setError(CGI_ERR_FCNTL_CLOEXEC));
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		return (this->setError(CGI_ERR_FCNTL_CLOEXEC));
	return (true);
}

bool	CgiProcess::writeChildSetupError(int fd, t_CgiError cgi_error,
										int sys_errno)
{
	t_CgiChildSetupError	msg;
	ssize_t					res = 0;

	msg.cgi_error = static_cast<int>(cgi_error);
	msg.sys_errno = sys_errno;

	res = write(fd, &msg, sizeof(msg));	
	if (res != static_cast<ssize_t>(sizeof(msg)))
		return (/*this->setError(CGI_ERR_WRITE_EXEC_PIPE)*/false);
	return (true);
}

bool	CgiProcess::readChildSetupError(int fd, t_CgiChildSetupError &error_msg, bool &has_error)
{
	ssize_t	res;

	has_error = false;
	error_msg.cgi_error = 0;
	error_msg.sys_errno = 0;
	res = read(fd, &error_msg, sizeof(error_msg));
	if (res == 0)
		return (true);
	if (res == static_cast<ssize_t>(sizeof(error_msg)))
	{
		has_error = true;
		return (true);
	}
	return (this->setError(CGI_ERR_READ_EXEC_PIPE));
}

bool	CgiProcess::checkExecFailurePipe(void)
{
	t_CgiChildSetupError	error_msg;
	bool					has_error = false;

	if (this->_runState.exec_setup_checked)
		return (true);
	if (this->_runState.exec_fd_parent == -1)
		return (this->setError(CGI_ERR_READ_EXEC_PIPE));

	if (!this->readChildSetupError(this->_runState.exec_fd_parent,
			error_msg, has_error))
	{
		this->closeIfOpen(this->_runState.exec_fd_parent);
		this->_runState.exec_setup_checked = true;
		return (false);
	}
	this->closeIfOpen(this->_runState.exec_fd_parent);
	this->_runState.exec_setup_checked = true;

	if (has_error)
	{
		this->_runState.child_setup_errno = error_msg.sys_errno;
		this->_runState.child_setup_failed_before_exec = true;
		this->updateCompletionState();
		return (this->setError(static_cast<t_CgiError>(error_msg.cgi_error)));
	}
	this->updateCompletionState();
	return (true);
}

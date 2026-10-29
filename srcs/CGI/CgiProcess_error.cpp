/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_error.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 03:04:54 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 02:00:52 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"

void	CgiProcess::logError(t_CgiError err)
{
	switch (err)
	{
		case CGI_ERR_NONE:
			break ;
		case CGI_ERR_ALREADY_RUNNING:
			g_Log << "[e] CGI: cannot spawn a new CGI while previous execution "
					"is already running\n";
			break ;
		case CGI_ERR_BAD_SPEC:
			g_Log << "[e] CGI: CGI launch specification is invalid or "
					"incomplete\n";
			break ;
		case CGI_ERR_BUILD_ARGV:
			g_Log << "[e] CGI: Failed to build argv array for execve()\n";
			break ;
		case CGI_ERR_BUILD_ENVP:
			g_Log << "[e] CGI: Failed to build envp array for execve()\n";
			break ;
		case CGI_ERR_PIPE_STDIN:
			g_Log << "[e] CGI: Failed to create CGI stdin pipe\n";
			break ;
		case CGI_ERR_PIPE_STDOUT:
			g_Log << "[e] CGI: Failed to create CGI stdout pipe\n";
			break ;
		case CGI_ERR_FORK:
			g_Log << "[e] CGI: fork() failed while spawning CGI process\n";
			break ;
		case CGI_ERR_CHDIR:
			g_Log << "[e] CGI: chdir() failed before CGI execution\n";
			break ;
		case CGI_ERR_DUP2_STDIN:
			g_Log << "[e] CGI: dup2() failed while redirecting CGI stdin\n";
			break ;
		case CGI_ERR_DUP2_STDOUT:
			g_Log << "[e] CGI: dup2() failed while redirecting CGI stdout\n";
			break ;
		case CGI_ERR_EXECVE:
			g_Log << "[e] CGI: execve() failed in CGI child process\n";
			break ;
		case CGI_ERR_WRITE_STDIN:
			g_Log << "[e] CGI: Failed to write request body to CGI stdin\n";
			break ;
		case CGI_ERR_READ_STDOUT:
			g_Log << "[e] CGI: Failed to read data from CGI stdout\n";
			break ;
		case CGI_ERR_WAITPID:
			g_Log << "[e] CGI: waitpid() failed while collecting CGI child "
					"status\n";
			break ;
		case CGI_ERR_CLOSE:
			g_Log << "[e] CGI: close() failed on CGI-related file descriptor\n";
			break ;
		case CGI_ERR_PIPE_EXEC:
			g_Log << "[e] CGI: Failed to create exec failure communication "
					"pipe\n";
			break ;
		case CGI_ERR_FCNTL_CLOEXEC:
			g_Log << "[e] CGI: fcntl() failed while setting FD_CLOEXEC on exec "
					"pipe\n";
			break ;
		case CGI_ERR_FCNTL_NONBLOCK_EXEC:
			g_Log << "[e] CGI: fcntl() failed while setting O_NONBLOCK on "
					"exec failure pipe\n";
			break ;
		case CGI_ERR_FCNTL_NONBLOCK_STDIN:
			g_Log << "[e] CGI: fcntl() failed while setting O_NONBLOCK on stdin"
					" pipe\n";
			break ;
		case CGI_ERR_FCNTL_NONBLOCK_STDOUT:
			g_Log << "[e] CGI: fcntl() failed while setting O_NONBLOCK on "
					"stdout pipe\n";
			break ;
		case CGI_ERR_READ_EXEC_PIPE:
			g_Log << "[e] CGI: Failed to read execve() error code from exec "
					"pipe\n";
			break ;
	}
}

bool	CgiProcess::terminateChildAndReap(void)
{
	int		status = 0;
	pid_t	res;

	this->closeRunStateFds();

	if (this->_runState.pid <= 0
		|| this->_runState.child_status_collected == true)
	{
		this->destroyExecArrays();
		this->resetLaunchSpec();
		this->resetRunState();
		return (true);
	}

	kill(this->_runState.pid, SIGTERM);
	res = waitpid(this->_runState.pid, &status, WNOHANG);

	if (res == this->_runState.pid)
	{
		this->decodeWaitStatus(status);
		this->destroyExecArrays();
		this->resetLaunchSpec();
		this->resetRunState();
		return (true);
	}

	if (res == 0)
	{
		kill(this->_runState.pid, SIGKILL);
		res = waitpid(this->_runState.pid, &status, WNOHANG);
		if (res == this->_runState.pid)
		{
			this->decodeWaitStatus(status);
			this->destroyExecArrays();
			this->resetLaunchSpec();
			this->resetRunState();
			return (true);
		}
	}

	if (res < 0)
	{
		this->setError(CGI_ERR_WAITPID);
		this->destroyExecArrays();
		this->resetLaunchSpec();
		this->resetRunState();
		return (false);
	}
	return (false);
}

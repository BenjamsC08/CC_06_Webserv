/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_spawn.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 02:51:06 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 01:59:17 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"
#include <unistd.h>

bool	CgiProcess::handleParentAfterFork(int (&exec_pipe)[2],
										int (&stdout_pipe)[2],
										int (&stdin_pipe)[2])
{
	this->closeIfOpen(exec_pipe[1]);
	this->closeIfOpen(stdout_pipe[1]);
	this->closeIfOpen(stdin_pipe[0]);

	this->_runState.exec_fd_parent = exec_pipe[0];
	this->_runState.stdout_fd_parent = stdout_pipe[0];
	this->_runState.stdin_fd_parent = stdin_pipe[1];

	exec_pipe[0] = -1;
	stdout_pipe[0] = -1;
	stdin_pipe[1] = -1;
	this->_runState.spawned = true;
	this->_runState.exec_setup_checked = false;
	this->updateStep(CGI_RUNNING);
	return (true);

}

void	CgiProcess::handleChildAfterFork(int (&exec_pipe)[2],
										int (&stdout_pipe)[2],
										int (&stdin_pipe)[2])
{
	t_CgiException	cgiException;
	int				sys_errno = 0;

	this->closeIfOpen(exec_pipe[0]);
	this->closeIfOpen(stdin_pipe[1]);
	this->closeIfOpen(stdout_pipe[0]);



	if (this->applyChildWorkingDirectory() == false)
	{
		sys_errno = errno;
		this->writeChildSetupError(exec_pipe[1],
				CGI_ERR_CHDIR, sys_errno);
		this->closePipePair(exec_pipe);
		this->closePipePair(stdout_pipe);
		this->closePipePair(stdin_pipe);
		throw cgiException;
	}



	if (dup2(stdin_pipe[0], STDIN_FILENO) == -1)
	{
		sys_errno = errno;
		this->writeChildSetupError(exec_pipe[1],
				CGI_ERR_DUP2_STDIN, sys_errno);
		this->closePipePair(exec_pipe);
		this->closePipePair(stdout_pipe);
		this->closePipePair(stdin_pipe);
		throw cgiException;
	}
	this->closeIfOpen(stdin_pipe[0]);



	if (dup2(stdout_pipe[1], STDOUT_FILENO) == -1)
	{
		sys_errno = errno;
		this->writeChildSetupError(exec_pipe[1],
				CGI_ERR_DUP2_STDOUT, sys_errno);
		this->closePipePair(exec_pipe);
		this->closePipePair(stdout_pipe);
		this->closePipePair(stdin_pipe);
		if (close(STDIN_FILENO) == -1) {;}
		throw cgiException;
	}
	this->closeIfOpen(stdout_pipe[1]);



	execve(this->_launchSpec.program_path.c_str(),
			this->_argvExec, this->_envpExec);
	sys_errno = errno;
	this->writeChildSetupError(exec_pipe[1], CGI_ERR_EXECVE, sys_errno);
	this->closePipePair(exec_pipe);
	this->closePipePair(stdout_pipe);
	this->closePipePair(stdin_pipe);
	if (close(STDOUT_FILENO) == -1 || close(STDIN_FILENO) == -1) {;}
	throw cgiException;
}

bool	CgiProcess::spawn(void)
{
	int	exec_pipe[2];
	int	stdout_pipe[2];
	int	stdin_pipe[2];

	if (this->isAlreadyRunning())
		return (this->setError(CGI_ERR_ALREADY_RUNNING));
	this->closeRunStateFds();
	this->resetRunState();
	if (this->hasMinimalLaunchSpec() == false)
		return (setError(CGI_ERR_BAD_SPEC));
	if (this->buildExecArrays() == false)
		return (false);
	if (this->createExecFailurePipe(exec_pipe) == false)
		return (false);
	if (this->createStdinPipe(stdin_pipe) == false)
		return (this->closePipePair(exec_pipe), false);
	if (this->createStdoutPipe(stdout_pipe) == false)
	{
		this->closePipePair(stdin_pipe);
		this->closePipePair(exec_pipe);
		return (false);
	}

	if (!this->setNonBlocking(exec_pipe[0], CGI_ERR_FCNTL_NONBLOCK_EXEC)
		|| !this->setNonBlocking(stdin_pipe[1], CGI_ERR_FCNTL_NONBLOCK_STDIN)
		|| !this->setNonBlocking(stdout_pipe[0], CGI_ERR_FCNTL_NONBLOCK_STDOUT))
	{
		this->closePipePair(stdin_pipe);
		this->closePipePair(stdout_pipe);
		this->closePipePair(exec_pipe);
		return (false);
	}

	this->_runState.pid = fork();
	if (this->_runState.pid == -1)
	{
		this->closePipePair(stdin_pipe);
		this->closePipePair(stdout_pipe);
		this->closePipePair(exec_pipe);
		return (this->setError(CGI_ERR_FORK));
	}
	if (this->_runState.pid == 0)
		this->handleChildAfterFork(exec_pipe, stdout_pipe, stdin_pipe);
	else
		return (this->handleParentAfterFork(exec_pipe, stdout_pipe, stdin_pipe));
	return (true);
}

void	CgiProcess::decodeWaitStatus(int newStatus)
{
	this->_runState.raw_wait_status = newStatus;
	if (WIFEXITED(newStatus))
	{
		this->_runState.child_status_collected = true;
		this->_runState.child_exited_normally = true;
		this->_runState.exit_status = WEXITSTATUS(newStatus);
	}
	if (WIFSIGNALED(newStatus))
	{
		this->_runState.child_status_collected = true;
		this->_runState.child_signaled = true;
		this->_runState.term_signal = WTERMSIG(newStatus);
	}
}

bool	CgiProcess::applyChildWorkingDirectory(void)
{
	if (this->_launchSpec.working_directory.empty())
		return (true);
	if (chdir(this->_launchSpec.working_directory.c_str()) == -1)
		return (false);
	return (true);

}

bool	CgiProcess::isAlreadyRunning(void)
{
	if (this->_runState.pid > 0 && !this->_runState.child_status_collected)
		return (true);
	if (this->_runState.stdin_fd_parent != -1
			|| this->_runState.exec_fd_parent != -1
			|| this->_runState.stdout_fd_parent != -1)
		return (true);
	return (false);
}

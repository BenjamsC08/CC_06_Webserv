/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_setters_getters.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 03:03:49 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 01:59:27 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"

bool	CgiProcess::hasMinimalLaunchSpec(void) const
{
	if (this->_launchSpec.program_path.empty())
		return (false);
	if (this->_launchSpec.argv.empty())
		return (false);
	return (true);
}

bool	CgiProcess::hasError(void) const
{ return (this->_runState.error != CGI_ERR_NONE); }

bool	CgiProcess::isComplete(void) const
{ return (this->_runState.step == CGI_DONE); }

t_CgiStep	CgiProcess::getStep(void) const
{ return (this->_runState.step); }

const std::string	&CgiProcess::getStdoutBuffer(void) const
{ return (this->_runState.stdout_buffer); }


const t_CgiLaunchSpec	&CgiProcess::getLaunchSpec(void) const
{ return (this->_launchSpec); }

const t_CgiRunState	&CgiProcess::getRunState(void) const
{ return (this->_runState); }

char	**CgiProcess::getArgvExec(void) const
{ return (this->_argvExec); }

char	**CgiProcess::getEnvpExec(void) const
{ return (this->_envpExec); }





int	CgiProcess::getStdinFdParent(void) const
{ return (this->_runState.stdin_fd_parent); }

int	CgiProcess::getStdoutFdParent(void) const
{ return (this->_runState.stdout_fd_parent); }


bool	CgiProcess::isStdinClosed(void) const
{ return (this->_runState.stdin_closed); }

bool	CgiProcess::isStdoutEof(void) const
{ return (this->_runState.stdout_eof); }





int	CgiProcess::getExecFdParent(void) const
{ return (this->_runState.exec_fd_parent); }





void	CgiProcess::addArg(const std::string &arg)
{
	this->destroyExecArrays();
	this->_launchSpec.argv.push_back(arg);
}

void	CgiProcess::addEnvLine(const std::string &envLine)
{
	this->destroyExecArrays();
	this->_launchSpec.env_lines.push_back(envLine);
}

void	CgiProcess::setProgramPath(const std::string &programPath)
{
	this->destroyExecArrays();
	this->_launchSpec.program_path = programPath;
}

void	CgiProcess::setWorkingDirectory(const std::string &workingDirectory)
{
	this->destroyExecArrays();
	this->_launchSpec.working_directory = workingDirectory;
}

void	CgiProcess::setStdinData(const std::string &stdinData)
{
	this->destroyExecArrays();
	this->_launchSpec.stdin_data = stdinData;
}

bool	CgiProcess::setError(t_CgiError err)
{
	this->logError(err);
	if (this->hasError() == false && err != CGI_ERR_NONE)
	{
		this->_runState.last_step_before_error = this->getStep();
		this->_runState.error = err;
		this->updateStep(CGI_ERROR);
	}
	return (false);
}

void	CgiProcess::updateStep(t_CgiStep newStep)
{ this->_runState.step = newStep; }

void	CgiProcess::updateCompletionState(void)
{
	if (!this->_runState.exec_setup_checked)
		return ;
	if (this->_runState.stdin_closed && this->_runState.stdout_eof
			&& this->_runState.child_status_collected)
		this->updateStep(CGI_DONE);
}

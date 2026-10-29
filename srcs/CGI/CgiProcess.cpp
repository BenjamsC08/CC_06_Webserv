/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/28 09:34:19 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 01:59:59 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"



CgiProcess::CgiProcess(void) : _argvExec(NULL), _envpExec(NULL)
{
	this->resetLaunchSpec();
	this->resetRunState();
}

CgiProcess::~CgiProcess(void)
{
	this->closeRunStateFds();
	this->destroyExecArrays();
}

void	CgiProcess::resetLaunchSpec(void)
{
	this->destroyExecArrays();
	this->_launchSpec.program_path.clear();
	this->_launchSpec.working_directory.clear();
	this->_launchSpec.argv.clear();
	this->_launchSpec.env_lines.clear();
	this->_launchSpec.stdin_data.clear();
}

void	CgiProcess::resetRunState(void)
{
	this->_runState.pid = -1;
	this->_runState.exec_fd_parent = -1;
	this->_runState.stdin_fd_parent = -1;
	this->_runState.stdout_fd_parent = -1;
	this->_runState.stdin_sent = 0;
	this->_runState.stdout_buffer.clear();
	this->_runState.exec_setup_checked = false;
	this->_runState.stdin_closed = false;
	this->_runState.stdout_eof = false;
	this->_runState.spawned = false;
	this->_runState.exit_status = 0;
	this->_runState.child_status_collected = false;
	this->_runState.child_exited_normally = false;
	this->_runState.child_signaled = false;
	this->_runState.term_signal = 0;
	this->_runState.raw_wait_status = 0;
	this->_runState.child_setup_errno = 0;
	this->_runState.child_setup_failed_before_exec = false;
	this->_runState.step = CGI_EMPTY;
	this->_runState.last_step_before_error = CGI_EMPTY;
	this->_runState.error = CGI_ERR_NONE;
}

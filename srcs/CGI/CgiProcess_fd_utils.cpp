/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_fd_utils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 02:59:21 by mkerrien          #+#    #+#             */
/*   Updated: 2026/04/01 02:59:58 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"

void	CgiProcess::closeRunStateFds(void)
{
	closeIfOpen(this->_runState.exec_fd_parent);
	closeIfOpen(this->_runState.stdin_fd_parent);
	closeIfOpen(this->_runState.stdout_fd_parent);
}

void	CgiProcess::closePipePair(int (&pipe_fd)[2])
{
	this->closeIfOpen(pipe_fd[0]);
	this->closeIfOpen(pipe_fd[1]);
}

void	CgiProcess::closeIfOpen(int &fd)
{
	if (fd == -1)
		return ;
	if (close(fd) == -1)
		this->setError(CGI_ERR_CLOSE);
	fd = -1;
}


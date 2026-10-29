/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client_CGI.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 19:22:36 by mkerrien          #+#    #+#             */
/*   Updated: 2026/04/28 06:22:26 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include <ctime>

void	Client::startCgiResponse(CgiCtx &ctx, int efd, t_CgiFdMap &cgiMap)
{
	const t_CgiLaunchInfos	&infos = ctx.getLaunchInfos();

	this->cleanupCgi(efd, cgiMap);
	this->_cgi.setProgramPath(infos._program_path);
	this->_cgi.setWorkingDirectory(infos._working_directory);
	this->_cgi.setStdinData(infos._stdin_data);

	for (long unsigned int i = 0; i < infos._argv.size(); ++i)
		this->_cgi.addArg(infos._argv[i]);
	for (long unsigned int i = 0; i < infos._env_lines.size(); ++i)
		this->_cgi.addEnvLine(infos._env_lines[i]);
	if (!this->_cgi.spawn())
		return (this->startErrorResponse(efd, 500));

	this->_hasCgi = true;
	this->_cgiStartTime = time(NULL);
	this->_state = CLIENT_PROCESSING_CGI;

	this->_cgiExecFd = this->_cgi.getExecFdParent();
	this->_cgiStdoutFd = this->_cgi.getStdoutFdParent();
	this->_cgiStdinFd = this->_cgi.getStdinFdParent();

	if (!this->registerCgiFd(efd, cgiMap, this->_cgiExecFd,
			CGI_FD_EXEC, EPOLLIN | EPOLLHUP))
		return (this->failCgi(efd, cgiMap, 500));

	if (!this->registerCgiFd(efd, cgiMap, this->_cgiStdoutFd,
			CGI_FD_STDOUT, EPOLLIN | EPOLLHUP))
		return (this->failCgi(efd, cgiMap, 500));

	this->disableClientSocketEventsDuringCgi(efd);
}

void	Client::handleCgiEvent(int efd, int cgiFd, uint32_t events,
			t_CgiFdRole role, t_CgiFdMap &cgiMap)
{
	if (!this->_hasCgi || this->_state != CLIENT_PROCESSING_CGI)
	{
		this->unregisterCgiFd(efd, cgiMap, cgiFd);
		return ;
	}
	if ((events & EPOLLERR) && role == CGI_FD_STDIN)
	{
		this->_cgi.closeChildStdin();
		this->unregisterCgiFd(efd, cgiMap, cgiFd);
		this->_cgiStdinFd = -1;
		this->tickCgi(efd, cgiMap);
		this->finishCgiIfComplete(efd, cgiMap);
		return ;
	}
	if (events & EPOLLERR)
		return (this->failCgi(efd, cgiMap, 500));
	if (role == CGI_FD_EXEC)
		return (this->handleCgiExecEvent(efd, cgiFd, events, cgiMap));
	if (role == CGI_FD_STDIN)
		return (this->handleCgiStdinEvent(efd, cgiFd, events, cgiMap));
	if (role == CGI_FD_STDOUT)
		return (this->handleCgiStdoutEvent(efd, cgiFd, events, cgiMap));
}

void	Client::handleCgiExecEvent(int efd, int cgiFd, uint32_t events,
								t_CgiFdMap &cgiMap)
{
	if (!(events & EPOLLIN) && !(events & EPOLLHUP))
		return ;

	if (!this->_cgi.checkExecFailurePipe())
		return (this->failCgi(efd, cgiMap, 500));

	this->unregisterCgiFd(efd, cgiMap, cgiFd);
	this->_cgiExecFd = -1;

	if (this->_cgi.hasError())
		return (this->failCgi(efd, cgiMap, 500));

	this->registerCgiStdinIfNeeded(efd, cgiMap);
	this->tickCgi(efd, cgiMap);
	this->finishCgiIfComplete(efd, cgiMap);
}

void	Client::handleCgiStdinEvent(int efd, int cgiFd, uint32_t events,
									t_CgiFdMap &cgiMap)
{
	if (events & EPOLLHUP)
	{
		this->_cgi.closeChildStdin();
		this->unregisterCgiFd(efd, cgiMap, cgiFd);
		this->_cgiStdinFd = -1;
		this->tickCgi(efd, cgiMap);
		this->finishCgiIfComplete(efd, cgiMap);
		return ;
	}

	if (!(events & EPOLLOUT))
		return ;

	if (!this->_cgi.writeChildStdin())
		return (this->failCgi(efd, cgiMap, 500));

	if (this->_cgi.isStdinClosed()
		|| this->_cgi.getStdinFdParent() == -1)
	{
		this->unregisterCgiFd(efd, cgiMap, cgiFd);
		this->_cgiStdinFd = -1;
	}

	this->tickCgi(efd, cgiMap);
	this->finishCgiIfComplete(efd, cgiMap);
}

void	Client::handleCgiStdoutEvent(int efd, int cgiFd, uint32_t events,
			t_CgiFdMap &cgiMap)
{
	if (!(events & EPOLLIN) && !(events & EPOLLHUP))
		return ;
	if (!this->_cgi.readChildStdout())
		return (this->failCgi(efd, cgiMap, 500));

	if (this->_cgi.isStdoutEof()
		|| this->_cgi.getStdoutFdParent() == -1)
	{
		this->unregisterCgiFd(efd, cgiMap, cgiFd);
		this->_cgiStdoutFd = -1;
	}
	this->tickCgi(efd, cgiMap);
	this->finishCgiIfComplete(efd, cgiMap);
}

void	Client::tickCgi(int efd, t_CgiFdMap &cgiMap)
{
	if (!this->_hasCgi)
		return ;
	if (!this->_cgi.epollChildExit())
		return (this->failCgi(efd, cgiMap, 500));
}

void	Client::pollCgiProgress(int efd, t_CgiFdMap &cgiMap)
{
	if (!this->_hasCgi)
		return ;
	if (this->_state != CLIENT_PROCESSING_CGI)
		return ;

	if (!this->_cgi.epollChildExit())
		return (this->failCgi(efd, cgiMap, 500));
	this->finishCgiIfComplete(efd, cgiMap);
}

void	Client::finishCgiIfComplete(int efd, t_CgiFdMap &cgiMap)
{
	std::string	body;
	std::string	response;

	if (!this->_hasCgi)
		return ;
	if (this->_cgi.hasError())
		return (this->failCgi(efd, cgiMap, 500));
	if (!this->_cgi.isComplete())
		return ;

	body = this->_cgi.getStdoutBuffer();
	if (!this->buildCgiHttpResponse(body, response))
		return (this->failCgi(efd, cgiMap, 502));

	DEBUG(GREEN << "CGI complete\n");

	this->_res.reset();
	this->_res.setErrorPath(this->_serv.getHttpErrors());
	this->_res.setConnection(this->selectConnectionValue());
	this->_res.setResponse(response);
	this->_res.setTotalBytes(response.length());

	this->_state = CLIENT_WRITING_RESPONSE;
	this->cleanupCgi(efd, cgiMap);
	this->monitorClientSocketForWrite(efd);
}

bool	Client::buildCgiHttpResponse(const std::string &cgi_output,
			std::string &response) const
{
	t_CgiParsedResponse	parsed;

	if (!CgiResponseParser::parse(cgi_output, parsed))
		return (false);
	response = this->serializeCgiHttpResponse(parsed);
	return (true);
}

std::string	Client::serializeCgiHttpResponse(
				const t_CgiParsedResponse &parsed) const
{
	std::ostringstream	oss;
	std::string			body;

	body = parsed.body;
	if (parsed.status_code == 204 || parsed.status_code == 304)
		body.clear();

	oss << "HTTP/1.1 " << parsed.status_code << " "
		<< parsed.reason_phrase << "\r\n";
	oss << "Connection: " << this->selectConnectionValue() << "\r\n";

	for (size_t i = 0; i < parsed.headers.size(); i++)
	{
		oss << parsed.headers[i].name << ": "
			<< parsed.headers[i].value << "\r\n";
	}

	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "\r\n";
	oss << body;
	return (oss.str());
}

void	Client::failCgi(int efd, t_CgiFdMap &cgiMap, int httpCode)
{
	this->cleanupCgi(efd, cgiMap);
	this->startErrorResponse(efd, httpCode);
}

void	Client::cleanupCgi(int efd, t_CgiFdMap &cgiMap)
{
	this->unregisterCgiFd(efd, cgiMap, this->_cgiStdinFd);
	this->unregisterCgiFd(efd, cgiMap, this->_cgiStdoutFd);
	this->unregisterCgiFd(efd, cgiMap, this->_cgiExecFd);

	this->_cgiStdinFd = -1;
	this->_cgiStdoutFd = -1;
	this->_cgiExecFd = -1;

	this->_cgi.terminateChildAndReap();
	this->_hasCgi = false;
	this->_cgiStartTime = 0;
}

bool	Client::registerCgiFd(int efd, t_CgiFdMap &cgiMap, int fd,
							t_CgiFdRole role, uint32_t events)
{
	epoll_event		ev;
	t_CgiFdBinding	binding;

	if (fd == -1 || cgiMap.find(fd) != cgiMap.end())
		return (true);

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1)
		return (false);

	binding.client = this;
	binding.role = role;
	cgiMap[fd] = binding;
	return (true);
}

void	Client::unregisterCgiFd(int efd, t_CgiFdMap &cgiMap, int fd)
{
	if (fd == -1)
		return ;
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
	cgiMap.erase(fd);
}

void	Client::registerCgiStdinIfNeeded(int efd, t_CgiFdMap &cgiMap)
{
	if (!this->_hasCgi || this->_cgi.isStdinClosed()
			|| this->_cgi.getStdinFdParent() == -1)
		return ;
	if (this->_req.getBody().empty())
	{
		this->_cgi.closeChildStdin();
		this->_cgiStdinFd = -1;
		return ;
	}
	this->_cgiStdinFd = this->_cgi.getStdinFdParent();
	if (!this->registerCgiFd(efd, cgiMap, this->_cgiStdinFd,
			CGI_FD_STDIN, EPOLLOUT | EPOLLHUP))
		this->failCgi(efd, cgiMap, 500);
}

time_t	Client::getCgiStartTime(void) const
{ return (this->_cgiStartTime); }

void	Client::timeoutCgi(int efd, t_CgiFdMap &cgiMap)
{
	if (!this->_hasCgi)
		return ;
	this->cleanupCgi(efd, cgiMap);
	this->_closeAfterResponse = true;
	this->startErrorResponse(efd, 504);
}

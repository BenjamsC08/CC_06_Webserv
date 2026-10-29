/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:32:55 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:57:39 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"


Client::Client(Server & serv): _state(CLIENT_READING_REQUEST), _timeoutCounter(time(NULL)), _fd(-1), _serv(serv), _req(serv.getClientMaxBodySize()), _bodyTooBig(false), _killswitch(false), _closeAfterResponse(false), _hasCgi(false), _cgiStdinFd(-1), _cgiStdoutFd(-1),/* _cgiStderrFd(-1),*/ _cgiExecFd(-1), _cgiStartTime(0)
{}

Client::~Client()
{}

void	Client::receiveData(int fd_toFind, int efd,
					std::map<int, Client *> &list_Clients, t_CgiFdMap &list_cgi)
{
	char buffer[300] = {0};
	int bytesRead = read(fd_toFind, buffer, 299);
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		_req.setReadBytes(bytesRead);
		std::string str(buffer, bytesRead);
		_req.setRawData(str);
		_req.setIp(_ipClient);
		_req.parseHandler(_serv.getHttpRules());
		_bodyTooBig = _req.getBodyTooBig();
	    _killswitch = _req.getToClose();

		if (_req.getState() == FINISHED && this->_state == CLIENT_READING_REQUEST)
			this->startProcessing(efd, list_cgi);
		_timeoutCounter = time(NULL);
	}
	else if (bytesRead == -1 || bytesRead == 0)
	{
		if (bytesRead == 0)
		{
			DEBUG("Client " << fd_toFind << " has sent an EOF.\n");
			DEBUG("Bytes read is: " << bytesRead << "\n");
		}
		else
			DEBUG("Client " << fd_toFind << " has had a read error.\n");
		removeClient(efd, fd_toFind, list_Clients, list_cgi);
	}
}


void	Client::sendData(int fd_toFind, int efd, t_CgiFdMap &list_cgi)
{
	int			writeCheck;
	std::string	response;
	int			remaining;

	(void)list_cgi;

	if (this->_state != CLIENT_WRITING_RESPONSE)
	{
		return ;
	}
	response = this->_res.getResponse();
	if (response.empty())
	{
		this->_state = CLIENT_CLOSING;
		this->_killswitch = true;
		return ;
	}

	this->_res.setTotalBytes(response.length());
	remaining = this->_res.getTotalByte() - this->_res.getByteSent();
	if (remaining <= 0)
	{
		this->connectionHandler(efd, list_cgi);
		return ;
	}
	writeCheck = send(fd_toFind,
			response.c_str() + this->_res.getByteSent(),
			remaining, 0);

	
	if (writeCheck > 0)
		this->_res.setBytesent(this->_res.getByteSent() + writeCheck);
	else if (writeCheck < 0)
	{
		this->_state = CLIENT_CLOSING;
		this->_killswitch = true;
		return ;
	}

	this->_timeoutCounter = time(NULL);

	if (this->_res.getByteSent() == this->_res.getTotalByte())
		this->connectionHandler(efd, list_cgi);
}

void	Client::connectionHandler(int efd, t_CgiFdMap &cgiMap)
{
	if (this->selectConnectionValue() == "Close")
	{
		this->_state = CLIENT_CLOSING;
		this->_killswitch = true;
		return ;
	}
	this->cleanupCgi(efd, cgiMap);
	this->_req.reset();
	this->_res.reset();
	this->_bodyTooBig = false;
	this->_closeAfterResponse = false;
	this->_state = CLIENT_READING_REQUEST;
	this->monitorClientSocketForRead(efd);
}

void		removeClient(int efd, int fd_to_remove,
				std::map<int, Client *> &list_client, t_CgiFdMap &cgiMap)
{
	std::map<int, Client *>::iterator it = list_client.find(fd_to_remove);
	if (it != list_client.end())
	{
		g_Log << "[i] Client " << it->first << " has disconnected.\n";
		it->second->cleanupCgi(efd, cgiMap);
		epoll_ctl(efd, EPOLL_CTL_DEL, fd_to_remove, NULL);
		close(fd_to_remove);
		delete it->second;
		list_client.erase(it);
	}
}

Request & Client::getRequest()
{
	return(_req);
}

time_t	  Client::getTimeoutCount()
{
	return(_timeoutCounter);
}

int		 Client::getFd()
{
	return(_fd);
}
void		Client::setFd(int toUse_fd)
{
	_fd = toUse_fd;
}

std::string	Client::getIp()
{
	return(_ipClient);
}
void		Client::setIp(std::string toSet)
{
	_ipClient = toSet;
}

bool		Client::getKillSwitch()
{
	return (_killswitch);
}

bool	Client::isProcessingCgi(void) const
{
	return (this->_state == CLIENT_PROCESSING_CGI);
}

void	Client::startProcessing(int efd, t_CgiFdMap &cgiMap)
{
	if (this->_req.getPath().find("/set-theme") == 0)
		return (this->startStaticResponse(efd));
	
	if (!this->_req.getErrorCode().empty())
		return(this->startErrorResponse(efd,
				atoi(_req.getErrorCode().c_str())));

	CgiCtx ctx(this->_req, this->_serv);

	if (!ctx.createCgiCtx())
		return (this->startErrorResponse(efd, 500));

	switch (ctx.getStatus())
	{
		case CGI_CTX_NOT_CGI:
			return (this->startStaticResponse(efd));

		case CGI_CTX_READY:
			return (this->startCgiResponse(ctx, efd, cgiMap));

		case CGI_CTX_BAD_TARGET:
			return (this->startErrorResponse(efd, 400));

		case CGI_CTX_BAD_URI_SUFFIX:
		case CGI_CTX_SCRIPT_FORBIDDEN:
			return (this->startErrorResponse(efd, 403));

		case CGI_CTX_NO_LOCATION:
		case CGI_CTX_SCRIPT_NOT_FOUND:
			return (this->startErrorResponse(efd, 404));

		case CGI_CTX_METHOD_NOT_ALLOWED:
			return (this->startErrorResponse(efd, 405));

		default:
			return (this->startErrorResponse(efd, 500));
	}
}

void	Client::handleClientEvent(int efd, uint32_t events,
			std::map<int, Client *> &clients, t_CgiFdMap &cgiMap)
{
	if (events & (EPOLLERR | EPOLLHUP))
	{
		removeClient(efd, this->_fd, clients, cgiMap);
		return ;
	}

	if (this->_state == CLIENT_READING_REQUEST)
	{
		if (events & EPOLLIN)
			this->receiveData(this->_fd, efd, clients, cgiMap);
		return ;
	}

	if (this->_state == CLIENT_WRITING_RESPONSE)
	{
		if (events & EPOLLOUT)
			this->sendData(this->_fd, efd, cgiMap);
		return ;
	}

	if (this->_state == CLIENT_PROCESSING_CGI)
		return ;

	if (this->_state == CLIENT_CLOSING)
	{
		removeClient(efd, this->_fd, clients, cgiMap);
		return ;
	}
}

void	Client::monitorClientSocketForRead(int efd)
{
	epoll_event	ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = this->_fd;
	epoll_ctl(efd, EPOLL_CTL_MOD, this->_fd, &ev);
}
void	Client::monitorClientSocketForWrite(int efd)
{
	epoll_event	ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLOUT;
	ev.data.fd = this->_fd;
	epoll_ctl(efd, EPOLL_CTL_MOD, this->_fd, &ev);
}
void	Client::disableClientSocketEventsDuringCgi(int efd)
{
	epoll_event	ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = 0;
	ev.data.fd = this->_fd;
	epoll_ctl(efd, EPOLL_CTL_MOD, this->_fd, &ev);
}

std::string	Client::httpCodeToString(int code) const
{
	std::ostringstream	oss;

	oss << code;
	return (oss.str());
}

std::string	Client::selectConnectionValue(void) const
{
	const std::map<std::string, std::string> & headers = this->_req.getHeader();
	std::map<std::string, std::string>::const_iterator	it;

	it = headers.find("connection");
	if (it != headers.end() && it->second == "close")
		return ("Close");
	if (this->_bodyTooBig || this->_closeAfterResponse || this->_killswitch)
		return ("Close");
	if (this->_req.getToClose())
		return ("Close");
	return ("Keep-Alive");
}

void	Client::switchClientSocketToWriteEvents(int efd)
{
	epoll_event	ev;

	ev.events = EPOLLOUT;
	ev.data.fd = this->_fd;
	if (epoll_ctl(efd, EPOLL_CTL_MOD, this->_fd, &ev) == -1)
		this->_state = CLIENT_CLOSING;
}

void	Client::startErrorResponse(int efd, int code)
{
	std::string	codeStr = this->httpCodeToString(code);

	this->_res.reset();
	this->_res.setErrorPath(this->_serv.getHttpErrors());
	this->_res.setConnection(this->selectConnectionValue());
	this->_res.errorConstructors(codeStr, _req);

	if (this->_res.getResponse().empty())
	{
		this->_state = CLIENT_CLOSING;
		this->_killswitch = true;
		return ;
	}

	this->_res.setTotalBytes(this->_res.getResponse().length());
	this->_state = CLIENT_WRITING_RESPONSE;
	this->switchClientSocketToWriteEvents(efd);
}

void	Client::startErrorResponseClient(int efd, int code)
{
	std::string	codeStr = this->httpCodeToString(code);

	this->_res.reset();
	this->_res.setErrorPath(this->_serv.getHttpErrors());
	this->_res.setConnection(this->selectConnectionValue());
	this->_res.errorConstructors(codeStr, _req);

	if (this->_res.getResponse().empty())
	{
		this->_state = CLIENT_CLOSING;
		this->_killswitch = true;
		return ;
	}

	this->_res.setTotalBytes(this->_res.getResponse().length());
	this->_state = CLIENT_WRITING_RESPONSE;
	this->switchClientSocketToWriteEvents(efd);
}

void	Client::startStaticResponse(int efd)
{
	this->_res.reset();

	this->_res.setErrorPath(this->_serv.getHttpErrors());
	this->_res.setConnection(this->selectConnectionValue());

	this->_res.prepareResponse(this->_req);

	if (this->_req.getIsChunked() == true)
		this->_req.setIsChunked(false);

	if (this->_res.getResponse().empty())
	{
		return (this->startErrorResponse(efd, 500));
	}

	this->_res.setTotalBytes(this->_res.getResponse().length());
	this->_state = CLIENT_WRITING_RESPONSE;
	this->switchClientSocketToWriteEvents(efd);
}

void	Client::timeoutClient(int efd)
{
	this->_closeAfterResponse = true;
	this->startErrorResponseClient(efd, 408);
}

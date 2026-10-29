/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:32:58 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:32:59 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ctime>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include "CgiCtx.hpp"
#include "CgiEpoll.hpp"
#include "CgiProcess.hpp"
#include "CgiResponseParser.hpp"
#include "Request.hpp"
#include "Response.hpp"

enum    State
{
	CLIENT_READING_REQUEST,
	CLIENT_PROCESSING_CGI,
	CLIENT_WRITING_RESPONSE,
	CLIENT_CLOSING
};

class Server;

class Client{

    public:
        Client(Server &serv);
        ~Client();

        Request			&getRequest();
        time_t          getTimeoutCount();
		time_t			getCgiStartTime() const;
        int             getFd();
        std::string     getIp();
        bool			getKillSwitch();
		bool			isProcessingCgi() const;

        void            setFd(int);
        void            setIp(std::string);

        void            receiveData(int, int, std::map<int, Client *> &,
						t_CgiFdMap &);
        void            sendData(int, int, t_CgiFdMap &);
        void            connectionHandler(int, t_CgiFdMap &);
		void			timeoutClient(int efd);
		void			startErrorResponseClient(int efd, int code);


	// CGI
		void			cleanupCgi(int, t_CgiFdMap &);
		void			handleClientEvent(int, uint32_t,
						std::map<int, Client *> &, t_CgiFdMap &);
		void			handleCgiEvent(int, int, uint32_t, t_CgiFdRole,
						t_CgiFdMap &);
		void			handleCgiExecEvent(int, int, uint32_t, t_CgiFdMap &);
		void			tickCgi(int, t_CgiFdMap &);
		void			pollCgiProgress(int, t_CgiFdMap &);
		void			timeoutCgi(int efd, t_CgiFdMap &cgiMap);

    private:
        Client(const Client &);
        Client & operator=(const Client &);

		void			monitorClientSocketForRead(int);
		void 			monitorClientSocketForWrite(int);
		void 			disableClientSocketEventsDuringCgi(int);
		void			startProcessing(int, t_CgiFdMap &);

	// CGI
		void			startStaticResponse(int);
		void			startErrorResponse(int, int);
		void			startCgiResponse(CgiCtx &, int, t_CgiFdMap &);

		void			handleCgiStdinEvent(int, int, uint32_t, t_CgiFdMap &);
		void			handleCgiStdoutEvent(int, int, uint32_t, t_CgiFdMap &);

		void			finishCgiIfComplete(int, t_CgiFdMap &);
		void			failCgi(int, t_CgiFdMap &, int);

		bool			registerCgiFd(int, t_CgiFdMap &, int, t_CgiFdRole,
						uint32_t);
		void			unregisterCgiFd(int, t_CgiFdMap &, int);
		void			registerCgiStdinIfNeeded(int, t_CgiFdMap &);

		void			switchClientSocketToWriteEvents(int);
		std::string		httpCodeToString(int) const;
		std::string		selectConnectionValue(void) const;

		bool			buildCgiHttpResponse(
						const std::string &, std::string &) const;
		std::string		serializeCgiHttpResponse(
						const t_CgiParsedResponse &) const;

        State           _state;
        time_t          _timeoutCounter;
        int             _fd;
        Server &        _serv;
        Request         _req;
        Response        _res;
        std::string     _ipClient;
        bool            _bodyTooBig;
        bool            _killswitch;
		bool			_closeAfterResponse;

		CgiProcess		_cgi;
		bool			_hasCgi;
		int				_cgiStdinFd;
		int				_cgiStdoutFd;
		int				_cgiExecFd;
		time_t			_cgiStartTime;
};

void        removeClient(int, int, std::map<int, Client *> &, t_CgiFdMap &);

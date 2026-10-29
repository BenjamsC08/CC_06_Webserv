/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:10 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:11 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fstream>
#include <stdlib.h>
#include "HttpRules.hpp"

enum    Parsing{
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISHED,
};

class Request{

    public:
        Request(size_t max_body);
        Request(const Request & src);
        ~Request();

        Request & operator=(const Request & src);

        void        requestLineParser();
        void        rulefinder(std::map<std::string, HttpRule> &_httpRules);
        void        parseHandler(std::map<std::string, HttpRule> &_httpRules);
        void        headerParser();
        void        bodyParser();
        void        multipartHandle();
        void        uploadHandle();


        const std::map<std::string, std::string> &    getHeader() const;
        std::string                             getHttpversion();
        std::string                             getMethod();
        std::string                             getPath();
        std::string                             getBody();
        std::string                             getRawData();
        std::string                             getQuery();
        std::string                             getErrorCode();
		std::string		                        getHostname() const;
        std::string                             getPathNoPrefix();
        std::string                             getIp();
        std::string                             getRedir();
        bool                                    getIsHeaderFinished();
        bool                                    getIsChunked();
        bool                                    getToClose() const;
        bool                                    getSizeNotFound();
        bool                                    getBodyTooBig();
        bool                                    getCreated();
        bool                                    getToRedir();
        size_t                                  getContentLength();
        int                                     getMaxBodySize();
        int                                     getReadBytes();
        Parsing                                 getState();
        time_t                                  getParsingTimer();
        HttpRule &                              getHttpRule();

        

        void        	reset();
        void        	setState(Parsing toSet);
        void        	setIp(std::string toSet);
        void        	setRawData(std::string data);
        void        	setReadBytes(int toSet);
        void			setIsChunked(bool toSet);
        void            setPath(std::string toSet);
        void            setRedir(std::string toSet);
        void            setCreated(bool toSet);


    private:
        std::map<std::string, std::string>  _header;
        std::string                         _httpVersion;
        std::string                         _method;
        std::string                         _path;
        std::string                         _body;
        std::string                         _rawData;
        std::string                         _query;
        std::string                         _headerContent;
        std::string                         _errorCode;
        std::string                         _redir;
        std::string                         _bodyParsingChunk;
        std::string                         _ipClient;
        std::string                         _requestLineContent;
        std::string                         _hostname;
        std::string                         _pathNoPrefix;
        HttpRule                            _rulematch;
        Parsing                             _state;
        size_t                              _contentLength;
        size_t                              _bodyRead;
        size_t                              _end;
        size_t                              _maxBodySize;
        size_t                              _start;
        time_t                              _parsingTimer;
        bool                                _isChunked;
        bool                                _isHeaderFinished;
        bool                                _isChunkedInProgress;
        bool                                _bodyTooBig;
        bool                                _toClose;
        bool                                _sizeNotFound;
        bool                                _created;
        bool                                _toRedir;
        int                                 _bytesRead;
        int                                 _headerSize;
};

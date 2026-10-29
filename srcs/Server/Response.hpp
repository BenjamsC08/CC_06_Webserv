/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:17 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:19 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <cstdlib>
#include "HttpRules.hpp"
#include "Request.hpp"


class Response{

    public:
        Response();
        Response(const Response & src);
        ~Response();

        Response & operator=(const Response & src);

        std::string getContentType();
        std::string getHeaderBuffer();
        std::string getBodyBuffer();
        std::string getPath();
        std::string getResponse();
        int         getByteSent();
        int         getTotalByte();
        int         getStatusCode();
        int         getFdToWrite();

        void        setResponse(std::string toSet);
        void        setBytesent(int toSet);
        void        setTotalBytes(int toSet);
        void        setConnection(std::string connection);
        void        setErrorPath(std::map<int, std::string> ErrorPath);
        void        handleCookieTheme(Request & request);

        void        prepareResponse(Request & request);
        void        errorConstructors(std::string errorCode, Request & request);
        void        multipartHandle(Request & request);
        void        uploadHandle(Request & request);


        std::string themeInjector(Request & request, std::string body);

        void        galleryHandle(Request & request);
        void        reset();
        std::string AutoIndexCreate(std::string directoryPath, std::string requestPath, Request & request);

    private:
        std::string                 _contentType;
        std::string                 _headerBuffer;
        std::string                 _bodyBuffer;
        std::string                 _path;
        std::string                 _response;
        int                         _bytesSent;
        int                         _totalByte;
        int                         _statusCode;
        int                         _fdToWrite;
        bool                        _bodyTooBig;
        std::string                 _connection;
        std::map<int, std::string>  _errorPath;


};


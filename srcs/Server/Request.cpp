/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:05 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:44:35 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(size_t max_body): _header(),_httpVersion(""),_method(""),_path(""),_body(""),_rawData(""),_query(""),
    _headerContent(""),_errorCode(""),_redir(""),_bodyParsingChunk(""),_ipClient(""),_requestLineContent(""),_hostname(""),
    _pathNoPrefix(""),_rulematch(),_state(REQUEST_LINE),_contentLength(0),_bodyRead(0),_end(0),_maxBodySize(max_body),_start(0),
    _parsingTimer(time(NULL)),_isChunked(false),_isHeaderFinished(false),_isChunkedInProgress(false),_bodyTooBig(false),
    _toClose(false),_sizeNotFound(false),_created(false),_toRedir(false),_bytesRead(0),_headerSize(0)
{
}

Request::Request(const Request & src):_header(src._header),_httpVersion(src._httpVersion),_method(src._method),_path(src._path),_body(src._body),_rawData(src._rawData),_query(src._query),
    _headerContent(src._headerContent),_errorCode(src._errorCode),_redir(src._redir),_bodyParsingChunk(src._bodyParsingChunk),_ipClient(src._ipClient),_requestLineContent(src._requestLineContent),_hostname(src._hostname),
    _pathNoPrefix(src._pathNoPrefix),_rulematch(src._rulematch),_state(src._state),_contentLength(src._contentLength),_bodyRead(src._bodyRead),_end(src._end),_maxBodySize(src._maxBodySize),_start(src._start),
    _parsingTimer(src._parsingTimer),_isChunked(src._isChunked),_isHeaderFinished(src._isHeaderFinished),_isChunkedInProgress(src._isChunkedInProgress),_bodyTooBig(src._bodyTooBig),
    _toClose(src._toClose),_sizeNotFound(src._sizeNotFound),_created(src._created),_toRedir(src._toRedir),_bytesRead(src._bytesRead),_headerSize(src._headerSize)
{}

Request & Request::operator=(const Request & src)
{
    if (this != &src)
    {
        _header = src._header;
        _httpVersion = src._httpVersion;
        _method = src._method;
        _path = src._path;
        _body = src._body;
        _rawData = src._rawData;
        _query = src._query;
        _headerContent = src._headerContent;
        _errorCode = src._errorCode;
        _redir = src._redir;
        _bodyParsingChunk = src._bodyParsingChunk;
        _ipClient = src._ipClient;
        _requestLineContent = src._requestLineContent;
        _hostname = src._hostname;
        _pathNoPrefix = src._pathNoPrefix;
        _rulematch = src._rulematch;
        _state = src._state;
        _contentLength = src._contentLength;
        _bodyRead = src._bodyRead;
        _end = src._end;
        _maxBodySize = src._maxBodySize;
        _start = src._start;
        _parsingTimer = src._parsingTimer;
        _isChunked = src._isChunked;
        _isHeaderFinished = src._isHeaderFinished;
        _isChunkedInProgress = src._isChunkedInProgress;
        _bodyTooBig = src._bodyTooBig;
        _toClose = src._toClose;
        _sizeNotFound = src._sizeNotFound;
        _created = src._created;
        _toRedir = src._toRedir;
        _bytesRead = src._bytesRead;
        _headerSize = src._headerSize;
    }
    return (*this);
}

Request::~Request()
{}

void    Request::rulefinder(std::map<std::string, HttpRule> &_httpRules)
{
    _rulematch = _httpRules["/"];

    size_t bestmatch = 0;
    for (std::map<std::string, HttpRule>::iterator it = _httpRules.begin(); it != _httpRules.end(); ++it)
    {
        const std::string location = it->first;
        if (_path.find(location) == 0)
        {
            if (location.length() > bestmatch)
            {
                bestmatch = location.length();
                _rulematch = it->second;
            }
        }
    }

    if (bestmatch != 1)
        _pathNoPrefix = _path.substr(bestmatch, _path.size() - bestmatch);
    else
        _pathNoPrefix = _path;
}

void        Request::parseHandler(std::map<std::string, HttpRule> &_httpRules)
{
    if (_method.empty() && _path.empty() && _state == REQUEST_LINE)
    {
        requestLineParser();
    }
    if (_state == HEADERS)
    {
        headerParser();
    }
    if (_state == BODY)
    {
        rulefinder(_httpRules);
        if (!_rulematch.getHttpRedir().empty())
        {
            _redir = _rulematch.getHttpRedir();
            _toRedir = true;
            _state = FINISHED;
            return ;
        }

        if (_bodyTooBig)
        { 
            _toClose = true;
            _state = FINISHED;
        	return ;
        }
        else if (_method == "GET" || _method == "DELETE")
        {
            _state = FINISHED;
			return ;
        }
        else if (_method == "POST")
		{
            if (_header.find("content-length") == _header.end() && _isChunked == false)
            {
				_toClose = true;
				_state = FINISHED;
                _sizeNotFound = true;
                _errorCode = "411";
                DEBUG("No content lenght nor chunked found. Error 411\n");
                return ;
            }
            else if (!_rulematch.getMethods().flag.POST_METHOD/* || !_rulematch.isUploadAct()*/)            {
                _state = FINISHED;
                _errorCode = "405";
                DEBUG("Post isn't a Method of the location or Upload is disabled. Error 405\n");
				return ;
            }
            else if ((_body.size() < _contentLength) || (_isChunked == true && _isChunkedInProgress == true))
            {
                bodyParser();
            }

            if (!_body.empty() && _body.size() == _contentLength && _isChunkedInProgress == false)
            {
                _state = FINISHED;
            }
		}
        else
        {
            _state = FINISHED;
            _errorCode = "405";
            DEBUG("Not POST,GET nor DELETE. Error 405\n");
            return ;
        }
    }
}

void        Request::requestLineParser()
{
    _requestLineContent.append(_rawData);
    size_t start = _requestLineContent.find("\r\n");
    if (start == std::string::npos)
        return;

    start = _requestLineContent.find("GET");
    if (start == std::string::npos)
        start = _requestLineContent.find("DELETE");
    if (start == std::string::npos)
        start = _requestLineContent.find("POST");
    if (start == std::string::npos)
    {
        start = 0;
    }

    _state = HEADERS;
    size_t pos = _requestLineContent.find("\r\n");
    std::string request_line = _requestLineContent.substr(start, pos);

    pos = request_line.find(" ");
    _method = request_line.substr(0, pos);

    size_t next_pos = request_line.find(" ", pos + 1);

    _path = request_line.substr(pos + 1, next_pos - (pos + 1));
    _httpVersion = request_line.substr(next_pos + 1, request_line.size());

    pos = _requestLineContent.find("\r\n");
    _rawData = _requestLineContent.substr(pos + 2, _requestLineContent.size() - (pos + 2));
}

void        Request::headerParser()
{
    _headerContent.append(_rawData);
    if (_headerContent.find("\r\n\r\n") == std::string::npos)
    {
        return;
    }
    else
    {
        size_t start = 0;
        size_t end = -2;
        if (start != std::string::npos && end != std::string::npos)
        {
            while (1)
            {
                start = end;
                end = _headerContent.find("\r\n", start + 2);
                std::string header_line = _headerContent.substr(start + 2, (end - 1 ) - (start + 1));
                if (end - (start + 2) == 0)
                {
                    break;
                }
                size_t delim = header_line.find(":");
                std::string key = header_line.substr(0, delim);
                for (size_t i = 0; i < key.length(); i++)
                    key[i] = std::tolower(static_cast<unsigned char>(key[i]));
                _header[key] = header_line.substr(delim + 2, header_line.size() - (delim + 1));
            }
        }

        std::map<std::string, std::string>::iterator it = _header.find("transfer-encoding");
        if (it != _header.end() && it->second == "chunked")
        {    
            _isChunked = true;
            _isChunkedInProgress = true;
        }
        else
        {
            it = _header.find("content-length");
            if (it != _header.end())
                _contentLength = atoi(it->second.c_str());
        }

        start = _rawData.find("\r\n\r\n");
        if (start != std::string::npos)
        {
            _headerSize = start + 4;
            _state = BODY;
            int bodyBytes = _rawData.size() - _headerSize;
            if (bodyBytes > 0 && _isChunked == false)
            {
                _body.append(_rawData, _headerSize, bodyBytes);
                _bodyRead = bodyBytes;
            }
            else if (bodyBytes > 0 && _isChunked == true)
            {
                _bodyParsingChunk.append(_rawData, _headerSize, bodyBytes);
            }
            _rawData.clear();
        }

        _isHeaderFinished = true;
        _state = BODY;
        _headerContent.clear();
        if (_header.find("host") != _header.end())
            _hostname = _header["host"];
        if (_contentLength > _maxBodySize || _body.size() > _maxBodySize)
        {
            _bodyTooBig = true;
            _state = FINISHED;
            _errorCode = "413";
            DEBUG("Content Lenght is bigger than body size, error found during header parsing. Error 413\n");
            return;
        }

    }
}
void        Request::bodyParser()
{
    if (_isChunked == true && _isChunkedInProgress == true)
    {
        size_t size = 0;
        _bodyParsingChunk.append(_rawData);
        if (_bodyParsingChunk.find("\r\n0\r\n\r\n") == std::string::npos)
            return;

        _start = 0;
        _end = _bodyParsingChunk.find("\r\n");

        while (42)
        {
            size = std::strtol(_bodyParsingChunk.substr(_start, _end).c_str(), NULL, 16);
            _start = _end + 2;
            
            if (size == 0 && _bodyParsingChunk.find("0\r\n\r\n") != std::string::npos)
            {
                _state = FINISHED;
                break;
            }
            _body += _bodyParsingChunk.substr(_start, size);

            if (_body.size() > _maxBodySize)
            {
                _state = FINISHED;
                _bodyTooBig = true;
                _errorCode = "413";
                DEBUG("Chunk body size ends up being bigger than max body size. Error 413\n");
                return;
            }

            _start = _bodyParsingChunk.find("\r\n", _start) + 2;
            _end = _bodyParsingChunk.find("\r\n", _start);
            if (_end == std::string::npos)
                break;
        }
    }
    else if (_isChunked == false)
    {
        size_t remaining = _contentLength - _bodyRead;
        if ( _rawData.size() > remaining )
        {
            _state = FINISHED;
            _toClose = true;
            _errorCode = "400";
            DEBUG("Trying to send more data than what's in content lenght. Error 400\n");
            return;
        }

        if (_bodyRead + _rawData.size() > _maxBodySize)
        {
            _state = FINISHED;
            _errorCode = "413";
            DEBUG("Body size ends up bigger than max body size. Error 413\n");
            return;
        }

        _body.append(_rawData);
        _bodyRead += _rawData.size();
        _rawData.clear();

        if (_bodyRead == _contentLength)
            _state = FINISHED;
    }
}


const std::map<std::string, std::string> & Request::getHeader() const
{
    return(_header);
}

std::string Request::getHttpversion()
{
    return(_httpVersion);
}

std::string Request::getMethod()
{
    return(_method);
}

std::string Request::getPath()
{
    return(_path);
}

std::string Request::getBody()
{
    return(_body);
}

std::string Request::getQuery()
{
    return(_query);
}

bool        Request::getIsHeaderFinished()
{
    return(_isHeaderFinished);
}

bool        Request::getIsChunked()
{
    return(_isChunked);
}

std::string   Request::getErrorCode()
{
    return(_errorCode);
}

size_t         Request::getContentLength()
{
    return(_contentLength);
}

int         Request::getMaxBodySize()
{
    return(_maxBodySize);
}

std::string Request::getRawData()
{
    return(_rawData);
}

Parsing     Request::getState()
{
    return(_state);
}

void        Request::setRawData(std::string data)
{
    _rawData.assign(data);
}

void       Request::setPath(std::string toSet)
{
    _path = toSet;
}


void        Request::setState(Parsing toSet)
{
    _state = toSet;
}

void        Request::reset()
{
    Request empty(_maxBodySize);

    *this = empty;
}

void        Request::setReadBytes(int toSet)
{
    _bytesRead = toSet;
}

int         Request::getReadBytes()
{
    return(_bytesRead);
}
std::string    Request::getIp()
{
    return(_ipClient);
}
void        Request::setIp(std::string toSet)
{
    _ipClient = toSet;
}

std::string     Request::getRedir()
{
    return (_redir);
}

void        Request::setRedir(std::string ToSet)
{
    _redir = ToSet;
}

void            Request::setCreated(bool toSet)
{
    _created = toSet;
}

void        Request::setIsChunked(bool toSet)
{
    _isChunked = toSet;
}

time_t          Request::getParsingTimer()
{
    return(_parsingTimer);
}

bool            Request::getBodyTooBig()
{
    return(_bodyTooBig);
}

bool            Request::getCreated()
{
    return(_created);
}
bool            Request::getToRedir()
{
    return(_toRedir);
}

std::string	Request::getHostname() const
{
	return(_hostname);
}

HttpRule & Request::getHttpRule()
{
    return (_rulematch);
}
std::string   Request::getPathNoPrefix()
{
    return(_pathNoPrefix);
}

bool   Request::getToClose() const
{
    return(_toClose);
}

bool    Request::getSizeNotFound()
{
    return(_sizeNotFound);
}

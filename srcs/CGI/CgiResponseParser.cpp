/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiResponseParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 01:50:14 by mkerrien          #+#    #+#             */
/*   Updated: 2026/04/28 06:26:15 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiResponseParser.hpp"
#include "WebServ.hpp"

static bool	isSameHeaderName(const std::string &headerName, const std::string &toFind);
static bool	findHeaderSeparator(const std::string &raw,
							size_t &pos, size_t &sep_len);
static bool	parseOneCgiHeaderLine(const std::string &line,
				t_CgiParsedResponse &parsed);
static bool	parseHeaderLine(const std::string &raw_line, t_CgiHeader &header);
static bool	isHeaderNameChar(char c);
static bool	hasForbiddenHeaderValue(const std::string &value);
static bool	shouldSkipCgiHeader(const std::string &name);
static bool	parseStatusHeader(const std::string &value,
				int &code, std::string &reason);
static std::string	trimHeaderValue(const std::string &s);
static std::string	getDefaultReasonPhrase(int code);
static std::string	normalizeStringToLower(const std::string &toNormalize);

bool	CgiResponseParser::parse(const std::string &raw,
								t_CgiParsedResponse &parsed)
{
	size_t		header_end;
	size_t		sep_len;
	std::string	headers_part;
	std::string	line;
	size_t		next;

	parsed.status_code = 200;
	parsed.reason_phrase = "OK";
	parsed.headers.clear();
	parsed.body.clear();
	parsed.has_status = false;
	parsed.has_location = false;
	parsed.has_content_type = false;

	if (!findHeaderSeparator(raw, header_end, sep_len))
		return (false);
	headers_part = raw.substr(0, header_end);
	parsed.body = raw.substr(header_end + sep_len);

	for (size_t pos = 0; pos < headers_part.size();)
	{
		next = headers_part.find('\n', pos);
		if (next == std::string::npos)
			next = headers_part.size();
		line = headers_part.substr(pos, next - pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (!line.empty())
		{
			if (!parseOneCgiHeaderLine(line, parsed))
				return (false);
		}
		pos = next + 1;
	}
	if (!parsed.has_content_type && !parsed.has_location)
		return (false);
	return (true);

}

static bool	findHeaderSeparator(const std::string &raw,
							size_t &pos, size_t &sep_len)
{
	if (raw.empty())
		return (false);
	pos = raw.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		sep_len = 4;
		return (true);
	}
	pos = raw.find("\n\n");
	if (pos != std::string::npos)
	{
		sep_len = 2;
		return (true);
	}
	pos = 0;
	sep_len = 0;
	return (false);
}

static bool	parseOneCgiHeaderLine(const std::string &line,
				t_CgiParsedResponse &parsed)
{
	t_CgiHeader	header;
	int			status_code = 0;
	std::string	reason;

	if (!parseHeaderLine(line, header))
		return (false);
	if (isSameHeaderName(header.name, "Status"))
	{
		if (!parseStatusHeader(header.value, status_code, reason))
			return (false);
		parsed.status_code = status_code;
		parsed.reason_phrase = reason;
		parsed.has_status = true;
		return (true);
	}
	if (isSameHeaderName(header.name, "Content-Type"))
		parsed.has_content_type = true;
	if (isSameHeaderName(header.name, "Location"))
	{
		parsed.has_location = true;
		if (!parsed.has_status)
		{
			parsed.status_code = 302;
			parsed.reason_phrase = "Found";
		}
	}
	if (!shouldSkipCgiHeader(header.name)
		&& !isSameHeaderName(header.name, "Content-Length"))
		parsed.headers.push_back(header);
	return (true);
}

static bool	parseHeaderLine(const std::string &raw_line, t_CgiHeader &header)
{
	std::string	line;
	std::string	name;
	std::string	value;
	size_t		colon_pos;

	line = raw_line;
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	if (line.empty())
		return (false);
	if (line[0] == ' ' || line[0] == '\t')
		return (false);
	colon_pos = line.find(':');
	if (colon_pos == std::string::npos || colon_pos == 0)
		return (false);
	name = line.substr(0, colon_pos);
	for (size_t i = 0; i < name.size(); ++i)
	{
		if (!isHeaderNameChar(name[i]))
			return (false);
	}
	value = line.substr(colon_pos + 1);
	value = trimHeaderValue(value);
	if (hasForbiddenHeaderValue(value))
		return (false);
	header.name = name;
	header.value = value;
	return (true);
}

static bool	isHeaderNameChar(char c)
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
	|| (c >= '0' && c <= '9') || (c == '!' || c == '#' || c == '$' || c == '%')
	|| (c == '&' || c == '\'' || c == '*') || (c == '+' || c == '-' || c == '.')
	|| (c == '^' || c == '_' || c == '`') || (c == '|' || c == '~'))
		return (true);
	return (false);
}

static bool	hasForbiddenHeaderValue(const std::string &value)
{
	unsigned char	c;

	for (size_t i = 0; i < value.size(); ++i)
	{
		c = static_cast<unsigned char>(value[i]);
		if (c == '\r' || c == '\n' || c == '\0'
		|| c == 127 || (c < 32 && c != '\t'))
			return (true);
	}
	return (false);
}

static bool	shouldSkipCgiHeader(const std::string &name)
{
	std::string	lower;

	lower = normalizeStringToLower(name);
	if (lower == "connection")
		return (true);
	if (lower == "keep-alive")
		return (true);
	if (lower == "transfer-encoding")
		return (true);
	if (lower == "te")
		return (true);
	if (lower == "trailer")
		return (true);
	if (lower == "upgrade")
		return (true);
	if (lower == "proxy-authenticate")
		return (true);
	if (lower == "proxy-authorization")
		return (true);
	return (false);
}

static bool	parseStatusHeader(const std::string &value,
				int &code, std::string &reason)
{
	std::string	tmp;

	tmp = trimHeaderValue(value);
	if (tmp.size() < 3)
		return (false);
	if (!std::isdigit(tmp[0]) || !std::isdigit(tmp[1])
		|| !std::isdigit(tmp[2]))
		return (false);
	code = (tmp[0] - '0') * 100 + (tmp[1] - '0') * 10 + (tmp[2] - '0');
	if (code < 100 || code > 599)
		return (false);
	if (tmp.size() > 3)
		reason = trimHeaderValue(tmp.substr(3));
	else
		reason = getDefaultReasonPhrase(code);
	if (reason.empty())
		reason = getDefaultReasonPhrase(code);
	return (true);
}

static std::string	trimHeaderValue(const std::string &s)
{
	size_t	start;
	size_t	end;

	start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t'))
		++start;
	if (start == s.size())
		return ("");
	end = s.size() - 1;
	while (end > start && (s[end] == ' ' || s[end] == '\t'))
		--end;
	return (s.substr(start, end - start + 1));
}

static std::string	getDefaultReasonPhrase(int code)
{
	switch (code)
	{
		case 100:
			return ("Continue");
		case 101:
			return ("Switching Protocols");

		case 200:
			return ("OK");
		case 201:
			return ("Created");
		case 202:
			return ("Accepted");
		case 204:
			return ("No Content");

		case 301:
			return ("Moved Permanently");
		case 302:
			return ("Found");
		case 303:
			return ("See Other");
		case 304:
			return ("Not Modified");
		case 307:
			return ("Temporary Redirect");
		case 308:
			return ("Permanent Redirect");

		case 400:
			return ("Bad Request");
		case 401:
			return ("Unauthorized");
		case 403:
			return ("Forbidden");
		case 404:
			return ("Not Found");
		case 405:
			return ("Method Not Allowed");
		case 408:
			return ("Request Timeout");
		case 413:
			return ("Payload Too Large");
		case 415:
			return ("Unsupported Media Type");
		case 418:
			return ("I'm a teapot");

		case 500:
			return ("Internal Server Error");
		case 501:
			return ("Not Implemented");
		case 502:
			return ("Bad Gateway");
		case 503:
			return ("Service Unavailable");
		case 504:
			return ("Gateway Timeout");
	}
	return ("Unknown");
}

static bool	isSameHeaderName(const std::string &headerName, const std::string &toFind)
{
	std::string	normalizedHeader;
	std::string	normalizedToFind;

	if (headerName.empty() || toFind.empty())
		return (false);
	normalizedHeader = normalizeStringToLower(headerName);
	normalizedToFind = normalizeStringToLower(toFind);
	return (normalizedHeader == normalizedToFind);
}

static std::string	normalizeStringToLower(const std::string &toNormalize)
{
	std::string	normalized;
	char		c;

	for (size_t i = 0; i < toNormalize.size(); ++i)
	{
		c = toNormalize[i];
		if (c >= 'A' && c <= 'Z')
			normalized += c + 32;
		else
			normalized += c;
	}
	return (normalized);
}

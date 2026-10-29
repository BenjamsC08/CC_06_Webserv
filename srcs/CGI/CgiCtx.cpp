/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiCtx.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/27 07:00:05 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 02:01:11 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstddef>
#include <exception>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include "CgiCtx.hpp"
#include "Request.hpp"
#include "Server.hpp"


CgiCtx::CgiCtx(Request &request, Server &server) :
	_request(request), _server(server), _status(CGI_CTX_EMPTY)
{
	this->_location._httpRules = NULL;
}

CgiCtx::~CgiCtx(void) {}


const t_CgiLaunchInfos	&CgiCtx::getLaunchInfos(void) const
{
	return (this->_launchInfos);
}

t_CgiCtxStatus	CgiCtx::getStatus(void) const
{
	return (this->_status);
}

bool	CgiCtx::setStatus(t_CgiCtxStatus newStatus)
{
	this->_status = newStatus;
	return (true);
}

bool	CgiCtx::setError(t_CgiCtxStatus newStatus)
{
	this->logError(newStatus);
	this->_status = newStatus;
	return (false);
}

void	CgiCtx::logError(t_CgiCtxStatus status) const
{
	switch (status)
	{
		case CGI_CTX_BAD_TARGET:
			g_Log << "[w] CGI: Invalid request target for CGI resolution ('"
				<< this->_request.getPath() << "')\n";
			break;
		case CGI_CTX_BAD_URI_SUFFIX:
			g_Log << "[w] CGI: Invalid URI suffix while resolving CGI target\n";
			break;
		case CGI_CTX_NO_LOCATION:
			g_Log << "[w] CGI: No matching location found for CGI request\n";
			break;
		case CGI_CTX_SCRIPT_NOT_FOUND:
			g_Log << "[w] CGI: Script file was not found\n";
			break;
		case CGI_CTX_METHOD_NOT_ALLOWED:
			g_Log << "[w] CGI: Request method '" << this->_request.getMethod()
				<< "' is not allowed for this CGI location ('"
				<< this->_location._locationPath << "')\n";
			break;
		case CGI_CTX_SCRIPT_FORBIDDEN:
			g_Log << "[w] CGI: Script file '"
				<< this->_resolvedTarget._script_path
				<< "' exists but is not readable\n";
			break;
		case CGI_CTX_ERR_BAD_LOCATION_ROOT:
			g_Log << "[e] CGI: Location root is missing, invalid, or unsafe ('"
				<< this->_location._httpRules->getRootPath() << "')\n";
			break;
		case CGI_CTX_ERR_TARGET_RESOLUTION:
			g_Log << "[e] CGI: Failed to resolve request target to filesystem "
					"path\n";
			break;
		case CGI_CTX_ERR_BAD_CGI_INTERPRETER:
			g_Log << "[e] CGI: CGI interpreter path is missing, invalid, or "
					"not executable\n";
			break;
		case CGI_CTX_ERR_INTERNAL:
			g_Log << "[e] CGI: Internal error while building CGI context\n";
			break;
		default:
			break;
	}
}

bool	CgiCtx::isNotFatalStatus(void) const
{
	switch (this->_status)
	{
		case CGI_CTX_ERR_BAD_LOCATION_ROOT:
		case CGI_CTX_ERR_TARGET_RESOLUTION:
		case CGI_CTX_ERR_BAD_CGI_INTERPRETER:
		case CGI_CTX_ERR_INTERNAL:
			return (false);
		default:
			return (true);
	}
}





bool	CgiCtx::createCgiCtx(void)
{
	try
	{
		this->setStatus(CGI_CTX_BUILDING);
		this->resetResolvedBase();
		this->resetResolvedTarget();
		this->resetContext();
		this->resetLaunchInfos();

		if (!this->parseRawTargetOriginForm())
			return (this->isNotFatalStatus());
		if (!this->findBestLocation())
			return (this->isNotFatalStatus());
		if (!this->resolveLocationFilesystemBase())
			return (this->isNotFatalStatus());
		if (!this->resolveCgiScriptTarget())
			return (this->isNotFatalStatus());
		if (!this->isAccessibleScript())
			return (this->isNotFatalStatus());
		if (!this->isAllowedMethod())
			return (this->isNotFatalStatus());
		if (!this->buildLaunchInfos())
			return (this->isNotFatalStatus());
		return (this->setStatus(CGI_CTX_READY));
	}
	catch (const std::exception &e)
	{ return (this->setError(CGI_CTX_ERR_INTERNAL)); }
}

/*
 * - Check the syntax validity, then try to locate and isolate the URI path and
 *   the query string from the raw target path. Query string can finally be "".
 * - Return false if raw target path is not valid, or true if an URI
 *   path can be found.
*/
bool	CgiCtx::parseRawTargetOriginForm(void)
{
	size_t	queryMark;
	const std::string	&rawTarget = this->_request.getPath();

	this->_target._uri_path.clear();
	this->_target._query_string.clear();

	if (rawTarget.empty() || rawTarget[0] != '/'
		|| rawTarget.find('#') != std::string::npos || hasCtlOrSpace(rawTarget))
		return (this->setError(CGI_CTX_BAD_TARGET));
	queryMark = rawTarget.find('?');
	if (queryMark == std::string::npos)
	{
		this->_target._uri_path = rawTarget;
		return (true);
	}
	this->_target._uri_path = rawTarget.substr(0, queryMark);
	this->_target._query_string = rawTarget.substr(queryMark + 1);
	return (true);
}

bool	CgiCtx::findBestLocation(void)
{
	const std::map<std::string, HttpRule>		&rulesPerLocation =
										this->_server.getConf().getHttpRules();
	std::map<std::string, HttpRule>::const_iterator	it;
	size_t										bestMatchLength = 0;

	this->_location._locationPath.clear();
	this->_location._httpRules = NULL;

	for (it = rulesPerLocation.begin(); it != rulesPerLocation.end(); it++)
	{
		if (doesLocationMatchUriPath(it->first, this->_target._uri_path))
		{
			if (it->first.length() > bestMatchLength)
			{
				bestMatchLength = it->first.length();
				this->_location._locationPath = it->first;
				this->_location._httpRules = &it->second;
			}
		}
	}
	if (!this->_location._httpRules)
		return (this->setError(CGI_CTX_NO_LOCATION));
	return (true);
}

void	CgiCtx::resetLaunchInfos(void)
{
	this->_launchInfos._program_path.clear();
	this->_launchInfos._working_directory.clear();
	this->_launchInfos._script_path.clear();
	this->_launchInfos._argv.clear();
	this->_launchInfos._env_lines.clear();
	this->_launchInfos._stdin_data.clear();
}

void	CgiCtx::resetContext(void)
{
	this->_context._request_method.clear();
	this->_context._query_string.clear();
	this->_context._server_protocol.clear();
	this->_context._server_port.clear();
	this->_context._server_name.clear();
	this->_context._remote_addr.clear();
	this->_context._script_name.clear();
	this->_context._path_info.clear();
	this->_context._content_type.clear();
	this->_context._content_length.clear();
}

void	CgiCtx::resetResolvedBase(void)
{
	this->_resolvedBase._location_prefix_uri.clear();
	this->_resolvedBase._uri_suffix.clear();
	this->_resolvedBase._filesystem_base.clear();
	this->_resolvedBase._normalized_suffix_segments.clear();
}

void	CgiCtx::resetResolvedTarget(void)
{
	this->_resolvedTarget._script_path.clear();
	this->_resolvedTarget._script_local_path.clear();
	this->_resolvedTarget._script_uri.clear();
	this->_resolvedTarget._path_info.clear();
	this->_resolvedTarget._extension.clear();
}

bool	CgiCtx::resolveLocationFilesystemBase(void)
{
	this->resetResolvedBase();

	if (this->_location._httpRules == NULL)
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (this->_location._locationPath.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (!this->extractSuffixUriFromLocation())
		return (false);
	if (!this->getLocationFilesystemBase())
		return (false);
	this->_resolvedBase._location_prefix_uri = this->_location._locationPath;
	return (true);
}

/*
	Extraire le suffixe de URI grace a la location deja matchee :
		- URI = "/cgi-bin/tools/foo/script.py"
		- Best location match (prefix) = "/cgi-bin/tools/"
		- Donc suffixe = "foo/script.py"
*/
bool	CgiCtx::extractSuffixUriFromLocation(/*std::string &suffix*/void)
{
	std::string	uri = this->_target._uri_path;
	std::string	prefix = this->_location._locationPath;
	size_t		i = 0;

	if (!doesLocationMatchUriPath(prefix, uri))
	return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_resolvedBase._uri_suffix.clear();
	if (prefix.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_resolvedBase._uri_suffix = uri.substr(prefix.length());
	while (i < this->_resolvedBase._uri_suffix.length()
			&& this->_resolvedBase._uri_suffix[i] == '/')
		++i;
	this->_resolvedBase._uri_suffix.erase(0, i);
	return (true);
}

bool	CgiCtx::getLocationFilesystemBase(void)
{
	std::string	base = this->_location._httpRules->getRootPath();

	if (base.empty())
		return (this->setError(CGI_CTX_ERR_BAD_LOCATION_ROOT));
	if (!this->validateAndNormalizeRoot(base))
		return (false);
	this->_resolvedBase._filesystem_base = base;
	return (true);
}

bool	CgiCtx::resolveCgiScriptTarget(void)
{
	std::string	diskPath;
	std::string	extension;
	t_PathKind	pathKind;
	size_t		i = 0;
	size_t		count;

	this->resetResolvedTarget();
	if (this->_resolvedBase._filesystem_base.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (this->_resolvedBase._location_prefix_uri.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (!this->buildNormalizedSuffixSegments())
		return (false);
	if (this->_resolvedBase._normalized_suffix_segments.empty())
		return (this->setError(CGI_CTX_NOT_CGI));	
	count = this->_resolvedBase._normalized_suffix_segments.size();
	
	while (i < count)
	{
		diskPath = this->buildScriptPathFromBaseAndCount(i + 1);
		pathKind = getPathKind(diskPath);
		if (pathKind == PATH_KIND_ERROR)
			return (this->setError(CGI_CTX_ERR_TARGET_RESOLUTION));
		if (pathKind == PATH_KIND_MISSING)
		{
			i++;
			continue;
		}
		if (pathKind == PATH_KIND_DIRECTORY)
		{
			i++;
			continue;
		}
		if (pathKind == PATH_KIND_OTHER)
			return (this->setError(CGI_CTX_NOT_CGI));
		extension = this->getFileExtension(diskPath);
		if (!this->isConfiguredCgiExtension(extension))
			return (this->setError(CGI_CTX_NOT_CGI));
		this->_resolvedTarget._script_path = diskPath;
		this->_resolvedTarget._script_uri =
			this->buildUriFromPrefixAndCount(i + 1);
		this->_resolvedTarget._path_info = this->buildPathInfoFromIndex(i + 1);
		this->_resolvedTarget._extension = extension;
		return (true);
	}
	return (this->setError(CGI_CTX_NOT_CGI));
}

bool	CgiCtx::buildNormalizedSuffixSegments(void)
{
	std::vector<std::string>	parts;
	std::vector<std::string>	stack;
	size_t						i = 0;

	this->_resolvedBase._normalized_suffix_segments.clear();
	splitPathsegments(this->_resolvedBase._uri_suffix, parts);
	while (i < parts.size())
	{
		if (parts[i] == ".")
		{ ; }
		else if (parts[i] == "..")
		{
			if (stack.empty())
				return (this->setError(CGI_CTX_BAD_URI_SUFFIX));
			stack.pop_back();
		}
		else
			stack.push_back(parts[i]);
		i++;
	}
	this->_resolvedBase._normalized_suffix_segments = stack;
	return (true);
}

std::string	CgiCtx::buildScriptPathFromBaseAndCount(size_t count) const
{
	std::vector<std::string>	baseParts;
	std::vector<std::string>	allParts;
	size_t						i = 0;
	
	splitConfiguredRoot(this->_resolvedBase._filesystem_base, baseParts);
	allParts = baseParts;
	while (i < count && i < this->_resolvedBase._normalized_suffix_segments.size())
	{
		allParts.push_back(this->_resolvedBase._normalized_suffix_segments[i]);
		i++;
	}
	return (joinRelativeSegments(allParts));
}

std::string	CgiCtx::buildUriFromPrefixAndCount(size_t count) const
{
	std::vector<std::string>	prefixParts;
	std::vector<std::string>	allParts;
	size_t						i = 0;

	splitPathsegments(this->_resolvedBase._location_prefix_uri, prefixParts);
	allParts = prefixParts;
	while (i < count && i < this->_resolvedBase._normalized_suffix_segments.size())
	{
		allParts.push_back(this->_resolvedBase._normalized_suffix_segments[i]);
		i++;
	}
	return (joinUriSegments(allParts));
}

std::string	CgiCtx::buildPathInfoFromIndex(size_t startIndex) const
{
	std::vector<std::string>	parts;
	size_t						i = startIndex;
	if (startIndex >= this->_resolvedBase._normalized_suffix_segments.size())
		return ("");
	while (i < this->_resolvedBase._normalized_suffix_segments.size())
	{
		parts.push_back(this->_resolvedBase._normalized_suffix_segments[i]);
		i++;
	}
	return (joinUriSegments(parts));
}

std::string	CgiCtx::getFileExtension(const std::string &path) const
{
	size_t	lastSlash = path.rfind('/');
	size_t	lastDot = path.rfind('.');

	if (lastDot == std::string::npos)
		return ("");
	if (lastSlash != std::string::npos && lastDot < lastSlash)
		return ("");
	return (path.substr(lastDot));
}

bool	CgiCtx::isConfiguredCgiExtension(const std::string &ext) const
{
	const std::map<std::string, std::string> &cgis =
		this->_location._httpRules->getCgis();

	if (cgis.find(ext) != cgis.end())
		return (true);
	return (false);
}

bool	CgiCtx::isAccessibleScript(void)
{
	if (access(this->_resolvedTarget._script_path.c_str(), R_OK) == -1)
		return (this->setError(CGI_CTX_SCRIPT_FORBIDDEN));
	return (true);
}

bool	CgiCtx::isAllowedMethod(void)
{
	std::string			method;

	if (this->_location._httpRules == NULL)
		return (this->setError(CGI_CTX_ERR_INTERNAL));

	method = this->_request.getMethod();

	if (method == "GET"
			&& this->_location._httpRules->getMethods().flag.GET_METHOD == 1)
		return (true);
	if (method == "POST"
			&& this->_location._httpRules->getMethods().flag.POST_METHOD == 1)
		return (true);
	if (method == "DELETE"
			&& this->_location._httpRules->getMethods().flag.DELETE_METHOD == 1)
		return (true);
	return (this->setError(CGI_CTX_METHOD_NOT_ALLOWED));
}

bool	CgiCtx::setLaunchScriptPath(void)
{
	if (this->_resolvedTarget._script_local_path.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_launchInfos._script_path = this->_resolvedTarget._script_local_path;
	return (true);
}

bool	CgiCtx::setCgiInterpretorPath(void)
{
	const std::map<std::string, std::string>			&cgis =
		this->_location._httpRules->getCgis();
	std::map<std::string, std::string>::const_iterator	it;

	if (this->_resolvedTarget._extension.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	it = cgis.find(this->_resolvedTarget._extension);
	if (it == cgis.end())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (it->second.empty())
		return (this->setError(CGI_CTX_ERR_BAD_CGI_INTERPRETER));
	this->_launchInfos._program_path = it->second;
	return (this->validateCgiInterpretorPath());
}

bool	CgiCtx::validateCgiInterpretorPath(void)
{
	t_PathKind	pathKind;

	if (this->_launchInfos._program_path.empty())
		return (this->setError(CGI_CTX_ERR_BAD_CGI_INTERPRETER));
	if (this->_launchInfos._program_path[0] != '/')
		return (this->setError(CGI_CTX_ERR_BAD_CGI_INTERPRETER));
	pathKind = getPathKind(this->_launchInfos._program_path);
	if (pathKind != PATH_KIND_REGULAR)
		return (this->setError(CGI_CTX_ERR_BAD_CGI_INTERPRETER));
	if (access(this->_launchInfos._program_path.c_str(), X_OK) == -1)
		return (this->setError(CGI_CTX_ERR_BAD_CGI_INTERPRETER));
	return (true);
}

bool	CgiCtx::validateAndNormalizeRoot(std::string &root)
{
	std::vector<std::string>	parts;
	std::vector<std::string>	stack;
	size_t						i = 0;

	if (root.empty() || root.length() < 1)
		return (this->setError(CGI_CTX_ERR_BAD_LOCATION_ROOT));

	if (root[0] == '.' && root.length() >= 2 && root[1] != '/')
		return (this->setError(CGI_CTX_ERR_BAD_LOCATION_ROOT));
	else if (root[0] != '.' && root[0] != '/')
		return (this->setError(CGI_CTX_ERR_BAD_LOCATION_ROOT));

	splitPathsegments(root, parts);
	while (i < parts.size())
	{
		if (parts[i] == ".")
		{ ; }
		else if (parts[i] == "..")
		{
			if (stack.size() <= 1)
				return (this->setError(CGI_CTX_BAD_URI_SUFFIX));
			stack.pop_back();
		}
		else
			stack.push_back(parts[i]);
		i++;
	}
	root = joinRelativeSegments(stack);
	return (true);
}

bool	CgiCtx::setCgiWorkingDirectory(void)
{
	size_t		lastSlash = 0;
	std::string	scriptPath = this->_resolvedTarget._script_path;

	if (scriptPath.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	lastSlash = scriptPath.rfind('/');
	if (lastSlash == std::string::npos)
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (lastSlash == 0)
		this->_launchInfos._working_directory = "/";
	else
		this->_launchInfos._working_directory = scriptPath.substr(0, lastSlash);
	return (true);
}

bool	CgiCtx::setScriptLocalPath(void)
{
	std::string	scriptPath = this->_resolvedTarget._script_path;
	std::string	cwd = this->_launchInfos._working_directory;

	this->_resolvedTarget._script_local_path.clear();
	if (scriptPath.empty() || cwd.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_resolvedTarget._script_local_path = "./" + scriptPath.substr(cwd.length() + 1);
	return (true);
}

bool	CgiCtx::buildArgv(void)
{
	try
	{
		if (this->_launchInfos._program_path.empty())
			return (this->setError(CGI_CTX_ERR_INTERNAL));
		if (this->_launchInfos._script_path.empty())
			return (this->setError(CGI_CTX_ERR_INTERNAL));
		this->_launchInfos._argv.clear();
		this->_launchInfos._argv.push_back(this->_launchInfos._program_path);
		this->_launchInfos._argv.push_back(this->_launchInfos._script_path);
	}
	catch (const std::exception &e)
	{
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	}
	return (true);
}

bool	CgiCtx::buildLaunchInfos(void)
{
	if (!this->setCgiInterpretorPath())
		return (false);
	if (!this->setCgiWorkingDirectory())
		return (false);
	if (!this->setScriptLocalPath())
		return (false);
	if (!this->setLaunchScriptPath())
		return (false);
	if (!this->buildArgv())
		return (false);
	if (!this->buildRequestContext())
		return (false);
	if (!this->buildEnvLines())
		return (false);
	if (!this->buildStdinData())
		return (false);
	return (true);
}

bool	CgiCtx::buildRequestContext(void)
{
	this->resetContext();
	if (!this->setRequestMethod())
		return (false);
	if (!this->setQueryString())
		return (false);
	if (!this->setServerProtocol())
		return (false);
	if (!this->setServerPort())
		return (false);
	if (!this->setServerName())
		return (false);
	if (!this->setRemoteAddr())
		return (false);
	if (!this->setScriptName())
		return (false);
	if (!this->setPathInfo())
		return (false);
	if (!this->setContentType())
		return (false);
	if (!this->setContentLength())
		return (false);
	return (true);
}

bool	CgiCtx::setRequestMethod(void)
{
	if (this->_request.getMethod().empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_context._request_method = this->_request.getMethod();
	return (true);
}

bool	CgiCtx::setQueryString(void)
{
	this->_context._query_string = this->_target._query_string;
	return (true);
}

bool	CgiCtx::setServerProtocol(void)
{
	if (this->_request.getHttpversion().empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_context._server_protocol = this->_request.getHttpversion();
	return (true);
}

bool	CgiCtx::setScriptName(void)
{
	if (this->_resolvedTarget._script_uri.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_context._script_name = this->_resolvedTarget._script_uri;
	return (true);
}

bool	CgiCtx::setPathInfo(void)
{
	this->_context._path_info = this->_resolvedTarget._path_info;
	return (true);
}

bool	CgiCtx::setContentType(void)
{
	std::map<std::string, std::string>::const_iterator	it;
	const std::map<std::string, std::string>			&headers =
		this->_request.getHeader();

	this->_context._content_type.clear();
	it = headers.find("content-type");
	if (it != headers.end())
		this->_context._content_type = it->second;
	return (true);
}

bool	CgiCtx::setContentLength(void)
{
	size_t				bodySize = this->_request.getBody().size();
	std::ostringstream	oss;

	this->_context._content_length.clear();
	oss << bodySize;
	this->_context._content_length = oss.str();
	return (true);
}

bool	CgiCtx::setRemoteAddr(void)
{
	if (this->_request.getIp().empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	this->_context._remote_addr = this->_request.getIp();
	return (true);
}

static bool	isValidPort(std::string s)
{
	std::istringstream	iss;
	long				value;

	if (s.empty())
		return (false);
	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] > '9' || s[i] < '0')
			return (false);
	}
	iss.str(s);
	iss >> value;
	if (iss.fail() || !iss.eof())
		return (false);
	if (value < 1  || value > 65535)
		return (false);
	return (true);
}

bool	CgiCtx::setServerPort(void)
{
	size_t				colonMark;
	std::string			candidate;
	long				fallback;
	std::ostringstream	oss;

	if (!this->_request.getHostname().empty())
	{
		colonMark = this->_request.getHostname().rfind(':');
		if (colonMark != std::string::npos)
			candidate = this->_request.getHostname().substr(colonMark + 1);
		if (isValidPort(candidate))
		{
			this->_context._server_port = candidate;
			return (true);
		}
	}
	if (!this->_server.getPorts().empty())
	{
		fallback = static_cast<long>(this->_server.getPorts()[0]);
		if (fallback < 1 || fallback > 65535)
			return (this->setError(CGI_CTX_ERR_INTERNAL));
		oss << fallback;
		this->_context._server_port = oss.str();
		return (true);
	}
	return (this->setError(CGI_CTX_ERR_INTERNAL));
}

bool	CgiCtx::setServerName(void)
{
	this->_context._server_name.clear();
	this->_context._server_name = this->_server.getConf().getServName();
	return (true);
}

bool	CgiCtx::appendEnvLine(const std::string &name, const std::string &value)
{
	std::string	line;

	if (name.empty() || name.find('=') != std::string::npos)
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	try
	{
		line = name + "=" + value;
		this->_launchInfos._env_lines.push_back(line);
	}
	catch (const std::exception &e)
	{ return (this->setError(CGI_CTX_ERR_INTERNAL)); }
	return (true);
}

bool	CgiCtx::appendMandatoryEnvLines(void)
{

	if (!this->appendEnvLine("GATEWAY_INTERFACE", "CGI/1.1"))
		return (false);
	if (!this->appendEnvLine("REQUEST_METHOD",
							this->_context._request_method))
		return (false);
	if (!this->appendEnvLine("QUERY_STRING",
							this->_context._query_string))
		return (false);
	if (!this->appendEnvLine("SERVER_PROTOCOL",
							this->_context._server_protocol))
		return (false);
	if (!this->appendEnvLine("SERVER_PORT",
							this->_context._server_port))
		return (false);
	if (!this->appendEnvLine("SERVER_NAME",
							this->_context._server_name))
		return (false);
	if (!this->appendEnvLine("SERVER_SOFTWARE", "Ratserv/42.0"))
		return (false);
	if (!this->appendEnvLine("REMOTE_ADDR",
							this->_context._remote_addr))
		return (false);
	if (!this->appendEnvLine("SCRIPT_NAME",
							this->_context._script_name))
		return (false);
	if (!this->appendEnvLine("SCRIPT_FILENAME",
				this->_launchInfos._script_path))
		return (false);
	if (!this->appendEnvLine("PATH_INFO",
							this->_context._path_info))
		return (false);
	if (!this->appendEnvLine("REDIRECT_STATUS", "200"))
		return (false);
	return (true);
}

bool	CgiCtx::appendOptionalEnvLines(void)
{
	if (!this->_context._content_type.empty())
	{
		if (!this->appendEnvLine("CONTENT_TYPE", this->_context._content_type))
			return (false);
	}
	if (!this->_context._content_length.empty()
			&& this->_context._content_length[0] != '0')
	{
		if (!this->appendEnvLine("CONTENT_LENGTH",
								this->_context._content_length))
			return (false);
	}
	return (true);
}

bool	CgiCtx::buildEnvLines(void)
{
	this->_launchInfos._env_lines.clear();
	if (this->_context._request_method.empty()
			|| this->_context._server_protocol.empty()
			|| this->_context._script_name.empty())
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	if (!this->appendMandatoryEnvLines())
		return (false);
	if (!this->appendOptionalEnvLines())
		return (false);
	return (true);
}

bool	CgiCtx::buildStdinData(void)
{
	try
	{
		this->_launchInfos._stdin_data.clear();
		this->_launchInfos._stdin_data = this->_request.getBody();
	}
	catch (const std::exception &)
	{ return (this->setError(CGI_CTX_ERR_INTERNAL)); }
	return (this->validateStdinData());
}

bool	CgiCtx::validateStdinData(void)
{
	std::ostringstream	oss;

	oss << this->_launchInfos._stdin_data.size();
	if (oss.str() != this->_context._content_length)
		return (this->setError(CGI_CTX_ERR_INTERNAL));
	return (true);
}

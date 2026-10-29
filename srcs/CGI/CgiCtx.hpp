/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiCtx.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 04:47:19 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 02:01:20 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "WebServ.hpp"

class Request;
class Server;
class HttpRule;

typedef enum	e_CgiCtxStatus
{
	CGI_CTX_EMPTY,
	CGI_CTX_BUILDING,

	CGI_CTX_NOT_CGI,
	CGI_CTX_READY,

	CGI_CTX_BAD_TARGET,
	CGI_CTX_BAD_URI_SUFFIX,
	CGI_CTX_NO_LOCATION,
	CGI_CTX_SCRIPT_NOT_FOUND,
	CGI_CTX_METHOD_NOT_ALLOWED,
	CGI_CTX_SCRIPT_FORBIDDEN,

	CGI_CTX_ERR_BAD_LOCATION_ROOT,
	CGI_CTX_ERR_TARGET_RESOLUTION,
	CGI_CTX_ERR_BAD_CGI_INTERPRETER,
	CGI_CTX_ERR_INTERNAL
}									t_CgiCtxStatus;

typedef enum	e_PathKind
{
	PATH_KIND_ERROR,
	PATH_KIND_MISSING,
	PATH_KIND_DIRECTORY,
	PATH_KIND_REGULAR,
	PATH_KIND_OTHER
}									t_PathKind;

typedef struct	s_CgiResolvedTarget
{
	std::string						_script_path;
	std::string						_script_local_path;
	std::string						_script_uri;
	std::string						_path_info;
	std::string						_extension;
}									t_CgiResolvedTarget;

typedef struct	s_CgiResolvedBase
{
	std::string						_location_prefix_uri;
	std::string						_uri_suffix;
	std::string						_filesystem_base;
	std::vector<std::string>		_normalized_suffix_segments;
}									t_CgiResolvedBase;

typedef struct	s_MatchedLocation
{
	std::string						_locationPath;
	const HttpRule					*_httpRules;
}									t_MatchedLocation;

typedef struct	s_RequestTarget
{
	std::string						_uri_path;
	std::string						_query_string;
}									t_RequestTarget;

typedef struct	s_CgiRequestContext
{
	std::string						_request_method;
	std::string						_query_string;
	std::string						_server_protocol;
	std::string						_server_port;
	std::string						_server_name;
	std::string						_remote_addr;
	std::string						_script_name;
	std::string						_path_info;
	std::string						_content_type;
	std::string						_content_length;
}									t_CgiRequestContext;

typedef struct	s_CgiLaunchInfos
{
	std::string						_program_path;
	std::string						_working_directory;
	std::string						_script_path;
	std::vector<std::string>		_argv;
	std::vector<std::string>		_env_lines;
	std::string						_stdin_data;
}									t_CgiLaunchInfos;





class CgiCtx
{
public:
	CgiCtx(Request &, Server &);
	~CgiCtx();

	bool							createCgiCtx();
	const t_CgiLaunchInfos			&getLaunchInfos() const;
	t_CgiCtxStatus					getStatus() const;

	


private:
	CgiCtx();
	CgiCtx(const CgiCtx &);
	CgiCtx &operator=(const CgiCtx &);

	bool							setStatus(t_CgiCtxStatus);
	bool							setError(t_CgiCtxStatus);
	void							logError(t_CgiCtxStatus) const;
	bool							isNotFatalStatus() const;

	bool							parseRawTargetOriginForm();
	bool							findBestLocation();
	bool							resolveLocationFilesystemBase();
	bool							extractSuffixUriFromLocation();
	bool							getLocationFilesystemBase();
	bool							resolveCgiScriptTarget();
	bool							buildNormalizedSuffixSegments();
	std::string						buildScriptPathFromBaseAndCount(
									size_t) const;
	std::string						buildUriFromPrefixAndCount(size_t) const;
	std::string						buildPathInfoFromIndex(size_t) const;
	std::string						getFileExtension(const std::string &) const;
	bool							isConfiguredCgiExtension(
									const std::string &) const;
	bool							isAccessibleScript();
	bool							isAllowedMethod();
	bool							setLaunchScriptPath();
	bool							validateCgiInterpretorPath();
	bool							validateAndNormalizeRoot(std::string &);
	bool							setCgiInterpretorPath();
	bool							setCgiWorkingDirectory();
	bool							setScriptLocalPath();
	bool							setRequestMethod();
	bool							setQueryString();
	bool							setServerProtocol();
	bool							setScriptName();
	bool							setPathInfo();
	bool							setContentType();
	bool							setContentLength();
	bool							setRemoteAddr();
	bool							setServerPort();
	bool							setServerName();
	bool							buildLaunchInfos();
	bool							buildRequestContext();
	bool							buildArgv();
	bool							buildEnvLines();
	bool							appendEnvLine(const std::string &,
									const std::string &);
	bool							appendMandatoryEnvLines();
	bool							appendOptionalEnvLines();
	bool							buildStdinData();
	bool							validateStdinData();


	void							resetLaunchInfos();
	void							resetContext();
	void							resetResolvedBase();
	void							resetResolvedTarget();

	Request							&_request;
	Server							&_server;

	t_CgiLaunchInfos				_launchInfos;
	t_CgiRequestContext				_context;
	t_RequestTarget					_target;
	t_MatchedLocation				_location;
	t_CgiResolvedBase				_resolvedBase;
	t_CgiResolvedTarget				_resolvedTarget;
	t_CgiCtxStatus					_status;
};

bool								doesLocationMatchUriPath(
									const std::string &, const std::string &);
bool								hasCtlOrSpace(const std::string &);
void								splitPathsegments(const std::string &,
									std::vector<std::string> &);
void								splitConfiguredRoot(const std::string &,
									std::vector<std::string> &);
std::string							joinRelativeSegments(const std::vector<std::string> &);
std::string							joinUriSegments(const std::vector<std::string> &);
t_PathKind							getPathKind(const std::string &);


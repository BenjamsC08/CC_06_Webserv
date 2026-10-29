/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_utils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 04:46:37 by mkerrien          #+#    #+#             */
/*   Updated: 2026/03/27 16:50:22 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cerrno>
#include <string>
#include <sys/stat.h>
#include "CgiCtx.hpp"


bool	hasCtlOrSpace(const std::string &s)
{
	for (size_t i = 0; i < s.size(); i++)
	{
		unsigned char	c = s[i];

		if (c <= 32 || c == 127)
			return (true);
	}
	return (false);
}

bool doesLocationMatchUriPath(const std::string &locationPath,
		const std::string &uriPath)
{
	size_t	len = locationPath.length();

	if (locationPath.empty())
		return (false);

	if (uriPath.compare(0, len, locationPath))
		return (false);

	if (uriPath.length() == len)
		return (true);
	if(locationPath[len - 1] == '/')
		return (true);
	if (uriPath[len] == '/')
		return (true);
	return (false);
}

void	splitPathsegments(const std::string &path, std::vector<std::string> &parts)
{
	size_t	start = 0;
	size_t	end;

	parts.clear();
	while (start < path.length())
	{
		while (start < path.length() && path[start] == '/')
			start++;
		end = start;
		while (end < path.length() && path[end] != '/')
			end++;
		if (end > start)
			parts.push_back(path.substr(start, end - start));
		start = end;
	}
}

void	splitConfiguredRoot(const std::string &root,
		std::vector<std::string> &parts)
{
	if (root.compare(0, 2, "./") == 0)
		splitPathsegments(root.substr(2), parts);
	else
		splitPathsegments(root, parts);
}

std::string	joinRelativeSegments(const std::vector<std::string> &parts)
{
	std::string	result = "./";
	size_t		i = 0;

	if (parts.empty())
		return ("./");

	while (i < parts.size())
	{
		result += parts[i];
		if (i + 1 < parts.size())
			result += "/";
		i++;
	}
	return (result);
}

std::string	joinUriSegments(const std::vector<std::string> &parts)
{
	std::string	result;
	size_t		i;

	if (parts.empty())
		return ("/");
	result = "/";
	i = 0;
	while (i < parts.size())
	{
		result += parts[i];
		if (i + 1 < parts.size())
			result += "/";
		i++;
	}
	return (result);
}

t_PathKind	getPathKind(const std::string &path)
{
	struct stat	stat_ret;

	if (stat(path.c_str(), &stat_ret) == 0)
	{
		if (S_ISDIR(stat_ret.st_mode))
			return (PATH_KIND_DIRECTORY);
		if (S_ISREG(stat_ret.st_mode))
			return (PATH_KIND_REGULAR);
		return (PATH_KIND_OTHER);
	}
	if (errno == ENOENT || errno == ENOTDIR)
		return (PATH_KIND_MISSING);
	return (PATH_KIND_ERROR);
}

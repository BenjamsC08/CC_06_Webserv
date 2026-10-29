/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRules.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:54 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:55 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "HttpRules.hpp"
#include "Logging.hpp"
#include "WebServ.hpp"

HttpRule::HttpRule() : _canUpload(false), _listingActivated(false), _canRedir(false)
{
	this->_methods.flags = 0;
}

HttpRule::HttpRule(const HttpRule &other) : _canUpload(other._canUpload),
	_listingActivated(other._listingActivated), _canRedir(other._canRedir), _rootPath(other._rootPath),
	_noIndexPath(other._noIndexPath), _uploadPath(other._uploadPath),
	_httpRedir(other._httpRedir), _cgis(other._cgis)
{
	this->_methods.flag.GET_METHOD = other._methods.flag.GET_METHOD;
	this->_methods.flag.POST_METHOD = other._methods.flag.POST_METHOD;
	this->_methods.flag.DELETE_METHOD = other._methods.flag.DELETE_METHOD;

}

HttpRule::~HttpRule()
{}

HttpRule &HttpRule::operator=(const HttpRule &other)
{
	if (this != &other)
	{
		this->_cgis = other._cgis;
		this->_rootPath = other._rootPath;
		this->_canUpload = other._canUpload;
		this->_canRedir = other._canRedir;
		this->_httpRedir = other._httpRedir;
		this->_uploadPath = other._uploadPath;
		this->_noIndexPath = other._noIndexPath;
		this->_listingActivated = other._listingActivated;
		this->_methods.flag.GET_METHOD = other._methods.flag.GET_METHOD;
		this->_methods.flag.POST_METHOD = other._methods.flag.POST_METHOD;
		this->_methods.flag.DELETE_METHOD = other._methods.flag.DELETE_METHOD;
	}
	return (*this);
}



void  HttpRule::setUpload(bool act)
{	this->_canUpload = act;	}

void  HttpRule::setListing(bool	act)
{	this->_listingActivated = act;	}

void  HttpRule::addCgi(std::string &line)
{
	std::vector<size_t> poss;

	std::size_t k = 0;
	int i = 0;
	while (k != std::string::npos)
	{
		k = line.find_first_of('"', i);
		if (k == std::string::npos)
			break ;
		poss.push_back(k);
		i = k+1;
	}

	if (poss.size() != 4)
	{
		g_Log << "[e] SERVCONF: CGI error: can't load : ` " << line;
		g_Log << "`\nformat: `cgi \".{ext}\" \"{path to exec file}\"\n";
		return ;
	}
	std::string ext = line.substr(poss[0] + 1, poss[1] - poss[0] - 1);
	this->_cgis[ext] = line.substr(poss[2] + 1, poss[3] - poss[2] - 1);
	
}

void	HttpRule::addCgi(std::map<std::string, std::string>::iterator &it)
{
	std::map<std::string, std::string>::iterator jt;

	jt = this->_cgis.begin();
	this->_cgis.insert(jt,
		std::pair<std::string, std::string>(it->first, it->second));
}

void  HttpRule::setRootPath(std::string	path)
{	this->_rootPath = path;	}

void  HttpRule::setUploadPath(std::string path)
{	this->_uploadPath = path; }

void  HttpRule::addHttpMethods(HttpMethods methd)
{
	if (methd == GET)
		this->_methods.flag.GET_METHOD = 1;
	if (methd == POST)
		this->_methods.flag.POST_METHOD= 1;
	if (methd == DELETE)
		this->_methods.flag.DELETE_METHOD= 1;
}

void  HttpRule::setnoListingPath(std::string path)
{	this->_noIndexPath = path;	}

void  HttpRule::addRedirHttp( std::string redir)
{
	DEBUG("Setting redir :" << redir << '\n');
	this->_httpRedir = redir;
	this->_canRedir = true;
}





bool	HttpRule::isListingAct() const
{	return (this->_listingActivated);	}

bool	HttpRule::isUploadAct() const
{	return (this->_canUpload);		}

bool	HttpRule::isRedirAct() const
{	return (this->_canRedir);		}


std::string	HttpRule::getRootPath()	const
{	return (this->_rootPath);	}

std::string	HttpRule::getPathNoListing() const
{	return (this->_noIndexPath);	}

std::string	HttpRule::getUploadPath() const
{	return (this->_uploadPath);	}

std::string	HttpRule::getHttpRedir() const
{	return (this->_httpRedir);	}

std::map<std::string, std::string>	HttpRule::getCgis()	const
{	return (this->_cgis);	}

volatile union CONFIG_FLAGS HttpRule::getMethods() const
{	return (const_cast<union CONFIG_FLAGS&>(this->_methods));	}



int	upload_check(bool act, std::string path);

int  HttpRule::isRuleOk(const std::string &route)
{
	if (this->_rootPath.empty())
	{
		g_Log << "[e] `" << route << "' no root path this route cannot ";
		g_Log << "be activated\n";
		return (NO_ROOT_PATH);
	}
	
	
	
	
	
	
	
	
	
	if (this->_methods.flags == 0)
	{
		g_Log << "[w] `" << route << "' no methods mentionned";
		g_Log << " so they're all activated by default\n";
		this->_methods.flag.DELETE_METHOD = 1;
		this->_methods.flag.GET_METHOD = 1;
		this->_methods.flag.POST_METHOD = 1;
	}
	if (this->_httpRedir.empty())
		g_Log << "[i] `" << route << "' no redir set\n";
	if (!this->_listingActivated)
	{
		if (this->_noIndexPath.empty())
		{
			g_Log << "[w] " <<  this->_rootPath;
			g_Log << ": Listing Activated but no path so trying to load ";
			g_Log << "default\n";
			
		}
	}
	switch (upload_check(this->_canUpload, this->_uploadPath))
	{
		case 0:
			break ;
		case 1:
			g_Log << "[w] " << this->_rootPath;
			g_Log << ": Cannot enable Upload.The path to the directory does ";
			g_Log << "not exist\n";
			this->_canUpload = false;
	}
	return (NO_HTTP_RULE_ERROR);
}





int	upload_check(bool act, std::string path)
{
	int fd;

	if (!act)
		return (0);
	if (path.empty())
		return (1);
	fd = open(path.c_str(), O_DIRECTORY);
	if (fd == -1)
		return (1);
	close(fd);
	return (0);
}

std::ostream& operator<<(std::ostream &out, HttpRule &rule)
{
	std::string redir = rule.getHttpRedir();
	if (!redir.empty())
		out << "redir http: " << redir << '\n';
	else
		out << "no redir http\n";
	out << "root path: " << rule.getRootPath() << '\n';
	out << std::boolalpha << "listing on ?: " << rule.isListingAct() << '\n';
	if (!rule.isListingAct())
		out << "path no listing: " << rule.getPathNoListing() << '\n';
	out << "Upload on ?: " << rule.isUploadAct() << '\n';
	if (rule.isUploadAct())
		out << "path upload: " << rule.getUploadPath() << '\n';
	std::map<std::string, std::string> tmap = rule.getCgis();
	if (!tmap.empty())
	{
		out << "cgi ON:\n";
		std::map<std::string, std::string>::iterator jt;
		for (jt = tmap.begin(); jt != tmap.end(); jt++)
			out << "ext: " << jt->first << " | path: " << jt->second << '\n';
	}
	else
		out << "CGI EMPTY\n";
	return (out);
}

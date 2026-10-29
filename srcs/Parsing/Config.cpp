/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:48 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 02:00:29 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Logging.hpp"
#include "WebServ.hpp"
#include <algorithm>
#include <vector>

Config::Config(std::string path) : _pathToConfig(path)
{
	if (path.empty() || !fileLoad(path))
		throw FileLoadingException();
	this->_configFileLoaded = true;
	this->preParse();
	this->parse();
}

Config::Config(const Config &other) : 
	_configFileLoaded(other._configFileLoaded), 
	_pathToConfig(other._pathToConfig)
{
	if (!this->_pathToConfig.empty() && this->_configFileLoaded)
		fileLoad();
}

Config::~Config()
{
	if (this->_in.is_open())
		this->_in.close();
}

Config &Config::operator=(const Config &other)
{
	if (this != &other)
	{
		this->_configFileLoaded = other._configFileLoaded;
		this->_pathToConfig = other._pathToConfig;
		fileLoad();
	}
	return (*this);
}

bool Config::canRun()
{
	if (!this->_configFileLoaded)
	{
		std::cerr << "Config not loaded\n";
		return false;
	}
	std::vector<int> allPorts;
	for (t_uint i=0; i < this->_servers.size(); i++)
	{
		if (!this->_servers[i].isServConfOk())
			return (false);
		std::vector<int> tmp = this->_servers[i].getPorts();
		for (t_uint j=0; j < tmp.size(); j++)
		{
			if (std::find(allPorts.begin(), allPorts.end(), tmp[j]) != allPorts.end())
			{
				g_Log << "[e] Cannot accept multiple server used the same port\n";
				return (false);
			}
			allPorts.push_back(tmp[j]);
		}
	}
	return true;
}

void  Config::fileLoad()
{
	if (this->_pathToConfig.empty())
		throw FileLoadingException();
	this->_in.open(this->_pathToConfig.c_str());
	if (!this->_in.is_open())
		throw FileLoadingException();
}


bool  Config::fileLoad(std::string path)
{
	if (path.empty())
		return false;
	this->_in.open(path.c_str());
	if (!this->_in.is_open())
		return false;
	this->_pathToConfig = path;
	return true;
}

void	Config::preParse()
{
	std::string str;
	std::size_t pos;

	while (std::getline(this->_in, str)) 
	{
		pos = str.find('#');
		if (pos != std::string::npos)
			str = str.substr(0, pos);
		if (str.empty())
			continue ;
		pos = str.find_first_not_of("\t\r ");
		str.erase(0, pos);
		pos = str.find_last_not_of("\t\r ");
		if (pos != std::string::npos)
			str.erase(pos+1);
		this->_extractedData.push_back(str);
	}
	if (this->_extractedData.empty())
		throw FileLoadingException();
	this->_in.close();
}

void	Config::parse()
{
	int depth = 0;
	std::string line;
	std::vector<std::string> data;
	static t_uint serv_id = 0;
	for (t_uint i = 0; i < this->_extractedData.size(); i++)
	{
		if (this->_extractedData[i].find("}") != std::string::npos)
			depth--;
		if (depth > 0)
		{
			data.push_back(this->_extractedData[i]);
		}
		if (this->_extractedData[i].find("{") != std::string::npos)
			depth++;
		if (depth == 0 && !data.empty())
		{
			ServConf tmp(data, serv_id);
			std::vector<int> ports = tmp.getPorts();
			this->_servers.push_back(tmp);
			for (t_uint k = 0; k < ports.size(); k++)
				(this->_portToIdServConf[ports[k]]).push_back(serv_id);
			data.clear();
			serv_id++;
		}
	}
	if (depth != 0 || (data.empty() && !serv_id))
		throw FileLoadingException();
}

ServConf  Config::getServConf(t_uint id) const
{ return (this->_servers[id]); }

const std::vector<ServConf> &Config::getAllServConf() const
{ return (this->_servers); }

const std::vector<t_uint> &Config::getIdServConfByPort(t_uint port) const
{ return (this->_portToIdServConf.at(port)); }

const std::map<int, std::vector<t_uint> > &Config::getPortsMap() const
{ return (this->_portToIdServConf); };

std::vector<t_uint> Config::extractPorts()
{
	std::map<int, std::vector<t_uint> > map = getPortsMap();
	std::map<int, std::vector<t_uint> >::iterator it;
	std::vector<t_uint> tab;
	for (it = map.begin();it != map.end(); it++)
		tab.push_back(it->first);
	return (tab);
}


const char *Config::FileLoadingException::what() const throw()
{
	return (RED "Something wrong with the config file\nCheck the exemple" RESET);
};

std::ostream& operator<<(std::ostream &out, Config &conf)
{
	std::vector<ServConf>tmp(conf.getAllServConf());
	std::vector<ServConf>::iterator it;
	for (it = tmp.begin(); it != tmp.end(); it++)
	{
		out << RED << "server N" << it->getServId() << RESET << '\n';
		out << *it << "\n\n\n";
	}
	return (out);
}


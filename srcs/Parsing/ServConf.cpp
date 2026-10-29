/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConf.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:59 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:44:27 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <string>

#include "ServConf.hpp"
#include "Logging.hpp"
#include "WebServ.hpp"


int get_rule_id(const std::string input);
int get_rule_id(const std::string input, t_uint &end_pos);
std::vector<int> get_numbers_in_line(std::string input, t_uint &pos_end, bool t);

bool  inHttpRule = false;

ServConf::ServConf(std::vector<std::string> &input, t_uint id)
{
	
	std::string line;
	this->_default = false;
	this->_id = id;
	this->_maxBodySize = 0;
	for (t_uint i=0; i < input.size(); i++)
	{
		line = input[i];
		switch (get_rule_id(line))
		{
			case 0:
				this->extractPort(line);
				break ;
			case 1:
				this->extractError(line);
				break ;
			case 2:
				this->extractServName(line);
				break ;
			case 3:
				this->_default = true;
				break ;
			case 4:
				this->fillBodySize(line);
				break ;
			case 5:
				inHttpRule = true;
				this->fillRules(input, i);
				break ;
			case 6:
				if (!inHttpRule)
					this->addCgi(line);
				break ;
		}
	}
	if (!this->_maxBodySize)
		this->_maxBodySize = 0x300000;
}

ServConf::ServConf(const ServConf &other) :	_default(other._default),
	_id(other._id), _maxBodySize(other._maxBodySize), _name(other._name),
	_ports(other._ports), _errors(other._errors), _httpRules(other._httpRules),
	_cgis(other._cgis)
{}

ServConf	&ServConf::operator=(const ServConf &other)
{
	if (this != &other)
	{
		this->_default = other._default;
		this->_id = other._id;
		this->_maxBodySize = other._maxBodySize;
		this->_name = other._name;
		this->_ports = other._ports;
		this->_errors = other._errors;
		this->_httpRules = other._httpRules;
		this->_cgis = other._cgis;
	}
	return (*this);
}

ServConf::~ServConf()
{}

bool	ServConf::isDefault() const
{	return (this->_default);	};

t_uint	  ServConf::getServId() const
{	return (this->_id);	}

std::string ServConf::getServName() const
{	return (this->_name);	}

/*
 *	Return a vector of prots used
 */
std::vector<int>	ServConf::getPorts() const
{	return (this->_ports);	}


t_uint	ServConf::getMaxBodySize()  const
{	return (this->_maxBodySize);	}

/*
 *	Return a map of the errors
 */
std::map<int, std::string>	ServConf::getErrors() const
{	return (this->_errors);	  }

/*
 *	Return a map with the location and rules associate with
 */
const std::map<std::string, HttpRule>	&ServConf::getHttpRules() const
{	return (this->_httpRules);}

const std::map<std::string, std::string>	&ServConf::getCgis() const
{	return (this->_cgis);	  }



bool	ServConf::isServConfOk()
{
	if (this->_ports.empty())
	{
		g_Log <<"[e] SERVCONF: No port listen for server " << this->_id << '\n';
		return (false);
	}
	if (this->_name.empty())
	{
		g_Log << "[w] SERVCONF: No name for server " << this->_id;
		g_Log << " it will be set as localhost\n";
		this->_name = "localhost";
	}

	std::map<std::string, HttpRule>::iterator it;
	for (it = this->_httpRules.begin(); it != this->_httpRules.end();)
	{
		if (it->second.isRuleOk(it->first) != 0)
		{
			g_Log << "[w] `" << it->first << "' in serv " << this->_id;
			g_Log << " cannot be set\n";
			this->_httpRules.erase(it++);
		}
		else
			it++;
	}
	if (this->_httpRules.empty())
	{
		g_Log << "[e] this serv has no route to deserve, ";
		g_Log << "it will not be launched\n";
		return (false);
	}
	if (!this->_cgis.empty())
	{
		std::map<std::string, std::string>::iterator it;
		std::map<std::string, HttpRule> &hrs = this->_httpRules;


		for (it=this->_cgis.begin(); it != this->_cgis.end(); it++)
		{
			std::map<std::string, HttpRule>::iterator jt;

			for (jt=hrs.begin(); jt != hrs.end(); jt++)
			{
				std::map<std::string, std::string> cgis = jt->second.getCgis();
				if (cgis.find(it->first) == cgis.end())
					jt->second.addCgi(it);
			}
		}
	}
	return (true);
}


/*
 *
 * Tools Functions
 *
 */

/*
 *	Fill the string given in the argument with all the ports used in the config
 */
void	ServConf::extractPort(std::string &in)
{
	t_uint t = in.length();
	std::vector<int>  tab;
	tab = get_numbers_in_line(in, t, 1);
	if (tab.empty())
		return ;
	for (t_uint i=0; i < tab.size(); i++)
	{
		if (tab[i] < 0 || tab[i] > 65535)
		{
			std::ostringstream c;
			c << tab[i];
			g_Log << "[e] SERVCONF: port `" << c.str();
			g_Log << "` is not in range 0;65535. it will be ignored\n";
			continue ;
		}
		this->_ports.push_back(tab[i]);
	}
}

/*
 *	Fill the string given in the argument with all the errors used in the config
 */
void	  ServConf::extractError(std::string &in)
{
	t_uint start = 0;
	std::vector<int>  tab(get_numbers_in_line(in, start, 0));
	std::string path = in.substr(start);
	for (t_uint i=0; i < tab.size(); i++)
		this->_errors[tab[i]] = path;
}

void	ServConf::extractServName(std::string &in)
{
	std::size_t b = in.find_first_of(" \t\v");
	std::string tmp (in, b+1);
	tmp.substr(0, tmp.find_first_of(" \t\v"));
	this->_name = tmp;
}

void	errorBodySizeTooBig(t_uint &bodySize)
{
	g_Log << "[w] Max body size to big, it will be set at Uint max\n";
	bodySize = UINT_MAX;
}

void	ServConf::fillBodySize(std::string &line)
{
	std::size_t b = line.find_first_of(" \t\v");
	std::string tmp (line, b+1);
	tmp.substr(0, tmp.find_first_of(" \t\v"));
	char *ptrE;
	long int size = strtol(tmp.c_str(), &ptrE, 10);
	if ((size > 4 && *ptrE == 'G') || (size > 4000 && *ptrE == 'M')
			|| (size > 4000000 && *ptrE == 'K'))
		errorBodySizeTooBig(this->_maxBodySize);
	else
	{
		if (*ptrE == 'K')
			size *= 1000;
		if (*ptrE == 'M')
			size *= 1000000;
		if (*ptrE == 'G')
			size *= 1000000;
	}
	if (size > std::numeric_limits<t_uint>::max())
		errorBodySizeTooBig(this->_maxBodySize);
	else
		this->_maxBodySize = size;
	return;
}

void  ServConf::addCgi(std::string &line)
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

void  ServConf::fillRules(std::vector<std::string> &in, t_uint &pos)
{
	std::size_t found = in[pos].find_first_of("/");
	if (found == std::string::npos)
		return ;
	std::size_t end = in[pos].find("{");
	std::string location = in[pos].substr(found, end - found - 1);
	HttpRule &rule = this->_httpRules[location];
	pos++;
	std::vector<int> tab;
	std::string tmp;
	bool	mtdCheck = false;
	while (42)
	{
		t_uint	end = 0;
		if (in[pos].find('}') != std::string::npos)
		{
			inHttpRule = false;
			break ;
		}
		switch (get_rule_id(in[pos], end))
		{
			case 6:
				if (inHttpRule)
					rule.addCgi(in[pos]);
				break;
			case 7:	
				if (in[pos].find("GET") != std::string::npos)
				{
					mtdCheck = true;
					rule.addHttpMethods(GET);
				}
				if (in[pos].find("POST") != std::string::npos)
				{
					mtdCheck = true;
					rule.addHttpMethods(POST);
				}
				if (in[pos].find("DELETE") != std::string::npos)
				{
					mtdCheck = true;
					rule.addHttpMethods(DELETE);
				}
				if (mtdCheck == true)
				{
					g_Log << "[w] Method not allowed in serv " << this->_id
						<< '\n';
				}
				break;
			case 8:
			{
				size_t start = in[pos].find("return ");
				if (start != std::string::npos)
				{
					tmp = in[pos].substr(6, in[pos].size() - 6);
					rule.addRedirHttp(tmp);
				}
				tab.clear();
				break;
			}
			case 9:
				tmp = in[pos].substr(end);
				tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
				rule.setRootPath(tmp);
				break;
			case 10:
				tmp = in[pos].substr(end);
				tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
				rule.setnoListingPath(tmp);
				break;
			case 11:
				if (in[pos].find("on") != std::string::npos ||
						in[pos].find("enable") != std::string::npos)
					rule.setListing(true);
				break;
			case 12:
				tmp = in[pos].substr(end);
				tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
				rule.setUploadPath(tmp);
				break;
			case 13:
				if (in[pos].find("on") != std::string::npos ||
						in[pos].find("enable") != std::string::npos)
					rule.setUpload(true);
				else
					rule.setUpload(false);
				break;
		}
		pos++;
	}
}




/*
*
* OTHER
*
*/

int get_rule_id(const std::string input)
{
	const std::string	rules[SERV_RULE_NB] = {"listen", "error", "name",
		"default", "max_body_size", "location", "cgi", "methods", "return",
		"root",	"nolisting_path", "listing", "upload_path", "upload"};

	for (int i=0; i < SERV_RULE_NB; i++)
	{
		if (input.find(rules[i]) <= 1)
			return (i);
	}
	return(42);
}

int get_rule_id(const std::string input, t_uint &end_pos)
{
	const std::string	rules[SERV_RULE_NB] = {"listen", "error", "name",
		"default", "max_body_size", "location", "cgi", "methods", "return",
		"root",	"nolisting_path", "listing", "upload_path", "upload"};

	for (int i=0; i < SERV_RULE_NB; i++)
	{
		if (input.find(rules[i]) <= 1)
		{
			end_pos = rules[i].length();
			while (std::isspace(rules[i][end_pos]))
			{
				end_pos++;
			}
			return (i);
		}
	}
	return(42);
}


std::vector<int> get_numbers_in_line(std::string input, t_uint &pos_end, bool t)
{
	std::size_t found = input.find_first_of("0123456789");
	if (found == std::string::npos)
		throw std::runtime_error("Port need to be specified");

	std::vector<int> tab;
	long int number = 0;
	std::size_t len = input.length() - found;
	std::string verify(input, found, len);
	char *tmp;

	if (t)
	{
		if (verify.find_first_not_of("0123456789,") != std::string::npos)
			throw std::runtime_error("Port can only be numbers comma separated");
	}
	input += found;
	pos_end = found;
	tmp = &input[found];
	while (42)
	{
		char *e;
		number = strtol(tmp, &e, 10);
		if (number == 0)
			break ;
		pos_end += e - tmp + 1;
		tmp = e;
		if (number > INT_MAX || number < INT_MIN)
		{
			g_Log << "[i] SERVCONF: Cannot convert bigger than int, " << number << " will be ignored\n";
			continue ;
		}
		tab.push_back((int)number);
		if (!*e || !*tmp)
			break ;
		if (*tmp != ',' || !tmp[1])
			break ;
		if (isdigit(tmp[1]))
			tmp++;
		else
			break ;
	}
	return (tab);
}

std::ostream& operator<<(std::ostream &out, ServConf &serv)
{
	std::vector<int> tmp(serv.getPorts());
	std::vector<int>::iterator it;
	out << "serv name :" << serv.getServName() << '\n';
	out << "max body size :" << serv.getMaxBodySize() << '\n';
	out << "is default serv: " << serv.isDefault() << '\n';
	out << "ports inside this serv :\n";
	for (it = tmp.begin(); it != tmp.end(); it++)
		out << *it << ", ";
	out << '\n';
	std::map<int, std::string> emap = serv.getErrors();
	if (!emap.empty())
	{
		std::map<int, std::string>::iterator jt;
		out << "error returns :\n";
		for (jt = emap.begin(); jt != emap.end(); jt++)
			out << "code: " << jt->first << " | path: " << jt->second << '\n';
	}
	else
		out << "Errors EMPTY\n";
	std::map<std::string, HttpRule> rmap = serv.getHttpRules();
	if (!rmap.empty())
	{
		std::map<std::string, HttpRule>::iterator jt;
		out << "\nLocation: ";
		for (jt = rmap.begin(); jt != rmap.end(); jt++)
			out << jt->first << '\n' << jt->second << "\n\n";

	}
	else
		out << "HttpRules EMPTY\n";
	std::map<std::string, std::string> tmap = serv.getCgis();
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

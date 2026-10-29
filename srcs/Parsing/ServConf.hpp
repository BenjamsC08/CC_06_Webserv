/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServConf.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:34:02 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:34:03 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVCONF_HPP
#define SERVCONF_HPP

#include "HttpRules.hpp"
#include "WebServ.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>

class ServConf
{
public:
	ServConf(std::vector<std::string> &, t_uint);
	ServConf(const ServConf &);
	~ServConf();

	ServConf &operator=(const ServConf &);
	
	
	bool										isDefault()			const;
	t_uint										getServId()			const;
	t_uint										getMaxBodySize()	const;
	std::string									getServName()		const;
	std::vector<int>							getPorts()			const;
	std::map<int, std::string>					getErrors()			const;
	const std::map<std::string, HttpRule>		&getHttpRules()		const;
	const std::map<std::string, std::string>	&getCgis()			const;
	
	

	bool										isServConfOk();

private:
	ServConf();

	bool										_default;
	t_uint										_id;
	t_uint										_maxBodySize;
	std::string									_name;
	std::vector<int>							_ports;
	std::map<int, std::string>					_errors;
	std::map<std::string, HttpRule>				_httpRules;	
	std::map<std::string, std::string>			_cgis;

	void	  extractPort(std::string &);
	void	  extractError(std::string &);
	void	  extractServName(std::string &);
	void	  fillBodySize(std::string &);
	void	  fillRules(std::vector<std::string> &, t_uint &);
	void	  addCgi(std::string &);
};


std::ostream& operator<<(std::ostream &out, ServConf &serv);

#endif

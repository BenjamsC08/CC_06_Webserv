/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:50 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:51 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "WebServ.hpp"
#include "ServConf.hpp"

class Config
{
public:
	Config(std::string   );
	Config(const Config &);
	~Config();

	Config &operator=(const Config &);

	void	parse();
	bool	canRun();
	void	preParse();
	void	fileLoad();
	bool	fileLoad(std::string);

	
	
	class FileLoadingException : public std::exception {
		public:
			virtual const char *what() const throw();
	};

	std::ifstream	_in; 
	
	ServConf									getServConf(t_uint)	const;
	const std::vector<ServConf>					&getAllServConf()	const;
	const std::map<int, std::vector<t_uint> >	&getPortsMap()	const;
	std::vector<t_uint>							extractPorts();
	const std::vector<t_uint>				&getIdServConfByPort(t_uint) const;




private:
	Config();

	bool								  _configFileLoaded;
	std::string							  _pathToConfig;
	std::vector<std::string>			  _extractedData;
	std::vector<ServConf>				  _servers;
	std::map<int , std::vector<t_uint> >  _portToIdServConf; 

};


std::ostream& operator<<(std::ostream &out, Config &conf);

#endif

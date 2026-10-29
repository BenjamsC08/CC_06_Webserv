/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRules.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:56 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:58 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRULE_HPP
#define HTTPRULE_HPP

#include <iostream>
#include <map>
#include <string>

#include "WebServ.hpp"

class HttpRule
{
public:
	HttpRule();
	HttpRule(const HttpRule &);
	~HttpRule();

	HttpRule &operator=(const HttpRule &);
//Setters
	void	setUpload(bool);
	void	setListing(bool);
	void	addCgi(std::string &);
	void	addCgi(std::map<std::string, std::string>::iterator &);
	void	setRootPath(std::string);
	void	setUploadPath(std::string);
	void	addHttpMethods(HttpMethods);
	void	setnoListingPath(std::string);
	void	addRedirHttp(std::string);
//Getters
	bool								isListingAct() const;
	bool								isUploadAct() const;
	bool								isRedirAct() const;
	std::string							getRootPath() const;
	std::string							getPathNoListing() const;
	std::string							getUploadPath() const;
	std::string							getHttpRedir() const;
	std::map<std::string, std::string>	getCgis()		const;
//Methods
	int  isRuleOk(const std::string &);
	volatile union CONFIG_FLAGS getMethods() const;

private:
	bool								_canUpload;
	bool								_listingActivated;
	bool								_canRedir;
	std::string							_rootPath;
	std::string							_noIndexPath;
	std::string							_uploadPath;
	std::string							_httpRedir;
	std::map<std::string, std::string>	_cgis;

	volatile union CONFIG_FLAGS			_methods;
};


std::ostream& operator<<(std::ostream &out, HttpRule &rule);

#endif

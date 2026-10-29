/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:34:05 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:34:06 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>
#include <iostream>
#include "Logging.hpp"

Logger::Logger() :  _buffer(""), _pathLog(""), _consoleEnabled(true)
{}
	
Logger::~Logger()
{
	if (this->_file.is_open())
		_file.close();
}

void Logger::setConsole(bool status) {	this->_consoleEnabled = status; }

bool Logger::setFile(const std::string &filename)
{
	if (this->_file.is_open())
		this->_file.close();

	std::string path = filename;

	if (filename.find(".log") == std::string::npos)
		path += ".log";

	_file.open(path.c_str(), std::ios::out | std::ios::app);
	if (!_file.is_open())
		return (false);
	_file.close();
	this->_pathLog = path;
	return (true);
}

void Logger::flush()
{
	if (this->_buffer.empty()) return ;

    std::time_t now = std::time(NULL);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%y%m%d-%H:%M", std::localtime(&now));

	std::string line = std::string(buf) + " | " + this->_buffer;
	if (_consoleEnabled)
        std::cerr << line;
	if (!this->_pathLog.empty())
	{
		this->_file.open(this->_pathLog.c_str(), std::ios::out | std::ios::app);
        this->_file << line;
		this->_file.close();
	}
	this->_buffer.clear();
}

Logger g_Log;


/*
 *
 * refaire le logging pour faire en sorte d'ecire et fermer le fichier a chaque fois
 * essayer de differenciez ce qui va dans verbose et dans le fichier
 *
 *
 *
 */

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:34:08 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:34:09 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <fstream>
#include <string>


class Logger
{
public:
	Logger();
	~Logger();

	void	setConsole(bool status);
	bool	setFile(const std::string &filename);

	template<typename T>
	Logger &operator<<(const T& value);

	void		  flush();
private:
	std::string	  _buffer;
	std::string	  _pathLog;
	std::ofstream _file;
	bool		  _consoleEnabled;
};

#include "Logging.tpp"

extern Logger g_Log;

#endif

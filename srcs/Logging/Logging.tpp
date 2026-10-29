/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.tpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:34:10 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:34:11 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <sstream>

template<typename T>
Logger &Logger::operator<<(const T& value)
{
	std::ostringstream oss;
	oss << value;
	_buffer += oss.str();

	if (_buffer.find_last_of('\n') !=  std::string::npos)
		flush();

    return *this;
}


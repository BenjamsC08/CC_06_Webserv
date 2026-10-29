/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiResponseParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:34:23 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:34:24 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include <iostream>

typedef struct s_CgiHeader
{
	std::string					name;
	std::string					value;
}								t_CgiHeader;

typedef struct s_CgiParsedResponse
{
	int							status_code;
	std::string					reason_phrase;
	std::vector<t_CgiHeader>	headers;
	std::string					body;
	bool						has_status;
	bool						has_location;
	bool						has_content_type;
}								t_CgiParsedResponse;

class CgiResponseParser
{
public:
	static bool					parse(const std::string&, t_CgiParsedResponse&);

private:
								CgiResponseParser(void);
};

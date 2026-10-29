/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess_exec_arrays.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkerrien <mkerrien@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 02:56:55 by mkerrien          #+#    #+#             */
/*   Updated: 2026/04/01 02:58:08 by mkerrien         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"

bool	CgiProcess::buildExecArrays(void)
{
	this->destroyExecArrays();

	if (!this->hasMinimalLaunchSpec())
		return (this->setError(CGI_ERR_BAD_SPEC));
	if (!this->buildStringArray(&this->_argvExec, this->_launchSpec.argv))
	{
		this->destroyExecArrays();
		return (this->setError(CGI_ERR_BUILD_ARGV));
	}
	if (!this->buildStringArray(&this->_envpExec, this->_launchSpec.env_lines))
	{
		this->destroyExecArrays();
		return (this->setError(CGI_ERR_BUILD_ENVP));
	}
	return (true);
}

void	CgiProcess::destroyExecArrays(void)
{
	if (this->_argvExec)
	{
		this->freeStringArray(this->_argvExec);
		this->_argvExec = NULL;
	}
	if (this->_envpExec)
	{
		this->freeStringArray(this->_envpExec);
		this->_envpExec = NULL;
	}
}

bool	CgiProcess::buildStringArray(char ***dst,
						const std::vector<std::string> &src)
{
	size_t	i;
	char	**array = NULL;

	if (!dst)
		return (false);
	*dst = NULL;

	try { array = new char *[src.size() + 1]; }
	catch (std::exception &e)
	{
		std::cerr << "Exception thrown: " << e.what() << "\n";
		return (false);
	}
	for (i = 0; i <= src.size(); i++)
		array[i] = NULL;
	for (i = 0; i < src.size(); i++)
	{
		try { array[i] = new char[src[i].size() + 1]; }
		catch (std::exception &e)
		{
			for (size_t j = 0; j < i; j++)
				delete[] array[j];
			delete[] array;
			return (false);
		}
		std::strcpy(array[i], src[i].c_str());
	}
	*dst = array;
	return (true);
}

void	CgiProcess::freeStringArray(char **array)
{
	size_t	i = 0;

	if (!array)
		return ;

	while (array[i])
	{
		delete[] array[i];
		array[i] = NULL;
		i++;
	}
	delete[] array;
	array = NULL;
}


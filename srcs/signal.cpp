/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 01:33:31 by obajja            #+#    #+#             */
/*   Updated: 2026/05/01 01:33:32 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <signal.h>

void	ctrlc_handler(int rat)
{
    (void) rat;
    std::cout << "\nExiting the WebServer...\n";
    throw (42);
}

void	signal_handling(void)
{
	struct sigaction	ear;
	struct sigaction	pipe_action;

	sigemptyset(&ear.sa_mask);
	ear.sa_handler = &ctrlc_handler;
    sigemptyset(&ear.sa_mask);
	ear.sa_flags = SA_RESTART;
	sigaction(SIGINT, &ear, NULL);

	sigemptyset(&pipe_action.sa_mask);
	pipe_action.sa_handler = SIG_IGN;
	pipe_action.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &pipe_action, NULL);

}

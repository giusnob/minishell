/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:38 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 22:33:22 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Gestisce Ctrl-C (SIGINT) */
void	handle_sigint(int sig)
{
	(void)sig;
	g_signal = 1;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

/* Gestisce Ctrl-\ (SIGQUIT) */
void	handle_sigquit(int sig)
{
	char	*line;

	(void)sig;
	line = rl_line_buffer;
	if (line && *line)
	{
		write(STDOUT_FILENO, "\nQuit\n", 6);
		rl_on_new_line();
		exit(131);
	}
}

/* Setup dei segnali */
void	setup_signals(void)
{
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigquit);
}

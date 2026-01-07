/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:38 by ginobile          #+#    #+#             */
/*   Updated: 2026/01/06 23:59:56 by gifanell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Gestisce Ctrl-C (SIGINT) */
void	handle_sigint(int sig)
{
	(void)sig;
	g_signal = sig;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

/* Setup dei segnali */
void	setup_signals(void)
{
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, SIG_IGN);
}

void	handle_heredoc_signal(int sig)
{
	g_signal = sig;
	close(0);
}

void	set_heredoc_signal(void)
{
	signal(SIGINT, handle_heredoc_signal);
}

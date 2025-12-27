/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:59 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 19:25:33 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Variabile globale per i segnali (unica permessa) */
int	g_signal = 0;

/* Processa un singolo input */
static void	process_input(t_data *data, char *input)
{
	t_token	*tokens;

	if (!*input)
		return ;
	add_history(input);
	tokens = lexer(input, data);
	data->cmd_list = parser(tokens);
	if (data->cmd_list)
		data->last_exit_status = executor(data);
	free_tokens(tokens);
	free_cmd_list(data->cmd_list);
}

int	main(int argc, char **argv, char **envp)
{
	t_data	data;
	char	*input;

	(void)argc;
	(void)argv;
	init_data(&data, envp);
	setup_signals();
	while (1)
	{
		input = readline("minishell$ ");
		if (!input)
		{
			ft_putendl_fd("exit", STDOUT_FILENO);
			break ;
		}
		process_input(&data, input);
		free(input);
	}
	cleanup_data(&data);
	return (data.last_exit_status);
}

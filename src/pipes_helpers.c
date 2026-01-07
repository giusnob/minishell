/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes_helpers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:50:01 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 00:20:39 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Chiude pipes */
void	close_pipes(int *pipe_fd)
{
	close(pipe_fd[0]);
	close(pipe_fd[1]);
}

/* Salva pipe corrente come precedente */
void	save_pipe(int *prev_pipe, int *pipe_fd, int *has_prev)
{
	prev_pipe[0] = pipe_fd[0];
	prev_pipe[1] = pipe_fd[1];
	*has_prev = 1;
}

/* Inizializza pipeline */
int	*init_pipeline(t_data *data, int *num_cmds)
{
	int	*pids;

	*num_cmds = count_commands(data->cmd_list);
	pids = (int *)malloc(sizeof(int) * (*num_cmds));
	return (pids);
}

/* Setup redirection per il figlio nella pipe */
void	setup_child_pipes(t_cmd *cmd, t_pipe_data *pipe_data)
{
	if (!cmd_has_input_redir(cmd) && pipe_data->prev_pipe[0] != -1)
		dup2(pipe_data->prev_pipe[0], STDIN_FILENO);
	if (pipe_data->prev_pipe[0] != -1)
	{
		close(pipe_data->prev_pipe[0]);
		close(pipe_data->prev_pipe[1]);
	}
	if (!cmd_has_output_redir(cmd) && !pipe_data->is_last)
		dup2(pipe_data->curr_pipe[1], STDOUT_FILENO);
	if (!pipe_data->is_last)
	{
		close(pipe_data->curr_pipe[0]);
		close(pipe_data->curr_pipe[1]);
	}
}

/* Loop principale della pipeline */
int	pipeline_loop(t_data *data, int *pids, int *prev_pipe, int *has_prev)
{
	t_cmd	*current;
	int		i;

	current = data->cmd_list;
	i = 0;
	while (current)
	{
		pids[i] = process_pipeline_cmd(data, current, prev_pipe, has_prev);
		if (pids[i] == -1)
			return (-1);
		current = current->next;
		i++;
	}
	return (i);
}

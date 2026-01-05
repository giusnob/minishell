/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:32:26 by ginobile          #+#    #+#             */
/*   Updated: 2026/01/05 03:06:57 by gifanell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Setup dei dati per la pipe */
void	setup_pipe_data(t_pipe_data *pipe_data, int *prev_pipe,
			int *curr_pipe, int is_last)
{
	if (prev_pipe && prev_pipe[0] != -1)
	{
		pipe_data->prev_pipe[0] = prev_pipe[0];
		pipe_data->prev_pipe[1] = prev_pipe[1];
	}
	else
	{
		pipe_data->prev_pipe[0] = -1;
		pipe_data->prev_pipe[1] = -1;
	}
	pipe_data->curr_pipe[0] = curr_pipe[0];
	pipe_data->curr_pipe[1] = curr_pipe[1];
	pipe_data->is_last = is_last;
}

/* Esegue un comando in una pipe (processo figlio) */
/* Applica redirections se ci sono */
/* Esegue il comando */
void	exec_cmd_in_pipe(t_data *data, t_cmd *cmd, char *cmd_path)
{
	int	exit_status;

	if (is_builtin(cmd->args[0]))
	{
		exit_status = exec_builtin(data, cmd);
		return (cleanup_child(data), free(cmd_path), exit(exit_status));
	}
	if (execve(cmd_path, cmd->args, data->envp) == -1)
	{
		print_error(cmd->args[0], "execution failed");
		return (cleanup_child(data), free(cmd_path), exit(CMD_NOT_EXECUTABLE));
	}
}

/* Conta il numero di comandi nella pipeline */
int	count_commands(t_cmd *cmd_list)
{
	int		count;
	t_cmd	*current;

	count = 0;
	current = cmd_list;
	while (current)
	{
		count++;
		current = current->next;
	}
	return (count);
}

/* Aspetta tutti i processi figli */
/* Solo l'ultimo conta */
int	wait_all_processes(int *pids, int num_cmds)
{
	int	status;
	int	last_status;
	int	i;

	last_status = 0;
	i = 0;
	while (i < num_cmds)
	{
		if (waitpid(pids[i], &status, 0) != -1
			&& i == num_cmds - 1)
		{
			if (WIFEXITED(status))
				last_status = WEXITSTATUS(status);
			else if (WIFSIGNALED(status))
				last_status = 128 + WTERMSIG(status);
		}
		i++;
	}
	return (last_status);
}

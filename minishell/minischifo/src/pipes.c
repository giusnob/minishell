/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:27 by ginobile          #+#    #+#             */
/*   Updated: 2026/01/05 03:02:16 by gifanell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Gestisce il path del comando (builtin o esterno) */
static char	*get_cmd_path_for_pipe(t_cmd *cmd, char **envp)
{
	char	*cmd_path;

	if (!is_builtin(cmd->args[0]))
	{
		cmd_path = find_command_path(cmd->args[0], envp);
		if (!cmd_path)
		{
			print_error(cmd->args[0], "command not found");
			return (NULL);
		}
		return (cmd_path);
	}
	return (NULL);
}

/* Esegue un comando in una pipeline */
/* Trova il path del comando se pid = 0 Processo figlio */
/* Se non è il primo comando, leggi dalla pipe precedente */
/* Se non è l'ultimo comando, scrivi sulla pipe corrente */
static int	exec_pipeline_cmd(t_data *data, t_cmd *cmd, t_pipe_data *pipe_data)
{
	pid_t	pid;
	char	*cmd_path;

	cmd_path = get_cmd_path_for_pipe(cmd, data->envp);
	if (!cmd_path && !is_builtin(cmd->args[0]))
		return (CMD_NOT_FOUND);
	pid = fork();
	if (pid == -1)
	{
		print_error("fork", "failed");
		if (cmd_path)
			free(cmd_path);
		return (ERROR);
	}
	if (pid == 0)
	{
		setup_child_pipes(pipe_data);
		exec_cmd_in_pipe(data, cmd, cmd_path);
	}
	if (cmd_path)
		free(cmd_path);
	return (pid);
}

/* Processa un singolo comando della pipeline */
/* Crea pipe se non è l'ultimo comando */
/* Esegui il comando */
/* Salva la pipe corrente come precedente */
int	process_pipeline_cmd(t_data *data, t_cmd *current,
				int *prev_pipe, int *has_prev)
{
	t_pipe_data	pipe_data;
	int			pipe_fd[2];
	int			pid;

	if (current->next && pipe(pipe_fd) == -1)
	{
		print_error("pipe", "failed");
		return (-1);
	}
	if (*has_prev)
		setup_pipe_data(&pipe_data, prev_pipe, pipe_fd,
			current->next == NULL);
	else
		setup_pipe_data(&pipe_data, NULL, pipe_fd, current->next == NULL);
	pid = exec_pipeline_cmd(data, current, &pipe_data);
	if (*has_prev)
		close_pipes(prev_pipe);
	if (current->next)
		save_pipe(prev_pipe, pipe_fd, has_prev);
	return (pid);
}

/* Esegue una pipeline completa */
/* Aspetta tutti i processi */
int	execute_pipeline(t_data *data)
{
	int	num_cmds;
	int	prev_pipe[2];
	int	has_prev;
	int	result;

	data->cmds_pids = init_pipeline(data, &num_cmds);
	if (!data->cmds_pids)
		return (ERROR);
	has_prev = 0;
	result = pipeline_loop(data, data->cmds_pids, prev_pipe, &has_prev);
	if (result == -1)
	{
		free(data->cmds_pids);
		data->cmds_pids = NULL;
		return (ERROR);
	}
	result = wait_all_processes(data->cmds_pids, num_cmds);
	free(pdata->cmds_pids);
	data->cmds_pids = NULL;
	return (result);
}

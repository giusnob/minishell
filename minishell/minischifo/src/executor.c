/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:03 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/31 00:38:01 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Esegue il comando nel processo figlio */
static void	exec_child_process(t_data *data, t_cmd *cmd, char *cmd_path)
{
	if (apply_redirections(cmd) != SUCCESS)
	{
		cleanup_data(data);
		exit(ERROR);
	}
	if (execve(cmd_path, cmd->args, data->envp) == -1)
	{
		print_error(cmd->args[0], "execution failed");
		free(cmd_path);
		cleanup_child(data);
		exit(CMD_NOT_EXECUTABLE);
	}
}

/* Aspetta il processo figlio e ritorna exit status */
static int	wait_child_process(pid_t pid, char *cmd_path)
{
	int	status;

	free(cmd_path);
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (SUCCESS);
}

/* Esegue un comando esterno con fork + execve */
/* Se pid = 0 Processo figlio */
/* Applica redirections */
/* altrimenti Processo padre */
/* Esegue un comando esterno con fork + execve */
static int	exec_external_cmd(t_data *data, t_cmd *cmd)
{
	pid_t	pid;
	char	*cmd_path;

	cmd_path = find_command_path(cmd->args[0], data->envp);
	if (!cmd_path)
	{
		print_error(cmd->args[0], "command not found");
		return (CMD_NOT_FOUND);
	}
	pid = fork();
	if (pid == -1)
	{
		print_error("fork", "failed");
		free(cmd_path);
		return (ERROR);
	}
	if (pid == 0)
		exec_child_process(data, cmd, cmd_path);
	else
		return (wait_child_process(pid, cmd_path));
	return (SUCCESS);
}

/* Esegue un singolo comando (built-in o esterno) */
/* Controlla se è un built-in */
/* Applica redirections anche per built-ins */
/* Ripristina stdin/stdout se c'erano redirections */
/* Altrimenti esegue comando esterno */
static int	execute_simple_cmd(t_data *data, t_cmd *cmd)
{
	int	exit_status;

	if (!cmd || !cmd->args || !cmd->args[0])
		return (SUCCESS);
	if (is_builtin(cmd->args[0]))
	{
		if (cmd->redirs)
		{
			if (apply_redirections(cmd) != SUCCESS)
			{
				restore_std_fds(data);
				return (ERROR);
			}
		}
		exit_status = exec_builtin(data, cmd);
		if (cmd->redirs)
			restore_std_fds(data);
		return (exit_status);
	}
	return (exec_external_cmd(data, cmd));
}

/* Funzione principale executor */
/* Se ci sono più comandi, usa le pipe */
int	executor(t_data *data)
{
	int	exit_status;

	if (!data->cmd_list)
		return (SUCCESS);
	if (data->cmd_list->next)
		exit_status = execute_pipeline(data);
	else
		exit_status = execute_simple_cmd(data, data->cmd_list);
	return (exit_status);
}

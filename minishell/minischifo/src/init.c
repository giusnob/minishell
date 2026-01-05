/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:32 by ginobile          #+#    #+#             */
/*   Updated: 2026/01/05 02:58:00 by gifanell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void	init_data(t_data *data, char **envp)
{
	data->envp = copy_envp(envp);
	data->export_marks = NULL;
	data->cmd_list = NULL;
	data->cmds_pids = NULL;
	data->last_exit_status = 0;
	data->stdin_backup = dup(STDIN_FILENO);
	data->stdout_backup = dup(STDOUT_FILENO);
}

void	cleanup_data(t_data *data)
{
	if (data->envp)
		free_envp(data->envp);
	if (data->cmd_list)
		free_cmd_list(data->cmd_list);
	close(data->stdin_backup);
	close(data->stdout_backup);
	clear_history();
}

void	cleanup_child(t_data *data)
{
	if (data->envp)
		free_envp(data->envp);
	if (data->export_marks)
		free_envp(data->export_marks);
	if (data->cmd_list)
		free_cmd_list(data->cmd_list);
	if (data->cmds_pids)
		free(data->cmds_pids);
	data->cmds_pids = NULL;
	close(data->stdin_backup);
	close(data->stdout_backup);
}

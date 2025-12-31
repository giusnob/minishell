/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:32 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/31 00:36:20 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void	init_data(t_data *data, char **envp)
{
	data->envp = copy_envp(envp);
	data->cmd_list = NULL;
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
	if (data->cmd_list)
		free_cmd_list(data->cmd_list);
	close(data->stdin_backup);
	close(data->stdout_backup);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:14 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 17:49:02 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static t_cmd	*process_command(t_token **tokens);

static int	handle_process_result(int result, t_cmd *cmd)
{
	if (result == 0)
	{
		free_cmd_list(cmd);
		return (0);
	}
	if (result == -1)
		return (-1);
	return (1);
}

/* Processa un singolo comando (fino alla prossima pipe) */
static t_cmd	*process_command(t_token **tokens)
{
	t_cmd	*cmd;
	t_token	*current;
	int		result;

	cmd = create_cmd();
	if (!cmd)
		return (NULL);
	current = *tokens;
	while (current && current->type != TOKEN_PIPE)
	{
		result = process_token(cmd, &current);
		result = handle_process_result(result, cmd);
		if (result <= 0)
			break ;
	}
	*tokens = current;
	if (!cmd->args)
	{
		free_cmd_list(cmd);
		return (NULL);
	}
	return (cmd);
}

/* Funzione principale del parser */
t_cmd	*parser(t_token *tokens)
{
	t_cmd	*cmd_list;
	t_cmd	*new_cmd;
	t_token	*current;

	if (!tokens)
		return (NULL);
	cmd_list = NULL;
	current = tokens;
	while (current)
	{
		new_cmd = process_command(&current);
		if (!new_cmd)
		{
			free_cmd_list(cmd_list);
			return (NULL);
		}
		add_cmd(&cmd_list, new_cmd);
		if (current && current->type == TOKEN_PIPE)
			current = current->next;
	}
	return (cmd_list);
}

void	free_cmd_list(t_cmd *cmd_list)
{
	t_cmd	*tmp_cmd;
	t_redir	*tmp_redir;

	while (cmd_list)
	{
		tmp_cmd = cmd_list;
		cmd_list = cmd_list->next;
		if (tmp_cmd->args)
			free_array(tmp_cmd->args);
		while (tmp_cmd->redirs)
		{
			tmp_redir = tmp_cmd->redirs;
			tmp_cmd->redirs = tmp_cmd->redirs->next;
			if (tmp_redir->file)
				free(tmp_redir->file);
			free(tmp_redir);
		}
		free(tmp_cmd);
	}
}

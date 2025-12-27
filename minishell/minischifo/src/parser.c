/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:14 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 14:08:47 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static t_cmd	*process_command(t_token **tokens);
static int		process_command_loop(t_cmd *cmd, t_token **current);

void	free_cmd(t_cmd *cmd)
{
	t_redir	*tmp_redir;

	if (!cmd)
		return ;
	if (cmd->args)
		free_array(cmd->args);
	while (cmd->redirs)
	{
		tmp_redir = cmd->redirs;
		cmd->redirs = cmd->redirs->next;
		if (tmp_redir->file)
			free(tmp_redir->file);
		if (tmp_redir->heredoc_content)
			free(tmp_redir->heredoc_content);
		free(tmp_redir);
	}
	free(cmd);
}

static int	process_command_loop(t_cmd *cmd, t_token **current)
{
	int	result;

	while (*current && (*current)->type != TOKEN_PIPE)
	{
		result = process_token(cmd, current);
		if (result == 0)
			return (0);
		if (result == -1)
			break ;
	}
	return (1);
}

/* Processa un singolo comando (fino alla prossima pipe) */
static t_cmd	*process_command(t_token **tokens)
{
	t_cmd	*cmd;
	t_token	*current;

	cmd = create_cmd();
	if (!cmd)
		return (NULL);
	current = *tokens;
	if (!process_command_loop(cmd, &current))
	{
		free_cmd(cmd);
		return (NULL);
	}
	*tokens = current;
	if (!cmd->args)
	{
		free_cmd(cmd);
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
	if (!validate_tokens(tokens))
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

	while (cmd_list)
	{
		tmp_cmd = cmd_list;
		cmd_list = cmd_list->next;
		free_cmd(tmp_cmd);
	}
}

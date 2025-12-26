/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_handlers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:30:01 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 17:45:34 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static t_redir_type	get_redir_type(t_token_type token_type);

/* Aggiunge gli argomenti al comando */
int	add_args_to_cmd(t_cmd *cmd, t_token **tokens)
{
	int		i;
	int		arg_count;
	t_token	*current;

	arg_count = count_args(*tokens);
	if (arg_count == 0)
		return (1);
	cmd->args = malloc(sizeof(char *) * (arg_count + 1));
	if (!cmd->args)
		return (0);
	i = 0;
	current = *tokens;
	while (i < arg_count)
	{
		cmd->args[i] = ft_strdup(current->value);
		if (!cmd->args[i])
			return (0);
		i++;
		current = current->next;
	}
	cmd->args[i] = NULL;
	*tokens = current;
	return (1);
}

static t_redir_type	get_redir_type(t_token_type token_type)
{
	if (token_type == TOKEN_REDIR_IN)
		return (REDIR_IN);
	else if (token_type == TOKEN_REDIR_OUT)
		return (REDIR_OUT);
	else if (token_type == TOKEN_REDIR_APPEND)
		return (REDIR_APPEND);
	else if (token_type == TOKEN_REDIR_HEREDOC)
		return (REDIR_HEREDOC);
	return (REDIR_IN);
}

/* Gestisce una redirection e avanza nei token */
int	handle_redirection(t_cmd *cmd, t_token **tokens)
{
	t_redir_type	type;
	t_redir			*new_redir;
	t_token			*current;

	current = *tokens;
	type = get_redir_type(current->type);
	current = current->next;
	if (!current || current->type != TOKEN_WORD)
	{
		print_error("parser", "syntax error near redirection");
		return (0);
	}
	new_redir = create_redir(type, current->value);
	if (!new_redir)
		return (0);
	add_redir(&cmd->redirs, new_redir);
	*tokens = current->next;
	return (1);
}

int	process_token(t_cmd	*cmd, t_token **current)
{
	if ((*current)->type == TOKEN_WORD)
	{
		if (!add_args_to_cmd(cmd, current))
			return (0);
	}
	else if (((*current)->type >= TOKEN_REDIR_IN)
		&& ((*current)->type <= TOKEN_REDIR_HEREDOC))
	{
		if (!handle_redirection(cmd, current))
			return (0);
	}
	else
		return (-1);
	return (1);
}

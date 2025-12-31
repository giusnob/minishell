/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_handlers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:30:01 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/31 04:30:07 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static t_redir_type	get_redir_type(t_token_type token_type);

/* Determina il tipo di redirection */
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

static int	contains_space(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (ft_isspace(str[i]))
			return (1);
		i++;
	}
	return (0);
}

/* Aggiunge UN SINGOLO argomento al comando */
int	add_args_to_cmd(t_cmd *cmd, t_token **tokens)
{
	int		count;
	char	**new_args;
	char	**split_words;

	count = count_current_args(cmd->args);
	if (count == 0 && contains_space((*tokens)->value))
	{
		split_words = ft_split((*tokens)->value, ' ');
		if (!split_words)
			return (0);
		cmd->args = split_words;
		*tokens = (*tokens)->next;
		return (1);
	}
	new_args = alloc_new_args(count);
	if (!new_args)
		return (0);
	copy_old_args(new_args, cmd->args, count);
	new_args[count] = ft_strdup((*tokens)->value);
	if (!new_args[count])
	{
		free(new_args);
		return (0);
	}
	new_args[count + 1] = NULL;
	if (cmd->args)
		free(cmd->args);
	cmd->args = new_args;
	*tokens = (*tokens)->next;
	return (1);
}

/* Helper: processa heredoc in una redirection */
static int	process_heredoc_redir(t_redir *redir, char *delimiter, t_data *data)
{
	redir->heredoc_content = read_heredoc_content(delimiter, data);
	if (!redir->heredoc_content)
	{
		if (redir->file)
			free(redir->file);
		free(redir);
		return (0);
	}
	return (1);
}

/* Gestisce una redirection e avanza nei token */
int	handle_redirection(t_cmd *cmd, t_token **tokens, t_data *data)
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
	if (type == REDIR_HEREDOC)
	{
		if (!process_heredoc_redir(new_redir, current->value, data))
			return (0);
	}
	add_redir(&cmd->redirs, new_redir);
	*tokens = current->next;
	return (1);
}

/* Processa un singolo token */
int	process_token(t_cmd *cmd, t_token **current, t_data *data)
{
	if ((*current)->type == TOKEN_WORD)
	{
		if (!add_args_to_cmd(cmd, current))
			return (0);
	}
	else if (((*current)->type >= TOKEN_REDIR_IN)
		&& ((*current)->type <= TOKEN_REDIR_HEREDOC))
	{
		if (!handle_redirection(cmd, current, data))
			return (0);
	}
	else
		return (-1);
	return (1);
}

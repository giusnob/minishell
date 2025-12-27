/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_handlers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:38 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 17:02:40 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Gestisce operatori (|, <, >, <<, >>) */
t_token	*handle_pipe(char **input)
{
	t_token_type	type;
	char			*value;

	type = TOKEN_PIPE;
	value = ft_strdup("|");
	(*input)++;
	return (create_token(type, value));
}

t_token	*handle_redir_in(char **input)
{
	t_token_type	type;
	char			*value;

	if ((*input)[1] == '<')
	{
		type = TOKEN_REDIR_HEREDOC;
		value = ft_strdup("<<");
		(*input) += 2;
	}
	else
	{
		type = TOKEN_REDIR_IN;
		value = ft_strdup("<");
		(*input)++;
	}
	return (create_token(type, value));
}

t_token	*handle_redir_out(char **input)
{
	t_token_type	type;
	char			*value;

	if ((*input)[1] == '>')
	{
		type = TOKEN_REDIR_APPEND;
		value = ft_strdup(">>");
		(*input) += 2;
	}
	else
	{
		type = TOKEN_REDIR_OUT;
		value = ft_strdup(">");
		(*input)++;
	}
	return (create_token(type, value));
}

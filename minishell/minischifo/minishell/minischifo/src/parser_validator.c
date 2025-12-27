/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_validator.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 01:07:42 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 01:22:05 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static int	validate_pipe_in_loop(t_token *current);

/* Helper: valida pipe nel loop */
static int	validate_pipe_in_loop(t_token *current)
{
	if (!current->next)
	{
		print_error("parser", "syntax error near unexpected token `|'");
		return (0);
	}
	if (current->next->type == TOKEN_PIPE)
	{
		print_error("parser", "syntax error near unexpected token `|'");
		return (0);
	}
	return (1);
}

/* Valida la sintassi dei token (pipe errors) */
int	validate_tokens(t_token *tokens)
{
	t_token	*current;

	if (!tokens)
		return (1);
	current = tokens;
	if (current->type == TOKEN_PIPE)
	{
		print_error("parser", "syntax error near unexpected token `|'");
		return (0);
	}
	while (current)
	{
		if (current->type == TOKEN_PIPE)
		{
			if (!validate_pipe_in_loop(current))
				return (0);
		}
		current = current->next;
	}
	return (1);
}

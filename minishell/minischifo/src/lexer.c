/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:50 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 20:40:25 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static t_token	*handle_operator(char **input);
static t_token	*handle_word(char **input, t_data *data);

/* Gestisce le parole (con o senza quote) */
static t_token	*handle_word(char **input, t_data *data)
{
	char	*final_word;
	char	*word;
	char	*tmp;

	final_word = ft_strdup("");
	if (!final_word)
		return (NULL);
	while (**input && !ft_isspace(**input)
		&& **input != '|' && **input != '<' && **input != '>')
	{
		word = read_next_part(input, data);
		if (!word)
		{
			free(final_word);
			return (NULL);
		}
		tmp = final_word;
		final_word = ft_strjoin((const char *)final_word, (const char *)word);
		free(tmp);
		free(word);
		if (!final_word)
			return (NULL);
	}
	return (create_token(TOKEN_WORD, final_word));
}

/* Funzione principale del lexer */
t_token	*lexer(char *input, t_data *data)
{
	t_token	*tokens;
	t_token	*new_token;

	tokens = NULL;
	while (*input)
	{
		skip_spaces(&input);
		if (!*input)
			break ;
		if (*input == '|' || *input == '<' || *input == '>')
			new_token = handle_operator(&input);
		else
			new_token = handle_word(&input, data);
		if (!new_token)
		{
			free_tokens(tokens);
			return (NULL);
		}
		add_token(&tokens, new_token);
	}
	return (tokens);
}

void	free_tokens(t_token *tokens)
{
	t_token	*tmp;

	while (tokens)
	{
		tmp = tokens;
		tokens = tokens->next;
		if (tmp->value)
			free(tmp->value);
		free(tmp);
	}
}

static t_token	*handle_operator(char **input)
{
	if (**input == '|')
		return (handle_pipe(input));
	else if (**input == '<')
		return (handle_redir_in(input));
	else if (**input == '>')
		return (handle_redir_out(input));
	return (NULL);
}

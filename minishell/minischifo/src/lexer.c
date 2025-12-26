/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:50 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 00:19:16 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static char		*extract_word(char **input, t_data *data);
static t_token	*handle_operator(char **input);
static t_token	*handle_word(char **input, t_data *data);

/* Estrae una parola normale (CON espansione) */
/* Espande le variabili */
static char	*extract_word(char **input, t_data *data)
{
	char	*start;
	int		len;
	char	*word;
	char	*expanded;

	start = *input;
	len = 0;
	while ((*input)[len] && !ft_isspace((*input)[len]) &&
		(*input)[len] != '|' && (*input)[len] != '<' && (*input)[len] != '>' &&
		(*input)[len] != '\'' && (*input)[len] != '"')
		len++;
	word = (char *)malloc(sizeof(char) * (len + 1));
	if (!word)
		return (NULL);
	ft_strncpy(word, start, len);
	word[len] = '\0';
	*input += len;
	expanded = expand_variables(word, data);
	free(word);
	return (expanded);
}

/* Gestisce le parole (con o senza quote) */
static t_token	*handle_word(char **input, t_data *data)
{
	char	*word;

	if (**input == '\'')
		word = extract_single_quote(input);
	else if (**input == '"')
		word = extract_double_quote(input, data);
	else
		word = extract_word(input, data);
	if (!word)
		return (NULL);
	return (create_token(TOKEN_WORD, word));
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

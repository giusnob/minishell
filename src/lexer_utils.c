/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:45 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 00:19:02 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Crea un nuovo token */
t_token	*create_token(t_token_type type, char *value)
{
	t_token	*token;

	token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	token->value = value;
	token->next = NULL;
	return (token);
}

/* Aggiunge un token alla fine della lista */
void	add_token(t_token **head, t_token *new_token)
{
	t_token	*current;

	if (!*head)
	{
		*head = new_token;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_token;
}

/* Salta gli spazi bianchi */
void	skip_spaces(char **input)
{
	while (**input && ft_isspace(**input))
		(*input)++;
}

/* Estrae una parola tra quote singole (NO espansione) */
char	*extract_single_quote(char **input)
{
	char	*start;
	int		len;
	char	*word;

	(*input)++;
	start = *input;
	len = 0;
	while ((*input)[len] && (*input)[len] != '\'')
		len++;
	if ((*input)[len] != '\'')
	{
		print_error("lexer", "unclosed single quote");
		return (NULL);
	}
	word = (char *)malloc(sizeof(char) * (len + 1));
	if (!word)
		return (NULL);
	ft_strncpy(word, start, len);
	word[len] = '\0';
	*input += len + 1;
	return (word);
}

/* Estrae una parola tra quote doppie (CON espansione) */
/* Espande le variabili */
char	*extract_double_quote(char **input, t_data *data)
{
	char	*start;
	int		len;
	char	*word;
	char	*expanded;

	(*input)++;
	start = *input;
	len = 0;
	while ((*input)[len] && (*input)[len] != '"')
		len++;
	if ((*input)[len] != '"')
	{
		print_error("lexer", "unclosed double quote");
		return (NULL);
	}
	word = (char *)malloc(sizeof(char) * (len + 1));
	if (!word)
		return (NULL);
	ft_strncpy(word, start, len);
	word[len] = '\0';
	*input += len + 1;
	expanded = expand_variables(word, data);
	free(word);
	return (expanded);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_helpers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 20:35:30 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 20:42:28 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Estrae una parte di word (per concatenazione) */
char	*extract_word_part(char **input, t_data *data)
{
	char	*start;
	int		len;
	char	*word;
	char	*expanded;

	start = *input;
	len = 0;
	while ((*input)[len] && !ft_isspace((*input)[len]) &&
		(*input)[len] != '|' && (*input)[len] != '<' &&
		(*input)[len] != '>' && (*input)[len] != '\'' &&
		(*input)[len] != '"')
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

/* Legge la prossima parte (quote o word) */
char	*read_next_part(char **input, t_data *data)
{
	if (**input == '\'')
		return (extract_single_quote(input));
	else if (**input == '"')
		return (extract_double_quote(input, data));
	else
		return (extract_word_part(input, data));
}

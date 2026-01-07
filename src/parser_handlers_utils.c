/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_handlers_utils.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/27 01:39:26 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 01:41:02 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Conta quanti argomenti ci sono già nell'array */
int	count_current_args(char **args)
{
	int	count;

	count = 0;
	if (args)
	{
		while (args[count])
			count++;
	}
	return (count);
}

/* Alloca nuovo array args con spazio per un elemento in più */
char	**alloc_new_args(int count)
{
	char	**new_args;
	int		i;

	new_args = (char **)malloc(sizeof(char *) * (count + 2));
	if (!new_args)
		return (NULL);
	i = -1;
	while (++i < count + 2)
		new_args[i] = NULL;
	return (new_args);
}

/* Copia vecchi args nel nuovo array */
void	copy_old_args(char **new_args, char **old_args, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		new_args[i] = old_args[i];
		i++;
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export_utils.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 23:58:10 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/30 00:19:57 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	is_valid_identifier(char *key)
{
	int	i;

	if (!key || !key[0])
		return (0);
	if (key[0] >= '0' && key[0] <= '9')
		return (0);
	if (key[0] != '_' && !(key[0] >= 'a' && key[0] <= 'z')
		&& !(key[0] >= 'A' && key[0] <= 'Z'))
		return (0);
	i = 1;
	while (key[i])
	{
		if (key[i] != '_' && !(key[i] >= 'a' && key[i] <= 'z')
			&& !(key[i] >= 'A' && key[i] <= 'Z')
			&& !(key[i] >= '0' && key[i] <= '9'))
			return (0);
		i++;
	}
	return (1);
}

/* Alloca e copia la key */
char	*extract_key(char *arg, int len)
{
	char	*key;

	key = (char *)malloc(sizeof(char) * (len + 1));
	if (!key)
		return (NULL);
	ft_strncpy(key, arg, len);
	key[len] = '\0';
	return (key);
}

/* Helper per validazione export */
int	validate_export_arg(char *key, char *value)
{
	if (!is_valid_identifier(key))
	{
		print_error("export", "not a valid identifier");
		free(key);
		if (value)
			free(value);
		return (0);
	}
	return (1);
}

/* Estrae valore dopo '=' */
static int	extract_value(char *arg, int i, char **key, char **value)
{
	*value = ft_strdup(arg + i + 1);
	if (!*value)
	{
		free(*key);
		*key = NULL;
		return (-1);
	}
	return (1);
}

/* Estrae key e value da una stringa "KEY=VALUE" */
int	parse_export_arg(char *arg, char **key, char **value)
{
	int	i;

	i = 0;
	while (arg[i] && arg[i] != '=')
		i++;
	if (i == 0)
	{
		*key = NULL;
		*value = NULL;
		return (0);
	}
	*key = extract_key(arg, i);
	if (!*key)
		return (-1);
	if (arg[i] == '=')
		return (extract_value(arg, i, key, value));
	*value = NULL;
	return (0);
}

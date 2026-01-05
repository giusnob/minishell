/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:20 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/29 04:54:41 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static char	*extract_braced_var(char *str, int *len)
{
	int		i;
	int		start;
	char	*name;

	i = 1;
	start = i;
	while (str[i] && str[i] != '}' && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	if (str[i] != '}')
	{
		*len = 1;
		return (NULL);
	}
	name = (char *)malloc(sizeof(char) * (i - start + 1));
	if (!name)
		return (NULL);
	ft_strncpy(name, str + start, i - start);
	name[i - start] = '\0';
	*len = i - start + 3;
	return (name);
}

/* Estrae nome variabile normale $VAR */
static char	*extract_normal_var(char *str, int *len)
{
	int		i;
	char	*name;

	i = 0;
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	if (i == 0)
	{
		*len = 1;
		return (NULL);
	}
	name = (char *)malloc(sizeof(char) * (i + 1));
	if (!name)
		return (NULL);
	ft_strncpy(name, str, i);
	name[i] = '\0';
	*len = i + 1;
	return (name);
}

/* Estrae il nome della variabile (es: "USER" da "$USER") */
/* Salta il $ */
/* Caso speciale: $? */
/* Estrai il nome della variabile */
char	*extract_var_name(char *str, int *len)
{
	int		i;

	i = 0;
	if (str[i] == '$')
		i++;
	if (str[i] == '?')
	{
		*len = 2;
		return (ft_strdup("?"));
	}
	if (str[i] == '{')
		return (extract_braced_var(str + i, len));
	return (extract_normal_var(str + i, len));
}

/* Espande una singola variabile */
/* Caso speciale: $? */
/* Cerca la variabile nell'environment */
char	*expand_single_var(char *var_name, t_data *data)
{
	char	*value;

	if (!var_name)
		return (ft_strdup("$"));
	if (ft_strcmp(var_name, "?") == 0)
		return (ft_itoa(data->last_exit_status));
	value = get_env_value(var_name, data->envp);
	if (!value)
		return (ft_strdup(""));
	return (value);
}

/* Calcola la lunghezza della stringa espansa */
int	calculate_expanded_len(char *str, t_data *data)
{
	int	len;
	int	i;

	len = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
			len += get_var_length(str, &i, data);
		else
		{
			len++;
			i++;
		}
	}
	return (len);
}

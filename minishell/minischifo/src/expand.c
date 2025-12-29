/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:20 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/29 03:46:30 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Estrae il nome della variabile (es: "USER" da "$USER") */
/* Salta il $ */
/* Caso speciale: $? */
/* Estrai il nome della variabile */
char	*extract_var_name(char *str, int *len)
{
	int		i;
	int		start;
	char	*name;

	i = 0;
	if (str[i] == '$')
		i++;
	if (str[i] == '?')
	{
		*len = 2;
		return (ft_strdup("?"));
	}
	if (str[i] == '{')
	{
		i++;
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
	start = i;
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	if (i == start)
	{
		*len = 1;
		return (NULL);
	}
	name = (char *)malloc(sizeof(char) * (i - start + 1));
	if (!name)
		return (NULL);
	ft_strncpy(name, str + start, i - start);
	name[i - 1] = '\0';
	*len = i - start + 1;
	return (name);
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

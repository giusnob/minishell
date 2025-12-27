/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:00:19 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 19:42:31 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Stampa una singola variabile export */
static void	print_single_export(char *env_var)
{
	int	j;

	ft_putstr_fd("declare -x ", STDOUT_FILENO);
	j = 0;
	while (env_var[j] && env_var[j] != '=')
	{
		write(STDOUT_FILENO, &env_var[j], 1);
		j++;
	}
	if (env_var[j] == '=')
	{
		write(STDOUT_FILENO, "=\"", 2);
		j++;
		while (env_var[j])
		{
			write(STDOUT_FILENO, &env_var[j], 1);
			j++;
		}
		write(STDOUT_FILENO, "\"", 1);
	}
	write(STDOUT_FILENO, "\n", 1);
}

/* Stampa tutte le variabili d'ambiente in formato export */
static void	print_export_vars(t_data *data)
{
	int	i;

	i = 0;
	while (data->envp[i])
	{
		print_single_export(data->envp[i]);
		i++;
	}
}

/* Estrae key e value da una stringa "KEY=VALUE" */
static int	parse_export_arg(char *arg, char **key, char **value)
{
	int	i;

	i = 0;
	while (arg[i] && arg[i] != '=')
		i++;
	if (i == 0)
		return (0);
	*key = (char *)malloc(sizeof(char) * (i + 1));
	if (!*key)
		return (0);
	ft_strncpy(*key, arg, i);
	(*key)[i] = '\0';
	if (arg[i] == '=')
	{
		*value = ft_strdup(arg + i + 1);
		if (!*value)
		{
			free(*key);
			return (0);
		}
	}
	else
		*value = ft_strdup("");
	return (1);
}

/* Processa un singolo argomento export */
static int	process_export_arg(char *arg, t_data *data)
{
	char	*key;
	char	*value;

	if (parse_export_arg(arg, &key, &value))
	{
		set_env_value(key, value, data);
		free(key);
		free(value);
		return (SUCCESS);
	}
	else
	{
		print_error("export", "invalid identifier");
		return (ERROR);
	}
}

/* Se non ci sono argomenti, stampa tutte le variabili */
/* Altrimenti esporta le variabili */
int	builtin_export(char **args, t_data *data)
{
	int	i;
	int	ret;

	if (!args[1])
	{
		print_export_vars(data);
		return (SUCCESS);
	}
	ret = SUCCESS;
	i = 1;
	while (args[i])
	{
		if (process_export_arg(args[i], data) == ERROR)
			ret = ERROR;
		i++;
	}
	return (ret);
}

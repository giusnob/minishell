/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:00:19 by ginobile          #+#    #+#             */
/*   Updated: 2026/01/05 02:11:27 by gifanell         ###   ########.fr       */
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
/*Stampa export marks (variabili ambientali senza valore)*/
static void	print_export_marks(t_data *data)
{
	int i;

	if (!data->export_marks)
		return ;
	i = 0;
	while (data->export_marks[i])
	{
		ft_putstr_fd("declare -x ", STDOUT_FILENO);
		ft_putstr_fd(data->export_marks[i], STDOUT_FILENO);
		write(STDOUT_FILENO, "\n", 1);
		i++;
	}
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
	print_export_marks(data);
}

/* Helper: gestisce export con value*/
static void	handle_export_with_value(char *key, char *value, t_data *data)
{
	remove_export_mark(key, data);
	set_env_value(key, value, data);
	free(key);
	free(value);
}

/* Processa un singolo argomento export */
static int	process_export_arg(char *arg, t_data *data)
{
	char	*key;
	char	*value;
	int		has_equal;

	has_equal = parse_export_arg(arg, &key, &value);
	if (has_equal == -1)
		return (print_error("export", "invalid identifier"), ERROR);
	if (has_equal == 0 && !key)
		return (print_error("export", "not a valid identifier"), ERROR);
	if (!validate_export_arg(key, value))
		return (ERROR);
	if (has_equal == 1)
		handle_export_with_value(key, value, data);
	else
	{
		value = get_env_value(key, data->envp);
		if (!value)
			add_export_mark(key, data);
	}
	return (free(key), free(value), SUCCESS);
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

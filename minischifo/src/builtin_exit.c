/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_exit.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:00:12 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 18:09:00 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

static int	is_numeric(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '+' || str[i] == '-')
		i++;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

static int	ft_atoi(const char *str)
{
	int	result;
	int	sign;
	int	i;

	result = 0;
	sign = 1;
	i = 0;
	while (ft_isspace(str[i]))
		i++;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (ft_isdigit(str[i]))
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (sign * result);
}

int	builtin_exit(char **args, t_data *data)
{
	int		exit_code;

	ft_putendl_fd("exit", STDOUT_FILENO);
	if (!args[1])
	{
		cleanup_data(data);
		exit(data->last_exit_status);
	}
	if (!is_numeric(args[1]))
	{
		print_error("exit", "numeric argument required");
		cleanup_data(data);
		exit(2);
	}
	if (args[2])
	{
		print_error("exit", "too many arguments");
		return (ERROR);
	}
	exit_code = ft_atoi(args[1]);
	cleanup_data(data);
	exit(exit_code % 256);
}

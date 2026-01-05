/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_unset.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:00:31 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 17:00:33 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	builtin_unset(char **args, t_data *data)
{
	int	i;
	int	ret;

	if (!args[1])
		return (SUCCESS);
	ret = SUCCESS;
	i = 1;
	while (args[i])
	{
		if (unset_env_value(args[i], data) != SUCCESS)
			ret = ERROR;
		i++;
	}
	return (ret);
}

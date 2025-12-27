/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:00:55 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 00:16:07 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

char	*get_env_value(char *key, char **envp)
{
	int		i;
	int		key_len;
	char	*value;

	if (!key || !envp)
		return (NULL);
	key_len = 0;
	while (key[key_len])
		key_len++;
	i = 0;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], key, key_len) == 0 && envp[i][key_len] == '=')
		{
			value = ft_strdup(envp[i] + key_len + 1);
			return (value);
		}
		i++;
	}
	return (NULL);
}

char	**copy_envp(char **envp)
{
	char	**copy;
	int		i;
	int		count;

	count = 0;
	while (envp[count])
		count++;
	copy = (char **)malloc(sizeof(char *) * (count + 1));
	if (!copy)
		return (NULL);
	i = 0;
	while (i < count)
	{
		copy[i] = ft_strdup(envp[i]);
		if (!copy[i])
		{
			free_array(copy);
			return (NULL);
		}
		i++;
	}
	copy[i] = NULL;
	return (copy);
}

void	free_envp(char **envp)
{
	free_array(envp);
}

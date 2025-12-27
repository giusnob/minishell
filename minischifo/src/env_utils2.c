/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils2.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 19:35:22 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 00:16:38 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Aggiunge una nuova variabile all'environment */
static int	add_new_env(t_data *data, char *key, char *value, int count)
{
	char	**new_envp;
	char	*temp;
	char	*new_entry;
	int		i;

	new_envp = (char **)malloc(sizeof(char *) * (count + 2));
	if (!new_envp)
		return (ERROR);
	i = 0;
	while (data->envp[i])
	{
		new_envp[i] = data->envp[i];
		i++;
	}
	temp = ft_strjoin(key, "=");
	new_entry = ft_strjoin(temp, value);
	free(temp);
	new_envp[i] = new_entry;
	new_envp[i + 1] = NULL;
	free(data->envp);
	data->envp = new_envp;
	return (SUCCESS);
}

int	set_env_value(char *key, char *value, t_data *data)
{
	char	*new_entry;
	char	*temp;
	int		i;
	int		key_len;

	if (!key || !value)
		return (ERROR);
	key_len = ft_strlen(key);
	i = 0;
	while (data->envp[i])
	{
		if (ft_strncmp(data->envp[i], key, key_len) == 0
			&& data->envp[i][key_len] == '=')
		{
			temp = ft_strjoin(key, "=");
			new_entry = ft_strjoin(temp, value);
			free(temp);
			free(data->envp[i]);
			data->envp[i] = new_entry;
			return (SUCCESS);
		}
		i++;
	}
	return (add_new_env(data, key, value, i));
}

/* Conta env senza key */
static int	count_env_without_key(char **envp, char *key, int key_len)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (envp[i])
	{
		if (!(ft_strncmp(envp[i], key, key_len) == 0
				&& envp[i][key_len] == '='))
			count++;
		i++;
	}
	return (count);
}

/* Copia environment escludendo una chiave */
static char	**copy_env_without_key(char **envp, char *key, int key_len)
{
	char	**new_envp;
	int		i;
	int		j;

	new_envp = (char **)malloc(sizeof(char *)
			* (count_env_without_key(envp, key, key_len) + 1));
	if (!new_envp)
		return (NULL);
	i = 0;
	j = 0;
	while (envp[i])
	{
		if (!(ft_strncmp(envp[i], key, key_len) == 0
				&& envp[i][key_len] == '='))
			new_envp[j++] = envp[i];
		else
			free(envp[i]);
		i++;
	}
	new_envp[j] = NULL;
	return (new_envp);
}

int	unset_env_value(char *key, t_data *data)
{
	char	**new_envp;
	int		key_len;

	if (!key)
		return (ERROR);
	key_len = ft_strlen(key);
	new_envp = copy_env_without_key(data->envp, key, key_len);
	if (!new_envp)
		return (ERROR);
	free(data->envp);
	data->envp = new_envp;
	return (SUCCESS);
}

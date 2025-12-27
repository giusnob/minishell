/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   path_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:21 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 19:28:06 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Verifica se un file esiste ed è eseguibile */
static int	is_executable(char *path)
{
	struct stat	st;

	if (access(path, F_OK) == 0)
	{
		if (stat(path, &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
				return (0);
			if (access(path, X_OK) == 0)
				return (1);
		}
	}
	return (0);
}

/* Controlla se il comando contiene '/' (path assoluto o relativo) */
static int	has_slash(char *cmd)
{
	int	i;

	i = 0;
	while (cmd[i])
	{
		if (cmd[i] == '/')
			return (1);
		i++;
	}
	return (0);
}

/* Controlla se il comando esiste in una directory */
static char	*check_path_dir(char *dir, char *cmd)
{
	char	*temp;
	char	*full_path;

	temp = ft_strjoin(dir, "/");
	full_path = ft_strjoin(temp, cmd);
	free(temp);
	if (is_executable(full_path))
		return (full_path);
	free(full_path);
	return (NULL);
}

/* Cerca il comando nelle directory del PATH */
static char	*search_in_path(char *cmd, char **envp)
{
	char	**paths;
	char	*path_value;
	char	*full_path;
	int		i;

	path_value = get_env_value("PATH", envp);
	if (!path_value)
		return (NULL);
	paths = ft_split(path_value, ':');
	free(path_value);
	if (!paths)
		return (NULL);
	i = 0;
	while (paths[i])
	{
		full_path = check_path_dir(paths[i], cmd);
		if (full_path)
		{
			free_array(paths);
			return (full_path);
		}
		i++;
	}
	free_array(paths);
	return (NULL);
}

/* Trova il path completo di un comando */
/* Se il comando contiene '/', è un path assoluto o relativo */
/* Altrimenti cerca nel PATH */
char	*find_command_path(char *cmd, char **envp)
{
	if (!cmd || !*cmd)
		return (NULL);
	if (has_slash(cmd))
	{
		if (is_executable(cmd))
			return (ft_strdup(cmd));
		return (NULL);
	}
	return (search_in_path(cmd, envp));
}

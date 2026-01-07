/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 16:59:43 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 17:45:16 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Aggiorna pwd e oldpwd nell'environment */
static void	update_pwd(t_data *data, char *old_pwd)
{
	char	cwd[1024];

	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		set_env_value((char *)"OLDPWD", old_pwd, data);
		set_env_value((char *)"PWD", cwd, data);
	}
}

/* Gestisce cd senza argomenti (HOME) */
static int	cd_home(t_data *data, char *old_pwd)
{
	char	*home;

	home = get_env_value((char *)"HOME", data->envp);
	if (!home)
	{
		print_error("cd", "HOME not set");
		return (ERROR);
	}
	if (chdir(home) == -1)
	{
		print_error_errno("cd", home);
		free(home);
		return (ERROR);
	}
	free(home);
	update_pwd(data, old_pwd);
	return (SUCCESS);
}

/* Gestisce cd - (OLDPWD) */
static int	cd_oldpwd(t_data *data, char *old_pwd)
{
	char	*oldpwd;

	oldpwd = get_env_value((char *)"OLDPWD", data->envp);
	if (!oldpwd)
	{
		print_error("cd", "OLDPWD not set");
		return (ERROR);
	}
	ft_putendl_fd(oldpwd, STDOUT_FILENO);
	if (chdir(oldpwd) == -1)
	{
		print_error_errno("cd", oldpwd);
		free(oldpwd);
		return (ERROR);
	}
	free(oldpwd);
	update_pwd(data, old_pwd);
	return (SUCCESS);
}

/* Gestisce cd ~ (tilde expansion) */
static int	cd_tilde(char *arg, t_data *data, char *old_pwd)
{
	char	*home;
	char	*path;

	home = get_env_value((char *)"HOME", data->envp);
	if (!home)
	{
		print_error("cd", "HOME not set");
		return (ERROR);
	}
	if (arg[1] == '\0')
		path = ft_strdup(home);
	else
		path = ft_strjoin(home, arg + 1);
	free(home);
	if (!path)
		return (ERROR);
	if (chdir(path) == -1)
	{
		print_error_errno("cd", path);
		free(path);
		return (ERROR);
	}
	free(path);
	update_pwd(data, old_pwd);
	return (SUCCESS);
}

int	builtin_cd(char **args, t_data *data)
{
	char	old_pwd[1024];

	if (args[0] && args[1] && args[2])
		return (print_error("cd", "too many arguments"), ERROR);
	if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
		old_pwd[0] = '\0';
	if (!args[1])
		return (cd_home(data, old_pwd));
	if (ft_strcmp(args[1], "-") == 0)
		return (cd_oldpwd(data, old_pwd));
	if (args[1][0] == '~')
		return (cd_tilde(args[1], data, old_pwd));
	if (chdir(args[1]) == -1)
	{
		print_error_errno("cd", args[1]);
		return (ERROR);
	}
	update_pwd(data, old_pwd);
	return (SUCCESS);
}

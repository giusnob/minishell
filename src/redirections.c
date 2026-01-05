/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:03:33 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 14:06:34 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Apre un file per una redirection */
static int	open_redir_file(t_redir *redir)
{
	int	fd;

	fd = -1;
	if (redir->type == REDIR_IN)
	{
		fd = open(redir->file, O_RDONLY);
		if (fd == -1)
			print_error(redir->file, "no such file or directory");
	}
	else if (redir->type == REDIR_OUT)
	{
		fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd == -1)
			print_error(redir->file, "cannot create file");
	}
	else if (redir->type == REDIR_APPEND)
	{
		fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd == -1)
			print_error(redir->file, "cannot create file");
	}
	else if (redir->type == REDIR_HEREDOC)
		fd = handle_heredoc(redir->heredoc_content);
	return (fd);
}

/* Applica una singola redirection */
static int	apply_single_redir(t_redir *redir, int fd)
{
	int	target_fd;

	if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC)
		target_fd = STDIN_FILENO;
	else
		target_fd = STDOUT_FILENO;
	if (dup2(fd, target_fd) == -1)
	{
		print_error("dup2", "failed");
		return (ERROR);
	}
	return (SUCCESS);
}

/* Applica tutte le redirections di un comando */
/* Redirigi stdin o stdout */
int	apply_redirections(t_cmd *cmd)
{
	t_redir	*redir;
	int		fd;

	redir = cmd->redirs;
	while (redir)
	{
		fd = open_redir_file(redir);
		if (fd == -1)
			return (ERROR);
		if (cmd->args && cmd->args[0]
			&& apply_single_redir(redir, fd) == ERROR)
			return (close(fd), ERROR);
		close(fd);
		redir = redir->next;
	}
	return (SUCCESS);
}

/* Ripristina stdin e stdout originali */
void	restore_std_fds(t_data *data)
{
	dup2(data->stdin_backup, STDIN_FILENO);
	dup2(data->stdout_backup, STDOUT_FILENO);
}

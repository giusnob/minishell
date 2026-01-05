/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 19:17:17 by ginobile          #+#    #+#             */
/*   Updated: 2026/01/05 03:54:50 by gifanell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Strlen personalizzato per evitare conflitti */
int	ft_strlen_custom(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

/* Gestisce heredoc (<<) */
int	handle_heredoc(char *content)
{
	int		pipe_fd[2];

	if (pipe(pipe_fd) == -1)
	{
		print_error("heredoc", "pipe failed");
		return (-1);
	}
	if (content && *content)
	{
		write(pipe_fd[1], content, ft_strlen(content));
	}
	close(pipe_fd[1]);
	return (pipe_fd[0]);
}

/* Helper: concatena line + newline a content */
static char	*append_line_to_content(char *content, char *line)
{
	char	*tmp;
	char	*result;

	tmp = content;
	result = ft_strjoin((const char *)content, (const char *)line);
	free(tmp);
	if (!result)
		return (NULL);
	tmp = result;
	result = ft_strjoin((const char *)result, "\n");
	free(tmp);
	return (result);
}

/* Legge il contenuto dell'heredoc e lo ritorna come stringa */
char	*read_heredoc_content(char *delimiter, t_data *data)
{
	char	*line;
	char	*content;

	content = ft_strdup("");
	if (!content)
		return (NULL);
	while (1)
	{
		line = readline("> ");
		if (!line || ft_strcmp(line, delimiter) == 0)
		{
			free(line);
			break ;
		}
		content = append_line_to_content(content, line, data);
		free(line);
		if (!content)
			return (NULL);
	}
	return (content);
}

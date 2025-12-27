/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 14:16:57 by giusmerynob       #+#    #+#             */
/*   Updated: 2025/12/27 01:13:10 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <signal.h>
# include <errno.h>
# include <readline/readline.h>
# include <readline/history.h>

/* Exit codes */
# define SUCCESS 0
# define ERROR 1
# define SYNTAX_ERROR 2
# define CMD_NOT_FOUND 127
# define CMD_NOT_EXECUTABLE 126

/* Token types */
typedef enum e_token_type
{
	TOKEN_WORD,
	TOKEN_PIPE,
	TOKEN_REDIR_IN,
	TOKEN_REDIR_OUT,
	TOKEN_REDIR_APPEND,
	TOKEN_REDIR_HEREDOC,
	TOKEN_END
}	t_token_type;
/* Struttura per i token */
typedef struct s_token
{
	t_token_type	type;
	char			*value;
	struct s_token	*next;
}	t_token;

/* Tipi di redirection */
typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}	t_redir_type;
/* Struttura per le redirections */
typedef struct s_redir
{
	t_redir_type	type;
	char			*file;
	struct s_redir	*next;
}	t_redir;
/* Struttura per un singolo comando */
typedef struct s_cmd
{
	char			**args;		// array di argomenti (args[0] è il comando)
	t_redir			*redirs;	// lista di redirections
	struct s_cmd	*next;		// prossimo comando (per le pipe)
}	t_cmd;

/* Struttura principale - minishell data */
typedef struct s_data
{
	char			**envp;			// environment variables
	t_cmd			*cmd_list;		// lista dei comandi
	int				last_exit_status;	// exit status dell'ultimo comando ($?)
	int				stdin_backup;	// backup di stdin
	int				stdout_backup;	// backup di stdout
}	t_data;

typedef struct s_pipe_data
{
	int	prev_pipe[2];
	int	curr_pipe[2];
	int	is_last;
}	t_pipe_data;

/* Struttura per espansione variabili */
typedef struct s_expand_ctx
{
	char	*str;
	char	*result;
	int		*i;
	int		*j;
	t_data	*data;
}	t_expand_ctx;

/* Variabile globale per i segnali (unica permessa) */
extern int			g_signal;

/* === MAIN === */
void				init_data(t_data *data, char **envp);
void				cleanup_data(t_data *data);

/* === SIGNALS === */
void				setup_signals(void);
void				handle_sigint(int sig);
void				handle_sigquit(int sig);

/* === LEXER === */
t_token				*lexer(char *input, t_data *data);
void				free_tokens(t_token *tokens);

/* === LEXER UTILS === */
t_token				*create_token(t_token_type type, char *value);
void				add_token(t_token **head, t_token *new_token);
void				skip_spaces(char **input);
char				*extract_single_quote(char **input);
char				*extract_double_quote(char **input, t_data *data);

/* === LEXER HANDLERS === */
t_token				*handle_pipe(char **input);
t_token				*handle_redir_in(char **input);
t_token				*handle_redir_out(char **input);

/* === PARSER === */
t_cmd				*parser(t_token *tokens);
int					validate_tokens(t_token *tokens);
void				free_cmd(t_cmd *cmd);
void				free_cmd_list(t_cmd *cmd_list);

/* === PARSER UTILS === */
t_cmd				*create_cmd(void);
t_redir				*create_redir(t_redir_type type, char *file);
void				add_redir(t_redir **head, t_redir *new_redir);
int					count_args(t_token *tokens);
void				add_cmd(t_cmd **head, t_cmd *new_cmd);

/* === PARSER HANDLERS === */
int					add_args_to_cmd(t_cmd *cmd, t_token **tokens);
int					handle_redirection(t_cmd *cmd, t_token **tokens);
int					process_token(t_cmd *cmd, t_token **current);

/* === EXPANSION === */
char				*expand_variables(char *str, t_data *data);

/* === EXPANSION UTILS === */
int					ft_strlen(char *str);
int					ft_isalnum(int c);
char				*ft_itoa(int n);

/* === EXPAND HELPERS === */
char				*extract_var_name(char *str, int *len);
char				*expand_single_var(char *var_name, t_data *data);
int					get_var_length(char *str, int *i, t_data *data);
void				copy_var_to_result(char *result, int *j, char *var_value);
int					calculate_expanded_len(char *str, t_data *data);
char				*expand_variables(char *str, t_data *data);

/* === EXECUTOR === */
int					executor(t_data *data);
char				*find_command_path(char *cmd, char **envp);

/* === REDIRECTIONS === */
int					apply_redirections(t_cmd *cmd);
void				restore_std_fds(t_data *data);

/* === REDIRECTIONS UTILS === */
int					ft_strlen_custom(char *str);
int					handle_heredoc(char *delimiter);

/* === PIPES === */
int					execute_pipeline(t_data *data);
int					process_pipeline_cmd(t_data *data, t_cmd *current,
						int *prev_pipe, int *has_prev);

/* === PIPES UTILS === */
void				setup_pipe_data(t_pipe_data *pipe_data, int *prev_pipe,
						int *curr_pipe, int is_last);
void				exec_cmd_in_pipe(t_data *data, t_cmd *cmd, char *cmd_path);
int					count_commands(t_cmd *cmd_list);
int					wait_all_processes(int *pids, int num_cmds);

/* === PIPES HELPERS === */
void				close_pipes(int *pipe_fd);
void				save_pipe(int *prev_pipe, int *pipe_fd, int *has_prev);
int					*init_pipeline(t_data *data, int *num_cmds);
void				setup_child_pipes(t_pipe_data *pipe_data);
int					pipeline_loop(t_data *data, int *pids, int *prev_pipe,
						int *has_prev);

/* === BUILTINS === */
int					is_builtin(char *cmd);
int					exec_builtin(t_data *data, t_cmd *cmd);
int					builtin_echo(char **args);
int					builtin_cd(char **args, t_data *data);
int					builtin_pwd(void);
int					builtin_export(char **args, t_data *data);
int					builtin_unset(char **args, t_data *data);
int					builtin_env(t_data *data);
int					builtin_exit(char **args, t_data *data);

/* === ENV UTILS === */
char				*get_env_value(char *key, char **envp);
char				**copy_envp(char **envp);
void				free_envp(char **envp);
int					set_env_value(char *key, char *value, t_data *data);
int					unset_env_value(char *key, t_data *data);

/* === UTILS === */
char				*ft_strdup(const char *s);
char				*ft_strjoin(char const *s1, char const *s2);
char				*ft_strncpy(char *dest, const char *src, size_t n);
int					ft_strcmp(const char *s1, const char *s2);
int					ft_strncmp(const char *s1, const char *s2, size_t n);
char				**ft_split(char const *s, char c);
void				free_array(char **arr);
int					ft_isspace(int c);
void				ft_putstr_fd(char *s, int fd);
void				ft_putendl_fd(char *s, int fd);

/* === ERROR === */
void				print_error(char *cmd, char *msg);
void				exit_error(char *msg);

#endif

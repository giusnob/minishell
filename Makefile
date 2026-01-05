# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/26 16:59:28 by ginobile          #+#    #+#              #
#    Updated: 2026/01/05 05:24:26 by gifanell         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Colors
MAGENTA = \033[1;35m
CYAN = \033[1;36m
YELLOW = \033[1;33m
GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

# Program name
NAME		= minishell

# Directories
SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= includes

# Compiler and flags
CC			= cc
CFLAGS		= -Wall -Wextra -Werror -g
INCLUDES	= -I$(INC_DIR) -I/opt/homebrew/opt/readline/include
LDFLAGS		= -L/opt/homebrew/opt/readline/lib -lreadline

# Source files
SRCS		= $(SRC_DIR)/main.c \
			  $(SRC_DIR)/init.c \
			  $(SRC_DIR)/signals.c \
			  $(SRC_DIR)/lexer.c \
			  $(SRC_DIR)/lexer_utils.c \
			  $(SRC_DIR)/lexer_helpers.c \
			  $(SRC_DIR)/lexer_handlers.c \
			  $(SRC_DIR)/parser.c \
			  $(SRC_DIR)/parser_utils.c \
			  $(SRC_DIR)/parser_validator.c \
			  $(SRC_DIR)/parser_handlers.c \
			  $(SRC_DIR)/parser_handlers_utils.c \
			  $(SRC_DIR)/executor.c \
			  $(SRC_DIR)/path_utils.c \
			  $(SRC_DIR)/expand.c \
			  $(SRC_DIR)/expand_utils.c \
			  $(SRC_DIR)/expand_helpers.c \
			  $(SRC_DIR)/export_marks_utils.c \
			  $(SRC_DIR)/redirections.c \
			  $(SRC_DIR)/redirections_utils.c \
			  $(SRC_DIR)/pipes.c \
			  $(SRC_DIR)/pipes_utils.c \
			  $(SRC_DIR)/pipes_helpers.c \
			  $(SRC_DIR)/builtins.c \
			  $(SRC_DIR)/builtin_echo.c \
			  $(SRC_DIR)/builtin_cd.c \
			  $(SRC_DIR)/builtin_pwd.c \
			  $(SRC_DIR)/builtin_export.c \
			  $(SRC_DIR)/builtin_export_utils.c \
			  $(SRC_DIR)/builtin_unset.c \
			  $(SRC_DIR)/builtin_env.c \
			  $(SRC_DIR)/builtin_exit.c \
			  $(SRC_DIR)/env_utils.c \
			  $(SRC_DIR)/env_utils2.c \
			  $(SRC_DIR)/utils.c \
			  $(SRC_DIR)/utils2.c \
			  $(SRC_DIR)/utils_fd.c \
			  $(SRC_DIR)/error.c

# Banner
define BANNER

$(CYAN)    ╔═══════════════════════════════════════════════════╗
    ║                                                   ║
    ║       $(MAGENTA)✨ MINISHELL by Giusmery & Giulia ✨$(CYAN)        ║
    ║                                                   ║
    ║         $(YELLOW)🌙 In the name of the moon,$(CYAN)               ║
    ║        $(YELLOW)I will execute your commands! 🌙$(CYAN)           ║
    ║                                                   ║
    ╚═══════════════════════════════════════════════════╝
$(RESET)

endef
export BANNER

# Default target
all: $(NAME)

# Link executable
$(NAME): $(SRCS)
	@$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(NAME)
	@echo "$(GREEN)✓ Minishell compiled successfully!$(RESET)"
	@echo "$$BANNER"

clean:
# Clean everything
fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)✗ Minishell removed$(RESET)"

# Rebuild
re: fclean all

valgrind: all
	clear
	valgrind --leak-check=full --track-fds=yes --show-leak-kinds=all --track-origins=yes --trace-children=yes --suppressions=$(PWD)/good.supp --quiet ./$(NAME)

.PHONY: all clean fclean re

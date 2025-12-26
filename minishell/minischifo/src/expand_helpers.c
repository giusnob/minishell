/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 19:50:52 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/27 00:17:32 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Calcola lunghezza di una variabile */
int	get_var_length(char *str, int *i, t_data *data)
{
	char	*var_name;
	char	*var_value;
	int		var_len;
	int		len;

	var_name = extract_var_name(str + *i, &var_len);
	var_value = expand_single_var(var_name, data);
	len = ft_strlen(var_value);
	free(var_name);
	free(var_value);
	*i += var_len;
	return (len);
}

/* Copia valore variabile nel risultato */
void	copy_var_to_result(char *result, int *j, char *var_value)
{
	int	k;

	k = 0;
	while (var_value[k])
	{
		result[*j] = var_value[k];
		(*j)++;
		k++;
	}
}

/* Processa espansione variabile nel loop */
static void	process_expansion(t_expand_ctx *ctx)
{
	char	*var_name;
	char	*var_value;
	int		var_len;

	var_name = extract_var_name(ctx->str + *(ctx->i), &var_len);
	var_value = expand_single_var(var_name, ctx->data);
	copy_var_to_result(ctx->result, ctx->j, var_value);
	free(var_name);
	free(var_value);
	*(ctx->i) += var_len;
}

/* Inizializza contesto espansione */
static void	init_expand_ctx(t_expand_ctx *ctx, char *str, t_data *data)
{
	ctx->str = str;
	ctx->data = data;
}

/* Espande tutte le variabili in una stringa */
char	*expand_variables(char *str, t_data *data)
{
	t_expand_ctx	ctx;
	char			*result;
	int				i;
	int				j;

	if (!str)
		return (NULL);
	result = (char *)malloc(sizeof(char)
			* (calculate_expanded_len(str, data) + 1));
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	init_expand_ctx(&ctx, str, data);
	ctx.result = result;
	ctx.i = &i;
	ctx.j = &j;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
			process_expansion(&ctx);
		else
			result[j++] = str[i++];
	}
	result[j] = '\0';
	return (result);
}

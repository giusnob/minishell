/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ginobile <ginobile@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 17:02:11 by ginobile          #+#    #+#             */
/*   Updated: 2025/12/26 18:13:36 by ginobile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Funzioni helper - dichiarate prima */
int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

int	ft_isalnum(int c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9'));
}

static void	reverse_string(char *str, int len)
{
	int		i;
	int		j;
	char	tmp;

	i = 0;
	j = len - 1;
	while (i < j)
	{
		tmp = str[i];
		str[i] = str[j];
		str[j] = tmp;
		i++;
		j--;
	}
}

char	*ft_itoa(int n)
{
	char	buffer[12];
	int		i;
	int		is_neg;

	is_neg = 0;
	if (n < 0)
	{
		is_neg = 1;
		n = -n;
	}
	i = 0;
	if (n == 0)
		buffer[i++] = '0';
	while (n > 0)
	{
		buffer[i++] = (n % 10) + '0';
		n /= 10;
	}
	if (is_neg)
		buffer[i++] = '-';
	buffer[i] = '\0';
	reverse_string(buffer, i);
	return (ft_strdup(buffer));
}

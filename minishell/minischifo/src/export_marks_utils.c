/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export_marks_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gifanell <giuliafanelli111@gmail.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/05 01:50:35 by gifanell          #+#    #+#             */
/*   Updated: 2026/01/05 05:25:28 by gifanell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/* Conta quante export marks ci sono*/
static int  count_marks(char **marks)
{
    int i;

    i = 0;
    if (!marks)
        return (0);
    while (marks[i])
        i++;
    return (i);
}

/* Aggiunge una export mark*/
void    add_export_mark(char *key, t_data *data)
{
    int     i;
    int     count;
    char    **new_marks;

    count = count_marks(data->export_marks);
    new_marks = (char **)malloc(sizeof(char *) * (count + 2));
    if (!new_marks)
        return ;
    i = 0;
    while (i < count)
    {
        new_marks[i] = data->export_marks[i];
        i++;
    }
    new_marks[count] = ft_strdup(key);
    new_marks[count + 1] = NULL;
    if (data->export_marks)
        free(data->export_marks);
    data->export_marks = new_marks;
}

/* Rimuove una export mark */
void    remove_export_mark(char *key, t_data *data)
{
    int     i;
    int     j;
    int     count;
    char    **new_marks;

    if (!data->export_marks)
        return ;
    count = count_marks(data->export_marks);
    new_marks = malloc(sizeof(char *) * (count + 1));
    if (!new_marks)
        return ;
    i = 0;
    j = 0;
    while (i < count)
    {
        if (ft_strcmp(data->export_marks[i], key) != 0)
            new_marks[j++] = data->export_marks[i];
        else
            free(data->export_marks[i]);
        i++;
    }
    new_marks[j] = NULL;
    free(data->export_marks);
    data->export_marks = new_marks;
}

int is_marked_for_export(char *key, t_data *data)
{
    int i;

    if (!data->export_marks)
        return (0);
    i = 0;
    while (data->export_marks[i])
    {
        if (ft_strcmp(data->export_marks[i], key) == 0)
            return (1);
        i++;
    }
    return (0);
}
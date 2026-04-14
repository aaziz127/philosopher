/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:51:50 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Chandy-Misra initial ownership: fork[i] belongs to the lower-ID
** philosopher among its two users.
**
**   fork[0]  : shared by philo N (right) and philo 1 (left)  -> owner = 1
**   fork[i]  : shared by philo i (right) and philo i+1 (left) -> owner = i
**
** All forks start dirty so any owner will pass them on the first request.
** This produces an acyclic dependency graph at t=0: only philo 1 starts
** with both its forks, preventing the circular-wait deadlock that occurs
** when every philosopher waits for its right neighbor.
**
** BUG in original: used `i + 2` (higher-ID neighbor) so philo 1 started
** with neither fork, spun in get_forks(), and starved to death at TTD ms.
*/
static void	init_forks(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		pthread_mutex_init(&data->forks[i].mutex, NULL);
		data->forks[i].is_dirty = 1;
		if (i == 0)
			data->forks[i].owner_id = 1;
		else
			data->forks[i].owner_id = i;
	}
}

/*
** Input validation: each argument must be a strictly positive integer
** with no leading sign (subject spec) and no overflow.
*/
static int	is_valid_arg(const char *str)
{
	long	res;
	int		i;

	i = 0;
	res = 0;
	if (!str[i])
		return (0);
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = res * 10 + (str[i] - '0');
		if (res > 2147483647)
			return (0);
		i++;
	}
	return (str[i] == '\0' && res > 0);
}

static int	validate_args(int ac, char **av)
{
	int	i;

	i = 0;
	while (++i < ac)
	{
		if (!is_valid_arg(av[i]))
		{
			printf("Error: invalid argument '%s'\n", av[i]);
			return (1);
		}
	}
	return (0);
}

/*
** init_data: parse arguments, initialize shared state, and prepare
** the mutex-protected fork array used by Chandy-Misra ordering.
*/
int	init_data(t_data *data, int ac, char **av)
{
	if (validate_args(ac, av))
		return (1);
	data->num_philos = ft_atoi(av[1]);
	data->time_to_die = ft_atoi(av[2]);
	data->time_to_eat = ft_atoi(av[3]);
	data->time_to_sleep = ft_atoi(av[4]);
	data->must_eat = -1;
	if (ac == 6)
		data->must_eat = ft_atoi(av[5]);
	data->dead = 0;
	data->all_ate = 0;
	data->start_time = get_time();
	data->forks = malloc(sizeof(t_fork) * data->num_philos);
	if (!data->forks)
		return (1);
	init_forks(data);
	pthread_mutex_init(&data->print_mutex, NULL);
	pthread_mutex_init(&data->dead_mutex, NULL);
	return (0);
}

/*
** init_philos: allocate philosopher records and initialize per-philo
** mutex state for tracking last_eat and eat_count safely.
*/
int	init_philos(t_data *data)
{
	int	i;

	data->philos = malloc(sizeof(t_philo) * data->num_philos);
	if (!data->philos)
		return (1);
	i = -1;
	while (++i < data->num_philos)
	{
		data->philos[i].id = i + 1;
		data->philos[i].eat_count = 0;
		data->philos[i].last_eat = data->start_time;
		data->philos[i].data = data;
		pthread_mutex_init(&data->philos[i].philo_mutex, NULL);
	}
	return (0);
}

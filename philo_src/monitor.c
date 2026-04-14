/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:50:51 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** check_death: scan all philosophers for TTD expiry and set the shared
** dead flag in a mutex-protected way before printing the death event.
*/
static int	check_death(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		pthread_mutex_lock(&data->philos[i].philo_mutex);
		if (get_time() - data->philos[i].last_eat > data->time_to_die)
		{
			pthread_mutex_lock(&data->dead_mutex);
			data->dead = 1;
			pthread_mutex_lock(&data->print_mutex);
			printf("%lld %d died\n", get_time() - data->start_time,
				data->philos[i].id);
			pthread_mutex_unlock(&data->print_mutex);
			pthread_mutex_unlock(&data->dead_mutex);
			pthread_mutex_unlock(&data->philos[i].philo_mutex);
			return (1);
		}
		pthread_mutex_unlock(&data->philos[i].philo_mutex);
	}
	return (0);
}

/*
** check_ate: if must_eat is specified, detect when every philosopher has
** reached the required meal count and mark the simulation as complete.
*/
static int	check_ate(t_data *data)
{
	int	i;
	int	finished;

	if (data->must_eat == -1)
		return (0);
	i = -1;
	finished = 0;
	while (++i < data->num_philos)
	{
		pthread_mutex_lock(&data->philos[i].philo_mutex);
		if (data->philos[i].eat_count >= data->must_eat)
			finished++;
		pthread_mutex_unlock(&data->philos[i].philo_mutex);
	}
	if (finished == data->num_philos)
	{
		pthread_mutex_lock(&data->dead_mutex);
		data->all_ate = 1;
		pthread_mutex_unlock(&data->dead_mutex);
		return (1);
	}
	return (0);
}

void	monitor(t_data *data)
{
	while (1)
	{
		if (check_death(data) || check_ate(data))
			break ;
		usleep(1000);
	}
}

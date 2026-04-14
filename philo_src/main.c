/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:54:20 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** cleanup: destroy all mutexes and free heap-allocated philosopher
** and fork arrays when the simulation ends.
*/
static void	cleanup(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		pthread_mutex_destroy(&data->forks[i].mutex);
		pthread_mutex_destroy(&data->philos[i].philo_mutex);
	}
	pthread_mutex_destroy(&data->print_mutex);
	pthread_mutex_destroy(&data->dead_mutex);
	free(data->forks);
	free(data->philos);
}

/*
** start_simulation: spawn one pthread per philosopher and then run the
** parent monitor loop until either a death or the must_eat condition.
*/
static int	start_simulation(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		if (pthread_create(&data->philos[i].thread, NULL,
				&routine, &data->philos[i]))
			return (1);
	}
	monitor(data);
	i = -1;
	while (++i < data->num_philos)
		pthread_join(data->philos[i].thread, NULL);
	return (0);
}

/*
** main: parse arguments, initialize shared and per-philosopher state,
** then start the threaded philosopher simulation.
*/
int	main(int ac, char **av)
{
	t_data	data;

	if (ac != 5 && ac != 6)
	{
		printf("Error: Wrong number of arguments\n");
		return (1);
	}
	if (init_data(&data, ac, av))
		return (1);
	if (init_philos(&data))
	{
		free(data.forks);
		return (1);
	}
	if (start_simulation(&data))
	{
		cleanup(&data);
		return (1);
	}
	cleanup(&data);
	return (0);
}

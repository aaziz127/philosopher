/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:54:24 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/*
** cleanup: release and unlink all named semaphores.
**
** FIX: sem_close only detaches this process; the kernel object persists
** in /dev/shm until sem_unlink removes it.  Without unlink, stale
** semaphores from a previous crashed run corrupt the next run.
** The stat_mutex is process-local (not pshared), so only destroy it
** in the parent process where it was init'd (children re-init via fork).
*/
static void	cleanup(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
	{
		sem_close(data->philos[i].turn_sem);
		sem_unlink(data->philos[i].turn_name);
		pthread_mutex_destroy(&data->philos[i].stat_mutex);
	}
	sem_close(data->forks);
	sem_close(data->print_sem);
	sem_unlink("/philo_forks");
	sem_unlink("/philo_print");
	free(data->philos);
}

static void	kill_all(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->num_philos)
		kill(data->philos[i].pid, SIGKILL);
}

/*
** start_simulation:
**   Fork N children. Each child enters routine() and exits internally.
**   The parent waits for any child to finish and uses exit status to
**   decide whether a philosopher died or the meal requirement was met.
*/
static int	start_simulation(t_data *data)
{
	int	i;
	int	status;

	i = -1;
	while (++i < data->num_philos)
	{
		data->philos[i].pid = fork();
		if (data->philos[i].pid == 0)
			routine(&data->philos[i]);
		else if (data->philos[i].pid < 0)
			return (1);
	}
	i = -1;
	while (++i < data->num_philos)
	{
		waitpid(-1, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
		{
			kill_all(data);
			while (waitpid(-1, NULL, WNOHANG) > 0)
				;
			return (0);
		}
	}
	return (0);
}

/*
** main: parse arguments, initialize semaphores and philosopher metadata,
** then run the bonus token-ring simulation.
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
		free(data.philos);
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

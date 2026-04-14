/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:52:42 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

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
** init_data: parse arguments and set up semaphores used across child
** processes for fork availability and serialized printing.
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
	data->start_time = get_time();
	sem_unlink("/philo_forks");
	sem_unlink("/philo_print");
	data->forks = sem_open("/philo_forks", O_CREAT, 0644, data->num_philos);
	data->print_sem = sem_open("/philo_print", O_CREAT, 0644, 1);
	if (data->forks == SEM_FAILED || data->print_sem == SEM_FAILED)
		return (1);
	return (0);
}

/*
** Token-ring init:
**
** We circulate floor(N/2) tokens among N philosophers.
** Token assignment: philosopher i gets a token if (i-1) < floor(N/2).
** Philosophers 1 .. floor(N/2) start with token = 1 (may eat immediately).
** Philosophers floor(N/2)+1 .. N start with token = 0 (must wait).
**
** After eating, philosopher i posts to philosopher i%N + 1.
** Because floor(N/2) tokens are in circulation, at any instant exactly
** floor(N/2) philosophers are allowed to compete for forks. With N forks
** and floor(N/2) competitors each needing 2, they need 2*floor(N/2) ≤ N
** forks total, so no philosopher ever blocks on their second fork.
** Fork-wait time = 0, making max_gap = TTE + TTS + TTE = 2*TTE + TTS.
** All well-formed cases have TTD > 2*TTE + TTS, so no spurious death.
*/
static int	open_turn_sem(t_philo *philo, int has_token)
{
	char	*p;
	int		n;

	n = philo->id;
	p = philo->turn_name;
	p[0] = '/';
	p[1] = 'p';
	p[2] = 't';
	p[3] = '_';
	p[4] = '0' + (n / 100 % 10);
	p[5] = '0' + (n / 10 % 10);
	p[6] = '0' + (n % 10);
	p[7] = '\0';
	sem_unlink(p);
	philo->turn_sem = sem_open(p, O_CREAT, 0644, has_token);
	return (philo->turn_sem == SEM_FAILED);
}

/*
** init_philos: allocate philosopher processes and create one token
** semaphore per philosopher for the token-ring deadlock avoidance scheme.
*/
int	init_philos(t_data *data)
{
	int	i;
	int	slots;
	int	has_token;

	data->philos = malloc(sizeof(t_philo) * data->num_philos);
	if (!data->philos)
		return (1);
	slots = data->num_philos / 2;
	if (slots < 1)
		slots = 1;
	i = -1;
	while (++i < data->num_philos)
	{
		data->philos[i].id = i + 1;
		data->philos[i].eat_count = 0;
		data->philos[i].last_eat = data->start_time;
		data->philos[i].data = data;
		has_token = (i < slots) ? 1 : 0;
		if (open_turn_sem(&data->philos[i], has_token))
			return (1);
		pthread_mutex_init(&data->philos[i].stat_mutex, NULL);
	}
	return (0);
}

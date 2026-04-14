/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 13:22:42 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/14 13:22:44 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Chandy-Misra request: atomically take a fork from its neighbor if
** the neighbor currently owns it AND the fork is dirty (= has been
** used since it was last cleaned, meaning the owner is willing to
** yield it on demand).
*/
static void	request_fork(t_philo *philo, int fork_idx)
{
	t_fork	*fork;
	int		neighbor_id;

	fork = &philo->data->forks[fork_idx];
	if (fork_idx == philo->id - 1)
		neighbor_id = (philo->id == 1) ? philo->data->num_philos : philo->id
			- 1;
	else
		neighbor_id = (philo->id % philo->data->num_philos) + 1;
	pthread_mutex_lock(&fork->mutex);
	if (fork->owner_id == neighbor_id && fork->is_dirty)
	{
		fork->is_dirty = 0;
		fork->owner_id = philo->id;
	}
	pthread_mutex_unlock(&fork->mutex);
}

/*
** Chandy-Misra transfer: while idle (thinking/sleeping), yield any
** dirty owned fork to the neighbor who needs it.
*/
static void	give_fork_if_requested(t_philo *philo, int fork_idx)
{
	t_fork	*fork;
	int		neighbor_id;

	fork = &philo->data->forks[fork_idx];
	if (fork_idx == philo->id - 1)
		neighbor_id = (philo->id == 1) ? philo->data->num_philos : philo->id
			- 1;
	else
		neighbor_id = (philo->id % philo->data->num_philos) + 1;
	pthread_mutex_lock(&fork->mutex);
	if (fork->owner_id == philo->id && fork->is_dirty)
	{
		fork->is_dirty = 0;
		fork->owner_id = neighbor_id;
	}
	pthread_mutex_unlock(&fork->mutex);
}

/*
** FIX 1 - Lock ordering: always acquire the lower-index mutex first.
** Without this, N=2 deadlocks: philo1 holds forks[0] waits forks[1]
** while philo2 holds forks[1] waits forks[0] -> circular wait.
**
** FIX 2 - Clean-on-acquire: mark both forks dirty=0 atomically while
** both mutexes are held, before releasing. Without this, a neighbor
** can re-steal a fork in the window between the ownership check and
** the eat phase, breaking mutual exclusion and causing starvation.
*/
static int	get_forks(t_philo *philo)
{
	int	l;
	int	r;

	l = philo->id - 1;
	r = philo->id % philo->data->num_philos;
	while (1)
	{
		pthread_mutex_lock(&philo->data->dead_mutex);
		if (philo->data->dead || philo->data->all_ate)
			return (pthread_mutex_unlock(&philo->data->dead_mutex), 0);
		pthread_mutex_unlock(&philo->data->dead_mutex);
		request_fork(philo, l);
		request_fork(philo, r);
		if (l < r)
		{
			pthread_mutex_lock(&philo->data->forks[l].mutex);
			pthread_mutex_lock(&philo->data->forks[r].mutex);
		}
		else
		{
			pthread_mutex_lock(&philo->data->forks[r].mutex);
			pthread_mutex_lock(&philo->data->forks[l].mutex);
		}
		if (philo->data->forks[l].owner_id == philo->id
			&& philo->data->forks[r].owner_id == philo->id)
		{
			philo->data->forks[l].is_dirty = 0;
			philo->data->forks[r].is_dirty = 0;
			pthread_mutex_unlock(&philo->data->forks[l].mutex);
			pthread_mutex_unlock(&philo->data->forks[r].mutex);
			return (1);
		}
		pthread_mutex_unlock(&philo->data->forks[l].mutex);
		pthread_mutex_unlock(&philo->data->forks[r].mutex);
		give_fork_if_requested(philo, l);
		give_fork_if_requested(philo, r);
		usleep(100);
	}
}

/*
** FIX 3 - "has taken a fork" printed twice (one per fork acquired),
** as required by the subject. The original printed it zero times.
** After eating, mark both forks dirty so Chandy-Misra can pass them.
*/
static void	eat(t_philo *philo)
{
	int	l;
	int	r;

	l = philo->id - 1;
	r = philo->id % philo->data->num_philos;
	if (!get_forks(philo))
		return ;
	print_status(philo, "has taken a fork");
	print_status(philo, "has taken a fork");
	print_status(philo, "is eating");
	pthread_mutex_lock(&philo->philo_mutex);
	philo->last_eat = get_time();
	philo->eat_count++;
	pthread_mutex_unlock(&philo->philo_mutex);
	ft_usleep(philo->data->time_to_eat);
	pthread_mutex_lock(&philo->data->forks[l].mutex);
	philo->data->forks[l].is_dirty = 1;
	pthread_mutex_unlock(&philo->data->forks[l].mutex);
	pthread_mutex_lock(&philo->data->forks[r].mutex);
	philo->data->forks[r].is_dirty = 1;
	pthread_mutex_unlock(&philo->data->forks[r].mutex);
}

/*
** FIX 4 - n=1 special case: l == r == 0, so get_forks would lock the
** same mutex twice causing a silent deadlock. Correct behaviour: pick
** up the single available fork, display it, then wait for the monitor
** to declare death (impossible to eat with only one fork).
*/
static void	single_philo(t_philo *philo)
{
	print_status(philo, "has taken a fork");
	while (1)
	{
		pthread_mutex_lock(&philo->data->dead_mutex);
		if (philo->data->dead)
			return (pthread_mutex_unlock(&philo->data->dead_mutex), (void)0);
		pthread_mutex_unlock(&philo->data->dead_mutex);
		usleep(100);
	}
}

/*
** Even-ID philosophers stagger by time_to_eat ms before their first
** action. Philo 1 already owns both forks at t=0 and eats immediately;
** without this offset, even philos spin in get_forks() adding CPU noise
** that nudges timing past TTD on tight cases.
*/
void	*routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	if (philo->data->num_philos == 1)
		return (single_philo(philo), NULL);
	if (philo->id % 2 == 0)
		ft_usleep(philo->data->time_to_eat);
	while (1)
	{
		pthread_mutex_lock(&philo->data->dead_mutex);
		if (philo->data->dead || philo->data->all_ate)
			return (pthread_mutex_unlock(&philo->data->dead_mutex), NULL);
		pthread_mutex_unlock(&philo->data->dead_mutex);
		eat(philo);
		print_status(philo, "is sleeping");
		ft_usleep(philo->data->time_to_sleep);
		print_status(philo, "is thinking");
		give_fork_if_requested(philo, philo->id - 1);
		give_fork_if_requested(philo, philo->id % philo->data->num_philos);
	}
	return (NULL);
}

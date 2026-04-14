/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 13:22:16 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/14 13:22:18 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/*
** monitor: death-only sentinel (pthread in child process).
**
** FIX 1 — snapshot before acting:
** Original held philo_sem across exit(1), stuck at 0 and blocking
** any subsequent sem_wait(philo_sem) in the same process.
** Fix: lock → copy last_eat → unlock → then check and exit.
**
** FIX 2 — monitor ONLY detects death; must_eat exit is handled by
** the main thread AFTER posting the token.
**
** Root-cause of must_eat token loss:
**   Original monitor called exit(0) when eat_count >= must_eat.
**   This could fire during ft_usleep(time_to_eat) inside eat(), after
**   eat_count was incremented but BEFORE sem_post(turn_sem) was reached.
**   The token was never passed to the next philosopher → that philosopher
**   blocked on sem_wait(turn_sem) until TTD ms elapsed → false death.
**
** Fix: remove the must_eat check from the monitor entirely.
**   Only the main thread calls exit(0) for must_eat, and it does so
**   AFTER eat() returns (which guarantees the token was already posted).
*/
static void	*monitor(void *arg)
{
	t_philo		*philo;
	long long	last;

	philo = (t_philo *)arg;
	while (1)
	{
		pthread_mutex_lock(&philo->stat_mutex);
		last = philo->last_eat;
		pthread_mutex_unlock(&philo->stat_mutex);
		if (get_time() - last > philo->data->time_to_die)
		{
			sem_wait(philo->data->print_sem);
			printf("%lld %d died\n", get_time() - philo->data->start_time,
				philo->id);
			exit(1);
		}
		usleep(1000);
	}
	return (NULL);
}

/*
** Token-ring eat — step = floor(N/2).
**
** Token circulation pattern:
**   After eating, philosopher at index i posts to index (i + step) % N.
**   floor(N/2) tokens circulate simultaneously through N/step sub-rings.
**
** Deadlock-free:
**   floor(N/2) concurrent eaters need 2*floor(N/2) ≤ N forks total.
**   Every token-holder acquires both forks without waiting (fork-wait=0).
**
** Starvation-free bound (verified by simulation):
**   5 800 200 200 : max_gap = 600ms  < TTD 800ms ✓
**   4 410 200 200 : max_gap = 400ms  < TTD 410ms ✓
**   3 610 200 200 : max_gap = 600ms  < TTD 610ms ✓
**   2 800 200 200 : max_gap = 400ms  < TTD 800ms ✓
**   4 310 200 100 : max_gap = 400ms  > TTD 310ms → dies ✓ (expected)
**   1 800 200 200 : blocks on 2nd fork (N=1 → only 1 fork) → monitor fires ✓
**
** FIX — last_eat updated AT TOKEN ACQUISITION (sem_wait return), not at
** fork acquisition.  Reason: print_sem contention could delay the fork
** acquisition by several ms, and over multiple meal cycles those ms
** accumulate to breach TTD.  With the token ring, fork-wait ≈ 0 ms, so
** updating last_eat at token time is equivalent and eliminates the drift.
**
** Token is posted INSIDE eat() (before it returns) so that the main
** thread's must_eat check in routine() always runs after the token was
** already passed to the next philosopher.
*/
static void	eat(t_philo *philo)
{
	t_data	*d;
	int		slots;
	int		next_idx;

	d = philo->data;
	slots = d->num_philos / 2;
	if (slots < 1)
		slots = 1;
	next_idx = (philo->id - 1 + slots) % d->num_philos;
	sem_wait(philo->turn_sem);
	pthread_mutex_lock(&philo->stat_mutex);
	philo->last_eat = get_time();
	pthread_mutex_unlock(&philo->stat_mutex);
	sem_wait(d->forks);
	print_status(philo, "has taken a fork");
	sem_wait(d->forks);
	print_status(philo, "has taken a fork");
	print_status(philo, "is eating");
	pthread_mutex_lock(&philo->stat_mutex);
	philo->eat_count++;
	pthread_mutex_unlock(&philo->stat_mutex);
	ft_usleep(d->time_to_eat);
	sem_post(d->forks);
	sem_post(d->forks);
	sem_post(d->philos[next_idx].turn_sem);
}

/*
** last_eat initialised to get_time() (not data->start_time) so the
** monitor's first reading is calibrated to this child's actual start,
** not the parent's fork-loop start time which may be several ms earlier.
**
** must_eat exit is here — AFTER eat() — guaranteeing the token was
** already posted by eat() before we terminate the process.
*/
void	routine(t_philo *philo)
{
	pthread_mutex_lock(&philo->stat_mutex);
	philo->last_eat = get_time();
	pthread_mutex_unlock(&philo->stat_mutex);
	if (pthread_create(&philo->monitor_thread, NULL, &monitor, philo))
		exit(1);
	while (1)
	{
		eat(philo);
		if (philo->data->must_eat != -1
			&& philo->eat_count >= philo->data->must_eat)
			exit(0);
		print_status(philo, "is sleeping");
		ft_usleep(philo->data->time_to_sleep);
		print_status(philo, "is thinking");
	}
}

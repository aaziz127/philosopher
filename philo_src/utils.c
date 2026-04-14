/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:51:41 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** get_time: return the current wall-clock time in milliseconds.
*/
long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/*
** ft_usleep: sleep for approximately the given number of milliseconds.
** Uses short usleep intervals to avoid oversleeping on slower timers.
*/
void	ft_usleep(long long time)
{
	long long	start;

	start = get_time();
	while ((get_time() - start) < time)
		usleep(500);
}

/*
** FIX: print_status now suppresses output when all_ate is set, not
** only when dead. Without this, stray messages appear after the
** must_eat condition is met (visible in the evaluator's terminal).
*/
void	print_status(t_philo *philo, char *status)
{
	pthread_mutex_lock(&philo->data->dead_mutex);
	if (!philo->data->dead && !philo->data->all_ate)
	{
		pthread_mutex_lock(&philo->data->print_mutex);
		printf("%lld %d %s\n", get_time() - philo->data->start_time,
			philo->id, status);
		pthread_mutex_unlock(&philo->data->print_mutex);
	}
	pthread_mutex_unlock(&philo->data->dead_mutex);
}

/*
** ft_atoi used only after validate_args passes, so input is clean.
** Handles optional leading '+' and whitespace for robustness.
*/
int	ft_atoi(const char *str)
{
	long	res;
	int		i;

	i = 0;
	res = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return ((int)res);
}

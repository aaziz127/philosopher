/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:53:46 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

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
** ft_usleep: sleep for approximately the specified number of milliseconds.
** It polls frequently to reduce oversleeping caused by OS scheduler delays.
*/
void	ft_usleep(long long time)
{
	long long	start;

	start = get_time();
	while ((get_time() - start) < time)
		usleep(500);
}

/*
** ft_atoi: convert a clean numeric string to an int.
** Input validation is handled earlier in init_bonus.c.
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

/*
** print_status: serialize status output from child philosophers.
*/
void	print_status(t_philo *philo, char *status)
{
	sem_wait(philo->data->print_sem);
	printf("%lld %d %s\n", get_time() - philo->data->start_time,
		philo->id, status);
	sem_post(philo->data->print_sem);
}

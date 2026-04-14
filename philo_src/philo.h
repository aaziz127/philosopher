/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:54:10 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>

typedef struct s_data	t_data;
/*
** t_fork: a fork is represented as a mutex plus ownership metadata for
** Chandy-Misra resource transfer. `owner_id` tracks the current philosopher
** holding the fork; `is_dirty` signals whether the fork can be passed on.
*/
typedef struct s_fork
{
	int				owner_id;
	int				is_dirty;
	pthread_mutex_t	mutex;
}	t_fork;

typedef struct s_philo
{
	int				id;
	int				eat_count;
	long long		last_eat;
	pthread_t		thread;
	t_data			*data;
	pthread_mutex_t	philo_mutex;
}	t_philo;

/*
** s_data: shared simulation state for the mandatory philosopher model.
** `num_philos` and timing fields are static after init, while `dead`
** and `all_ate` are published across threads using mutex protection.
*/

struct s_data
{
	int				num_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				must_eat;
	int				dead;
	int				all_ate;
	long long		start_time;
	t_fork			*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	dead_mutex;
	t_philo			*philos;
};

/* utils.c */
long long	get_time(void);
void		ft_usleep(long long time);
int			ft_atoi(const char *str);
void		print_status(t_philo *philo, char *status);

/* init.c */
int			init_data(t_data *data, int ac, char **av);
int			init_philos(t_data *data);

/* routine.c */
void		*routine(void *arg);

/* monitor.c */
void		monitor(t_data *data);

#endif

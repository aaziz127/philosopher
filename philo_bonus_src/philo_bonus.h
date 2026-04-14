/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/31 12:00:00 by manus             #+#    #+#             */
/*   Updated: 2026/04/09 03:52:11 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <semaphore.h>
# include <fcntl.h>
# include <signal.h>

typedef struct s_data	t_data;

/*
** Token-ring design:
**
** turn_sem  : named binary semaphore "/pt_NNN" (one per philosopher).
**             Initialized to 1 for the first floor(N/2) philosophers
**             (they hold a token and may eat immediately), 0 for the
**             rest (they wait until a token is passed to them).
**             After eating, philosopher i does sem_post on philosopher
**             (i % N)'s turn_sem, passing the token to the next one.
**             This bounds fork-wait to 0 and starvation to 2*TTE+TTS.
**
** stat_mutex: process-local pthread mutex (not PTHREAD_PROCESS_SHARED).
**             Protects last_eat and eat_count between the eat() thread
**             and the monitor thread running in the same process.
**             Safe to use without shared-memory setup because monitor
**             and routine run as pthreads inside the same fork()ed child.
*/
typedef struct s_philo
{
	int				id;
	int				eat_count;
	long long		last_eat;
	pid_t			pid;
	t_data			*data;
	pthread_t		monitor_thread;
	sem_t			*turn_sem;
	char			turn_name[32];
	pthread_mutex_t	stat_mutex;
}	t_philo;

/*
** s_data: shared simulation state for the bonus philosopher model.
** `forks` counts available forks via a semaphore and `print_sem`
** serializes output across child processes.
*/

struct s_data
{
	int				num_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				must_eat;
	long long		start_time;
	sem_t			*forks;
	sem_t			*print_sem;
	t_philo			*philos;
};

/* utils_bonus.c */
long long	get_time(void);
void		ft_usleep(long long time);
int			ft_atoi(const char *str);
void		print_status(t_philo *philo, char *status);

/* init_bonus.c */
int			init_data(t_data *data, int ac, char **av);
int			init_philos(t_data *data);

/* routine_bonus.c */
void		routine(t_philo *philo);

#endif

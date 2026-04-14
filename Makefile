# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/04/14 13:22:55 by alaziz            #+#    #+#              #
#    Updated: 2026/04/14 13:31:07 by alaziz           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ─── Compiler ─────────────────────────────────────────────────────────────── #
CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

# ─── Mandatory (philo) ────────────────────────────────────────────────────── #
NAME		= philo
SRCS		= philo_src/main.c \
			  philo_src/init.c \
			  philo_src/routine.c \
			  philo_src/monitor.c \
			  philo_src/utils.c
OBJS		= $(SRCS:.c=.o)

# ─── Bonus (philo_bonus) ──────────────────────────────────────────────────── #
NAME_BONUS	= philo_bonus
SRCS_BONUS	= philo_bonus_src/main_bonus.c \
			  philo_bonus_src/init_bonus.c \
			  philo_bonus_src/routine_bonus.c \
			  philo_bonus_src/utils_bonus.c
OBJS_BONUS	= $(SRCS_BONUS:.c=.o)

# ─── Rules ────────────────────────────────────────────────────────────────── #
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

bonus: $(NAME_BONUS)

$(NAME_BONUS): $(OBJS_BONUS)
	$(CC) $(CFLAGS) $(OBJS_BONUS) -o $(NAME_BONUS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS_BONUS)

fclean: clean
	rm -f $(NAME) $(NAME_BONUS)

re: fclean all

.PHONY: all bonus clean fclean re

# philosophers_final

This repository implements the `philo` the 42-project for the mandatory and bonus philosopher problems.

## Build

- `make` builds the mandatory executable: `philo`
- `make bonus` builds the bonus executable: `philo_bonus`
- `make clean` removes object files
- `make fclean` removes executables as well

## Usage

Mandatory:

```sh
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

Bonus:

```sh
./philo_bonus number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

## Project design

- `philo` uses pthreads and a Chandy-Misra fork ownership strategy.
- `philo_bonus` uses one process per philosopher with POSIX named semaphores.
- Both implementations print philosopher states using timestamped messages.
- The optional `must_eat` argument stops the simulation when all philosophers have eaten enough.

## File structure

- `philo_src/`: mandatory implementation
- `philo_bonus_src/`: bonus implementation
- `Makefile`: build rules for both executables

## Notes

- Argument parsing validates strictly positive integers and rejects invalid input.
- The bonus version uses a token-ring scheme to avoid deadlock and to bound fork waiting.
- The mandatory version initializes fork ownership to avoid circular wait at startup.

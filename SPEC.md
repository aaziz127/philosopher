# philosophers SPEC

## Program arguments

- `number_of_philosophers` : number of philosophers and forks
- `time_to_die` : max milliseconds a philosopher may go without eating
- `time_to_eat` : milliseconds spent in the eating state
- `time_to_sleep` : milliseconds spent in the sleeping state
- `number_of_times_each_philosopher_must_eat` (optional) : if present,
  the simulation ends successfully when every philosopher has eaten at
  least this many times.

## Behavior requirements

- Philosophers must alternate between thinking, taking forks, eating,
  and sleeping.
- A philosopher dies if it does not start eating within `time_to_die`
  milliseconds after its last meal.
- The program must print status messages in the format:
  `timestamp philosopher_id action`
- If `must_eat` is specified, the simulation must stop when all
  philosophers reach that number of meals.
- Invalid arguments should print an error and exit with status `1`.

## Mandatory implementation (`philo`)

- Uses pthreads and shared memory structures in `philo_src/`.
- Implements a Chandy-Misra fork ownership protocol with `owner_id`
  and `is_dirty` flags.
- Fork ownership is initialized to avoid a circular wait at startup.
- Shared state flags `dead` and `all_ate` are protected by mutexes.
- The monitor thread polls each philosopher for death and sets the
  termination condition.

## Bonus implementation (`philo_bonus`)

- Uses `fork()` to create one process per philosopher.
- Uses POSIX named semaphores for forks, printing, and token passing.
- Uses a token-ring scheme with `floor(N/2)` tokens to prevent
  deadlock and bound concurrent fork acquisition.
- A child process monitors its own philosopher for death and exits
  with status `1` on death.
- The parent process waits for children and terminates remaining
  philosophers if any child exits with status `1`.

## Edge cases and fixes

- `N == 1` is handled specially in the mandatory implementation to
  avoid locking the same mutex twice.
- The bonus implementation ensures tokens are passed before the
  `must_eat` exit condition is evaluated.
- The bonus cleanup unlinks named semaphores to avoid stale objects
  across successive runs.

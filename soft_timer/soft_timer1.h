/*
 * soft_timer1.h
 */
#define SPIN_LOCK(lock)
#define SPIN_UNLOCK(lock)

#define MAX_TIMER      (128)
#define END_TIMER      (127)
#define NONE_TIMER     (-128)

typedef int (*timer_callback)(void *userdata);

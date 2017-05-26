/*
 *
 * soft_timer2 version 1st
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

typedef int (*tm_fun_cb)(void *userdata);

struct soft_timer {
	int remain;
	tm_fun_cb tm_func;
	/* struct soft_timer *prev; */
	struct soft_timer *next;
};

static unsigned long timer_1ms;
static struct soft_timer *timer_head;

static inline void timer_updated(void)
{
	/*clock_t t = clock();

	timer_1ms = (double)t / CLOCKS_PER_SEC * 1000; */
	timer_1ms = time(0);
}

static inline unsigned long timer_current(void)
{
	/*clock_t t = clock();

	return (double)t / CLOCKS_PER_SEC * 1000; */
	return time(0);
}

static long timer_is_expired(unsigned long remain)
{
	unsigned long diff;

	diff = (unsigned long)-1 - (remain + timer_1ms) + timer_current();
	return diff < (unsigned long)(~(1l << (sizeof(unsigned long) * 8 - 1))) ?
		diff : (long)diff + 1;
}

static void timer_expired(void)
{
	int diff;
	struct soft_timer *pt_next;

	while (timer_head) {
		diff = timer_is_expired(timer_head->remain);
		if (diff < 0) {
			if ((timer_head->remain + diff) > 0) {
				timer_head->tm_func(&timer_head->remain);
				timer_head->remain = 0 - diff;
				timer_updated();
			}
			break;
		}
		pt_next = timer_head->next;
		timer_head->tm_func(&timer_head->remain);
		free(timer_head);
		timer_head = pt_next;
		timer_updated();
	}
}

void init_timer(void)
{
	timer_updated();
	timer_head = NULL;
}

int set_timer(int ms, tm_fun_cb cb_func, void *userdata)
{
	unsigned long sum;
	struct soft_timer *pt_tmp, *pt_cur, *pt_prev;

	timer_expired();
	
	pt_tmp = malloc(sizeof(struct soft_timer));
	if (pt_tmp == NULL)
		assert(0);

	pt_tmp->tm_func = *cb_func;
	if (timer_head == NULL) {
		pt_tmp->remain = ms;
		pt_tmp->next = NULL;
		timer_head = pt_tmp;
		return 0;
	}

	sum = 0;
	pt_prev = NULL;
	pt_cur = timer_head;
	while (pt_cur) {
		if ((sum + pt_cur->remain) >= ms) {
			break;
		}
		sum += pt_cur->remain;
		pt_prev = pt_cur;
		pt_cur = pt_cur->next;
	}

	pt_tmp->remain = ms - sum;
	pt_tmp->next = pt_cur;
	if (pt_cur) {
		pt_cur->remain -= pt_tmp->remain;
		if (pt_prev == NULL)
			timer_head = pt_tmp;
	}

	if (pt_prev) {
		pt_prev->next = pt_tmp;
	}

	return 0;
}


/*
 *
 * Test Case
 *
 */
#include <unistd.h>

int hello_cb(void *userdata)
{
	printf("hello:%d\n", *(int *)userdata);
	return 0;
}

int main()
{
	int t, i;
	int count  = 0;

	srand((int)time(0));
	init_timer();
	while (1) {
		for (i = 0; i < 3; i++) {
			t = rand() % 20 + 1;
			printf("set timer %d\n", t);
			set_timer(t, hello_cb, &t);
		}
		sleep(1);
		timer_expired();
		count++;
		if (count % 5 == 0)
			sleep(1);
		if (count == 50)
			break;
	}
	return 0;
}

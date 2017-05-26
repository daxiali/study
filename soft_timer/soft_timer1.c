/*
 * soft_timer1.c 1st version
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "soft_timer1.h"

/* 1ms timer count */
static unsigned long timer_1ms_count;
static char timer_current;
static char timer_list[MAX_TIMER];
static unsigned long timer_count[MAX_TIMER];

static void timer_interrupt()
{
	SPIN_LOCK(0);
	timer_1ms_count += 1;
	SPIN_UNLOCK(0);
}

static short timer_is_expired(unsigned short expired_time)
{
    unsigned short difference;

	difference = 0xffff - expired_time + timer_1ms_count;
	return difference < 0x7fff ? difference : (short)difference + 1;
}

void timer_delay(unsigned long ms)
{
	clock_t end;

	end = clock() + (float)ms / 1000 * CLOCKS_PER_SEC;
	while (clock() < end);
}

void init_timer(void)
{
	unsigned char i;

    timer_1ms_count = 0;
	timer_current = END_TIMER;

	for (i = 0; i < MAX_TIMER; i++) {
		timer_list[i] = NONE_TIMER;
		timer_count[i] = 0;
	}
}

int set_timer(int timer_id, unsigned short ms)
{
    char i, prev;
	short rest;

	if (timer_id < 0 || timer_id > END_TIMER)
		assert(0);

	if (timer_list[timer_id] != NONE_TIMER) {
		if (timer_current == timer_id) {
			if (timer_list[timer_current] == END_TIMER)	
				timer_current = END_TIMER;
			else
				timer_current += timer_list[timer_current]; 
		} else {
			i = timer_current;
			prev = END_TIMER;
			while (i != timer_id) {
				prev = i;
				i += timer_list[i];
			}

			if (prev != END_TIMER) {
				if (timer_list[i] == END_TIMER)
					timer_list[prev] = END_TIMER;
				else
					timer_list[prev] = i + timer_list[i] - prev;
			}
		}
		timer_list[timer_id] = NONE_TIMER;
	}

	timer_count[timer_id] = timer_1ms_count + ms;
	rest = timer_is_expired(timer_count[timer_id]);
	if (rest >= 0)
		return rest;
	
	if (timer_current == END_TIMER) {
		timer_current = timer_id;
		timer_list[timer_current] = END_TIMER;
		return rest;
	}

	i = timer_current;
	prev = END_TIMER;
	while (1) {
		if (timer_is_expired(timer_count[i]) < rest) {
			if (prev == END_TIMER) {
				timer_current = timer_id;
				timer_list[timer_current] = i - timer_current;
			} else {
				timer_list[prev] = timer_id - prev;
				timer_list[timer_id] = i - timer_id;
			}
			break;
		} else if (timer_list[i] == END_TIMER) {
			timer_list[i] = timer_id - i;
			timer_list[timer_id] = END_TIMER;
			break;
		}
		prev = i;
		i += timer_list[i];
	}
    return rest;
}

void process_timer(timer_callback timer_cb, void *userdata)
{
	char prev;
	int ret;

	while (timer_current != END_TIMER) {
		if (timer_is_expired(timer_count[timer_current]) >= 0) {
			ret = timer_cb(userdata);
			if (ret < 0)
				assert(0);
			prev = timer_list[timer_current];
			if (prev == END_TIMER) {
				timer_current = END_TIMER;
			} else {
				timer_list[timer_current] = NONE_TIMER;
				timer_current += prev;
			}
		}
	}
}

/*
 * UseCase Test
 */
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

pthread_t th1, th2, th3;

void *int_timer()
{
	printf("create int timer thread.\n");
	while (1) {
		printf("1");
		timer_delay(1);
		timer_interrupt();
	}
}

void *test_timer()
{
	int timer_id, tt;

	printf("create test timer thread.\n");
	srand((int)time(0));
	while (1) {
		printf("2");
		timer_id = rand() % END_TIMER;
		tt = rand() % 100;
		printf("set timer: %d %d\n", timer_id, tt);
		set_timer(timer_id, tt);
		timer_delay(10);
	}
}

int test_cb(void *data)
{
	printf("invoke timer_id %d\n", *(int *)data);
	return 1;
}

void *do_timer()
{
	printf("create do timer thread.\n");
	while (1) {
		printf("3");
		process_timer(test_cb, &timer_current);
		timer_delay(20);
	}
}


/* test program */
int main()
{
	int ret;

	init_timer();
	ret = pthread_create(&th1, NULL, int_timer, NULL);
	if (ret != 0)
		printf("thread 1 create fail!\n");

	ret = pthread_create(&th2, NULL, test_timer, NULL);
	if (ret != 0)
		printf("thread 2 create fail!\n");

	ret = pthread_create(&th3, NULL, do_timer, NULL);
	if (ret != 0)
		printf("thread 3 create fail!\n");

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);

	return 0;
}

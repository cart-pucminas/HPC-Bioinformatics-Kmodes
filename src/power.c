/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * power.c - Power utility library for Intel Xeon Phi
 */
#define _BSD_SOURCE
#define _XOPEN_SOURCE
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>

/*
 * /sys/class/micras/power
 */
static const char *micras_power_file = "micsmc -f | grep 'Total Power: ............. ' | awk '{print $4}'";
static FILE *micras_power;


static const long time_stamp = 50000000L;
static pthread_t tid;
static float avg = 0;
static unsigned count = 0;
static unsigned live;

/*
 * Gets power information.
 */
static float power_get(void)
{
	int error;

	micras_power = popen(micras_power_file, "r");
	assert(micras_power != NULL);

	float avarage;

	error = fscanf(micras_power, "%f", &avarage);

	((void)error);

	fclose(micras_power);

	return avarage;
}

/*
 * Listen to micras power file.
 */
static void *power_listen(void *unused)
{

	struct timespec ts;

	((void)unused);

	ts.tv_sec = 0;
	ts.tv_nsec = time_stamp;

	do
	{
		avg += power_get();
		count++;

		nanosleep(&ts, NULL);
	} while (live);

	avg /= count;
	return (NULL);
}

/*
 * Initializes power measurement utility.
 */
void power_init(void)
{
	live = 1;
	pthread_create(&tid, NULL, power_listen, NULL);
}

/*
 * Terminates power measurement utility.
 */
float power_end(void)
{
	live = 0;
	pthread_join(tid, NULL);
	printf ("Power consumption = %f\n", avg);
	return (avg);
}

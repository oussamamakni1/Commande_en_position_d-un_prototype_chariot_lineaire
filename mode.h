#ifndef _MODE_H
#define _MODE_H

#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include "eqep.h"
#include "gpio.h"
#include "pwm.h"

enum{
	STOP,
    START,
    RUN,
	IDLE
};



struct status
{
	float des_pos;
	float des_vel;
	float act_pos;
	float act_vel;
	unsigned char mode;
	pthread_mutex_t access;
};

void state_machine(struct status *);
void mode_start(struct status *);
void mode_run(struct status *);
void mode_stop(struct status *);
void mode_idle(struct status *);
void * ihm(void * param);

#endif
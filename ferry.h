#ifndef FERRY_H
#define FERRY_H

#include <semaphore.h>
#include <pthread.h>

#define TOTAL_CARS 12
#define TOTAL_MINIBUSES 10
#define TOTAL_TRUCKS 8
#define TOTAL_VEHICLES (TOTAL_CARS + TOTAL_MINIBUSES + TOTAL_TRUCKS)

#define MAX_CAPACITY 20

extern sem_t toll_sem[4];
extern pthread_mutex_t boarding_mutex;
extern int current_capacity;

void* ferry_func(void* arg);

#endif

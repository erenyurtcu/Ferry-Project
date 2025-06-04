#ifndef FERRY_H
#define FERRY_H

#define TOTAL_CARS 12
#define TOTAL_MINIBUSES 10
#define TOTAL_TRUCKS 8
#define TOTAL_VEHICLES (TOTAL_CARS + TOTAL_MINIBUSES + TOTAL_TRUCKS)
#define MAX_CAPACITY 20

void* ferry_func(void* arg);

extern int is_first_return;

#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "vehicle.h"
#include "ferry.h"
#include "visual.h"

#define TOTAL_CARS 12
#define TOTAL_MINIBUSES 10
#define TOTAL_TRUCKS 8
#define TOTAL_VEHICLES (TOTAL_CARS + TOTAL_MINIBUSES + TOTAL_TRUCKS)

// GLOBAL DEĞİŞKENLER
sem_t toll_sem[4];
pthread_mutex_t boarding_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t return_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

int current_capacity = 0;
int total_returned = 0;

FILE* log_file;

int boarded_ids[30];
int boarded_count = 0;

int car_count = 0;
int minibus_count = 0;
int truck_count = 0;

Vehicle vehicles[TOTAL_VEHICLES];

int main() {
    srand(time(NULL));
    log_file = fopen("output.txt", "w");
    if (!log_file) {
        perror("Log file could not be opened");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; ++i)
        sem_init(&toll_sem[i], 0, 1);

    pthread_t ferry_thread;
    pthread_create(&ferry_thread, NULL, ferry_func, NULL);

    pthread_t printer_thread;
    pthread_create(&printer_thread, NULL, print_loop, NULL);

    pthread_t vehicle_threads[TOTAL_VEHICLES];
    int id = 0;

    for (int i = 0; i < TOTAL_CARS; ++i, ++id)
        create_vehicle(&vehicles[id], id, CAR, &vehicle_threads[id]);

    for (int i = 0; i < TOTAL_MINIBUSES; ++i, ++id)
        create_vehicle(&vehicles[id], id, MINIBUS, &vehicle_threads[id]);

    for (int i = 0; i < TOTAL_TRUCKS; ++i, ++id)
        create_vehicle(&vehicles[id], id, TRUCK, &vehicle_threads[id]);

    for (int i = 0; i < TOTAL_VEHICLES; ++i)
        pthread_join(vehicle_threads[i], NULL);

    pthread_join(ferry_thread, NULL);

    for (int i = 0; i < 4; ++i)
        sem_destroy(&toll_sem[i]);

    fclose(log_file);
    return 0;
}

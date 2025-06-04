#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "ferry.h"
#include "vehicle.h"

sem_t toll_sem[4];
pthread_mutex_t boarding_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t return_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE* log_file;

int current_capacity = 0;
int total_returned = 0;
int car_count = 0;
int minibus_count = 0;
int truck_count = 0;
int boarded_ids[30];
int boarded_count = 0;
int ferry_trip_number = 1;

int direction = 0; // 0: SIDE-A, 1: SIDE-B
int is_first_return = 1;

Vehicle vehicles[TOTAL_VEHICLES];

extern void* print_loop(void*);

int main() {
    srand(time(NULL));
    log_file = fopen("trip_log.txt", "w");

    printf("Feribot başlangıç yönü: SIDE-A → SIDE-B\n");

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

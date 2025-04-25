#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "vehicle.h"
#include "ferry.h"

extern int total_returned;
extern pthread_mutex_t return_mutex;

extern sem_t toll_sem[4];
extern pthread_mutex_t boarding_mutex;
extern int current_capacity;

// extern globaller
extern int car_count;
extern int minibus_count;
extern int truck_count;

extern int boarded_ids[30];
extern int boarded_count;

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread) {
    v->id = id;
    v->type = type;
    v->port = rand() % 2;
    v->toll = v->port * 2 + (rand() % 2); // 0-1 or 2-3
    v->returned = 0;

    pthread_create(thread, NULL, vehicle_func, v);
}

void* vehicle_func(void* arg) {
    Vehicle* v = (Vehicle*)arg;

    const char* type_str = (v->type == CAR) ? "Car" :
                           (v->type == MINIBUS) ? "Minibus" : "Truck";

    printf("Vehicle %d (%s) started at port %d, toll %d\n",
           v->id, type_str, v->port, v->toll);

    sleep(1 + rand() % 2);  // Toll'e yürüyüş süresi

    // 1. Toll geçişi (critical section 1)
    sem_wait(&toll_sem[v->toll]);
    printf("Vehicle %d (%s) passed toll %d\n", v->id, type_str, v->toll);
    sleep(1); // Gişeden geçme süresi
    sem_post(&toll_sem[v->toll]);

    // 2. Ferry boarding (critical section 2)
    int boarded = 0;
    while (!boarded) {
        pthread_mutex_lock(&boarding_mutex);
        if (current_capacity + v->type <= MAX_CAPACITY) {
            current_capacity += v->type;
            boarded_ids[boarded_count++] = v->id;
            boarded = 1;
            printf("Vehicle %d (%s) boarded the ferry (uses %d units, current: %d)\n",
                   v->id, type_str, v->type, current_capacity);
        }
        pthread_mutex_unlock(&boarding_mutex);
        if (!boarded) usleep(100000);
    }

    // Dönüş
    printf("Vehicle %d (%s) returning...\n", v->id, type_str);
    sleep(3 + rand() % 3); // 3-5 saniye arası

    // Sayaçlar
    pthread_mutex_lock(&return_mutex);
    total_returned++;
    if (v->type == CAR) car_count++;
    else if (v->type == MINIBUS) minibus_count++;
    else if (v->type == TRUCK) truck_count++;
    pthread_mutex_unlock(&return_mutex);

    printf("Vehicle %d (%s) has returned to original port\n", v->id, type_str);

    return NULL;
}

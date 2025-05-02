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

extern int car_count;
extern int minibus_count;
extern int truck_count;

extern int boarded_ids[30];
extern int boarded_count;

extern Vehicle vehicles[]; // 👈 Konumları izleyebilmek için

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread) {
    v->id = id;
    v->type = type;
    v->port = rand() % 2;
    v->toll = v->port * 2 + (rand() % 2); // 0-1 or 2-3
    v->returned = 0;
    v->location = 0; // Başlangıçta Side A

    pthread_create(thread, NULL, vehicle_func, v);
}

void* vehicle_func(void* arg) {
    Vehicle* v = (Vehicle*)arg;

    sleep(1 + rand() % 2);

    sem_wait(&toll_sem[v->toll]);
    sleep(1);
    sem_post(&toll_sem[v->toll]);

    int boarded = 0;
    while (!boarded) {
        pthread_mutex_lock(&boarding_mutex);
        if (current_capacity + v->type <= MAX_CAPACITY) {
            current_capacity += v->type;
            boarded_ids[boarded_count++] = v->id;
            v->location = 1; // Ferry
            boarded = 1;
        }
        pthread_mutex_unlock(&boarding_mutex);
        if (!boarded) usleep(100000);
    }

    // B'ye varınca
    sleep(3 + rand() % 3); // Feribotta yolculuk

    // B tarafına in
    v->location = 2;  // Side B
    sleep(2 + rand() % 2); // B'de kısa bekleme

    // Geri dönüş
    v->location = 0; // Geri A’ya döner
    sleep(1); // A’ya dönme süresi

    pthread_mutex_lock(&return_mutex);
    total_returned++;
    if (v->type == CAR) car_count++;
    else if (v->type == MINIBUS) minibus_count++;
    else if (v->type == TRUCK) truck_count++;
    pthread_mutex_unlock(&return_mutex);

    return NULL;
}

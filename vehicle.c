#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "vehicle.h"
#include "ferry.h"

extern Vehicle vehicles[];
extern int current_capacity;
extern pthread_mutex_t boarding_mutex;
extern int total_returned;
extern pthread_mutex_t return_mutex;
extern int car_count;
extern int minibus_count;
extern int truck_count;
extern int direction;
extern int ferry_trip_number;
extern int boarded_ids[30];
extern int boarded_count;
extern sem_t toll_sem[4];
extern FILE* log_file;
extern pthread_mutex_t log_mutex;
extern int is_first_return;
extern int final_trip_done;

void* vehicle_func(void* arg) {
    Vehicle* v = (Vehicle*)arg;

    sleep(1 + rand() % 2);
    sem_wait(&toll_sem[v->toll]);
    usleep(100000);
    sem_post(&toll_sem[v->toll]);

    // İlk biniş: Araçlar SIDE-A'dan SIDE-B'ye gider
    int boarded = 0;
    while (!boarded) {
        pthread_mutex_lock(&boarding_mutex);
        if (direction == 0 && current_capacity + v->type <= MAX_CAPACITY) {
            current_capacity += v->type;
            boarded_ids[boarded_count++] = v->id;
            v->location = 1;
            v->b_trip_no = ferry_trip_number;
            boarded = 1;
        }
        pthread_mutex_unlock(&boarding_mutex);
        if (!boarded) usleep(20000);
    }

    // SIDE-B'ye ulaşmayı bekle
    while (direction != 1) usleep(20000);

    // SIDE-B'de bir sefer bekle
    while (ferry_trip_number < v->b_trip_no + 2) {
        usleep(20000);
    }

    // SIDE-B'den SIDE-A'ya dönüş
    boarded = 0;
    while (!boarded) {
        pthread_mutex_lock(&boarding_mutex);
        if (direction == 1 && current_capacity + v->type <= MAX_CAPACITY && !is_first_return) {
            current_capacity += v->type;
            boarded_ids[boarded_count++] = v->id;
            v->location = 1;
            v->a_trip_no = ferry_trip_number;
            boarded = 1;
        }
        pthread_mutex_unlock(&boarding_mutex);
        if (!boarded) usleep(20000);
    }

    // SIDE-A'ya ulaşmayı bekle
    while (direction != 0) usleep(20000);

    pthread_mutex_lock(&return_mutex);
    total_returned++;
    if (total_returned == TOTAL_VEHICLES)
        final_trip_done = 1;
    if (v->type == CAR) car_count++;
    else if (v->type == MINIBUS) minibus_count++;
    else if (v->type == TRUCK) truck_count++;
    v->returned = 1;
    pthread_mutex_unlock(&return_mutex);

    pthread_mutex_lock(&log_mutex);
    fprintf(log_file, "Araç %d (%s): B'ye %d. seferde gitti, A'ya %d. seferde döndü.\n",
            v->id, vehicle_type_abbr(v->type), v->b_trip_no, v->a_trip_no);
    pthread_mutex_unlock(&log_mutex);

    return NULL;
}

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread) {
    v->id = id;
    v->type = type;
    v->port = 0; // Tüm araçlar SIDE-A'da başlayacak
    v->toll = rand() % 2; // SIDE-A: Gişe 0 veya 1
    v->returned = 0;
    v->location = 0; // SIDE-A
    v->b_trip_no = -1;
    v->a_trip_no = -1;
    pthread_create(thread, NULL, vehicle_func, v);
}

const char* vehicle_type_str(VehicleType t) {
    switch (t) {
        case CAR: return "CAR";
        case MINIBUS: return "MINIBUS";
        case TRUCK: return "TRUCK";
        default: return "?";
    }
}


const char* vehicle_type_abbr(VehicleType t) {
    switch (t) {
        case CAR: return "C";
        case MINIBUS: return "MB";
        case TRUCK: return "TR";
        default: return "?";
    }
}

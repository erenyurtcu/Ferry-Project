#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "vehicle.h"
#include "ferry.h"

// Global değişkenler
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

extern Vehicle vehicles[];

extern FILE* log_file;
extern pthread_mutex_t log_mutex;

void* vehicle_func(void* arg);

const char* vehicle_type_str(VehicleType t) {
    return t == CAR ? "C" : t == MINIBUS ? "M" : "T";
}

const char* vehicle_type_abbr(VehicleType t) {
    return t == CAR ? "C" : t == MINIBUS ? "MB" : "TR";
}

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread) {
    v->id = id;
    v->type = type;
    v->port = rand() % 2;
    v->toll = v->port * 2 + (rand() % 2);
    v->returned = 0;
    v->location = 0;

    pthread_create(thread, NULL, vehicle_func, v);
}

void* vehicle_func(void* arg) {
    Vehicle* v = (Vehicle*)arg;

    sleep(1 + rand() % 2);

    // Gişeden geç
    sem_wait(&toll_sem[v->toll]);
    sleep(1);
    sem_post(&toll_sem[v->toll]);

    // Feribota binmeye çalış
    int boarded = 0;
    while (!boarded) {
        pthread_mutex_lock(&boarding_mutex);
        if (current_capacity + v->type <= MAX_CAPACITY) {
            current_capacity += v->type;
            boarded_ids[boarded_count++] = v->id;
            v->location = 1;
            boarded = 1;
        }
        pthread_mutex_unlock(&boarding_mutex);

        if (!boarded) usleep(100000);
    }

    // Feribotla yolculuk
    sleep(3 + rand() % 3);

    // Side B'ye in
    v->location = 2;
    sleep(2 + rand() % 2);

    // Geri dönüş
    v->location = 0;
    sleep(1);

    // Geri dönüş sonrası sayacı güncelle
    pthread_mutex_lock(&return_mutex);
    total_returned++;
    if (v->type == CAR) car_count++;
    else if (v->type == MINIBUS) minibus_count++;
    else if (v->type == TRUCK) truck_count++;
    pthread_mutex_unlock(&return_mutex);

    // Çıktı: terminal + log dosyası
    pthread_mutex_lock(&log_mutex);
    printf("Vehicle %d (%s) returned to SIDE-A\n", v->id, vehicle_type_abbr(v->type));
    fflush(stdout); // ekranda hemen göster
    if (log_file) {
        fprintf(log_file, "Vehicle %d (%s) returned to SIDE-A\n", v->id, vehicle_type_abbr(v->type));
        fflush(log_file);
    }
    pthread_mutex_unlock(&log_mutex);

    return NULL;
}

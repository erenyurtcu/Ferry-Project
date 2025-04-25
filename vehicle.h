#ifndef VEHICLE_H
#define VEHICLE_H

#include <pthread.h>

typedef enum { CAR = 1, MINIBUS = 2, TRUCK = 3 } VehicleType;

typedef struct {
    int id;
    VehicleType type;
    int port; // 0 or 1
    int toll; // 0-3
    int returned;
    pthread_t thread_id;
} Vehicle;

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread);
void* vehicle_func(void* arg);

#endif

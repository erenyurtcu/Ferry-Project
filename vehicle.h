#ifndef VEHICLE_H
#define VEHICLE_H

#include <pthread.h>  // ✅ pthread_t kullanımı için gerekli

typedef enum { CAR = 1, MINIBUS = 2, TRUCK = 3 } VehicleType;

typedef struct {
    int id;
    VehicleType type;
    int toll;
    int port;
    int returned;
    int location; // 0: Side A, 1: Ferry, 2: Side B
} Vehicle;

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread);
#endif

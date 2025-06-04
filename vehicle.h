#ifndef VEHICLE_H
#define VEHICLE_H

#include <pthread.h>

typedef enum { CAR = 1, MINIBUS = 2, TRUCK = 3 } VehicleType;

typedef struct {
    int id;
    VehicleType type;
    int toll;
    int port; // 0: SIDE-A, 1: Feribot, 2: SIDE-B
    int returned; // 0: not returned, 1: returned
    int location; // 0: SIDE-A, 1: Feribot, 2: SIDE-B
    int b_trip_no; // SIDE-B'ye gidiş seferi
    int a_trip_no; // SIDE-A'ya gidiş seferi
} Vehicle;

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread);
const char* vehicle_type_str(VehicleType t);
const char* vehicle_type_abbr(VehicleType t);

#endif
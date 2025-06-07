#ifndef VEHICLE_H
#define VEHICLE_H

#include <pthread.h>

typedef enum { CAR = 1, MINIBUS = 2, TRUCK = 3 } VehicleType;

typedef struct {
    int id;
    VehicleType type;
    int toll;
    int port; 
    int returned;
    int location; 
    int b_trip_no;
    int a_trip_no;
} Vehicle;

void create_vehicle(Vehicle* v, int id, VehicleType type, pthread_t* thread);
const char* vehicle_type_str(VehicleType t);
const char* vehicle_type_abbr(VehicleType t);

#endif
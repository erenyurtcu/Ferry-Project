#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "ferry.h"
#include "vehicle.h"

extern Vehicle vehicles[];
extern FILE* log_file;
extern int ferry_trip_number;
extern int current_capacity, boarded_count, direction;
extern int total_returned, car_count, minibus_count, truck_count;
extern int boarded_ids[30];
extern pthread_mutex_t boarding_mutex, return_mutex, log_mutex;

int is_first_return = 1;

void* ferry_func(void* arg) {
    int wait_counter = 0;

    while (1) {
        pthread_mutex_lock(&return_mutex);
        if (total_returned >= TOTAL_VEHICLES) {
            pthread_mutex_unlock(&return_mutex);
            printf("\n📋 Trip Summary:\n");
            for (int i = 0; i < TOTAL_VEHICLES; i++) {
                int simplified_b_trip = (vehicles[i].b_trip_no + 1) / 2;
                int simplified_a_trip = (vehicles[i].a_trip_no + 1) / 2;
            
                const char* type_str = vehicle_type_str(vehicles[i].type);
            
                if (simplified_b_trip == simplified_a_trip) {
                    printf("Vehicle %d (%s): Completed round trip in trip #%d.\n",
                           vehicles[i].id, type_str, simplified_b_trip);
                } else {
                    printf("Vehicle %d (%s): Went to SIDE-B in trip #%d, returned to SIDE-A in trip #%d.\n",
                           vehicles[i].id, type_str, simplified_b_trip, simplified_a_trip);
                }
            }            
            printf("\n✅ Statistics:\nCars: %d | Minibuses: %d | Trucks: %d\n",
                   car_count, minibus_count, truck_count);
            fclose(log_file);
            break;
        }
        pthread_mutex_unlock(&return_mutex);

        pthread_mutex_lock(&boarding_mutex);
        int should_depart = 0;

        // Are there any waiting vehicles?
        int vehicles_waiting = 0;
        for (int i = 0; i < TOTAL_VEHICLES; i++) {
            if ((direction == 0 && vehicles[i].location == 0 && vehicles[i].returned == 0) ||
                (direction == 1 && vehicles[i].location == 2 && vehicles[i].returned == 0)) {
                vehicles_waiting = 1;
                break;
            }
        }

        // 🚢 Departure conditions updated
        if (current_capacity >= MAX_CAPACITY ||  // Ferry is full
            total_returned + current_capacity >= TOTAL_VEHICLES || // All vehicles handled
            (vehicles_waiting && wait_counter >= 20) || // Vehicles waiting too long
            (!vehicles_waiting && current_capacity > 0 && !is_first_return) || // No waiting vehicles but ferry not empty (except first return)
            (is_first_return && direction == 1) || // First return should be empty
            (!vehicles_waiting && current_capacity == 0 && wait_counter >= 30)) { // No vehicles and ferry is empty, force direction switch
            should_depart = 1;
        }

        if (should_depart) {
            if (boarded_count > 0) {
                printf("🚢 Departing with vehicles: ");
                for (int i = 0; i < boarded_count; i++) {
                    printf("%d ", boarded_ids[i]);
                }
                printf("\n");
            } else {
                printf("🚢 Departing empty (direction: %s, trip #%d)\n",
                       direction == 0 ? "A→B" : "B→A", ferry_trip_number);
            }

            // Update vehicle locations
            for (int i = 0; i < boarded_count; i++) {
                vehicles[boarded_ids[i]].location = (direction == 0) ? 2 : 0;
            }

            wait_counter = 0;
            direction = 1 - direction;
            current_capacity = 0;
            boarded_count = 0;
            ferry_trip_number++;

            // Mark first return as done
            if (is_first_return && direction == 0)
                is_first_return = 0;

            pthread_mutex_unlock(&boarding_mutex);
            sleep(3); // Travel time
        } else {
            wait_counter++;
            pthread_mutex_unlock(&boarding_mutex);
            usleep(100000);
        }
    }

    return NULL;
}

#include <stdio.h>
#include <stdlib.h>
#include "vehicle.h"
#include <unistd.h>
#include <pthread.h>
#include "ferry.h"
#include "vehicle.h"

extern Vehicle vehicles[];
extern FILE* log_file;
extern int ferry_trip_number;
extern int current_capacity, boarded_count;
extern int total_returned, car_count, minibus_count, truck_count;
extern int boarded_ids[30];
extern pthread_mutex_t boarding_mutex, return_mutex, log_mutex;
extern int direction;
extern int is_first_return;
extern int total_trip_duration;
extern int trip_durations[20];
extern int trip_count;
extern int trip_directions[20];

int can_fill_remaining(int capacity, int direction) {
    int dp[MAX_CAPACITY + 1] = {0};
    dp[0] = 1;

    for (int i = 0; i < TOTAL_VEHICLES; i++) {
        if (vehicles[i].returned == 0 &&
            vehicles[i].location == (direction == 0 ? 0 : 2)) {

            int weight = vehicles[i].type;
            for (int j = MAX_CAPACITY; j >= weight; j--) {
                if (dp[j - weight])
                    dp[j] = 1;
            }
        }
    }

    return dp[capacity];
}

void* ferry_func(void* arg) {
    int wait_counter = 0;

    while (1) {
        pthread_mutex_lock(&return_mutex);
        if (total_returned >= TOTAL_VEHICLES) {
            pthread_mutex_unlock(&return_mutex);

            printf("\n📋 Trip Summary:\n");
            for (int i = 0; i < TOTAL_VEHICLES; i++) {
                // ✅ Artık ilk dolu sefer Trip #1 olacak
                int simplified_b_trip = ((vehicles[i].b_trip_no + 1) / 2);
                int simplified_a_trip = ((vehicles[i].a_trip_no + 1) / 2);

                const char* type_full = vehicle_type_str(vehicles[i].type);

                if (simplified_b_trip == simplified_a_trip) {
                    printf("Vehicle %d (%s): Completed round trip in trip #%d.\n",
                           vehicles[i].id, type_full, simplified_b_trip);
                } else {
                    printf("Vehicle %d (%s): Went to SIDE-B in trip #%d, returned to SIDE-A in trip #%d.\n",
                           vehicles[i].id, type_full, simplified_b_trip - 1, simplified_a_trip - 1);
                }                
            }

            printf("\n✅ Statistics:\nCars: %d | Minibuses: %d | Trucks: %d\n",
                   car_count, minibus_count, truck_count);
            

            printf("\n🕓 Individual Trip Durations:\n");
            for (int i = 0; i < trip_count; i++) {
                const char* dir_str = trip_directions[i] == 0 ? "A→B" : "B→A";
                printf("  Trip #%d (%s): %d seconds\n", i + 1, dir_str, trip_durations[i]);
            }

            printf("\n📊 Total Trip Time: %d seconds\n", total_trip_duration);
            pthread_mutex_lock(&log_mutex);
            fprintf(log_file, "\n📊 Total Trip Time: %d seconds\n", total_trip_duration);
            pthread_mutex_unlock(&log_mutex);
            fclose(log_file);
            break;
        }
        pthread_mutex_unlock(&return_mutex);

        pthread_mutex_lock(&boarding_mutex);
        int should_depart = 0;

        int vehicles_waiting = 0;
        for (int i = 0; i < TOTAL_VEHICLES; i++) {
            if ((direction == 0 && vehicles[i].location == 0 && vehicles[i].returned == 0) ||
                (direction == 1 && vehicles[i].location == 2 && vehicles[i].returned == 0)) {
                vehicles_waiting = 1;
                break;
            }
        }

        int remaining_capacity = MAX_CAPACITY - current_capacity;
        int cannot_fill = !can_fill_remaining(remaining_capacity, direction);

        should_depart = (
            current_capacity >= MAX_CAPACITY ||
            (current_capacity < MAX_CAPACITY && cannot_fill) ||
            total_returned + current_capacity >= TOTAL_VEHICLES ||
            (vehicles_waiting && wait_counter >= 20) ||
            (!vehicles_waiting && current_capacity > 0 && !is_first_return) ||
            (is_first_return && direction == 1) ||
            (!vehicles_waiting && current_capacity == 0 && wait_counter >= 30)
        );

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

            for (int i = 0; i < boarded_count; i++) {
                vehicles[boarded_ids[i]].location = (direction == 0) ? 2 : 0;
            }

            wait_counter = 0;
            direction = 1 - direction;
            current_capacity = 0;
            boarded_count = 0;
            ferry_trip_number++;

            if (is_first_return && direction == 0)
                is_first_return = 0;

            pthread_mutex_unlock(&boarding_mutex);

            int travel_time = 3 + rand() % 8;
            sleep(travel_time);

            total_trip_duration += travel_time;

            trip_durations[trip_count] = travel_time;
            trip_directions[trip_count] = direction == 0 ? 0 : 1;
            trip_count++;

        } else {
            wait_counter++;
            pthread_mutex_unlock(&boarding_mutex);
            usleep(100000);
        }
    }

    return NULL;
}
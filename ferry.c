#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicle.h"
#include <unistd.h>
#include <pthread.h>
#include "ferry.h"
#include "vehicle.h"

#define ANSI_RESET    "\033[0m"
#define ANSI_BLUE     "\033[34m"
#define ANSI_MAGENTA  "\033[35m"
#define ANSI_GREEN    "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"

extern Vehicle vehicles[];
extern FILE* log_file;
extern int ferry_trip_number;
extern int current_capacity, boarded_count;
extern int total_returned, car_count, minibus_count, truck_count;
extern int boarded_ids[30];
extern pthread_mutex_t boarding_mutex, return_mutex, log_mutex;
extern int direction;
extern int is_first_return;
extern int trip_durations[20];
extern int trip_count;
extern int trip_directions[20];
extern int final_trip_done;
extern int time_elapsed_a[TOTAL_VEHICLES];
extern int time_elapsed_b[TOTAL_VEHICLES];
extern int time_elapsed_ferry[TOTAL_VEHICLES];

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

        if (total_returned == TOTAL_VEHICLES)
            final_trip_done = 1;

        if (total_returned >= TOTAL_VEHICLES) {
            pthread_mutex_unlock(&return_mutex);

        printf("📋 Trip Summary:\n");
        printf("┌────────────┬──────────────┬──────────────────┬──────────────────┬────────────────────────────────────────┐\n");
        printf("│            │              │                  │                  │              " ANSI_MAGENTA "TIME ELAPSED" ANSI_RESET "              │\n");
        printf("│ Vehicle ID │ Vehicle Type │ " ANSI_GREEN "     A -> B     " ANSI_RESET " │ " ANSI_GREEN "     B -> A     " ANSI_RESET " ├────────────┬────────────┬──────────────┤\n");
        printf("│            │              │                  │                  │   " ANSI_MAGENTA "SIDE A" ANSI_RESET "   │   " ANSI_MAGENTA "SIDE B" ANSI_RESET "   │   " ANSI_MAGENTA "ON FERRY" ANSI_RESET "   │\n");
        printf("├────────────┼──────────────┼──────────────────┼──────────────────┼────────────┼────────────┼──────────────┤\n");

        for (int i = 0; i < TOTAL_VEHICLES; i++) {
            int simplified_b_trip = (vehicles[i].b_trip_no / 2);
            int simplified_a_trip = (vehicles[i].a_trip_no / 2);
            const char* type_full = vehicle_type_str(vehicles[i].type);

            const char* color;
            if (strcmp(type_full, "CAR") == 0) color = ANSI_MAGENTA;
            else if (strcmp(type_full, "TRUCK") == 0) color = ANSI_YELLOW;
            else color = ANSI_BLUE;

            printf("│    %2d      │    %s%-7s%s   │  Went in trip " ANSI_MAGENTA "#%d" ANSI_RESET " │  Returned in " ANSI_MAGENTA "#%d" ANSI_RESET "  │",
                vehicles[i].id, color, type_full, ANSI_RESET, simplified_b_trip, simplified_a_trip);

            int elapsed_a = time_elapsed_a[i];
            int elapsed_b = time_elapsed_b[i];
            int elapsed_f = time_elapsed_ferry[i];

            printf("   %2d s     │   %2d s     │    %2d s      │\n", elapsed_a, elapsed_b, elapsed_f);

            if (i != TOTAL_VEHICLES - 1)
                printf("├────────────┼──────────────┼──────────────────┼──────────────────┼────────────┼────────────┼──────────────┤\n");
        }

        printf("└────────────┴──────────────┴──────────────────┴──────────────────┴────────────┴────────────┴──────────────┘\n");




            printf("\n✅ Statistics:\nCars: %d | Minibuses: %d | Trucks: %d\n",
                   car_count, minibus_count, truck_count);
            
            printf("\nTrip Durations for Each Trip\n");

            printf(ANSI_RESET "┌───────────────┬──────────────────────────────────────────────────────────────────────────┬──────────────────────────────────────────────────────────────────────────┐\n");
            printf("│               │                                  " ANSI_GREEN "A -> B" ANSI_RESET "                                  │                                  " ANSI_GREEN "B -> A" ANSI_RESET "                                  │\n");
            printf("│  " ANSI_RESET "Trip Number" ANSI_RESET "  ├──────────────┬───────────────────────────────────────────────────────────┼──────────────┬───────────────────────────────────────────────────────────┤\n");
            printf("│               │     TIME     │                         VEHICLES                          │     TIME     │                         VEHICLES                          │\n");
            printf("├───────────────┼──────────────┼───────────────────────────────────────────────────────────┼──────────────┼───────────────────────────────────────────────────────────┤\n");


            int total_a_to_b = 0;
            int total_b_to_a = 0;

            int logical_trip_no = 1;

            for (int i = 0; i < trip_count; i += 2) {
                int a_to_b_duration = -1, b_to_a_duration = -1;
                char a_to_b_vehicles[256] = "", b_to_a_vehicles[256] = "";

                // A -> B
                if (i < trip_count && trip_directions[i] == 0) {
                    a_to_b_duration = trip_durations[i];
                    for (int v = 0; v < TOTAL_VEHICLES; v++) {
                        if (vehicles[v].b_trip_no != -1 && ((vehicles[v].b_trip_no + 1) / 2) == logical_trip_no) {
                            char temp[8];
                            snprintf(temp, sizeof(temp), "%s%d ", vehicle_type_abbr(vehicles[v].type), vehicles[v].id);
                            strcat(a_to_b_vehicles, temp);
                        }
                    }
                }

                // B -> A
                if (i + 1 < trip_count && trip_directions[i + 1] == 1) {
                    b_to_a_duration = trip_durations[i + 1];
                    for (int v = 0; v < TOTAL_VEHICLES; v++) {
                        if (vehicles[v].a_trip_no != -1 && ((vehicles[v].a_trip_no + 1) / 2) == logical_trip_no + 1) {
                            char temp[8];
                            snprintf(temp, sizeof(temp), "%s%d ", vehicle_type_abbr(vehicles[v].type), vehicles[v].id);
                            strcat(b_to_a_vehicles, temp);
                        }
                    }
                }




                // son sefer sadece A→B ise, atla
                if (i + 1 >= trip_count && a_to_b_duration != -1 && b_to_a_duration == -1)
                    break;

                // print line
                printf("│%4s" ANSI_MAGENTA "%4d" ANSI_RESET "%7s│%4s" ANSI_BLUE "%3d s" ANSI_RESET "%5s│ %-58s│%4s" ANSI_BLUE "%3d s" ANSI_RESET "%5s│ %-58s│\n",
                    "", logical_trip_no, "",
                    "", a_to_b_duration >= 0 ? a_to_b_duration : 0, "", a_to_b_vehicles,
                    "", b_to_a_duration >= 0 ? b_to_a_duration : 0, "", b_to_a_vehicles);

                printf("├───────────────┼──────────────┼───────────────────────────────────────────────────────────┼──────────────┼───────────────────────────────────────────────────────────┤\n");

                total_a_to_b += (a_to_b_duration >= 0 ? a_to_b_duration : 0);
                total_b_to_a += (b_to_a_duration >= 0 ? b_to_a_duration : 0);
                logical_trip_no++;
            }





            // Total Row
            printf("│               │" ANSI_MAGENTA "%7d s" ANSI_RESET "     │%-59s│" ANSI_MAGENTA "%7d s" ANSI_RESET "     │%-59s│\n",
                total_a_to_b, "", total_b_to_a, "");

            printf("│     " ANSI_MAGENTA "Total" ANSI_RESET "     ├──────────────┴───────────────────────────────────────────────────────────┴──────────────┴───────────────────────────────────────────────────────────┤\n");
            printf("│               │          " ANSI_RED "%59d s" ANSI_RESET " total travel time                                                            │\n", total_a_to_b + total_b_to_a);

            printf("└───────────────┴─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘\n");


            int total_trip_duration = total_a_to_b + total_b_to_a;
            printf("\n📊 Total Trip Time: %d seconds\n", total_trip_duration);

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

        if (final_trip_done && direction == 0) {
            pthread_mutex_unlock(&boarding_mutex);
            break;
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
            
            int travel_time = 1 /*2 + rand() % 8*/;
            sleep(travel_time);

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
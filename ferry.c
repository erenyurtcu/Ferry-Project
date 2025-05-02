#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "ferry.h"

// Diğer modüllerden gelen global değişkenler
extern int total_returned;
extern pthread_mutex_t return_mutex;

extern int current_capacity;
extern pthread_mutex_t boarding_mutex;

extern int boarded_ids[30];
extern int boarded_count;

extern int car_count;
extern int minibus_count;
extern int truck_count;

extern int direction;

// 🔔 Görsel çıktıyı bastırmak için visual.c'den fonksiyon alınır
extern void print_state();

void* ferry_func(void* arg) {
    while (1) {
        pthread_mutex_lock(&return_mutex);
        if (total_returned >= TOTAL_VEHICLES) {
            pthread_mutex_unlock(&return_mutex);

            usleep(500000); // ✨ son araçlara zaman tanı
            print_state();  // ✅ SON GÖRSELİ BAS

            printf("🚢 All vehicles returned. Ferry shutting down.\n");

            // 📊 İstatistik özeti
            printf("\n✅ Statistics:\n");
            printf("Cars returned: %d\n", car_count);
            printf("Minibuses returned: %d\n", minibus_count);
            printf("Trucks returned: %d\n", truck_count);
            break;
        }
        pthread_mutex_unlock(&return_mutex);

        pthread_mutex_lock(&boarding_mutex);
        int should_depart = 0;

        if (current_capacity >= MAX_CAPACITY) {
            should_depart = 1;
        } else {
            pthread_mutex_lock(&return_mutex);
            if (total_returned + current_capacity >= TOTAL_VEHICLES)
                should_depart = 1;
            pthread_mutex_unlock(&return_mutex);
        }

        if (should_depart) {
            printf("🚢 Departing with vehicles: ");
            for (int i = 0; i < boarded_count; i++) {
                printf("%d ", boarded_ids[i]);
            }
            printf("\n");
            pthread_mutex_unlock(&boarding_mutex);

            sleep(2); // yolculuk süresi
            direction = 1 - direction; // yön değişir

            printf("🚢 Ferry arrived. Unloading...\n");
            sleep(2);

            pthread_mutex_lock(&boarding_mutex);
            current_capacity = 0;
            boarded_count = 0;
            pthread_mutex_unlock(&boarding_mutex);
        } else {
            pthread_mutex_unlock(&boarding_mutex);
        }

        usleep(200000);
    }

    return NULL;
}

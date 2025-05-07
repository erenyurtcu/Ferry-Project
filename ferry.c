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
            printf("\n📋 Sefer Özeti:\n");
            for (int i = 0; i < TOTAL_VEHICLES; i++) {
                printf("Araç %d (%s): B'ye %d. seferde gitti, A'ya %d. seferde döndü.\n",
                       vehicles[i].id, vehicle_type_abbr(vehicles[i].type),
                       vehicles[i].b_trip_no, vehicles[i].a_trip_no);
            }
            printf("\n✅ İstatistikler:\nArabalar: %d | Minibüsler: %d | Kamyonlar: %d\n",
                   car_count, minibus_count, truck_count);
            fclose(log_file);
            break;
        }
        pthread_mutex_unlock(&return_mutex);

        pthread_mutex_lock(&boarding_mutex);
        int should_depart = 0;

        // Bekleyen araç var mı?
        int vehicles_waiting = 0;
        for (int i = 0; i < TOTAL_VEHICLES; i++) {
            if ((direction == 0 && vehicles[i].location == 0 && vehicles[i].returned == 0) ||
                (direction == 1 && vehicles[i].location == 2 && vehicles[i].returned == 0)) {
                vehicles_waiting = 1;
                break;
            }
        }

        // 🔧 Kalkış koşulları güncellendi
        if (current_capacity >= MAX_CAPACITY ||  // Kapasite doluysa kalk
            total_returned + current_capacity >= TOTAL_VEHICLES || // Tüm araçlar tamam
            (vehicles_waiting && wait_counter >= 20) || // Araç var ama uzun süredir binemedi
            (!vehicles_waiting && current_capacity > 0 && !is_first_return) || // Feribotta araç var ama bekleyen kalmadı (ilk sefer hariç)
            (is_first_return && direction == 1) || // İlk dönüş, boş kalkmalı
            (!vehicles_waiting && current_capacity == 0 && wait_counter >= 30)) { // Araç yok, feribot boş, yönü değiştir
            should_depart = 1;
        }

        if (should_depart) {
            if (boarded_count > 0) {
                printf("🚢 Kalkıyor, araçlar: ");
                for (int i = 0; i < boarded_count; i++) {
                    printf("%d ", boarded_ids[i]);
                }
                printf("\n");
            } else {
                printf("🚢 Boş kalkıyor (yön: %s, sefer: %d)\n",
                       direction == 0 ? "A→B" : "B→A", ferry_trip_number);
            }

            // Araç konumlarını güncelle
            for (int i = 0; i < boarded_count; i++) {
                vehicles[boarded_ids[i]].location = (direction == 0) ? 2 : 0;
            }

            wait_counter = 0;
            direction = 1 - direction;
            current_capacity = 0;
            boarded_count = 0;
            ferry_trip_number++;

            // İlk dönüş tamamlandıysa flag'i sıfırla
            if (is_first_return && direction == 0)
                is_first_return = 0;

            pthread_mutex_unlock(&boarding_mutex);
            sleep(3); // Seyahat süresi
        } else {
            wait_counter++;
            pthread_mutex_unlock(&boarding_mutex);
            usleep(100000);
        }
    }

    return NULL;
}

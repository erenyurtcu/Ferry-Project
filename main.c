#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "vehicle.h"
#include "ferry.h"

#define TOTAL_CARS 12
#define TOTAL_MINIBUSES 10
#define TOTAL_TRUCKS 8
#define TOTAL_VEHICLES (TOTAL_CARS + TOTAL_MINIBUSES + TOTAL_TRUCKS)

// 🧠 Global değişkenler
sem_t toll_sem[4];
pthread_mutex_t boarding_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t return_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int current_capacity = 0;
int total_returned = 0;

int car_count = 0;
int minibus_count = 0;
int truck_count = 0;

int boarded_ids[30];
int boarded_count = 0;

Vehicle vehicles[TOTAL_VEHICLES];
int direction = 0; // 0: A→B, 1: B→A

FILE* log_file;

extern void* print_loop(void*);

int main() {
    srand(time(NULL));

    // 📂 Log dosyasını aç
    log_file = fopen("output.txt", "w");
    if (!log_file) {
        perror("Log file could not be opened");
        exit(EXIT_FAILURE);
    }

    // 🛂 Gişe semaforları başlatılıyor
    for (int i = 0; i < 4; ++i)
        sem_init(&toll_sem[i], 0, 1);

    // 🚢 Feribot thread'i
    pthread_t ferry_thread;
    pthread_create(&ferry_thread, NULL, ferry_func, NULL);

    // 👀 Görsel yazıcı thread
    pthread_t printer_thread;
    pthread_create(&printer_thread, NULL, print_loop, NULL);

    // 🚗 Araç thread’leri
    pthread_t vehicle_threads[TOTAL_VEHICLES];
    int id = 0;

    for (int i = 0; i < TOTAL_CARS; ++i, ++id)
        create_vehicle(&vehicles[id], id, CAR, &vehicle_threads[id]);

    for (int i = 0; i < TOTAL_MINIBUSES; ++i, ++id)
        create_vehicle(&vehicles[id], id, MINIBUS, &vehicle_threads[id]);

    for (int i = 0; i < TOTAL_TRUCKS; ++i, ++id)
        create_vehicle(&vehicles[id], id, TRUCK, &vehicle_threads[id]);

    // 🧠 Tüm araçlar dönmeden program bitmesin
    for (int i = 0; i < TOTAL_VEHICLES; ++i)
        pthread_join(vehicle_threads[i], NULL);

    // 🛑 Feribot thread'i de bekleniyor
    pthread_join(ferry_thread, NULL);

    // 📂 Log dosyasını kapat
    fclose(log_file);

    // 🧹 Temizlik
    for (int i = 0; i < 4; ++i)
        sem_destroy(&toll_sem[i]);

    return 0;
}

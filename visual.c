#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "vehicle.h"
#include "ferry.h"

extern Vehicle vehicles[];
extern int boarded_ids[30];
extern int boarded_count;
extern int total_returned;
extern int current_capacity;
extern int direction;  // 0: A→B, 1: B→A
extern pthread_mutex_t print_mutex;

#define TOTAL_VEHICLES (TOTAL_CARS + TOTAL_MINIBUSES + TOTAL_TRUCKS)

// ANSI renk kodları
#define ANSI_RESET "\033[0m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_GREEN "\033[32m"

// ✅ Eksik tanımı static olarak ekliyoruz (uyarıları engeller)
static const char* vehicle_type_str(VehicleType t) {
    return t == CAR ? "C" : t == MINIBUS ? "M" : "T";
}

void center_text(char* dest, size_t size, const char* content) {
    int pad = (60 - (int)strlen(content)) / 2;
    if (pad < 0) pad = 0;
    snprintf(dest, size, "%*s%.50s", pad, "", content);
}

void print_state() {
    pthread_mutex_lock(&print_mutex);
    system("clear");

    printf("📊 Returned: %d | Ferry Capacity: %d\n\n", total_returned, current_capacity);

    char side_a[61], ferry[61], side_b[61];
    snprintf(side_a, sizeof(side_a), "%*s%s", (30 - (int)strlen("SIDE-A") / 2), "", "SIDE-A");
    snprintf(ferry, sizeof(ferry), "%*s%s", (30 - (int)strlen("🚢 Ferry") / 2), "", "🚢 Ferry");
    snprintf(side_b, sizeof(side_b), "%*s%s", (30 - (int)strlen("SIDE-B") / 2), "", "SIDE-B");
    printf("%s%-60s%s|%s%-62s%s|%s%-60s%s\n", 
           ANSI_BLUE, side_a, ANSI_RESET, 
           ANSI_BLUE, ferry, ANSI_RESET, 
           ANSI_BLUE, side_b, ANSI_RESET);

    // Çizgi
    printf("%-60s|%-60s|%-60s\n",
           "------------------------------------------------------------",
           "------------------------------------------------------------",
           "------------------------------------------------------------");

    // Yön bilgisi
    char direction_line[62];
    if (direction == 0)
        snprintf(direction_line, sizeof(direction_line), "%*s→→→ MOVING TO SIDE-B →→→", (34) / 2, "");
    else
        snprintf(direction_line, sizeof(direction_line), "%*s←←← RETURNING TO SIDE-A ←←←", (31) / 2, "");
    printf("%-60s|%s%-72s%s|%-60s\n", "", 
           direction == 0 ? ANSI_GREEN : ANSI_YELLOW, direction_line, ANSI_RESET, "");

    // Ferry içindekiler
    char ferry_buf[256] = "";
    for (int i = 0; i < boarded_count; ++i) {
        int id = boarded_ids[i];
        char temp[16];
        snprintf(temp, sizeof(temp), "%s%d ", vehicle_type_str(vehicles[id].type), id);
        if (strlen(ferry_buf) + strlen(temp) < sizeof(ferry_buf) - 1)
            strcat(ferry_buf, temp);
        else
            break;
    }
    char ferry_line[61];
    center_text(ferry_line, sizeof(ferry_line), ferry_buf);
    printf("%-60s|%-60s|%-60s\n", "", ferry_line, "");

    // Alt çizgi
    printf("%-60s|%-60s|%-60s\n",
           "------------------------------------------------------------",
           "------------------------------------------------------------",
           "------------------------------------------------------------");

    // Araç grupları
    char left[256], right[256], safe_left[51], safe_right[51];
    char label_left[61], label_right[61];
    char line_left[61], line_right[61];

    const struct {
        VehicleType type;
        const char* label;
        int pad;
    } groups[] = {
        { CAR, "CARS", 3 },
        { MINIBUS, "MINIBUSES", 5 },
        { TRUCK, "TRUCKS", 4 }
    };

    for (int g = 0; g < 3; g++) {
        left[0] = right[0] = '\0';

        for (int i = 0; i < TOTAL_VEHICLES; ++i) {
            if (vehicles[i].type == groups[g].type) {
                char temp[16];
                snprintf(temp, sizeof(temp), "%s%d ", vehicle_type_str(vehicles[i].type), vehicles[i].id);
                if (vehicles[i].location == 0)
                    strcat(left, temp);
                else if (vehicles[i].location == 2)
                    strcat(right, temp);
            }
        }

        strncpy(safe_left, left, 50); safe_left[50] = '\0';
        strncpy(safe_right, right, 50); safe_right[50] = '\0';

        snprintf(label_left, sizeof(label_left), "%*s%-*s", 30 - groups[g].pad, "", 10, groups[g].label);
        snprintf(label_right, sizeof(label_right), "%*s%-*s", 30 - groups[g].pad, "", 10, groups[g].label);
        printf("%s%-60s%s|%-60s|%s%-60s%s\n", 
               ANSI_MAGENTA, label_left, ANSI_RESET, 
               "", 
               ANSI_MAGENTA, label_right, ANSI_RESET);

        center_text(line_left, sizeof(line_left), safe_left);
        center_text(line_right, sizeof(line_right), safe_right);
        printf("%-60s|%-60s|%-60s\n", line_left, "", line_right);
    }

    printf("\n");
    pthread_mutex_unlock(&print_mutex);
}

void* print_loop(void* arg) {
    while (1) {
        print_state();
        usleep(500000);
    }
    return NULL;
}

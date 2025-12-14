/*
 * main.c
 * Program buradan basliyor.
 * Dosyayi okuyup sistemi calistiriyor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "scheduler.h"
#include "tasks.h"

#define MAX_TASKS 100

TaskHandle_t_Custom allTasks[MAX_TASKS];
int total_tasks = 0;
extern int global_tick_count;

/* Dosyadan gorevleri okuyan fonksiyon */
void readInputFile(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("HATA: Dosya acilamadi: %s\n", filename);
        exit(1);
    }
    char line[100];
    int id_counter = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (total_tasks >= MAX_TASKS) break;
        TaskHandle_t_Custom *t = &allTasks[total_tasks];
        
        // Verileri alip degiskenlere atiyoruz
        if(sscanf(line, "%d, %d, %d", &t->arrival_time, &t->priority, &t->burst_time) == 3) {
            t->id = id_counter++;
            t->remaining_time = t->burst_time;
            t->current_prio = t->priority;
            t->wait_time = 0;
            total_tasks++;
        }
    }
    fclose(file);
}

/* Bu gorev yeni gelen isleri kuyruga ekliyor (Loader) */
void vTaskLoader(void *pvParameters) {
    (void) pvParameters;
    int i;
    int tasks_dispatched[MAX_TASKS] = {0}; // Gonderilenleri isaretle
    int last_checked_tick = -1;

    for(;;) {
        // Her saniye kontrol et
        if(global_tick_count > last_checked_tick) {
            last_checked_tick = global_tick_count;

            for(i = 0; i < total_tasks; i++) {
                // Vakti gelen gorevi ilgili kuyruga gonder
                if(!tasks_dispatched[i] && allTasks[i].arrival_time == last_checked_tick) {
                    
                    if(allTasks[i].priority == 0) xQueueSend(xRealTimeQueue, &allTasks[i], 0);
                    else if(allTasks[i].priority == 1) xQueueSend(xQueue1, &allTasks[i], 0);
                    else if(allTasks[i].priority == 2) xQueueSend(xQueue2, &allTasks[i], 0);
                    else xQueueSend(xQueue3, &allTasks[i], 0);
                    
                    tasks_dispatched[i] = 1;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(int argc, char *argv[]) {
    const char* filename = (argc > 1) ? argv[1] : "giris.txt";
    
    // Ciktilar takilmasin diye bufferi kapattim
    setvbuf(stdout, NULL, _IONBF, 0);

    readInputFile(filename);
    vSchedulerInit(); 
    xTaskCreate(vTaskLoader, "Loader", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    vTaskStartScheduler();
    return 0;
}

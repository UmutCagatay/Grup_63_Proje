/*
 * scheduler.c
 * Is siralama islemleri burada yapiliyor.
 * Kuyruklardan gorev secip calistiriyoruz.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "scheduler.h"
#include "tasks.h"

/* Main dosyasindaki gorev listesini kullaniyoruz */
extern TaskHandle_t_Custom allTasks[];
extern int total_tasks;

int global_tick_count = 0; // Saniye sayaci

/* Kuyruk tanimlamalari */
QueueHandle_t xRealTimeQueue;
QueueHandle_t xQueue1;
QueueHandle_t xQueue2;
QueueHandle_t xQueue3;

#define ANSI_COLOR_RESET "\x1b[0m"

/* Ekrana yazdirirken kullandigim kelimeler */
#define STR_RUNNING "yürütülüyor"
#define STR_START   "başladı    "
#define STR_SUSPEND "askıda     "
#define STR_END     "sonlandı   "
#define STR_TIMEOUT "ZAMANAŞIMI "

/* Cikti duzenli olsun diye formatladim */
#define LOG_FORMAT "%s%4d.0000 sn   task%-2d   %s    (id:%04d   öncelik:%d   kalan süre:%d sn)%s\n"
#define LOG_END_FORMAT "%s%4d.0000 sn   task%-2d   %s    (id:%04d   öncelik:%d   kalan süre:0 sn)%s\n"

/* Islemciyi simule eden gorev */
void vSystemSimulatorTask(void *pvParameters) {
    (void) pvParameters;
    TaskHandle_t_Custom currentTask; 
    int task_found = 0;
    int completed_tasks = 0;
    int i;

    for(;;) {
        task_found = 0;

        /* --- ADIM 1: Bekleyenleri kontrol et --- */
        /* Burada gorevleri calistirmiyoruz, sadece cok bekleyen var mi ona bakiyoruz */
        for(i = 0; i < total_tasks; i++) {
            // Gorev sisteme girmis ama bitmemis ise:
            if(allTasks[i].arrival_time <= global_tick_count && allTasks[i].remaining_time > 0) {
                
                // Gorevin bekleme suresini artiriyoruz
                allTasks[i].wait_time++; 

                // 20 saniye kurali: Eger 20 saniyedir hic calismadiysa siliyoruz
                if(allTasks[i].wait_time >= 20) {
                    const char* clr = get_task_color(allTasks[i].id);
                    printf(LOG_FORMAT, 
                          clr, global_tick_count, allTasks[i].id, STR_TIMEOUT, 
                          allTasks[i].id, allTasks[i].current_prio, allTasks[i].remaining_time, ANSI_COLOR_RESET);
                    fflush(stdout);
                    
                    allTasks[i].remaining_time = 0; // Gorevi iptal et
                    completed_tasks++;
                }
            }
        }

        /* --- ADIM 2: Hangi gorevi yapacagiz? --- */
        /* Kuyruklara sirasiyla bakiyoruz. Once 0, sonra 1, 2, 3. */
        while(!task_found) {
            if(xQueueReceive(xRealTimeQueue, &currentTask, 0) == pdTRUE) task_found = 1;
            else if(xQueueReceive(xQueue1, &currentTask, 0) == pdTRUE) task_found = 1;
            else if(xQueueReceive(xQueue2, &currentTask, 0) == pdTRUE) task_found = 1;
            else if(xQueueReceive(xQueue3, &currentTask, 0) == pdTRUE) task_found = 1;
            else {
                // Hicbir kuyrukta is yok, donguden cik
                break; 
            }

            // Sectigimiz gorev az once zaman asimina ugradiysa onu atliyoruz
            if(allTasks[currentTask.id].remaining_time == 0) {
                task_found = 0;
            }
        }

        /* --- ADIM 3: Gorevi Calistir --- */
        if(task_found) {
            // Gorev calistigi icin bekleme suresini sifirliyoruz
            allTasks[currentTask.id].wait_time = 0;
            
            const char* clr = get_task_color(currentTask.id);
            
            // Ilk defa basliyorsa "basladi" yaz
            if(currentTask.remaining_time == currentTask.burst_time) {
                printf(LOG_FORMAT, 
                    clr, global_tick_count, currentTask.id, STR_START, 
                    currentTask.id, currentTask.current_prio, currentTask.remaining_time, ANSI_COLOR_RESET);
            } 
            
            // Yurutuluyor yaz
            printf(LOG_FORMAT, 
                clr, global_tick_count, currentTask.id, STR_RUNNING, 
                currentTask.id, currentTask.current_prio, currentTask.remaining_time, ANSI_COLOR_RESET);
            fflush(stdout); // Bekletmeden ekrana bas

            // Suresini azaltiyoruz
            currentTask.remaining_time--;
            allTasks[currentTask.id].remaining_time = currentTask.remaining_time; 
            
            /* Gorev bitti mi? */
            if(currentTask.remaining_time <= 0) {
                printf(LOG_END_FORMAT, 
                    clr, global_tick_count + 1, currentTask.id, STR_END, 
                    currentTask.id, currentTask.current_prio, ANSI_COLOR_RESET);
                fflush(stdout);
                completed_tasks++;
            } 
            else {
                /* Bitmedi, geri kuyruga atalim */
                if(currentTask.priority == 0) {
                    // Oncelikli gorevler kesilmez, basa doner
                    xQueueSendToFront(xRealTimeQueue, &currentTask, 0);
                }
                else {
                    // Normal gorevlerin onceligi duser (3'e kadar)
                    int new_prio = currentTask.current_prio;
                    if(new_prio < 3) new_prio++; 

                    printf(LOG_FORMAT, 
                        clr, global_tick_count + 1, currentTask.id, STR_SUSPEND, 
                        currentTask.id, new_prio, currentTask.remaining_time, ANSI_COLOR_RESET);
                    fflush(stdout);
                    
                    currentTask.current_prio = new_prio;
                    allTasks[currentTask.id].current_prio = new_prio;

                    // Uygun kuyrugun sonuna ekle
                    if(currentTask.current_prio == 1) xQueueSend(xQueue1, &currentTask, 0);
                    else if(currentTask.current_prio == 2) xQueueSend(xQueue2, &currentTask, 0);
                    else xQueueSend(xQueue3, &currentTask, 0);
                }
            }
        } 
        else {
            /* Yapacak is yoksa bos durdugunu goster */
             if(completed_tasks < total_tasks) {
                 printf("%4d.0000 sn   CPU Bosta\n", global_tick_count);
                 fflush(stdout);
             }
        }

        /* Hepsi bittiyse kapat */
        if(completed_tasks >= total_tasks) {
            printf("\n%sTUM GOREVLER TAMAMLANDI.%s\n", ANSI_COLOR_RESET, ANSI_COLOR_RESET);
            fflush(stdout);
            exit(0);
        }

        global_tick_count++;
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 saniye bekle
    }
}

void vSchedulerInit(void) {
    // Kuyruklari burada olusturuyoruz
    xRealTimeQueue = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    xQueue1 = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    xQueue2 = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    xQueue3 = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    
    // Simulatoru baslatiyoruz
    xTaskCreate(vSystemSimulatorTask, "Simulator", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
}

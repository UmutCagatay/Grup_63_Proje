#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "scheduler.h"
#include "tasks.h"

extern TaskHandle_t_Custom allTasks[];
extern int total_tasks;

int global_tick_count = 0;

QueueHandle_t xRealTimeQueue;
QueueHandle_t xQueue1;
QueueHandle_t xQueue2;
QueueHandle_t xQueue3;

#define ANSI_COLOR_RESET "\x1b[0m"

#define STR_RUNNING "yürütülüyor"
#define STR_START   "başladı    "
#define STR_SUSPEND "askıda     "
#define STR_END     "sonlandı   "
#define STR_TIMEOUT "ZAMANAŞIMI "

#define LOG_FORMAT "%s%4d.0000 sn   task%-2d   %s    (id:%04d   öncelik:%d   kalan süre:%d sn)%s\n"
#define LOG_END_FORMAT "%s%4d.0000 sn   task%-2d   %s    (id:%04d   öncelik:%d   kalan süre:0 sn)%s\n"

void vSystemSimulatorTask(void *pvParameters) {
    (void) pvParameters;
    TaskHandle_t_Custom currentTask;
    int task_found = 0;
    int completed_tasks = 0;
    int i;

    for(;;) {
        task_found = 0;

        for(i = 0; i < total_tasks; i++) {
            if(allTasks[i].arrival_time <= global_tick_count && allTasks[i].remaining_time > 0) {
                
                allTasks[i].wait_time++;

                if(allTasks[i].wait_time >= 20) {
                    const char* clr = get_task_color(allTasks[i].id);
                    printf(LOG_FORMAT, 
                          clr, global_tick_count, allTasks[i].id, STR_TIMEOUT, 
                          allTasks[i].id, allTasks[i].current_prio, allTasks[i].remaining_time, ANSI_COLOR_RESET);
                    fflush(stdout);
                    
                    allTasks[i].remaining_time = 0;
                    completed_tasks++;
                }
            }
        }

        while(!task_found) {
            if(xQueueReceive(xRealTimeQueue, &currentTask, 0) == pdTRUE) task_found = 1;
            else if(xQueueReceive(xQueue1, &currentTask, 0) == pdTRUE) task_found = 1;
            else if(xQueueReceive(xQueue2, &currentTask, 0) == pdTRUE) task_found = 1;
            else if(xQueueReceive(xQueue3, &currentTask, 0) == pdTRUE) task_found = 1;
            else {
                break;
            }

            if(allTasks[currentTask.id].remaining_time == 0) {
                task_found = 0;
            }
        }

        if(task_found) {
            allTasks[currentTask.id].wait_time = 0;
            
            const char* clr = get_task_color(currentTask.id);
            
            if(currentTask.remaining_time == currentTask.burst_time) {
                printf(LOG_FORMAT, 
                    clr, global_tick_count, currentTask.id, STR_START, 
                    currentTask.id, currentTask.current_prio, currentTask.remaining_time, ANSI_COLOR_RESET);
            } 
            
            printf(LOG_FORMAT, 
                clr, global_tick_count, currentTask.id, STR_RUNNING, 
                currentTask.id, currentTask.current_prio, currentTask.remaining_time, ANSI_COLOR_RESET);
            fflush(stdout);

            currentTask.remaining_time--;
            allTasks[currentTask.id].remaining_time = currentTask.remaining_time;
            
            if(currentTask.remaining_time <= 0) {
                printf(LOG_END_FORMAT, 
                    clr, global_tick_count + 1, currentTask.id, STR_END, 
                    currentTask.id, currentTask.current_prio, ANSI_COLOR_RESET);
                fflush(stdout);
                completed_tasks++;
            } 
            else {
                if(currentTask.priority == 0) {
                    xQueueSendToFront(xRealTimeQueue, &currentTask, 0);
                }
                else {
                    int new_prio = currentTask.current_prio;
                    if(new_prio < 3) new_prio++;

                    printf(LOG_FORMAT, 
                        clr, global_tick_count + 1, currentTask.id, STR_SUSPEND, 
                        currentTask.id, new_prio, currentTask.remaining_time, ANSI_COLOR_RESET);
                    fflush(stdout);
                    
                    currentTask.current_prio = new_prio;
                    allTasks[currentTask.id].current_prio = new_prio;

                    if(currentTask.current_prio == 1) xQueueSend(xQueue1, &currentTask, 0);
                    else if(currentTask.current_prio == 2) xQueueSend(xQueue2, &currentTask, 0);
                    else xQueueSend(xQueue3, &currentTask, 0);
                }
            }
        }
        else {
             if(completed_tasks < total_tasks) {
                 printf("%4d.0000 sn   CPU Bosta\n", global_tick_count);
                 fflush(stdout);
             }
        }

        if(completed_tasks >= total_tasks) {
            printf("\n%sTUM GOREVLER TAMAMLANDI.%s\n", ANSI_COLOR_RESET, ANSI_COLOR_RESET);
            fflush(stdout);
            exit(0);
        }

        global_tick_count++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vSchedulerInit(void) {
    xRealTimeQueue = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    xQueue1 = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    xQueue2 = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    xQueue3 = xQueueCreate(100, sizeof(TaskHandle_t_Custom));
    
    xTaskCreate(vSystemSimulatorTask, "Simulator", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
}

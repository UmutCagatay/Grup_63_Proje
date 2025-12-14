#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "tasks.h"

void vSchedulerInit(void);
void vSystemSimulatorTask(void *pvParameters);

/* 4 tane kuyruk var. 0 en onemli, 3 en dusuk oncelikli */
extern QueueHandle_t xRealTimeQueue; 
extern QueueHandle_t xQueue1;        
extern QueueHandle_t xQueue2;        
extern QueueHandle_t xQueue3;        

#endif

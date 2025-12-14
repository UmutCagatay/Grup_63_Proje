#ifndef TASKS_H
#define TASKS_H

typedef struct {
    int id;
    int arrival_time;
    int priority;
    int burst_time;
    int remaining_time;
    int current_prio;
    int wait_time;
} TaskHandle_t_Custom;

const char* get_task_color(int id);

#endif

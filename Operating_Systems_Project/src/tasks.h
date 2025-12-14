#ifndef TASKS_H
#define TASKS_H

/* Prosesin butun ozelliklerini burada tutuyoruz (PCB yapisi) */
typedef struct {
    int id;             // Prosesin numarasi
    int arrival_time;   // Ne zaman geldi
    int priority;       // Onceligi (0-3 arasi)
    int burst_time;     // Ne kadar calisacak
    int remaining_time; // Ne kadar suresi kaldi
    int current_prio;   // O anki onceligi
    int wait_time;      // Kuyrukta ne kadar bekledi (20sn kurali icin)
} TaskHandle_t_Custom; 

/* Renk ayarlamak icin fonksiyon */
const char* get_task_color(int id);

#endif

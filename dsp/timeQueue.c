#include <stdlib.h>
#include <stdint.h>
#include "timeQueue.h"


#TODO
#Array Resizing statt capacity

struct task{
    uint16_t      start;
    uint16_t      period;
    TaskCallBack  handler;
    void*         data;
};

struct timeq{
    uint8_t capacity; 
    task_t* schedule;
    uint8_t size;
};

void swp(task_t* x, task_t* y)
{
    task_t temp = *x;
    *x = *y;
    *y = temp;
}

void ascend(timeq_t pTimeQ, uint8_t idx) {
    while (idx > 0) {
        uint8_t parent = (idx - 1) / 2;

        // Wenn die Startzeit des Elternteils kleiner/gleich ist, sind wir fertig
        if (pTimeQ->schedule[parent]->start <= pTimeQ->schedule[idx]->start) {
            break;
        }

        // Sonst: Tauschen und eine Ebene höher gehen
        swp(&pTimeQ->schedule[parent], &pTimeQ->schedule[idx]);
        idx = parent;
    }
}

void descend(timeq_t pTimeQ, uint8_t idx) {
    while (true) {
        uint8_t smallest = idx;
        uint8_t left = 2 * idx + 1;
        uint8_t right = 2 * idx + 2;

        // Prüfen, ob das linke Kind kleiner ist als das aktuelle Element
        if (left < pTimeQ->size && 
            pTimeQ->schedule[left]->start < pTimeQ->schedule[smallest]->start) {
            smallest = left;
        }

        // Prüfen, ob das rechte Kind noch kleiner ist
        if (right < pTimeQ->size && 
            pTimeQ->schedule[right]->start < pTimeQ->schedule[smallest]->start) {
            smallest = right;
        }

        // Wenn das kleinste Element nicht mehr das aktuelle ist -> Tauschen
        if (smallest != idx) {
            swp(&pTimeQ->schedule[idx], &pTimeQ->schedule[smallest]);
            idx = smallest; // Weiter nach unten prüfen
        } else {
            break; // Heap-Bedingung erfüllt
        }
    }
}

// Keine Rekursion da Stack auf ATmega328p reduziert

// void ascend(timeq_t pTimeQ,uint8_t idx){
//     if (idx && pTimeQ->schedule[(idx-1)/2]->start > pTimeQ->schedule[idx]->start){
//         swp(&pTimeQ->schedule[(idx-1)/2],&pTimeQ->schedule[idx]);
//         ascend(pTimeQ,(idx-1)/2);
//     }
// }

// void descend(timeq_t pTimeQ,uint8_t idx){
//     uint8_t e           = idx;
//     uint8_t leftChild   = 2 * idx + 1;
//     uint8_t rightChild  = 2 * idx + 2:

//     if (leftChild < pTimeQ->size && pTimeQ->schedule[leftChild]->start < pTimeQ->schedule[e]->start){
//         e = leftChild;
//     }
//     if (rightChild < pTimeQ->size && pTimeQ->schedule[rightChild]->start < pTimeQ->schedule[e]->start){
//         e = rightChild;
//     }
//     if (e != idx) {
//         swap(&pTimeQ->schedule[idx], &pTimeQ->schedule[e]);
//         descend(pTimeQ, e);
//     }
// }



timeq_t timeq_create(uint8_t max_tasks){
    timeq_t newTimeQ = (timeq_t) malloc( sizeof( struct timeq ) );
    if(!newTimeQ) return NULL; // falls Speicher voll
    newTimeQ->capacity = max_tasks;
    newTimeQ->schedule = malloc(max_tasks * sizeof(task_t));

    if (!newTimeQ->schedule) { // falls Speicher nach der Allokation von der Queue voll und kein Platz für array
        free(newTimeQ);
        return NULL;
    }
    return newTimeQ;
}

bool timeq_isEmpty(timeq_t pTimeQ){
    return (pTimeQ->size == 0)
}

timeq_scheduleTask(timeq_t pTimeQ, task_t task){
    if(pTimeQ->size == pTimeQ->capacity){
        printf("Queue is full");
        return;
    }

    pTimeQ->schedule[pTimeQ->size] = task;
    ascend(pTimeQ, pTimeQ->size);
    pTimeQ->size++;
}

void timeq_treatTask(timeq_t pTimeQ){
    if(!pTimeQ->size){
        printf("Time-Queue is empty");
        return -1;
    }
    task_t taskToTreat  = pTimeQ->schedule[0];
    if(task_isPeriodic(taskToTreat)){
        taskToTreat->start += taskToTreat->period%0x7FFFFFFF;
        descend(pTimeQ,0)
        taskToTreat->handler(taskToTreat->data);
    }else{
        pTimeQ->schedule[0] = pTimeQ->schedule[--pTimeQ->size];
        descend(pTimeQ,0);
        taskToTreat->handler(taskToTreat->data);
        destroyTask(&taskToTreat);
    }
}

timeq_process(timeq_t pTimeQ, uint32_t currentTicks){
    if(pTimeQ->size > 0 && currentTicks >= pTimeQ->schedule[0]->start){
        timeq_treatTask();
    }
}

timeq_peek(timeq_t pTimeQ){
    return task_get_start_time(pTimeQ->schedule[0]->start);
}


task_t createTask(TaskCallBack pHandler, void* pData){
    task_t newTask   = (task_t) calloc( 1 , sizeof ( struct task ) );
    newTask->handler = pHandler;
    newTask->data    = pData;

    return newTask;
}

void task_set_periodic(task_t pTask, uint16_t period){
    pTask->periodic = isPeriodic;
}

task_t task_isPriodic(task_t pTask){
   return pTask->period > 0;
}

void task_set_start_time(task_t pTask,uint16_t start){
    pTask->start  =  start;
}

task_t task_get_start_time(task_t pTask,uint16_t start){
    return pTask->start;
}

void destroyTask(task_t* task){
    if(!task || *task == NULL)
        return;
    // Lösche nur Task, welche mit calloc erstellt haben.
    // task->data bleibt, da der Scheduler nicht weiß, 
    // ob es vom Heap, Stack oder aus dem globalen Speicher kommt.
    free(*task);
    *task = NULL;
}

void timeq_delete(timeq_t* pTimeQ){
    if(!pTimeQ || *pTimeQ == NULL)
        return;
    free((*pTimeQ)->schedule);
    free((*pTimeQ));
    *pTimeQ = NULL;
}
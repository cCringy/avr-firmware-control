#ifndef TIMEQUEUE_H_
#define TIMEQUEUE_H_
 
#include <stdint.h>
#include <stdbool.h>
 
// ADT Pattern
typedef struct timeq timeq;    // timeq instance representation (opaque record)
typedef timeq* timeq_t;        // pointer to timeq instance (opaque pointer)
typedef struct task task;      // task instance representation (opaque record)
typedef task* task_t;          // value type stored in timeq instance
 
// Zeittyp für Ticks. 32 Bit, Vergleiche sind overflow-sicher (siehe timeQueue.c)
typedef uint32_t tick_t;
 
// Typ des Callbacks, der beim Ausführen eines Tasks aufgerufen wird
typedef void (*TaskCallback)(void *data);
 
// ---------- Task ----------
task_t task_create(TaskCallback pHandler, void* pData);
void   task_destroy(task_t* pTask);
void   task_set_periodic(task_t pTask, tick_t period);
bool   task_isPeriodic(task_t pTask);
void   task_set_start_time(task_t pTask, tick_t start);
tick_t task_get_start_time(task_t pTask);
 
// ---------- Time-Queue ----------
timeq_t timeq_create(uint8_t max_tasks);
void    timeq_delete(timeq_t* pTimeQ);
bool    timeq_isEmpty(timeq_t pTimeQ);
bool    timeq_scheduleTask(timeq_t pTimeQ, task_t task);
bool    timeq_treatTask(timeq_t pTimeQ);
bool    timeq_process(timeq_t pTimeQ, tick_t currentTicks);
bool    timeq_peek(timeq_t pTimeQ, tick_t* pStart);
 
#endif
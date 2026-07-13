#ifndef TIMEQUEUE_H_
#define TIMEQUEUE_H_

//ADT Pattern
typedef struct timeq timeq;    // timeq instance representation (opaque record)
typedef timeq* timeq_t;        // path to timerq instance (opaque pointer)
typedef struct task task;      // task instance representation (opaque record)
typedef task* task_t;          // value type stored in timeq instance
//Definiere typ des Callbacks für Event
typedef void (*TaskCallback)(void *data);

timeq_t timeq_create(void);

#endif
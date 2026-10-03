#include <stdlib.h>
#include <stdint.h>
#include "timeQueue.h"

// TODO
// Array Resizing statt capacity

struct task{
    tick_t        start;    // Zeitpunkt der nächsten Ausführung
    tick_t        period;   // 0 = einmaliger Task, >0 = periodisch
    TaskCallback  handler;  // wird bei Ausführung aufgerufen
    void*         data;     // Argument für handler (gehört dem Aufrufer)
};

struct timeq{
    uint8_t capacity;       // maximale Anzahl Tasks
    uint8_t size;           // aktuelle Anzahl Tasks
    task_t* schedule;       // Min-Heap (Array), sortiert nach start
};

// ---------------------------------------------------------------
// Interne Hilfsfunktionen
// ---------------------------------------------------------------

/**
 * Overflow-sicherer Zeitvergleich: true, wenn Zeitpunkt a vor b liegt.
 * Funktioniert auch, wenn der 32-Bit-Tickzähler überläuft, solange
 * a und b weniger als 2^31 Ticks auseinander liegen.
 */
static bool isBefore(tick_t a, tick_t b){
    return (int32_t)(a - b) < 0;
}

/**
 * Vertauscht zwei Task-Zeiger im Heap-Array.
 */
static void swp(task_t* x, task_t* y)
{
    task_t temp = *x;
    *x = *y;
    *y = temp;
}

/**
 * Stellt die Heap-Bedingung nach oben wieder her (sift-up).
 * Das Element an idx wandert nach oben, solange es früher dran ist
 * als sein Elternteil. Iterativ, da der Stack auf dem ATmega328P knapp ist.
 */
static void ascend(timeq_t pTimeQ, uint8_t idx) {
    while (idx > 0) {
        uint8_t parent = (idx - 1) / 2;

        // Wenn der Elternteil nicht später dran ist, sind wir fertig
        if (!isBefore(pTimeQ->schedule[idx]->start, pTimeQ->schedule[parent]->start)) {
            break;
        }

        // Sonst: Tauschen und eine Ebene höher gehen
        swp(&pTimeQ->schedule[parent], &pTimeQ->schedule[idx]);
        idx = parent;
    }
}

/**
 * Stellt die Heap-Bedingung nach unten wieder her (sift-down).
 * Das Element an idx wandert nach unten, solange eines seiner Kinder
 * früher dran ist. Iterativ, da der Stack auf dem ATmega328P knapp ist.
 * Indizes sind uint16_t, damit 2*idx+2 bei capacity > 127 nicht überläuft.
 */
static void descend(timeq_t pTimeQ, uint8_t idx) {
    while (true) {
        uint16_t smallest = idx;
        uint16_t left     = 2 * (uint16_t)idx + 1;
        uint16_t right    = 2 * (uint16_t)idx + 2;

        // Ist das linke Kind früher dran als das aktuelle Element?
        if (left < pTimeQ->size &&
            isBefore(pTimeQ->schedule[left]->start, pTimeQ->schedule[smallest]->start)) {
            smallest = left;
        }

        // Ist das rechte Kind noch früher dran?
        if (right < pTimeQ->size &&
            isBefore(pTimeQ->schedule[right]->start, pTimeQ->schedule[smallest]->start)) {
            smallest = right;
        }

        // Wenn das früheste Element nicht das aktuelle ist -> Tauschen
        if (smallest != idx) {
            swp(&pTimeQ->schedule[idx], &pTimeQ->schedule[smallest]);
            idx = (uint8_t)smallest; // Weiter nach unten prüfen
        } else {
            break; // Heap-Bedingung erfüllt
        }
    }
}

// ---------------------------------------------------------------
// Time-Queue
// ---------------------------------------------------------------

/**
 * Erzeugt eine leere Time-Queue für höchstens max_tasks Tasks.
 * Rückgabe: Zeiger auf die Queue, oder NULL bei max_tasks == 0
 * bzw. wenn kein Speicher mehr frei ist.
 */
timeq_t timeq_create(uint8_t max_tasks){
    if (max_tasks == 0) return NULL;

    // calloc setzt size auf 0
    timeq_t newTimeQ = (timeq_t) calloc(1, sizeof(struct timeq));
    if (!newTimeQ) return NULL; // falls Speicher voll

    newTimeQ->capacity = max_tasks;
    newTimeQ->schedule = (task_t*) malloc(max_tasks * sizeof(task_t));

    if (!newTimeQ->schedule) { // kein Platz für das Array -> Queue wieder freigeben
        free(newTimeQ);
        return NULL;
    }
    return newTimeQ;
}

/**
 * Gibt true zurück, wenn keine Tasks eingeplant sind.
 */
bool timeq_isEmpty(timeq_t pTimeQ){
    return (pTimeQ->size == 0);
}

/**
 * Plant einen Task ein (Start- und Periodenzeit müssen vorher am Task
 * gesetzt sein). Die Queue übernimmt den Besitz des Tasks.
 * Rückgabe: true bei Erfolg, false bei vollem Heap oder task == NULL.
 */
bool timeq_scheduleTask(timeq_t pTimeQ, task_t task){
    if (!task || pTimeQ->size == pTimeQ->capacity){
        return false;
    }

    pTimeQ->schedule[pTimeQ->size] = task;
    ascend(pTimeQ, pTimeQ->size);
    pTimeQ->size++;
    return true;
}

/**
 * Führt den Task an der Spitze des Heaps aus (unabhängig von der Zeit).
 * Periodische Tasks werden um ihre Periode verschoben und bleiben in der
 * Queue, einmalige Tasks werden entfernt und nach der Ausführung freigegeben.
 * Rückgabe: false, wenn die Queue leer ist, sonst true.
 */
bool timeq_treatTask(timeq_t pTimeQ){
    if (pTimeQ->size == 0){
        return false;
    }

    task_t taskToTreat = pTimeQ->schedule[0];

    if (task_isPeriodic(taskToTreat)){
        // Nächsten Start berechnen (Überlauf wird durch isBefore abgefangen)
        taskToTreat->start += taskToTreat->period;
        descend(pTimeQ, 0);
        if (taskToTreat->handler) taskToTreat->handler(taskToTreat->data);
    } else {
        // Task aus dem Heap nehmen: letztes Element nach vorne, dann absenken
        pTimeQ->schedule[0] = pTimeQ->schedule[--pTimeQ->size];
        descend(pTimeQ, 0);
        if (taskToTreat->handler) taskToTreat->handler(taskToTreat->data);
        task_destroy(&taskToTreat);
    }
    return true;
}

/**
 * Soll regelmäßig (z.B. in der Main-Loop) mit der aktuellen Tickzeit
 * aufgerufen werden. Führt den ersten Task aus, falls er fällig ist.
 * Rückgabe: true, wenn ein Task ausgeführt wurde.
 */
bool timeq_process(timeq_t pTimeQ, tick_t currentTicks){
    if (pTimeQ->size > 0 && !isBefore(currentTicks, pTimeQ->schedule[0]->start)){
        return timeq_treatTask(pTimeQ);
    }
    return false;
}

/**
 * Liefert die Startzeit des nächsten Tasks in *pStart, ohne ihn zu entfernen.
 * Rückgabe: false, wenn die Queue leer ist (*pStart bleibt dann unverändert).
 */
bool timeq_peek(timeq_t pTimeQ, tick_t* pStart){
    if (pTimeQ->size == 0 || !pStart){
        return false;
    }
    *pStart = task_get_start_time(pTimeQ->schedule[0]);
    return true;
}

/**
 * Gibt die Queue samt aller noch eingeplanten Tasks frei und setzt
 * den Zeiger des Aufrufers auf NULL. task->data wird nicht freigegeben.
 */
void timeq_delete(timeq_t* pTimeQ){
    if (!pTimeQ || *pTimeQ == NULL)
        return;

    // Noch eingeplante Tasks gehören der Queue -> freigeben
    for (uint8_t i = 0; i < (*pTimeQ)->size; i++){
        task_destroy(&(*pTimeQ)->schedule[i]);
    }
    free((*pTimeQ)->schedule);
    free(*pTimeQ);
    *pTimeQ = NULL;
}

// ---------------------------------------------------------------
// Task
// ---------------------------------------------------------------

/**
 * Erzeugt einen neuen Task (start = 0, period = 0, also einmalig).
 * Rückgabe: Task oder NULL, wenn kein Speicher mehr frei ist.
 */
task_t task_create(TaskCallback pHandler, void* pData){
    task_t newTask = (task_t) calloc(1, sizeof(struct task));
    if (!newTask) return NULL;

    newTask->handler = pHandler;
    newTask->data    = pData;
    return newTask;
}

/**
 * Macht den Task periodisch mit der gegebenen Periode in Ticks.
 * period == 0 macht ihn wieder zum einmaligen Task.
 */
void task_set_periodic(task_t pTask, tick_t period){
    pTask->period = period;
}

/**
 * Gibt true zurück, wenn der Task periodisch ist (period > 0).
 */
bool task_isPeriodic(task_t pTask){
    return pTask->period > 0;
}

/**
 * Setzt den Zeitpunkt der (nächsten) Ausführung.
 * Nur vor timeq_scheduleTask aufrufen, sonst stimmt der Heap nicht mehr.
 */
void task_set_start_time(task_t pTask, tick_t start){
    pTask->start = start;
}

/**
 * Liefert den Zeitpunkt der nächsten Ausführung.
 */
tick_t task_get_start_time(task_t pTask){
    return pTask->start;
}

/**
 * Gibt einen Task frei und setzt den Zeiger des Aufrufers auf NULL.
 * task->data bleibt unangetastet, da der Scheduler nicht weiß,
 * ob es vom Heap, Stack oder aus dem globalen Speicher kommt.
 */
void task_destroy(task_t* task){
    if (!task || *task == NULL)
        return;
    free(*task);
    *task = NULL;
}
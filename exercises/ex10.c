#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "drivers/timer.h"
#include "drivers/adc.h"
#include "debug_uart.h"
#include "dsp/buffer.h"
#include "dsp/timeQueue"
#include "board_config.h"

volatile uint32_t currentTime;

void setup(void){
    //timerFreq = 1000 HZ
    // pre = 1 topVal = 16000000/1000*pre = 16.000
    timer_init_timer1(1);

    timeq_t q = timeq_create();
}

void task1()
{
    printf("Task1 executed!\n");
}

void task2()
{
    printf("Task2 executed!\n");
}

void task3()
{
    printf("Task3 (periodic) executed!\n");
}

void task4()
{
    printf("Task4 (periodic) executed!\n");
}


int main(void){
    setup();

    currentTime = 0;

    task_t test_task1 = task_createTask(&task1);
    task_t test_task2 = task_createTask(&task2);
    task_t test_task3 = task_createTask(&task3); // periodic
    task_t test_task4 = task_createTask(&task4); // periodic

    task_set_start_time(test_task1,1000);
    task_set_start_time(test_task2,3500);
    task_set_start_time(test_task3,3000);
    task_set_start_time(test_task4,500);
    
    task_set_periodic(test_task3,1000);
    task_set_periodic(test_task3,700);

    q.scheduleTask(test_task1);
    q.scheduleTask(test_task1);
    q.scheduleTask(test_task1);
    q.scheduleTask(test_task1);

    while(1){
        if(!timeq_isEmpty(q)){
            timeq_process(q,currentTime);
        }
    }
    cli();
    timeq_delete((timeq_t*) &q)

    return 0;
}

ISR(TIMER1_COMPA_vect){
    currentTime++;
}

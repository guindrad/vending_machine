//https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Timer/RepeatTimer/RepeatTimer.ino

#include "timer.h"

hw_timer_t * timer_A = NULL;
hw_timer_t * timer_B = NULL;
hw_timer_t * timer_C = NULL;
hw_timer_t * timer_debouncer = NULL;

void set_timer(void (*onTime_timer)(void), int timer_choice, int time_ms)
{
    //Configure Prescaler to 80, as our timer runs @80Mhz
    //Giving an output of 80,000,000/80 = 1,000,000 ticks/second

    /*
        As input, this function receives the number of the timer we want to use
        (from 0 to 3, since we have 4 hardware timers)
        the value of the prescaler and a flag indicating if the counter should count up (true) or down (false).
    */

   if(timer_choice==1)
   {
        timer_A = timerBegin(1, 80, true);
        timerAttachInterrupt(timer_A, onTime_timer, true);    //Atrela a função de tratamento da interrupção à interrupção
        timerAlarmWrite(timer_A, 1000*time_ms, true);         //Counter value in which the timer interrupt will be generated
        timerAlarmEnable(timer_A);                            // Fire Interruption
   }
   else if(timer_choice==2)
   {
        timer_B = timerBegin(2, 80, true);
        timerAttachInterrupt(timer_B, onTime_timer, true);    //Atrela a função de tratamento da interrupção à interrupção
        timerAlarmWrite(timer_B, 1000*time_ms, true);         //Counter value in which the timer interrupt will be generated
        timerAlarmEnable(timer_B);                            // Fire Interruption
   }
   else if(timer_choice==3)
   {
        timer_C = timerBegin(3, 80, true);
        timerAttachInterrupt(timer_C, onTime_timer, true);    //Atrela a função de tratamento da interrupção à interrupção
        timerAlarmWrite(timer_C, 1000*time_ms, true);         //Counter value in which the timer interrupt will be generated
        timerAlarmEnable(timer_C);                            // Fire Interruption
   }
}

void set_debouncer(void (*onTime_debouncer)(void), int time_ms)
{
    //Configure Prescaler to 80, as our timer runs @80Mhz
    //Giving an output of 80,000,000/80 = 1,000,000 ticks/second
    /*
        As input, this function receives the number of the timer we want to use
        (from 0 to 3, since we have 4 hardware timers)
        the value of the prescaler and a flag indicating if the counter should count up (true) or down (false).

        hw_timer_t *timer_debouncer
    */

    timer_debouncer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer_debouncer, onTime_debouncer, true);      //Atrela a função de
    timerAlarmWrite(timer_debouncer, 1000*time_ms, true);     //Counter value in w
    timerAlarmEnable(timer_debouncer);                        // Fire Interruption
}

void stop_debouncer(void)
{
    if (timer_debouncer)
    {
      // Stop and free timer
      timerEnd(timer_debouncer);
      timer_debouncer = NULL;
    }
}

void stop_timerA(void)
{
    if(timer_A)
    {
      // Stop and free timer
      timerEnd(timer_A);
      timer_A = NULL;
    }
}

void stop_timerB(void)
{
    if(timer_B)
    {
      // Stop and free timer
      timerEnd(timer_B);
      timer_B = NULL;
    }
}
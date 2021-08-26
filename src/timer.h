#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

void set_timer(void (*onTime_timer)(void), int timer_choice, int time_ms);
void set_debouncer(void (*onTime_debouncer)(void), int time_ms);
void stop_debouncer(void);
void stop_timerA(void);
void stop_timerB(void);

#endif
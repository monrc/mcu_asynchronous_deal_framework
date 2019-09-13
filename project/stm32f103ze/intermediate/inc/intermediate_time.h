#ifndef INTERMEDIATE_TIME_H
#define INTERMEDIATE_TIME_H

#include <stdint.h>

void delay_ms(uint32_t time);
uint32_t get_tick(void);
uint16_t get_timer_tick(void);

#endif
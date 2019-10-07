#ifndef INTERMEDIATE_LAYER_H
#define INTERMEDIATE_LAYER_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "intermediate_key.h"
#include "intermediate_led.h"
#include "intermediate_time.h"
#include "intermediate_uart.h"


void mcu_init(void);
void iwdg_refresh(void);

#endif
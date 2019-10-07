#ifndef INTERMEDIATE_UART_H
#define INTERMEDIATE_UART_H

#include "sys_config.h"
#include <stdint.h>
#include <stdbool.h>

#define UART_QUEUE_MAX_SIZE		80


typedef struct
{
	uint8_t byHead;
	uint8_t byRear;
	uint8_t byBuff[UART_QUEUE_MAX_SIZE];
}UART_QUEUE_t;


bool de_queue(uint8_t *pData);

void serial1_put_char(uint8_t byData);
void serial2_put_char(uint8_t byData);

#endif
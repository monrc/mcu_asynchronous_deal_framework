#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stdbool.h>
#include "sys_config.h"

#define STACK_SIZE(x)	(x + STACK_COST_SIZE)

typedef struct
{
	uint8_t wOrder;
	uint8_t byData;
	uint8_t byNext;
}Node;


enum
{
	STACK_CAPACITY = 0,
	STACK_POINT = 1,
	STACK_BUTTON = 1,
	STACK_COST_SIZE = 2,
};




#endif


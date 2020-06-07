#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "task_config.h"

#define TASK_QUEUE_MAX_SIZE		TASK_QUEUE_SIZE
#define TASK_TYPE_TIMER			1
#define TASK_TYPE_TASK			2


typedef struct
{
	union
	{
		struct
		{
			uint8_t byRepeat;		//定时器重复次数
			uint8_t byPriority;		//任务优先级
			uint16_t wPeirod;		//定时器周期
		};
		uint32_t dwVal;
	};
	
	FUNC func;			//回调函数
	uint8_t byID;		//定时器、任务ID号
	uint8_t byType;		//任务类型
	
	uint8_t res[2];
}Queue_element_t;


typedef struct
{
	uint8_t byFront;	//队首
	uint8_t byRear;		//队尾
	uint8_t res[2];		//预留对齐
	Queue_element_t List[TASK_QUEUE_MAX_SIZE];	//队列缓冲池
}Task_queue_t;



void manager_init(void);
bool manager_enque_isr(Queue_element_t *pTask);
void manager_scan(void);

void task_enter_critical(void);
void task_exit_critical(void);

#endif
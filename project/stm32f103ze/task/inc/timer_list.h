#ifndef TIMER_TASK_H
#define TIMER_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"

//最大支持的定时器任务数量
#define TIMER_MAX_SIZE		TIMER_MAX_NUM
#define TIMER_MEMORY_SIZE	(TIMER_MAX_SIZE + 1)


//根据repeat的值为0XFF，标志为一直存在的任务
#define TIMER_CONITNUOUS_PERIOD 0xFF
#define TIMER_CONTINUOUS_VAL	(0xff - 1)

#define MAX_PERIOD_VAL		0xFFFF
#define MAX_TIMER_ID		0xFF
#define END_GUARD_INDEX		TIMER_MAX_NUM
typedef struct
{
	FUNC call_back;			//定时器回调
	uint16_t tick;			//定时器值
	uint16_t period;		//任务周期
	uint8_t repeat;			//重复次数
	uint8_t task_id;		//任务标志号
	uint8_t next;			//下一个定时器任务位置
	//uint8_t res[1];
}Timer_node_t;

typedef struct
{
	Timer_node_t list[TIMER_MEMORY_SIZE];	//定时器任务表
	bool mem_flag[TIMER_MAX_SIZE];			//任务事件表内存中空闲标志位
	bool id_flag[TIMER_MAX_SIZE];			//唯一定时器
	uint8_t mem_index;						//内存记录符号，若当前的内存空闲，则下一个内存大概率也是空闲
	uint8_t size;							//任务事件表中事件的个数
	uint8_t head;							//任务事件表首元素
}Timer_list_t;



void timer_list_init(void);
void timer_list_scan(void);
void timer_list_push(Timer_node_t *pNode);
bool timer_list_empty(void);
#endif
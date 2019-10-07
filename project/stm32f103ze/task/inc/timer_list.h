#ifndef TIMER_TASK_H
#define TIMER_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"

//最大支持的定时器任务数量
#define TIMER_MAX_SIZE		TIMER_MAX_NUM
#define TIMER_MEMORY_SIZE	(TIMER_MAX_SIZE + 1)
#define MAX_PERIOD			0xFFFF
#define MAX_TIMER_ID		0xFF
#define MAX_REPEAT			0xFF


#define TIMER_MAX_UNIQUE_ID		32
//根据byRepeat的值为0XFF，标志为一直存在的任务
#define TIMER_CONITNUOUS_REPEAT MAX_REPEAT
#define TIMER_CONTINUOUS_VAL	(MAX_REPEAT - 1)

//守卫相关
#define TIMER_GUARTD_INDEX		TIMER_MAX_NUM
#define TIMER_GUARTD_ID			MAX_TIMER_ID
#define TIMER_GUARTD_PERIOD		MAX_PERIOD
#define TIMER_GUARTD_REPEAT		MAX_REPEAT

#if (1 == TIMER_DEBUG_ON)
	#define TIMER_PRINT(x)	printf x
#else
	#define TIMER_PRINT(x)
#endif

typedef struct
{
	FUNC CallBack;			//定时器回调
	uint16_t wTick;			//定时器值
	uint16_t wPeriod;		//任务周期
	uint8_t byRepeat;			//重复次数
	uint8_t byID;		//任务标志号
	uint8_t byNext;			//下一个定时器任务位置
	uint8_t res[1];
}Timer_node_t;

typedef struct
{
	Timer_node_t List[TIMER_MEMORY_SIZE];	//定时器任务表
	bool bMemFlag[TIMER_MAX_SIZE];			//任务事件表内存中空闲标志位
	bool bIDFlag[TIMER_MAX_SIZE];			//唯一定时器标志位
	uint8_t byMemIndex;						//内存记录符号，若当前的内存空闲，则下一个内存大概率也是空闲
	uint8_t byNum;							//任务事件表中事件的个数
	uint8_t byHead;							//任务事件表首元素
	uint8_t byMutex;							//互斥信号量
}Timer_list_t;



void timer_list_init(void);
void timer_list_pop(void);
void timer_list_push(Timer_node_t *pNode);
bool timer_list_empty(void);
void timer_list_delete(uint8_t byID);
void timer_list_print(void);

#endif
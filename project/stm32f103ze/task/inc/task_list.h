#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"

#define TASK_MAX_SIZE		TASK_MAX_NUM
#define TASK_MEMORY_SIZE	(TASK_MAX_SIZE + 1)


typedef struct
{
	FUNC call_back;		//任务函数
	uint8_t priority;	//任务优先级   1 -> 32    低优先级 -> 高优先级
	uint8_t next;		//下一个任务位置
	uint8_t res[2];		//预留对齐
}Task_node_t;

typedef struct 
{
	Task_node_t list[TASK_MEMORY_SIZE];	//任务事件表
	bool mem_flag[TASK_MAX_SIZE];	//任务事件表内存中空闲标志位
	uint8_t mem_index;				//内存记录符号，若当前的内存空闲，则下一个内存大概率也是空闲
	uint8_t size;					//任务事件表中事件的个数
	uint8_t head;					//任务事件表首元素
}Task_list_t;

typedef struct
{
	void (*init)(void);
	void (*priority_increase)(void);
	void (*push)(Task_node_t *ptask);
	FUNC (*pop)(void);
	void (*print)(void);
}Task_t;

void task_list_init(void);
void task_list_priority_increase(void);
FUNC task_list_pop(void);
void task_list_push(Task_node_t *ptask);
void task_list_print(void);


#endif
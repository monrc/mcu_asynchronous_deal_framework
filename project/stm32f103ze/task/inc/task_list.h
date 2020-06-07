#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"


#define TASK_MAX_SIZE		TASK_MAX_NUM		//最大的缓冲任务数量
#define TASK_MEMORY_SIZE	(TASK_MAX_SIZE + 1)	//申请内存个数
#define MAX_PRIORITY		0xFF				//最高优先级
#define MAX_TASK_ID			0xFF				//最大任务ID
#define TASK_MAX_UNIQUE_ID	32					//唯一ID范围


//守卫相关
#define TASK_GUARTD_INDEX		TASK_MAX_SIZE
#define TASK_GUARTD_ID			MAX_TASK_ID
#define TASK_GUARTD_PRIORITY	0

#if (1 == TASK_DEBUG_ON)
	#define TASK_PRINT(x)	printf x
#else
	#define TASK_PRINT(x)
#endif



typedef struct
{
	FUNC CallBack;		//任务函数
	uint8_t byPriority;	//任务优先级   1 -> 32    低优先级 -> 高优先级
	uint8_t byID;		//任务ID
	uint8_t byNext;		//下一个任务位置
	uint8_t res[1];		//预留对齐
}Task_node_t;

typedef struct 
{
	Task_node_t List[TASK_MEMORY_SIZE];	//任务事件表
	bool bMemFlag[TASK_MAX_SIZE];	//任务事件表内存中空闲标志位
	bool bIDFlag[TASK_MAX_UNIQUE_ID];	//唯一任务标志位
	uint8_t byMemIndex;				//内存记录符号，若当前的内存空闲，则下一个内存大概率也是空闲
	uint8_t byNum;					//任务事件表中事件的个数
	uint8_t byHead;					//任务事件表首元素
}Task_list_t;

typedef struct
{
	void (*init)(void);
	void (*priority_increase)(void);
	void (*push)(Task_node_t *pTask);
	FUNC (*pop)(void);
	void (*print)(void);
}Task_t;


void task_list_init(void);
bool task_list_pop(void);
bool task_list_push(Task_node_t *pTask, bool bReplace);
void task_list_delete(uint8_t byID);
void task_list_print(void);

#endif


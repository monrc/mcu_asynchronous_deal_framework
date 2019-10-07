/********************************************************
*file name: task_list.c
*function: 实现任务的管理，提供添加、删除定时器的接口
*实现：链表采用数组的方式实现，无头部指针，但是存在尾部标兵，
	   链表是一个有序链表，依据优先级，优先级高的在首部，
	   任务优先级相同，先添加的任务先执行
	   任务ID 0~31 在任务链表中是唯一存在的，不存在两个相同的ID任务
	   任务冲突，若任务已经存在造成ID冲突，则新的相同任务再添加
	   不同的任务可以共享的任务ID号为 32~254
	   任务ID号最大值为0xFF被链表守卫占用，故ID有效值为0~0xFE
	   任务优先级值为1~0xFF，数值大的优先级高，优先级0的被守卫占用
	   
*note: 任务链表的删除与添加不适合在中断中执行，如有必要，需任务管理器中转
********************************************************/
#include <string.h>
#include "task_list.h"
#include "debug.h"
#include "task_manager.h"

//模块内私有变量定义
static volatile Task_list_t stTask;

//模块内私有函数声明
static bool task_list_add(Task_node_t *pTask);
static void task_list_priority_increase(void);

/*********************************************************/
//name: task_list_init
//function: 任务链表初始化
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
void task_list_init(void)
{
	memset((uint8_t *)&stTask, 0, sizeof(stTask));
	
	//链表尾部标兵
	stTask.List[TASK_GUARTD_INDEX].byID = TASK_GUARTD_ID;
	stTask.List[TASK_GUARTD_INDEX].byPriority = TASK_GUARTD_PRIORITY; //最低优先级
	stTask.List[TASK_GUARTD_INDEX].CallBack = NULL;
	stTask.List[TASK_GUARTD_INDEX].byNext = TASK_GUARTD_INDEX;
	
	stTask.byHead = TASK_GUARTD_INDEX;
}

#if (1 == USE_PRIORITY_INCREASE_WITH_TIME)
/*********************************************************/
//name: task_list_priority_increase
//function: 任务优先级随时间逐渐增加
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
static void task_list_priority_increase(void)
{
	uint8_t i;
	uint8_t byHead = stTask.byHead;
	for(i = 0; i < stTask.byNum; i++)
	{
		stTask.List[byHead].byPriority++;
		byHead = stTask.List[byHead].byNext;
	}
}
#endif

/*********************************************************/
//name: task_list_pop
//function: 任务函数出链表，并执行任务函数
//input: None
//output: None
//return: true 函数执行完毕    false 链表为空
//note: None
/*********************************************************/
bool task_list_pop(void)
{
	if(stTask.byNum)
	{
		printf("%u", stTask.List[stTask.byHead].byID);
		stTask.List[stTask.byHead].CallBack();
		
		stTask.bMemFlag[stTask.byHead] = false;
		if(stTask.List[stTask.byHead].byID < TASK_MAX_UNIQUE_ID)
		{
			stTask.bIDFlag[stTask.List[stTask.byHead].byID] = false;
		}
		
		stTask.byHead = stTask.List[stTask.byHead].byNext;
		stTask.byNum--;
#if (1 == USE_PRIORITY_INCREASE_WITH_TIME)		
		task_list_priority_increase();
#endif		
		return true;
	}
	return false;
}

/*********************************************************/
//name: task_list_add
//function: 把任务添加至任务链表
//input: Task_node_t *pTask 任务结构体指针
//output: None
//return: true 添加成功		false 添加失败
//note: None
/*********************************************************/
static bool task_list_add(Task_node_t *pTask)
{
	uint8_t index = stTask.byMemIndex;
	
	volatile uint8_t *pPre = &stTask.byHead;
	uint8_t byHead = stTask.byHead;
	uint8_t byNext = stTask.byHead;
	
	TASK_ERROR("system pTask memeroy is full", TASK_MAX_SIZE > stTask.byNum, return false);
	TASK_ERROR("pointer is null", NULL != pTask, return false);
	
	//查找空闲的内存块
	while(true == stTask.bMemFlag[index])
	{
		index = (index + 1) % TASK_MAX_SIZE;
		
	}
	//复制任务内存块
	memcpy((uint8_t *)&stTask.List[index], pTask, sizeof(Task_node_t));
	
	//依据优先级查找对应的插入位置
	while(stTask.List[byNext].byPriority >= pTask->byPriority)
	{
		byHead = byNext;
		byNext = stTask.List[byNext].byNext;
	}
	//插入位置不在首部
	if(stTask.byHead != byNext)
	{
		pPre = &stTask.List[byHead].byNext;
	}
	
	//将任务添加至链表中
	*pPre = index;
	stTask.List[index].byNext = byNext;
	
	//更改任务状态字
	stTask.byNum++;
	stTask.bMemFlag[index] = true;
	if(pTask->byID < TASK_MAX_UNIQUE_ID)
	{
		stTask.bIDFlag[pTask->byID] = true;
	}
	stTask.byMemIndex = (index + 1) % TASK_MAX_SIZE;  //更新内存块标记
	return true;
}

/*********************************************************/
//name: task_list_push
//function: 把任务添加至任务链表  若任务已经存在，则忽略此次任务添加
//input:
//output:
//return: true 添加成功 	false 添加失败
//note:
/*********************************************************/
bool task_list_push(Task_node_t *pTask)
{
	TASK_ERROR("pointer is null", NULL != pTask, return false);
	
	if(0 == pTask->byPriority)
	{
		TASK_PRINT(("task_priority_is_zero\r\n"));
		return false;
	}
	
	if (pTask->byID < TASK_MAX_UNIQUE_ID)
	{
		if(true == stTask.bIDFlag[pTask->byID])
		{
			TASK_PRINT(("task_id:%u already exists\r\n", pTask->byID));
			return false;
		}
	}
	
	return task_list_add(pTask);
}

/*********************************************************/
//name: task_list_delete
//function: 删除所有ID为 byDeletID的任务
//input: uint8_t byDeletID 任务ID号
//output: None
//return: None
//note: 
/*********************************************************/
void task_list_delete(uint8_t byDeletID)
{
	volatile uint8_t *pre = &stTask.byHead;
	uint8_t byNext = stTask.byHead;
	uint8_t flag;
	uint8_t byTempID;
	
	while (stTask.List[byNext].byID != TASK_GUARTD_ID)
	{
		//尾部标兵设置
		stTask.List[TASK_GUARTD_INDEX].byID = byDeletID;

		while (stTask.List[byNext].byID != byDeletID)
		{
			pre = &stTask.List[byNext].byNext;
			byNext = stTask.List[byNext].byNext;
		}
		//非标兵，删除任务
		if (byNext != TASK_GUARTD_INDEX)
		{
			//清除ID标志位
			if(byDeletID < TASK_MAX_UNIQUE_ID)
			{
				stTask.bIDFlag[byDeletID] = false;				
			}

			//删除任务
			*pre = stTask.List[byNext].byNext;
			stTask.bMemFlag[byNext] = false;
			stTask.byNum--;
			byNext = stTask.List[byNext].byNext;
		}
		//恢复标兵参数
		stTask.List[TASK_GUARTD_INDEX].byID = TASK_GUARTD_ID;
	}
}
//----------------------------任务调试支持--------------------------------------------------
#if (1 == TASK_DEBUG_ON)
/*********************************************************/
//name: task_list_print
//function: 打印任务链表
//input:
//output:
//return:
//note:
/*********************************************************/
void task_list_print(void)
{
	uint8_t byNext = stTask.byHead;
	TASK_PRINT(("\r\n---- pTask list num %u ---\r\n", stTask.byNum));
	TASK_PRINT(("ID   Priority   FUN_ADDR\r\n"));
	
	while (TASK_GUARTD_INDEX != byNext)
	{
		TASK_PRINT(("%-5u", stTask.List[byNext].byID));
		TASK_PRINT(("%-11u", stTask.List[byNext].byPriority));
		TASK_PRINT(("0x%08x\r\n", (uint32_t)stTask.List[byNext].CallBack));
		byNext = stTask.List[byNext].byNext;
	}
}

#else
void task_list_print(void) {}

#endif











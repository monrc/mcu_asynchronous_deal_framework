/********************************************************
*file name: task_manager.c
*function:  主要用来对中断函数中添加任务进行管理，用于实现临界资源的互斥访问
			临界资源主要是指定时器任务链表，优先级任务链表，以下统称：任务链表
			如果需要在中断函数中需要对任务链表进行更改，功能实现为A函数，则先把函数A添加至任务队列中，
			然后在后台程序中执行A函数实现对任务链表的更改。
*实现：		数据结构采用队列的方式实现，入队列函数在中断中调用，出队函数在后台程序中调用，将任务转存至任务链表中
			同过这种转储的方式实现临界资源的互斥访问
			
*note: 		任务队列作用：管理中断函数中定时器任务或者普通任务的添加删除
			保护临界段资源
********************************************************/
#include <stdio.h>
#include "task_manager.h"
#include "timer_list.h"
#include "task_list.h"
#include "terminal.h"

//内部变量定义
static volatile Task_queue_t stQueue;	//任务队列
volatile uint8_t critical_nesting = 0;	//临界区变量

//内部函数声明
static bool manager_deque(Queue_element_t *pTask);
static void manager_queue_init(void);

/*********************************************************/
//name: manager_init
//function: 任务管理器初始化
//input: none
//output: none
//return: none
/*********************************************************/
void manager_init(void)
{
	timer_list_init();
	task_list_init();
	terminal_init();
}

/*********************************************************/
//name: manager_queue_init
//function: 任务队列初始化
//input: none
//output: none
//return: none
/*********************************************************/
static void manager_queue_init(void)
{
	memset((uint8_t *)&stQueue, 0, sizeof(Task_queue_t));
}

/*********************************************************/
//name: manager_enque_isr
//function: 任务入队列，在中断中使用
//input:
//output:
//return: 
//note:
/*********************************************************/
bool manager_enque_isr(Queue_element_t *pTask)
{
	uint8_t byNewRear = (stQueue.byRear + 1) % TASK_QUEUE_MAX_SIZE;
	if(byNewRear == stQueue.byFront)
	{
		return false;
	}
	
	stQueue.List[stQueue.byRear].dwVal = pTask->dwVal;
	stQueue.List[stQueue.byRear].func = pTask->func;
	stQueue.List[stQueue.byRear].byType = pTask->byType;
	stQueue.List[stQueue.byRear].byID = pTask->byID;
	
	stQueue.byRear = byNewRear;
	return true;
}

/*********************************************************/
//name: manager_deque
//function: 任务出队列
//input: None
//output: Queue_element_t *pTask 队列任务指针
//return: true 出队成功   false 出队失败
//note: 
/*********************************************************/
static bool manager_deque(Queue_element_t *pTask)
{
	if(stQueue.byRear == stQueue.byFront)
	{
		return false;
	}
	
	pTask->dwVal = stQueue.List[stQueue.byFront].dwVal;
	pTask->func = stQueue.List[stQueue.byFront].func;
	pTask->byType = stQueue.List[stQueue.byFront].byType;
	pTask->byID = stQueue.List[stQueue.byFront].byID;
	
	stQueue.byFront = (stQueue.byFront + 1) % TASK_QUEUE_MAX_SIZE;
	return true;
}


/*********************************************************/
//name: manager_scan
//function: 任务入队列，在后台使用,将队列中的任务添加至对应的链表中
//input: None
//output: None
//return: None
//note: 需要在主循环中运行
/*********************************************************/
void manager_scan(void)
{
	Queue_element_t que_task;
	Task_node_t pTask;
	Timer_node_t timer;
	while(manager_deque(&que_task))
	{
		if(que_task.byType == TASK_TYPE_TIMER)
		{
			timer.CallBack = que_task.func;
			timer.wPeriod = que_task.wPeirod;
			timer.byRepeat = que_task.byRepeat;
			timer.byID = que_task.byID;
			timer_list_push(&timer);
		}
		else
		{
			pTask.CallBack = que_task.func;
			pTask.byPriority = que_task.byPriority;
			pTask.byID = que_task.byID;
			task_list_push(&pTask);
		}
	}
}


/*********************************************************/
//name: task_enter_critical
//function: 进入临界段
//input: None
//output: None
//return: None
//note: 与task_exit_critical()函数成对使用 
/*********************************************************/
void task_enter_critical(void)
{
	DISABLE_INT();
	critical_nesting++;
}

/*********************************************************/
//name: task_exit_critical
//function: 退出临界段
//input: None
//output: None
//return: None
//note: 与task_enter_critical()函数成对使用 
/*********************************************************/
void task_exit_critical(void)
{
	critical_nesting--;
	if(0 == critical_nesting)
	{
		ENABLE_INT();
	}
}

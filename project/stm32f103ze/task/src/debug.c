#include "intermediate_time.h"
#include "debug.h"
#include "task_manager.h"
#include "task_list.h"
#include "timer_list.h"

void test(void)
{
	printf(" ");
}

void timer_test(uint8_t size)
{
	Queue_element_t stTask;
	Timer_node_t stTimer;
	uint8_t i;
	if (timer_list_empty())
	{
		printf("\r\n");
		srand(get_tick());
		for (i = 0; i < size; i++)
		{
			stTimer.byID = rand() % 100;
			stTimer.byRepeat = 1;
			stTimer.CallBack = test;
			stTimer.wPeriod = stTimer.byID * 10;
			//timer_list_push(&stTimer);
			
			stTask.byID = rand() % 100;
			stTask.wPeirod = stTask.byID * 10;
			stTask.byRepeat = 1;
			stTask.byType = TASK_TYPE_TIMER;
			stTask.func = test;	
			manager_enque_isr(&stTask);
		}
	}
}

void task_test(uint8_t size)
{
	Queue_element_t stTask;
	uint8_t i;
	if (timer_list_empty())
	{
		printf("\r\n");
		srand(get_tick());
		for (i = 0; i < size; i++)
		{
			stTask.byID = rand() % 100;
			stTask.byPriority = stTask.byID;
			stTask.byType = TASK_TYPE_TASK;
			stTask.func = test;
			
			manager_enque_isr(&stTask);
		}
	}
}

void task_print1(void)
{
	printf("task1_exec_complete\r\n");
}

void task_print2(void)
{
	printf("task2_exec_complete\r\n");
}

void add_task1(void)
{
	Queue_element_t stTask;
	stTask.byID = 2;
	stTask.byPriority = 2;
	stTask.byType = TASK_TYPE_TASK;
	stTask.func = task_print1;
	
	manager_enque_isr(&stTask);
}

void add_task2(void)
{
	Queue_element_t stTask;
	
	stTask.byID = 2;
	stTask.byPriority = 2;
	stTask.byType = TASK_TYPE_TASK;
	stTask.func = task_print1;
	
	manager_enque_isr(&stTask);
	
	stTask.byID = 3;
	stTask.byPriority = 3;
	stTask.byType = TASK_TYPE_TASK;
	stTask.func = task_print2;
	
	manager_enque_isr(&stTask);
}
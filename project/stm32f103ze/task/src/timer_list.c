/********************************************************
*file name: timer_list.c
*function: 实现定时器任务的管理，提供添加、删除任务的接口
*实现：链表采用数组的方式实现，无头部指针，但是存在尾部标兵，
	   链表是一个有序链表，依据时间，优先执行的在首部，
	   若某一时刻，两个定时器任务同时时间到，则优先执行先添加的任务
	   定时器最高分辨率为1ms，最低分辨率为timer_list_scan执行的频率
	   定时器任务的执行是：先延迟，后执行

*note: timer_list_scan()函数需要运行在后台，运行需要运行的定时器任务
********************************************************/
#include <string.h>
#include "timer_list.h"
#include "intermediate_time.h"
#include "debug.h"

static void timer_list_add(Timer_node_t *pNode, uint8_t mem_index);
void timer_list_print(void);

static Timer_list_t stTimer;

/*********************************************************/
//name: timer_task_init
//function: 定时器任务链表初始化
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_init(void)
{
	memset((uint8_t *)&stTimer, 0, sizeof(stTimer));

	//链表尾部标兵
	stTimer.list[END_GUARD_INDEX].task_id = MAX_TIMER_ID;
	stTimer.list[END_GUARD_INDEX].tick = 0;
	stTimer.list[END_GUARD_INDEX].period = MAX_PERIOD_VAL;
	stTimer.list[END_GUARD_INDEX].call_back = NULL;
	stTimer.head = END_GUARD_INDEX;
}

/*********************************************************/
//name: timer_list_scan
//function: 定时器任务链表扫描函数，在mian函数while(1)中调用
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_scan(void)
{
	FUNC fun = NULL;
	uint8_t head = stTimer.head;
	uint8_t next;
	uint8_t period;

	uint8_t exec_num = 0;
	uint8_t exec_index[TIMER_MAX_SIZE] = {0};
	uint8_t exec_id;
	uint8_t i;

	uint16_t tick = get_timer_tick();
	if (0 == stTimer.size)
		return;
	//队尾标兵
	stTimer.list[TIMER_MAX_SIZE].tick = tick;
	while ((uint16_t)(tick - stTimer.list[head].tick) > stTimer.list[head].period)
	{
		if (NULL != stTimer.list[head].call_back)
		{
			printf("%02u", stTimer.list[head].task_id);
			stTimer.list[head].call_back();
		}

		if (--stTimer.list[head].repeat)
		{
			exec_index[exec_num++] = head;//暂存缓冲区
		}
		else
		{
			exec_id = stTimer.list[head].task_id < TIMER_MAX_SIZE ? stTimer.list[head].task_id : TIMER_MAX_SIZE;
			stTimer.id_flag[exec_id] = false;
			stTimer.mem_flag[exec_index[i]] = false; //清除内存块标记
		}
		stTimer.size--;
		next = stTimer.list[head].next;//下一个循环扫描判断
		head = next;
	}
	stTimer.head = head;

	for (i = 0; i < exec_num; i++)
	{
		//若定时器任务是一直存在的，则恢复repeat的值为0xFF
		stTimer.list[exec_index[i]].repeat += (TIMER_CONTINUOUS_VAL == stTimer.list[exec_index[i]].repeat);
		timer_list_add(NULL, exec_index[i]);
		timer_list_print();
	}
}

/*********************************************************/
//name: timer_list_push
//function: 将定时器任务添加至链表当中
//input:
//output:
//return:
//note:若两个任务在将来的某个时刻，同时到时间，则先添加的任务，优先执行
/*********************************************************/
static void timer_list_add(Timer_node_t *pNode, uint8_t mem_index)
{
	uint8_t index = stTimer.mem_index;

	uint8_t *pPre = &stTimer.head;
	uint8_t head = stTimer.head;
	uint8_t next = stTimer.head;

	TASK_ERROR("timer task memeroy is full", (TIMER_MAX_SIZE > stTimer.size), return;);

	if (stTimer.size >=  TIMER_MAX_SIZE)
		return;

	uint16_t tick = get_timer_tick();//获得当前的定时器的值
	uint16_t dia_tick;

	//内存块中不存在该定时器任务,需要申请内存
	if (TIMER_MAX_SIZE == mem_index)
	{
		//查找空闲的内存块
		while (true == stTimer.mem_flag[index])
		{
			index = (index + 1) % TIMER_MAX_SIZE;
		}
		//复制定时器任务内存块
		memcpy(&stTimer.list[index], pNode, sizeof(Timer_node_t));
		mem_index = index;

		//更新内存块标记
		stTimer.mem_flag[index] = true;
		stTimer.mem_index = (index + 1) % TIMER_MAX_SIZE;
	}
	//保存当前的tick值
	stTimer.list[mem_index].tick = tick;

	if (stTimer.size)
	{
		//尾部标兵
		stTimer.list[TIMER_MAX_SIZE].tick = tick;

		dia_tick = tick - stTimer.list[next].tick;
		//过滤掉时间已到，但是还未来得及处理的任务
		while (stTimer.list[next].period <= dia_tick)
		{
			head = next;
			next = stTimer.list[next].next;
			dia_tick = tick - stTimer.list[next].tick;
		}
		//查找插入位置
		while (stTimer.list[mem_index].period >= (uint16_t)(stTimer.list[next].period - dia_tick))
		{
			head = next;
			next = stTimer.list[next].next;
			dia_tick = tick - stTimer.list[next].tick;
		}

		//插入位置不在头部
		if (stTimer.head != next)
		{
			pPre = &stTimer.list[head].next;
		}
	}

	*pPre = mem_index;
	stTimer.list[mem_index].next = next;
	stTimer.size++;
}

/*********************************************************/
//name: timer_list_push
//function: 定时器任务链表添加任务
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_push(Timer_node_t *pNode)
{
	TASK_ASSERT("pointer is null", NULL != pNode);
	TASK_ERROR("pointer is null", NULL != pNode, return;);
	timer_list_add(pNode, TIMER_MAX_SIZE);
}

/*********************************************************/
//name: timer_list_delete
//function: 删除所有ID为 task_ID的定时器任务
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_delete(uint8_t task_ID)
{
	uint8_t head = stTimer.head;
	uint8_t next = head;
	uint8_t flag;
	while (stTimer.list[next].task_id != MAX_TIMER_ID)
	{
		//尾部标兵设置
		stTimer.list[END_GUARD_INDEX].task_id = task_ID;

		while (stTimer.list[next].task_id != task_ID)
		{
			head = next;
			next = stTimer.list[next].next;
		}
		//非标兵，删除任务
		if (next != MAX_TIMER_ID)
		{
			stTimer.list[head].next = stTimer.list[next].next;
			stTimer.mem_flag[next] = false;
			stTimer.size--;

			next = stTimer.list[next].next;
		}
		//恢复标兵参数
		stTimer.list[END_GUARD_INDEX].task_id = MAX_TIMER_ID;
	}
}
/*********************************************************/
//name: timer_list_push
//function: 定时器任务链表添加任务
//input:
//output:
//return:
//note:
/*********************************************************/
bool timer_list_empty(void)
{
	return !stTimer.size;
}


//----------------------------定时器任务调试支持--------------------------------------------------
#if (1 == TIMER_TASK_DEBUG_ON)
/*********************************************************/
//name: timer_list_print
//function: 定时器任务链表添加任务
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_print(void)
{
	uint8_t next = stTimer.head;
	printf("\r\n----timer task list size %u ---\r\n", stTimer.size);
	while (stTimer.list[next].period != MAX_PERIOD_VAL)
	{
		printf("ID:%2u ", stTimer.list[next].task_id);
		printf("repeat:%2u ", stTimer.list[next].repeat);
		printf("period:%2u ", stTimer.list[next].period);
		printf("tick:%2u \r\n", stTimer.list[next].tick);
		next = stTimer.list[next].next;
	}
}

#else
void timer_list_print(void) {}

#endif

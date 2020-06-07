/********************************************************
*file name: timer_list.c
*function: 实现软件定时器任务的管理，提供添加、删除定时器的接口
*实现：链表采用数组的方式实现，无头部指针，但是存在尾部标兵，
	   链表是一个有序链表，依据时间，优先执行的在首部，
	   若某一时刻，两个定时器任务同时时间到，则优先执行先添加的任务
	   定时器最高分辨率为1ms，最低分辨率为timer_list_pop执行的频率
	   定时器的执行是：先延迟，后执行
	   定时器ID 0~31 在任务链表中是唯一存在的，不存在两个相同的ID任务
	   定时器冲突，若定时器已经存在造成ID冲突，则把相同的任务删除后再添加
	   定时器ID 最大值为0xFF被链表守卫占用，故ID有效值为0~0xFE

*note: timer_list_pop()
						1.在前台运行
							不需要考虑线程安全问题，定时器函数规模大小不受限制
							但是定时器函数响应时间得不到保证，响应时间受系统规模影响较大，
						2.在后台运行（采用此方案）
							定时器函数在中断中执行，函数规模不宜过大
							需要考虑线程安全问题，程序相对复杂
							定时器响应时间得到保证，受系统规模影响小
		timer_list_pop()函数可以在中断中调用，其他的函数不应该在中断中调用，
		如果需要，可以先把任务添加至任务队列当中，然后在后台程序中把任务添加至链表中
		若在中断中调用，需要考虑线程安全问题
********************************************************/
#include <string.h>
#include "timer_list.h"
#include "intermediate_time.h"
#include "task_manager.h"
#include "debug.h"


//模块内私有变量定义
static volatile Timer_list_t stTimer;

//模块内私有函数声明
static void timer_list_add(Timer_node_t *pNode, uint8_t byMemIndex);

/*********************************************************
* Name		: timer_task_init
* Function	: 定时器任务链表初始化
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void timer_list_init(void)
{
	memset((uint8_t *)&stTimer, 0, sizeof(stTimer));

	//链表尾部标兵
	stTimer.List[TIMER_GUARTD_INDEX].byID = TIMER_GUARTD_ID;
	stTimer.List[TIMER_GUARTD_INDEX].wTick = 0;
	stTimer.List[TIMER_GUARTD_INDEX].wPeriod = TIMER_GUARTD_PERIOD;
	stTimer.List[TIMER_GUARTD_INDEX].byRepeat = TIMER_GUARTD_REPEAT;
	stTimer.List[TIMER_GUARTD_INDEX].CallBack = NULL;	
	stTimer.byHead = TIMER_GUARTD_INDEX;
}

/*********************************************************
* Name		: timer_list_pop
* Function	: 软件定时器链表扫描函数，在定时器中断函数中调用
* Input		: None
* Output	: None
* Return	: None
* Note		: 由于在中断中执行，定时器任务函数规模应尽可能简短
*********************************************************/
void timer_list_pop(void)
{
	uint8_t byHead = stTimer.byHead;

	uint8_t byExecNum = 0;
	uint8_t byExecIndex[TIMER_MAX_SIZE] = {0};
	uint8_t i;

	uint16_t wTick;

	if (0 == stTimer.byNum)
	{
		return;
	}
	//中断打断了定时器任务，则退出，主动退让，保证定时器链表数据安全
	if (stTimer.byMutex) 
	{
		return;
	}

	//队尾标兵
	wTick = get_timer_tick();
	stTimer.List[TIMER_MAX_SIZE].wTick = wTick;
	while ((uint16_t)(wTick - stTimer.List[byHead].wTick) > stTimer.List[byHead].wPeriod)
	{
		if (NULL != stTimer.List[byHead].CallBack)
		{
			printf("%-2u", stTimer.List[byHead].byID);
			stTimer.List[byHead].CallBack();
		}

		if (--stTimer.List[byHead].byRepeat)
		{
			byExecIndex[byExecNum++] = byHead;//暂存缓冲区
		}
		else
		{
			if (stTimer.List[byHead].byID < TIMER_MAX_UNIQUE_ID)
			{
				stTimer.bIDFlag[stTimer.List[byHead].byID] = false;
			}
			stTimer.bMemFlag[byHead] = false; //清除内存块标记
		}
		stTimer.byNum--;
		byHead = stTimer.List[byHead].byNext;//下一个循环扫描判断
	}
	stTimer.byHead = byHead;

	for (i = 0; i < byExecNum; i++)
	{
		//若定时器任务是一直存在的，则恢复byRepeat的值为0xFF
		stTimer.List[byExecIndex[i]].byRepeat += (TIMER_CONTINUOUS_VAL == stTimer.List[byExecIndex[i]].byRepeat);
		timer_list_add(NULL, byExecIndex[i]);
		timer_list_print();
	}
}

/*********************************************************
* Name		: timer_list_add
* Function	: 将软件定时器添加至链表当中
* Input		: Timer_node_t *pNode	定时器属性指针
			  uint8_t byMemIndex		在内存中的位置
* Output	: None
* Return	: None
* Note		: 若两个定时器任务在将来的某个时刻，同时到时间，则先添加的任务，优先执行
*********************************************************/
static void timer_list_add(Timer_node_t *pNode, uint8_t byMemIndex)
{
	uint8_t index = stTimer.byMemIndex;
	volatile uint8_t *pPre = &stTimer.byHead;
	uint8_t byHead = stTimer.byHead;
	uint8_t byNext = stTimer.byHead;

	ERROR("timer pTask memeroy is full", (TIMER_MAX_SIZE > stTimer.byNum), return;);

	if (stTimer.byNum >=  TIMER_MAX_SIZE)
		return;

	uint16_t wTick = get_timer_tick();//获得当前的定时器的值
	uint16_t dia_tick;

	//内存块中不存在该定时器任务,需要申请内存
	if (TIMER_GUARTD_INDEX == byMemIndex)
	{
		//查找空闲的内存块
		while (true == stTimer.bMemFlag[index])
		{
			index = (index + 1) % TIMER_MAX_SIZE;
		}
		//复制定时器任务内存块
		memcpy((uint8_t *)&stTimer.List[index], pNode, sizeof(Timer_node_t));
		byMemIndex = index;

		//更新内存块标记
		stTimer.bMemFlag[index] = true;
		stTimer.byMemIndex = (index + 1) % TIMER_MAX_SIZE;
		if(pNode->byID < TIMER_MAX_UNIQUE_ID)
		{
			stTimer.bIDFlag[pNode->byID] = true;
		}
	}	
	stTimer.List[byMemIndex].wTick = wTick;	//保存当前的wTick值
	
	
	stTimer.List[TIMER_MAX_SIZE].wTick = wTick;		//更新尾部标兵时间
	dia_tick = wTick - stTimer.List[byNext].wTick;	//计算链表首部时间差
	//过滤掉时间已到，但是还未来得及处理的任务
	while (stTimer.List[byNext].wPeriod <= dia_tick)
	{
		byHead = byNext;
		byNext = stTimer.List[byNext].byNext;
		dia_tick = wTick - stTimer.List[byNext].wTick;
	}
	//查找插入位置
	while (stTimer.List[byMemIndex].wPeriod >= (uint16_t)(stTimer.List[byNext].wPeriod - dia_tick))
	{
		byHead = byNext;
		byNext = stTimer.List[byNext].byNext;
		dia_tick = wTick - stTimer.List[byNext].wTick;
	}
	//插入位置不在头部
	if (stTimer.byHead != byNext)
	{
		pPre = &stTimer.List[byHead].byNext;
	}

	*pPre = byMemIndex;
	stTimer.List[byMemIndex].byNext = byNext;
	stTimer.byNum++;
}

/*********************************************************
* Name		: timer_list_push
* Function	: 定时器任务链表添加任务，若任务已经存在，则删除后，再添加
* Input		: Timer_node_t *pNode 任务信息指针
* Output	: None
* Return	: None
* Note		: 该函数不能在中断里面调用，如果需要在中断里面添加定时器任务，
			  可以通过先把任务添加任务至任务队列中，在后台程序中取队列添加至链表中
*********************************************************/
void timer_list_push(Timer_node_t *pNode)
{
	TASK_ASSERT("pointer is null", NULL != pNode);
	ERROR("pointer is null", NULL != pNode, return;);
		
	task_enter_critical(); 	//临界段保护互斥量
	stTimer.byMutex++;		//与timer_list_pop函数形成互斥访问stTimer资源
	task_exit_critical();
	
	if (pNode->byID < TIMER_MAX_UNIQUE_ID)
	{
		if(true == stTimer.bIDFlag[pNode->byID])
		{
			timer_list_delete(pNode->byID);
			TIMER_PRINT(("timer_id:%u already exists\r\n", pNode->byID));
		}
	}

	timer_list_add(pNode, TIMER_GUARTD_INDEX);
	
	stTimer.byMutex--; //允许中断函数执行定时器扫描函数
}

/*********************************************************
* Name		: timer_list_delete
* Function	: 删除所有ID为 byID的定时器任务
* Input		: uint8_t byDeletID 定时器任务ID号
* Output	: None
* Return	: None
* Note		: 该函数不建议在中断中调用，原因：保护临界资源（stTimer）
			  如有需求，添加至任务队列中，再转储至任务链表中，再执行
*********************************************************/
void timer_list_delete(uint8_t byDeletID)
{
	volatile uint8_t *pre = &stTimer.byHead;
	uint8_t byNext = stTimer.byHead;
	uint8_t flag;
	uint8_t byTempID;
	
	task_enter_critical(); 	//临界段保护互斥量 
	stTimer.byMutex++;		//与timer_list_pop函数形成互斥访问stTimer资源
	task_exit_critical();
	
	while (stTimer.List[byNext].byID != TIMER_GUARTD_ID)
	{
		//尾部标兵设置
		stTimer.List[TIMER_GUARTD_INDEX].byID = byDeletID;

		while (stTimer.List[byNext].byID != byDeletID)
		{
			pre = &stTimer.List[byNext].byNext;
			byNext = stTimer.List[byNext].byNext;
		}
		//非标兵，删除任务
		if (byNext != TIMER_GUARTD_INDEX)
		{
			//清除ID标志位
			if(byDeletID < TIMER_MAX_UNIQUE_ID)
			{
				stTimer.bIDFlag[byDeletID] = false;				
			}

			//删除定时器任务
			*pre = stTimer.List[byNext].byNext;
			stTimer.bMemFlag[byNext] = false;
			stTimer.byNum--;

			byNext = stTimer.List[byNext].byNext;
		}
		//恢复标兵参数
		stTimer.List[TIMER_GUARTD_INDEX].byID = TIMER_GUARTD_ID;
	}
	
	stTimer.byMutex--; //互斥信号量恢复
}

/*********************************************************
* Name		: timer_list_empty
* Function	: 查询定时器任务是否为空
* Input		: None
* Output	: None
* Return	: true 链表空		false 链表非空
* Note		: None
*********************************************************/
bool timer_list_empty(void)
{
	return !stTimer.byNum;
}


//----------------------------定时器任务调试支持--------------------------------------------------
#if (1 == TIMER_DEBUG_ON)
/*********************************************************
* Name		: timer_list_print
* Function	: 打印定时器链表
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void timer_list_print(void)
{
	uint8_t byNext = stTimer.byHead;
	TIMER_PRINT(("\r\n----timer pTask list num %u ---\r\n", stTimer.byNum));
	TIMER_PRINT(("ID   Repeat   Period   FUN_ADDR   Tick\r\n"));
	
	while (TIMER_GUARTD_INDEX != byNext)
	{
		TIMER_PRINT(("%-5u", stTimer.List[byNext].byID));
		TIMER_PRINT(("%-9u", stTimer.List[byNext].byRepeat));
		TIMER_PRINT(("%-9u", stTimer.List[byNext].wPeriod));
		TIMER_PRINT(("0x%08x ", (uint32_t)stTimer.List[byNext].CallBack));
		TIMER_PRINT(("%4u\r\n", stTimer.List[byNext].wTick));
		byNext = stTimer.List[byNext].byNext;
	}
}

#else

void timer_list_print(void) {}

#endif

#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

//#define PLATFORM_ASSERT
#include "sys_config.h"

#define TASK_MAX_NUM		32
#define TIMER_MAX_NUM		32
#define TASK_QUEUE_SIZE		32


#define TIMER_DEBUG_ON		1
#define TASK_DEBUG_ON		1

//-------------公用的宏定义----------------//
//硬件文件包含，提供开关中断的接口
#include <stm32f103xe.h>
typedef void (*FUNC)();

#define DISABLE_INT() 	__set_PRIMASK(1)
#define ENABLE_INT()	__set_PRIMASK(0)

//-----------------------------------------//

//#define NOASSERT

//#define NOERROR


#endif

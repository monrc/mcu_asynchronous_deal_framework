#include <stdint.h>
#include <stdbool.h>
#include "task_manager.h"
#include "timer_list.h"
#include "task_list.h"

void task_manager_init(void)
{
	timer_list_init();
	task_list_init();
}

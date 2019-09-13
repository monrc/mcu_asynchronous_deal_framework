#include "app_layer.h"
#include "intermediate_layer.h"
#include "task_manager.h"

void test(void)
{
	printf(" ");
}
extern void timer_list_print(void);
int main()
{
	Timer_node_t node;
	uint8_t timId[32];
	uint8_t id;
	
	mcu_init();
	task_manager_init();
	
	//timer_list_push(&node);
	
	led_control(YELLOW_LED, OFF);
	led_control(BULUE_LED, OFF);
	led_test();
	printf("system reboot success \r\n");
	srand(get_tick());
	
	node.call_back = test;
	node.repeat = 1;
	
	while(1)
	{
		if(timer_list_empty())
		{
			for(int i = 0; i < 32; i++)
			{
			id = rand() % 100;
			node.period = id;
			node.task_id = id;
			timer_list_push(&node);
				
			}
			printf("\r\n");
		}
//		id = rand() % 100;
//		node.period = id;
//		node.task_id = id;
//		timer_list_push(&node);
		
		timer_list_scan();
		//printf("test %u\r\n", get_tick());
		//delay_ms(200);
		//iwdg_refresh();
	}
	return true;
}
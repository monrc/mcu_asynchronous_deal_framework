#include "app_layer.h"
#include "intermediate_layer.h"
#include "task_manager.h"




int main()
{
	Timer_node_t timer;
	Task_node_t task;
	
	uint8_t timId[32];
	uint8_t id,i,j;
	uint8_t data;

	mcu_init();
	manager_init();
	terminal_init(serial2_put_char);

	led_control(YELLOW_LED, OFF);
	led_control(BULUE_LED, OFF);
	
	printf("system reboot success \r\n");
	
	while (1)
	{
		
		timer_list_pop();
		task_list_pop();
		manager_scan();
		
		terminal_handler();
		
		//printf("test %u\r\n", get_tick());
		//delay_ms(200);
		//iwdg_refresh();
	}
	return true;
}
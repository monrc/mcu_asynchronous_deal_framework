#include "intermediate_led.h"
#include "mcu_init.h"
#include "timer_list.h"

//
GPIO_TypeDef * const led_port_map[] = 
{
	[YELLOW_LED] = LED0_GPIO_Port,
	[BULUE_LED] = LED1_GPIO_Port
};

//
const uint32_t led_pin_map[] = 
{
	[YELLOW_LED] = LED0_Pin,
	[BULUE_LED] = LED1_Pin
};

//
const uint32_t led_state_map[] = 
{
	[ON] = GPIO_PIN_RESET,
	[OFF] = GPIO_PIN_SET
};

//
void led_control(Led_type_t type, Led_state_t state)
{
	HAL_GPIO_WritePin(led_port_map[type], led_pin_map[type], led_state_map[state]);
}


void led_on(void)
{
	led_control(YELLOW_LED, ON);
}

void led_off(void)
{
	led_control(YELLOW_LED, OFF);
}

void led_off_time(void)
{
	Timer_node_t node;
	led_off();
	node.period = 100;
	node.call_back = led_off;
	node.repeat = 0xF0;
	node.task_id = 1;
	timer_list_push(&node);
}

void led_test(void)
{
	Timer_node_t node;
	
	led_on();
	
	node.call_back = led_on;
	node.period = 100;
	node.repeat = 0xF0;
	node.task_id = 1;
	timer_list_push(&node);	
	
	node.call_back = led_off_time;
	node.period = 50;
	node.repeat = 1;
	node.task_id = 1;
	timer_list_push(&node);
}



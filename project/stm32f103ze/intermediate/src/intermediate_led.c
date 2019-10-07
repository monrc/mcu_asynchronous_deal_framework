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
	node.wPeriod = 100;
	node.CallBack = led_off;
	node.byRepeat = 0xff;
	node.byID = 1;
	timer_list_push(&node);
}

void led_test(void)
{
	Timer_node_t node;
	
	led_on();
	
	node.CallBack = led_on;
	node.wPeriod = 100;
	node.byRepeat = 0xff;
	node.byID = 1;
	timer_list_push(&node);	
	timer_list_print();
	
	
	node.CallBack = led_off_time;
	node.wPeriod = 50;
	node.byRepeat = 1;
	node.byID = 1;
	timer_list_push(&node);
	timer_list_print();
	
	node.CallBack = led_on;
	node.wPeriod = 100;
	node.byRepeat = 0xff;
	node.byID = 1;
	timer_list_push(&node);	
	timer_list_print();
	
	node.CallBack = led_off_time;
	node.wPeriod = 50;
	node.byRepeat = 1;
	node.byID = 1;
	timer_list_push(&node);
	timer_list_print();
}



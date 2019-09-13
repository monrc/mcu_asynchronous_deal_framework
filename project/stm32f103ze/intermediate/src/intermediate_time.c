#include "intermediate_time.h"
#include "intermediate_layer.h"


volatile uint32_t ticks = 0;
volatile uint16_t timer_ticks = 0;
/*********************************************************/
//name:
//function:
//input:
//output:
//note:
/*********************************************************/
void timer_irq_callback(void)
{
	ticks++;
	timer_ticks++;
}

/*********************************************************/
//name: get_tick
//function:
//input:
//output:
//note: None
/*********************************************************/
uint32_t get_tick(void)
{
	return ticks;
}

/*********************************************************/
//name: get_timer_tick
//function:
//input:
//output:
//note: None
/*********************************************************/
uint16_t get_timer_tick(void)
{
	return timer_ticks;
}
/*********************************************************/
//name: delay_ms
//function: 阻塞延迟一段时间，阻塞期间刷新看门狗
//input:
//output:
//note:
/*********************************************************/
void delay_ms(uint32_t time)
{
	uint32_t tick = get_tick();
	while(get_tick() - tick < time)
	{
		iwdg_refresh();
	}
}
	
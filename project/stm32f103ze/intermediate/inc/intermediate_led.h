#ifndef INTERMEDIATE_LED_H
#define INTERMEDIATE_LED_H

typedef enum
{
	YELLOW_LED  = 0,
	BULUE_LED,
}Led_type_t;


typedef enum
{
	ON = 0,
	OFF,
}Led_state_t;

void led_control(Led_type_t type, Led_state_t state);
void led_test(void);
#endif
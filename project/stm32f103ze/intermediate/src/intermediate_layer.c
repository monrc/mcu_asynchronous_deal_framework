#include "intermediate_layer.h"
#include "mcu_init.h"

void iwdg_refresh(void)
{
	HAL_IWDG_Refresh(&hiwdg);
}

void mcu_init(void)
{
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	//MX_IWDG_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_TIM6_Init();
}
#include "intermediate_uart.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <rt_misc.h>


#if (USE_ARM_COMPILER_6 == 1)
__asm(".global __use_no_semihosting");

extern int  sendchar(int ch);  /* in Serial.c */
extern int  getkey(void);      /* in Serial.c */
extern long timeval;           /* in Time.c   */

FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
	return (sendchar(ch));
}

int fgetc(FILE *f) {
	return (sendchar(getkey()));
}

int ferror(FILE *f)
{
	/* Your implementation of ferror */
	return EOF;
}

void _ttywrch(int ch)
{
	sendchar(ch);
}

void _sys_exit(int return_code)
{
	while (1);    /* endless loop */
}

int sendchar(int ch)
{
	while ((USART1->SR & 0X40) == 0); //循环发送,直到发送完毕
	USART1->DR = (uint8_t) ch;
	return ch;
}

int getkey(void)
{
	return 1;
}


void uart1_irq_callback(void)
{
	uint8_t data;
	if(READ_BIT(USART1->SR, USART_SR_RXNE) != USART_SR_RXNE)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		return;
	
	data = USART1->DR & 0xff;	
}
#else
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif
/*----------------------------------------------------------*/
/*----------------------------------------------------------*/
/*----------------------------------------------------------*/


int SerialPutChar(int ch)
{
	while ((USART2->SR & 0X40) == 0); //循环发送,直到发送完毕
	USART2->DR = (uint8_t) ch;
	return ch;
}

void uart2_irq_callback(void)
{
	uint8_t data;
	if(READ_BIT(USART2->SR, USART_SR_RXNE) != USART_SR_RXNE)  
		return;
	
	data = USART2->DR & 0xff;	
}
#include <string.h>
#include "list.h"
#include "debug.h"

/*********************************************************
* Name		: statck_init
* Function	: 栈初始化
* Input		: uint8_t *pStack	栈内存地址
			  uint8_t bySize	栈的空间大小
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void stack_init(uint8_t *pStack, uint8_t bySize)
{
	memset(pStack, 0, bySize);
	pStack[STACK_CAPACITY] = bySize - 1;
	pStack[STACK_POINT] = STACK_BUTTON;
}
/*********************************************************
* Name		: stack_push
* Function	: 栈元素入栈
* Input		: uint8_t *pStack	栈内存地址
			  uint8_t byData	入栈元素
* Output	: None
* Return	: true 入栈成功  false 入栈失败
* Note		: None
*********************************************************/
bool stack_push(uint8_t *pStack, uint8_t byData)
{
	ERROR("stack full", pStack[STACK_POINT] < pStack[STACK_CAPACITY], return false);
	pStack[++pStack[STACK_POINT]] = byData;
	return true;
}

/*********************************************************
* Name		: stack_pop
* Function	: 出栈
* Input		: uint8_t *pStack	栈内存地址
* Output	: uint8_t *pData	出栈元素
* Return	: true 出栈成功  false 栈为空，出栈失败
* Note		: None
*********************************************************/
bool stack_pop(uint8_t *pStack, uint8_t *pData)
{
	ERROR("stack empty", pStack[STACK_POINT] > STACK_BUTTON,return false);
	*pData = pStack[pStack[STACK_POINT]--]; 
	return true;
}
/*********************************************************
* Name		: stack_empty
* Function	: 检测栈是否为空
* Input		: uint8_t *pStack	栈内存地址
* Output	: None
* Return	: true 栈为空  false 栈非空
* Note		: None
*********************************************************/
bool stack_empty(uint8_t *pStack)
{
	return pStack[STACK_POINT] == STACK_BUTTON;
}

/*********************************************************
* Name		: stack_full
* Function	: 检测栈空间是否满
* Input		: uint8_t *pStack	栈内存地址
* Output	: None
* Return	: true 栈满  false 栈非满
* Note		: None
*********************************************************/
bool stack_full(uint8_t *pStack)
{
	return pStack[STACK_POINT] == pStack[STACK_CAPACITY];
}

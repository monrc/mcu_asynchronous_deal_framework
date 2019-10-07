#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

#define RECV_BUFF_MAX_SIZE			99
#define RECV_BUFF_ARRAY_SIZE		(RECV_BUFF_MAX_SIZE + 1)
#define FUN_ARGUMENTS_MAX_SIZE		5


#define RECV_BUFF_OVERFLOW			32		

#define	INPUT_KEY_UP			33
#define INPUT_KEY_DOWN			34
#define INPUT_KEY_RIGHT			35	
#define INPUT_KEY_LEFT			36



#define CHAR_TO_SPECIAL(x)			((x) - 65 + 33)

typedef struct
{
	void *func;
	const char *pName;
	const uint8_t byParamterNum;
}Function_map_t;




typedef enum
{
	TERMINAL_IDEL = 0x01,	//空闲状态,等待输入第一个字符
	TERMINAL_BUSY,			//输入中,等待输入完成
	TERMINAL_READY,			//输入了控制符，等待分析处理
}Terminal_state_t;

typedef struct
{
	void (*OutPutCallBack)(uint8_t byData);		//打印数据回调函数
	char byRecvBuff[RECV_BUFF_ARRAY_SIZE];		//接收缓冲区
	uint8_t byRecvLen;							//接收长度
	uint8_t byRecvLast;							//接收的数据缓存个数
	uint8_t byCtrlType;							//控制字符类型
	uint8_t bySpecialCharFlag;					//特殊控制符号标志
	volatile Terminal_state_t eState;			//接收状态处理状态标志
}Terminal_t;


void terminal_input_predeal(uint8_t byData);
void terminal_handler(void);
void terminal_init(void);

#endif
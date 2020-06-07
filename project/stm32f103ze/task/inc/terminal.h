#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>


#define DEFAULT_AUTHORITY		0
#define USER_NUMBER				2		//用户个数
#define USER_NAME_TESTER		"tester"
#define	USER_KEY_TESETER		123
#define USER_NAME_ADMIN			"admin"
#define	USER_KEY_ADMIN			12345



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

typedef union
{
	struct
	{
		uint8_t State:2;
		uint8_t Authority:2;
		uint8_t Res:4;
	}Bit;
	uint8_t byVal;
}Control_t;

typedef enum
{
	TERMINAL_IDEL,			//空闲状态,等待输入第一个字符
	TERMINAL_BUSY,			//输入中,等待输入完成
	TERMINAL_READY,			//输入了控制符，等待分析处理
}Terminal_state_t;

typedef struct
{
	void (*OutPutCallBack)(uint8_t byData);		//打印数据回调函数
	char byRecvBuff[RECV_BUFF_ARRAY_SIZE];		//接收缓冲区
	uint8_t byRecvLen;							//接收长度
	uint8_t byShowLen;							//输入回显长度
	uint8_t byRecvLast;							//接收的数据缓存个数
	uint8_t byCtrlType;							//控制字符类型
	uint8_t byUserIndexTab[USER_NUMBER];		//用户登陆索引表，在命令表中的索引
	uint8_t byAuthority;						//访问权限等级，不同用户不同访问限制
	uint8_t bySpecialCharFlag;					//特殊控制符号标志
	volatile Control_t Flag;					//接收状态处理状态标志
}Terminal_t;


void terminal_input_predeal(uint8_t byData);
void terminal_handler(void);
void terminal_init(void (*pCallBack)(uint8_t));

#endif
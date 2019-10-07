/********************************************************
*file name: terminal.c
*function: 解析用户输入的指令并执行，指令最多支持5个参数
*使用说明：
	1.外部接口
		terminal_init() 			终端内存初始化工作
		terminal_input_predeal() 	放在串口接收中断中（前台程序）
		terminal_handler()			放在主循环while(1)中（后台程序）
	2.按键功能
		Tab 		显示可能匹配的命令、自动匹配指令
		↑			输入历史命令
		↓			执行历史命令
		Enter		输入结束，开始执行
		Backspace	删除前一个字符
	3.添加指令
		将函数添加至stFunTab数组中，参数依次为函数名、指令名、参数个数
	4.输入示例
		指令名 空格 参数1 空格 参数2 空格 参数3 回车
		eg：test 123 345 回车
	5.移植说明
		将外部接口放在合适的位置
		重定义实现printf函数功能
		实现串口打印字符的函数，该模块中使用serial1_put_char
		添加函数指令映射表
********************************************************/

#include <string.h>
#include <stdio.h>
#include "terminal.h"
#include "intermediate_uart.h"
#include "debug.h"

//外部指令函数声明
extern void timer_list_print(void);
extern void task_list_print(void);
extern void timer_test(void);
extern void task_test(void);
extern void task_list_pop(void);
//函数指令映射表定义，参数为：函数、命令映射表、函数形参个数
const Function_map_t stFunTab[] = 
{
	[0] = {.func = timer_list_print, 	.pName = "timerprint", 	.byParamterNum = 0},
	[1] = {.func = task_list_print,  	.pName = "taskprint", 	.byParamterNum = 0},
	[2] = {.func = timer_test, 		 	.pName = "timertest", 	.byParamterNum = 1},
	[3] = {.func = task_test, 			.pName = "tasktest", 	.byParamterNum = 1},
	[4] = {.func = task_list_pop, 		.pName = "pop", 		.byParamterNum = 0},
};
#define STFUNTAB_SIZE		(sizeof(stFunTab) / sizeof(stFunTab[0]))

//内部终端模块结构体定义
static Terminal_t stTerminal;

//内部函数声明
static void searching_command(void);
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd);
static bool string_to_uint(char *pStr, uint8_t byLen, uint32_t *pValue);
static void execute_handled(uint8_t byFunIndex, uint32_t *pArg, uint8_t byArgNum);
static bool recv_semantic_analysis(void);

/*********************************************************/
//name: terminal_init
//function: 终端模块参数初始化
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
void terminal_init(void)
{
	uint8_t i;
	memset(&stTerminal, 0, sizeof(Terminal_t));
	stTerminal.eState = TERMINAL_IDEL;
	stTerminal.OutPutCallBack = serial1_put_char; //串口打印函数
	//函数列表检测
	for(i = 0; i < STFUNTAB_SIZE; i++)
	{
		if(NULL == stFunTab[i].func)
		{
			printf("stFunTab[%u] fun is NULL\r\n", i);
			while(1);
		}
	}
}

/*********************************************************/
//name: output_string
//function: 输入字符串至串口
//input: const char *pStr	字符串起始地址
//		 uint8_t byLen		字符串长度
//output: None
//return: None
//note: None
/*********************************************************/
void output_string(const char *pStr, uint8_t byLen)
{
	TASK_ERROR("pointer is null", (pStr != NULL), return;);
	
	while(byLen--)
	{
		stTerminal.OutPutCallBack(*pStr++);
	}
}

/*********************************************************/
//name: terminal_input_predeal
//function: 终端输入预处理
//input: 串口读取到的数据
//output: None
//return: None
//note: 该函数串口接收中断中调用为佳
/*********************************************************/
void terminal_input_predeal(uint8_t byData)
{

	//ready状态，直接退出，等待分析函数分析命令
	if(stTerminal.eState == TERMINAL_READY)
	{
		return;
	}
	//扩充ASCII码字符集 不予处理
	if(byData > 127)
	{
		return;
	}
	
	//特殊控制字符（上下左右）处理
	if(stTerminal.bySpecialCharFlag)
	{
		stTerminal.bySpecialCharFlag++;
		
		switch(stTerminal.bySpecialCharFlag)
		{
			case 2:
			{
				//存在特殊字符的可能性
				if(91 == byData)
				{
					return;
				}
				else	//非特殊字符
				{
					//此处应该产生esc按键的处理信号，由于esc按键应用层暂未做功能实现，故滤掉
					
					//恢复特殊字符处理标志字
					stTerminal.bySpecialCharFlag = 0;
					break;
				}
			}
			case 3:
			{
				if (65 <= byData && byData <= 68)
				{
					stTerminal.byCtrlType = CHAR_TO_SPECIAL(byData);
					stTerminal.eState = TERMINAL_READY;
				}
				stTerminal.bySpecialCharFlag = 0;
				return;
			}
			default:
			{
				stTerminal.bySpecialCharFlag = 0;
				return;
			}
		}
	}
	
	//处理ASCII码控制字符
	if(byData < 32)
	{
		if(27 == byData)
		{
			stTerminal.bySpecialCharFlag = 1;
		}
		else	//非特殊控制字符
		{
			stTerminal.eState = TERMINAL_READY;
			stTerminal.byCtrlType = byData;
		}
	}
	else	//处理ASCII码打印字符
	{
		//输入回显至窗口
		stTerminal.OutPutCallBack(byData);
		
		//接收缓冲区满，强制切换至接收完成状态
		if(RECV_BUFF_MAX_SIZE == stTerminal.byRecvLen)
		{
			stTerminal.eState = TERMINAL_READY;
			stTerminal.byCtrlType = RECV_BUFF_OVERFLOW;
		}
		else
		{
			stTerminal.byRecvBuff[stTerminal.byRecvLen++] = byData;
			stTerminal.eState = TERMINAL_BUSY;
		}
	}
}

/*********************************************************/
//name: terminal_handler
//function: 终端输入结果分析处理
//input: None
//output: None
//return: None
//note: 该函数适合放在后台程序中调用
/*********************************************************/
void terminal_handler(void)
{
	//起着同步与互斥的作用
	if (TERMINAL_READY != stTerminal.eState)
	{
		return;
	}
	
	switch(stTerminal.byCtrlType)
	{
		case '\b':	//backspace 按键
		{
			stTerminal.OutPutCallBack('\b');
			stTerminal.OutPutCallBack(' ');
			stTerminal.OutPutCallBack('\b');
			stTerminal.byRecvLen = stTerminal.byRecvLen ? stTerminal.byRecvLen - 1: 0;
			break;
		}
			
		case '\t':	//Tab 按键
		{
			searching_command();
			break;
		}
		
		case '\r':	//Enter 按键
		{
			printf("\r\n");
			recv_semantic_analysis();
			if(stTerminal.byRecvLen)
			{
				stTerminal.byRecvLast = stTerminal.byRecvLen;
				stTerminal.byRecvLen = 0;
			}
			break;
		}
		
		case INPUT_KEY_UP:	//↑按键
		{
			if(stTerminal.byRecvLen == 0)
			{
				output_string(stTerminal.byRecvBuff, stTerminal.byRecvLast);
				stTerminal.byRecvLen = stTerminal.byRecvLast;
			}
			break;
		}
		
		case INPUT_KEY_DOWN: //↓按键
		{
			if(stTerminal.byRecvLen == 0)
			{
				output_string(stTerminal.byRecvBuff, stTerminal.byRecvLast);
				stTerminal.byRecvLen = stTerminal.byRecvLast;
				
				printf("\r\n");
				if(false == recv_semantic_analysis())
				{
					printf("input error\r\n");
				}
				stTerminal.byRecvLast = stTerminal.byRecvLen;
				stTerminal.byRecvLen = 0;
			}
			break;
		}
		
		case RECV_BUFF_OVERFLOW:	//缓冲区溢出
		{
			printf("uart recv over flow\r\n");
			break;
		}
		default :
		{
			break;
		}
	}
	stTerminal.eState = TERMINAL_IDEL;
}

/*********************************************************/
//name: searching_command
//function: 根据已经输入的内容查找命令
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
static void searching_command(void)
{
	uint8_t i = 0;
	uint8_t byNameLength;
	uint8_t byMatchIndex[STFUNTAB_SIZE] = {0};
	uint8_t byMatchNum = 0;
	bool bMatchFlag = true;
	char byChar;
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		byNameLength = strlen(stFunTab[i].pName);
		if(stTerminal.byRecvLen > byNameLength)
		{
			continue;
		}
		
		if (0 == strncmp(stFunTab[i].pName, stTerminal.byRecvBuff, stTerminal.byRecvLen))
		{
			//存储匹配的字符
			byMatchIndex[byMatchNum++] = i;
		}
	}
	
	if (byMatchNum)
	{
		if(byMatchNum > 1)
		{
			printf("\r\n");
			for (i = 0; i < byMatchNum; i++)
			{
				printf("%s   ", stFunTab[byMatchIndex[i]].pName);
			}
			printf("\r\n");
			
			//查找相同的字符，如有命令：add1 add2，输入a之后，按tab显示add
			while(bMatchFlag)
			{
				byChar = stFunTab[byMatchIndex[0]].pName[stTerminal.byRecvLen];
				for (i = 1; i < byMatchNum; i++)
				{
					if (byChar != stFunTab[byMatchIndex[i]].pName[stTerminal.byRecvLen])
					{
						bMatchFlag = false;
						break;
					}
				}
				stTerminal.byRecvLen++;
			}
			
			strncpy(stTerminal.byRecvBuff, stFunTab[byMatchIndex[0]].pName, --stTerminal.byRecvLen);
			output_string(stTerminal.byRecvBuff, stTerminal.byRecvLen);
		}
		else
		{
			byNameLength = strlen(stFunTab[byMatchIndex[0]].pName);
			strncpy(stTerminal.byRecvBuff, stFunTab[byMatchIndex[0]].pName, byNameLength);
			stTerminal.byRecvBuff[byNameLength++] = ' '; //添加空格符
			output_string(&stTerminal.byRecvBuff[stTerminal.byRecvLen], byNameLength - stTerminal.byRecvLen);
			stTerminal.byRecvLen = byNameLength;
		}
	}
}
/*********************************************************/
//name: recv_semantic_analysis
//function: 根据输入的内容分析语义
//input: None
//output: None
//return: true：输入合理	false：输入错误
//note: None
/*********************************************************/
static bool recv_semantic_analysis(void)
{
	uint8_t i, byCmdLen;
	uint8_t byTemp;
	uint8_t byHead = 0, byTail = 0;
	uint32_t dwPramter[FUN_ARGUMENTS_MAX_SIZE] = {0};	 //参数缓冲区
	uint8_t byParamterNum = 0;			//参数个数
	uint8_t byCmdIndex = STFUNTAB_SIZE;	//匹配命令位置
	if (0 == stTerminal.byRecvLen)	//数据段为空
	{
		return false;
	}
	
	byCmdLen = separate_string(&byHead, ' ', &byTail);
	if(0 == byCmdLen)
	{
		printf("input error\r\n");
		return false;
	}
	//查找命令
	for(i = 0; i < STFUNTAB_SIZE; i++)	
	{
		byTemp = strlen(stFunTab[i].pName);
		if(byCmdLen != byTemp)
		{
			continue;
		}
		
		if(0 == strncmp(stFunTab[i].pName, &stTerminal.byRecvBuff[byHead], byCmdLen))
		{
			byCmdIndex = i; //记录命令的位置
			break;
		}
	}
	if (STFUNTAB_SIZE == byCmdIndex)	//未找到命令
	{
		printf("can not find that cmd\r\n");
		return false;
	}
	
	//处理输入的参数
	for(i = byTail; i < stTerminal.byRecvLen; i += byTemp)
	{
		byHead = byTail + 1;
		byTemp = separate_string(&byHead, ' ', &byTail);
		if(0 == byTemp || byParamterNum == FUN_ARGUMENTS_MAX_SIZE)
		{
			break;
		}
		
		if(false == string_to_uint(&stTerminal.byRecvBuff[byHead], byTemp, &dwPramter[byParamterNum++]))
		{
			printf("paramater error\r\n");
			return false;
		}
	}
	
	if(byParamterNum != stFunTab[byCmdIndex].byParamterNum)
	{
		printf("paramter num not match\r\n");
		return false;
	}
	
	execute_handled(byCmdIndex, dwPramter, byParamterNum);
	return true;
}

/*********************************************************/
//name: separate_string
//function: 根据分隔符对接收的数据进行分段
//input: uint8_t *pStart 分隔开始位置
//		 const char chr	分隔符
//output: uint8_t *pEnd	字符串分隔结束位置
//return: 匹配的字符串长度
//note: pEnd的结束位置为第一个分隔符
/*********************************************************/
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd)
{
	uint8_t byLen = 0;
	TASK_ERROR("pointer is null", (pStart != NULL && pEnd != NULL), return false;);
	//过滤多余的分隔符
	while(*pStart < stTerminal.byRecvLen)
	{
		if(stTerminal.byRecvBuff[*pStart] != chr)
		{
			break;
		}
		++(*pStart);
	}
	
	//查找字符串
	*pEnd = *pStart;
	while(*pEnd < stTerminal.byRecvLen)
	{
		if(stTerminal.byRecvBuff[*pEnd] == chr)
		{
			break;
		}
		++(*pEnd);
	}
	return *pEnd - *pStart;
}

/*********************************************************/
//name: string_to_uint
//function: 把字符串转化为数字
//input: char *pStr 字符串起始地址
//		 uint8_t byLen	字符串长度
//output: uint32_t *pValue 转换后的数值
//return: true 字符串转换成功  false 转换出错
//note: 仅支持16位进制和10进制正整数
/*********************************************************/
static bool string_to_uint(char *pStr, uint8_t byLen, uint32_t *pValue)
{
	uint32_t dwBaseValue = 10;
	uint32_t dwCharValue;
	bool bHexFlag = false;
	
	TASK_ERROR("pointer is null", (pStr != NULL && pValue != NULL), return false;);
	
	if('0' == pStr[0] && ('x' == pStr[1] || 'X' == pStr[1]))
	{
		if(byLen > 10)	//长度溢出
		{
			return false;
		}
		byLen -= 2;
		pStr += 2;
		dwBaseValue = 16;
		bHexFlag = true;
	}
	
	*pValue = 0;
	
	if(false == bHexFlag)
	{
		while(byLen--)
		{
			if(*pStr > '9' || *pStr < '0')
			{
				return false;
			}
			*pValue *= dwBaseValue;
			*pValue += *pStr - '0';
			pStr++;
		}
	}
	else
	{
		while(byLen--)
		{
			//参数检查
			if(*pStr <= '9' && *pStr >= '0')
			{
				dwCharValue = *pStr - '0';
			}
			else if(*pStr <= 'f' && *pStr >= 'a')
			{
				dwCharValue = *pStr - 'a' + 10;
			}
			else if (*pStr <= 'F' && *pStr >= 'A')
			{
				dwCharValue = *pStr - 'A' + 10;
			}
			else
			{
				return false;
			}
			//运算获得结果
			*pValue *= dwBaseValue;
			*pValue += dwCharValue;
			pStr++;
		}
	}

	return true;
}

/*********************************************************/
//name: execute_handled
//function: 执行解析的命令
//input: uint8_t byFunIndex 	需要运行的函数在函数列表的位置
//		 uint32_t *pArg	  		参数列表
//		 uint8_t byArgNum		参数个数
//output: uint32_t *pValue 转换后的数值
//return: true 字符串转换成功  false 转换出错
//note: 仅支持16位进制和10进制正整数
/*********************************************************/
static void execute_handled(uint8_t byFunIndex, uint32_t *pArg, uint8_t byArgNum)
{
	switch(byArgNum)
	{
		case 0:
		{
			((void (*)())stFunTab[byFunIndex].func)();
			break;
		}
		case 1:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0]);
			break;
		}
		case 2:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1]);
			break;
		}
		case 3:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2]);
			break;
		}
		case 4:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2], pArg[3]);
			break;
		}
		case 5:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2], pArg[3], pArg[4]);
			break;
		}
	}
}

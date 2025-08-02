/*********************************************************************************************************
* 模块名称：UART1.c
* 摘    要：串口模块，包括串口模块初始化，以及中断服务函数处理，以及读写串口函数实现
* 当前版本：1.0.0
* 作    者：SZLY(COPYRIGHT 2018 - 2020 SZLY. All rights reserved.)
* 完成日期：2020年01月01日 
* 内    容：
* 注    意：                                                                  
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "UART1.h"
#include "stm32f10x_conf.h"
#include "Queue.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
typedef enum{
	UART_STATE_OFF,				//串口未发送数据
	UART_STATE_ON,				//串口正在发送数据
	UART_STATE_MAX
}EnumUARTState;
/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static	StructCirQue s_structUARTSendCirQue;				//发送串口循环队列
static	StructCirQue s_structUARTRecCirQue;					//接受串口循环队列
static	u8	s_arrSendBuf[UART1_BUF_SIZE];						//发送串口循环队列的缓冲区
static	u8	s_arrRecBuf[UART1_BUF_SIZE];						//接受串口循环队列的缓冲区

static	u8	s_iUARTTxSts;														//串口发送数据状态
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static	void	InitUARTBuf(void);									//初始化串口缓冲区，包括发送缓冲区和接收缓冲区
static	u8		WriteReceiveBuf(u8 d);							//将接收到的数据写入接收缓冲区
static	u8		ReadSendBuf(u8 *p);									//读取发送缓冲区中的数据

static	void	ConfigUART(u32 bound);							//配置串口相关的参数，包括GPIO、RCC、USART和NVIC
static	void	EnableUARTTx(void);									//使串口能发送，在WriteUARTx中调用，每次发送数据之后需要调用
static	void	SendCharUsedByFputc(u16 ch);				//发送字符函数，专由fputc函数调用
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitUARTBuf
* 函数功能：对发送缓冲区s_structUARTSendCirQue和接收缓冲区s_structUARTRecCirQue进行初始化
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static	void	InitUARTBuf(void)
{
	i16 i;
	for(i = 0;i<UART1_BUF_SIZE;i++)
	{
		s_arrSendBuf[i] = 0;
		s_arrRecBuf[i] = 0;
	}
	
	InitQueue(&s_structUARTSendCirQue,s_arrSendBuf,UART1_BUF_SIZE);
	InitQueue(&s_structUARTRecCirQue,s_arrRecBuf,UART1_BUF_SIZE);
}

/*********************************************************************************************************
* 函数名称：static	u8		WriteReveiveBuf(u8 d)
* 函数功能：调用EnQueue函数，将数据写入接收缓冲区s_structUARTRecCirQue
* 输入参数：u8 d
* 输出参数：ok;																//返回写入数据成功标志，0—不成功，1—成功
* 返 回 值：ok;																//返回写入数据成功标志，0—不成功，1—成功
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static	u8		WriteReceiveBuf(u8 d)
{
	u8	ok = 0;																//写入数据成功标志，0—不成功，1—成功
	ok = EnQueue(&s_structUARTRecCirQue,&d,1);
	return	ok;																//返回写入数据成功标志，0—不成功，1—成功
}

/*********************************************************************************************************
* 函数名称：static	u8		ReadSendBuf(u8 *p)
* 函数功能：调用DeQueue函数，将数据写入发送缓冲区s_structUARTSendCirQue
* 输入参数：u8 *p
* 输出参数：ok;																//返回写入数据成功标志，0—不成功，1—成功
* 返 回 值：ok;																//返回写入数据成功标志，0—不成功，1—成功
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static	u8		ReadSendBuf(u8 *p)
{
	u8	ok = 0;																//读取数据成功标志，0—不成功，1—成功
	ok = DeQueue(&s_structUARTSendCirQue,p,1);
	return	ok;																//返回读取数据成功标志，0—不成功，1—成功
}

/*********************************************************************************************************
* 函数名称：ConfigUART(u32 bound)
* 函数功能：配置串口相关的参数，包括GPIO、RCC、USART和NVIC
* 输入参数：u32 bound
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static	void	ConfigUART(u32 bound)
{
	GPIO_InitTypeDef	GPIO_InitStructure;							//GPIO_InitStructure用于存放GPIO的参数
	USART_InitTypeDef USART_InitStructure;						//USART_InitStructure用于存放USART的参数
	NVIC_InitTypeDef	NVIC_InitStructure;							//NVIC_InitStructure用于存放NVIC的参数
	
	//使能RCC相关时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);			//使能USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);			//使能GPIOA的时钟
	//配置TX的GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;									//设置TX的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;						//设置TX的模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;					//设置TX的I/O口输出速度
	GPIO_Init(GPIOA,&GPIO_InitStructure);											//根据参数初始化TX的GPIO
	//配置RX的GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;									//设置RX的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;				//设置RX的模式
	GPIO_Init(GPIOA,&GPIO_InitStructure);												//根据参数初始化RX的GPIO
	
	//配置USART的参数
	USART_StructInit(&USART_InitStructure);											//初始化USART_InitStructure
	USART_InitStructure.USART_BaudRate = bound;									//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//设置数据字的长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//设置停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;					//设置奇偶校验
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;//设置模式
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//设置硬件流控制模式
	USART_Init(USART1,&USART_InitStructure);										//根据参数初始化USART1
	
	//配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;						//中断信号通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					//设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//使能中断
	NVIC_Init(&NVIC_InitStructure);															//根据参数初始化NVIC
	
	//使能USART1及其中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);								//使能接收缓冲区非空中断
	USART_ITConfig(USART1,USART_IT_TXE,ENABLE);									//使能发送缓冲区空中断
	USART_Cmd(USART1,ENABLE);																		//使能USART1
	
	s_iUARTTxSts = UART_STATE_OFF;															//串口发送数据状态为未发送状态
}
	
/*********************************************************************************************************
* 函数名称：static	void	EnableUARTTx(void)
* 函数功能：将s_iUARTTxSts变量赋值为UART_STATE_ON，并调用USART_ITConfig函数使能发送缓冲区空中断
						该函数再WriteUARTx中调用，即每次发送数据后，调用该函数使能发送缓冲区空中断
* 输入参数：void
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static	void	EnableUARTTx(void)
{
	s_iUARTTxSts = UART_STATE_ON;								//串口发送数据状态设置为正在发送数据
	USART_ITConfig(USART1,USART_IT_TXE,ENABLE);	//使能发送中断
}

/*********************************************************************************************************
* 函数名称：static	void	SendCharUsedByFputc(u16 ch);
* 函数功能：printf函数调用fputc函数，fputc函数调用SendCharUsedByFputc函数
						SendCharUsedByFputc函数调用USART_SendData函数，向USART_DR写入数据实现基于串口的信息输出
* 输入参数：u16 ch
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static	void	SendCharUsedByFputc(u16 ch)
{
	USART_SendData(USART1,(u8)ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC) == RESET)
	{
	
	}
}

/*********************************************************************************************************
* 函数名称：USART1_IRQHandler(void)
* 函数功能：中断服务函数代码,只要有中断都触发这个函数
* 输入参数：u16 ch
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void USART1_IRQHandler(void)
{
	u8 uData = 0;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!= RESET)//接收缓冲区非空中断
	{
		NVIC_ClearPendingIRQ(USART1_IRQn);//清除USART1中断挂起
		uData = USART_ReceiveData(USART1);//将接收到的数据保存的uData
		
		WriteReceiveBuf(uData);//将接收到的数据写入接收缓冲区
	}
	
	if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)//溢出错误标志为1
	{
		USART_ClearFlag(USART1,USART_FLAG_ORE);//清除溢出错误标志
		USART_ReceiveData(USART1);//读取USART_DR
	}
	
	if(USART_GetITStatus(USART1,USART_IT_TXE) != RESET)//发送缓冲区空中断
	{
		USART_ClearITPendingBit(USART1,USART_IT_TXE);//清空发送中断标志
		NVIC_ClearPendingIRQ(USART1_IRQn);//清除中断挂起
		
		ReadSendBuf(&uData);//读取发送缓冲区的数据到uData
		
		USART_SendData(USART1,uData);//将uData写入USART_DR
		
		if(QueueEmpty(&s_structUARTSendCirQue))//当发送缓冲区为空时
		{
			s_iUARTTxSts = UART_STATE_OFF;//串口发送数据状态设置为未发送数据
			USART_ITConfig(USART1,USART_IT_TXE,DISABLE);//关闭串口发送缓冲区空中断
		}
	}
}


/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitUART
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitUART1(u32 bound)
{
	InitUARTBuf();//初始化串口缓冲区，包括发送缓冲区和接收缓冲区
	ConfigUART(bound);//配置串口相关参数，包括GPIO,RCC,USART和NVIC
}
/*********************************************************************************************************
* 函数名称：WriteUART1
* 函数功能：将存放在pBuf中的待发送数据通过EnQueue写入发送缓冲区s_structUARTSendCirQue，同时通过EnableUARTTx函数开启中断使能
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u8 WriteUART1(u8 *pBuf,u8 len)
{
	u8 wLen = 0;		//实际写入数据的个数
	wLen = EnQueue(&s_structUARTSendCirQue,pBuf,len);
	if(wLen <UART1_BUF_SIZE)
	{
		if(s_iUARTTxSts == UART_STATE_OFF)
		{
			EnableUARTTx();
		}
	}
	return wLen;		//返回实际写入数据的个数
}

/*********************************************************************************************************
* 函数名称：ReadUART1
* 函数功能：将存放在接收缓冲区s_structUARTRecCirQue的数据通过DeQueue函数读出，并存放于pBuf只想的存储空间
* 输入参数：u8 *pBuf,u8 len
* 输出参数：u8 rLen
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u8	ReadUART1(u8 *pBuf,u8 len)
{
	u8 rLen = 0;//实际读取数据的长度
	rLen = DeQueue(&s_structUARTRecCirQue,pBuf,len);
	return rLen;//返回实际读取数据的长度
}
/*********************************************************************************************************
* 函数名称：fputc
* 函数功能：将存放在接收缓冲区s_structUARTRecCirQue的数据通过DeQueue函数读出，并存放于pBuf只想的存储空间
* 输入参数：u8 *pBuf,u8 len
* 输出参数：u8 rLen
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
int fputc(int ch,FILE* f)
{
	SendCharUsedByFputc((u8)ch);			//发送字符函数，专由fputc函数调用
	return ch;												//返回ch
}

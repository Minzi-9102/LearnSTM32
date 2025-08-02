/*********************************************************************************************************
* 模块名称：DAC.c
* 摘    要：DAC模块
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
#include "DAC.h"
#include "Wave.h"
#include "stm32f10x_conf.h"
/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define DAC_DHR12R1_ADDR ((u32)0x40007408)	//DAC1的地址（12位右对齐）
#define DAC_DHR12R2_ADDR ((u32)0x40007414)	//DAC2的地址（12位右对齐）
/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
 static StructDACWave s_strDAC1WaveBuf;//存储DAC1波形属性，包括波形地址和点数
 static StructDACWave s_strDAC2WaveBuf;//存储DAC2波形属性，包括波形地址和点数
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTimer4(u16 arr,u16 psc);//配置TIM4
static void ConfigDAC1(void);//配置DAC1
static void ConfigDMA2Ch3ForDAC1(StructDACWave wave);//配置DMA2的通道3
static void ConfigDAC2(void);//配置DAC2
static void ConfigDMA2Ch4ForDAC2(StructDACWave wave);//配置DMA2的通道4
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTimer4
* 函数功能：配置 TIM4 为触发源，用于定时触发 DAC 转换。
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigTimer4(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;//用于存放定时器的参数
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//使能定时器的时钟
	//配置TIM4
	TIM_DeInit(TIM4);//重置为默认值
	TIM_TimeBaseStructure.TIM_Period = arr;//设置自动重载值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;//设置预分频器值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割：tDTS=tCK_INT 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//设置递增计数模式
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);//根据参数初始化定时器
	
	TIM_SelectOutputTrigger(TIM4,TIM_TRGOSource_Update);//选择更新事件为触发输入
	TIM_Cmd(TIM4,ENABLE);//使能定时器
}
/*********************************************************************************************************
* 函数名称：static void ConfigDAC1(void)
* 函数功能：初始化 DAC1（PA4 引脚）为 DMA 触发模式。
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDAC1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//存放GPIO的参数
	DAC_InitTypeDef DAC_InitStructure;//存放DAC的参数
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIO的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);//使能DAC的时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//设置引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//设置IO输出速度
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//设置输入类型
	GPIO_Init(GPIOA,&GPIO_InitStructure);//根据参数初始化GPIO
	
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T4_TRGO;//设置DAC触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;//关闭波形发生器
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
																													//不屏蔽LSFR位0/三角波幅值等于1
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;//使能DAC输出缓存
	DAC_Init(DAC_Channel_1,&DAC_InitStructure);//初始化DAC通道1
	
	DAC_DMACmd(DAC_Channel_1,ENABLE);//使能DAC通道1的DMA模式
	
	DAC_SetChannel1Data(DAC_Align_12b_R,0);//设置为12位右对齐数据格式
	
	DAC_Cmd(DAC_Channel_1,ENABLE);//使能DAC通道1
}
/*********************************************************************************************************
* 函数名称：ConfigDMA2Ch3ForDAC1
* 函数功能：配置 DMA2 通道3，将波形数据从内存搬运到 DAC 数据寄存器。
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDMA2Ch3ForDAC1(StructDACWave wave)
{
	DMA_InitTypeDef DMA_InitStructure;//存放DMA的参数
	NVIC_InitTypeDef NVIC_InitStructure;//存放NVIC的参数
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);//使能DMA2的时钟
	
	DMA_DeInit(DMA2_Channel3);//将DMA1_CH1寄存器设置为默认值
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDR;//设置外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = wave.waveBufAddr;//设置储存器地址
	DMA_InitStructure.DMA_BufferSize = wave.waveBufSize;//设置要传输的数据项目数
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//设置为存储器到外设模式
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//设置外设为非递增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置存储器为递增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//设置外设数据长度为半字
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//设置存储器数据长度为半字
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//设置为循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//设置为高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//禁止存储器到存储器访问
	
	DMA_Init(DMA2_Channel3,&DMA_InitStructure);//根据参数初始化DMA2_Channel3
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel3_IRQn;//中断通道号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能中断
	NVIC_Init(&NVIC_InitStructure);//根据参数初始化NVIC
	
	DMA_ITConfig(DMA2_Channel3,DMA_IT_TC,ENABLE);//使能DMA2_Channel3的传输完成中断
	
	DMA_Cmd(DMA2_Channel3,ENABLE);//使能DMA2_Channel3
}
/*********************************************************************************************************
* 函数名称：DMA2_Channel3_IRQHandler
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void DMA2_Channel3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_IT_TC3))//判断DMA2_Channel3传输完成中断是否发生
	{
		NVIC_ClearPendingIRQ(DMA2_Channel3_IRQn);//清除DMA2_Channel3中断挂起
		DMA_ClearITPendingBit(DMA2_IT_GL3);//清除DMA2_Channel3传输完成中断标志
		
		ConfigDMA2Ch3ForDAC1(s_strDAC1WaveBuf);//配置DMA2通道3
	}
}

/*********************************************************************************************************
* 函数名称：static void ConfigDAC2(void)
* 函数功能：初始化 DAC2（PA5 引脚）为 DMA 触发模式。
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDAC2(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//存放GPIO的参数
	DAC_InitTypeDef DAC_InitStructure;//存放DAC的参数
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIO的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);//使能DAC的时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//设置引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//设置IO输出速度
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//设置输入类型
	GPIO_Init(GPIOA,&GPIO_InitStructure);//根据参数初始化GPIO
	
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T4_TRGO;//设置DAC触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;//关闭波形发生器
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
																													//不屏蔽LSFR位0/三角波幅值等于1
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;//使能DAC输出缓存
	DAC_Init(DAC_Channel_2,&DAC_InitStructure);//初始化DAC通道2
	
	DAC_DMACmd(DAC_Channel_2,ENABLE);//使能DAC通道2的DMA模式
	
	DAC_SetChannel1Data(DAC_Align_12b_R,0);//设置为12位右对齐数据格式
	
	DAC_Cmd(DAC_Channel_2,ENABLE);//使能DAC通道2
}
/*********************************************************************************************************
* 函数名称：ConfigDMA2Ch4ForDAC2
* 函数功能：配置 DMA2 通道3，将波形数据从内存搬运到 DAC 数据寄存器。
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDMA2Ch4ForDAC2(StructDACWave wave)
{
	DMA_InitTypeDef DMA_InitStructure;//存放DMA的参数
	NVIC_InitTypeDef NVIC_InitStructure;//存放NVIC的参数
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);//使能DMA2的时钟
	
	DMA_DeInit(DMA2_Channel4);//将DMA1_CH1寄存器设置为默认值
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2_ADDR;//设置外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = wave.waveBufAddr;//设置储存器地址
	DMA_InitStructure.DMA_BufferSize = wave.waveBufSize;//设置要传输的数据项目数
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//设置为存储器到外设模式
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//设置外设为非递增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置存储器为递增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//设置外设数据长度为半字
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//设置存储器数据长度为半字
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//设置为循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//设置为高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//禁止存储器到存储器访问
	
	DMA_Init(DMA2_Channel4,&DMA_InitStructure);//根据参数初始化DMA2_Channel4
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQn;//中断通道号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能中断
	NVIC_Init(&NVIC_InitStructure);//根据参数初始化NVIC
	
	DMA_ITConfig(DMA2_Channel4,DMA_IT_TC,ENABLE);//使能DMA2_Channel4的传输完成中断
	
	DMA_Cmd(DMA2_Channel4,ENABLE);//使能DMA2_Channel3
}
/*********************************************************************************************************
* 函数名称：DMA2_Channel3_IRQHandler
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void DMA2_Channel4_5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_IT_TC4))//判断DMA2_Channel4传输完成中断是否发生
	{
		NVIC_ClearPendingIRQ(DMA2_Channel4_5_IRQn);//清除DMA2_Channel4中断挂起
		DMA_ClearITPendingBit(DMA2_IT_GL4);//清除DMA2_Channel4传输完成中断标志
		
		ConfigDMA2Ch4ForDAC2(s_strDAC2WaveBuf);//配置DMA2通道4
	}
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitDAC
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitDAC(void)
{
	s_strDAC1WaveBuf.waveBufAddr = (u32)GetSineWave100PointAddr();//波形地址
	s_strDAC1WaveBuf.waveBufSize = 100;//波形点数
	s_strDAC2WaveBuf.waveBufAddr = (u32)GetSineWave100PointAddr();//波形地址
	s_strDAC2WaveBuf.waveBufSize = 100;//波形点数
	ConfigDAC1();//配置DAC1
	ConfigDAC2();//配置DAC1
	ConfigTimer4(799,712);//100kHz，计数到800为8ms
	ConfigDMA2Ch3ForDAC1(s_strDAC1WaveBuf);//配置DMA2通道3
	ConfigDMA2Ch4ForDAC2(s_strDAC2WaveBuf);//配置DMA2通道4
}

/*********************************************************************************************************
* 函数名称：SetDACWave
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void SetDAC1Wave(StructDACWave wave)
{
	s_strDAC1WaveBuf = wave;//根据wave设置DAC1波形属性
}

void SetDAC2Wave(StructDACWave wave)
{
	s_strDAC2WaveBuf = wave;//根据wave设置DAC2波形属性
}

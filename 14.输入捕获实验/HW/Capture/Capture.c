/*********************************************************************************************************
* 模块名称：Capture.c
* 摘    要：Capture模块
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
#include "Capture.h"
#include "stm32f10x_conf.h"
/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
 static u8 s_iCaptureSts = 0;//捕获状态
 static u16 s_iCaptureVal;//捕获值
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTIM5ForCapture(u16 arr,u16 psc);//配置TIM3
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTIM5ForCapture
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigTIM5ForCapture(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;//用于存放GPIO的参数
	TIM_TimeBaseInitTypeDef TIMx_TimeBaseStructure;//用于存放定时器的基本参数
	TIM_ICInitTypeDef TIMx_ICInitStructure;//用于存放定时器的通道参数
	NVIC_InitTypeDef NVIC_InitStructure;//用于存放NVIC的参数
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//使能TIM5的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能捕获的GPIOA的时钟
	
	//配置PA0，对应TIM5的CH1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//设置引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置输入模式
	GPIO_Init(GPIOA,&GPIO_InitStructure);//根据参数初始化GPIO
	GPIO_SetBits(GPIOA,GPIO_Pin_0);//将捕获对应的引脚置为高电平
	//GPIO_SetBits 只是 额外保险——把输出寄存器置 1，在输入模式下会开启上拉（有些早期固件库版本必需）。
	
	//配置TIM5
	TIMx_TimeBaseStructure.TIM_Period = arr;//设置计数器的自动重装载值
	//PSC = 71 → 计数频率 = 72 MHz / (71+1) = 1 MHz → 1 μs/计数
	TIMx_TimeBaseStructure.TIM_Prescaler = psc;//设置TIMx时钟频率的预分频值
	//ARR = 0xFFFF → 计数到 65535 后溢出（65.535 ms）
	TIMx_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割
	//TIM_CKD_DIV1 ：数字滤波器采样时钟 = 定时器时钟，不分频。
	TIMx_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//设置定时器TIMx为递增计数模式
	//CounterMode_Up ：从 0 数到 ARR，再回 0（向上计数）。
	TIM_TimeBaseInit(TIM5,&TIMx_TimeBaseStructure);//根据指定的参数初始化TIMx的计数方式
	
	//配置TIM5的CH1为输入捕获
	//CC1S = 01，CC1通道被配置为输入，输入通道IC1映射到定时器引脚TI1上
	TIMx_ICInitStructure.TIM_Channel = TIM_Channel_1;//设置输入通道
	//CC1S = 01 → CH1 被配置为 输入，映射到 TI1（即 PA0）。
	TIMx_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;//设置为下降沿捕获
	TIMx_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//设置为直接映射到TI1
	//ICSelection = DirectTI → 没有交叉映射，直接用 TI1 → IC1。
	TIMx_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;//设置为每一个边沿都捕获，捕捉不分频
	TIMx_ICInitStructure.TIM_ICFilter = 0x08;//设置输入滤波器
	//Filter = 8 → 数字滤波器以定时器时钟采样 8 次，防止毛刺。
	TIM_ICInit(TIM5,&TIMx_ICInitStructure);//根据参数初始化TIM5的CH1
	
	//配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;//中断通道号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//设置子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能中断
	NVIC_Init(&NVIC_InitStructure);//根据参数初始化NVIC
	//  把 TIM5 全局中断号 挂到 NVIC。
  //  抢占优先级 2，子优先级 0（具体数值取决于你工程的优先级分组）。
  //  只有使能了 NVIC，中断向量表里 TIM5_IRQHandler 才会被真正调用。
	
	TIM_ITConfig(TIM5,TIM_IT_Update | TIM_IT_CC1,ENABLE);//使能定时器的更新中断和CC1IE捕获中断
	TIM_Cmd(TIM5,ENABLE);//使能TIM5
}

/*********************************************************************************************************
* 函数名称：TIM5_IRQHandler
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/

void TIM5_IRQHandler(void)
{
	if((s_iCaptureSts & 0x80) == 0)//最高位为0，表示捕获还未完成
	{
		//判断 TIM5 的“更新中断标志位”是否被置位，也就是判断 TIM5 是否刚刚发生了“计数器溢出”（Update Event）。
		if (TIM_GetITStatus(TIM5,TIM_IT_Update) != RESET)
		{
			if(s_iCaptureSts & 0x40)//发生溢出，并且前一次已经捕获到低电平
			{
				//TIM_APR 16位预装载值，即CNT > 65536-1（2^16-1）时溢出
				//若不处理，(s_iCaptureSts & 0x3F)++等于0x40，溢出数等于清零
				if((s_iCaptureSts & 0x3F) == 0x3F)//达到多次溢出，低电平太长
				{
					s_iCaptureSts |= 0x80;//强制标记成功捕获了一次
					s_iCaptureVal = 0xFFFF;//捕获值为0xFFFF
				}
				else
				{
					s_iCaptureSts++;//标记计数器溢出一次
				}
			}
		}
		//判断 TIM5 的“通道1捕获/比较中断标志位”是否被置位，
		//也就是检查 TIM5 通道1（CH1）是否刚刚发生了一次输入捕获事件（如检测到上升沿或下降沿）。
		if(TIM_GetITStatus(TIM5,TIM_IT_CC1) != RESET)//发生捕获事件
		{
			if(s_iCaptureSts & 0x40)//bit6为1，即上次捕获到下降沿，则这次捕获到上升沿
			{
				s_iCaptureSts |= 0x80;//完成捕获，标记成功捕获到一次上升沿
				//在 输入捕获模式 下，当外部信号触发（如上升沿或下降沿）时，
				//定时器会自动把 当前的计数器值（CNT） 保存到 捕获比较寄存器（CCR1） 中
				s_iCaptureVal = TIM_GetCapture1(TIM5);//s_iCapture记录捕获比较寄存器的值
				//CC1P = 1，设置为下降沿捕获，为下次捕获做准备
				TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling);
			}
			else//bit6为0，表示未捕获到下降沿，这是第一次捕获到下降沿
			{
				s_iCaptureSts = 0;//清空溢出次数
				s_iCaptureVal = 0;//捕获值为0
				
				TIM_SetCounter(TIM5,0);//设置寄存器的值为0
				
				s_iCaptureSts |= 0x40;//bit6置为1，标记捕获到了下降沿
				TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising);//CC1P = 0，设置为上升沿捕获
			}
		}
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_CC1|TIM_IT_Update);//清除中断标志
}
/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitCapture
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitCapture(void)
{
	ConfigTIM5ForCapture(0xFFFF,72-1);
}

/*********************************************************************************************************
* 函数名称：gETcAPTURE
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u8 GetCaptureVal(i32* pCapVal)
{
	u8 ok = 0;
	if(s_iCaptureSts & 0x80)
	{
		ok = 1;
		(*pCapVal) = s_iCaptureSts & 0x3F;
		(*pCapVal) *= 65536;
		//读取 TIM5 通道 1（CH1）的“捕获比较寄存器”CCR1 的当前值，
		//并将其保存到变量 s_iCaptureVal 中。
		(*pCapVal) += s_iCaptureVal;
		
		s_iCaptureSts = 0;
	}
	return(ok);
}

/*********************************************************************************************************
* 模块名称：KeyOne.c
* 摘    要：KeyOne模块，进行独立按键初始化，以及按键扫描函数实现
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
#include "KeyOne.h"
#include "stm32f10x_conf.h"


/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//KEY1为读取PC1引脚电平
#define KEY1				(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1))
//KEY1为读取PC1引脚电平
#define KEY2				(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2))
//KEY1为读取PC1引脚电平
#define KEY3				(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
 //按键按下时的电压，0xFF表示按下时为高电平，0x00表示按下时为低电平

 static u8 s_arrKeyDownLevel[KEY_NAME_MAX];			//使用前要在InitKeyOne函数中进行初始化
 static u8 s_arrKeyFlag[KEY_NAME_MAX];//定义一个u8类型的数组，用于存放按键的标志位
/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigKeyOneGPIO(void);					//配置按键的GPIO
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigKeyOneGPIO(void)
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigKeyOneGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructrue;		//GPIO_InitStructrue用于存放GPIO的参数
	
	
	//使能RCC相关时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能GPIOC的时钟
	
	//配置PC1
	GPIO_InitStructrue.GPIO_Pin = GPIO_Pin_1;						//设置引脚
	GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_IPU;				//设置输入类型,设置为上拉输入模式
	GPIO_Init(GPIOC,&GPIO_InitStructrue);								//根据参数初始化GPIO
	//配置PC2
	GPIO_InitStructrue.GPIO_Pin = GPIO_Pin_2;						//设置引脚
	GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_IPU;				//设置输入类型,设置为上拉输入模式
	GPIO_Init(GPIOC,&GPIO_InitStructrue);								//根据参数初始化GPIO
	//配置PA0
	GPIO_InitStructrue.GPIO_Pin = GPIO_Pin_0;						//设置引脚
	GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_IPU;				//设置输入类型,设置为上拉输入模式
	GPIO_Init(GPIOA,&GPIO_InitStructrue);								//根据参数初始化GPIO
}
/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitKeyOne
* 函数功能：初始化KeyOne模块
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitKeyOne(void)
{
	
	i16 i = 0;
	ConfigKeyOneGPIO();																	//配置按键的GPIO
	
	s_arrKeyDownLevel[KEY_NAME_KEY1] = KEY_DOWN_LEVEL_KEY1;
																											//按键KEY1按下时为低电平
	s_arrKeyDownLevel[KEY_NAME_KEY2] = KEY_DOWN_LEVEL_KEY2;
																											//按键KEY2按下时为低电平
	s_arrKeyDownLevel[KEY_NAME_KEY3] = KEY_DOWN_LEVEL_KEY3;
																											//按键KEY3按下时为低电平
	for (i=0; i < KEY_NAME_MAX; ++i)
	{
    s_arrKeyFlag[i] = TRUE;
	}
}

/*********************************************************************************************************
* 函数名称：ScanKeyOne(u8 keyName,void(*OnKeyOneUp)(void),void(*OnKeyOneDown)(void))
* 函数功能：按键扫描函数
* 输入参数：keyName、OnKeyOneUp、OnKeyOneDown
* 输出参数：无
* 返 回 值：无
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void ScanKeyOne(u8 keyName,void(*OnKeyOneUp)(void),void(*OnKeyOneDown)(void))
{
	static u8 s_arrKeyVal[KEY_NAME_MAX];//定义一个u8类型的数组，用于存放按键的数值

	
	
	s_arrKeyVal[keyName] = s_arrKeyVal[keyName]<<1;		//左移一位
	switch(keyName)
	{
		case KEY_NAME_KEY1:
			s_arrKeyVal[keyName] = s_arrKeyVal[keyName]|KEY1;//按下/松开时，KEY1为0/1
			break;
		case KEY_NAME_KEY2:
			s_arrKeyVal[keyName] = s_arrKeyVal[keyName]|KEY2;//按下/松开时，KEY2为0/1
			break;
		case KEY_NAME_KEY3:
			s_arrKeyVal[keyName] = s_arrKeyVal[keyName]|KEY3;//按下/松开时，KEY3为0/1
			break;
		default:
			break;
	}
	
	//按键标志位的值为TRUE时，判断是否有按键有效按下
	if(s_arrKeyVal[keyName] == s_arrKeyDownLevel[keyName] && s_arrKeyFlag[keyName] == TRUE)
	{
		(*OnKeyOneDown)();
		s_arrKeyFlag[keyName] = FALSE;//表示按键处于按下状态，按键标记值改为FALSE					
	}
	else if(s_arrKeyVal[keyName] == (u8)(~s_arrKeyDownLevel[keyName]) && s_arrKeyFlag[keyName] == FALSE)
	{
		(*OnKeyOneUp)();
		s_arrKeyFlag[keyName] = TRUE;//表示按键处于松开状态，按键标记值改为TRUE
	}
}

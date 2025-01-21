//用串口发送数据给单片机在菜单上实现不同功能 E,R,N分别是进入，返回，下一项
//num1,2,3,分别是计算器，灯，秒表
/*计算器可以实现加减乘除一起计算，输入F，得结果。在屏幕右边显示记录的结果，输入C，记录清除，N开始下一次计算
输入R返回上一界面*/
/*灯进入三级界面选择其中一个即可以实现功能，输入R可以退出*/
/*秒表功能，输入A开始计时，输入B暂停，C清零，D记录时间，R退出*/
//董展宏
#include "stm32f10x.h"                
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"                  
#include "pwm.h"
#include "Timer.h"
#include "caidann.h"
uint8_t RxData;	
extern int shouldReturn;
uint8_t taskIndex =0;	//任务调度序号
typedef struct
{
	uint8_t CurrentNum;	//当前索引序号:页码
	uint8_t Enter;		//确认键
	uint8_t Next;		//下一个
	uint8_t Return;		//返回键
	void (*Current_Operation)(void);//当前操作(函数指针)
}Menu_table_t;
 
Menu_table_t taskTable[] =
{//当前索引序号:页码//确认键//下一个//返回键
    //菜单界面函数 -- 一级界面
    {0, 1, 0, 0, Menu_Interface}, 
    //功能界面函数 -- 二级界面
    {1, 4, 2, 0, Function_Interface1},
    {2, 5, 3, 0, Function_Interface2},
    {3, 7, 1, 0, Function_Interface3},
	//功能设置界面函数 -- 三级界面
	{4, 8, 8, 1, Function_Interface11},//SUANFA
	 
	{5, 9, 6, 2, Function_Interface21},//基本流水灯
	{6, 10, 5, 2, Function_Interface22},//呼吸流水灯
	
	{7 ,7, 7, 3, Function_Interface31},//时钟 
	//四级页面
	 {8, 8, 8, 4, Function_Interface111},
	 {9, 9, 9, 5, Function_Interface211},//基本流水灯
	 {10, 10, 10, 6, Function_Interface221},//呼吸流水灯
};
int main(void)
{
	
	 OLED_Init();	
	 OLED_ShowString(3, 7, "MENU");
	 Serial_Init();
	while (1)
	{  
 		if (Serial_GetRxFlag() == 1)			//检查串口接收数据的标志位
 		{  
			RxData = Serial_GetRxData();			//获取串口接收的数据
			if(RxData=='E')			 
       {  
	    taskIndex = taskTable[taskIndex].Enter;
	    OLED_Clear();	
        while(!RxData);
        }
	       if(RxData=='R')			 
       {
		   taskIndex = taskTable[taskIndex].Return;
	    OLED_Clear();	
        while(!RxData);
	   }	
           if(RxData=='N')			 
      {
	    taskIndex = taskTable[taskIndex].Next;
        while(!RxData);
        OLED_Clear();
	  }
	    Serial_SendByte(RxData);
		taskTable[taskIndex].Current_Operation();//执行函数
		 if (shouldReturn)
            {
                shouldReturn = 0;  // 清除返回标志
                taskIndex = taskTable[taskIndex].Return;  // 返回上一个页面
                OLED_Clear();
            }      
        }
		}	     
	}

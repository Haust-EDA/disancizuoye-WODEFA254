#include "stm32f10x.h"     
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"                  
#include "pwm.h"
#include "Timer.h"// Device header
uint8_t RxData;	
 int hour = 0, min = 0, sec = 0;      // 当前时间
int hour1 = 0, min1 = 0, sec1 = 0;   // 记录的时间
int isRunning = 0;                   // 秒表运行标志
int isUpdated = 0;                   // 屏幕刷新标志
 #define MAX_RECORDS 5                // 最大记录数量
int recordedTimes[MAX_RECORDS][3];   // 存储记录的时间（小时、分钟、秒）
int recordCount = 0;                 // 当前记录数量 
 
#define MAX_DATA_SIZE 100   
int shouldReturn = 0;                // 返回上一个页面的标志

uint8_t DataArray[MAX_DATA_SIZE];  // 存储接收到的数据的数组
uint8_t DataIndex = 0;             // 数组索引
uint32_t Sum = 0;                   
#define MAX_RESULTS 10  // 最大记录结果数量
int calculationResults[MAX_RESULTS];  // 存储计算结果
int resultCount = 0;                  // 当前记录结果数量
 
 
 void Menu_Interface(void)
{
 OLED_ShowString(3, 7, "MENU");
}
 
 //一层
void Function_Interface1(void)
{   OLED_ShowString(2, 0,   " num1");
	OLED_ShowString(3, 0,   " num2");
	OLED_ShowString(4, 0,   " num3");
	OLED_ShowString(2, 7,   "*");
}
void Function_Interface2(void)
{   OLED_ShowString(2, 0,   " num1");
	OLED_ShowString(3, 0,   " num2");
	OLED_ShowString(4, 0,   " num3");
	OLED_ShowString(3, 7,   "*");
}
void Function_Interface3(void)
{   OLED_ShowString(2, 0,   " num1");
	OLED_ShowString(3, 0,   " num2");
	OLED_ShowString(4, 0,   " num3");
	OLED_ShowString(4, 7,   "*");
}
 
 //二层
void Function_Interface11(void)
{    
	 OLED_ShowString(1, 1,   "+");
	 OLED_ShowString(2, 1,   "-");
	 OLED_ShowString(3, 1,   "*");
	 OLED_ShowString(4, 1,   "/");
}
  void Function_Interface111(void) {
    char expression[MAX_DATA_SIZE];   
    int index = 0;
    int resultHistory[MAX_DATA_SIZE];  
    int historyIndex = 0;   

    
    OLED_Clear();
    OLED_ShowString(1, 1, "Input:");   

     
    while (1) {
        if (Serial_GetRxFlag() == 1) {   
            char receivedData = Serial_GetRxData();   

            if (receivedData == 'F') {  // 输入 'F' 结束
                
                int numbers[MAX_DATA_SIZE];  // 存储数字
                char operators[MAX_DATA_SIZE];  // 存储运算符
                int numIndex = 0, opIndex = 0;

                int i = 0;
                while (expression[i] != '\0') {
                    if (expression[i] >= '0' && expression[i] <= '9') {
                        // 解析数字
                        int num = 0;
                        while (expression[i] >= '0' && expression[i] <= '9') {
                            num = num * 10 + (expression[i] - '0');
                            i++;
                        }
                        numbers[numIndex++] = num;
                    } else if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/') {
                        // 处理运算符优先级
                        while (opIndex > 0 && (operators[opIndex - 1] == '*' || operators[opIndex - 1] == '/')) {
                            char op = operators[--opIndex];
                            int num2 = numbers[--numIndex];
                            int num1 = numbers[--numIndex];
                            if (op == '*') {
                                numbers[numIndex++] = num1 * num2;
                            } else if (op == '/') {
                                numbers[numIndex++] = num1 / num2;
                            }
                        }
                        operators[opIndex++] = expression[i++];
                    } else {
                        // 忽略无效字符
                        i++;
                    }
                }

                // 处理剩余的运算符
                while (opIndex > 0) {
                    char op = operators[--opIndex];
                    int num2 = numbers[--numIndex];
                    int num1 = numbers[--numIndex];
                    if (op == '+') {
                        numbers[numIndex++] = num1 + num2;
                    } else if (op == '-') {
                        numbers[numIndex++] = num1 - num2;
                    } else if (op == '*') {
                        numbers[numIndex++] = num1 * num2;
                    } else if (op == '/') {
                        numbers[numIndex++] = num1 / num2;
                    }
                }

                // 显示最终结果
                int result = numbers[0];
                OLED_ShowNum(3, 1, result, 4);   

                // 记录结果到历史记录
                if (historyIndex < MAX_DATA_SIZE) {
                    resultHistory[historyIndex++] = result;
                }

                // 显示历史记录（最右侧一列）
                for (int row = 1; row <= 4; row++) {  // 假设 OLED 有 4 行
                    if (row <= historyIndex) {
                        OLED_ShowNum(row, 12, resultHistory[row - 1], 4);  // 显示历史记录
                    } else {
                        OLED_ShowString(row, 12, "    ");  // 清空未使用的行
                    }
                }

                // 等待用户输入 'N'、'R' 或 'C'
                while (1) {
                    if (Serial_GetRxFlag() == 1) {
                        char nextCommand = Serial_GetRxData();
                        if (nextCommand == 'N') {  // 进入下一次计算
                            index = 0;  // 重置索引
                            for (int col = 0; col < 16; col++) {  // 清空输入行
                                OLED_ShowChar(2, col, ' ');
                            }
                            for (int col = 0; col < 16; col++) {  // 清空结果行
                                OLED_ShowChar(3, col, ' ');
                            }
                            OLED_ShowString(1, 1, "Input:");  // 显示输入提示
                            break;
                        } else if (nextCommand == 'R') {  // 返回上一个页面
                            return;  // 退出函数，返回上一个页面
                        } else if (nextCommand == 'C') {  // 清除历史记录
                            historyIndex = 0;  // 重置历史记录索引
                            for (int row = 1; row <= 4; row++) {  // 清空历史记录列
                                OLED_ShowString(row, 12, "    ");
                            }
                        }
                    }
                }
            } else {
                
                expression[index++] = receivedData;
                expression[index] = '\0';  // 添加字符串结束符

                // 实时显示当前输入
                OLED_ShowChar(2, 1 + (index - 1), receivedData);  
            }
        }
    }
}
void Function_Interface21(void)
{
	OLED_ShowString(2, 1, "LIUSHUIDENG:");
	OLED_ShowString(3, 1, "HUXIDENG:");
	OLED_ShowString(2, 15, "*");
} 
void Function_Interface22(void)
{
	OLED_ShowString(2, 1, "LIUSHUIDENG:");
	OLED_ShowString(3, 1, "HUXIDENG:");
	OLED_ShowString(3, 15, "*");
	 
}
void Function_Interface211(void)
{
	Liushuideng();
} 
void Function_Interface221(void)
{
	Huxi();
	 
}

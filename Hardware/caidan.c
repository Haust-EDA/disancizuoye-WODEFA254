#include "stm32f10x.h"     
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"                  
#include "pwm.h"
#include "Timer.h"// Device header
uint8_t RxData;	
 int hour = 0, min = 0, sec = 0;      // ��ǰʱ��
int hour1 = 0, min1 = 0, sec1 = 0;   // ��¼��ʱ��
int isRunning = 0;                   // ������б�־
int isUpdated = 0;                   // ��Ļˢ�±�־
 #define MAX_RECORDS 5                // ����¼����
int recordedTimes[MAX_RECORDS][3];   // �洢��¼��ʱ�䣨Сʱ�����ӡ��룩
int recordCount = 0;                 // ��ǰ��¼���� 
 
#define MAX_DATA_SIZE 100   
int shouldReturn = 0;                // ������һ��ҳ��ı�־

uint8_t DataArray[MAX_DATA_SIZE];  // �洢���յ������ݵ�����
uint8_t DataIndex = 0;             // ��������
uint32_t Sum = 0;                   
#define MAX_RESULTS 10  // ����¼�������
int calculationResults[MAX_RESULTS];  // �洢������
int resultCount = 0;                  // ��ǰ��¼�������
 
 
 void Menu_Interface(void)
{
 OLED_ShowString(3, 7, "MENU");
}
 
 //һ��
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
 
 //����
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

            if (receivedData == 'F') {  // ���� 'F' ����
                
                int numbers[MAX_DATA_SIZE];  // �洢����
                char operators[MAX_DATA_SIZE];  // �洢�����
                int numIndex = 0, opIndex = 0;

                int i = 0;
                while (expression[i] != '\0') {
                    if (expression[i] >= '0' && expression[i] <= '9') {
                        // ��������
                        int num = 0;
                        while (expression[i] >= '0' && expression[i] <= '9') {
                            num = num * 10 + (expression[i] - '0');
                            i++;
                        }
                        numbers[numIndex++] = num;
                    } else if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/') {
                        // ������������ȼ�
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
                        // ������Ч�ַ�
                        i++;
                    }
                }

                // ����ʣ��������
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

                // ��ʾ���ս��
                int result = numbers[0];
                OLED_ShowNum(3, 1, result, 4);   

                // ��¼�������ʷ��¼
                if (historyIndex < MAX_DATA_SIZE) {
                    resultHistory[historyIndex++] = result;
                }

                // ��ʾ��ʷ��¼�����Ҳ�һ�У�
                for (int row = 1; row <= 4; row++) {  // ���� OLED �� 4 ��
                    if (row <= historyIndex) {
                        OLED_ShowNum(row, 12, resultHistory[row - 1], 4);  // ��ʾ��ʷ��¼
                    } else {
                        OLED_ShowString(row, 12, "    ");  // ���δʹ�õ���
                    }
                }

                // �ȴ��û����� 'N'��'R' �� 'C'
                while (1) {
                    if (Serial_GetRxFlag() == 1) {
                        char nextCommand = Serial_GetRxData();
                        if (nextCommand == 'N') {  // ������һ�μ���
                            index = 0;  // ��������
                            for (int col = 0; col < 16; col++) {  // ���������
                                OLED_ShowChar(2, col, ' ');
                            }
                            for (int col = 0; col < 16; col++) {  // ��ս����
                                OLED_ShowChar(3, col, ' ');
                            }
                            OLED_ShowString(1, 1, "Input:");  // ��ʾ������ʾ
                            break;
                        } else if (nextCommand == 'R') {  // ������һ��ҳ��
                            return;  // �˳�������������һ��ҳ��
                        } else if (nextCommand == 'C') {  // �����ʷ��¼
                            historyIndex = 0;  // ������ʷ��¼����
                            for (int row = 1; row <= 4; row++) {  // �����ʷ��¼��
                                OLED_ShowString(row, 12, "    ");
                            }
                        }
                    }
                }
            } else {
                
                expression[index++] = receivedData;
                expression[index] = '\0';  // ����ַ���������

                // ʵʱ��ʾ��ǰ����
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

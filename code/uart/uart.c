#include <STC8G.H>
#include <stdio.h>

void UartInit(void) // 115200bps@11.0592MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0xFE;			//设置定时初始值
	T2H = 0xFF;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时

	P_SW1 = (P_SW1 & 0x3F) | (0 << 6); 
	
}

void Uart_SendChar(unsigned char dat)
{
	SBUF = dat;
	while (!TI);
	TI = 0;
}

char putchar(char c) // 重定�?
{
	Uart_SendChar(c);
	return c;
}


void Uart1_Isr(void) interrupt 4
{
    if (TI) // 检测串口1发送中断
    {
        TI = 0; // 清除串口1发送中断请求位
    }
    if (RI) // 检测串口1接收中断
    {

      
        RI = 0; // 清除串口1接收中断请求位
    }
}
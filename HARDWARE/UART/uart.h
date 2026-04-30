#ifndef __UART_H
#define __UART_H
#include "sys.h"

// 串口6接收缓冲大小
#define USART6_REC_LEN  		200  	//定义最大接收字节数 200
#define EN_USART6_RX 			1		//使能（1）/禁止（0）串口6接收

// 变长数据包接收相关
#define USART6_USE_IDLE_RX    1     // 使用IDLE空闲中断接收变长数据

extern u8  USART6_RX_BUF[USART6_REC_LEN]; //串口6接收缓冲
extern volatile u8 USART6_RX_FLAG;     // 接收完成标志
extern volatile u16 USART6_RX_LEN;     // 接收到的数据长度

void USART2_Init(void);  // RFID模块串口初始化 (PD5/PD6)
void USART3_Init(void);  // 语音模块串口初始化 (PB10/PB11)
void uart6_init(u32 bound);  // 指纹模块串口初始化 (PC6/PC7)
void uart6_send_string(u8* string,u8 len);
void uart6_rx_clear(void);   // 清空串口6接收缓冲区

#endif

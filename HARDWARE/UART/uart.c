#include "uart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include <stdio.h>

// 串口6相关变量定义
#if EN_USART6_RX
u8 USART6_RX_BUF[USART6_REC_LEN];
volatile u8 USART6_RX_FLAG = 0;
volatile u16 USART6_RX_LEN = 0;
#endif

/*
 * USART2初始化 - 用于RFID刷卡模块
 * 波特率：115200
 * 引脚：PD5(TX), PD6(RX)
 */
void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能GPIOD和USART2时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 配置PD5(TX), PD6(RX)为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // 配置引脚复用为USART2
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

    // USART2配置
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    // 使能接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能USART2
    USART_Cmd(USART2, ENABLE);
}

/*
 * USART3初始化 - 用于语音模块
 * 波特率：9600
 * 引脚：PB10(TX), PB11(RX)
 */
void USART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能GPIOB和USART3时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // 配置PB10(TX), PB11(RX)为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置引脚复用为USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    // USART3配置
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);

    // 使能接收中断（如需要）
    // USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能USART3
    USART_Cmd(USART3, ENABLE);
}

/*
 * USART6初始化 - 用于AS608指纹模块
 * 波特率：115200
 * 引脚：PC6(TX), PC7(RX)
 */
void uart6_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	// 使能GPIOC和USART6时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
 
	// 串口6对应引脚复用映射
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); //PC6复用为USART6_TX
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6); //PC7复用为USART6_RX
	
	// USART6端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// USART6初始化设置
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART6, &USART_InitStructure);
	
	USART_Cmd(USART6, ENABLE);
	USART_ClearFlag(USART6, USART_FLAG_TC);
	
#if EN_USART6_RX	
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
#if USART6_USE_IDLE_RX
	USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);
#endif

	// NVIC配置
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

// 串口6发送数据串
void uart6_send_string(u8* string, u8 len)
{
	u8 i;
	for(i = 0; i < len; i++)
	{
		USART_SendData(USART6, string[i]);
		while(USART_GetFlagStatus(USART6, USART_FLAG_TC) == 0);
	}
}

// 清空串口6接收缓冲区
void uart6_rx_clear(void)
{
#if USART6_USE_IDLE_RX
	USART6_RX_LEN = 0;
	USART6_RX_FLAG = 0;
#else
	// 非IDLE模式不需要额外处理
#endif
}

#if EN_USART6_RX
// 串口6中断服务程序
void USART6_IRQHandler(void)
{
	u8 Res;
	
	// 接收中断
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART6);

#if USART6_USE_IDLE_RX
		// 使用IDLE方式接收变长数据
		if(USART6_RX_LEN < USART6_REC_LEN)
		{
			USART6_RX_BUF[USART6_RX_LEN++] = Res;
		}
#else
		// 可以添加其他接收方式的处理
#endif
	}

#if USART6_USE_IDLE_RX
	// 空闲中断 - 一包数据接收完成
	if(USART_GetITStatus(USART6, USART_IT_IDLE) != RESET)
	{
		// 清除IDLE标志：先读SR，再读DR
		Res = USART6->SR;
		Res = USART6->DR;
		USART6_RX_FLAG = 1;
	}
#endif
}
#endif

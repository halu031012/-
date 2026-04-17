#include "key.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/*
 * 矩阵键盘引脚定义（STM32F407VET6）
 * 行输出（推挽输出）：
 * R4 - PD11
 * R3 - PD9
 * R2 - PE15
 * R1 - PE13
 *
 * 列输入（上拉输入）：
 * C1 - PE11
 * C2 - PE9
 * C3 - PE7
 * C4 - PC5
 */

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOD时钟（PD9, PD11）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    // 配置PD9, PD11为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // 使能GPIOE时钟（PE7, PE9, PE11, PE13, PE15）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    
    // 配置PE13, PE15为推挽输出（行输出R1, R2）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    // 配置PE7, PE9, PE11为上拉输入（列输入C3, C2, C1）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    // 使能GPIOC时钟（PC5）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    // 配置PC5为上拉输入（列输入C4）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // 初始化所有行线为高电平
    GPIO_SetBits(GPIOD, GPIO_Pin_9 | GPIO_Pin_11);
    GPIO_SetBits(GPIOE, GPIO_Pin_13 | GPIO_Pin_15);
}

u8 KEY_Read(void)
{
    u8 temp = 0;
    
    // 扫描第4行（R4 - PD11）
    GPIO_ResetBits(GPIOD, GPIO_Pin_11);
    GPIO_SetBits(GPIOD, GPIO_Pin_9);
    GPIO_SetBits(GPIOE, GPIO_Pin_15 | GPIO_Pin_13);
    delay_ms(1); // 消抖延时
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 13;  // C1 - 清除键
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 14;   // C2 - 数字0
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 15;   // C3
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 16;   // C4 - 确认键
    
    // 扫描第3行（R3 - PD9）
    GPIO_SetBits(GPIOD, GPIO_Pin_11);
    GPIO_ResetBits(GPIOD, GPIO_Pin_9);
    GPIO_SetBits(GPIOE, GPIO_Pin_15 | GPIO_Pin_13);
    delay_ms(1);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 9;   // C1 - 数字7
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 10;  // C2 - 数字8
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 11;  // C3 - 数字9
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 12;  // C4
    
    // 扫描第2行（R2 - PE15）
    GPIO_SetBits(GPIOD, GPIO_Pin_11 | GPIO_Pin_9);
    GPIO_ResetBits(GPIOE, GPIO_Pin_15);
    GPIO_SetBits(GPIOE, GPIO_Pin_13);
    delay_ms(1);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 5;   // C1 - 数字4
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 6;   // C2 - 数字5
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 7;   // C3 - 数字6
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 8;   // C4
    
    // 扫描第1行（R1 - PE13）
    GPIO_SetBits(GPIOD, GPIO_Pin_11 | GPIO_Pin_9);
    GPIO_SetBits(GPIOE, GPIO_Pin_15);
    GPIO_ResetBits(GPIOE, GPIO_Pin_13);
    delay_ms(1);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) temp = 1;   // C1 - 数字1
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) temp = 2;   // C2 - 数字2
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) temp = 3;   // C3 - 数字3
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == 0) temp = 4;   // C4
    
    // 恢复所有行线为高电平
    GPIO_SetBits(GPIOD, GPIO_Pin_9 | GPIO_Pin_11);
    GPIO_SetBits(GPIOE, GPIO_Pin_13 | GPIO_Pin_15);
    
    return temp;
}

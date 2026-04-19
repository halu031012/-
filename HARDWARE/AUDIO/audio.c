#include "audio.h"
#include "uart.h"
#include "delay.h"
#include "stm32f4xx_usart.h"

/*
 * 语音模块初始化
 * 使用USART3与语音模块通信
 */
void Audio_Init(void)
{
    USART3_Init();
    // 语音模块上电后需要较长的启动时间
    // JQ8400模块通常需要1000-1500ms才能完全就绪
    delay_ms(1200);
    
    // 发送唤醒/同步命令（查询模块状态，不发声音）
    // 模块上电后第一次通信用于唤醒串口接收，实际命令在第二次才生效
    Audio_WakeUp();
}

/*
 * 语音模块唤醒
 * 发送空命令用于同步和唤醒模块，不产生声音
 */
void Audio_WakeUp(void)
{
    u8 cmd[7];
    u8 i;
    
    // 构建查询状态命令（命令0x01，无声音）
    cmd[0] = 0x7E;      // 帧头
    cmd[1] = 0x03;      // 长度
    cmd[2] = 0x01;      // 查询状态命令
    cmd[3] = 0x00;      // 反馈
    cmd[4] = 0x00;      // 无参数
    cmd[5] = cmd[1] ^ cmd[2] ^ cmd[3] ^ cmd[4]; // 校验和
    cmd[6] = 0xEF;      // 帧尾
    
    // 发送命令
    for(i = 0; i < 7; i++)
    {
        USART_SendData(USART3, cmd[i]);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
    }
    delay_ms(50);  // 等待模块响应
}

/*
 * 播放指定语音
 * num: 语音编号 (1-21)
 * 通信协议：帧头(0x7E) + 长度(0x05) + 命令(0x41) + 反馈(0x00) + 参数(语音编号) + 校验和 + 帧尾(0xEF)
 */
void Audio_Play(u8 num)
{
    u8 cmd[7];
    u8 i;
    
    // 构建命令帧
    cmd[0] = 0x7E;                              // 帧头
    cmd[1] = 0x05;                              // 长度
    cmd[2] = 0x41;                              // 命令字
    cmd[3] = 0x00;                              // 反馈
    cmd[4] = num;                               // 语音编号
    cmd[5] = cmd[1] ^ cmd[2] ^ cmd[3] ^ cmd[4]; // 校验和
    cmd[6] = 0xEF;                              // 帧尾
    
    // 发送命令
    for(i = 0; i < 7; i++)
    {
        USART_SendData(USART3, cmd[i]);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); // 等待发送完成
    }
    
    // 添加延时，确保命令被模块接收处理
    delay_ms(10);
}

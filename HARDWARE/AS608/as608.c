#include "as608.h"
#include "uart.h"
#include "delay.h"
#include <stddef.h>   // 包含 NULL 的定义

// WAK 引脚定义 (PD1)
#define AS608_WAK_PIN   GPIO_Pin_1
#define AS608_WAK_PORT  GPIOD

// 初始化 AS608 GPIO (WAK 引脚 PD1)
void As608_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能 GPIOD 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	// 配置 PD1 为输入模式 (WAK 引脚)
	GPIO_InitStructure.GPIO_Pin = AS608_WAK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;      // 输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  // 浮空输入
	GPIO_Init(AS608_WAK_PORT, &GPIO_InitStructure);
}

// 读取 WAK 引脚状态
// 返回: 0-低电平, 1-高电平
u8 As608_Wak_Read(void)
{
	return GPIO_ReadInputDataBit(AS608_WAK_PORT, AS608_WAK_PIN);
}

void as608_inti()
{
	As608_Gpio_Init();      // 初始化 WAK 引脚
	uart6_init(115200);     // 初始化串口6
}

// 等待AS608响应，超时时间ms
// 返回: 0-成功接收到数据, 1-超时
u8 AS608_WaitResponse(u16 timeout_ms)
{
	u16 cnt = 0;
	while(!USART6_RX_FLAG && cnt < timeout_ms)
	{
		delay_ms(1);
		cnt++;
	}
	return (USART6_RX_FLAG && USART6_RX_LEN >= 12) ? 0 : 1;
}

//获取指纹图像,并存入图像缓存区
//返回: 确认码 (0-成功, 1-失败, 2-无手指, 0xFF-超时)
u8 PS_GetImage(void)
{
	u8 string[]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x01,0x00,0x05};
	
	uart6_rx_clear();  // 清空接收缓冲区
	uart6_send_string(string,12);
	
	if(AS608_WaitResponse(500) == 0)
	{
		return USART6_RX_BUF[9];  // 返回确认码
	}
	return 0xFF;  // 超时
}

//生成特征, buffer=1或2
//返回: 确认码 (0-成功, 其他-失败, 0xFF-超时)
u8 Ps_GenChar(u8 buffer)
{
	u8 string[13];
	u8 checksum;
	
	// 固定部分
    string[0]  = 0xEF;
    string[1]  = 0x01;
    string[2]  = 0xFF;
    string[3]  = 0xFF;
    string[4]  = 0xFF;
    string[5]  = 0xFF;
    string[6]  = 0x01;
    string[7]  = 0x00;
    string[8]  = 0x04;
    string[9]  = 0x02;
    string[10] = buffer;
    string[11] = 0x00;
	
	// 校验和单独计算
    checksum = 0x01 + 0x00 + 0x04 + 0x02 + buffer;
    string[12] = checksum;
	
	uart6_rx_clear();  // 清空接收缓冲区
    uart6_send_string(string,13);
	
	if(AS608_WaitResponse(500) == 0)
	{
		return USART6_RX_BUF[9];  // 返回确认码
	}
	return 0xFF;  // 超时
}

//合并特征(Buffer1+Buffer2→Buffer2)
//返回: 确认码 (0-成功, 其他-失败, 0xFF-超时)
u8 PS_RegModel(void)
{
    u8 string[]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x05,0x00,0x09};
    
	uart6_rx_clear();  // 清空接收缓冲区
    uart6_send_string(string,12);
	
	if(AS608_WaitResponse(500) == 0)
	{
		return USART6_RX_BUF[9];  // 返回确认码
	}
	return 0xFF;  // 超时
}

//清空指纹库（删除所有已存指纹）
//返回: 确认码 (0-成功, 其他-失败, 0xFF-超时)
u8 PS_Empty(void)
{
    u8 string[]={0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x0d,0x00,0x11};
    uart6_rx_clear();
    uart6_send_string(string,12);
    if(AS608_WaitResponse(500) == 0) return USART6_RX_BUF[9];
    return 0xFF;
}

//存储模板到Flash数据库
//bufferID: 1或2 (CharBuffer1或CharBuffer2)
//pageID: 存储位置 (0-299)
//返回: 确认码 (0-成功, 其他-失败, 0xFF-超时)
u8 PS_StoreChar(u8 bufferID, u16 pageID)
{
	u8 string[15];
	u8 checksum;
	
	// 固定部分
	string[0] = 0xEF;
	string[1] = 0x01;
	string[2] = 0xFF;
	string[3] = 0xFF;
	string[4] = 0xFF;
	string[5] = 0xFF;
	string[6] = 0x01;           // 包标识
	string[7] = 0x00;           // 包长度高字节
	string[8] = 0x06;           // 包长度低字节 (6字节)
	string[9] = 0x06;           // 命令字: StoreChar
	string[10] = bufferID;      // 缓冲区号: 1或2
	string[11] = (pageID >> 8) & 0xFF;  // PageID高字节
	string[12] = pageID & 0xFF;         // PageID低字节
	
	// 计算校验和: 包标识(1) + 包长度(2) + 命令字(1) + 缓冲区号(1) + PageID(2)
	checksum = 0x01 + 0x00 + 0x06 + 0x06 + bufferID + string[11] + string[12];
	string[13] = 0x00;          // 校验和高字节(通常为0，因为和小于256)
	string[14] = checksum;      // 校验和低字节
	
	uart6_rx_clear();  // 清空接收缓冲区
	uart6_send_string(string, 15);
	
	if(AS608_WaitResponse(500) == 0)
	{
		return USART6_RX_BUF[9];  // 返回确认码
	}
	return 0xFF;  // 超时
}

//搜索指纹
//bufferID: 1或2 (CharBuffer1或CharBuffer2)
//startPage: 起始页 (0-299)
//pageNum: 搜索页数 (1-300)
//matchPage: 输出参数，匹配到的页码（指针）
//matchScore: 输出参数，匹配分数（指针）
//返回: 确认码 (0-搜索到, 9-没搜索到, 其他-错误, 0xFF-超时)
u8 PS_Search(u8 bufferID, u16 startPage, u16 pageNum, u16* matchPage, u16* matchScore)
{
	u8 string[17];
	u16 checksum;
	
	// 固定部分
	string[0] = 0xEF;
	string[1] = 0x01;
	string[2] = 0xFF;
	string[3] = 0xFF;
	string[4] = 0xFF;
	string[5] = 0xFF;
	string[6] = 0x01;           // 包标识
	string[7] = 0x00;           // 包长度高字节
	string[8] = 0x08;           // 包长度低字节 (8字节)
	string[9] = 0x04;           // 命令字: Search
	string[10] = bufferID;      // 缓冲区号: 1或2
	string[11] = (startPage >> 8) & 0xFF;  // 起始页高字节
	string[12] = startPage & 0xFF;         // 起始页低字节
	string[13] = (pageNum >> 8) & 0xFF;    // 搜索页数高字节
	string[14] = pageNum & 0xFF;           // 搜索页数低字节
	
	// 计算校验和
	checksum = 0x01 + 0x00 + 0x08 + 0x04 + bufferID + string[11] + string[12] + string[13] + string[14];
	string[15] = (checksum >> 8) & 0xFF;   // 校验和高字节
	string[16] = checksum & 0xFF;          // 校验和低字节
	
	uart6_rx_clear();  // 清空接收缓冲区
	uart6_send_string(string, 17);
	
	if(AS608_WaitResponse(500) == 0)
	{
		u8 result = USART6_RX_BUF[9];  // 确认码
		
		// 如果搜索成功，提取匹配页码和分数
		if(result == 0 && matchPage != NULL && matchScore != NULL)
		{
			// 响应包格式: ... 确认码(1) 页码高(1) 页码低(1) 分数高(1) 分数低(1) 校验和(2)
			*matchPage = (USART6_RX_BUF[10] << 8) | USART6_RX_BUF[11];
			*matchScore = (USART6_RX_BUF[12] << 8) | USART6_RX_BUF[13];
		}
		return result;
	}
	return 0xFF;  // 超时
}

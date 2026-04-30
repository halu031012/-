#ifndef __AS608_H
#define __AS608_H
#include "sys.h"

void As608_Gpio_Init(void);     // 初始化 WAK 引脚 (PD1)
u8 As608_Wak_Read(void);        // 读取 WAK 引脚状态
void as608_inti(void);
u8 PS_GetImage(void);           // 返回确认码: 0-成功, 2-无手指, 0xFF-超时
u8 Ps_GenChar(u8 buffer);       // 返回确认码: 0-成功, 0xFF-超时
u8 PS_RegModel(void);           // 返回确认码: 0-成功, 0xFF-超时
u8 PS_StoreChar(u8 bufferID, u16 pageID);
u8 PS_Search(u8 bufferID, u16 startPage, u16 pageNum, u16* matchPage, u16* matchScore);
u8 PS_Empty(void);

#endif

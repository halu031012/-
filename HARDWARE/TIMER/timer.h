#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

// PWM和舵机控制
void TIM2_PWM_Init(void);
void Lock(u8 enable);  // 1上锁, 0开锁

// 任务调度定时器
void TIM3_Init(u16 arr, u16 psc);

// 系统滴答计数（在TIM3中断中更新）
extern volatile u32 uwTick;

#endif

#ifndef __PWM_H
#define __PWM_H
#include "sys.h"

// 定义舵机的PWM周期和占空比
#define PWM_PERIOD 20000  // PWM周期为20ms (20000us)
#define PWM_MIN 500       // 最小脉宽为0.5ms (500us)
#define PWM_MAX 2500      // 最大脉宽为2.5ms (2500us)

void TIM2_PWM_Init(void);   // 初始化TIM2以生成PWM信号
void GPIO_Init_PWM(void);   // 初始化GPIO以输出PWM信号
void Servo_SetAngle(u8 angle);  // 设置舵机角度（0-180度）
void lock(uint8_t enable);      // 门锁控制：1-上锁，0-解锁

#endif

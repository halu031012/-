#include "pwm.h"
#include "stm32f4xx_tim.h"

// 初始化TIM2以生成PWM信号
void TIM2_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 使能GPIOA时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    // 配置PA0为复用功能（TIM2_CH1）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置PA0复用为TIM2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
    
    // 配置TIM2的基本参数
    // STM32F407主频168MHz，APB1时钟42MHz，TIM2时钟84MHz
    // 预分频84-1，得到1MHz (1us精度)
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1;      // 自动重装值
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;           // 预分频84
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 配置TIM2的PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1500;    // 初始占空比为1500us（90度）
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    // 配置TIM2通道1
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // 使能TIM2的自动重装寄存器预装载
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    
    // 使能TIM2
    TIM_Cmd(TIM2, ENABLE);
}

// 初始化GPIO以输出PWM信号（已集成到TIM2_PWM_Init中）
void GPIO_Init_PWM(void)
{
    // GPIO初始化已整合到TIM2_PWM_Init函数中
}

// 设置舵机角度（0-180度）
void Servo_SetAngle(u8 angle)
{
    u16 pulse;
    if(angle > 180) angle = 180;
    
    // 角度转换为脉宽：0度=500us, 180度=2500us
    pulse = PWM_MIN + (PWM_MAX - PWM_MIN) * angle / 180;
    TIM_SetCompare1(TIM2, pulse);
}

void lock(uint8_t enable)
{
    if (enable)
    {
        TIM_SetCompare1(TIM2, PWM_MIN);  // 上锁位置（0度附近）
    }
    else
    {
        TIM_SetCompare1(TIM2, 1500);     // 解锁位置（90度附近）
    }
}

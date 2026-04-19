#include "timer.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"

// PWM参数定义
#define PWM_PERIOD 20000  // PWM周期为20ms (20000us)
#define PWM_MIN 500       // 最小脉宽为0.5ms (500us) - 上锁位置
#define PWM_MAX 2500      // 最大脉宽为2.5ms (2500us)

// 系统滴答计数
volatile u32 uwTick = 0;

// 外部变量声明（在main.c中定义）
extern u8 lock_flag;           // 锁状态标志：0开锁，1上锁
extern u8 show_flag;           // 显示标志
extern u8 password_error;      // 密码错误次数
extern u16 time5000ms;         // 5秒自动锁门计时
extern u16 time1000ms;         // 1秒计时
extern u16 time30s;            // 30秒锁定倒计时

// 初始化TIM2以生成PWM信号（舵机控制）
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
    TIM_OCInitStructure.TIM_Pulse = PWM_MIN;    // 初始占空比为最小值（上锁状态）
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    // 配置TIM2通道1
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // 使能TIM2的自动重装寄存器预装载
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    
    // 使能TIM2
    TIM_Cmd(TIM2, ENABLE);
}

// 门锁控制函数
// enable: 1上锁(500us), 0开锁(1500us)
void Lock(u8 enable)
{
    if(enable)
    {
        TIM_SetCompare1(TIM2, 500);   // 上锁位置
    }
    else
    {
        TIM_SetCompare1(TIM2, 1500);  // 开锁位置
    }
}

// 初始化TIM3用于系统滴答计数（任务调度）
void TIM3_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 配置TIM3
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = arr;        // 自动重装载值
    TIM_TimeBaseInitStruct.TIM_Prescaler = psc;     // 预分频值
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    
    // 使能TIM3更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 使能TIM3
    TIM_Cmd(TIM3, ENABLE);
}

// TIM3中断服务函数 - 1ms中断
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        uwTick++;  // 系统滴答计数器
        
        // 自动锁门功能：开门5秒后自动上锁
        if(lock_flag == 0)  // 门已开
        {
            if(++time5000ms >= 5000)  // 计时5秒
            {
                time5000ms = 0;
                lock_flag = 1;   // 关门
                show_flag = 0;   // 显示"门已上锁"
            }
        }
        
        // 密码错误3次锁定功能：30秒倒计时
        if(password_error >= 3)
        {
            if(++time1000ms >= 1000)  // 1秒
            {
                time1000ms = 0;
                if(--time30s == 0)  // 30秒倒计时结束
                {
                    time30s = 30;
                    password_error = 0;  // 密码错误次数归零
                    show_flag = 0;
                }
            }
        }
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}

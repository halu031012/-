#include "stm32f4xx.h"
#include "delay.h"
#include "lcd.h"
#include "pic.h"
#include "pwm.h"




int main(void)
{
    u32 t = 0;
    unsigned char i = 0;
    
    // NVIC配置
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 延时初始化（168MHz主频）
    delay_init(168);
    
    // PWM初始化（舵机控制）
    TIM2_PWM_Init();
    
    // 延时等待系统稳定
    delay_ms(100);
    
    // 设置舵机初始角度（约90度）
    TIM_SetCompare1(TIM2, 1500);
    
    // 再次设置，确保PWM输出稳定
    delay_ms(10);
    TIM_SetCompare1(TIM2, 1500);
    
    // LCD初始化
    LCD_Init();
    
    // 清屏为白色
    LCD_Fill(0, 0, 127, 127, WHITE);
    
    // 显示启动信息
    LCD_ShowChinese(32, 50, "正", RED, WHITE, 16, 0);
    LCD_ShowChinese(32+16, 50, "在", RED, WHITE, 16, 0);
    LCD_ShowChinese(32+32, 50, "启", RED, WHITE, 16, 0);
    LCD_ShowChinese(32+48, 50, "动", RED, WHITE, 16, 0);
    
    // 进度条动画
    while(i < 128)
    {
        LCD_DrawLine(i, 100, i, 127, RED);
        delay_ms(50);
        i++;
    }
    
    // 显示启动成功
    LCD_ShowChinese(32, 50, "启", RED, WHITE, 16, 0);
    LCD_ShowChinese(32+16, 50, "动", RED, WHITE, 16, 0);
    LCD_ShowChinese(32+32, 50, "成", RED, WHITE, 16, 0);
    LCD_ShowChinese(32+48, 50, "功", RED, WHITE, 16, 0);
    
    delay_ms(500);
    
    // 显示图片
    LCD_ShowPicture(1, 1, 128, 128, gImage_1);
    
    while(1)
    {
        // 主循环 - 舵机来回运动测试
        Servo_SetAngle(0);    // 转到0度
        delay_ms(1000);
        
        Servo_SetAngle(90);   // 转到90度
        delay_ms(1000);
        
        Servo_SetAngle(180);  // 转到180度
        delay_ms(1000);
        
        Servo_SetAngle(90);   // 转回90度
        delay_ms(1000);
        
        t++;
    }
}

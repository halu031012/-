#include "stm32f4xx.h"
#include "delay.h"
#include "lcd.h"
#include "pic.h"
#include "timer.h"
#include "key.h"


/*函数声明*/
u8 String_Check(u8* string1, u8* string2, u8 len);

/*变量声明*/
u8 num;
u8 key_val, key_old, key_down, key_up;
u8 password[6] = {1, 2, 3, 4, 5, 6};      // 正确密码 123456
u8 key_temp[6] = {0};                      // 临时存储输入的密码（最多6位）
u8 key_index;
u8 key_index_old;

/*按键处理函数 - 每10ms执行一次*/
void KEY_Proc(void)
{
    key_val = KEY_Read();
    key_down = key_val & (key_val ^ key_old);
    key_up = ~key_val & (key_val ^ key_old);
    key_old = key_val;

    switch(key_down)
    {
        case 1:  // 数字1
            if(key_index < 6) { key_temp[key_index] = 1; key_index++; }
            break;
        case 2:  // 数字2
            if(key_index < 6) { key_temp[key_index] = 2; key_index++; }
            break;
        case 3:  // 数字3
            if(key_index < 6) { key_temp[key_index] = 3; key_index++; }
            break;
        case 4:
            break;
        case 5:  // 数字4
            if(key_index < 6) { key_temp[key_index] = 4; key_index++; }
            break;
        case 6:  // 数字5
            if(key_index < 6) { key_temp[key_index] = 5; key_index++; }
            break;
        case 7:  // 数字6
            if(key_index < 6) { key_temp[key_index] = 6; key_index++; }
            break;
        case 8:
            break;
        case 9:  // 数字7
            if(key_index < 6) { key_temp[key_index] = 7; key_index++; }
            break;
        case 10: // 数字8
            if(key_index < 6) { key_temp[key_index] = 8; key_index++; }
            break;
        case 11: // 数字9
            if(key_index < 6) { key_temp[key_index] = 9; key_index++; }
            break;
        case 12:
            break;
        case 13: // 清除键
            key_index = 0;
            key_temp[0] = key_temp[1] = key_temp[2] = key_temp[3] = key_temp[4] = key_temp[5] = 0;
            break;
        case 14: // 数字0
            if(key_index < 6) { key_temp[key_index] = 0; key_index++; }
            break;
        case 15:
            break;
        case 16: // 确认键
            if(key_index == 6 && String_Check(key_temp, password, 6))
                Lock(0);  // 密码正确，开锁
            else
                Lock(1);  // 密码错误或长度不足，保持上锁
            key_index = 0;
            key_temp[0] = key_temp[1] = key_temp[2] = key_temp[3] = key_temp[4] = key_temp[5] = 0;
            break;
    }


}

/*屏幕处理函数 - 每100ms执行一次*/
void LCD_Proc(void)
{
    if(key_index != key_index_old)
    {
        u8 i = key_index;
        LCD_Fill(16, 45, 112, 66, YELLOW);  // 填充黄色背景
        
        if(key_index == 7) key_index = 6;
        
        while(i--)
        {
            if(i < 6)
                LCD_ShowChar(20 + 16 * i, 45, '*', RED, YELLOW, 16, 0);
        }
        key_index_old = key_index;
    }
}

/*任务调度器相关*/
typedef struct
{
    void (*task_func)(void);     // 任务函数
    u32 rate_ms;                  // 任务执行周期
    u32 last_run;                 // 任务上次运行的时间
} task_t;

task_t scheduler_task[] = {
    {LCD_Proc, 100, 0},  // 屏幕处理函数，100毫秒执行一次
    {KEY_Proc, 10, 0},   // 按键处理函数，10毫秒执行一次
};

u8 task_num;

void Scheduler_Init(void)
{
    task_num = sizeof(scheduler_task) / sizeof(task_t);
}

void Scheduler_Run(void)
{
    u8 i;
    for(i = 0; i < task_num; i++)
    {
        u32 now_time = uwTick;
        if(now_time > (scheduler_task[i].last_run + scheduler_task[i].rate_ms))
        {
            scheduler_task[i].last_run = now_time;
            scheduler_task[i].task_func();
        }
    }
}

/*字符串比较函数*/
u8 String_Check(u8* string1, u8* string2, u8 len)
{
    while(len--)
    {
        if(string1[len] == string2[len])
            ;
        else
            return 0;
    }
    return 1;
}

/*主函数*/
int main(void)
{
    u8 i = 0;
    
    // NVIC配置
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 延时初始化
    delay_init(168);
    
    // PWM初始化（舵机控制）
    TIM2_PWM_Init();
    
    // 初始默认上锁
    Lock(1);
    
    // LCD初始化
    LCD_Init();
    
    // 整个屏幕填充白色
    LCD_Fill(0, 0, 127, 127, WHITE);
    
    // 开机显示
    //LCD_ShowString(32, 50, "Starting", RED, WHITE, 16, 0);
	//lcd_disp_chinese(32,50,"正在启动",RED,WHITE,16,0);
	lcd_disp_chinese(32,50,"\xD5\xFD\xD4\xDA\xC6\xF4\xB6\xAF",RED,WHITE,16,0);
    
    // 循环划线实现进度条的效果
    while(i < 128)
    {
        LCD_DrawLine(i, 100, i, 127, RED);
        delay_ms(10);
        i++;
    }
    
    // 启动成功
    lcd_disp_chinese(32, 50, "\xC6\xF4\xB6\xAF\xB3\xC9\xB9\xA6", RED, WHITE, 16, 0);
    delay_ms(500);
    
    // 显示logo和主页
//    LCD_ShowPicture(0, 0, 128, 128, gImage_1);
//    delay_ms(1000);
    LCD_ShowPicture(0, 0, 128, 128, gImage_2);
    
    // 显示密码输入提示
    lcd_disp_chinese(0, 0, "\xD2\xD1\xCB\xF8\xB6\xA8", RED, WHITE, 16, 0);
    lcd_disp_chinese(0, 16, "\xC7\xEB\xCA\xE4\xC8\xEB\xC3\xDC\xC2\xEB", RED, WHITE, 16, 0);
    
    // 填充密码输入区域背景
    LCD_Fill(16, 45, 112, 66, YELLOW);
    
    // 矩阵按键初始化
    KEY_Init();
    
    // 系统计时定时器初始化（1ms中断）
    // TIM3时钟84MHz，预分频84-1=83，周期1000-1=999 → 1ms中断
    TIM3_Init(1000 - 1, 84 - 1);
    
    // 任务调度器初始化
    Scheduler_Init();
    
    while(1)
    {
        Scheduler_Run();
    }
}





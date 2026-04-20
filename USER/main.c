/*
 * 智能门锁系统 - STM32F407版本
 * 移植自第四讲（沁恒CH32V307版本）
 * 
 * 功能说明：
 * 1. 密码开锁（6位数字密码）
 * 2. RFID刷卡开锁
 * 3. 语音播报提示
 * 4. 密码错误3次锁定15秒
 * 5. 开门5秒后自动上锁
 * 6. 管理员模式（修改密码、录入卡片）
 */

#include "stm32f4xx.h"
#include "delay.h"
#include "lcd.h"
#include "pic.h"
#include "timer.h"
#include "key.h"
#include "uart.h"
#include "audio.h"
#include "string.h"

/*==================== 函数声明 ====================*/
u8 String_Check(u8* string1, u8* string2, u8 len);  // 数组比较函数
void String_Copy(u8* string1, u8* string2, u8 len);  // 数组复制函数
void Key_Clear(void);                                // 清空按键缓冲区
u8 RFID_Check(void);                                 // RFID卡片校验

/* 任务处理函数 */
void KEY_Proc(void);    // 按键处理
void LCD_Proc(void);    // 屏幕显示处理
void Lock_Proc(void);   // 门锁控制处理

/* 中断服务函数 */
void USART2_IRQHandler(void);  // RFID串口接收中断

/*==================== 变量定义 ====================*/
/* 系统变量 */
u8 key_val, key_old, key_down, key_up;      // 按键相关变量

/* 密码相关 */
u8 password[6] = {1, 2, 3, 4, 5, 6};        // 开锁密码（默认123456）
u8 password_cmd[6] = {2, 7, 7, 5, 1, 6};    // 管理员密码（277516）
u8 key_temp[7] = {10, 10, 10, 10, 10, 10};  // 按键暂存区
u8 key_index = 0;                           // 当前输入位数
u8 key_index_old = 0;                       // 上一次输入位数
u8 password_error = 0;                      // 密码错误次数

/* 门锁状态 */
u8 lock_flag = 1;                           // 锁状态：0开锁，1上锁
u8 lock_flag_old = 1;                       // 上一次锁状态（用于检测自动锁门）
u8 show_flag = 0;                           // 显示标志
u8 show_flag_old = 0;                       // 上一次显示标志

/* 定时器变量（在TIM3中断中更新） */
u16 time5000ms = 0;                         // 5秒自动锁门计时
u16 time1000ms = 0;                         // 1秒计时
u16 time30s = 30;                           // 30秒锁定倒计时

/* 模式变量 */
u8 mode = 0;
// 0 - 主页（输入密码开锁）
// 1 - 修改密码，未验证管理员权限
// 2 - 修改密码，已验证管理员权限
// 3 - 录入卡片，未验证管理员权限
// 4 - 录入卡片，已验证管理员权限

/* RFID相关 */
u8 rfid_index = 0;                          // RFID接收索引
u8 rfid_temp[4] = {0};                      // 临时卡号存储
u8 rfid[4][4] = {0};                        // 已录入的卡片存储（最多4张）
u8 rfid_password_index = 0;                 // 已录入卡片数量

/*==================== 任务调度器 ====================*/
typedef struct
{
    void (*task_func)(void);     // 任务函数指针
    u32 rate_ms;                 // 执行周期（毫秒）
    u32 last_run;                // 上次执行时间
} task_t;

task_t scheduler_task[] = {
    {LCD_Proc,   100, 0},  // 屏幕显示处理，100ms
    {KEY_Proc,    10, 0},  // 按键处理，10ms
    {Lock_Proc,   30, 0},  // 门锁控制，30ms
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

/*==================== 按键处理函数 ====================*/
void KEY_Proc(void)
{
    key_val = KEY_Read();
    key_down = key_val & (key_val ^ key_old);
    key_up = ~key_val & (key_val ^ key_old);
    key_old = key_val;

    /* 密码错误3次，锁定期间不响应按键 */
    if(password_error >= 3) return;

    /* 按键音效 */
    if(key_down)
    {
        Audio_Play(1);  // 播放按键音
    }

    switch(key_down)
    {
        /* 数字键 1-9 */
        case 1: key_temp[key_index] = 1; key_index++; break;
        case 2: key_temp[key_index] = 2; key_index++; break;
        case 3: key_temp[key_index] = 3; key_index++; break;
        case 5: key_temp[key_index] = 4; key_index++; break;
        case 6: key_temp[key_index] = 5; key_index++; break;
        case 7: key_temp[key_index] = 6; key_index++; break;
        case 9: key_temp[key_index] = 7; key_index++; break;
        case 10: key_temp[key_index] = 8; key_index++; break;
        case 11: key_temp[key_index] = 9; key_index++; break;

        /* 功能键 */
        case 4:  // 进入修改密码模式
            if(mode == 0)
            {
                mode = 1;
                Audio_Play(6);  // "修改开锁密码，请输入管理员密码"
                password_error = 0;
                Key_Clear();
            }
            break;

        case 12:  // 进入录入卡片模式
            if(mode == 0)
            {
                mode = 3;
                Audio_Play(13);  // "录入卡片，请输入管理员密码"
                Key_Clear();
            }
            break;

        case 13:  // 清除键
            Key_Clear();
            break;

        case 14:  // 数字0
            key_temp[key_index] = 0;
            key_index++;
            break;

        case 15:  // 退格键
            if(key_index > 0)
            {
                key_index--;
                key_temp[key_index] = 10;
            }
            break;

        case 16:  // 确认键
            switch(mode)
            {
                /* 主页模式 - 输入密码开锁 */
                case 0:
                    if(String_Check(key_temp, password, 6))
                    {
                        lock_flag = 0;      // 开锁
                        show_flag = 1;      // 显示"门已开启"
                        Audio_Play(3);      // "密码正确，门已打开"
                        Key_Clear();
                        password_error = 0;
                    }
                    else
                    {
                        Audio_Play(4);      // "密码错误，请重新输入"
                        Key_Clear();
                        if(++password_error >= 3)
                        {
                            Audio_Play(5);  // "连续三次错误，已锁定"
                            show_flag = 2;  // 显示倒计时
                        }
                    }
                    break;

                /* 修改密码模式 - 验证管理员密码 */
                case 1:
                    if(String_Check(key_temp, password_cmd, 6))
                    {
                        mode = 2;           // 进入修改密码状态
                        Audio_Play(7);      // "管理员密码验证成功"
                        Key_Clear();
                    }
                    else
                    {
                        Audio_Play(9);      // "管理员密码错误"
                        Key_Clear();        // 清空输入，让用户重新输入
                        if(++password_error >= 3)
                        {
                            Audio_Play(10); // "连续三次错误，已锁定"
                            show_flag = 2;
                        }
                    }
                    break;

                /* 修改密码模式 - 设置新密码 */
                case 2:
                    String_Copy(key_temp, password, 6);
                    Audio_Play(8);          // "新密码设置成功"
                    mode = 0;               // 返回主页
                    Key_Clear();
                    break;

                /* 录入卡片模式 - 验证管理员密码 */
                case 3:
                    if(String_Check(key_temp, password_cmd, 6))
                    {
                        mode = 4;           // 进入录入卡片状态
                        Audio_Play(14);     // "请将卡片平放在传感器上"
                        Key_Clear();
                    }
                    else
                    {
                        Audio_Play(9);      // "管理员密码错误"
                        Key_Clear();
                        if(++password_error >= 3)
                        {
                            Audio_Play(10);
                            show_flag = 2;
                        }
                    }
                    break;
            }
            break;
    }

    /* 限制输入位数不超过6位 */
    if(key_index > 6) key_index = 6;
}

/*==================== LCD显示处理函数 ====================*/
void LCD_Proc(void)
{
    u8 i;

    /* 更新密码输入区显示 */
    if(key_index != key_index_old)
    {
        LCD_Fill(16, 45, 112, 66, YELLOW);  // 填充黄色背景

        i = key_index;
        if(i > 6) i = 6;

        while(i--)
        {
            if(i < 6)
                LCD_ShowChar(20 + 16 * i, 45, '*', RED, YELLOW, 16, 0);
        }
        key_index_old = key_index;
    }

    /* 更新顶部状态显示 */
    if(show_flag != show_flag_old)
    {
        LCD_Fill(0, 0, 128, 32, WHITE);  // 清除顶部区域
        show_flag_old = show_flag;
    }

    switch(show_flag)
    {
        case 0:  // 门已上锁
            lcd_disp_chinese(0, 0, "\xC3\xC5\xD2\xD1\xC9\xCF\xCB\xF8", RED, WHITE, 16, 0);
            lcd_disp_chinese(0, 16, "\xC7\xEB\xCA\xE4\xC8\xEB\xC3\xDC\xC2\xEB", RED, WHITE, 16, 0);
            break;

        case 1:  // 门已开启
            lcd_disp_chinese(0, 0, "\xC3\xC5\xD2\xD1\xC3\xAA\xC6\xF4", RED, WHITE, 16, 0);  // 门已开启(开=C3AA)
            lcd_disp_chinese(0, 16, "\xCB\xBB\xD3\xAD\xD8\xD0\xBC\xD2", RED, WHITE, 16, 0);  // 欢迎回家(欢=CBBB,回=D8D0)
            break;

        case 2:  // 锁定倒计时
            lcd_disp_chinese(0, 0, "\xD2\xD1\xCB\xF8\xB6\xA8\xA3\xAC\xC7\xEB\xB5\xC8\xB4\xFD", RED, WHITE, 16, 0);
            LCD_ShowIntNum(50, 16, time30s, 2, RED, WHITE, 16);
            break;
    }
}

/*==================== 门锁控制处理函数 ====================*/
void Lock_Proc(void)
{
    // 检测从开锁到上锁的状态变化（自动锁门）
    if(lock_flag == 1 && lock_flag_old == 0)
    {
        Audio_Play(2);  // 播放"门已上锁"语音
    }
    lock_flag_old = lock_flag;
    
    Lock(lock_flag);  // 控制舵机
}

/*==================== 工具函数 ====================*/
u8 String_Check(u8* string1, u8* string2, u8 len)
{
    while(len--)
    {
        if(string1[len] != string2[len])
            return 0;
    }
    return 1;
}

void String_Copy(u8* string1, u8* string2, u8 len)
{
    u8 i;
    for(i = 0; i < len; i++)
    {
        string2[i] = string1[i];
    }
}

void Key_Clear(void)
{
    memset(key_temp, 10, 6);
    key_index = 0;
}

/* RFID_Check函数 - 检查卡片是否已录入 */
u8 RFID_Check(void)
{
    u8 i;
    for(i = 0; i < rfid_password_index; i++)
    {
        if(String_Check(rfid_temp, rfid[i], 4))
            return 1;
    }
    return 0;
}

/*==================== USART2中断 - RFID接收 ====================*/
void USART2_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        temp = USART_ReceiveData(USART2);
        
        switch(rfid_index)
        {
            case 0:
                if(temp == 0x04) rfid_index++;
                break;
            case 1:
                if(temp == 0x0c) rfid_index++;
                else rfid_index = 0;
                break;
            case 2:
                if(temp == 0x02) rfid_index++;
                else rfid_index = 0;
                break;
            case 3:
                if(temp == 0x30) rfid_index++;
                else rfid_index = 0;
                break;
            case 4:
                if(temp == 0x00) rfid_index++;
                else rfid_index = 0;
                break;
            case 5:
                if(temp == 0x04) rfid_index++;
                else rfid_index = 0;
                break;
            case 6:
                if(temp == 0x00) rfid_index++;
                else rfid_index = 0;
                break;
            case 7:
                rfid_temp[0] = temp;
                rfid_index++;
                break;
            case 8:
                rfid_temp[1] = temp;
                rfid_index++;
                break;
            case 9:
                rfid_temp[2] = temp;
                rfid_index++;
                break;
            case 10:
                rfid_temp[3] = temp;
                rfid_index = 0;

                // 主页模式 - 刷卡开锁
                if(mode == 0)
                {
                    if(RFID_Check())
                    {
                        Audio_Play(11);     // "刷卡成功"
                        lock_flag = 0;      // 开锁
                        show_flag = 1;
                    }
                    else
                    {
                        Audio_Play(12);     // "刷卡失败"
                    }
                }

                // 录入卡片模式
                if(mode == 4)
                {
                    if(rfid_password_index < 4)
                    {
                        String_Copy(rfid_temp, rfid[rfid_password_index], 4);
                        Audio_Play(15);     // "卡片添加成功"
                        mode = 0;
                        rfid_password_index++;
                    }
                }
                break;
        }
    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}


/*==================== 主函数 ====================*/
int main(void)
{
    u8 i = 0;

    /* 系统初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 中断优先级分组
    delay_init(168);                                 // 延时初始化
    
    /* 硬件初始化 */
    TIM2_PWM_Init();     // 舵机PWM初始化
    Lock(1);             // 初始状态：上锁

    LCD_Init();          // LCD初始化
    LCD_Fill(0, 0, 127, 127, WHITE);  // 清屏

    /* 开机画面 */
    lcd_disp_chinese(32, 50, "\xD5\xFD\xD4\xDA\xC6\xF4\xB6\xAF", RED, WHITE, 16, 0);

    /* 进度条动画 */
    for(i = 0; i < 128; i++)
    {
        LCD_DrawLine(i, 100, i, 127, RED);
        delay_ms(10);
    }

    lcd_disp_chinese(32, 50, "\xC6\xF4\xB6\xAF\xB3\xC9\xB9\xA6", RED, WHITE, 16, 0);
    delay_ms(500);

    /* 显示Logo和主页 */
    LCD_ShowPicture(0, 0, 128, 128, gImage_2);
//    delay_ms(500);

    /* 显示密码输入界面 */
    //LCD_Fill(0, 0, 127, 127, WHITE);
    lcd_disp_chinese(0, 0, "\xC3\xC5\xD2\xD1\xC9\xCF\xCB\xF8", RED, WHITE, 16, 0);
    lcd_disp_chinese(0, 16, "\xC7\xEB\xCA\xE4\xC8\xEB\xC3\xDC\xC2\xEB", RED, WHITE, 16, 0);
    LCD_Fill(16, 45, 112, 66, YELLOW);  // 密码输入区背景

    /* 其他模块初始化 */
    KEY_Init();          // 矩阵按键初始化
    TIM3_Init(1000 - 1, 84 - 1);  // TIM3初始化，1ms中断
	Audio_Init();        // 语音模块初始化（内部已包含唤醒和延时）
    //Audio_Play(2);       // 播放"门已上锁"提示
    Scheduler_Init();    // 任务调度器初始化
    USART2_Init();    // RFID串口初始化

    /* 主循环 */
    while(1)
    {
        Scheduler_Run();  // 运行任务调度器
    }
}

/*
 * lcd.c - STM32F407版本
 * ST7735 LCD驱动
 */

#include "lcd.h"
#include "font_ascii_16x8.h"
#include "delay.h"
#include "stm32f4xx_spi.h"

// 初始化SPI3和GPIO（STM32F407VET6）
void SPI_LCD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;
    
    // 使能GPIO时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | 
                           RCC_AHB1Periph_GPIOC, ENABLE);
    
    // 使能SPI3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    
    // 配置LCD控制引脚（CS - PA4, DC - PA3, RES - PA1）- 推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOA, GPIO_Pin_1);   // RES
    GPIO_SetBits(GPIOA, GPIO_Pin_3);   // DC
    GPIO_SetBits(GPIOA, GPIO_Pin_4);   // CS
    
    // 配置LCD背光引脚（BL - PB9）- 推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_9);
    
    // 配置SPI3引脚（PC10=SCK, PC11=MISO, PC12=MOSI）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // 配置GPIO复用功能
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);  // SCK
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);  // MISO
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);  // MOSI
    
    // SPI3配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI3, &SPI_InitStructure);
    
    // 使能SPI3
    SPI_Cmd(SPI3, ENABLE);
}

// SPI读写一个字节
u8 spi_readwrite(u8 TxData)
{
    u16 i = 0;
    
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET)
    {
        i++;
        if (i > 200) return 0;
    }
    
    SPI_I2S_SendData(SPI3, TxData);
    i = 0;
    
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET)
    {
        i++;
        if (i > 200) return 0;
    }
    
    return SPI_I2S_ReceiveData(SPI3);
}

// LCD写入数据（8位）
void LCD_WR_DATA8(u8 dat)
{
    LCD_CS_Clr();
    LCD_DC_Set();
    spi_readwrite(dat);
    LCD_CS_Set();
}

// LCD写入数据（16位）
void LCD_WR_DATA(u16 dat)
{
    LCD_CS_Clr();
    LCD_DC_Set();
    spi_readwrite(dat >> 8);
    spi_readwrite(dat);
    LCD_CS_Set();
}

// LCD写入命令
void LCD_WR_REG(u8 dat)
{
    LCD_CS_Clr();
    LCD_DC_Clr();
    spi_readwrite(dat);
    LCD_CS_Set();
}

// 设置起始和结束地址
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
    if (USE_HORIZONTAL == 0)
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1 + 2);
        LCD_WR_DATA(x2 + 2);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1 + 1);
        LCD_WR_DATA(y2 + 1);
        LCD_WR_REG(0x2c);
    }
    else if (USE_HORIZONTAL == 1)
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1 + 2);
        LCD_WR_DATA(x2 + 2);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1 + 3);
        LCD_WR_DATA(y2 + 3);
        LCD_WR_REG(0x2c);
    }
    else if (USE_HORIZONTAL == 2)
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1 + 1);
        LCD_WR_DATA(x2 + 1);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1 + 2);
        LCD_WR_DATA(y2 + 2);
        LCD_WR_REG(0x2c);
    }
    else
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1 + 3);
        LCD_WR_DATA(x2 + 3);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1 + 2);
        LCD_WR_DATA(y2 + 2);
        LCD_WR_REG(0x2c);
    }
}

// ST7735初始化
void LCD_Init(void)
{
    SPI_LCD_Init();
    
    LCD_RES_Clr();
    delay_ms(100);
    LCD_RES_Set();
    delay_ms(100);
    
    LCD_BLK_Set();
    delay_ms(100);
    
    LCD_WR_REG(0x11);
    delay_ms(120);
    
    // Frame rate
    LCD_WR_REG(0xB1);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    
    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    
    LCD_WR_REG(0xB3);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    
    LCD_WR_REG(0xB4);
    LCD_WR_DATA8(0x03);
    
    LCD_WR_REG(0x3a);
    LCD_WR_DATA8(0x05);
    
    // Power sequence
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0xA2);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x84);
    LCD_WR_REG(0xC1);
    LCD_WR_DATA8(0xC5);
    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x00);
    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0x2A);
    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0xEE);
    
    LCD_WR_REG(0xC5);
    LCD_WR_DATA8(0x0a);
    
    LCD_WR_REG(0x36);
    if (USE_HORIZONTAL == 0) LCD_WR_DATA8(0x08);
    else if (USE_HORIZONTAL == 1) LCD_WR_DATA8(0xC8);
    else if (USE_HORIZONTAL == 2) LCD_WR_DATA8(0x78);
    else LCD_WR_DATA8(0xA8);
    
    // Gamma sequence
    LCD_WR_REG(0XE0);
    LCD_WR_DATA8(0x12);
    LCD_WR_DATA8(0x1C);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x25);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x27);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x10);
    
    LCD_WR_REG(0XE1);
    LCD_WR_DATA8(0x12);
    LCD_WR_DATA8(0x1C);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x2D);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x23);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x26);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x3B);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x10);
    
    LCD_WR_REG(0x20);
    LCD_WR_REG(0x13);
    
    LCD_WR_REG(0x2A);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x7F);
    
    LCD_WR_REG(0x2B);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x7F);
    
    LCD_WR_REG(0x29);
}

// 在指定区域填充颜色
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
    u16 i, j;
    LCD_Address_Set(xsta, ysta, xend - 1, yend - 1);
    for (i = ysta; i < yend; i++)
    {
        for (j = xsta; j < xend; j++)
        {
            LCD_WR_DATA(color);
        }
    }
}

// 在指定位置画点
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA(color);
}

// 画线
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0) incx = 1;
    else if (delta_x == 0) incx = 0;
    else { incx = -1; delta_x = -delta_x; }
    if (delta_y > 0) incy = 1;
    else if (delta_y == 0) incy = 0;
    else { incy = -1; delta_y = -delta_x; }
    if (delta_x > delta_y) distance = delta_x;
    else distance = delta_y;
    for (t = 0; t < distance + 1; t++)
    {
        LCD_DrawPoint(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// 画矩形
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

// 画圆
void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color)
{
    int a, b;
    a = 0; b = r;
    while (a <= b)
    {
        LCD_DrawPoint(x0 - b, y0 - a, color);
        LCD_DrawPoint(x0 + b, y0 - a, color);
        LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 + a, color);
        LCD_DrawPoint(x0 + a, y0 - b, color);
        LCD_DrawPoint(x0 + a, y0 + b, color);
        LCD_DrawPoint(x0 - b, y0 + a, color);
        a++;
        if ((a * a + b * b) > (r * r))
        {
            b--;
        }
    }
}

void lcd_disp_chinese(u16 x,u16 y,u8* s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
    while(*s)
    {
        LCD_ShowChinese(x,y,s,fc,bc,sizey,mode);
        x+=16;
        if(x>=127)
        {
            x=0;
            y+=16;
        }
        s+=2;
    }
}



// 显示汉字串
void LCD_ShowChinese(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    if (*s == 0) return;
    if (sizey == 16) LCD_ShowChinese16x16(x, y, s, fc, bc, sizey, mode);
    else if (sizey == 24) LCD_ShowChinese24x24(x, y, s, fc, bc, sizey, mode);
    else if (sizey == 32) LCD_ShowChinese32x32(x, y, s, fc, bc, sizey, mode);
}



// 显示单个16x16汉字
void LCD_ShowChinese16x16(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    u8 i, j;
    u16 k;
    u16 HZnum;
    u16 TypefaceNum;
    u16 x0 = x;
    TypefaceNum = sizey / 8 * sizey;
    HZnum = sizeof(tfont16) / sizeof(typFNT_GB16);
    for (k = 0; k < HZnum; k++)
    {
        if ((tfont16[k].Index[0] == *(s)) && (tfont16[k].Index[1] == *(s + 1)))
        {
            LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);
            for (i = 0; i < TypefaceNum; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    if (!mode)
                    {
                        if (tfont16[k].Msk[i] & (0x01 << j)) LCD_WR_DATA(fc);
                        else LCD_WR_DATA(bc);
                    }
                    else
                    {
                        if (tfont16[k].Msk[i] & (0x01 << j)) LCD_DrawPoint(x, y, fc);
                        x++;
                        if ((x - x0) == sizey)
                        {
                            x = x0;
                            y++;
                            break;
                        }
                    }
                }
            }
            break;  // 找到并显示后退出循环
        }
    }
}

// 显示单个24x24汉字
void LCD_ShowChinese24x24(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    // 24x24字库未定义，暂不支持
}

// 显示单个32x32汉字
void LCD_ShowChinese32x32(u16 x, u16 y, u8 *s, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    // 32x32字库未定义，暂不支持
}

// 显示单个字符
void LCD_ShowChar(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    u8 temp, sizex, t;
    u16 i, TypefaceNum;
    u16 x0 = x;
    sizex = sizey / 2;
    TypefaceNum = sizex / 8 * sizey;
    num = num - ' ';
    LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);
    for (i = 0; i < TypefaceNum; i++)
    {
        if (sizey == 16) temp = ascii_1608[num][i];
        else if (sizey == 32) temp = ascii_3216[num][i];
        else return;
        for (t = 0; t < 8; t++)
        {
            if (!mode)
            {
                if (temp & (0x01 << t)) LCD_WR_DATA(fc);
                else LCD_WR_DATA(bc);
            }
            else
            {
                if (temp & (0x01 << t)) LCD_DrawPoint(x, y, fc);
                x++;
                if ((x - x0) == sizex)
                {
                    x = x0;
                    y++;
                    break;
                }
            }
        }
    }
}

// 显示字符串
void LCD_ShowString(u16 x, u16 y, const u8 *p, u16 fc, u16 bc, u8 sizey, u8 mode)
{
    while (*p != '\0')
    {
        LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
        x += sizey / 2;
        p++;
    }
}

// 求幂
u32 mypow(u8 m, u8 n)
{
    u32 result = 1;
    while (n--) result *= m;
    return result;
}

// 显示整数
void LCD_ShowIntNum(u16 x, u16 y, u16 num, u8 len, u16 fc, u16 bc, u8 sizey)
{
    u8 t, temp;
    u8 enshow = 0;
    u8 sizex = sizey / 2;
    for (t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + t * sizex, y, ' ', fc, bc, sizey, 0);
                continue;
            }
            else enshow = 1;
        }
        LCD_ShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, 0);
    }
}

// 显示两位小数
void LCD_ShowFloatNum1(u16 x, u16 y, float num, u8 len, u16 fc, u16 bc, u8 sizey)
{
    u8 t, temp, sizex;
    u16 num1;
    sizex = sizey / 2;
    num1 = num * 100;
    for (t = 0; t < len; t++)
    {
        temp = (num1 / mypow(10, len - t - 1)) % 10;
        if (t == (len - 2))
        {
            LCD_ShowChar(x + (len - 2) * sizex, y, '.', fc, bc, sizey, 0);
            t++;
            len += 1;
        }
        LCD_ShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, 0);
    }
}

// 显示图片
void LCD_ShowPicture(u16 x, u16 y, u16 length, u16 width, const u8 pic[])
{
    u16 i, j, k = 0;
    LCD_Address_Set(x, y, x + length - 1, y + width - 1);
    for (i = 0; i < length; i++)
    {
        for (j = 0; j < width; j++)
        {
            LCD_WR_DATA8(pic[k * 2]);
            LCD_WR_DATA8(pic[k * 2 + 1]);
            k++;
        }
    }
}

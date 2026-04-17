# STM32F407 智能门锁项目

基于STM32F407VET6的智能门锁系统，从CH32V30x平台移植而来。

## 功能特性

- ✅ **LCD显示** - ST7735屏幕显示（128x128）
- ✅ **PWM舵机控制** - 舵机角度控制（0-180度）
- ✅ **中文显示** - 支持汉字显示和图片显示
- ✅ **进度动画** - 启动动画效果

## 硬件平台

- **MCU**: STM32F407VET6
- **主频**: 168MHz
- **LCD**: ST7735 (128x128)
- **舵机**: 标准PWM舵机

## 引脚配置

### LCD ST7735 (SPI3)
| 功能 | 引脚 |
|------|------|
| SPI_SCK | PC10 |
| SPI_MOSI | PC12 |
| LCD_CS | PA4 |
| LCD_DC | PA3 |
| LCD_RES | PA1 |
| LCD_BL | PB9 |

### 舵机 PWM (TIM2_CH1)
| 功能 | 引脚 |
|------|------|
| PWM信号 | PA0 |

## 目录结构

```
STM32F407智能门锁/
├── CORE/              # CMSIS核心文件
├── FWLIB/             # STM32固件库
├── HARDWARE/          # 硬件驱动
│   ├── LCD/          # LCD驱动
│   └── PWM/          # PWM驱动
├── SYSTEM/            # 系统文件
│   ├── delay/        # 延时函数
│   ├── usart/        # 串口
│   └── sys/          # 系统配置
└── USER/              # 用户代码
    └── main.c        # 主程序
```

## 编译环境

- **IDE**: Keil MDK-ARM 5.x
- **编译器**: ARM Compiler V5.06
- **固件库**: STM32F4xx Standard Peripheral Library

## 使用方法

1. 使用Keil MDK打开项目文件 `Template.uvprojx`
2. 编译项目（Build或F7）
3. 下载到STM32F407VET6开发板（Download或F8）
4. 观察LCD显示和舵机运动

## 硬件连接

### LCD连接
```
LCD      STM32F407
VCC  →   3.3V
GND  →   GND
SCK  →   PC10
MOSI →   PC12
RES  →   PA1
DC   →   PA3
CS   →   PA4
BL   →   PB9
```

### 舵机连接
```
Servo    STM32F407
信号线 →  PA0
电源线 →  5V（外部电源）
地线   →  GND（共地）
```

## 移植说明

本项目从沁恒CH32V30x平台移植到STM32F407VET6，主要修改：

1. **GPIO配置** - STM32F407使用AHB1总线时钟
2. **定时器配置** - 时钟频率从96MHz调整为84MHz
3. **SPI引脚** - SPI3引脚重新映射
4. **LCD控制引脚** - 避免与USART冲突

详细说明请查看 [移植说明.md](移植说明.md)

## 注意事项

⚠️ **舵机供电**: 舵机需要5V外部电源，电流需求200-500mA

⚠️ **共地**: 舵机地线必须与STM32共地

⚠️ **LCD背光**: PB9引脚控制LCD背光

## 后续功能

计划添加的功能：

- [ ] 指纹模块识别
- [ ] 密码键盘输入
- [ ] 蜂鸣器提示音
- [ ] WiFi远程控制
- [ ] Flash数据存储

## 许可证

本项目仅供学习和研究使用。

## 联系方式

如有问题，请提交Issue。

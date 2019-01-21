/****************************************************************************
 * This module is used to initial all the software module and hardware module.
 * *************************************************************************/
#include "headfile.h"
void everythinginit(void)
{
    /**上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置**/
    get_clk();
    /**蜂鸣器,LED,串口,摄像头,OLED***/
    gpio_init(LED,GPO,0);       //开LED灯

    gpio_init(BUZZER,GPO,1);	    //开蜂鸣器
    systick_delay_ms(300);
    gpio_set(BUZZER,0);            //关蜂鸣器   

    uart_init (uart2, 115200);  //初始换串口与电脑通信
    camera_init();
    OLED_Init();
    OLED_Fill(0x00);            //清屏
    
    /**********************************中断初始化****************************/
    pit_init_ms(pit0,500);								
    set_irq_priority(PIT0_IRQn,3);						
    enable_irq(PIT0_IRQn);
    EnableInterrupts;

    /*********************************电机，舵机初始化**********************/
    //初始化ftm3模块，4通道为15kHZ，占空比为百分之0，默认精度为10000 引脚对应查看MK60DN10_port_cfg.h
    
    ftm_pwm_init(MOTOR_FTM,LEFT_BACK_CH,15000,0);//左电机反转
    ftm_pwm_init(MOTOR_FTM,LEFT_FORWARD_CH,15000,1200);//左电机正转
    ftm_pwm_init(MOTOR_FTM,RIGHT_FORWARD_CH,15000,1200);//右电机正转
    ftm_pwm_init(MOTOR_FTM,RIGHT_BACK_CH,15000,0);//右电机反转
    
    /*1400-1550-1700*/
    ftm_pwm_init(STEER_FTM, STEER_CH, 100, 1550);
}
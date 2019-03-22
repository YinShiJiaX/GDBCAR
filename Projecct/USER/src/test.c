#include "headfile.h"
#include <string.h>

//测试led
void test_led(void)
{
    get_clk();//上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置

    //相关的库函数都在 MK60DN10_gpio.c 里面
    gpio_init(A19,GPO,0);	//初始化A19引脚为输出，默认输出低电平
//         gpio_init(E6,GPO,0);	//初始化E6引脚为输出，默认输出低电平
//         gpio_init(E7,GPO,0);	//初始化E7引脚为输出，默认输出低电平
//         gpio_init(E8,GPO,0);	//初始化E7引脚为输出，默认输出低电平
//    gpio_init(E1, GPO, 0);
//    gpio_init(D15,GPO,0);	//初始化A19引脚为输出，默认输出低电平    蜂鸣器
    for(;;)
    {
      

//		  gpio_set(A19,1);			//设置A16引脚为高电平
//		  gpio_set(A19,0);			//设置A16引脚为低电平
      gpio_turn(A19);			//翻转B0电平状态
//      gpio_turn(D15);                      //蜂鸣器
//      gpio_turn(E6);			//翻转B0电平状态
//      gpio_turn(E7);			//翻转B0电平状态
      systick_delay_ms(500);

    }

}


//测试串口
void test_uart(void)
{
    get_clk();//上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置

    //相关的库函数在 MK60DN10_uart.c 里面
    uart_init(uart2,9600);//初始化串口2，波特率为9600，引脚对应查看MK60DN10_port_cfg.c
    for(;;)
    {
        uart_putchar(uart2,0x5a);   //uart3发送 0x5a
        systick_delay_ms(500);


    }

}


//测试摄像头
void test_camera(void)
{

    uint8 dis_image[60][130];
    uint16 dat;
    uint8 num;


    get_clk();//上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置
    uart_init (uart2, 115200);                          //初始换串口与电脑通信
    camera_init();
    // OLED_Init();

    // OLED_Fill(0x00);         //清屏


    while(1)
    {
        if(mt9v032_finish_flag)
        {
            mt9v032_finish_flag = 0;
            //uart_putbuff(uart2, (uint8_t *)image, ROW*COL);

            //对数据进行抽点然后在显示
            
            // for(num=0; num<60; num++)
            // {
            //     memcpy(dis_image[num],&image[num][15],144);
            // }
            // dis_bmp(60,130,dis_image[0],70);
            // seekfree_sendimg_032();
            // systick_delay_ms(800);
            
        }

    }
}

void test_motor(void)
{
    get_clk();//上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置
    ftm_pwm_init(ftm3,ftm_ch4,15000,0);//初始化ftm3模块，4通道为15kHZ，占空比为百分之0，默认精度为10000 引脚对应查看MK60DN10_port_cfg.h
    ftm_pwm_init(ftm3,ftm_ch5,15000,0);//左电机正转
    ftm_pwm_init(ftm3,ftm_ch6,15000,0);//右电机正转
    ftm_pwm_init(ftm3,ftm_ch7,15000,0);
    ftm_pwm_init(STEER_FTM, STEER_CH, 100, 1735);
    while(1)
    {
        ftm_pwm_duty(ftm3,ftm_ch4,1000);
        ftm_pwm_duty(ftm3,ftm_ch7,1000);
        //systick_delay_ms(400);
        
        // for(int i = 0; i <= 150; i++)
        // {
        //     systick_delay_ms(5);
        //     ftm_pwm_duty(STEER_FTM, STEER_CH, 1550 - i);
        // }
        // for(int i = 0; i <= 150; i++)
        // {
        //     systick_delay_ms(5);
        //     ftm_pwm_duty(STEER_FTM, STEER_CH, 1400 + i);
        // }
        // for(int i = 0; i <= 150; i++)
        // {
        //     systick_delay_ms(5);
        //     ftm_pwm_duty(STEER_FTM, STEER_CH, 1550 + i);
        // }
        // for(int i = 0; i <= 300; i++)
        // {
        //     systick_delay_ms(6);
        //     ftm_pwm_duty(STEER_FTM, STEER_CH, 1700 - i);
        // }
        //while(1);
    }
}
void test_MPU6050(void)
{
    get_clk();
    IIC_init();
    InitMPU6050();
    OLED_Init();
    OLED_Fill(0x00);
    while(1)
    {
        Get_AccData();
        Get_Gyro();
        OLED_Print_Num1(0, 0, mpu_acc_x);
        OLED_Print_Num1(0, 2, mpu_acc_y);
        OLED_Print_Num1(0, 4, mpu_acc_z);
        

        systick_delay_ms(30);
    }

}

/***红外测距**/
void test_GP2Y0A0(void)
{
    int16 distance;
    get_clk();
    OLED_Init();
    OLED_Fill(0X00);
    adc_init(GP2Y0A0);

    while(1)
    {

        distance = adc_once(ADC0_SE5b, ADC_8bit);
        OLED_Print_Num1(0, 0, distance);  
    }



}
void test_middleline(void){


}


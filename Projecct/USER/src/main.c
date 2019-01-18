/********************************************
南京工业大学  疾风2.0  K66+MT9V032  
author：刘传顺
硬件版本:smartcar K66总钻风V2.0 18.12.22
软件版本:V1.1
最后更新:2018年1月16日

------------------------------------
软件版本： IAR 8.2 or MDK 5.17
目标核心： K66FX1M0VLQ18
============================================
MT9V032接线定义：
------------------------------------
    模块管脚            单片机管脚
    SDA(51的RX)         PTC17
    SCL(51的TX)         PTC16
	场中断(VSY)         PTC6
    像素中断(PCLK)      PTC18
    数据口(D0-D7)       PTC8-PTC15

    串口
    波特率 115200
    数据位 8
    校验位 无
    停止位 1位
    流控   无
	串口连接：
    RX                  PTD3
    TX                  PTD2
============================================

摄像头分辨率设置为           80*60

*********************************************/
#include "headfile.h"
#include "test.c"







uint8 dis_image[64][128];
uint16 dat;
uint8 image_threshold;  //图像阈值
uint32 use_time;        

int main(void)
{
    uint8 num;
    int i;
    uint8 *p;

    /************************以下为测试例程*********************/
//    test_led();
//    test_uart();
//    test_camera();
//    test_motor();



    /************************以上为测试例程*********************/

    get_clk();//上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置
    //gpio_init(A19,GPO,0);	//初始化A19引脚为输出，默认输出低电平
    //gpio_init(D15,GPO,1);	//初始化D15引脚为输出，默认输出低电平    蜂鸣器
    //systick_delay_ms(300);
    //gpio_set(D15,0);                   //关蜂鸣器 
    
    
    //uart_init (uart2, 115200);                          //初始换串口与电脑通信
    //camera_init();
    //OLED_Init();
    //OLED_Fill(0x00);         //清屏

    
    //ftm_pwm_init(ftm3,ftm_ch4,15000,0);//初始化ftm3模块，4通道为15kHZ，占空比为百分之0，默认精度为10000 引脚对应查看MK60DN10_port_cfg.h
    //ftm_pwm_init(ftm3,ftm_ch5,15000,0);
    //ftm_pwm_init(ftm3,ftm_ch6,15000,0);
    //ftm_pwm_init(ftm3,ftm_ch7,15000,0);
    
    
    pit_init_ms(pit0,500);								//定时5ms后中断
    set_irq_priority(PIT0_IRQn,3);						//设置优先级,根据自己的需求设置
    enable_irq(PIT0_IRQn);								//打开pit0的中断开关
    EnableInterrupts;									//打开总的中断开关


    while(1)
    {
        while(mt9v032_finish_flag)
        {
            mt9v032_finish_flag = 0;
            
            pit_time_start(pit1);
            image_threshold = OtsuThreshold(image[0],COL,ROW);  //大津法计算阈值    //60*80图像计算时间为907us
            use_time = pit_time_get(pit1)/bus_clk_mhz;          //计算大津法程序消耗时间，单位微秒。

            pit_close(pit1);
//            printf("use_time = %dus\n",  use_time);
            
            
//            //发送二值化图像至上位机
//            p = image[0];
//            uart_putchar(uart2,0x00);uart_putchar(uart2,0xff);uart_putchar(uart2,0x01);uart_putchar(uart2,0x01);//发送命令
//            for(i=0; i<COL*ROW; i++)
//            {
//                if(p[i]>image_threshold)    uart_putchar(uart2,0xff);
//                else                        uart_putchar(uart2,0x00);
//            }

//            seekfree_sendimg_032();                             //发送灰度至上位机
//            systick_delay_ms(400);


            //dui 数据进行抽点然后在显示
            for(num=0; num<ROW; num++)
            {
                memcpy(dis_image[num],&image[num][0],COL);
            }
            dis_bmp(64,128,dis_image[0],image_threshold);
            systick_delay_ms(400);
            motor();
        }

    }

}


void PIT0_IRQHandler(void)
{
    PIT_FlAG_CLR(pit0);
    gpio_turn(A19);			//翻转电平状态

}




//uint8  data1[8] = {1,2,3,4,5,6,7,8};
//uint16 data2 = 60000;
//uint32 data3 = 600051;
//
//uint8  data11[8];
//uint16 data22;
//uint32 data33;
//
//int main(void)
//{
//	get_clk();//上电后必须运行一次这个函数，获取各个频率信息，便于后面各个模块的参数设置
//
//	//相关的库函数在 MK60DN10_flash.c 里面
//	FLASH_Init();			//初始化flash模块
//	FLASH_EraseSector(10);	//擦除扇区
//	FLASH_WriteSector(10,(const uint8 *)data1,8,0);
//	FLASH_WriteSector(10,(const uint8 *)&data2,8,8);
//	FLASH_WriteSector(10,(const uint8 *)&data3,8,16);
//    for(;;)
//	{
//		//读取保存的data1数组数据
//		data11[0] = flash_read(10,0,uint8);
//		data11[1] = flash_read(10,1,uint8);
//		data11[2] = flash_read(10,2,uint8);
//		data11[3] = flash_read(10,3,uint8);
//        data11[4] = flash_read(10,4,uint8);
//        data11[5] = flash_read(10,5,uint8);
//        data11[6] = flash_read(10,6,uint8);
//        data11[7] = flash_read(10,7,uint8);
//
//		//读取保存的data2数组数据
//		data22 = flash_read(10,8,uint16);
//
//		//读取保存的data3变量数据
//		data33 = flash_read(10,16,uint32);
//	}
//}



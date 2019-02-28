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

uint8 dis_image[64][128];      
int main(void)
{
    /************************以下为测试例程*********************/
//    test_led();
//    test_uart();
//    test_camera();
//    test_motor();
//    test_MPU6050();
//    test_GP2Y0A0();
//    test_middleline();
    /************************测试例程结束*********************/


    everythinginit();  /***初始化所有模块**********/
    
    while(1)
    {

        while(mt9v032_finish_flag)
        {   

            mt9v032_finish_flag = 0;

            Handle_Gray();                                     /**处理图像并进行二值化**/

            Image_Handle(image[0]);                            /****处理图像，取中线***/

            Current_Point = Point_Average();                   /*******加权平均*******/

            Steer_Duty = Position_PID(80, Current_Point);      /*****位置式pid解算****/

            Steer_Duty = Range_Protect(Steer_Duty, 1400, 1700);/*******限幅保护*******/
            
            ftm_pwm_duty(STEER_FTM, STEER_CH, Steer_Duty);     /****改变舵机占空比****/

 
            
            for(int num=0; num<64; num++)
            {
                memcpy(dis_image[num],&image[num][16],128);
            }
            dis_bmp(60,128,dis_image[0],70);
            
            
            // uint32 use_time;
            // pit_time_start(pit1);
            //image_threshold = OtsuThreshold(image[0],COL,ROW);  //大津法计算阈值    //60*80图像计算时间为907us                     
        }
        
    }

}



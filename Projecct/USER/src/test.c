#include "headfile.h"
#include <string.h>

//����led
void test_led(void)
{
    get_clk();//�ϵ���������һ�������������ȡ����Ƶ����Ϣ�����ں������ģ��Ĳ�������

    //��صĿ⺯������ MK60DN10_gpio.c ����
    gpio_init(A19,GPO,0);	//��ʼ��A19����Ϊ�����Ĭ������͵�ƽ
//         gpio_init(E6,GPO,0);	//��ʼ��E6����Ϊ�����Ĭ������͵�ƽ
//         gpio_init(E7,GPO,0);	//��ʼ��E7����Ϊ�����Ĭ������͵�ƽ
//         gpio_init(E8,GPO,0);	//��ʼ��E7����Ϊ�����Ĭ������͵�ƽ
//    gpio_init(E1, GPO, 0);
//    gpio_init(D15,GPO,0);	//��ʼ��A19����Ϊ�����Ĭ������͵�ƽ    ������
    for(;;)
    {
      

//		  gpio_set(A19,1);			//����A16����Ϊ�ߵ�ƽ
//		  gpio_set(A19,0);			//����A16����Ϊ�͵�ƽ
      gpio_turn(A19);			//��תB0��ƽ״̬
//      gpio_turn(D15);                      //������
//      gpio_turn(E6);			//��תB0��ƽ״̬
//      gpio_turn(E7);			//��תB0��ƽ״̬
      systick_delay_ms(500);

    }

}


//���Դ���
void test_uart(void)
{
    get_clk();//�ϵ���������һ�������������ȡ����Ƶ����Ϣ�����ں������ģ��Ĳ�������

    //��صĿ⺯���� MK60DN10_uart.c ����
    uart_init(uart2,9600);//��ʼ������2��������Ϊ9600�����Ŷ�Ӧ�鿴MK60DN10_port_cfg.c
    for(;;)
    {
        uart_putchar(uart2,0x5a);   //uart3���� 0x5a
        systick_delay_ms(500);


    }

}


//��������ͷ
void test_camera(void)
{

    uint8 dis_image[60][130];
    uint16 dat;
    uint8 num;


    get_clk();//�ϵ���������һ�������������ȡ����Ƶ����Ϣ�����ں������ģ��Ĳ�������
    uart_init (uart2, 115200);                          //��ʼ�����������ͨ��
    camera_init();
    // OLED_Init();

    // OLED_Fill(0x00);         //����


    while(1)
    {
        if(mt9v032_finish_flag)
        {
            mt9v032_finish_flag = 0;
            //uart_putbuff(uart2, (uint8_t *)image, ROW*COL);

            //�����ݽ��г��Ȼ������ʾ
            
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
    get_clk();//�ϵ���������һ�������������ȡ����Ƶ����Ϣ�����ں������ģ��Ĳ�������
    ftm_pwm_init(ftm3,ftm_ch4,15000,0);//��ʼ��ftm3ģ�飬4ͨ��Ϊ15kHZ��ռ�ձ�Ϊ�ٷ�֮0��Ĭ�Ͼ���Ϊ10000 ���Ŷ�Ӧ�鿴MK60DN10_port_cfg.h
    ftm_pwm_init(ftm3,ftm_ch5,15000,0);//������ת
    ftm_pwm_init(ftm3,ftm_ch6,15000,0);//�ҵ����ת
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

/***������**/
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


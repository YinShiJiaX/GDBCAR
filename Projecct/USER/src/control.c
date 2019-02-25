#include "control.h"



int16 Speed_L,Speed_R;
/****************************�ٶȲ���*****************************/
int16            SetSpeedMax=1400;                     //����ٶ�

/****************************ת�����*****************************/
float           Turn_Kp=50;                                   //  126
float           Turn_Kd=2;                                  //  14.5
int32 GyroX=0,GyroXo=0;//�����ǣ�X�ᣩ�ɼ�ֵ
float Gyroroll,GyrorollOld[20];//ת����ٶ�
static float g_fDirectionControlOutOld, fvalueNew,fvalueOld,g_fDirectionControlOutNew,g_nDirectionControlPeriod;
float  g_fDirectionControlOut=0;
int32 Steer_Duty    = 0;/***��ǰ��Ҫ�������ռ�ձ�***/




void motor()
{
//  Speed_L=(int)(SetSpeedMax+g_fDirectionControlOut);
//  Speed_R=(int)(SetSpeedMax-g_fDirectionControlOut);
  
  Speed_L=1500;
  Speed_R=2200;
  
  if(Speed_L > 6000)       Speed_L =6000;            //����޷�
  else if(Speed_L < -6000)   Speed_L = -6000;
  
  if(Speed_R > 6000)  Speed_R = 6000;
  else if(Speed_R < -6000) Speed_R = -6000;
  
  
  if(Speed_L>=0)                                 //angle����0����ǰ��С��0�����
  {
    ftm_pwm_duty(ftm3,ftm_ch4,0);
    ftm_pwm_duty(ftm3,ftm_ch5,Speed_L + MOTOR_DEAD_VAL_L);    //����������ѹ(uint32_t)(Speed_L + MOTOR_DEAD_VAL_L)
  }
  else
  {
    ftm_pwm_duty(ftm3,ftm_ch5,0);
    ftm_pwm_duty(ftm3,ftm_ch4,Speed_L + MOTOR_DEAD_VAL_L);    //����������ѹ
  }
  
  if(Speed_R >=0)    //angle����0����ǰ��С��0�����     �ҵ��������ѹ50
  {
    ftm_pwm_duty(ftm3,ftm_ch7,0);
    ftm_pwm_duty(ftm3,ftm_ch6,Speed_R + MOTOR_DEAD_VAL_R);    //����������ѹ(uint32_t)(Speed_R + MOTOR_DEAD_VAL_R)
  }
  else
  {
    ftm_pwm_duty(ftm3,ftm_ch6,0);
    ftm_pwm_duty(ftm3,ftm_ch7,Speed_R + MOTOR_DEAD_VAL_R);    //����������ѹ(uint32_t)(Speed_R + MOTOR_DEAD_VAL_R)
  }
  
}

/***********************�޷�����*************************/
int32 
Range_Protect(int32 duty, int32 min, int32 max)
{
	if (duty >= max)
	{
		return max;
	}
	if (duty <= min)
	{
		return min;
	}
	else
	{
		return duty;
	}
}

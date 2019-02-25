#include "control.h"



int16 Speed_L,Speed_R;
/****************************速度参数*****************************/
int16            SetSpeedMax=1400;                     //最大速度

/****************************转向参数*****************************/
float           Turn_Kp=50;                                   //  126
float           Turn_Kd=2;                                  //  14.5
int32 GyroX=0,GyroXo=0;//陀螺仪（X轴）采集值
float Gyroroll,GyrorollOld[20];//转向角速度
static float g_fDirectionControlOutOld, fvalueNew,fvalueOld,g_fDirectionControlOutNew,g_nDirectionControlPeriod;
float  g_fDirectionControlOut=0;
int32 Steer_Duty    = 0;/***当前需要给舵机的占空比***/




void motor()
{
//  Speed_L=(int)(SetSpeedMax+g_fDirectionControlOut);
//  Speed_R=(int)(SetSpeedMax-g_fDirectionControlOut);
  
  Speed_L=1500;
  Speed_R=2200;
  
  if(Speed_L > 6000)       Speed_L =6000;            //输出限幅
  else if(Speed_L < -6000)   Speed_L = -6000;
  
  if(Speed_R > 6000)  Speed_R = 6000;
  else if(Speed_R < -6000) Speed_R = -6000;
  
  
  if(Speed_L>=0)                                 //angle大于0，向前，小于0，向后
  {
    ftm_pwm_duty(ftm3,ftm_ch4,0);
    ftm_pwm_duty(ftm3,ftm_ch5,Speed_L + MOTOR_DEAD_VAL_L);    //加入死区电压(uint32_t)(Speed_L + MOTOR_DEAD_VAL_L)
  }
  else
  {
    ftm_pwm_duty(ftm3,ftm_ch5,0);
    ftm_pwm_duty(ftm3,ftm_ch4,Speed_L + MOTOR_DEAD_VAL_L);    //加入死区电压
  }
  
  if(Speed_R >=0)    //angle大于0，向前，小于0，向后     右电机死区电压50
  {
    ftm_pwm_duty(ftm3,ftm_ch7,0);
    ftm_pwm_duty(ftm3,ftm_ch6,Speed_R + MOTOR_DEAD_VAL_R);    //加入死区电压(uint32_t)(Speed_R + MOTOR_DEAD_VAL_R)
  }
  else
  {
    ftm_pwm_duty(ftm3,ftm_ch6,0);
    ftm_pwm_duty(ftm3,ftm_ch7,Speed_R + MOTOR_DEAD_VAL_R);    //加入死区电压(uint32_t)(Speed_R + MOTOR_DEAD_VAL_R)
  }
  
}

/***********************限幅保护*************************/
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

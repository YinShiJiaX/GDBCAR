#include "control.h"

int32 Speed_Left = 0;
int32 Speed_Right = 0;
int32 Motor_Duty_Left = 1000;
int32 Motor_Duty_Right = 1000;
int32 Steer_Duty    = 0;/***当前需要给舵机的占空比***/




void Motor_Control(void)
{
  int32 Set_Speed = 0;
  if(Current_Point >= 83 || Current_Point <= 77)
  {
    Set_Speed = 150;
    // Steer_pid[0] = 7;
    // Steer_pid[2] = 15;
  }
  else
  {
    Set_Speed = 250;
    // Steer_pid[0] = 7;
    // Steer_pid[2] = 15;
  }
  Motor_Duty_Left += Increment_PID(Speed_Left, Set_Speed);
  Motor_Duty_Right += Increment_PID(Speed_Right, Set_Speed);

  /******左轮控制*******/
  if(Motor_Duty_Left >= 0)
  {
    Motor_Duty_Left = Range_Protect(Motor_Duty_Left, 100, 3200);
    ftm_pwm_duty(MOTOR_FTM,LEFT_FORWARD_CH,Motor_Duty_Left);
    ftm_pwm_duty(MOTOR_FTM,LEFT_BACK_CH,0);
  }
  else
  {
    Motor_Duty_Left = (0-Motor_Duty_Left);
    Motor_Duty_Left = Range_Protect(Motor_Duty_Left, 100, 3200);
    ftm_pwm_duty(MOTOR_FTM,LEFT_BACK_CH,Motor_Duty_Left);
    ftm_pwm_duty(MOTOR_FTM,LEFT_FORWARD_CH,0);
  }
  /******左轮控制结束*******/

  /******右轮控制**********/
  if(Motor_Duty_Right >= 0)
  {
    Motor_Duty_Right = Range_Protect(Motor_Duty_Right, 100, 3200);
    ftm_pwm_duty(MOTOR_FTM,RIGHT_FORWARD_CH,Motor_Duty_Right);
    ftm_pwm_duty(MOTOR_FTM,RIGHT_BACK_CH,0);
  }
  else
  {
    Motor_Duty_Right = (0-Motor_Duty_Right);
    Motor_Duty_Right = Range_Protect(Motor_Duty_Right, 100, 3200);
    ftm_pwm_duty(MOTOR_FTM,RIGHT_BACK_CH,Motor_Duty_Right);
    ftm_pwm_duty(MOTOR_FTM,RIGHT_FORWARD_CH,0);
  }
  /******右轮控制结束*******/
  //OLED_Print_Num1(0, 0, Motor_Duty_Right);
  //OLED_Print_Num1(0, 2, Motor_Duty_Left);
  //OLED_Print_Num1(0, 4, Current_Point);
  

  
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

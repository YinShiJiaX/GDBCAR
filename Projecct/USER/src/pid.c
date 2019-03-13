#include "headfile.h"

/************Macro definition******************/
#define STEER_MID 1550

/************Variable definition***************/
float Steer_pid[3] = {7.5, 0, 3};/*P, I, D*/
float Motor_pid[3] = {0, 2, 0};/*P, I, D*/

PID STEER_PID;
PID MOTOR_PID;



/***Incremental PID caculation(Just for motor engine)********************/
int16 Position_PID(int32 Need_Point, int32 Actual_Point)
{
	/* current deviation */
	register int32 iError;	
	/* real output */
	register int32 Actual;		
	
	/* caculate current deviation */
	iError = Need_Point - Actual_Point;	
	Actual = Steer_pid[0] * iError +  Steer_pid[1] * (STEER_PID.LastError + STEER_PID.PrevError) + Steer_pid[2] * (iError - STEER_PID.LastError);
	/* update last and before last deviation*/
	STEER_PID.LastError = iError;
	STEER_PID.PrevError = STEER_PID.LastError;
    
	
	return  STEER_MID - Actual; 

}

/***Position PID caculation(Just for steer engine)********************/
int16 Increment_PID(int32 Actual_Speed, int32 Set_Speed)
{
	int32 iError;
	int32 Increase;	

	iError = Set_Speed - Actual_Speed;

	Increase = Motor_pid[0] * (iError - MOTOR_PID.LastError)
			 + Motor_pid[1] * iError
			 + Motor_pid[2] * (iError - 2 * MOTOR_PID.LastError + MOTOR_PID.PrevError);
	
	MOTOR_PID.PrevError = MOTOR_PID.LastError;
	MOTOR_PID.LastError = iError;
	MOTOR_PID.LastSpeed = Actual_Speed;
	return Increase;
}

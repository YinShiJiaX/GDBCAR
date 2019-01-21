#include "headfile.h"

/************Macro definition******************/
#define STEER_MID 1550

/************Variable definition***************/
float Steer_pid[3] = {10, 0, 0};/*P, I, D*/
float Motor_pid[3] = {0, 0, 0};/*P, I, D*/

PID STEER_PID;
PID MOTOR_PID;



/***Position PID caculation********************/
int16 Position_PID(int32 Need_Point, int32 Actual_Point)
{
	/* 当前误差 */
	register int32 iError;	
	/* 最后得出的实际输出值 */
	register int32 Actual;		
	
	/* 计算当前误差 */
	iError = Need_Point - Actual_Point;	
	Actual = Steer_pid[0] * iError +  Steer_pid[1] * (STEER_PID.LastError + STEER_PID.PrevError) + Steer_pid[2] * (iError - STEER_PID.LastError);
	/* 更新上次误差 */
	STEER_PID.LastError = iError;
	STEER_PID.PrevError = STEER_PID.LastError;
    
	
	return  STEER_MID - Actual; 

}
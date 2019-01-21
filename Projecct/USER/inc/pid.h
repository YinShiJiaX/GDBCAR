/*This module is used to get the PID results to control the steering engine and the motor of the car*/


#ifndef _PID_H_
#define _PID_H_

#include "headfile.h"
/**Struct definitions***/
typedef struct PID
{
	/* 误差累计 */
	long SumError;		
	/*上次误差*/
	int LastError;	//Error[-1]
	/* 上上次误差 */
	int PrevError;	//Error[-2]	
	/* 上次速度 */
	int LastSpeed;	//Speed[-1]
} PID;

/******External Variable declaration******/


/********Function declarations***********/
int16 Position_PID(int32 Need_Point, int32 Actual_Point);

#endif/*_PID_H_*/

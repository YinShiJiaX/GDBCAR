#include "headfile.h"

extern int32 Steer_Duty;
extern int32 Speed_Left;
extern int32 Speed_Right;
extern int32 Motor_Duty_Left;
extern int32 Motor_Duty_Right;

void Motor_Control(void);
int32 Range_Protect(int32 duty, int32 min, int32 max);
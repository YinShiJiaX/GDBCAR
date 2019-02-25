#include "headfile.h"

#define MOTOR_DEAD_VAL_L  0
#define MOTOR_DEAD_VAL_R  0
extern int32 Steer_Duty;
void motor();
int32 Range_Protect(int32 duty, int32 min, int32 max);
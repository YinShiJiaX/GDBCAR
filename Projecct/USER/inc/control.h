#include "headfile.h"

#define MOTOR_DEAD_VAL_L  0
#define MOTOR_DEAD_VAL_R  0

void motor();
int32 range_protect(int32 duty, int32 min, int32 max);
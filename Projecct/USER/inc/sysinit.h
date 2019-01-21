/****************************************************************************
 * This module is used to initial all the software module and hardware module.
 * *************************************************************************/
#ifndef __SYSINIT_H__
#define __SYSINIT_H__

/**********************************端口宏定义********************************/
#define LED           A19
#define BUZZER        D15

#define MOTOR_FTM     ftm3
#define STEER_FTM     ftm0

#define LEFT_FORWARD_CH  ftm_ch5
#define LEFT_BACK_CH     ftm_ch4
#define RIGHT_FORWARD_CH ftm_ch6
#define RIGHT_BACK_CH    ftm_ch7
#define STEER_CH         ftm_ch4




void everythinginit(void);



#endif/*__SYSINIT_H__*/
WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
#ifndef BUZZER_H
#define BUZZER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <stdint.h>



#define TONE1_PER 9426
#define TONE2_PER 11210
#define TONE3_PER 7062
#define TONE4_PER 18853



void buzzer_init(void);
void buzzer_play(void);
void buzzer_stop(void);


#endif // BUZZER_H_

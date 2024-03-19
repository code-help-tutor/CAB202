WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
#include <avr/io.h>
#include <avr/interrupt.h>
#include "buzzer.h"
#include <math.h>
#include <stdint.h>



void buzzer_init(void) {


  // Set PORTB PIN0 as output for the buzzer
  PORTB.DIRSET = PIN0_bm;

  // Configure TCA0 to drive the buzzer
  TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
  TCA0.SINGLE.CMP0 = TONE1_PER; 
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}

void buzzer_play(void) {

}
void buzzer_stop(void) {
  // Disable the TCA0 to stop the buzzer
  TCA0.SINGLE.CMP0BUF = 0;
}
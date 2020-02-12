#include <xc.h>
#include <stdint.h>

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 8000000
#endif

#define R_SW GP5
#define B_SW GP4
#define G_SW GP3

#define R_SW_ON 0x01
#define B_SW_ON 0x02
#define G_SW_ON 0x04

#define CHATT_CNT 3

uint8_t pwm_on[3] = {1, 1, 1};

void __interrupt() isr(void) {
  static uint8_t cnt     = 0;
  static uint8_t duty[3] = {0x80, 0xff, 0x00};
  static int8_t  step[3] = {1, 1, 1};

  if (T1IF) {
    TMR1H = 0xd8;
    TMR1L = 0xf0;

    if (pwm_on[0]) {
      step[0]  = (duty[0] == 0) ? 1 : (duty[0] == 255) ? -1 : step[0];
      duty[0] += step[0];
    }

    if (pwm_on[1]) {
      step[1]  = (duty[1] == 0) ? 1 : (duty[1] == 255) ? -1 : step[1];
      duty[1] += step[1];
    }

    if (pwm_on[2]) {
      step[2]  = (duty[2] == 0) ? 1 : (duty[2] == 255) ? -1 : step[2];
      duty[2] += step[2];
    }

    T1IF  = 0;
  }

  if (T2IF) {
    cnt++;
    GP0  = (cnt <= duty[0]) ? 1 : 0;
    GP1  = (cnt <= duty[1]) ? 1 : 0;
    GP2  = (cnt <= duty[2]) ? 1 : 0;
    T2IF = 0;
  }
}

uint8_t read_sw(void) {
  static uint8_t sw;
  static uint8_t r_cnt[2];
  static uint8_t b_cnt[2];
  static uint8_t g_cnt[2];

  if ((sw & R_SW_ON) == 0) {
    if (R_SW == 0)
      r_cnt[0]++;
    else
      r_cnt[0] = 0;

    if (r_cnt[0] > CHATT_CNT) {
      r_cnt[0] = 0;
      sw |= R_SW_ON;
      return sw;
    }
  } else {
    if (R_SW == 1)
      r_cnt[1]++;
    else
      r_cnt[1] = 0;

    if (r_cnt[1] > CHATT_CNT) {
      r_cnt[1] = 0;
      sw &= ~R_SW_ON;
    }
  }

  if ((sw & B_SW_ON) == 0) {
    if (B_SW == 0)
      b_cnt[0]++;
    else
      b_cnt[0] = 0;

    if (b_cnt[0] > CHATT_CNT) {
      b_cnt[0] = 0;
      sw |= B_SW_ON;
      return sw;
    }
  } else {
    if (B_SW == 1)
      b_cnt[1]++;
    else
      b_cnt[1] = 0;

    if (b_cnt[1] > CHATT_CNT) {
      b_cnt[1] = 0;
      sw &= ~B_SW_ON;
    }
  }

  if ((sw & G_SW_ON) == 0) {
    if (G_SW == 0)
      g_cnt[0]++;
    else
      g_cnt[0] = 0;

    if (g_cnt[0] > CHATT_CNT) {
      g_cnt[0] = 0;
      sw |= G_SW_ON;
      return sw;
    }
  } else {
    if (G_SW == 1)
      g_cnt[1]++;
    else
      g_cnt[1] = 0;

    if (g_cnt[1] > CHATT_CNT) {
      g_cnt[1] = 0;
      sw &= ~G_SW_ON;
    }
  }

  return 0;
}

void main(void) {
  OSCCON = 0x70;
  GPIO   = 0x00;
  ANSEL  = 0x00;
  CMCON0 = 0x07;
  WPU    = 0x30;
  nGPPU  = 0;
  TRISIO = 0x38;

  T1CON = 0x00;
  TMR1H = 0xd8;
  TMR1L = 0xf0;

  T2CON = 0x00;
  PR2   = 0x63;

  TMR1IE = 1;
  TMR2IE = 1;
  PEIE   = 1;
  GIE    = 1;

  TMR1ON = 1;
  TMR2ON = 1;

  uint8_t sw = 0;

  while (1) {
    sw = read_sw();
    pwm_on[0] = sw & R_SW_ON ? pwm_on[0] ^ 1 : pwm_on[0];
    pwm_on[1] = sw & B_SW_ON ? pwm_on[1] ^ 1 : pwm_on[1];
    pwm_on[2] = sw & G_SW_ON ? pwm_on[2] ^ 1 : pwm_on[2];
  }
}

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

uint8_t tflg[3] = {1, 1, 1};
uint8_t cnt     = 0;
uint8_t duty[3] = {0x80, 0xff, 0x00};
int8_t  step[3] = {1, 1, 1};

void __interrupt() isr(void) {
  if (T1IF) {
    TMR1H = 0xd8;
    TMR1L = 0xf0;

    if (tflg[0]) {
      step[0]  = (duty[0] == 0) ? 1 : (duty[0] == 255) ? -1 : step[0];
      duty[0] += step[0];
    }
    
    if (tflg[1]) {
      step[1]  = (duty[1] == 0) ? 1 : (duty[1] == 255) ? -1 : step[1];
      duty[1] += step[1];
    }
    
    if (tflg[2]) {
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

  while (1) {
    if (GP5 == 0) {
      tflg[0] = tflg[0] ^ 1;

      while (GP5 == 0);
      __delay_ms(50);
    } else if (GP4 == 0) {
      tflg[1] = tflg[1] ^ 1;

      while (GP4 == 0);
      __delay_ms(50);
    } else if (GP3 == 0) {
      tflg[2] = tflg[2] ^ 1;

      while (GP3 == 0);
      __delay_ms(50);
    }
  }
}

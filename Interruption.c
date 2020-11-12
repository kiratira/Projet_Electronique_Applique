/*
 * File:   Interruption.c
 * Author: flore
 *
 * Created on 13 octobre 2020, 13:44
 */


#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF  

#include <xc.h>
#define _XTAL_FREQ 16000000
#define Led4 PORTBbits.RB4
#define Led5 PORTBbits.RB5

char tampon;

void __interrupt() receivedData(void);

void main(void) {
    //Reset
    INTCON = 0;
    PIE1 = 0;
    
   
    //interruption
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE1bits.RCIE = 1;
    RCSTAbits.CREN = 1;
    //I/O
    CMCON = 7;           // disable comparator
    CVRCON = 0;         
    ADCON1 = 6;         //disable analog
    TRISA = 0b00000011;
    TRISB = 0b00000000;
    //UART
    TRISC = 255;
    
    Led4 = 0;
    Led5 = 0;
    
    while(1){
        Led4 = !Led4;
        __delay_ms(200);
    }
}

void __interrupt() receivedData(void){
    
    tampon = RCREG;
    Led5 = 1;   
    __delay_ms(2000);
    PIR1bits.RCIF = 0;
    
}
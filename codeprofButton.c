/*
 * File:   Cours1.c
 * Author: Gaetan
 *
 * Created on 22 septembre 2020, 14:00
 */
// PIC16F876A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#define _XTAL_FREQ 16000000

//char test_BP(char flag);
void test_BP(void);
__bit cligno = 0;


void main(void) {
    //char cligno = 0;
    CMCON = 7;              //D?sactivation des comparateurs
    CVRCON = 0;
    ADCON1 = 6;             //D?sactivation des entr?e analogiques
    TRISA = 0b00000111;
    TRISB = 0b00000000;
    PORTB = 255;
    
    
    
    while(1)
    {
        //cligno = test_BP(cligno);
        test_BP();
        while(cligno == 1)
        {
            PORTBbits.RB4 = !PORTBbits.RB4;
            for (int k=0; k<10 ; k++)
            {
                //cligno = test_BP(cligno);
                test_BP();
                if(cligno == 0)
                    break;
                __delay_ms(100);
            }
        }
    }
    return;
}


//char test_BP(char flag)
void test_BP()
{
    static char etat = 1;
        
    if (PORTAbits.RA0 == 0 && etat == 1)
        {
            //flag = !flag;
            cligno = !cligno;
            etat = 0;
        }
        
    if (PORTAbits.RA0 == 1)
        {
            etat = 1;
        }
    //return flag;
}
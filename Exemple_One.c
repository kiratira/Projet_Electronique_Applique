/*
 * File:   Exemple_One.c
 * Author: flore
 *
 * Created on 22 septembre 2020, 14:00
 */

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
#define Button0 PORTAbits.RA0
#define Button1 PORTAbits.RA1
#define Led4 PORTBbits.RB4
#define Led5 PORTBbits.RB5

void reset(void);
void swap(void);


void main(void) {
   
    char etat = 1;
    CMCON = 7;           // disable comparator
    CVRCON = 0;         
    ADCON1 = 6;         //disable analog
    TRISA = 0b00000011;
    TRISB = 0b00000000;
    
    reset();
    
    while(1)
    { 
        
        //// Obligation de relachement
        /*      
       if(!Button0)
        {
           Led4 = !Led4;
           Led5 = !Led5;
           //__delay_ms(300); // delay
           
           while(!Button0); // vérification du relachement du button (Tant que le button est DOWN)
        } */
       
        
        //// Check de l'état + disponibilité autre part. A VERIFIER
        
       if(!Button0 && etat == 1)
        {
           etat = 0; 
           while(1)
           {
                swap();                             
                for( int i=0; i<10; i++)
                {
                    if(Button0)etat = 1;
                    if(!Button0 && etat == 1){
                        etat = 0; 
                        break;
                    }
                    __delay_ms(100);
                }
                if(Button0){
                    etat = 1;
                    break;
                }
           }
           reset();
        }    
    }
       
    return;
}

void reset(void){
    Led4 = 0;
    Led5 = 0;
}

void swap(void){
    Led4 = !Led4;
    Led5 = !Led5;
}
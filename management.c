/* 
 * File:   main.c
 * Author: Youri-User
 *
 * Created on November 19, 2020, 3:04 PM
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

#define SURVITESSE 50
#define _XTAL_FREQ 20000000

void init_all(){
    i2c_init();
    LCD_init();
    EEPROM_initialization();
    Init_RTC;
}

void main(void) {
    init_all();
    
    unsigned char vitesse = 0;
    unsigned char tab_EEPROM[8]; // Format : AA   MM   JJ   HH   SS   Entier Vitesse   Decimale Vitesse
    while(1){
        if(unsigned char vitesse = get_vitesse(SURVITESSE)){
        // En cas de survitesse crée un enregistrement dans l'EEPROM
        if(vitesse = get_vitesse(SURVITESSE)){
            unsigned char tab_RTC[6];
            Read_RTC(tab_RTC);

            // Copie la date dans le tableau destiné a l'EEPROM
            for(unsigned char i = 0; i < 6; i++){
                tab_EEPROM[i] = tab_RTC[i];
            }
            tab_EEPROM[6] = vitesse;
            tab_EEPROM[7] = 0;
            
            Stockage_EEPROM(tab_RTC);
        }
    }
    return;
}

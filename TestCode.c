/*
 * File:   UART.c
 * Author: laure
 *
 * Created on 6 octobre 2020, 14:03
 */
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF 


#define _XTAL_FREQ 16000000
#include <xc.h>

void envoyer(char data);
int recevoir();
void affichage(int addr,int data);
void __interrupt() stop (void);
void main(void) {
    TRISC = 255;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 15;
    //////////////////////
    PORTB = 0;
    TRISB = 0b00000000;
    //////////////////////
    SSPSTAT = 0b10000000;
    SSPCON = 0b00101000;
    SSPADD = 39; //= I2C Master mode, clock = FOSC/(4 * (SSPADD + 1))
    SSPCON2 = 0;
    ////////////////////// Config interruption
    INTCON = 0;
    PIE1 = 0;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE1bits.RCIE = 1;
    RCSTAbits.CREN = 1;
    ///////////////
    int pos_afficheur[4]= {0b01000010,0b01000110,0b01000000,0b01001110};
    int pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};

    
    
    while(1){ 
    
        PORTBbits.RB4 = !PORTBbits.RB4;
        __delay_ms(150);
        
    }
        
    
    
    return;
}
void envoyer(char data){
    PIR1bits.TXIF = 0;
    TXSTAbits.TXEN = 1;
    TXREG = data;
    while(!PIR1bits.TXIF);
}
int recevoir(){
    int valeur;
    int cpt=0;
    RCSTAbits.CREN = 1;
    while(!PIR1bits.RCIF){
        cpt++;
        if(cpt>5000)return -1;
    }
    valeur = RCREG;
    PIR1bits.RCIF = 0;
    
    return valeur;   
    
}

void affichage(int addr,int data) // char parce que c'est plus léger (8nits))
{
    
    PIR1bits.SSPIF = 0; 
    SEN = 1;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;
    SSPBUF = addr;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;
    SSPBUF = data;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;
    PEN = 1;
    while(!PIR1bits.SSPIF);
    
    
}
void __interrupt() stop (void)
{
    
    
    PORTBbits.RB4 = 0;
    __delay_ms(1000);
    char buffer = RCREG;
    PIR1bits.RCIF = 0;

}
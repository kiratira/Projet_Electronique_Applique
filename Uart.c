/*
 * File:   Uart.c
 * Author: flore
 *
 * Created on 6 octobre 2020, 13:42
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

void sendDataUART(char data);
char getDataUART();
char waitFlag();
char sendData7Seg(char address, char data);
char tampon;
void main(void) {
    
    //UART
    TRISC = 255;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 25;
    //I2C
    SSPSTAT = 0b10000000;
    SSPCON = 0b00101000;
    SSPCON2 = 0;
    SSPADD = 39;
    //I/O
    CMCON = 7;           // disable comparator
    CVRCON = 0;         
    ADCON1 = 6;         //disable analog
    TRISA = 0b00000011;
    TRISB = 0b00000000;
    //interruption
    INTCON = 0b11000000;
    PIE1bits.RCIE = 1;
    RCSTAbits.CREN = 1;
    
    
    int pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
    char Seg[4] = {0b01000000,0b01000110,0b01001110,0b01000010};
    char txt[20];
    char cpt = 2;
    
    /*  Sending a "A"
    while(1){
        sendDataUART(65);
        __delay_ms(500);
    }
    */
    while(1){
        sendDataUART(65);
        __delay_ms(500);
    }
    /*
    while(1){
        
        Led4 = !Led4;
        __delay_ms(200);
        
        /*
        char data = 0;
        data = getDataUART();
        if(data == 43)cpt++;
        else if (data == 45)cpt--;
        if(cpt==4)cpt=3;
        else if(cpt==0)cpt=0;
        if(data < 64) sendData7Seg(Seg[cpt],pos_segment[data-48]);
        else sendData7Seg(Seg[cpt],pos_segment[data-55]);        
        sendDataUART(data);     
    }
     * */
    
    return;
}


void sendDataUART(char data){
    
    PIR1bits.TXIF = 0;
    TXSTAbits.TXEN = 1;
    TXREG = data;
    while(!PIR1bits.TXIF);
}

char getDataUART(){
    char data;
        
    RCSTAbits.CREN = 1;
    while(!PIR1bits.RCIF);
    data = RCREG;
    PIR1bits.RCIF = 0;
    
    return data;
}

char sendData7Seg(char address, char data){

    char flag = 0;
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1; 
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPBUF = address;
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPBUF = data;
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPCON2bits.PEN = 1;
    flag += waitFlag();   
    return flag;
    
}

char waitFlag(){
    char cpt = 0;   
    while(!PIR1bits.SSPIF){
        cpt++;
        if (cpt == 4999)return -1;
    }
    return 0;
}
/*
void __interrupt() receivedData(void){
    
    Led5 = !Led5;
    __delay_ms(2000);
    tampon = RCREG;
    PIR1bits.RCIF = 0;    
}
 * */
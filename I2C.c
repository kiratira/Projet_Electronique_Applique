/*
 * File:   I2C.c
 * Author: flore
 *
 * Created on 29 septembre 2020, 13:58
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


// 7/3/1/0
// Les puces des 7 segments sont à 100 kHz
#include <xc.h>
#define _XTAL_FREQ 16000000



char sendData(char address, char data);
char readRTC(char address);
char setRTC(char address,char time);
char waitFlag();


void main(void) {
    
    SSPSTAT = 0b10000000;
    SSPCON = 0b00101000;
    SSPCON2 = 0;
    SSPADD = 39;
    
    //INITIALISATION   
    char cnt = 0;
    char minute = 0;
    char seconde = 0;
    int pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
    char Seg[4] = {0b01000000,0b01000110,0b01001110,0b01000010};
    
    setRTC(0,0);
    setRTC(1,0);
    
    while(1) 
    {
        /*
        sendData(Seg1,pos_segment[cnt]);
        cnt += 1;
        if(cnt == 16)cnt = 0;
        __delay_ms(1000); // delay
        */
        /*
        for (int i = 0; i < 16; i++){
            sendData(Seg4,pos_segment[i]);
            __delay_ms(1000); // delay
        }
        */
        
        seconde = readRTC(0);
        minute = readRTC(1);
        
        sendData(Seg[2],pos_segment[seconde & 0b00001111]);
        sendData(Seg[0],pos_segment[(seconde & 0b01110000)>>4]);
        sendData(Seg[1],pos_segment[minute & 0b00001111]);
        sendData(Seg[3],pos_segment[(minute & 0b01110000)>>4]);
    }
    
    
    return;
}

char sendData(char address, char data){
    
    //verif de flag
    char flag = 0;
    
    //flag à 0
    PIR1bits.SSPIF = 0;
    
    //Start Condition + wait for the flag & verif
    SSPCON2bits.SEN = 1; 
    flag += waitFlag();
    
    //flag à 0
    PIR1bits.SSPIF = 0;
    
    //Load address + wait for the flag
    SSPBUF = address;
    flag += waitFlag();
    
    //flag à 0
    PIR1bits.SSPIF = 0;
    
    //Load Data + wait for the flag
    SSPBUF = data;
    flag += waitFlag();
    
    //flag à 0
    PIR1bits.SSPIF = 0;
    
    //Stop Condition + Wait for the flag
    SSPCON2bits.PEN = 1;
    flag += waitFlag();
    
    return flag;
    
}

char setRTC(char address ,char data){
    
    char flag = 0;
    
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1; 
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPBUF = 0b11010000;
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

char readRTC (char address){
    
    char data = 0;
    char flag = 0;
    
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1; 
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPBUF = 0b11010000;
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPBUF = address;
    flag = waitFlag();
    PIR1bits.SSPIF = 0;
    SSPCON2bits.RSEN = 1;
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPBUF = 0b11010001;
    flag += waitFlag();
    PIR1bits.SSPIF = 0;
    SSPCON2bits.RCEN = 1;
    flag += waitFlag();
    data = SSPBUF;
    PIR1bits.SSPIF = 0;
    SSPCON2bits.PEN = 1;
    flag += waitFlag();
    
    if (flag == 0)return data;
    else return flag;
    
}
// Verification for the good reception of the flag
char waitFlag(){
    char cpt = 0;   
    while(!PIR1bits.SSPIF){
        cpt++;
        if (cpt == 4999)return -1;
    }
    return 0;
}


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
// test de modification
#include <xc.h>
#define _XTAL_FREQ 16000000   // Fréquence de la clock 16MHz
#define Led4 PORTBbits.RB4       // les Led 4 et 5 sont utiliséent pour des tests
#define Led5 PORTBbits.RB5

// Déclaration de fonction ( envoyer, recevoir, envoyer sur les 7 segments, attendre la remise à zéro du flag)
void sendDataUART(char data);  
char getDataUART();
char sendData7Seg(char address, char data);
char waitFlag();

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
    CMCON  = 7;         // disable comparator
    CVRCON = 0;         
    ADCON1 = 6;         //disable analog
    TRISA = 0b00000011;
    TRISB = 0b00000000;    
    // Déclaration du tableau pour les 7 segments
    int pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
    // Déclaration du tableau pour choisir les 7 segments
    char Seg[4] = {0b01000010,0b01000110,0b01000000,0b01001110};
    // Compteur 
    char cp = 0;
    char cp2 = 1;
    // trame de base
    char trame[10];
    char start = 0;
    // tableau de valeur à envoyer au radar
    char data[5] = {0b10101010,0b01010101,0b00000000,0b00011000,0b01000001};
    Led5 = 0;
    Led4 = 0;
    
    while(cp < 5){
    sendDataUART(data[cp]);
    cp++;
    __delay_ms(100);
    }
    
    // Boucle infinie
    while(1){
       sendData7Seg(Seg[2], pos_segment[0]);
       start = getDataUART(); //Il plante ICI le 2�me passage
       Led4 = 1;
       Led5 = 0;
       if(start == 0xAA){
           trame[0] = start;
           //sendData7Seg(Seg[0], pos_segment[trame[0]>>4]);
           //sendData7Seg(Seg[1], pos_segment[trame[0]& 0b00001111]);
           while(cp2 < 10){
               Led4 = 0;
               Led5 = 1;
               trame[cp2] = getDataUART();   
               sendData7Seg(Seg[3], pos_segment[cp2]);
               if (cp2 == 4 && trame[4] > 0x02){
                   
                 sendData7Seg(Seg[0], pos_segment[trame[4]/16]); //dizaine
                 sendData7Seg(Seg[1], pos_segment[trame[4]%16]); //unit�
               }
               Led5 = 0;
               cp2++;
           }
          cp2 = 1;
           
           
           /*
           Led5 = 1; //DEBUG
           __delay_ms(100);//DEBUG
           trame[0] = start;
           while(cp2 < 10){
               trame[cp2] = getDataUART();
               if (cp2 == 4){
                   sendData7Seg(Seg[1], pos_segment[trame[cp2]]);
               }
               cp2++;
           }
           
            * */
       }
       else{
           sendData7Seg(Seg[2], pos_segment[1]);
       }
       Led5 = 0;
    }    
    return;
}

// Fonction pour envoyer des données au radar
void sendDataUART(char data){
    
    PIR1bits.TXIF = 0;
    TXSTAbits.TXEN = 1;
    TXREG = data;
    while(!PIR1bits.TXIF);
}
// Fonction pour recevoir des données du radar
char getDataUART(){
    char data;
    
    RCSTAbits.CREN = 1;    
    while(!PIR1bits.RCIF);
    if(OERR){
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        OERR = 0;
    }
    data = RCREG;
    PIR1bits.RCIF = 0;
    
    return data;
}
// fontion pour envoyer des données vers les 7 segments
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
// Fonction pour attendre le flag
char waitFlag(){
    char cpt = 0;   
    while(!PIR1bits.SSPIF){
        cpt++;
        if (cpt == 4999)return -1;
    }
    return 0;
}


/*
 TRAME DE CONFIG
 * 
 * 1) 10101010
 * 2) 01010101
 * 3) 00000000
 * 4) 00001010  = 10 
 * 5) 01000000  = 64
 *
 */
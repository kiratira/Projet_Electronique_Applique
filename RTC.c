/*
 * File:   RTC.c
 * Author: Miki
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
#define _XTAL_FREQ 10000000


// DECLARATION PROTOTYPES FONCTIONS
void affichage(unsigned char adresse, unsigned char data);
void RTC_Send(char adresse, char regis ,char data);
char RTC_Recieve(char adresse, char regis);
char Init_RTC (char secondes, char minutes, char heures, char jour, char mois, char annee);
void Read_RTC(char Tab[6]);
char get_unit(char data);
char get_dizaine(char data);


void main(void)
{
    //char cligno = 0;
    CMCON = 7;              //Desactivation des comparateurs
    CVRCON = 0;
    ADCON1 = 6;             //Desactivation des entree analogiques
    TRISA = 0b00000111;
    TRISB = 0b00000000;
    PORTB = 255;

    SSPSTAT = 0b10000000;
    SSPCON = 0b00101000;
    SSPADD = 39;
    SSPCON2 = 0;

    TRISC = 255;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 15;

    // DEFINITION AFFICHAGE 7SEGMENTS POSITIONS + ADRESSES
    char pos_segment[16]=
    {
        0b01000100,
        0b11110101,
        0b10001100,
        0b10100100,
        0b00110101,
        0b00100110,
        0b00000110,
        0b11110100,
        0b00000100,
        0b00100100,
        0b00010100,
        0b00000111,
        0b01001110,
        0b10000101,
        0b00001110,
        0b00011110
    };

    char adresses[4] =
    {
        0b01000010,
        0b01000110,
        0b01000000,
        0b01001110
    };

    //AFFICHAGE DE BASE SUR LES 7SEGMENTS
    affichage(adresses[0], pos_segment[0]);
    affichage(adresses[1], pos_segment[0]);
    affichage(adresses[2], pos_segment[0]);
    affichage(adresses[3], pos_segment[0]);

    //INITIALISATION DU TEMPS 0 SUR LA RTC_
    Init_RTC(0, 0, 0, 0, 0, 0);

    // INITIALISATION TABLEAU POUR TRANSFERT ET STOCKAGE DES DONNEES 
    char Tab[6]= {0, 0, 0, 0, 0, 0};
    
    //BOUCLE INFINIE POUR LECTURE ET AFFICHAGE
    while(1)
    {
        Read_RTC(Tab);

        affichage(adresses[3], pos_segment[get_unit(Tab[5])]);
        affichage(adresses[2], pos_segment[get_dizaine(Tab[5])]);

        affichage(adresses[1], pos_segment[get_unit(Tab[4])]);
        affichage(adresses[0], pos_segment[get_dizaine(Tab[4])]);
        PORTBbits.RB4 = !PORTBbits.RB4;     // LED clignottage

        __delay_ms(500);
    }
   return;
}

void affichage(unsigned char adresse, unsigned char data)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1;        //START
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = adresse;           //Adress sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = data;              //Data sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPCON2bits.PEN = 1;        //Stop
    while(!PIR1bits.SSPIF){}
}

void RTC_Send(char adresse, char regis, char data)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1;        //START
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = adresse;           //Adress sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = regis;             //Registre sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = data;              //Data sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPCON2bits.PEN = 1;        //Stop
    while(!PIR1bits.SSPIF){}
}

char RTC_Recieve(char adresse, char regis)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1;        //START
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = (adresse & 0b11111110);           //Adress write sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = regis;           //Registre sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPCON2bits.RSEN = 1;       //Restart
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPBUF = ((adresse & 0b11111110) + 1);           //Adress read sending
    while(!PIR1bits.SSPIF){}

    PIR1bits.SSPIF = 0;
    SSPCON2bits.RCEN = 1;          //Allow incoming data
    while(!PIR1bits.SSPIF){}

    char data = SSPBUF;

    PIR1bits.SSPIF = 0;
    SSPCON2bits.PEN = 1;        //Stop
    while(!PIR1bits.SSPIF){}
    
    return data;
}

// FONCTION DE VERIFICATION ET INITIALISATION RTC
char Init_RTC (char secondes, char minutes, char heures, char jour, char mois, char annee){
    //VERIFICATION DES VALEURS ENVOYEES
    if(secondes>=60)
    {
        secondes= secondes%60;
    }

    if(minutes>=60)
    {
        minutes= minutes%60;
    }

    if(heures>=24)
    {
        heures= heures%24;
    }

    if(jour>=31)
    {
        jour= jour%31;
    }

    if(mois>=12)
    {
        mois= mois%12;
    }

    if(annee >= 100){
        annee = annee%100;
    }

    // INITIALISATION DES VALEURS CORRIGEES ET ENVOI A LA RTC
    RTC_Send(0b11010000, 0, secondes);
    RTC_Send(0b11010000, 1, minutes);
    RTC_Send(0b11010000, 2, heures);
    RTC_Send(0b11010000, 4, jour);
    RTC_Send(0b11010000, 5, mois);
    RTC_Send(0b11010000, 6, annee);

}

//FONTION DE LECTURE DE LA RTC
void Read_RTC(char Tab[6]){
    //LECTURE DANS LES REGISTRES
    Tab[0] = RTC_Recieve(0b11010000, 6);
    Tab[1] = RTC_Recieve(0b11010000, 5);
    Tab[2] = RTC_Recieve(0b11010000, 4);
    Tab[3] = RTC_Recieve(0b11010000, 2);
    Tab[4] = RTC_Recieve(0b11010000, 1);
    Tab[5] = RTC_Recieve(0b11010000, 0);
    
}

//FONCTIONS DE DECOUPE DES UNITES/DIZAINES
char get_unit(char data)
{//DECOUPE UNITES 
    return data & 0b00001111;
}

char get_dizaine(char data)
{//DECOUPES DIZAINES 
    return (data >> 4 );
}

// SI TU LIS CA T ES UN BG <3

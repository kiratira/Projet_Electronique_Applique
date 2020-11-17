/*
 * File:   EEPROM.c
 * Author: capal
 *
 * Created on 29 octobre 2020, 8:33
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
#define _XTAL_FREQ 20000000

//------------------------------ FONCTION ECRITURE EEPROM ------------------------------------------------------------------------------------------------------------------------------------------

// Ecris une donnee a l'adresse x de l'EEPROM 
void Ecriture_EEPROM(unsigned char adresse, unsigned char data);

// Ecris un enregistrement du radar et du la RTC  dans l'EEPROM (utilise des donnée individuelle)
void Stockage_EEPROM(
        unsigned char annee,
        unsigned char mois,
        unsigned char jour,
        unsigned char heure,
        unsigned char minute,
        unsigned char seconde,
        unsigned char partie_entiere_survitesse,
        unsigned char partie_decimal_survitesse
);

// Ecris un enregistrement du radar dans l'EEPROM (utilise un tableau contenant les 8 donnees necessaire)
void Stockage_TAB_EEPROM(unsigned char TAB[8],unsigned char NB_elem);

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Lis la donnee qui se trouve a l'adresse x de l'EEPROM
unsigned char Lecture_E(unsigned char adresse);
// Lis et Transmets dans le [Tab_transition] de l'enregistrement N
void Lecture_EEPROM(unsigned char N, unsigned char Tab_transition [8]);
// retourne le nombre d'enregistrements 
unsigned char Size_EEPROM();

//------------------------------ FONCTION DE SUPPRESSION EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Clean tout l'EEPROM
void Supp_ALL_EEPROM();
// on lit toute l'EEPROM pour voir si des data on ete supprime
void Lecture_donnees_supp();
// supprime l'enregistrement p et place mets dans le Tab_Adresse_supp[10] l'emplacement de la données supprimee
void Supp_data_EEPROM(unsigned char p);

//------------------------------ DECLARATION VARIABLE EN RAM -------------------------------------------------------------------------------------------------------------------------------------------

unsigned char Tab_data_transition[8] ;
unsigned char Tab_Adresse_supp[10];
unsigned char N = 0; //nombre d'enregistrement
unsigned char suppression = 0; 
unsigned char x = 0; //nombre de valeur supprimer

//------------------------------ DECLARATION / FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------

void affichage(unsigned char adresse, unsigned char data); //affichage 7 segments

unsigned char pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
//(0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F)
unsigned char adresse_segment[4] = {0b01000000,0b01000110,0b01001110,0b01000010};
//(SEG1 , SEG2, SEG3 , SEG4)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void main(void) 
{
    // Config de l'EEPROM
    // bit 7 EEPGD : 1 = Accesses program memory
    // bit 6-4 : Unimplemented  Read as '0'
    // bit 3 WRERR : 0 =  The write operation completed
    // bit 2 WREN : 1 = Allows write cycles
    // bit 1 WR : 0 =  Write cycle to the EEPROM is complete
    // bit 0 RD : 0 =  Does not initiate an EEPROM read
    EECON1 = 0b1000100;
    
    //7 segments et I²C
    
    SSPSTAT = 0b10000000; // Initialisation des SSPS
    SSPCON = 0b00101000;
    SSPADD = 49;
    SSPCON2 = 0;
    
    //Initialisation
    
    N = Lecture_E(1); // lire dans l'EEPROM La valeur a l'adresse 1 qui correspond a notre nombre d'enregistrement
    suppression = Lecture_E(2); // lire l'EERPOM si la il y a eu une suppresion
    Lecture_donnees_supp(); // on lit toute l'EEPROM pour voir si des data on ete supprime
    
    //tableau de test 
    unsigned char tableau[8] = {
        pos_segment[0],
        pos_segment[1],
        pos_segment[2],
        pos_segment[3],
        pos_segment[4],
        pos_segment[5],
        pos_segment[6],
        pos_segment[7]
    };
    
    // ON COMMENCE LA BOUCLE
    while(1)
    {        
        ;
    }
    
    return ;
}



//------------------------------ FONCTION ECRITURE EEPROM ------------------------------------------------------------------------------------------------------------------------------------------


void Ecriture_EEPROM(unsigned char adresse, unsigned char data)
{
    EECON1bits.WR = 1;
    EEADR = adresse;
    EEDATA = data ;
    EECON1bits.EEPGD = 0;
    EECON1bits.WREN = 1;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    EECON1bits.WREN = 0;
    while(EECON1bits.WR);
}

void Stockage_EEPROM(unsigned char annee ,unsigned char mois , unsigned char jour  , unsigned char heure, unsigned char minute,unsigned char seconde,  unsigned char partie_entiere_survitesse, unsigned char partie_decimal_survitesse)
{
    if(suppression==1) // si faut remplacer une data effacer
    {
        if(Tab_Adresse_supp[x] != 0xff)
        {
            Ecriture_EEPROM(10+x*8, annee);
            Ecriture_EEPROM(11+x*8, mois);
            Ecriture_EEPROM(12+x*8, jour);
            Ecriture_EEPROM(13+x*8, heure);
            Ecriture_EEPROM(14+x*8, minute);
            Ecriture_EEPROM(15+x*8, seconde);
            Ecriture_EEPROM(16+x*8, partie_entiere_survitesse);
            Ecriture_EEPROM(17+x*8, partie_decimal_survitesse);
            Tab_Adresse_supp[x] = 0xff; //remplace la valeur du tableau a la position de la derniÃ¨re suppression par 0xff
            x-- ; //dÃ©increment la pos dans le tableau
        }
        
    }
    else
    {
            Ecriture_EEPROM(10+N*8, annee);
            Ecriture_EEPROM(11+N*8, mois);
            Ecriture_EEPROM(12+N*8, jour);
            Ecriture_EEPROM(13+N*8, heure);
            Ecriture_EEPROM(14+N*8, minute);
            Ecriture_EEPROM(15+N*8, seconde);
            Ecriture_EEPROM(16+N*8, partie_entiere_survitesse);
            Ecriture_EEPROM(17+N*8, partie_decimal_survitesse);

       
         
    }
     // incremente le Nombre d'enregistrement
    N++;
    Ecriture_EEPROM(1,N);
}

void Stockage_TAB_EEPROM(unsigned char TAB[8],unsigned char NB_elem) // TAB[8] = {int annee ,int mois , int jour  , int heure, int minute,int seconde,  int partie_entiere_survitesse, int partie_decimal_survitesse};
{
    if(suppression==1) // si faut remplacer une data effacer
    {
        if(Tab_Adresse_supp[x] != 0xff)
        {
            for(unsigned char i = 0; i < NB_elem; i++)
            {
                Ecriture_EEPROM(10+i+x*8 , TAB[i]);
            }
        
            Tab_Adresse_supp[x] = 0xff; //remplace la valeur du tableau a la position de la derniÃ¨re suppression par 0xff
            x-- ; //decremente la pos dans le tableau
        
        }
    }
    else
    {//Ecris les donnees dans l'EEPROM suivant le tableau d'adressage
        for(unsigned char i =0 ; i<NB_elem ; i++)
        {
            Ecriture_EEPROM(10+i+N*8 , TAB[i]);
        }
    }// incremente le Nombre d'enregistrement
    N++;
    Ecriture_EEPROM(1,N); 
}

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

unsigned char Lecture_E(unsigned char adresse)
{
    EEADR = adresse;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1; 
    return EEDATA; 
}

void Lecture_EEPROM(unsigned char N, unsigned char Tab_transition [8]) // envoie les donnee dans le tableau de transition de donnees
{
    for(unsigned char i = 0; i < 8; i++)
    {
        Tab_transition [i] = Lecture_E(10 + i + N*8);
    }
}

unsigned char Size_EEPROM() //nombre d'enregistrement effectuer
{  
    unsigned char i = 0;
     i = Lecture_E(1);
    return i;
}

//------------------------------ FONCTION DE SUPPRESSION EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

void Supp_ALL_EEPROM() //Clear l'eeprom 
{
   for(unsigned char i=0;i<254;i++)
   {
        Ecriture_EEPROM(10+i*8,0);
        Ecriture_EEPROM(11+i*8,0);
        Ecriture_EEPROM(12+i*8,0);
        Ecriture_EEPROM(13+i*8,0);
        Ecriture_EEPROM(14+i*8,0);
        Ecriture_EEPROM(15+i*8,0);
        Ecriture_EEPROM(16+i*8,0);
        Ecriture_EEPROM(17+i*8,0);
        
   }
   Ecriture_EEPROM(1,0);
   N = 0;
}

void Supp_data_EEPROM(unsigned char p)
{
    // ici faut verifier si le tableau d'adresse supp est complet ?
    if(x<10)
    {
        Tab_Adresse_supp[x] = p; //rentre la valeur dans le tableau d'adresse supp
        x++;
    
        for(unsigned char i = 0 ; i<8 ; i++)
        {

            Ecriture_EEPROM(10+i+p*8 , 0xff);  // remplacement des donnees par 0xff
            suppression = 1;
            if(N>0)
            N--;
        }
        affichage(adresse_segment[1],pos_segment[10]); ////ici affiche A si le 1er 7 segments si suppresion effectuer
    }
    else
    {
        //retour d'erreur
        affichage(adresse_segment[1],pos_segment[16]); //ici affiche F sur le 1er 7 segments si tableau plein
    }
}

void Lecture_donnees_supp()
{
    if(suppression == 1)
    {
        for(unsigned char i = 0 ;i < 254 ; i++)
        {
            if(Lecture_E(i) == 0xff )
            {
                Tab_Adresse_supp[x] = (i-10)/8; // passage par 10 + n*8 = i
                i = i + 7; 
                x++;// on saute les adresses comprise ,int mois , int jour  , int heure, int minute,int seconde,  int partie_entiere_survitesse, int partie_decimal_survitesse
            }
        }
        affichage(adresse_segment[2],pos_segment[x]); //affiche sur le 2eme segment le nombre de donnÃ©es supprimer
    }
    else
    {
      affichage(adresse_segment[2],pos_segment[0]);  
    }
}

//------------------------------ FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------

void affichage(unsigned char adresse, unsigned char data)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;    
    SSPBUF = adresse;    
    while(!PIR1bits.SSPIF);    
    PIR1bits.SSPIF = 0;  
    SSPBUF = data;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;  
    SSPCON2bits.PEN = 1;
    while(!PIR1bits.SSPIF);
}

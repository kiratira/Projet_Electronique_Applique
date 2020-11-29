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
#define MAX_DELETED_DATA 16
#define MAX_DATA 30

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
void Stockage_TAB_EEPROM(unsigned char TAB[8]);

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Lis la donnee qui se trouve a l'adresse x de l'EEPROM
unsigned char Lecture_E(unsigned char adresse);
//Lis la valeur du (case 0 de l'EEPROM)
unsigned char Lecture_DataCount();
//Va lire si il y eu suppression de données (case 1 de L'EEPROM)
unsigned char Lecture_Suppression();
// Lis et Transmets dans le [Tab_transition] de l'enregistrement N
void Lecture_EEPROM(unsigned char N, unsigned char Tab_transition [8]);
// retourne le nombre d'enregistrements 
unsigned char NBR_DATA_EEPROM();

//------------------------------ FONCTION DE SUPPRESSION EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Clean tout l'EEPROM
void Supp_ALL_EEPROM();
// on lit toute l'EEPROM pour voir si des data on ete supprime
void Analyser_Deleted_Data();
// supprime l'enregistrement p et place mets dans le Tab_Adresse_supp[10] l'emplacement de la données supprimee
void Supp_data_EEPROM(unsigned char p);

//------------------------------ DECLARATION VARIABLE EN RAM -------------------------------------------------------------------------------------------------------------------------------------------

unsigned char Tab_data_transition[8] ;
unsigned char Tab_Adresse_supp[MAX_DELETED_DATA];
unsigned char DataCount = 0; //nombre d'enregistrement
static __bit suppression = 0; 
unsigned char DeletedData = 0; //nombre de valeur supprimer

//------------------------------ DECLARATION / FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------

void affichage(unsigned char adresse, unsigned char data); //affichage 7 segments

unsigned char pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
//(0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F)
unsigned char adresse_segment[4] = {0b01000000,0b01000110,0b01001110,0b01000010};
//(SEG1 , SEG2, SEG3 , SEG4)
const int pos_segmentproteus[16]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111,0b01110111,0b01111100,0b00111001,0b01011110,0b01111001,0b01110001};
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void main(void) 
{
    //truc de merder
    
    CMCON = 7;              //Desactivation des comparateurs
    CVRCON = 0;
    ADCON1 = 6;             //Desactivation des entree analogiques
    TRISA = 0b00000111;
    TRISB = 0b00000000;
    PORTB = 255;
    
    TRISC = 255;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 25;
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

    unsigned char tab[8]={48,48,48,48,48,48,48,48}; //ASCII {1,0,0,0,0,0,0,0}
    
    //Initialisation
   
    DataCount = Lecture_DataCount(); // lire dans l'EEPROM La valeur a l'adresse 1 qui correspond a notre nombre d'enregistrement
    suppression = Lecture_Suppression(); // lire l'EERPOM si la il y a eu une suppresion
    Analyser_Deleted_Data(); // on lit toute l'EEPROM pour voir si des data on ete supprime
    PORTBbits.RB4 = !PORTBbits.RB4;
   
    // ON COMMENCE LA BOUCLE
    while(1)
    {     
        
        if(PORTAbits.RA0 == 0)
        {
           Supp_data_EEPROM(4);
           Supp_data_EEPROM(2);
           while(PORTAbits.RA0 == 0);
        } 
        if(PORTAbits.RA1 == 0)
        {
            for(int i = 0 ; i<2 ; i++)
            {
               Stockage_TAB_EEPROM(tab);
               if(tab[1] == 57) //si  == 9
               {
                   tab[0]++;
                   tab[1]=48;
               }
               else
               tab[1]++; 
            }
           
           while(PORTAbits.RA1 == 0);
        } 
        
        if(PORTAbits.RA2 == 0)
        {
           Supp_ALL_EEPROM();
           while(PORTAbits.RA2 == 0);
        }
        affichage(adresse_segment[4],pos_segmentproteus[DataCount]);
        PORTBbits.RB4 = !PORTBbits.RB4;
        __delay_ms(200);
    }
    
    return ;
}



//------------------------------ FONCTION ECRITURE EEPROM ------------------------------------------------------------------------------------------------------------------------------------------


void Ecriture_EEPROM(unsigned char adresse, unsigned char data)
{ 
    EECON1bits.WREN=1; // allow EEPROM writes
    EEADR=adresse; // load address of write to EEPROM
    EEDATA=data; // load data to write to EEPROM
    EECON1bits.EEPGD=0;// access EEPROM data memory    
    INTCONbits.GIE=0; // disable interrupts for critical EEPROM write sequence
    //===============//
    EECON2=0x55;
    EECON2=0xAA;
    EECON1bits.WR=1;
    //==============//
    INTCONbits.GIE=1; // enable interrupts, critical sequence complete
    while (EECON1bits.WR==1); // wait for write to complete
    EECON1bits.WREN=0; // do not allow EEPROM writes
}

void Stockage_EEPROM(unsigned char annee ,unsigned char mois , unsigned char jour  , unsigned char heure, unsigned char minute,unsigned char seconde,  unsigned char partie_entiere_survitesse, unsigned char partie_decimal_survitesse)
{  
    if(DataCount <= MAX_DATA)  
    {
        if(suppression==1) // si faut remplacer une data effacer
        {
            if(Tab_Adresse_supp[DeletedData] != 0xff)
            {
                Ecriture_EEPROM(16+Tab_Adresse_supp[DeletedData]*8, annee);
                Ecriture_EEPROM(17+Tab_Adresse_supp[DeletedData]*8, mois);
                Ecriture_EEPROM(18+Tab_Adresse_supp[DeletedData]*8, jour);
                Ecriture_EEPROM(19+Tab_Adresse_supp[DeletedData]*8, heure);
                Ecriture_EEPROM(20+Tab_Adresse_supp[DeletedData]*8, minute);
                Ecriture_EEPROM(21+Tab_Adresse_supp[DeletedData]*8, seconde);
                Ecriture_EEPROM(22+Tab_Adresse_supp[DeletedData]*8, partie_entiere_survitesse);
                Ecriture_EEPROM(23+Tab_Adresse_supp[DeletedData]*8, partie_decimal_survitesse);
                Tab_Adresse_supp[DeletedData-1] = 0xff; //remplace la valeur du tableau a la position de la derniÃ¨re suppression par 0xff           
                DeletedData-- ; //decremente la pos dans le tableau

                if(DeletedData <= 0) //rénitialisation du suppression
                {
                    suppression = 0;
                    Ecriture_EEPROM(1,0);
                }
            }

        }
        else
        {
                Ecriture_EEPROM(16+DataCount*8, annee);
                Ecriture_EEPROM(17+DataCount*8, mois);
                Ecriture_EEPROM(18+DataCount*8, jour);
                Ecriture_EEPROM(19+DataCount*8, heure);
                Ecriture_EEPROM(20+DataCount*8, minute);
                Ecriture_EEPROM(21+DataCount*8, seconde);
                Ecriture_EEPROM(22+DataCount*8, partie_entiere_survitesse);
                Ecriture_EEPROM(23+DataCount*8, partie_decimal_survitesse);



        }
         // incremente le Nombre d'enregistrement
        DataCount++;
        Ecriture_EEPROM(0,DataCount);
    }
    else
    {
        //retourne une erreur comme quoi MAX_DATA est atteinte
    }

}

void Stockage_TAB_EEPROM(unsigned char TAB[8]) // TAB[8] = {int annee ,int mois , int jour  , int heure, int minute,int seconde,  int partie_entiere_survitesse, int partie_decimal_survitesse};
{
    if(DataCount < MAX_DATA)  
    {
        if(suppression == 1) // si faut remplacer une data effacer
        {
            if(Tab_Adresse_supp[DeletedData-1] != 0xff)
            {
                for(unsigned char i = 0; i < 8; i++)
                {
                    Ecriture_EEPROM(16+i+Tab_Adresse_supp[DeletedData-1]*8 , TAB[i]);
                }

                Tab_Adresse_supp[DeletedData-1] = 0xff; //remplace la valeur du tableau a la position de la derniÃ¨re suppression par 0xff           
                DeletedData-- ; //decremente la pos dans le tableau

                if(DeletedData <= 0) //rénitialisation du suppression
                {
                    suppression = 0;
                    Ecriture_EEPROM(1,0);
                }

            }
        }
        else
        {//Ecris les donnees dans l'EEPROM suivant le tableau d'adressage
            for(unsigned char i =0 ; i<8; i++)
            {
                Ecriture_EEPROM(16+i+DataCount*8 , TAB[i]);
            }
        }// incremente le Nombre d'enregistrement
        DataCount++;
        Ecriture_EEPROM(0,DataCount); 
    }
    else
    {
        
        //retourne une erreur comme quoi MAX_DATA est atteinte
        affichage(adresse_segment[1],pos_segmentproteus[16]);
    }
}

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

unsigned char Lecture_E(unsigned char adresse)
{
    EEADR = adresse;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1; 
    return EEDATA; 
}
unsigned char Lecture_DataCount()
{
    if(Lecture_E(0) == 0xff || Lecture_E(0) > 30 || Lecture_E(0) < 0)
    {
        return 0;        
    }
    else
    {
        return Lecture_E(0);
    }
}
unsigned char Lecture_Suppression()
{
    return Lecture_E(1);
}
void Lecture_EEPROM(unsigned char p, unsigned char Tab_transition [8]) // envoie les donnee dans le tableau de transition de donnees
{
    for(unsigned char i = 0; i < 8; i++)
    {
        Tab_transition [i] = Lecture_E(16 + i + p*8);
    }
}

unsigned char NBR_DATA_EEPROM() //nombre d'enregistrement effectuer
{  
    unsigned char i = 0;
     i = Lecture_E(0);
    return i;
}

//------------------------------ FONCTION DE SUPPRESSION EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

void Supp_ALL_EEPROM() //Clear l'eeprom 
{
   for(unsigned char i = 0 ; i < 30 ; i++)
   {
        Ecriture_EEPROM(16+i*8,0xFF);
        Ecriture_EEPROM(17+i*8,0xFF);
        Ecriture_EEPROM(18+i*8,0xFF);
        Ecriture_EEPROM(19+i*8,0xFF);
        Ecriture_EEPROM(20+i*8,0xFF);
        Ecriture_EEPROM(21+i*8,0xFF);
        Ecriture_EEPROM(22+i*8,0xFF);
        Ecriture_EEPROM(23+i*8,0xFF);
        
   }
   Ecriture_EEPROM(0,0b00000000);
   Ecriture_EEPROM(1,0b00000000);
   DataCount = 0;
}

void Supp_data_EEPROM(unsigned char DataIndex)
{
    // ici faut verifier si le tableau d'adresse supp est complet ?
    if(DeletedData >= 0 && DeletedData < MAX_DELETED_DATA)
    {
        Tab_Adresse_supp[DeletedData] = DataIndex; //rentre la valeur dans le tableau d'adresse supp
        DeletedData++;
        
        suppression = 1;
        Ecriture_EEPROM(1,1);
        
        for(unsigned char i = 0 ; i<8 ; i++)
        {
            Ecriture_EEPROM(16+i+DataIndex*8 , 0xff);  // remplacement des donnees par 0xff
        }
        
        //affichage(adresse_segment[1],pos_segment[10]); ////ici affiche A si le 1er 7 segments si suppresion effectuer
        if(DataCount>0)
        {
            DataCount--;
            Ecriture_EEPROM(0,DataCount);
        }
    }
    else if(DeletedData < MAX_DELETED_DATA) //ERREUR DE RANGE
    {
        ;
    }
    else
    {
        //retour d'erreur
        affichage(adresse_segment[1],pos_segment[16]); //ici affiche F sur le 1er 7 segments si tableau plein
    }
}

void Analyser_Deleted_Data()
{
    int dataCount = 0;
    
    if(suppression == 1)
    {
        for(unsigned char i = 1 ;i < 30 ; i++)
        {
            if(dataCount >= NBR_DATA_EEPROM()) //si le nombre d'enregistrement détecter par l'analyser est égal ou dépasse le nombre d'enregistrement
            {
              break;  
            }          
            if(Lecture_E(16+8*i) == 0xff )
            {
                
                Tab_Adresse_supp[DeletedData] = i; // passage par 16 + n*8 = i                
                DeletedData++;// on saute les adresses comprise ,int mois , int jour  , int heure, int minute,int seconde,  int partie_entiere_survitesse, int partie_decimal_survitesse
                Ecriture_EEPROM(14,i); //écris a l'adresse 14 la derniére adresses supprimer         
            }
            else
            {
                dataCount++;
            }
        }
        affichage(adresse_segment[2],pos_segmentproteus[dataCount]); //affiche sur le 2eme segment le nombre de donnÃ©es supprimer
    }
    else
    {
      affichage(adresse_segment[3],pos_segmentproteus[0]);  
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

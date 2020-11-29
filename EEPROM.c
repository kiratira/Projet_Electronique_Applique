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
void EEPROM_Ecriture(unsigned char adresse, unsigned char data);

// Ecris un enregistrement du radar et du la RTC  dans l'EEPROM (utilise des donnée individuelle)
void EEPROM_Stockage(
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
void EEPROM_Stockage_TAB(unsigned char TAB[8]);

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Lis la donnee qui se trouve a l'adresse x de l'EEPROM
unsigned char EEPROM_Lecture_data(unsigned char adresse);
//Lis la valeur du (case 0 de l'EEPROM)
unsigned char EEPROM_Lecture_DataCount();
//Va lire si il y eu suppression de données (case 1 de L'EEPROM)
unsigned char EEPROM_Lecture_Flag_Data_Delete();
// Lis et Transmets dans le [Tab_transition] de l'enregistrement N
void EEPROM_Lecture_Enregistrement(unsigned char DataIndex, unsigned char Tab_transition [8]);
// retourne le nombre d'enregistrements 
unsigned char EEPROM_NBR_Enregistrements();

//------------------------------ FONCTION DE SUPPRESSION EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Clean tout l'EEPROM
void EEPROM_Reset_ALL();
// on lit toute l'EEPROM pour voir si des data on ete supprime
void EEPROM_Analyser_Deleted_Data();
// supprime l'enregistrement p et place mets dans le Tab_Adresse_supp[10] l'emplacement de la données supprimee
void EEPROM_Supp_data(unsigned char DataIndex);

//------------------------------ DECLARATION VARIABLE EN RAM -------------------------------------------------------------------------------------------------------------------------------------------

unsigned char Tab_data_transition[8] ; //tableau dans lequel les donnees qui doivent être lu son envoyees
unsigned char Tab_Adresse_supp[MAX_DELETED_DATA];
unsigned char DataCount = 0; //nombre d'enregistrement
static __bit Flag_Data_Delete = 0; //donnee supp oui/non
unsigned char DeletedData = 0; //nombre de valeur supprimer

//------------------------------ DECLARATION / FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_initialization();
void affichage(unsigned char adresse, unsigned char data); //affichage 7 segments

unsigned char pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
//(0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F) pour le pcb reel
unsigned char adresse_segment[4] = {0b01000000,0b01000110,0b01001110,0b01000010};
//(SEG1 , SEG2, SEG3 , SEG4)
unsigned char pos_segmentproteus[16]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111,0b01110111,0b01111100,0b00111001,0b01011110,0b01111001,0b01110001};
//(0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F) pour proteus simulation
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void main(void) 
{
    //boutons
    
    CMCON = 7;              //Desactivation des comparateurs
    CVRCON = 0;
    ADCON1 = 6;             //Desactivation des entrees analogiques
    TRISA = 0b00000111;
    TRISB = 0b00000000;
    PORTB = 255;
    
    TRISC = 255;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 25;
    
    // Config de l'EEPROM
    
    EECON1 = 0b1000100;
    
    //7 segments et I²C
    
    SSPSTAT = 0b10000000; // Initialisation des SSPS
    SSPCON = 0b00101000;
    SSPADD = 49;
    SSPCON2 = 0;

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
    unsigned char tab[8]={48,48,48,48,48,48,48,48}; //ASCII {0,0,0,0,0,0,0,0} près pour le test uart
    
    //Initialisation
   
    EEPROM_initialization();
   
    // ON COMMENCE LA BOUCLE
    while(1)
    {     
        
        if(PORTAbits.RA0 == 0)
        {
           EEPROM_Supp_data(4);
           EEPROM_Supp_data(2);
           while(PORTAbits.RA0 == 0);
        } 
        if(PORTAbits.RA1 == 0)
        {
            for(int i = 0 ; i<2 ; i++)
            {
               EEPROM_Stockage_TAB(tab);
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
           EEPROM_Reset_ALL();
           while(PORTAbits.RA2 == 0);
        }
        affichage(adresse_segment[4],pos_segmentproteus[DataCount]);
        PORTBbits.RB4 = !PORTBbits.RB4;
        __delay_ms(200);
    }
    
    return ;
}



//------------------------------ FONCTION ECRITURE EEPROM ------------------------------------------------------------------------------------------------------------------------------------------


void EEPROM_Ecriture(unsigned char adresse, unsigned char data)
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

void EEPROM_Stockage(unsigned char annee ,unsigned char mois , unsigned char jour  , unsigned char heure, unsigned char minute,unsigned char seconde,  unsigned char partie_entiere_survitesse, unsigned char partie_decimal_survitesse)
{  
    if(DataCount <= MAX_DATA)  //Check si on depasse pas la limite max d'enregistement possible
    {
        if(Flag_Data_Delete==1) // Check si une data a été effacer
        {
            if(Tab_Adresse_supp[DeletedData] != 0xff) //check si la valeur comprise dans le tableau est =! de 0xff
            {
                //Ecrit les valeurs
                EEPROM_Ecriture(16+Tab_Adresse_supp[DeletedData]*8, annee); //-1 car incrémentation faites aprés Voir fct Supp_data_EEPROM(unsigned char DataIndex)        
                EEPROM_Ecriture(17+Tab_Adresse_supp[DeletedData]*8, mois);
                EEPROM_Ecriture(18+Tab_Adresse_supp[DeletedData]*8, jour);
                EEPROM_Ecriture(19+Tab_Adresse_supp[DeletedData]*8, heure);
                EEPROM_Ecriture(20+Tab_Adresse_supp[DeletedData]*8, minute);
                EEPROM_Ecriture(21+Tab_Adresse_supp[DeletedData]*8, seconde);
                EEPROM_Ecriture(22+Tab_Adresse_supp[DeletedData]*8, partie_entiere_survitesse);
                EEPROM_Ecriture(23+Tab_Adresse_supp[DeletedData]*8, partie_decimal_survitesse);
                             
                Tab_Adresse_supp[DeletedData-1] = 0xff; //remplace la valeur du tableau a la position de la derniere suppression par 0xff           
                DeletedData-- ; //decremente le nombre de data supprimer la pos dans le tableau 

                if(DeletedData <= 0) //check si le nombre de data supprimer est <= a 0
                {
                    //rénitialisation du suppression
                    Flag_Data_Delete = 0; //set bit supp a 0
                    EEPROM_Ecriture(1,0); // l'écrit de l'EEPROM
                }
                 
            }

        }
        else 
        {
            //Ecrit les valeurs
                EEPROM_Ecriture(16+DataCount*8, annee);
                EEPROM_Ecriture(17+DataCount*8, mois);
                EEPROM_Ecriture(18+DataCount*8, jour);
                EEPROM_Ecriture(19+DataCount*8, heure);
                EEPROM_Ecriture(20+DataCount*8, minute);
                EEPROM_Ecriture(21+DataCount*8, seconde);
                EEPROM_Ecriture(22+DataCount*8, partie_entiere_survitesse);
                EEPROM_Ecriture(23+DataCount*8, partie_decimal_survitesse);

            

        }
       // incremente le Nombre d'enregistrement
            DataCount++;
            EEPROM_Ecriture(0,DataCount);  
    }
    else
    {
        //retourne une erreur comme quoi MAX_DATA est atteinte
    }

}

void EEPROM_Stockage_TAB(unsigned char TAB[8]) // TAB[8] = {int annee ,int mois , int jour  , int heure, int minute,int seconde,  int partie_entiere_survitesse, int partie_decimal_survitesse};
{
    if(DataCount <= MAX_DATA)  //Check si on depasse pas la limite max d'enregistement possible
    {
        if(Flag_Data_Delete==1) // Check si une data a été effacer
        {
            if(Tab_Adresse_supp[DeletedData-1] != 0xff) //check si la valeur comprise dans le tableau est =! de 0xff
            {
                //Ecrit les données suivant le tableau d'adressage
                for(unsigned char i = 0; i < 8; i++)
                {
                    EEPROM_Ecriture(16+i+Tab_Adresse_supp[DeletedData-1]*8 , TAB[i]); //-1 car incrémentation faites aprés Voir fct Supp_data_EEPROM(unsigned char DataIndex)
                }

                Tab_Adresse_supp[DeletedData-1] = 0xff; //remplace la valeur du tableau a la position de la derniere suppression par 0xff           
                DeletedData-- ; //decremente le nombre de data supprimer la pos dans le tableau 

                if(DeletedData <= 0) //check si le nombre de data supprimer est <= a 0
                {
                    //rénitialisation du suppression
                    Flag_Data_Delete = 0; //set bit supp a 0
                    EEPROM_Ecriture(1,0); // l'écrit de l'EEPROM
                }
               
            }
             
        }
        else
        {
            //Ecris les donnees dans l'EEPROM suivant le tableau d'adressage
            for(unsigned char i =0 ; i<8; i++)
            {
                EEPROM_Ecriture(16+i+DataCount*8 , TAB[i]);
            }
            
             
        }
        // incremente le Nombre d'enregistrement
        DataCount++;
        EEPROM_Ecriture(0,DataCount); 
    }
    else
    {
        
        //retourne une erreur comme quoi MAX_DATA est atteinte
        affichage(adresse_segment[1],pos_segmentproteus[16]);
    }
}

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

unsigned char EEPROM_Lecture_data(unsigned char adresse)
{
    EEADR = adresse;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1; 
    return EEDATA; 
}
unsigned char EEPROM_Lecture_DataCount()
{
    if(EEPROM_NBR_Enregistrements() == 0xff || EEPROM_NBR_Enregistrements() > 30 || EEPROM_NBR_Enregistrements() < 0) //Check pour DATACOUNT si EEPROM VIDE ou  null
    {
        EEPROM_Ecriture(0,0);
        return 0;    //DataCount = 0    
    }
    else
    {
        return EEPROM_NBR_Enregistrements();//DataCount = Nbre enregistrement fait au dernier allumage (addresse 0 dans l'EEPROM)
    }
}
unsigned char EEPROM_Lecture_Flag_Data_Delete()
{
    if(EEPROM_Lecture_data(1) != 0 && EEPROM_Lecture_data(1) != 1)
    {
        EEPROM_Ecriture(1,0);
        return 0;
    }
    else
    return EEPROM_Lecture_data(1); //suppression = (addresse 1 dans l'EEPROM)
}
void EEPROM_Lecture_Enregistrement(unsigned char DataIndex, unsigned char Tab_transition [8]) // envoie les donnee dans le tableau de transition de donnees
{
    
    for(unsigned char i = 0; i < 8; i++)
    {
        Tab_transition [i] = EEPROM_Lecture_data(16 + i + DataIndex*8);
    }
}

unsigned char EEPROM_NBR_Enregistrements() //nombre d'enregistrement effectuer
{      
    return EEPROM_Lecture_data(0);
}

//------------------------------ FONCTION DE SUPPRESSION EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

void EEPROM_Reset_ALL() //Clear l'eeprom 
{
   for(unsigned char i = 0 ; i < 30 ; i++)
   {
        PORTBbits.RB5 = !PORTBbits.RB5;
        EEPROM_Ecriture(16+i*8,0xFF);
        EEPROM_Ecriture(17+i*8,0xFF);
        EEPROM_Ecriture(18+i*8,0xFF);
        EEPROM_Ecriture(19+i*8,0xFF);
        EEPROM_Ecriture(20+i*8,0xFF);
        EEPROM_Ecriture(21+i*8,0xFF);
        EEPROM_Ecriture(22+i*8,0xFF);
        EEPROM_Ecriture(23+i*8,0xFF);
        
   }
   EEPROM_Ecriture(0,0b00000000);//reset nbr enregistrement
   DataCount = 0;
   EEPROM_Ecriture(1,0b00000000);//reset flag de suppression
   Flag_Data_Delete = 0;
   EEPROM_Ecriture(2,0b00000000);//reset adresse de la dernière suppression
   
   //reset du tab de transition
   for(int i = 0; i<8;i++)
   Tab_data_transition[i]= 0xff;
     
   
   //reset du tableau d'adresse supprimer
   for(int i = 0; i<MAX_DELETED_DATA;i++)
   Tab_Adresse_supp[i]= 0xff;

   DeletedData = 0; //reset du nombre de valeur supprimer
   
}

void EEPROM_Supp_data(unsigned char DataIndex)
{
    if(EEPROM_Lecture_data(16+DataIndex*8) != 0xff)//check si l'élement est diff de 0xff pour pas effacer 2 fois
    {
        if(DeletedData >= 0 && DeletedData < MAX_DELETED_DATA)//check si on peut supprimer des données
        {
            Tab_Adresse_supp[DeletedData] = DataIndex; //rentre l'adresse du la donnee supprimee (DataIndex)dans le tableau d'adresse supp
            DeletedData++;//increment le nbre de données qui ont été supprime

            //set le flag de suppression a true
            Flag_Data_Delete = 1; 
            EEPROM_Ecriture(1,1);

            // Effacement des donnees en les remplacant par 0xff
            for(unsigned char i = 0 ; i<8 ; i++)
            {
                EEPROM_Ecriture(16+i+DataIndex*8 , 0xff);  
            }
            EEPROM_Ecriture(2,DataIndex); //écris a l'adresse 2 la derniére adresses supprimer 

            if(DataCount>0)//check si on a des donnees enregistrees
            {
                //désincremente le nbre de donnees enregistree et l'ecrit
                DataCount--; 
                EEPROM_Ecriture(0,DataCount);
            }
        }
        else if(DeletedData < MAX_DELETED_DATA) //ERREUR DE RANGE : on peut pas supprimer de données tableau complet
        {
            ;
        }
    }
    else
    {
        //retour d'erreur
        affichage(adresse_segment[1],pos_segment[9]); //ici affiche F sur le 1er 7 segments si tableau plein
    }
}

void EEPROM_Analyser_Deleted_Data()
{
    int dataCount = 0; //conteneur de donnees lue
    
    if(EEPROM_Lecture_Flag_Data_Delete()) // si il y a au minimum une donnee supprimee
    {
        for(unsigned char i = 0 ;i < 30 ; i++)
        {
            PORTBbits.RB5 = !PORTBbits.RB5;           
            if(dataCount >= EEPROM_Lecture_DataCount()) //si le nombre d'enregistrement détecter par l'analyser est égal ou dépasse le nombre d'enregistrement
            {
              break;  
            }          
            if(EEPROM_Lecture_data(16+8*i) == 0xff ) //si la lecture a l'adresse 16+8*i est effacee
            {
                
                Tab_Adresse_supp[DeletedData] = i; //rentre l'adresse du la donnee supprimee de la ou on se trouve (i)dans le tableau d'adresse supp             
                DeletedData++;//incremente le nombre de donnees supprimer
                EEPROM_Ecriture(2,i); //écris a l'adresse 2 la derniére adresses supprimer         
            }
            else
            {
                dataCount++; //increment le conteneur de donnees lue
            }
        }
        affichage(adresse_segment[2],pos_segmentproteus[dataCount]); //affiche sur le 2eme segment le nombre de donnÃ©es supprimer
    }
    else
    {
      //si pas de data supprimer flag a 0
      affichage(adresse_segment[3],pos_segmentproteus[0]);  
    }
}

//------------------------------ FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_initialization()
{
    DataCount = EEPROM_Lecture_DataCount(); // lire dans l'EEPROM La valeur a l'adresse 1 qui correspond a notre nombre d'enregistrement
    Flag_Data_Delete = EEPROM_Lecture_Flag_Data_Delete(); // lire l'EERPOM si la il y a eu une suppresion
    EEPROM_Analyser_Deleted_Data(); // on lit toute l'EEPROM pour voir si des data on ete supprime   
}
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

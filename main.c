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

// EEPROM
#define EEPROM_MAX_DELETED_DATA 16
#define EEPROM_MAX_DATA 30
#define EEPROM_DATA_SPACE 8
//////// PROTOTYPE & VARIABLES GLOBALES
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
void EEPROM_Stockage_TAB(unsigned char TAB[EEPROM_DATA_SPACE]);

//------------------------------ FONCTION LECTURE EEPROM -------------------------------------------------------------------------------------------------------------------------------------------

// Lis la donnee qui se trouve a l'adresse x de l'EEPROM
unsigned char EEPROM_Lecture_data(unsigned char adresse);
//Lis la valeur du (case 0 de l'EEPROM)
unsigned char EEPROM_Lecture_DataCount();
//Va lire si il y eu suppression de données (case 1 de L'EEPROM)
unsigned char EEPROM_Lecture_Flag_Data_Delete();
//Lire la N éme(DataIndex)donnees des 10 dernières enregistrement et le place dans un tab
static __bit EEPORM_Lecture_10Last(unsigned char DataIndex, unsigned char Tab_transition [EEPROM_DATA_SPACE]);
// Lis et Transmets dans le [Tab_transition] de l'enregistrement N
void EEPROM_Lecture_Enregistrement(unsigned char DataIndex, unsigned char Tab_transition [EEPROM_DATA_SPACE]);
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

unsigned char Tab_data_transition[EEPROM_DATA_SPACE] ; //tableau dans lequel les donnees qui doivent être lu son envoyees
unsigned char Tab_Adresse_supp[EEPROM_MAX_DELETED_DATA];
unsigned char SaveDataCount =0;
unsigned char DataCount = 0; //nombre d'enregistrement
static __bit Flag_Data_Delete = 0; //donnee supp oui/non
unsigned char DeletedData = 0; //nombre de valeur supprimer

//------------------------------ DECLARATION / FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_initialization();
void EEPROM_Last_SaveData();
void affichage(unsigned char adresse, unsigned char data); //affichage 7 segments

unsigned char pos_segment[16]={0b01000100,0b11110101,0b10001100,0b10100100,0b00110101,0b00100110,0b00000110,0b11110100,0b00000100,0b00100100,0b00010100,0b00000111,0b01001110,0b10000101,0b00001110, 0b00011110};
//(0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F) pour le pcb reel
unsigned char adresse_segment[4] = {0b01000000,0b01000110,0b01001110,0b01000010};
//(SEG1 , SEG2, SEG3 , SEG4)
unsigned char pos_segmentproteus[16]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111,0b01110111,0b01111100,0b00111001,0b01011110,0b01111001,0b01110001};
//(0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F) pour proteus simulation

//////////////////////////////////////////////////////////////////////////////// RADAR
// Déclaration de fonction ( envoyer, recevoir, envoyer sur les 7 segments, attendre la remise à zéro du flag)
void sendDataUART(char data);  
char getDataUART();
char sendData7Seg(char address, char data);
char waitFlag();
void scanVitesse();
void setSurvitesse(char vitesse);
char tabValueForEEPROM(char tab);

 //variable Global pour radar
char survitesse = 2;
char runRadar = 1;
char uniteSurvitesse = 0;
char decSurvitesse = 0;

//////////////////////////////////////////////////////////////////////////////// RTC
void affichage(unsigned char adresse, unsigned char data);  
void RTC_Send(char adresse, char regis ,char data);  
char RTC_Recieve(char adresse, char regis);  
char RTC_Init (char secondes, char minutes, char heures, char jour, char mois, char annee);
char RTC_Init_tab (char tab[6]);
void RTC_Read(char Tab[6]);  
char RTC_get_unit(char data);  
char RTC_get_dizaine(char data);  
void RTC_comp_Start(); 
void RTC_comp_Stop(); 
void RTC_comp_adress_sending(char adresse); 
void RTC_comp_registre_sending(char regis); 
void RTC_comp_data_sending(char data); 
char transfoDCB(char data);

void init_all(){
    // LCD
    i2c_init();
    LCD_init();
    // EEPROM
    EEPROM_initialization();
    // RTC
    char a[6] = {0,0,0,0,0,0};
    RTC_Init_tab(a);
    // RADAR
    setSurvitesse(SURVITESSE);
}

void main(void) {
    //I/O
    CMCON = 7;              //Desactivation des comparateurs
    CVRCON = 0;
    ADCON1 = 6;             //Desactivation des entrees analogiques
    TRISA = 0b00000111;
    TRISB = 0b00000000;
    PORTB = 255;
    
    //I2C
    SSPSTAT = 0b10000000;
    SSPCON = 0b00101000;
    SSPCON2 = 0;
    SSPADD = 39; // TODO CHANGER POUR LA FRÉQUENCE DU QUARTZ
    
    //UART
    TRISC = 255;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 25; // TODO CHANGER
    
    // Config de l'EEPROM
    EECON1 = 0b1000100;
    
    //Radar
    // Déclaration du tableau pour les 7 segments
    int pos_segment[16] = {
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
    // Déclaration du tableau pour choisir les 7 segments
    char Seg[4] = {
        0b01000010,
        0b01000110,
        0b01000000,
        0b01001110
    };
    // tableau de valeur à envoyer au radar
    char data[5] = {
        0b10101010,
        0b01010101,
        0b00000000,
        0b00011000,
        0b01000001
    };
    
    init_all();
    
    unsigned char vitesse = 0;
    unsigned char tab_EEPROM[8]; // Format : AA   MM   JJ   HH   SS   Entier Vitesse   Decimale Vitesse
    while(1){
        if(PORTAbits.RA0 == 0){}
        if(PORTAbits.RA1 == 0){}
        if(PORTAbits.RA2 == 0){}

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
// EEPROM
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
    if(DataCount <= EEPROM_MAX_DATA)  //Check si on depasse pas la limite max d'enregistement possible
    {
        if(Flag_Data_Delete==1) // Check si une data a été effacer
        {
            if(Tab_Adresse_supp[DeletedData] != 0xff) //check si la valeur comprise dans le tableau est =! de 0xff
            {
                //Ecrit les valeurs
                EEPROM_Ecriture(16+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, annee); //-1 car incrémentation faites aprés Voir fct Supp_data_EEPROM(unsigned char DataIndex)        
                EEPROM_Ecriture(17+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, mois);
                EEPROM_Ecriture(18+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, jour);
                EEPROM_Ecriture(19+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, heure);
                EEPROM_Ecriture(20+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, minute);
                EEPROM_Ecriture(21+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, seconde);
                EEPROM_Ecriture(22+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, partie_entiere_survitesse);
                EEPROM_Ecriture(23+Tab_Adresse_supp[DeletedData]*EEPROM_DATA_SPACE, partie_decimal_survitesse);
                             
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
                EEPROM_Ecriture(16+DataCount*EEPROM_DATA_SPACE, annee);
                EEPROM_Ecriture(17+DataCount*EEPROM_DATA_SPACE, mois);
                EEPROM_Ecriture(18+DataCount*EEPROM_DATA_SPACE, jour);
                EEPROM_Ecriture(19+DataCount*EEPROM_DATA_SPACE, heure);
                EEPROM_Ecriture(20+DataCount*EEPROM_DATA_SPACE, minute);
                EEPROM_Ecriture(21+DataCount*EEPROM_DATA_SPACE, seconde);
                EEPROM_Ecriture(22+DataCount*EEPROM_DATA_SPACE, partie_entiere_survitesse);
                EEPROM_Ecriture(23+DataCount*EEPROM_DATA_SPACE, partie_decimal_survitesse);

            

        }
        
        //enregistre dans les 10 derniers
        EEPROM_Last_SaveData();
        // incremente le Nombre d'enregistrement
        DataCount++;
        EEPROM_Ecriture(0,DataCount);  
    }
    else
    {
        //retourne une erreur comme quoi MAX_DATA est atteinte
    }

}

void EEPROM_Stockage_TAB(unsigned char TAB[EEPROM_DATA_SPACE]) // TAB[8] = {int annee ,int mois , int jour  , int heure, int minute,int seconde,  int partie_entiere_survitesse, int partie_decimal_survitesse};
{
    if(DataCount < EEPROM_MAX_DATA)  //Check si on depasse pas la limite max d'enregistement possible
    {
        //enregistre dans les 10 derniers
        EEPROM_Last_SaveData();
        if(Flag_Data_Delete==1) // Check si une data a été effacer
        {
            if(Tab_Adresse_supp[DeletedData-1] != 0xff) //check si la valeur comprise dans le tableau est =! de 0xff
            {
                //Ecrit les données suivant le tableau d'adressage
                for(unsigned char i = 0; i < EEPROM_DATA_SPACE; i++)
                {
                    EEPROM_Ecriture(16+i+Tab_Adresse_supp[DeletedData-1]*EEPROM_DATA_SPACE , TAB[i]); //-1 car incrémentation faites aprés Voir fct Supp_data_EEPROM(unsigned char DataIndex)
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
            for(unsigned char i =0 ; i<EEPROM_DATA_SPACE; i++)
            {
                EEPROM_Ecriture(16+i+DataCount*EEPROM_DATA_SPACE , TAB[i]);
            }    
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
static __bit EEPORM_Lecture_10Last(unsigned char DataIndex, unsigned char Tab_transition [EEPROM_DATA_SPACE])
{   
    // /!\ pas TESTER 
    unsigned char Data = EEPROM_Lecture_data(6+DataIndex);
    if(DataIndex <0 && DataIndex >10 && Data != 0xff)
    {
        for(unsigned char i = 0; i < EEPROM_DATA_SPACE; i++)
        {
            Tab_transition [i] = EEPROM_Lecture_data(16 + i + Data *EEPROM_DATA_SPACE);
        }
        return 1;
    }
    else
        return 0;
}
void EEPROM_Lecture_Enregistrement(unsigned char DataIndex, unsigned char Tab_transition [EEPROM_DATA_SPACE]) // envoie les donnee dans le tableau de transition de donnees
{   
    for(unsigned char i = 0; i < EEPROM_DATA_SPACE; i++)
    {
        Tab_transition [i] = EEPROM_Lecture_data(16 + i + DataIndex*EEPROM_DATA_SPACE);
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
        EEPROM_Ecriture(16+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(17+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(18+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(19+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(20+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(21+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(22+i*EEPROM_DATA_SPACE,0xFF);
        EEPROM_Ecriture(23+i*EEPROM_DATA_SPACE,0xFF);
        
   }
   EEPROM_Ecriture(0,0b00000000);//reset nbr enregistrement
   DataCount = 0;
   EEPROM_Ecriture(1,0b00000000);//reset flag de suppression
   Flag_Data_Delete = 0;
   EEPROM_Ecriture(2,0b00000000);//reset adresse de la dernière suppression
   
   EEPROM_Ecriture(3,0b00000000);//reset SaveDataCount
   SaveDataCount =0 ;
   //reset des enregistrements
   for(int i = 0 ; i<10 ; i ++)
    EEPROM_Ecriture(6+i,0xff);
   //reset du tab de transition
   for(int i = 0; i<8;i++)
   Tab_data_transition[i]= 0xff;
     
   
   //reset du tableau d'adresse supprimer
   for(int i = 0; i<EEPROM_MAX_DELETED_DATA;i++)
   Tab_Adresse_supp[i]= 0xff;

   DeletedData = 0; //reset du nombre de valeur supprimer
   
}

void EEPROM_Supp_data(unsigned char DataIndex)
{
    if(EEPROM_Lecture_data(16+DataIndex*EEPROM_DATA_SPACE) != 0xff)//check si l'élement est diff de 0xff pour pas effacer 2 fois
    {
        if(DeletedData >= 0 && DeletedData < EEPROM_MAX_DELETED_DATA)//check si on peut supprimer des données
        {
            
                
            Tab_Adresse_supp[DeletedData] = DataIndex; //rentre l'adresse du la donnee supprimee (DataIndex)dans le tableau d'adresse supp
            DeletedData++;//increment le nbre de données qui ont été supprime

            //set le flag de suppression a true
            Flag_Data_Delete = 1; 
            EEPROM_Ecriture(1,1);

            // Effacement des donnees en les remplacant par 0xff
            for(unsigned char i = 0 ; i<EEPROM_DATA_SPACE ; i++)
            {
                EEPROM_Ecriture(16+i+DataIndex*EEPROM_DATA_SPACE , 0xff);  
            }
            EEPROM_Ecriture(2,DataIndex); //écris a l'adresse 2 la derniére adresses supprimer 

            if(DataCount>0)//check si on a des donnees enregistrees
            {
                //désincremente le nbre de donnees enregistree et l'ecrit
                DataCount--; 
                EEPROM_Ecriture(0,DataCount);
            }
            //Supprimer si DataIndex dans les 10 derniers enregistrements
            for(int i =0 ; i<10;i++) //parcour les 10 derniers enregistrements pour vérifier si le dataindex est dedans
            {
                if(DataIndex == EEPROM_Lecture_data(6+i)) //si l'enregistrement est dans les 10 derniers enregistrements le supprime (remplace par 0xff)
                {
                    for(int j = i;j<10;j++) //on decalle tout
                    {
                        EEPROM_Ecriture(6+j,EEPROM_Lecture_data(7+j));
                    }                    
                    SaveDataCount --;
                    EEPROM_Ecriture(3,SaveDataCount);
                   // EEPROM_Last_SaveData();
                }
            }
        }
        else if(DeletedData < EEPROM_MAX_DELETED_DATA) //ERREUR DE RANGE : on peut pas supprimer de données tableau complet
        {
            ;
        }
    }
    else
    {
        //retour d'erreur
        ;
    }
}

void EEPROM_Analyser_Deleted_Data()
{
    //verification du nombre de donnees sauvees 
    if(EEPROM_Lecture_data(3)> 0 && EEPROM_Lecture_data(3)<=10) 
    {
       SaveDataCount = EEPROM_Lecture_data(3); //reprend la ou c'était arreté 0 à 10
    }    
    else
        SaveDataCount = 0; // recommence à 0
        EEPROM_Ecriture(3,SaveDataCount); 
        
    //partie suppression
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
            if(EEPROM_Lecture_data(16+EEPROM_DATA_SPACE*i) == 0xff ) //si la lecture a l'adresse 16+8*i est effacee
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
        
    }
    else
    {
      //si pas de data supprimer flag a 0
        ;  
    }
    
}

//------------------------------ FONCTION AUTRE  -------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_initialization()
{
    DataCount = EEPROM_Lecture_DataCount(); // lire dans l'EEPROM La valeur a l'adresse 1 qui correspond a notre nombre d'enregistrement
    Flag_Data_Delete = EEPROM_Lecture_Flag_Data_Delete(); // lire l'EERPOM si la il y a eu une suppresion
    EEPROM_Analyser_Deleted_Data(); // on lit toute l'EEPROM pour voir si des data on ete supprime   
}
void EEPROM_Last_SaveData()
{
    if(SaveDataCount < 10) //si le nombre de donnees savees n'est pas a son max
    {   
        if(Flag_Data_Delete) //si il y a eu une suppression de donnee
            EEPROM_Ecriture(6+SaveDataCount, Tab_Adresse_supp[DeletedData-1]); //ecrit dans les donnees sauvees la position des nouvelles donnees
        else
            EEPROM_Ecriture(6+SaveDataCount, DataCount); //ecrit a partir de l'adresse 6 les data save
        
        //incremente SaveDataCount
        SaveDataCount ++;
        EEPROM_Ecriture(3,SaveDataCount);         
    }
    else
    {
        //decalle les la pile de donnees sauvees
        for(int i = 0 ; i<9 ; i ++)
        {
            EEPROM_Ecriture(6+i,EEPROM_Lecture_data(7+i)); //ecrit a la position 6+i la valeur ecrite a la position 7+i
            PORTBbits.RB5 = !PORTBbits.RB5;
        }
        //Ecrit a la derniere position des dataSave la position de la nouvelle data
        if(Flag_Data_Delete)
            EEPROM_Ecriture(15, Tab_Adresse_supp[DeletedData-1]); //cas : si une donnees a ete supp
        else            
            EEPROM_Ecriture(15, DataCount); //ecrit a partir de l'adresse 6 les data save                
    }
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

//////////////////////////////////////////////////////////////////////////////// RADAR
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
//fonction de reception des trames du radar
void scanVitesse(){

    char trash = 0;
    char cp2 = 1;
    char checkSurvitesse = 0;

    while(runRadar){
        trash = getDataUART();
        if(trash == 0xAA){
            while(cp2 < 10){
                trash = getDataUART();   
                if (cp2 == 4 && trash > survitesse){  
                    checkSurvitesse = 1;
                    uniteSurvitesse = trash;
               }
                if (cp2 == 5 && checkSurvitesse){  
                    decSurvitesse = trash;
               }
               cp2++;
           }
          cp2 = 1;         
       }
       if (checkSurvitesse == 1)break;
    }
}
//fonction pour attribuer la valeur de la survitesse
void setSurvitesse(char vitesse){
    survitesse = vitesse;
}
//FONCTION D'ECRITURE DU TABLEAU
char tabValueForEEPROM(char tab){
    //Read_RTC(tab);
    tab[6] = uniteSurvitesse;
    tab[7] = decSurvitesse;
    return tab;
}

//////////////////////////////////////////////////////////////////////////////// RTC

void affichage(unsigned char adresse, unsigned char data)  
{  
     RTC_comp_Start(); 
     RTC_comp_adress_sending(adresse); 
     RTC_comp_data_sending(data); 
     RTC_comp_Stop(); 
} 
  //SEND DATA
void RTC_Send(char adresse, char regis, char data)  
{  
    RTC_comp_Start(); 
    RTC_comp_adress_sending(adresse); 
    RTC_comp_registre_sending(regis); 
    RTC_comp_data_sending(data); 
    RTC_comp_Stop(); 
}  
  //GET DATA
char RTC_Recieve(char adresse, char regis)  
{  
    RTC_comp_Start(); 
    RTC_comp_adress_sending(adresse & 0b11111110); 
    RTC_comp_registre_sending(regis); 
  
    PIR1bits.SSPIF = 0;  
    SSPCON2bits.RSEN = 1;       //Restart  
    while(!PIR1bits.SSPIF){}  
  
    RTC_comp_adress_sending((adresse & 0b11111110) + 1); 
  
    PIR1bits.SSPIF = 0;  
    SSPCON2bits.RCEN = 1;          //Allow incoming data  
    while(!PIR1bits.SSPIF){}  
  
    char data = SSPBUF;  
  
    RTC_comp_Stop(); 
 
      
    return data;  
}  
  
// FONCTION DE VERIFICATION ET INITIALISATION RTC  
char RTC_Init (char annee, char mois, char jour, char heures, char minutes, char secondes){  
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
    RTC_Send(0b11010000, 0, transfoDCB(secondes));  
    RTC_Send(0b11010000, 1, transfoDCB(minutes));  
    RTC_Send(0b11010000, 2, transfoDCB(heures));  
    RTC_Send(0b11010000, 4, transfoDCB(jour));  
    RTC_Send(0b11010000, 5, transfoDCB(mois));  
    RTC_Send(0b11010000, 6, transfoDCB(annee));  
  
}  
char RTC_Init_tab (char tab[6]){  
    //VERIFICATION DES VALEURS ENVOYEES  
    if(tab[5]>=60)  
    {  
        tab[5]= tab[5]%60;  
    }  
  
    if(tab[4]>=60)  
    {  
        tab[4] = tab[4]%60;  
    }  
  
    if(tab[3]>=24)  
    {  
        tab[3] = tab[3]%24;  
    }  
  
    if(tab[2]>=31)  
    {  
        tab[2] = tab[2]%31;  
    }  
  
    if(tab[1]>=12)  
    {  
        tab[1]= tab[1]%12;  
    }  
  
    if(tab[0] >= 100){  
        tab[0] = tab[0]%100; 
    }  
  
    // INITIALISATION DES VALEURS CORRIGEES ET ENVOI A LA RTC  
    RTC_Send(0b11010000, 0, transfoDCB(tab[5]));  
    RTC_Send(0b11010000, 1, transfoDCB(tab[4]));  
    RTC_Send(0b11010000, 2, transfoDCB(tab[3]));  
    RTC_Send(0b11010000, 4, transfoDCB(tab[2]));  
    RTC_Send(0b11010000, 5, transfoDCB(tab[1]));  
    RTC_Send(0b11010000, 6, transfoDCB(tab[0]));  
  
}  
   
//FONTION DE LECTURE DE LA RTC  
void RTC_Read(char Tab[6]){  
    //LECTURE DANS LES REGISTRES  
    Tab[0] = RTC_Recieve(0b11010000, 6);  
    Tab[1] = RTC_Recieve(0b11010000, 5);  
    Tab[2] = RTC_Recieve(0b11010000, 4);  
    Tab[3] = RTC_Recieve(0b11010000, 2);  
    Tab[4] = RTC_Recieve(0b11010000, 1);  
    Tab[5] = RTC_Recieve(0b11010000, 0);  
      
}  
  
//FONCTIONS DE DECOUPE DES UNITES/DIZAINES  
char RTC_get_unit(char data)  
{//DECOUPE UNITES   
    return data & 0b00001111;  
}  
  
char RTC_get_dizaine(char data)  
{//DECOUPES DIZAINES   
    return (data >> 4 );  
}  
  
// SI TU LIS CA T ES UN BG <3  
 
void RTC_comp_Start(){ 
    PIR1bits.SSPIF = 0;  
    SSPCON2bits.SEN = 1;         
    while(!PIR1bits.SSPIF){}  
} 
void RTC_comp_Stop(){ 
    PIR1bits.SSPIF = 0;  
    SSPCON2bits.PEN = 1;        //Stop  
    while(!PIR1bits.SSPIF){}  
} 
void RTC_comp_adress_sending(char adresse){ 
    PIR1bits.SSPIF = 0;  
    SSPBUF = adresse;           //Adress sending  
    while(!PIR1bits.SSPIF){}  
} 
void RTC_comp_registre_sending(char regis) 
{ 
    PIR1bits.SSPIF = 0;  
    SSPBUF = regis;             //Registre sending  
    while(!PIR1bits.SSPIF){} 
} 
void RTC_comp_data_sending(char data) 
{ 
     PIR1bits.SSPIF = 0;  
     SSPBUF = data;              //Data sending  
     while(!PIR1bits.SSPIF){}  
}
char transfoDCB(char data){
    char a, b, f =0;
    a = data%10;
    b = (data - a)/10;
    f = f + b;
    f = f<<4;
    f = f + a;
    return f;
}

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

// LCD
#define LCD_ADRESSE 0b01111000
#define CONTROLBYTE_CMD 0b00000000
#define CONTROLBYTE_DATA 0b01000000

#define ZERO 0b00110000 //provient de la datasheet du lcd pour le caractère zéro

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
void tabValueForEEPROM(char tab[]);

 //variable Global pour radar
char survitesse = 2;
char runRadar = 1;
char uniteSurvitesse = 0;
char decSurvitesse = 0;

//////////////////////////////////////////////////////////////////////////////// RTC
void RTC_affichage(unsigned char adresse, unsigned char data);
void RTC_Send(char adresse, char regis ,char data);  
char RTC_Recieve(char adresse, char regis);  
char RTC_Init (char secondes, char minutes, char heures, char jour, char mois, char annee);
char RTC_Init_tab (char tab[]);
void RTC_Read(char Tab[]);  
char RTC_get_unit(char data);  
char RTC_get_dizaine(char data);  
void RTC_comp_Start(); 
void RTC_comp_Stop(); 
void RTC_comp_adress_sending(char adresse); 
void RTC_comp_registre_sending(char regis); 
void RTC_comp_data_sending(char data); 
char transfoDCB(char data);

//////////////////////////////////////////////////////////////////////////////// LCD
void i2c_init();
void i2c_bus_scan();
void i2c_Wait();
void i2c_Start();
void i2c_Restart();
void i2c_Stop();
int i2c_Write(int data);

void LCD_send_cmd(unsigned char data);
void LCD_init();
void LCD_send_data(unsigned char data);
void LCD_data_composit(unsigned int data, char pos);
void LCD_send_string (char *str);
void LCD_menudisplay(char *var1, char *var2, char *var3, char *var4);
void LCD_setline(char poscursor);
void LCD_setposcursor(char pos);
void LCD_affichradardata(int tab[8], char num);

void init_all();

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
    
    //////////////////////////////////////////////////////////////////////////// LCD
    unsigned int cpt = 0;//compteur de temps
    
    //Variables pour bouger dans les différents menus d'affichage du LCD :
    unsigned char posmainmenu = 0, poseditmenu = 0, posspeedmenu = 0, menuselect = 0, posspeeddisplaymenu = 0;
    
    //variable pour le numéro de l'excès de vitesse qu'on veut afficher depuis le menu d'affichage du LCD :
    int num = 0;
    
    //variables ~ équivalentes à un booléen, mais qui prennent que 1 bit de mémoire. Aussi pour faire fonctionner le menu d'affichage du LCD :
    static __bit confirm = 0, confirmdelete=0, confirmdate =0, confirmtime=0, quitspeed = 0, quitallspeed = 0, eeprom_is_full = 0;
    static __bit setdatetime = 0, displayspeed = 0, displayallspeed = 0, setday = 0, setmon = 0, setyear = 0, setmin = 0, sethour = 0;
    
    //Tableau contenant {jour, mois, année, minute, heure} pour régler et afficher la date et l'heure sur le LCD :
    unsigned int initDateTime[6] = {1, 1, 20, 0, 12, 0};
    
    //Tableau contenant 1 excès de vitesse envoyé à la demande lorsqu'on veut afficher cet excès à l'écran {année, mois, jour, heure, min, sec, entier, déc } :
    int radardata[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    
    //////////////////////////////////////////////////////////////////////////// Radar
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
    
    LCD_menudisplay("------MainMenu------", "Set Date & Time", "Speeding Records", "Date & Time Display");
    posmainmenu = 1;
    LCD_setline(posmainmenu);
    while(1){
        if(setday || setmon || setyear || setmin || sethour) setdatetime=1;
        else setdatetime=0;
        //Main Menu
        if(PORTAbits.RA0 == 0)  //si on appuie sur bouton RA0
        {
            while(PORTAbits.RA0 == 0) cpt++;
            if(cpt>100) confirmdelete =1;
            
            if(setday || setmon || setyear) confirmdate=1;
            if(setmin || sethour) confirmtime=1;
            if(displayspeed) quitspeed=1;
            if(displayallspeed) quitallspeed=1;
            
            cpt=0;
            confirm=1;
        }
        if(PORTAbits.RA1 == 0)  //si on appuie sur bouton RA1
        {
            if(!setdatetime && !displayspeed && !displayallspeed && !eeprom_is_full)
            {
                if(menuselect == 0)
                {
                    posmainmenu++;
                    if(posmainmenu == 4) posmainmenu = 1;
                    LCD_setline(posmainmenu);                
                }
                if(menuselect == 1)
                {
                    poseditmenu++;
                    if(poseditmenu==4) poseditmenu=1;
                    LCD_setline(poseditmenu);                
                }
                if(menuselect == 2)
                {
                    posspeedmenu++;
                    if(posspeedmenu==4) posspeedmenu=1;
                    LCD_setline(posspeedmenu);                
                }
                if(menuselect == 3)
                {
                    posspeeddisplaymenu++;
                    if(posspeeddisplaymenu==4) posspeeddisplaymenu=1;
                    LCD_setline(posspeeddisplaymenu);  
                }
            }
            //Regler date et heure initiales
            if(setdatetime && !eeprom_is_full)
            {
                if(setday && !setmon)
                {
                    initDateTime[0]++;
                    if(initDateTime[0]==32) initDateTime[0]=1;
                }
                if(setmon && !setyear)
                {
                    initDateTime[1]++;
                    if(initDateTime[1]==13) initDateTime[1]=1;
                }
                if(setyear)
                {
                    initDateTime[2]++;
                    if(initDateTime[2]==100) initDateTime[2]=0;
                }
                if(setmin && !sethour)
                {
                    initDateTime[3]++;
                    if(initDateTime[3]==60) initDateTime[3]=0;
                }
                if(sethour)
                {
                    initDateTime[4]++;
                    if(initDateTime[4]==24) initDateTime[4]=0;
                }
                confirm =1;
            }
            if(displayspeed && !eeprom_is_full)
            {
                num++;
                if(num==10) num=0;
                confirm =1;
            }
            if(displayallspeed && !eeprom_is_full)
            {
                num++;
                if(num==30) num=0;
                confirm =1;
            }
            while(PORTAbits.RA1 == 0);
        }
        if(PORTAbits.RA2 == 0)  //Si on appuie sur bouton RA2
        {
            if(setdatetime && !eeprom_is_full)
            {
                //Regler date et heure initiales
                if(setday && !setmon)
                {
                    --initDateTime[0];
                    if(initDateTime[0]==0) initDateTime[0]=31;
                }
                if(setmon && !setyear)
                {
                    --initDateTime[1];
                    if(initDateTime[1]==0) initDateTime[1]=12;
                }
                if(setyear)
                {
                    --initDateTime[2];
                    if(initDateTime[2]==-1) initDateTime[2]=99;
                }
                if(setmin && !sethour)
                {
                    --initDateTime[3];
                    if(initDateTime[3]==-1) initDateTime[3]=59;
                }
                if(sethour)
                {
                    --initDateTime[4];
                    if(initDateTime[4]==-1) initDateTime[4]=23;
                }
                confirm = 1;
            }
            if(displayspeed && !eeprom_is_full)
            {
                --num;
                if(num==-1) num=9;
                confirm =1;
            }
            if(displayallspeed && !eeprom_is_full)
            {
                --num;
                if(num==-1) num=29;
                confirm =1;
            }
            while(PORTAbits.RA2 == 0);
        }
        if(!eeprom_is_full)
        {
            //Menu Principal du LCD        
            if(posmainmenu == 1 && confirm)
            {
                //Menu Secondaire
                menuselect = 1; //on change la variable que le bouton RA1 modifie
                if(poseditmenu == 1)    //on choisit Edit Date --> réglage de la date
                {
                    //menuselect = 2;
                    if(!setday)
                    {
                        setday = 1;
                        LCD_menudisplay("------EditingDate---", "Day", "", "");
                        LCD_setposcursor(0x67);
                        LCD_send_string(">");
                    }
                    if(setday && !setmon)   //on regle le jour
                    {
                        LCD_data_composit(initDateTime[0], 0x1C);
                        if(confirmdate)
                        {
                            confirmdate = 0;
                            setmon=1;   //on confirme le jour pour passer au mois
                            LCD_menudisplay("------EditingDate---", "Month", "", "");
                            LCD_setposcursor(0x67);
                            LCD_send_string(">");
                        }
                    }
                    if(setmon && !setyear)  //on regle le mois
                    {
                        LCD_data_composit(initDateTime[1], 0x1C);
                        if(confirmdate)
                        {
                            confirmdate=0;
                            setyear=1;  //on confirme le mois pour passer à l'année
                            LCD_menudisplay("------EditingDate---", "Year", "", "");
                            LCD_setposcursor(0x67);
                            LCD_send_string(">");
                        }
                    }
                    if(setyear)             //on regle l'année
                    {
                        LCD_data_composit(initDateTime[2], 0x1C);
                        if(confirmdate) //on confirme l'année pour retourner au menu précédent
                        {
                            setday =0;
                            setmon =0;
                            setyear =0;
                            confirmdate =0;
                            poseditmenu = 0;
                            posmainmenu = 1;
                        }
                    }
                }
                else if(poseditmenu == 2)    //on choisit Edit Time --> réglage de l'heure
                {
                    if(!setmin)
                    {
                        setmin = 1;
                        LCD_menudisplay("-----EditingTime----", "Minutes", "", "");
                        LCD_setposcursor(0x67);
                        LCD_send_string(">");
                    }
                    if(setmin && !sethour)   //on regle les minutes
                    {
                        LCD_data_composit(initDateTime[3], 0x1C);
                        if(confirmtime)
                        {
                            confirmtime=0;
                            sethour=1;   //on confirme les minutes pour passer aux heures
                            LCD_menudisplay("-----EditingTime----", "Hours", "", "");
                            LCD_setposcursor(0x67);
                            LCD_send_string(">");
                        }
                    }
                    if(sethour)             //on regle les heures
                    {
                        LCD_data_composit(initDateTime[4], 0x1C);
                        if(confirmtime) //on confirme les heures pour retourner au menu précédent
                        {
                            setmin =0;
                            sethour =0;
                            confirmtime=0;
                            poseditmenu = 0;
                            posmainmenu =1;
                        }
                    }
                }
                else if(poseditmenu == 3) posmainmenu = 0;   //On choisit Back

                if(poseditmenu ==0)
                {
                    LCD_menudisplay("------SousMenu1-----", "Edit Date", "Edit Time", "Back");
                    poseditmenu =1;
                    LCD_setline(poseditmenu);
                }
                //Fin du menu secondaire
            }
            else if(posmainmenu == 2 && confirm) //On choisit Speeding Records
            {
                //Deuxième menu secondaire
                menuselect = 2; //on change la variable que le bouton RA1 modifie
                if(posspeedmenu == 1)
                {
                    menuselect = 3;//on change la variable que le bouton RA1 modifie
                    if(posspeeddisplaymenu == 1) //on choisit "10 last records" pour afficher les 10 derniers excès enregistrés
                    {
                        if(!quitspeed)
                        {
                            displayspeed=1;
                            //EEPROM_lecture10last();
                            LCD_affichradardata(radardata, num);
                        }
                        if(quitspeed)posspeeddisplaymenu=0;                     
                    }
                    else if(posspeeddisplaymenu == 2) //on choisit "Show/Delete Records" pour afficher tous les excès enregistrés et en supprimer
                    {
                        if(!quitallspeed)
                        {
                            displayallspeed=1;
                            LCD_affichradardata(radardata, num);
                            if(confirmdelete ==1)
                            {
                                //delete eeprom;
                            }
                        }
                        if(quitallspeed) posspeeddisplaymenu =0;
                    }
                    else if(posspeeddisplaymenu == 3) posspeedmenu = 0; //Back
                    if(posspeeddisplaymenu == 0)
                    {                        
                        LCD_menudisplay("------SousMenu3-----", "10 last Records", "Show/Delete Records", "Back");
                        posspeeddisplaymenu =1;
                        quitspeed=0;
                        quitallspeed=0;
                        displayspeed=0;
                        displayallspeed=0;
                        num = 0;
                        LCD_setline(posspeeddisplaymenu);
                    }
                }
                else if(posspeedmenu == 2)
                {
                    //Start Record
                    //RADAR_fonctionrecord();
                }
                else if(posspeedmenu == 3) posmainmenu = 0;   //On choisit Back
                if(posspeedmenu ==0)
                {
                    LCD_menudisplay("------SousMenu2-----", "Display Records", "Start Record", "Back");
                    posspeedmenu =1;
                    menuselect = 2;
                    LCD_setline(posspeedmenu);
                }
                //Fin du deuxième menu secondaire
            }
            else if(posmainmenu == 3 && confirm) //On choisit Date & Time Display pour afficher la date et l'heure
            {
                //RTC_modifheuredate( initdatetime[]);
                
                LCD_send_cmd(0b00000001);
                __delay_ms(200);
                LCD_setposcursor(0x00);
                LCD_send_string("Today is the");
                LCD_data_composit(initDateTime[0], 0x40);//jour
                LCD_send_string("/");
                LCD_data_composit(initDateTime[1], 0x44);//mois
                LCD_send_string("/");
                LCD_data_composit(initDateTime[2], 0x48);//heure
                LCD_setposcursor(0x14);;
                LCD_send_string ("and the time is");
                LCD_data_composit(initDateTime[4], 0x54);//heure
                LCD_send_string(":");
                LCD_data_composit(initDateTime[3], 0x58);//minutes
                __delay_ms(5000);
                posmainmenu = 0;
            }
            if(posmainmenu ==0)
            {
                LCD_menudisplay("------MainMenu------", "Edit Date & Time", "Speeding Records", "Display Date & Time");
                poseditmenu = 0;
                posspeedmenu = 0;
                menuselect = 0;
                displayspeed=0;
                //quitspeed=0;
                posmainmenu = 1;
                LCD_setline(posmainmenu);
            }
            //Fin du menu principal
        
        confirm = 0;
        }
        else if (eeprom_is_full)
        {
            LCD_menudisplay("/!\\","ERROR","EEPROM IS FULL","DATA WILL BE DELETED");
            eeprom_is_full = 0;
            while(1);
        }
        // En cas de survitesse crée un enregistrement dans l'EEPROM
        scanVitesse();
        tabValueForEEPROM(tab_EEPROM);
        EEPROM_Stockage_TAB(tab_EEPROM);
    }
    return;
}
// INIT ALL
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
    char sensi = 2;
 
    while(!PORTAbits.RA0){ 
        trash = getDataUART(); 
        if(trash == 0xAA){ 
            while(cp2 < 10){ 
                trash = getDataUART();    
                if (cp2 == 4 && trash > survitesse && (((trash - uniteSurvitesse) > sensi)||(trash - uniteSurvitesse) < -sensi)){   
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
void tabValueForEEPROM(char tab[]){
    RTC_Read(tab);
    tab[6] = uniteSurvitesse;
    tab[7] = decSurvitesse;
}

//////////////////////////////////////////////////////////////////////////////// RTC

void RTC_affichage(unsigned char adresse, unsigned char data)  
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
char RTC_Init_tab (char tab[]){  
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
void RTC_Read(char Tab[]){
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

//////////////////////////////////////////////////////////////////////////////// LCD

//initialisation I²C
void i2c_init(void)
{
    TRISB = 0b00000000; //TRISB = 0; ou TRISB = 0x00;
    PORTB = 0;
    ADCON1 = 0b00000110;   //désactivation des entrées analogiques
    TRISA = 0b00000111;
    TRISC = 255;
    SSPSTAT = 0b10000000;
    SSPCON = 0b00101000;
    SSPADD = 39;
    SSPCON2 = 0;
    TXSTA = 0;
    RCSTA = 0b10000000;
    SPBRG = 25;
}

//Fonction pour scan les différents périphériques esclaves connectés en I²C avec le PIC et ressortir leur adresse
/*void i2c_bus_scan()
{
    unsigned int count = 0; 

    // Try all slave addresses from 0x10 to 0xEF.
    // See if we get a response from any slaves
    // that may be on the i2c bus.
    for(int i=0x10; i < 0xF0; i+=2)
    {
        //start
        i2c_Start();
        //write
        i2c_Wait();
        PIR1bits.SSPIF = 0;
        SSPBUF = i;
        while(!PIR1bits.SSPIF);
        if(SSPCON2bits.ACKSTAT == 0)
        {
            i2c_Stop();
            count++;
            //affichage(adresses[0], pos_segmentproteus[count]);
        }
        else i2c_Stop();
    } 
}*/


// i2c_Wait - wait for I2C transfer to finish
void i2c_Wait(void){
    while ( ( SSPCON2 & 0x1F ) || ( SSPSTAT & 0x04 ) );
}

// i2c_Start - Start I2C communication
void i2c_Start(void)
{
    i2c_Wait();
    SSPCON2bits.SEN=1;
}

// i2c_Restart - Re-Start I2C communication
void i2c_Restart(void){
    i2c_Wait();
    SSPCON2bits.RSEN=1;
}

// i2c_Stop - Stop I2C communication
void i2c_Stop(void)
{
    i2c_Wait();
    SSPCON2bits.PEN=1;
}

// i2c_Write - Sends one byte of data
int i2c_Write(int data)
{    
    i2c_Wait();
    SSPBUF = data;    
}

//sert à envoyer une commande au LCD, comme par exemple un display clear.
void LCD_send_cmd (unsigned char cmd)
{
    i2c_Start();
    i2c_Write(LCD_ADRESSE);
    i2c_Write(CONTROLBYTE_CMD);
    i2c_Write(cmd);
    i2c_Stop();
}

//initialisation du LCD
void LCD_init()
{
    __delay_ms(100);
    LCD_send_cmd(0x38); //function set
    LCD_send_cmd(0x0F); //display on off
    LCD_send_cmd(0b00000001); //display clear
    __delay_ms(100);
    LCD_send_cmd(0b00001111); //permet de set un curseur qui blink à la position où on est
    LCD_send_cmd(0b10000000|0x00);
    LCD_send_string ("-------Hello--------");
    LCD_send_cmd(0b10000000|0x14);
    LCD_send_string ("This is our radar !");
    __delay_ms(3000);
}

//sert à envoyer une donnée à écrire sur l'écran LCD
void LCD_send_data (unsigned char data)
{
    i2c_Start();
    i2c_Write(LCD_ADRESSE);
    i2c_Write(CONTROLBYTE_DATA);
    i2c_Write(data);
    __delay_ms(10);
    i2c_Stop();
}

//sert à afficher un int sur le LCD en le décomposant en ses chiffres. 
//ex : afficher 1234 en affichant 1 puis 2 puis 3 puis 4
void LCD_data_composit(unsigned int data, char pos)
{
    unsigned int data1, data2, data3, data4;
    data1 = data/1000;
    data2 = (data - data1*1000)/100;
    data3 = (data - data1*1000 - data2*100)/10;
    data4 = (data - data1*1000 - data2*100 - data3*10);
    
    LCD_setposcursor(pos); //0x1B
    if(data1) LCD_send_data(ZERO + data1);  //on met la condition if pour ne pas afficher un zéro inutile
    if(data1 || data2) LCD_send_data(ZERO + data2); //on met la 2eme condition if pour ne pas afficher 2 zéros de suite
    LCD_send_data(ZERO + data3);
    LCD_send_data(ZERO + data4);
}

//sert à envoyer un ensemble de char à la fois, qui vont se suivre
void LCD_send_string (char *str)
{
	while (*str) LCD_send_data (*str++);
}

//Cette fonction sert à afficher 4 "string" sur chacune des 4 lignes du LCD, ces strings sont var1 var2 var3 var4
void LCD_menudisplay(char *var1, char *var2, char *var3, char *var4)
{    
    LCD_send_cmd(0b00000001); //clear display
    __delay_ms(200);
    LCD_send_cmd(0b10000000|0x00);  // ligne 1 colonne 1
    LCD_send_string (var1);
    LCD_send_cmd(0b10000000|0x41);  // ligne 2 colonne 2
    LCD_send_string (var2);
    LCD_send_cmd(0b10000000|0x15);  // ligne 3 colonne 2
    LCD_send_string (var3);
    LCD_send_cmd(0b10000000|0x55);  // ligne 4 colonne 2
    LCD_send_string (var4);
}

//ca sert à mettre le curseur clignotant à la 1ere colonne des lignes 2, 3 ou 4 du LCD
void LCD_setline(char poscursor)
{    
    if(poscursor == 1) LCD_send_cmd(0b10000000|0x40);
    if(poscursor == 2) LCD_send_cmd(0b10000000|0x14);
    if(poscursor == 3) LCD_send_cmd(0b10000000|0x54);
}

//sert à mettre le curseur clignotant où on le veut sur le LCD
void LCD_setposcursor(char pos)
{
    LCD_send_cmd(0b10000000|pos);
}

//
void LCD_affichradardata (int tab[8], char num)
{
    LCD_send_cmd(0b00000001);
    __delay_ms(100);
    LCD_setposcursor(0x00);
    LCD_send_string("Speeding record ");
    if(num<9)LCD_send_data(ZERO + 1 + num);
    else 
    {
        LCD_send_data(ZERO + 1);
        LCD_send_data(ZERO);
    }
    LCD_setposcursor(0x41);
    LCD_send_string("Date:");
    LCD_data_composit(tab[2], 0x47);
    LCD_send_string("/");
    LCD_data_composit(tab[1], 0x4A);
    LCD_send_string("/");
    LCD_data_composit(tab[0], 0x4D);
    LCD_setposcursor(0x15);
    LCD_send_string("Time:");
    LCD_data_composit(tab[3], 0x1B);
    LCD_send_string(":");
    LCD_data_composit(tab[4], 0x1E);
    LCD_send_string(":");
    LCD_data_composit(tab[5], 0x21);
    LCD_setposcursor(0x55);
    LCD_send_string("Speed:");
    LCD_data_composit(tab[6], 0x5C);
    LCD_send_string(",");
    LCD_data_composit(tab[7], 0x60);
    LCD_send_string("km/h");
}
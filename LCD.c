
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
#include <stdint.h>

#define _XTAL_FREQ 16000000

//codes du LCD :
#define LCD_ADRESSE 0b01111000
#define CONTROLBYTE_CMD 0b00000000
#define CONTROLBYTE_DATA 0b01000000

void i2c_init();
void i2c_bus_scan();
void i2c_Wait();
void i2c_Start();
void i2c_Restart();
void i2c_Stop();
char i2c_Write(char data);

void lcd_send_cmd(char data);
void LCD_init();
void lcd_send_data(unsigned char data);
void data_composit(unsigned char data, char pos);
void lcd_send_string (char *str);
void menudisplay(char *str1, char *str2, char *str3, char *str4);
void setline(char poscursor);
void setposcursor(char pos);
void affichradardata(unsigned char tab[8], char num);
void Lecture_EEPROM(unsigned char n, unsigned char radardata[8]){}

void main(void){
    //Init i2c:
    i2c_init();
    
    //scan bus i2c pour trouver le nombre d'adresses en i2c
    //i2c_bus_scan();
    LCD_init();
    
    //Variables pour bouger dans les différents menus d'affichage du LCD :
    unsigned char posmainmenu = 0, poseditmenu = 0, posspeedmenu = 0, menuselect = 0;
    
    // TODO Pas compris ce que vous voulez dire
    //Variable pour le numéro de l'excès de vitesse qu'on veut afficher depuis le menu d'affichage du LCD :
    unsigned char num = 0;
    
    // TODO C'est pas équivalent a un booléen, c'est un booléen
    //Variables ~ équivalentes à un booléen, mais qui prennent que 1 bit de mémoire. Aussi pour faire fonctionner le menu d'affichage du LCD :
    static __bit confirm = 0, confirmdate = 0, confirmtime = 0, quitspeed = 0, eeprom_is_full = 0;
    static __bit setdatetime = 0, displayspeed = 0, setday = 0, setmon = 0, setyear = 0, setmin = 0, sethour = 0;

    //Tableau contenant {jour, mois, année, minute, heure} pour régler et afficher la date et l'heure sur le LCD :
    unsigned char initDateTime[5] = {1, 1, 20, 0, 12};
    
    //Tableau contenant 1'excès de vitesse envoyé à la demande lorsqu'on veut afficher cet excès à l'écran {année, mois, jour, heure, min, sec, entier, déc } :
    unsigned char radardata[8] = {20, 11, 13, 12, 17, 45, 136, 222};
    
    //variable pour afficher top 10 des excès de vitesse, il contiendra leurs position dans l'EEPROM :
    unsigned char radardata_pos[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    menudisplay("------MainMenu------", "Set Date & Time", "Speeding Records", "Date & Time Display");
    posmainmenu = 1;
    setline(posmainmenu);
    
    while(1){
        if(setday || setmon || setyear || setmin || sethour) setdatetime = 1;
        else setdatetime = 0;
        //Main Menu
        
        // TODO Compléter le commentaire
        //si on appuie sur bouton RA0
        if(PORTAbits.RA0 == 0){
           confirm = 1;
           while(PORTAbits.RA0 == 0);
           if(setday || setmon || setyear) confirmdate = 1;
           if(setmin || sethour) confirmtime = 1;
           if(displayspeed) quitspeed = 1;
        }
        
        // TODO Compléter le commentaire
        //si on appuie sur bouton RA1
        if(PORTAbits.RA1 == 0){
            if(!setdatetime && !eeprom_is_full){
                if(menuselect == 0 && !displayspeed){
                    posmainmenu++;
                    if(posmainmenu == 4) posmainmenu = 1;
                    setline(posmainmenu);
                }
                if(menuselect == 1 && !displayspeed){
                    poseditmenu++;
                    if(poseditmenu == 4) poseditmenu = 1;
                    setline(poseditmenu);
                }
                if(menuselect == 2){
                    posspeedmenu++;
                    if(posspeedmenu == 4) posspeedmenu = 1;
                    setline(posspeedmenu);
                }
            }
            //Regler date et heure initiales
            if(setdatetime && !eeprom_is_full){
                if(setday){
                    if(++initDateTime[0] == 32) initDateTime[0] = 1;
                }
                if(setmon){
                    if(++initDateTime[1] == 13) initDateTime[1] = 1;
                }
                if(setyear){
                    if(++initDateTime[2] == 100) initDateTime[2] = 00;
                }
                // TODO Pourquoi vérifier sethour ?
                if(setmin && !sethour){
                    if(++initDateTime[3] == 60) initDateTime[3] = 0;
                }
                if(sethour){
                    if(++initDateTime[4] == 24) initDateTime[4] = 0;
                }
                confirm = 1;
            }
            if(displayspeed && !eeprom_is_full){
                if(++num == 10) num = 0;
                confirm = 1;
            }
            while(PORTAbits.RA1 == 0);
        }
        // TODO Compléter le commentaire
        //Si on appuie sur bouton RA2
        if(PORTAbits.RA2 == 0){
            if(setdatetime && !eeprom_is_full){
                //Regler date et heure initiales
                // On vérifie si l'appuie a causé un dépassement de valeur de l'unsigned int (0 - 1 = 255)
                // Si oui on corrige avec une valeur appropriée
                if(setday && !setmon){
                    if(--initDateTime[0] == 255) initDateTime[0] = 31;
                }
                if(setmon && !setyear){
                    if(--initDateTime[1] == 255) initDateTime[1] = 12;
                }
                if(setyear){
                    if(--initDateTime[2] == 255) initDateTime[2] = 99;
                }
                if(setmin && !sethour){
                    if(--initDateTime[3] == 255) initDateTime[3] = 59;
                }
                if(sethour){
                    if(--initDateTime[4] == 255) initDateTime[4] = 23;
                }
                confirm = 1;
            }
            if(displayspeed && !eeprom_is_full){
                if(--num == 255) num = 9;
                confirm = 1;
            }
            while(PORTAbits.RA2 == 0);
        }
        if(!eeprom_is_full){
            //Menu Principal du LCD        
            if(posmainmenu == 1 && confirm){
                //Menu Secondaire
                //on change la variable que le bouton RA1 modifie
                menuselect = 1;
                //on choisit Edit Date --> réglage de la date
                if(poseditmenu == 1){
                    //menuselect = 2;
                    if(!setday){
                        setday = 1;
                        // TODO DRY
                        menudisplay("------EditingDate---", "Day", "", "");
                        setposcursor(0x67);
                        lcd_send_string(">");
                    }
                    //on regle le jour
                    if(setday && !setmon){
                        data_composit(initDateTime[0], 0x1C);
                        if(confirmdate){
                            confirmdate = 0;
                            //on confirme le jour pour passer au mois
                            setmon=1;
                            // TODO DRY
                            menudisplay("------EditingDate---", "Month", "", "");
                            setposcursor(0x67);
                            lcd_send_string(">");
                        }
                    }
                    //on regle le mois
                    if(setmon && !setyear){
                        data_composit(initDateTime[1], 0x1C);
                        if(confirmdate){
                            confirmdate = 0;
                            //on confirme le mois pour passer à l'année
                            setyear = 1;
                            // TODO DRY
                            menudisplay("------EditingDate---", "Year", "", "");
                            setposcursor(0x67);
                            lcd_send_string(">");
                        }
                    }
                    //on regle l'année
                    if(setyear){
                        data_composit(initDateTime[2], 0x1C);
                        //on confirme l'année pour retourner au menu précédent
                        if(confirmdate){
                            setday = 0;
                            setmon = 0;
                            setyear = 0;
                            confirmdate = 0;
                            poseditmenu = 0;
                            posmainmenu = 1;
                        }
                    }
                }else if(poseditmenu == 2){ //on choisit Edit Time --> réglage de l'heure
                    if(!setmin){
                        setmin = 1;
                        // TODO DRY
                        menudisplay("-----EditingTime----", "Minutes", "", "");
                        setposcursor(0x67);
                        lcd_send_string(">");
                    }
                    //on regle les minutes
                    if(setmin && !sethour){
                        data_composit(initDateTime[3], 0x1C);
                        if(confirmtime){
                            confirmtime = 0;
                            //on confirme les minutes pour passer aux heures
                            sethour = 1;
                            // TODO DRY
                            menudisplay("-----EditingTime----", "Hours", "", "");
                            setposcursor(0x67);
                            lcd_send_string(">");
                        }
                    }
                    //on regle les heures
                    if(sethour){
                        data_composit(initDateTime[4], 0x1C);
                        //on confirme les heures pour retourner au menu précédent
                        if(confirmtime){
                            setmin = 0;
                            sethour = 0;
                            confirmtime = 0;
                            poseditmenu = 0;
                            posmainmenu = 1;
                        }
                    }
                }else if(poseditmenu == 3) posmainmenu = 0; //On choisit Back
                if(poseditmenu == 0){
                    menudisplay("------SousMenu1-----", "Edit Date", "Edit Time", "Back");
                    poseditmenu = 1;
                    setline(poseditmenu);
                }
                //Fin du menu secondaire
            }else if(posmainmenu == 2 && confirm){ //On choisit Speeding Records
                //Deuxième menu secondaire
                //on change la variable que le bouton RA1 modifie
                menuselect = 2;
                //on choisit Edit Date --> réglage de la date
                if(posspeedmenu == 1){
                    if(!quitspeed){
                        displayspeed = 1;
                        Lecture_EEPROM(radardata_pos[num], radardata);
                        affichradardata(radardata, num);
                    }
                    if(quitspeed) posmainmenu = 0;
                }else if(posspeedmenu == 2){ //on choisit Edit Time --> réglage de l'heure
                    //Start Record
                    //fonctionrecord();
                }else if(posspeedmenu == 3) posmainmenu = 0; //On choisit Back
                if(posspeedmenu == 0){
                    menudisplay("------SousMenu2-----", "Display Records", "Start Record", "Back");
                    posspeedmenu = 1;
                    setline(posspeedmenu);
                }
                //Fin du deuxième menu secondaire
            }else if(posmainmenu == 3 && confirm){ //On choisit Date & Time Display pour afficher la date et l'heure
                lcd_send_cmd(0b00000001);
                __delay_ms(200);
                setposcursor(0x00);
                lcd_send_string("Today is the");
                data_composit(initDateTime[0], 0x40);
                lcd_send_string("/");
                data_composit(initDateTime[1], 0x44);
                lcd_send_string("/");
                data_composit(initDateTime[2], 0x48);
                setposcursor(0x14);;
                lcd_send_string ("and the time is");
                data_composit(initDateTime[4], 0x54);
                lcd_send_string(":");
                data_composit(initDateTime[3], 0x58);
                __delay_ms(5000);
                posmainmenu = 0;
            }
            if(posmainmenu == 0 && confirm){
                menudisplay("------MainMenu------", "Edit Date & Time", "Speeding Records", "Display Date & Time");
                poseditmenu = 0;
                posspeedmenu = 0;
                menuselect = 0;
                displayspeed = 0;
                quitspeed = 0;
                posmainmenu = 1;
                setline(posmainmenu);
            }
            //Fin du menu principal
        }else{
            menudisplay("/!\\","ERROR","EEPROM IS FULL","DATA WILL BE DELETED");
            __delay_ms(5000);
            //Supp_ALL_EEPROM();
            eeprom_is_full = 0;
        }
        confirm = 0;
    }  
    return;
}

// initialisation I²C
void i2c_init(void){
    TRISB = 0b00000000; // TRISB = 0; ou TRISB = 0x00;
    PORTB = 0;
    ADCON1 = 0b00000110;   // Désactivation des entrées analogiques
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
void i2c_Start(void){
    i2c_Wait();
    SSPCON2bits.SEN = 1;
}

// i2c_Restart - Re-Start I2C communication
void i2c_Restart(void){ // TODO JAMAIS APPELÉ
    i2c_Wait();
    SSPCON2bits.RSEN = 1;
}

// i2c_Stop - Stop I2C communication
void i2c_Stop(void){
    i2c_Wait();
    SSPCON2bits.PEN = 1;
}

// i2c_Write - Sends one byte of data
char i2c_Write(char data){
    i2c_Wait();
    SSPBUF = data;
    // TODO SOIT VOID SOIT AJOUTER UN RETURN
}

// sert à envoyer une commande au LCD, comme par exemple un display clear.
void lcd_send_cmd (char cmd){
    i2c_Start();
    i2c_Write(LCD_ADRESSE);
    i2c_Write(CONTROLBYTE_CMD);
    i2c_Write(cmd);
    i2c_Stop();
}

// initialisation du LCD
void LCD_init(){
    __delay_ms(100);
    lcd_send_cmd(0x38); //function set
    lcd_send_cmd(0x0F); //display on off
    lcd_send_cmd(0b00000001); //display clear
    __delay_ms(100);
    lcd_send_cmd(0b00001111); //permet de set un curseur qui blink à la position où on est
    lcd_send_cmd(0b10000000|0x00);
    lcd_send_string ("-------Hello--------");
    lcd_send_cmd(0b10000000|0x14);
    lcd_send_string ("This is our radar !");
    __delay_ms(3000);
}

// sert à envoyer une donnée à écrire sur l'écran LCD
void lcd_send_data(unsigned char data){
    i2c_Start();
    i2c_Write(LCD_ADRESSE);
    i2c_Write(CONTROLBYTE_DATA);
    i2c_Write(data);
    __delay_ms(10);
    i2c_Stop();
}

// TODO Ne doit utiliser que 2 chiffres 0 -> 99
// sert à afficher un int sur le LCD en le décomposant en ses chiffres. 
// ex : afficher 1234 en affichant 1 puis 2 puis 3 puis 4
void data_composit(unsigned char data, char pos){
    unsigned int data1, data2, data3, data4;
    data1 = data/1000;
    data2 = (data - data1*1000)/100;
    data3 = (data - data1*1000 - data2*100)/10;
    data4 = (data - data1*1000 - data2*100 - data3*10);
    
    setposcursor(pos); //0x1B
    //on met la condition if pour ne pas afficher un zéro inutile
    if(data1) lcd_send_data((unsigned char)('0' + data1));
    //on met la 2eme condition if pour ne pas afficher 2 zéros de suite
    if(data1 || data2) lcd_send_data((unsigned char)('0' + data2));
    lcd_send_data((unsigned char)('0' + data3));
    lcd_send_data((unsigned char)('0' + data4));
}

// sert à envoyer un ensemble de char à la fois, qui vont se suivre
void lcd_send_string(char *str){
	while (*str) lcd_send_data (*str++);
}

// Cette fonction sert à afficher 4 "string" sur chacune des 4 lignes du LCD, ces strings sont var1 var2 var3 var4
void menudisplay(char *str1, char *str2, char *str3, char *str4){
    lcd_send_cmd(0b00000001); //clear display
    __delay_ms(200);
    lcd_send_cmd(0b10000000 | 0x00);  // ligne 1 colonne 1
    lcd_send_string(str1);
    lcd_send_cmd(0b10000000 | 0x41);  // ligne 2 colonne 2
    lcd_send_string(str2);
    lcd_send_cmd(0b10000000 | 0x15);  // ligne 3 colonne 2
    lcd_send_string(str3);
    lcd_send_cmd(0b10000000 | 0x55);  // ligne 4 colonne 2
    lcd_send_string(str4);
}

// ca sert à mettre le curseur clignotant à la 1ere colonne des lignes 2, 3 ou 4 du LCD
void setline(char poscursor){
    if(poscursor == 1) lcd_send_cmd(0b10000000 | 0x40);
    if(poscursor == 2) lcd_send_cmd(0b10000000 | 0x14);
    if(poscursor == 3) lcd_send_cmd(0b10000000 | 0x54);
}

// sert à mettre le curseur clignotant où on le veut sur le LCD
void setposcursor(char pos){
    lcd_send_cmd(0b10000000 | pos);
}

void affichradardata(unsigned char tab[8], char num){
    lcd_send_cmd(0b00000001);
    __delay_ms(100);
    
    // Lignes 1
    setposcursor(0x00);
    lcd_send_string("Speeding record ");
    if(num < 9){
        lcd_send_data('0' + 1 + num);
    }else{
        lcd_send_data('0' + 1);
        lcd_send_data('0');
    }
    
    // Lignes 2
    setposcursor(0x41);
    lcd_send_string("Date:");
    data_composit((char)tab[2], 0x47); // JJ
    lcd_send_string("/");
    data_composit((char)tab[1], 0x4A); // MM
    lcd_send_string("/");
    data_composit((char)tab[0], 0x4D); // AA
    
    // Lignes 3
    setposcursor(0x15);
    lcd_send_string("Time:");
    data_composit((char)tab[3], 0x1B); // HH
    lcd_send_string(":");
    data_composit((char)tab[4], 0x1E); // MM
    lcd_send_string(":");
    data_composit((char)tab[5], 0x21); // SS
    
    // Lignes 4
    setposcursor(0x55);
    lcd_send_string("Speed:");
    data_composit((char)tab[6], 0x5C); // Vitesse
    lcd_send_string(",");
    data_composit((char)tab[7], 0x60); // Décimale
    lcd_send_string("km/h");
}

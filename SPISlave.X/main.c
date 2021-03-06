/*---Wait for BBB to initialise----*//*------------------------*/
/*---Launch a pulling to all RPI---*/
/*---All RPI PRESENT ANSWERED------*/
/*---Send control byte to BBB------*/
/*---Routine Update starts here----*/
/*---BBB  talk---------------------*//*------Possible talk------*//*--RPI WRITE MSG--*//*----RPI TURN PACKET----*//*--BBB TELLS PLAYER-TO PLAY-*//*-BBB BROADCAST----*//*----*/
/*---RPIA talk---------------------*//*----RPI WRITE MESSAGE----*//*--SOURCE,DEST----*//*--SENDS TO BEAGLEBONE--*//*--SENDS AFTER BBB FINISH---*//*-BBB TALKS TO ALL-*/
/*---RPIB talk---------------------*//*-BBB TELLS PLAYER TO PLAY*//*--LENGHT,MSG-----*//*-PIC WAITS ON COMEBACK-*//*--TREATING A RPI PACKET-AND*//*-HIGH PRIORITY-MSG*/
/*---RPIC talk---------------------*//*---RPI END TURN PACKET---*//*--ID:EE----------*//*-BBB SNDS TO ALL-------*//*--SENDS AFTER ANSWER TO----*//*---ID:CC---*/
/*---RPID talk---------------------*//*----BBB BROADCAST INFO---*//*-----------------*//*DEUX TRAMES, P1/OTHERS-*//*--END TURN-----------------*//*--*/
/*---REPEAT------------------------*//*----BBB TALKS TO RPI-----*/                     /*--ID:DD----------------*//*--ID:BB--------------------*//*--*/


#include "mcc_generated_files/mcc.h"
#include <stdio.h>

#define RPIA 0xA1       //B2
#define RPIB 0xA2       //B3 
#define RPIC 0xA3       //B4
#define RPID 0xA4       //B5
#define BBB  0xA6       //B1

//--------------------Function Declaration---------------------//

bool key_locking(int master, int slave);
int  receiving();
void transmit(int dest);
void BBB_broadcast();
void identify(int adress);
void routine_talk(int adress);
void send_control();
void BBB_turn_player(int dest);

//------------------END SEGMENT--------------------------------//

uint16_t    MY_BUFFER_SIZE = 256;
bool        BBB_INIT=false;
uint8_t     writeDummy[1]={0x55};
uint8_t     readData[60];       //grandeur du tableau de réception
uint8_t     readDummy;
uint8_t     i;


/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();
    SPI_Initialize();
    printf("intialisation complete\n\r");
    TMR1_StartTimer();
    //-----------------TODO-----------------------------//
    // while(BBB_INIT==false){
        //wait for lock with BBB 
        //wait for RPIA to lock ///
        //wait for RPIB to lock ///LES JOUEURS RECOIVENT UNE NOTIFICATION QU'ILS SONT CONNECTER
        //wait for RPIC to lock ///IL FAUT CLIQUER SUR LE CONTROLEUR PIC DU BEAGLEBONE POUR CONFIRMER LE NOMBRE DE JOUEUR
        //wait for RPID to lock ///
    // }
    while(1){
    routine_talk(BBB);
    routine_talk(RPIA);
    routine_talk(RPIB);
    //routine_talk(RPIC);
    //routine_talk(RPID);
    }
}


bool key_locking(int master,int slave){              //Attend l'adresse de source(Master)(va renvoyer son propre adresse)
    int master_lock = SPI_Exchange8bit(slave);      //Forme du read. Le writeData est un Dummy 
    if (master_lock == master){                          //Si la transmission qui vient de passer est un 0x6F
        return true;
    } else{
        return false;
    }
}

int receiving(){
    for(i=0;i<4;i++){
        readData[i]=SPI_Exchange8bit(0x55);
    }
    for(i=0;i<readData[3];i++){
        readData[i+4]=SPI_Exchange8bit(0x55);
    }
    return (readData[2]);
}
    
void transmit(int dest){  
    int TrueLenght;
    bool lock=false;
    identify(dest);
    while(lock==false)lock=key_locking(0x6F,0xA6);    //Attend un 6F, Envoie un A6 --> PIC veut communiquer
    TrueLenght=readData[3]+4;
    for(i=0;i<TrueLenght;i++){
    readDummy = SPI_Exchange8bit(readData[i]);
    }
}

void send_control(){
    
}

void BBB_broadcast(){
    int TrueLenght;
    TrueLenght=readData[3]+4;
    identify(RPIA);
    for(i=0;i<TrueLenght;i++){
        readDummy = SPI_Exchange8bit(readData[i]);
    }
    identify(RPIB);
    for(i=0;i<TrueLenght;i++){
        readDummy = SPI_Exchange8bit(readData[i]);
    }
    identify(RPIC);
    for(i=0;i<TrueLenght;i++){
        readDummy = SPI_Exchange8bit(readData[i]);
    }
    identify(RPID);
    for(i=0;i<TrueLenght;i++){
        readDummy = SPI_Exchange8bit(readData[i]);
    }
}

void BBB_turn_player(int dest){
    identify(dest);
    transmit(dest);
}

void identify(int adress){
    IO_RB1_SetHigh();    //BBB 
    IO_RB2_SetHigh();    //RPI1
    IO_RB3_SetHigh();    //RPI2
    IO_RB4_SetHigh();    //RPI3
    IO_RB5_SetHigh();    //RPI4
    __delay_ms(0.500);
    if (adress==BBB) IO_RB1_SetLow();    //BBB 
    if (adress==RPIA)IO_RB2_SetLow();    //RPIA 
    if (adress==RPIB)IO_RB3_SetLow();    //RPIB
    if (adress==RPIC)IO_RB4_SetLow();    //RPIC
    if (adress==RPID)IO_RB5_SetLow();    //RPID
}

void routine_talk(int adress){
    bool lock=false;
    int type;
    bool time_up=false;
    TMR1_WriteTimer(0);
    identify(adress);
    while(time_up==false){        
        int ReadTimer=TMR1_ReadTimer();
        if(ReadTimer>=11500){       //A revoir permet de calculer 20 ms avant de changer de RPI ou BBB
            time_up=true;           //FLAG up
        }
        lock=key_locking(0x60,0x06);        //Verifier si quelqu'un veut parler et PIC veut ecouter
        if(lock==true){                     //Etat handshake complete
            lock=false;
            type=receiving();               //receiving recoit la trame, traite le type ensuite
            if(type == 0xEE){               //RPI_MSG
                transmit(readData[1]);          //Transmission vers un destinateur particulier
            //**********************PAS TESTER********************//
            } else if(type == 0xDD){  //RPI_END TURN        
                transmit(BBB);                  
            } else if(type == 0xCC){  //BBB broadcast
                BBB_broadcast();
            } else if(type == 0xBB){  //BBB_SINGLE
                BBB_turn_player(readData[1]);
                BBB_broadcast();
            }
        }
    }

}

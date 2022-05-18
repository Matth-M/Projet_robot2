#include "MI2C.h"
#include <p18f2520.h>
#include "def.h"
#include "init.h"
#include "it.h"
#include "fonction.h"


#pragma config OSC = INTIO67
#pragma config PBADEN = OFF, WDT = OFF, LVP = OFF, DEBUG = ON


int distance_capteur_droit;
int distance_capteur_gauche;
int tps_avance=0;
int compteur_tourner=0;
int flag_bouton_central=0;
int flag_apres_tourne=0;
int flag_bat_faible=0;
int flag_tourne=0;
int seuil_distance=102;//Q=2^8*(2/5)
int tps_pour_acquerir=20;
unsigned char adresse_pcf=0x40;


/*
--------------------------------main-------------------------------------------
*/


void main(void) {
    init();
    while(1){
       
        if(flag_bouton_central){
            deplacement_autonome();
        }
    }

}

#include <stdio.h>
#include "MI2C.h"
#include <p18f2520.h>
#include "def.h"
#include "fonction.h"


unsigned char adresse_telecommande_lecture=0xA2;


int seuil_batterie=169; //seuil_batterie de la batterie a  10V, correspondant a 5V apres le pont diviseur de tension, avec Q=2^8*(3.3/5)


unsigned char message_recu[3];
int Ubat[8];

int k=0;
int flag_moyenne=0;


int temps_un_metre=21;      //supposition du temps pour avancer d'un metre
int tps_tourner=12;         //supposition du temps pour tourner





/*
----------------------------batterie-------------------------------------------
*/

void surveillance_batterie(void){

    int moyenne;
    int i=0;

    ADCON0bits.CHS0=0;                  //selection de la voix analogique 2
    ADCON0bits.CHS1=1;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS3=0;


    ADCON0bits.GO = 1; //if =1 : conversion en cours
    while(PIR1bits.ADIF !=1){}  // tant que conversion pas finie, reste dans la boucle vide
    Ubat[k]=ADRESH; // RÃ©cupÃ¨re la valeur en sortie de l'ADC
    PIR1bits.ADIF=0; // reset le bit pour le prochain passage

    if(k==7 && !flag_moyenne){ //permet de commencer la mesure 800ms aprÃ¨s le dÃ©marrage
        flag_moyenne=~flag_moyenne;
    }

    if(flag_moyenne){
        for(i;i<8;i++){
            moyenne+=Ubat[k]; //on incrÃ©mente la moyenne
        }
        moyenne=moyenne/8; //On effectue la moyenne
        i=0;

        if (moyenne<seuil_batterie){  //definir le seuil_batterie.
            flag_bat_faible=1;
            printf("batterie faible, valeur de la moyenne : %d\n\r", moyenne);
            PORTBbits.RB5 = 1;// led s'allume lorsque batterie faible
        }
        else {
            printf("batterie ok, valeur de la moyenne : %d\n\r", moyenne);
            flag_bat_faible=0;
            PORTBbits.RB5 = 0;// led eteinte lorsque batterie pas faible
        }
    }
    else{
        printf("pas assez de valeurs pour faire la moyenne flottante, derniï¿½re valeur de Ubat=%d ",Ubat[k]);
        if (Ubat[k]<seuil_batterie){
            flag_bat_faible=1;
            printf("batterie faible \n\r");
            PORTBbits.RB5 = 1;
        }
        else {
            printf("batterie ok \n\r");
            flag_bat_faible=0;
            PORTBbits.RB5 = 0;
        }
    }
    k++;
    k=k%8;
}


/*
-------------------------fonction capteur------------------------------------
*/
void acquisition_capteur(void){
    ADRESH=0;
    ADCON0bits.CHS0=0;                  //selection de la voix analogique 0
    ADCON0bits.CHS1=0;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS3=0;

    ADCON0bits.GO = 1;
    while(PIR1bits.ADIF !=1){}// tant que conversion pas finie, reste dans la boucle vide

    distance_capteur_droit=ADRESH;
    ADRESH=0;
    PIR1bits.ADIF=0;// reset le bit pour le prochain passage


    ADCON0bits.CHS0=1;                  //selection de la voix analogique 1
    ADCON0bits.CHS1=0;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS3=0;

    ADCON0bits.GO = 1;
    while(PIR1bits.ADIF !=1){}

    distance_capteur_gauche=ADRESH;
    ADRESH=0;
    PIR1bits.ADIF=0; // reset le bit pour le prochain passage
    printf("capteur gauche: %d //capteur droit : %d \n\r",distance_capteur_gauche,distance_capteur_droit);
}



/*
----------------------------telecommande-------------------------------------------
*/
void telecommande(void){
    if(Detecte_i2c(adresse_telecommande_lecture)!=0){
        printf("erreur la telecommande n'est pas detectee par l'I2C\n\r");
    }
    else{
        Lire_i2c_Telecom(adresse_telecommande_lecture, message_recu);
        printf("message recu par la telecommande : %s\n\r",message_recu);

        if(Detecte_i2c(adresse_pcf)!=0){
        printf("erreur le pcf n'est pas detecter par l'I2C\n\r");
        }
        else{
            Write_PCF8574(adresse_pcf, message_recu[1] );
        }
        if(message_recu[1]=='3')
        {
            flag_bouton_central= ~flag_bouton_central;

            if(flag_bouton_central){
                PORTBbits.RB1 = 0; //allume les capteurs
                printf("allumage des capteurs\n\r");

                flag_apres_tourne=0;
                tps_avance=0;
                flag_tourne=0;
                tps_avance=0;
                compteur_tourner=0;
                marche();
            }
            else{

                arret();
                PORTBbits.RB1 =1; // ï¿½teint les capteurs
                printf("fin des capteurs\n\r");
                flag_apres_tourne=0;
                tps_avance=0;
                flag_tourne=0;
                tps_avance=0;
                compteur_tourner=0;
            }

        }
        tps_pour_acquerir=0;
    }
}




void deplacement_autonome(void){
    if(flag_apres_tourne){
        if (tps_avance>=temps_un_metre){ // Si on a parcouru 1m ou plus
                arret();//on sarrete
                PORTBbits.RB1 = 1; // on ne lit plus l'input
                printf("fin des capteur\n\r");
                flag_bouton_central=0;


        }
        else{
        printf("avance sans obstacle\n\r");
        }
    }
    if(distance_capteur_droit>=seuil_distance || distance_capteur_gauche>=seuil_distance){
                printf("obstacle detecte, rotation vers la droite\n\r");
                arret();
                tourne();
                arret();
                
                tps_avance=0;
                if(flag_bouton_central){  //si arret par la telecommande pendant qu'il tourne
                flag_apres_tourne=1;
                marche();
                }
    }

}




void arret(void){
    CCPR1L=0;                            //choix du rapport cyclique a 0 sur CCP1
    CCP1CONbits.DC1B0=0;
    CCP1CONbits.DC1B1=0;

    CCPR2L=0;                            //choix du rapport cyclique a 0 sur CCP1
    CCP2CONbits.DC2B0=0;
    CCP2CONbits.DC2B1=0;
    printf ("arrï¿½t \n\r");
}

void marche(void){
    CCPR1L=53;                            //choix du rapport cyclique a 20% sur CCP1 : 20% de 2^8 moteur gauche
    CCP1CONbits.DC1B0=0; //on n'utilise pas les 2 derniers bits, résolution bloquée à 8bits
    CCP1CONbits.DC1B1=0;
    printf ("mise en marche moteur \n\r");

    CCPR2L=63;                            //choix du rapport cyclique a 20% sur CCP2 Moteur droit
    CCP2CONbits.DC2B0=0;
    CCP2CONbits.DC2B1=0;
}

void tourne(void){
    flag_tourne=1;
    printf("tourne\n\r");
    PORTAbits.RA6=~PORTAbits.RA6; // roue droite change de sens de rotation

    CCPR2L=30;                              //choix du rapport cyclique a 10%
    CCP2CONbits.DC2B0=0;
    CCP2CONbits.DC2B1=0;
    CCPR1L=30;
    CCP1CONbits.DC1B0=0;
    CCP1CONbits.DC1B1=0;
    while(compteur_tourner<tps_tourner && flag_tourne){ }
    flag_tourne=0;

    compteur_tourner=0;
    PORTAbits.RA6=~PORTAbits.RA6;


}





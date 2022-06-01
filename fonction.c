#include <stdio.h>
#include "MI2C.h"
#include <p18f2520.h>
#include "def.h"
#include "fonction.h"


unsigned char telecommande_adresse=0xA2;




unsigned char received_message[3];
int Ubat[8];
int bat_seuil=169; //bat_seuil de la batterie a 10V, correspondant a 5V apres le pont diviseur de tension, avec Q=2^8*(3.3/5)
int moy_f=0;
int k=0;

int turn_time=12;         //supposition du temps pour tourner
int metre_time=21;      //supposition du temps pour avancer d'un metre




/*
---------------------------Partie Batterie-------------------------------------------
*/

void surveillance_batterie(void){

    int moy;
    int i=0;

//Nous sélectionnons la voix analogique 2
    ADCON0bits.CHS0=0;                  
    ADCON0bits.CHS1=1;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS3=0;

//si ce bit vaut 1 : la conversion est en cours
    ADCON0bits.GO = 1; 
    while(PIR1bits.ADIF !=1){}  // tant que conversion pas finie, reste dans la boucle vide
    Ubat[k]=ADRESH; // Récupère la valeur en sortie de l'ADC
    PIR1bits.ADIF=0; // reset le bit pour le prochain passage

//permet de commencer la mesure 800ms après le démarrage
    if(k==7 && !moy_f){ 
        moy_f=~moy_f;
    }

    if(moy_f){
        // On mesure 8 valeurs différentes
        for(i;i<8;i++){
            moy+=Ubat[k]; //on incrémente la moyenne
        }
        moy=moy/8; //On effectue la moyenne
        i=0;

//Si la moyenne est trop basse
        if (moy<bat_seuil){  
            printf("la batterie est faible, sa moyenne est de: %d\n\r", moy);
            PORTBbits.RB5 = 1;// LED s'allume lorsque batterie faible
        }
        else {
            printf("La batterie est assez chargée, sa moyenne est de : %d\n\r", moy);
            PORTBbits.RB5 = 0;// LED eteinte quand la batterie n'est pas faible
        }
    }
    else{
        printf("pas assez de valeurs pour faire la moyenne flottante, derniere valeur de Ubat=%d ",Ubat[k]);
        if (Ubat[k]<bat_seuil){
            printf("La Batterie est faible (moyenne pas encore disponible) \n\r");
            PORTBbits.RB5 = 1;
        }
        else {
            printf("La batterie est assez chargée (moyenne pas encore disponible)\n\r");
            PORTBbits.RB5 = 0;
        }
    }
    //Incrémentation de k et modulo
    k++;
    k=k%8;
}


/*
-------------------------Partie Capteur------------------------------------
*/
void acquisition_capteur(void){
    ADRESH=0;
//Nous selectionnons la voix analogique 0
    ADCON0bits.CHS0=0;                 
    ADCON0bits.CHS1=0;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS3=0;

    ADCON0bits.GO = 1;
    while(PIR1bits.ADIF !=1){} // Tant que conversion pas finie, reste dans la boucle vide
//On récupère la valeur renvoyée par l'ADC
    distance_capteur_droit=ADRESH;
    ADRESH=0;
    PIR1bits.ADIF=0;// reset le bit pour le prochain passage

//Nous selectionnons la voix analogique 1

    ADCON0bits.CHS0=1;                 
    ADCON0bits.CHS1=0;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS3=0;

    ADCON0bits.GO = 1;
    while(PIR1bits.ADIF !=1){}

    distance_capteur_gauche=ADRESH;
    ADRESH=0;
    PIR1bits.ADIF=0; // reset le bit pour le prochain passage
    printf("Valeur capteur gauche: %d \nValeur capteur droit : %d \n\r",distance_capteur_gauche,distance_capteur_droit);
}



/*
----------------------------Partie Telecommande-------------------------------------------
*/
void telecommande(void){
    if(Detecte_i2c(telecommande_adresse)!=0){
        printf("Telecommande non detectee par l'I2C\n\r");
    }
    else{
        Lire_i2c_Telecom(telecommande_adresse, received_message);
        printf("Message recu : %s\n\r",received_message);

        if(Detecte_i2c(adresse_pcf)!=0){
        printf("PCF non detecte par l'I2C\n\r");
        }
        else{
            Write_PCF8574(adresse_pcf, received_message[1] );
        }

        //Si bouton central appuyé
        if(received_message[1]=='3')
        {
            // flag permettant de savoir si le bouton central a été poussé
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
                PORTBbits.RB1 =1; // �teint les capteurs
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
        if (tps_avance>=metre_time){ // Si on a parcouru 1m ou plus
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
    printf ("arr�t \n\r");
}

void marche(void){
    CCPR1L=53;                            //choix du rapport cyclique a 20% sur CCP1 : 20% de 2^8 moteur gauche
    CCP1CONbits.DC1B0=0; //on n'utilise pas les 2 derniers bits, r�solution bloqu�e � 8bits
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
    while(compteur_tourner<turn_time && flag_tourne){ }
    flag_tourne=0;

    compteur_tourner=0;
    PORTAbits.RA6=~PORTAbits.RA6;


}





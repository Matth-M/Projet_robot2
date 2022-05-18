#include "it.h"
#include "MI2C.h"
#include <p18f2520.h>
#include "def.h"
#include "fonction.h"


int temps_telecommande=10;
int n=10;

int i=0;                        //clignotant
unsigned char clignotant=0x03;

int m=0;
unsigned char Led_avance=0x01;

#pragma code HighVector=0x08
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code

/*
-----------------------interruption-------------------------------------------
*/


#pragma interrupt HighISR
void HighISR(void)
{
    if(INTCONbits.TMR0IF)
   {
        n++;
        if( tps_pour_acquerir<temps_telecommande){
             tps_pour_acquerir++;
        }

        /*-----------------------------
         fonction interruption timer 0
         -----------------------------*/
        if(!PORTBbits.RB1){
        acquisition_capteur();
        }

        if(n>=10)
        {
            /*-----------------------------
            fonction surveillance batterie
            -----------------------------*/
            surveillance_batterie();
            n=0;

        }
        if(flag_tourne){
            compteur_tourner++;
            i++;
            if(i>=2)
            {
                Write_PCF8574(adresse_pcf, 0xff-clignotant );
                clignotant=2* clignotant;
                if(!(clignotant =clignotant%0xff)){
                    clignotant=0x03;
                }
                i=0;
            }

        }
        if(flag_apres_tourne){
            tps_avance++;
        }
        if(flag_bouton_central && !flag_tourne)
        {
            m++;
            if(m>=2)
            {
                Led_avance=2*Led_avance +1;
                Write_PCF8574(adresse_pcf, 0xff-Led_avance );
               
                if(!(Led_avance =Led_avance%0xff)){
                    Led_avance=0x01;
                }
                m=0;
            }
        }
        /*-----------------------------
         remise a zero du timer0
         -----------------------------*/
        INTCONbits.TMR0IF=0;
        TMR0H=0b00111100;                    //recharge de 15 535
        TMR0L=0b010101111;
   }



    if(INTCONbits.INT0IF)
   {
   /*-----------------------------
        fonction int 0
   -----------------------------*/
        if( tps_pour_acquerir>=temps_telecommande){
           telecommande();
        }

        INTCONbits.INT0IF = 0;
   }


}



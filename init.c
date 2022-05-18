#include "init.h"
#include "MI2C.h"
#include "def.h"
#include <p18f2520.h>



void init(void){

   /*-----------------------------
       //init horloge
    ----------------------------*/

    OSCCONbits.IRCF0 = 1 ;               //initialise l'horloge 8Mhz
    OSCCONbits.IRCF1 = 1 ;
    OSCCONbits.IRCF2 = 1 ;

    /*-----------------------------
        init ADC
    ----------------------------*/

    ADCON1bits.PCFG0=0;                 //config entree analogique A0/A1/A2
    ADCON1bits.PCFG1=0;
    ADCON1bits.PCFG2=1;
    ADCON1bits.PCFG3=1;

    ADCON1bits.VCFG0=0;                 //on se place sur la tension d'alimentation
    ADCON1bits.VCFG1=0;

    ADCON2bits.ADCS0=1;                 //Tad a 1 micro seconde soit Fosc/8
    ADCON2bits.ADCS1=0;
    ADCON2bits.ADCS2=0;

    ADCON2bits.ACQT0=1;                 //Tacq=6Tad
    ADCON2bits.ACQT1=1;
    ADCON2bits.ACQT2=0;

    ADCON2bits.ADFM=0;                  //justification des resultat gauche car on va travailler en 8bit

    ADCON0bits.ADON=1;                  //validation ADC


    /*-----------------------------
        init I2C
    ----------------------------*/

    TRISCbits.RC3=1;    //scl
    TRISCbits.RC4=1;    //sda

    MI2CInit();

    /*-----------------------------
        init timer0 100ms
    ----------------------------*/

    T0CONbits.TMR0ON=1;                  //active le timer
    T0CONbits.T08BIT=0;                  //overflow sur 16bit
    T0CONbits.T0CS=0;                    //on prend la frequence d'horloge
    T0CONbits.PSA=0;                     //on prend le prescaler

    T0CONbits.T0PS2=0;                   //prescaler de 4
    T0CONbits.T0PS1=0;
    T0CONbits.T0PS0=1;

    TMR0H=0b00111100;                    //recharge de 15 535
    TMR0L=0b10101111;

    TRISBbits.RB5=0;                   //broche rb5 en sortie pour la batterie
    TRISBbits.RB1=0;                  //broche rb1 en sortie pour l'allumage des capteurs
    TRISAbits.RA7=0;
    TRISAbits.RA6=0;
    PORTAbits.RA6=1;
    PORTAbits.RA7=1;
    PORTBbits.RB1=1;
    /*-----------------------------
        init timer 2 1kHz
    ----------------------------*/


    T2CONbits.T2CKPS1=1;                 //Prescaler a 16
    T2CONbits.T2CKPS0=1;

    T2CONbits.TOUTPS0=0;                 //postacler x1 par rapport au pwm
    T2CONbits.TOUTPS1=0;
    T2CONbits.TOUTPS2=0;
    T2CONbits.TOUTPS3=0;

    PR2=124;                               //PR2 a pour 1kHz

    CCPR1L=0;                            //choix du rapport cyclique a 0 sur CCP1
    CCP1CONbits.DC1B0=0;
    CCP1CONbits.DC1B1=0;

    CCPR2L=0;                            //choix du rapport cyclique a 0 sur CCP1
    CCP2CONbits.DC2B0=0;
    CCP2CONbits.DC2B1=0;

    TRISCbits.RC1=0  ;                  //broche CCP1 et CCP2 en sortie
    TRISCbits.RC2=0  ;


    CCP1CONbits.CCP1M2=1;               //CCP1CON
    CCP1CONbits.CCP1M3=1;
    CCP2CONbits.CCP2M2=1;               //CCP2CON
    CCP2CONbits.CCP2M3=1;

    T2CONbits.TMR2ON =1;                //validation timer 2


    /*-----------------------------
     Initialisation uart
     ----------------------------*/
                                        //initialise baudrate en 8bit asynchrone
    TXSTAbits.SYNC = 0 ;                //asynchrone
    BAUDCONbits.BRG16 =0;               //8 bit
    TXSTAbits.BRGH=1 ;                  //plus de precision avec haute vitesse
    SPBRG = 103;                        //4800 bds


    TRISCbits.RC6=0;                    //broche en entrée

    TRISCbits.RC7=1;                    //broche en sortie

    //transmition----------------------------------

    RCSTAbits.SPEN =1;   //validation broche TX

    TXSTAbits.TXEN=1;   //validation transmition


    //reception----------------------------------

    RCSTAbits.CREN =1 ;  //validation reception


    /*-----------------------------
     Initialisation interruption
     ----------------------------*/



    PIR1bits.ADIF=0;                    //effacement interruption ADC
    PIE1bits.ADIE=0;                    //validation interruption ADC

    INTCONbits.TMR0IE = 1;               //interruption timer 0
    INTCONbits.TMR0IF = 0;

    PIR1bits.RCIF=0;    //interruption registre reception de l'uart
    //PIE1bits.RCIE=1;

    PIR1bits.TXIF=0;     //interruption registre de transimition TXREG de l'uart
    //PIE1bits.TXIE=1;

    INTCONbits.GIE=1;
    INTCONbits.PEIE=1;
    INTCON2bits.INTEDG0 = 0;
    INTCONbits.INT0IE = 1;              //initialise int0

}

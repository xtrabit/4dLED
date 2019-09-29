/*
 * File:   newmain4dLED.c
 * Author: lexa
 *
 * Created on July 4, 2018, 5:44 PM
 */

#include <xc.h>

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

//#include <stdio.h>
//#include <stdlib.h>

#define _XTAL_FREQ 16000000

/*
#define E   LATAbits.LATA2
#define C   LATAbits.LATA4
#define A   LATAbits.LATA5
#define G   LATBbits.LATB5
#define C1  LATBbits.LATB7
#define F   LATCbits.LATC0
#define D   LATCbits.LATC1
#define B   LATCbits.LATC2
#define DP  LATCbits.LATC3
#define C3  LATCbits.LATC4
#define C2  LATCbits.LATC5
#define C4  LATCbits.LATC6
#define SS  PORTAbits.RA3
 */
#define E   LATAbits.LATA2
#define C   LATAbits.LATA4
#define A   LATAbits.LATA5
#define G   LATBbits.LATB5
#define C4  LATBbits.LATB7
#define F   LATCbits.LATC0
#define D   LATCbits.LATC1
#define B   LATCbits.LATC2
#define DP  LATCbits.LATC3
#define C2  LATCbits.LATC4
#define C3  LATCbits.LATC5
#define C1  LATCbits.LATC6
#define SS  PORTAbits.RA3

unsigned int buf; // value from spibuffer
char t; // determines which segment to display
char temp_digit; //segment number
char digit1; // value
char digit2; // value
char digit3; // value
char digit4; // value
char digit_error; // error value 0 or 1
char dummy; // spi dummy read
char no_spi; // spi present toggle
unsigned int no_spi_timer; // spi timeout
char no_spi_counter; // no spi display number count
char no_spi_digit; // no spi display digit position
char update; // switch to another segment to display
char on;
char off;
void display(unsigned int x);
void display_dash(void); // display dash when no spi

void __interrupt() myIsr(void)
{
    if (TMR2IF == 1) //sets screen update duty cycle
    {
        if (t < 3)
        {
            t++;
        }
        else
        {
            t = 0;
        }

        if (no_spi_timer < 5700)
        {
            no_spi_timer++;
        }
        else
        {
            no_spi_timer = 0;
            no_spi = 1;
            if (no_spi_counter < 15)
            {
                no_spi_counter++;
            }
            else
            {
                no_spi_counter = 0;
                if (no_spi_digit < 3)
                {
                    no_spi_digit++;
                }
                else
                {
                    no_spi_digit = 0;
                }
            }
        }

        update = 1;
        TMR2IF = 0;
    }

    if (SSP1IF == 1)
    {
        buf = SSPBUF;
        temp_digit = (buf & 0x0070) >> 4;
        no_spi_timer = 0;
        no_spi = 0;
        digit_error = 0;

        switch (temp_digit)
        {
            case 1:
                digit1 = (buf & 0x000f);
                break;
            case 2:
                digit2 = (buf & 0x000f);
                break;
            case 3:
                digit3 = (buf & 0x000f);
                break;
            case 4:
                digit4 = (buf & 0x000f);
                break;
            default:
                digit_error = 1;
                SSPEN = 0;
                SSP1CON1bits.WCOL = 0;
                SSP1CON1bits.SSPOV = 0;
                //dummy = SSPBUF;
                SSPEN = 1;
        }
        SSP1IF = 0;
    }


    return;
}

void main(void)
{
    on = 0;
    off = 1;

    IRCF3 = 1; // 16mhz clock
    IRCF2 = 1;
    IRCF1 = 1;
    IRCF0 = 1;

    SCS1 = 1; //internal oscillator
    SCS0 = 1;

    SSSEL = 1; //SS function is on RA3 MCLR

    TRISA = 0x0B; //0b0000 1011   RA0 ICSPDAT; RA1 ICSPCLK; RA3 MCLR/SS
    TRISB = 0x50; //0b0101 0000   RB4 SDI; RB6 SCK
    TRISC = 0x00; //0b0000 0000

    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;

    WPUA = 0x00;
    WPUB = 0x00;

    SSP1IF = 0;
    SSP1IE = 1;

    SSPSTAT = 0x00; //SMP7=0; CKE6=0;
    SSPCON1 = 0x05; //0b0000 0101 SS=disabled; CKP4=0 Idle clock is low
    SSPCON3 = 0x00; //BOEN4=0 ; If BF is set, SSPOV is set and buffer is not updated

    T2CON = 0B01111001; //T2OUTPS6-3<3:0> = 1:16 Postscaler; T2CKPS0-1<1:0> = Prescaler is 4;
    PR2 = 0x0a; //0b0000 1010 = 10; 0.00016 seconds period-6.25 KHz??; 62500=0xF424=1second; 
    TMR2IF = 0;
    TMR2IE = 1;
    TMR2 = 0;

    A = off;
    B = off;
    C = off;
    D = off;
    E = off;
    F = off;
    G = off;
    DP = off;
    C1 = off;
    C2 = off;
    C3 = off;
    C4 = off;

    t = 0;
    update = 0;
    no_spi = 0;
    no_spi_timer = 0;
    no_spi_counter = 0;
    no_spi_digit = 0;

    GIE = 1;
    PEIE = 1;

    SSPEN = 1;
    TMR2ON = 1;

    while (1)
    {
        if (update == 1)
        {
            if (no_spi == 1)
            {
                switch (t)
                {
                    case 0:
                        if (no_spi_digit == 0)
                        {
                            display(no_spi_counter);
                        }
                        else
                        {
                            display_dash();
                        }
                        C1 = on;
                        break;
                    case 1:
                        if (no_spi_digit == 1)
                        {
                            display(no_spi_counter);
                        }
                        else
                        {
                            display_dash();
                        }
                        C2 = on;
                        break;
                    case 2:
                        if (no_spi_digit == 2)
                        {
                            display(no_spi_counter);
                        }
                        else
                        {
                            display_dash();
                        }
                        C3 = on;
                        break;
                    case 3:
                        if (no_spi_digit == 3)
                        {
                            display(no_spi_counter);
                        }
                        else
                        {
                            display_dash();
                        }
                        C4 = on;
                        break;
                }
            }
            else
            {
                switch (t)
                {
                    case 0:
                        if (digit_error == 1) display(14);
                        else display(digit1);
                        C1 = on;
                        break;
                    case 1:
                        if (digit_error == 1) display(14);
                        else display(digit2);
                        C2 = on;
                        break;
                    case 2:
                        if (digit_error == 1) display(14);
                        else display(digit3);
                        C3 = on;
                        break;
                    case 3:
                        if (digit_error == 1) display(14);
                        else display(digit4);
                        C4 = on;
                        break;
                }
            }
            update = 0;
        } // update

    } // while(1)

} // main

void display(unsigned int x)
{
    A = off;
    B = off;
    C = off;
    D = off;
    E = off;
    F = off;
    G = off;
    DP = off;
    C1 = off;
    C2 = off;
    C3 = off;
    C4 = off;

    switch (x)
    {
        case 0:
            A = on;
            B = on;
            C = on;
            D = on;
            E = on;
            F = on;
            G = off;
            DP = off;
            break;
        case 1:
            A = off;
            B = on;
            C = on;
            D = off;
            E = off;
            F = off;
            G = off;
            DP = off;
            break;
        case 2:
            A = on;
            B = on;
            C = off;
            D = on;
            E = on;
            F = off;
            G = on;
            DP = off;
            break;
        case 3:
            A = on;
            B = on;
            C = on;
            D = on;
            E = off;
            F = off;
            G = on;
            DP = off;
            break;
        case 4:
            A = off;
            B = on;
            C = on;
            D = off;
            E = off;
            F = on;
            G = on;
            DP = off;
            break;
        case 5:
            A = on;
            B = off;
            C = on;
            D = on;
            E = off;
            F = on;
            G = on;
            DP = off;
            break;
        case 6:
            A = on;
            B = off;
            C = on;
            D = on;
            E = on;
            F = on;
            G = on;
            DP = off;
            break;
        case 7:
            A = on;
            B = on;
            C = on;
            D = off;
            E = off;
            F = off;
            G = off;
            DP = off;
            break;
        case 8:
            A = on;
            B = on;
            C = on;
            D = on;
            E = on;
            F = on;
            G = on;
            DP = off;
            break;
        case 9:
            A = on;
            B = on;
            C = on;
            D = on;
            E = off;
            F = on;
            G = on;
            DP = off;
            break;
        case 10:
            A = on;
            B = on;
            C = on;
            D = off;
            E = on;
            F = on;
            G = on;
            DP = off;
            break;
        case 11:
            A = off;
            B = off;
            C = on;
            D = on;
            E = on;
            F = on;
            G = on;
            DP = off;
            break;
        case 12:
            A = on;
            B = off;
            C = off;
            D = on;
            E = on;
            F = on;
            G = off;
            DP = off;
            break;
        case 13:
            A = off;
            B = on;
            C = on;
            D = on;
            E = on;
            F = off;
            G = on;
            DP = off;
            break;
        case 14:
            A = on;
            B = off;
            C = off;
            D = on;
            E = on;
            F = on;
            G = on;
            DP = off;
            break;
        case 15:
            A = on;
            B = off;
            C = off;
            D = off;
            E = on;
            F = on;
            G = on;
            DP = off;
            break;
    }
}

void display_dash(void)
{
    A = off;
    B = off;
    C = off;
    D = off;
    E = off;
    F = off;
    G = off;
    DP = off;
    C1 = off;
    C2 = off;
    C3 = off;
    C4 = off;

    G = on;

}
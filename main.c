/*
    Spectral Digitizer: Code for the spectral digitzer project
    Copyright (C) 2017  Chris Thierauf <chris@cthierauf.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
char version[] = {"v2.0 (beta) 24-July-2017"};

// Includes
//#include <xc.h>             // For hardware-specific stuff
#include "stdio.h"          // stdio
#include "stdlib.h"         // stdlib
#include "stdbool.h"        // Booleans
#include "stdint.h"         // Gives the uint8_t stuff
#include "string.h"         // Strings

// Setup bits
// PIC18F2431 Configuration Bit Settings
// 'C' source line config statements
// CONFIG1H
#pragma config OSC = IRC      // Oscillator Selection bits (Internal oscillator block, port function on RA6 and port function on RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON        // Internal External Oscillator Switchover bit (Internal External Switchover mode enabled)
// CONFIG2L
#pragma config PWRTEN = OFF     // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled)
// BORV = No Setting
// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDPS = 32768     // Watchdog Timer Postscale Select bits (1:32768)
#pragma config WINEN = OFF      // Watchdog Timer Window Enable bit (WDT window disabled)
// CONFIG3L
#pragma config PWMPIN = OFF     // PWM output pins Reset state control (PWM outputs disabled upon Reset (default))
#pragma config LPOL = HIGH      // Low-Side Transistors Polarity (PWM0, 2, 4 and 6 are active-high)
#pragma config HPOL = HIGH      // High-Side Transistors Polarity (PWM1, 3, 5 and 7 are active-high)
#pragma config T1OSCMX = OFF    // Timer1 Oscillator MUX (Standard (legacy) Timer1 oscillator operation)
// CONFIG3H
#pragma config MCLRE = ON       // MCLR Pin Enable bit (Enabled)
// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Low-Voltage ICSP Enable bit (Low-voltage ICSP enabled)
// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000200-000FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (001000-001FFF) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (002000-002FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (003000-003FFFh) not code-protected)
// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)
// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000200-000FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (001000-001FFF) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (002000-002FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (003000-003FFFh) not write-protected)
// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000200-000FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (001000-001FFF) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (002000-002FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (003000-003FFFh) not protected from table reads executed in other blocks)
// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from table reads executed in other blocks)

/* Pin definitions, for keeping things easier later */
#define LEDpin          LATAbits.LATA0
#define directionpin    LATBbits.LATB4
#define stepperpulsepin LATBbits.LATB2
#define manualpin       LATBbits.LATB3

/* Definition regarding clocks and timing */
#define _XTAL_FREQ = 8000000
#define FP 40000000
#define BAUDRATE 2400
#define BRGVAL ((FP/BAUDRATE)/16)-1
#define DELAY_105us asm volatile ("REPEAT, #4201"); Nop();

/**
    Global variables
**/
char userCommand[99];
int  userCommandPos = 0;

unsigned long long steps_to_take = 0;
unsigned long long steps_taken   = 0;
unsigned int step_delay = 10;

unsigned long long steps_from_home = 0;
unsigned char steps_from_home_direction = 0;

unsigned long long steps_from_end = 0;
unsigned char steps_from_end_direction = 3;

unsigned int blinks_every_x_cycles = 100;
unsigned int blinks_taken = 0;
unsigned int blinks_delay = 10;
unsigned int blinks_mode = 0;

// InitApp: Sets up the pins.
void InitApp(void)
{
    // UART stuff
    RCSTAbits.SPEN = 1;
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 1;
    SPBRGH = 0x3;
    SPBRG  = 0x40;

    TXSTAbits.TXEN = 1;
    RCSTAbits.CREN = 1;

    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.SCS1  = 1;

    TRISAbits.RA0 = 0;
    TRISAbits.RA2 = 1;
    TRISAbits.RA3 = 1;
    TRISAbits.RA4 = 1;
    TRISAbits.RA6 = 0;

    TRISBbits.RB2 = 0;
    TRISBbits.RB3 = 0;
    TRISBbits.RB4 = 0;

    QEICONbits.VELM = 1;
    QEICONbits.QEIM = 001;
    QEICONbits.PDEC = 00;

    ANSEL0bits.ANS0 = 0;
    ANSEL0bits.ANS1 = 0;
    ANSEL0bits.ANS2 = 0;
    ANSEL0bits.ANS3 = 0;
    ANSEL0bits.ANS4 = 0;
}

void variableDelay_mu(int delay){

}

void variableDelay_ms(int delay){

}

void doBlinks(void){

}

void doMoves(void){

}

bool shouldMove(void){

}

unsigned long long parseStringToInt(char toParse[], int startPoint){

}

void echo(char echoChar){

}

void buildCommand(char charIn){

}

int getCharArraySize(char inputArray[]){

}

void clearUserCommand(void){

}

void putch(unsigned char data){

}

void printPrompt(void){

}

void startmsg(void){

}

bool waitForKey(void){

}

void cli_clear(void){

}

void cli_help(void){

}

void cli_helpCommand(void){

}

void cli_move(){

}

void cli_scan(void){

}

void cli_direction(void){

}

void cli_setdelay(void){

}

void cli_manual(void){

}

void cli_gohome(void){

}

void cli_sethome(void){

}

void cli_goend(void){

}

void cli_setend(void){

}

void cli_status(void){

}

void cli_halt(void){

}

void doInput(void){

}

void doBackspace(void){

}

void setDefaultPinState(void){
    LEDpin = 0;
    directionpin = 0;
    stepperpulsepin = 0;
    manualpin = 0;
}

void handleUART(void){
    char holderChar = 0x0;

    if(PIR1bits.RCIF){
        holderChar = RCREG;
        if(holderChar == 13 || holderChar == 10){ // Checking for Line Feed or Carriage Return
            if(userCommandPos){
                printf("\r\n");
                doInput();
                clearUserCommand();
            }
            printPrompt();

        } else if (holderChar == 27) { // Checking for escape key
            printPrompt();
            clearUserCommand();

        } else if (holderChar == 8 || holderChar == 127){ // Checking for backspace or delete
            doBackspace();

        } else if ( (holderChar >= 'a' && holderChar <= 'z') || (holderChar >= '0' && holderChar <= '9') || (holderChar == ' ') ){ // Checking for letters, numbers, or space
            buildCommand(holderChar);

        }
    }
}

void clearQuadRegister(void){
    POSCNTL = 0;
    POSCNTH = 0;
}

unsigned long long getPosition(void){
    return (POSCNTH *256) + POSCNTL;
}

void main(void){
    InitApp();

    _delay_ms(100);

    setDefaultPinState();
    clearQuadRegister();
    clearUserCommand();

    cli_clear();
    startmsg();

    while(1){
        handleUART();
        shouldMove() ? doMoves():;
        doBlinks();
    }
}

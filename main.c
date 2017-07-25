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

long long steps_from_home = 0;

long long steps_from_end = 0;

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
    ++blinks_taken;
    if(blinks_taken == blinks_every_x_cycles){
        blinks_taken = 0;
        LEDpin = ~LEDpin;
    }
}

void doMoves(void){
    stepperpulsepin = 1;
    variableDelay_mu(step_delay);
    stepperpulsepin = 0;
    variableDelay_mu(step_delay);
}

bool shouldMove(void){
    return (steps_taken != steps_to_take);
}

unsigned long long parseStringToInt(char toParse[], int startPoint){
    unsigned long long returnInt = 0;
    int storeInt;
    unsigned int count = startPoint;
    do{
        storeInt = toParse[count];
        storeInt -= 48;
        if(storeInt >= 0 && storeInt <= 9){
            returnInt = (returnInt*10)+storeInt;
            ++count;
        }

    } while (storeInt >= 0 && storeInt <= 9);

    return returnInt;
}

void echo(char echoChar){
    while(!TXSTAbits.TRMT);
    TXREG = echoChar;
}

void buildCommand(char charIn){
    echo(charIn);
    userCommand[userCommandPos] = charIn;
    ++userCommandPos;
}

void clearUserCommand(void){
    unsigned int count = 0;
    for(count = 0; count < 99; ++count){
        userCommand[count] = 0x0;
    }
    userCommandPos = 0;
}

void putch(unsigned char data){
    while(!PIR1bits.TXIF);

    TXREG = data;
}

void printPrompt(void){
    printf("\n\rcmd> ");
}

void startmsg(void){
    printf("Spectrum Digitizer, version %s\n\rType 'help' for a list of commands.\n\r", version);
    printPrompt();
}

int toNumber(char convertThis[]){
    int count;
    int returnAmount = 0;

    for(count = 0; convertThis[count] != 0x0; ++count){
        returnAmount += (changeThis[count] % 25) * count
    }

    return returnAmount;
}

bool waitForKey(void){
    if(PIR1bits.RCIF){
        char disposable = RCREG;
        return true;
    }
    return false;
}

void cli_clear(void){
    int count = 0;
    for(count = 0; count < 100; ++count){
        printf("\n");
    }
    printf("\r");
}

void cli_help(void){
    printf("\n\r");
    printf("Spectrum Digitizer %s, Christopher Thierauf (chris@cthiearauf.com)\n\r", VERSION);
    printf("COMMMANDS\n\r");
    printf("   move     \t Move the motor a given number of steps.\n\r");
    printf("   movecm   \t Move the motor a given number of centimeters.\n\r");
    printf("   scan     \t Scan the full length of the plate.\n\r");
    printf("   scancm   \t Scan from the current position the given number of centimeters.\n\r");
    printf("   direction\t Set the default direction for this session.\n\r");
    printf("   setdelay \t Set the minimum delay between steps.\n\r");
    printf("   status   \t Get the current status of the digitizer.\n\r");
    printf("   halt     \t Stop all current actions.\n\r");
    printf("   manual   \t Allow for manual movement of slider (disengage motor)\n\r");
    printf("   gohome   \t Go to the position set as home.\n\r");
    printf("   sethome  \t Set the current position as home.\n\r");
    printf("   goend    \t Go to the position set as end.\n\r");
    printf("   setend   \t Set the current position as end.\n\r");
    printf("   clear    \t Clear the screen.\n\r");
    printf("   help     \t Display this help dialogue\n\r");
    printf("Try help (command) for more information about the given command and command usage.\n\r");
    printf("More help, documentation, and license info can be found in the manual.\n\r");
    printf("Because of a lack of space on this device, the manual is online at\n\r");
    printf("cthierauf.com/digitizer-manual\n\r");
}

void cli_helpCommand(void){
    char searchArray[20];
    unsigned char count;

    for(count = 0; count < 20; ++count){
        searchArray[count] = userCommand[count+5];
    }

    printf("Spectrum Digitizer %s, Christopher Thierauf <chris@cthierauf.com>\n\r");

    // The different commands are simplified into numbers so that they can be
    // placed into a switch statement.
    switch (toNumber(searchArray)) {
        case 43:    // goend
            printf("Usage: goend\n\r");
            printf("Go to the position remembered as 'end'. The end position has no default, set it with setend.\n\r");
            break;
        case 50:    // move
            printf("Usage: move <direction> [steps]\n\r");
            printf("Moves the motor the given number of steps in the default direction, unless a direction has been given.\n\r");
            break;
        case 53:    // help
            printf("help: Get help on different functions and abilities of this digitizer.\n\r");
            break;
        case 76:    // setend
            printf("Usage: setend\n\r");
            printf("Set the current position as the end position so that it can be referenced by other commands.\n\r");
            break;
        case 86:    // halt
            printf("help: Get help on different functions and abilities of this digitizer.\n\r");
            break;
        case 93:    // gohome
            printf("Usage: gohome\n\r");
            printf("Go to the position remembered as 'home', defaulting to the startup position.\n\r");
            break;
        case 98:    // scan
            printf("Usage: scan\n\r");
            printf("Scans from the home position to the end position, saving the digital output.\n\r");
            break;
        case 140:   // sethome
            printf("Usage: sethome\n\r");
            printf("Set the current position as home so that it can be referenced by other commands.\n\r");
            break;
        case 221:   // manual
            printf("Usage: manual\n\r");
            printf("Allow for manual movement of the slider by disengaging the motor. Hit any key to exit manual mode.\n\r");
            break;
        case 251:   // status
            printf("Usage: status\n\r");
            printf("View the current status of various settings and activities of the digitizer.\n\r");
            break;
        case 356:   // setdelay
            printf("Usage: setdelay [delay in ms]\n\r");
            printf("Sets the minimum delay between each step in milliseconds. Delays may end up slightly longer because of other commands being typed/executed.\n\r");
            break;
        case 399:   // direction
            printf("Usage: direction [right/left]\n\r");
            printf("Sets the default direction of movement and scanning.\n\r");
            break;
        default:
            printf("That command wasn't found.\n\r");

    }
}

void cli_move(int steps){
    steps_to_take = steps;
    steps_taken = 0;
}

void cli_scan(void){
    printf("Unimplemented function until the camera gets hooked up.\n\r", );
}

void cli_direction(void){
    if(userCommand[10] == 'l' || userCommand = 'L'){
        directionpin = 1;
    } else if(userCommand[10] == 'r' || userCommand[10] == 'R'){
        directionpin = 0;
    } else {
        badFormatError();
    }

    printf("Direction is %i\n\r", directionpin);
}

void cli_setdelay(void){
    step_delay = parseStringToInt(userCommand, 9);
}

void cli_manual(void){
    printf("\n\rManual mode active. Move slider manually, hit any key to exit this mode.\n\r");
    manualpin = 1;
    while(!waitForKey());

    manualpin = 0;
}

void cli_gohome(void){
    if(steps_from_home > 0){
        setSteps(steps_from_home);
    } else if(steps_from_home < 0){
        direction ~= direction
        setSteps(steps_from_home * -1);
    }
}

void cli_sethome(void){
    steps_from_home = 0;
}

void cli_goend(void){
    if(steps_from_end > 0){
        setSteps(steps_from_end);
    } else if(steps_from_end < 0){
        direction ~= direction
        setSteps(steps_from_end * -1);
    }
}

void cli_setend(void){
    steps_from_end = 0;
}

void cli_status(void){
    printf("Version:    %s\n\r", version);
    printf("Steps:      %u of %u\n\r", steps_taken, steps_to_take);
    printf("Direction:  %u\n\r", directionpin);
}

void cli_halt(void){
    printf("Stopping any movement... ");
    steps_to_take   = 0;
    steps_taken     = 0;
    stepperpulsepin = 0;
    printf("Stopped. \r\n");
    blinks_mode     = 0;
    printf("Entering manual mode. \r\n");
    cli_manual();
}

void doInput(void){
    switch (toNumber(userCommand)) {
        case 43:    // goend
            cli_goend();
            break;
        case 50:    // move
            cli_move();
            break;
        case 53:    // help
            if( userCommandPos > 4 ){
                cli_helpCommand();
            } else {
                cli_help();
            }
            break;
        case 76:    // setend
            cli_setend();
            break;
        case 86:    // halt
            cli_halt();
            break;
        case 93:    // gohome
            cli_gohome();
            break;
        case 98:    // scan
            cli_setdelay();
            break;
        case 140:   // sethome
            cli_sethome();
            break;
        case 221:   // manual
            cli_manual();
            break;
        case 251:   // status
            cli_status();
            break;
        case 356:   // setdelay
            setdelay();
            break;
        case 399:   // direction
            cli_direction();
            break;
        default:
            printf("That command wasn't found.\n\r");
    }
}

void doBackspace(void){
    if(userCommandPos > 0){
        userCommand[userCommandPos] = 127;
        --userCommandPos;
        printf("\b \b");
    }
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
                printf("\n\r");
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

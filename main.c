/*
 *    Spectral Digitizer: Code for the spectral digitzer project
 *    Copyright (C) 2017  Chris Thierauf <chris@cthierauf.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Version 3: Baud Rate and Clock changes
char version[] = {"v3.0 (beta) 7-August-2017"};

// Includes
#include <xc.h>             // For hardware-specific stuff
#include "stdio.h"          // stdio
#include "stdlib.h"         // stdlib
#include "stdbool.h"        // Booleans
#include "stdint.h"         // Gives the uint8_t stuff
#include "string.h"         // Strings
#include "htc.h"            // Gives access to delays (?)
// TODO- test removing ^. With ct_delay_*, is this needed?

// Setup bits
// PIC18F2431 Configuration Bit Settings
// 'C' source line config statements
// CONFIG1H
#pragma config OSC = HSPLL      // Oscillator Selection bits (HS oscillator, PLL enabled (clock frequency = 4 x FOSC1))
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
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (001000-001FFF)  not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (002000-002FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (003000-003FFFh) not code-protected)
// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)
// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000200-000FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (001000-001FFF)  not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (002000-002FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (003000-003FFFh) not write-protected)
// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000200-000FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (001000-001FFF)  not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (002000-002FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (003000-003FFFh) not protected from table reads executed in other blocks)
// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from table reads executed in other blocks)

/* Pin definitions, for keeping things easier later */
#define LEDpin          LATAbits.LATA0 // The pin that the LED can be found on
#define directionpin    LATBbits.LATB4 // The pin that controls the stepper direction
#define stepperpulsepin LATBbits.LATB2 // The pin that gives pulses to the stepper
#define manualpin       LATBbits.LATB3 // The pin that controls the manual mode

/* Defining the ratio of quadrature pulses to centimeters */
#define quadPulses_in_cm 100 // Defined because it's set in the spec sheet
int  stepperPulses_in_cm = 1;   // Variable so that it can be used in control loop

/* Definition regarding clocks and timing */
#define _XTAL_FREQ = 40000000
#define FP 40 // Is this MIPS?
#define BAUDRATE 9600
#define BRGVAL ((FP/BAUDRATE)/16)-1
#define DELAY_105us asm volatile ("REPEAT, #4201"); Nop();

/* Defining my own delays since the almost identical built-in ones don't work */
#define ct_delay_ms(x) _delay((x)*(2))
#define ct_delay_us(x) _delay((x)*(2000))

/*
 * Global variables
 */
unsigned char userCommand[20];      // The user's characters, as they're being typed
unsigned int  userCommandPos = 0;   // The position in the userCommand array

/*
 * userCommandTotal is a pointer to a character array, where each point in the
 * array is used to store a different part of the command. [0] holds the command,
 * [1] holds the first argument, and so on.
 */
unsigned char *userCommandTotal[20];
unsigned char userCommandTotalPos = 0;

unsigned long long steps_to_take = 0;   // The number of steps that need to be taken
unsigned long long steps_taken   = 0;   // The number of steps that have actually been taken
unsigned int step_delay = 10;           // Desired minimum delay between pulses- here while testing only

unsigned long long pulses_to_get = 0;   // Number of pulses expected to receive (quadrature)
long long steps_from_home = 0;          // The number of steps away from the home position
long long steps_from_end = 0;           // The number of steps away from the end position

// Blinking
unsigned int blinks_every_x_cycles = 5000;
unsigned int blinks_taken = 0;

// InitApp: Sets up the pins.
void InitApp(void)
{
    // TODO- go through and finally label all of these.

    //CONFIG1Hbits.FOSC = 0110;
    // UART stuff
    RCSTAbits.SPEN = 1;     // Serial Port Enable bit (enabled)
    TRISCbits.RC6 = 1;      // Enabling RC6 so it can be used in UART
    TRISCbits.RC7 = 1;      // Enabling RC7 so it can be used in UART
    TXSTAbits.SYNC = 0;     // EUSART Mode Select (synchronous)
    TXSTAbits.BRGH = 0;     // High Baud Rate Select (low speed)
    BAUDCONbits.BRG16 = 0;  // 16-Bit Baud Rate Register Enable (0 means only SPBRG used)

    // Baud Rate Generator
    // Aiming for 64, SPBRGH ignored b/c BRG16 is 0
    //SPBRGH = ;
    SPBRG  = 01000000;


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

// TODO- finish this
void variableDelay_us(int delay){
    switch(delay){
        case 0:
            break;
        case 1:
            ct_delay_us(1);
            break;
        case 2:
            ct_delay_us(2);
            break;
        case 3:
            ct_delay_us(3);
            break;
        case 4:
            ct_delay_us(4);
            break;
        case 5:
            ct_delay_us(5);
            break;
        case 6:
            ct_delay_us(6);
            break;
        case 7:
            ct_delay_us(7);
            break;
        case 8:
            ct_delay_us(8);
            break;
        case 9:
            ct_delay_us(9);
            break;
    }
}

//TODO- Also finish this
/*
void variableDelay_ms(int delay){

}
 * */

void doBlinks(void){
    ++blinks_taken;
    if(blinks_taken == blinks_every_x_cycles){
        blinks_taken = 0;
        LEDpin = ~LEDpin;
    }
}

void setMovement(unsigned long long steps){
    steps_to_take = steps;
    steps_taken = 0;
}

void doMoves(void){
    stepperpulsepin = 1;
    variableDelay_us(step_delay);
    stepperpulsepin = 0;
    variableDelay_us(step_delay);
}

bool shouldMove(void){
    return (steps_taken != steps_to_take);
}

unsigned long long parseStringToInt(char toParse[], unsigned int startPoint){
    unsigned long long returnInt = 0;
    unsigned int storeInt;
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

void clearUserCommand(void){
    unsigned int count = 0;
    for(count = 0; count <= 20; ++count){
        userCommand[count] = 0x0;
    }
    userCommandPos = 0;
}

void clearUserCommandTotal(void){
    unsigned int count = 0;
    clearUserCommand();

    for(count = 0; count <= 20; ++count){
        userCommand[count] = userCommand;
    }
    userCommandTotalPos = 0;
}

void buildCommand(unsigned char charIn){
    echo(charIn);

    if(charIn == ' '){

        if(userCommandTotal[userCommandTotalPos] != '\0'){
            ++userCommandTotalPos;
        }

        userCommandTotal[userCommandTotalPos] = userCommand;
        clearUserCommand();

    } else {
        userCommand[userCommandPos] = charIn;
        ++userCommandPos;

    }
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
        returnAmount += (convertThis[count] % 25) * count;

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

void badFormatError(void){
    printf("\n\rThe format of that command was not recognized.");
    printf("\n\rPlease try a different command, or use 'help' for help.\n\r");

}

void setSteps(unsigned long long steps){
    steps_to_take = steps;

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
    printf("Spectrum Digitizer %s, Christopher Thierauf (chris@cthiearauf.com)\n\r", version);
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
    printf("   license  \t Print license information. ");
    printf("   help     \t Display this help dialogue\n\r");
    printf("Try help (command) for more information about the given command and command usage.\n\r");
    printf("More help, documentation, and license info can be found in the manual.\n\r");
    printf("Because of a lack of space on this device, the manual is online at\n\r");
    printf("cthierauf.com/digitizer-manual\n\r");

}

void cli_license(void){
    printf("Spectral Digitizer: Code for the spectral digitzer project\n\r");
    printf("Copyright (C) 2017  Chris Thierauf <chris@cthierauf.com>\n\r");
    printf("\n\r");
    printf("This program is free software: you can redistribute it and/or modify\n\r");
    printf("it under the terms of the GNU General Public License as published by\n\r");
    printf("the Free Software Foundation, either version 3 of the License, or\n\r");
    printf("(at your option) any later version.\n\r");
    printf("\n\r");
    printf("This program is distributed in the hope that it will be useful,\n\r");
    printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n\r");
    printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\r");
    printf("GNU General Public License for more details.\n\r");
    printf("\n\r");
    printf("You should have received a copy of the GNU General Public License\n\r");
    printf("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\r");

}


#define goend       43
#define move        50
#define help        53
#define quad        61
#define setend      76
#define halt        86
#define gohome      93
#define scan        98
#define sethome     140
#define manual      221
#define status      251
#define license     282
#define setdelay    356
#define direction   399

void cli_helpCommand(void){
    printf("Spectrum Digitizer %s, Christopher Thierauf <chris@cthierauf.com>\n\r", version);

    // The different commands are simplified into numbers so that they can be
    // placed into a switch statement.
    switch (toNumber(userCommandTotal[0])) {
        case goend:
            printf("Usage: goend\n\r");
            printf("Go to the position remembered as 'end'. The end position has no default, set it with setend.\n\r");
            break;
        case move:
            printf("Usage: move <direction> [steps]\n\r");
            printf("Moves the motor the given number of steps in the default direction, unless a direction has been given.\n\r");
            break;
        case help:
            printf("help: Get help on different functions and abilities of this digitizer.\n\r");
            break;
        case quad:
            printf("quad: Display quadrature information, hit any key to stop.");
            break;
        case setend:
            printf("Usage: setend\n\r");
            printf("Set the current position as the end position so that it can be referenced by other commands.\n\r");
            break;
        case halt:
            printf("help: Get help on different functions and abilities of this digitizer.\n\r");
            break;
        case gohome:
            printf("Usage: gohome\n\r");
            printf("Go to the position remembered as 'home', defaulting to the startup position.\n\r");
            break;
        case scan:
            printf("Usage: scan\n\r");
            printf("Scans from the home position to the end position, saving the digital output.\n\r");
            break;
        case sethome:
            printf("Usage: sethome\n\r");
            printf("Set the current position as home so that it can be referenced by other commands.\n\r");
            break;
        case manual:
            printf("Usage: manual\n\r");
            printf("Allow for manual movement of the slider by disengaging the motor. Hit any key to exit manual mode.\n\r");
            break;
        case status:
            printf("Usage: status\n\r");
            printf("View the current status of various settings and activities of the digitizer.\n\r");
            break;
        case setdelay:
            printf("Usage: setdelay [delay in ms]\n\r");
            printf("Sets the minimum delay between each step in milliseconds. Delays may end up slightly longer because of other commands being typed/executed.\n\r");
            break;
        case direction:
            printf("Usage: direction [right/left]\n\r");
            printf("Sets the default direction of movement and scanning.\n\r");
            break;
        default:
            printf("That command wasn't found.\n\r");

    }
}


#define toNumberLeft  53
#define toNumberRight 87

void cli_move(void){
    if(toNumber(userCommandTotal[1]) == toNumberLeft){
        directionpin = 1;
        printf("Starting movement in left direction.\r\n");

    } else if(toNumber(userCommandTotal[1]) == toNumberRight){
        directionpin = 0;
        setMovement(parseStringToInt(userCommand, 11));
        printf("Starting movement in the right direction.\r\n");

    } else {
        badFormatError();
    }

    //TODO- add checks to make sure the parser is getting a number
    setMovement(parseStringToInt(userCommandTotal[2], 0));

}

void cli_movecm(void){
    if(toNumber(userCommandTotal[1]) == toNumberLeft){
        directionpin = 1;
        printf("Setting movement in left direction.\r\n");

    } else if(toNumber(userCommandTotal[1]) == toNumberRight){
        directionpin = 0;
        printf("Setting movement in the right direction.\r\n");

    } else {
        badFormatError();

    }

    //TODO- add checks to make sure the parser is getting a number
    //setMovementCM(parseStringToInt(userCommandTotal[2], 0));

}

void cli_scan(void){
    printf("Unimplemented function until the camera gets hooked up.\n\r");
}

void cli_direction(void){
    if(toNumber(userCommandTotal[1]) == toNumberLeft){
        directionpin = 1;
    } else if(toNumber(userCommandTotal[1]) == toNumberRight){
        directionpin = 0;
    } else {
        badFormatError();
    }

    printf("Direction pin is %i", directionpin);
    (directionpin) ? printf("left"): printf("right");

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
        ct_delay_ms(10);
        directionpin = !directionpin;
        setSteps(steps_from_home * -1);
    } // If it isn't greater than, or less than 0, it is 0 and nothing needs to be done

}

// TODO- use quadrature, not steps, for greater accuracy
void cli_sethome(void){
    steps_from_home = 0;
}

void cli_goend(void){
    if(steps_from_end > 0){
        setSteps(steps_from_end);
    } else if(steps_from_end < 0){
        ct_delay_ms(10);
        directionpin = !directionpin;
        setSteps(steps_from_end * -1);
    }
}

// TODO- use quadrature, not steps, for greater accuracy
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
    printf("Entering manual mode. \r\n");
    cli_manual();
}

int cli_quad(void){
    while(1){
        printf("Displaying quadrature reading, press any key to stop: \r\n");
        printf("%u\r", getPosition());
        ct_delay_ms(10);
        if(waitForKey()) { return 1; }
    }
}

void doInput(void){
    switch (toNumber(userCommandTotal[0])) {
        case goend:
            cli_goend();
            break;
        case move:
            cli_move();
            break;
        case help:
            if( userCommandTotalPos > 0 ){
                cli_helpCommand();
            } else {
                cli_help();
            }
            break;
        case quad:
            cli_quad();
            break;
        case setend:
            cli_setend();
            break;
        case halt:
            cli_halt();
            break;
        case gohome:
            cli_gohome();
            break;
        case scan:
            cli_setdelay();
            break;
        case sethome:
            cli_sethome();
            break;
        case manual:
            cli_manual();
            break;
        case status:
            cli_status();
            break;
        case license:
            cli_license();
            break;
        case setdelay:
            cli_setdelay();
            break;
        case direction:
            cli_direction();
            break;
        default:
            printf("That command wasn't found.\n\r");
    }
}

void doBackspace(void){
    if(userCommandPos > 0){
        userCommand[userCommandPos] = (unsigned char) 0x0;
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
                clearUserCommandTotal();
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

int getPosition(void){
    return (POSCNTH *256) + POSCNTL;
}

void handlecm(void){
    // Put some form of PID here?
}

void main(void){
    InitApp();

    ct_delay_ms(100);

    setDefaultPinState();
    clearQuadRegister();
    clearUserCommandTotal();

    cli_clear();
    startmsg();

    while(1){
        handleUART();
        if(shouldMove()){ doMoves(); }
        doBlinks();
        handlecm();
    }
}

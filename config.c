// General includes
#include <xc.h>             // For hardware-specific stuff
#include "stdio.h"          // stdio
#include "stdlib.h"         // stdlib
#include "stdbool.h"        // Booleans
#include "stdint.h"         // Gives the uint8_t stuff
#include "string.h"         // Strings

// My includes
#include "globals.h"
#include "defines.h"
#include "pragmas.h"
#include "commandline.h"
#include "input.h"
#include "get_and_set.h"

void InitApp(void)
{
    // TODO- go through and finally label all of these.

    //CONFIG1Hbits.FOSC = 0110;
    // UART stuff
    RCSTAbits.SPEN = 1;     // Serial Port Enable bit (enabled)
    TRISCbits.RC6 = 1;      // Enabling RC6 so it can be used in UART
    TRISCbits.RC7 = 1;      // Enabling RC7 so it can be used in UART
    TXSTAbits.SYNC = 0;     // EUSART Mode Select (asynchronous)
    TXSTAbits.BRGH = 0;     // High Baud Rate Select (0 = low speed)
    BAUDCONbits.BRG16 = 0;  // 16-Bit Baud Rate Register Enable (0 means only SPBRG used)

    // Baud Rate Generator
    //SPBRGH = ;
    SPBRG  = 64;


    TXSTAbits.TXEN = 1; // Transmit Enable (enabled))
    RCSTAbits.CREN = 1; // Continuous Receive Enable bit (enabled))

    // Clock Settings
    OSCCONbits.IRCF2 = 1; // Internal Oscillator Frequency Select bits
    OSCCONbits.IRCF1 = 1; // Internal Oscillator Frequency Select bits
    OSCCONbits.IRCF0 = 1; // Internal Oscillator Frequency Select bits
    OSCCONbits.SCS1  = 1; // System Clock Select bits (should this be 0?)

    // Setting pins like the LED
    TRISAbits.RA0 = 0;
    TRISAbits.RA2 = 1;
    TRISAbits.RA3 = 1;
    TRISAbits.RA4 = 1;
    TRISAbits.RA6 = 0;

    TRISBbits.RB2 = 0;
    TRISBbits.RB3 = 0;
    TRISBbits.RB4 = 0;

    QEICONbits.VELM = 1;    // Velocity Mode (disabled)
    QEICONbits.QEIM = 001;  // QEI Mode (QEI enabled in 2x mode, INDX resets counter)
    QEICONbits.PDEC = 00;   // Velocity Pulse Reduction (1:1)

    // Analog Input Function Select Bits, 0 is Digital I/O
    ANSEL0bits.ANS0 = 0;
    ANSEL0bits.ANS1 = 0;
    ANSEL0bits.ANS2 = 0;
    ANSEL0bits.ANS3 = 0;
    ANSEL0bits.ANS4 = 0;
}

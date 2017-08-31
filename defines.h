#ifndef defines_defined
#define defines_defined

    /* Pin definitions, for keeping things easier later */
    #define LEDpin          LATAbits.LATA0 // The pin that the LED can be found on
    #define directionpin    LATBbits.LATB4 // The pin that controls the stepper direction
    #define stepperpulsepin LATBbits.LATB2 // The pin that gives pulses to the stepper
    #define manualpin       LATBbits.LATB3 // The pin that controls the manual mode
    #define handshake_in
    #define handshake_out

    /* Defining the ratio of quadrature pulses to centimeters */
    #define quadPulses_per_cm 100 // Defined because it's set in the spec sheet

    /* Definition regarding clocks and timing */
    #define _XTAL_FREQ = 40000000
    #define FP (_XTAL_FREQ*5)
    #define BAUDRATE 9600
    #define BRGVAL ((FP/BAUDRATE)/16)-1
    #define DELAY_105us asm volatile ("REPEAT, #4201"); Nop();

    /* Defining my own delays since the almost identical built-in ones don't work */
    #define ct_delay_ms(x) _delay((x)*(2))
    #define ct_delay_us(x) _delay((x)*(2000))

    #define MAX_COMMAND_SIZE 50
    #define MAX_ARGS 5

    #define step_delay 10           // Desired minimum delay between pulses- here while testing only
    #define blinks_every_x_cycles 5000

    #define goend       43
    #define move        50
    #define movecm      191
    #define help        53
    #define quad        61
    #define setend      76
    #define halt        86
    #define gohome      93
    #define scan        98
    #define clear       132
    #define sethome     140
    #define manual      221
    #define status      251
    #define license     177
    #define direction   399

    #define toNumberLeft  53
    #define toNumberRight 87
    #define toNumberSafemode  127

#endif
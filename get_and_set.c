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
#include "config.h"
#include "commandline.h"
#include "input.h"


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

void setMovementCM(unsigned long long cm){
    steps_to_take = cm * quadPulses_per_cm;
    steps_taken = 0;
}

double getPosition(void){
    return ((POSCNTH *256) + POSCNTL) / (double) quadPulses_per_cm;
}

void doMoves(void){
    stepperpulsepin = 1;
    ct_delay_us(step_delay);
    stepperpulsepin = 0;
    ct_delay_us(step_delay);
}

bool shouldMove(void){
    return (unsigned) (steps_taken != steps_to_take);
}

void clearQuadRegister(void){
    POSCNTL = 0;
    POSCNTH = 0;
}

void setSteps(unsigned long long steps){
    steps_to_take = steps;

}

void setDefaultPinState(void){
    LEDpin = 0;
    directionpin = 0;
    stepperpulsepin = 0;
    manualpin = 0;

}

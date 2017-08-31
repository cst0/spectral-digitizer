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
#include "input.h"
#include "get_and_set.h"

void do_handshake(void) {
    handshake_out = 0; // stepper output is low
    while(handshake_in == 0); // waiting for camera to go high (to acknowledge)

    handshake_out = 1; // Setting output to high
    while(handshake_in == 1); // waiting for camera to go low (to acknowledge)

    // Ready to do steps, so exit here

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
    printf("   direction\t Get the default direction for this session.\n\r");
    printf("   status   \t Get the current status of the digitizer.\n\r");
    printf("   halt     \t Stop all current actions.\n\r");
    printf("   manual   \t Allow for manual movement of slider (disengage motor)\n\r");
    printf("   gohome   \t Go to the position set as home.\n\r");
    printf("   sethome  \t Set the current position as home.\n\r");
    printf("   goend    \t Go to the position set as end.\n\r");
    printf("   setend   \t Set the current position as end.\n\r");
    printf("   clear    \t Clear the screen.\n\r");
    printf("   license  \t Print GPL license information.\n\r");
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

void cli_helpCommand(void){
    printf("Spectrum Digitizer %s, Christopher Thierauf <chris@cthierauf.com>\n\r", version);

    // The different commands are simplified into numbers so that they can be
    // placed into a switch statement.
    seperate_strings(user_command);
    char command[MAX_COMMAND_SIZE];
    tmparray_from_to_space(user_command, arg_positions[1]);
    strcpy(command, tmparray);

    switch (toNumber(command)) {
        case 0:
            cli_help();
            break;
        case goend:
            printf("Usage: goend\n\r");
            printf("Go to the position remembered as 'end'. The end position has no default, set it with setend.\n\r");
            break;

        case move:
            printf("Usage: move <direction> [steps] <safemode>\n\r");
            printf("Moves the motor the given number of steps in the default direction, unless a direction has been given.\n\r");
            printf("If 'safemode' is present, user can type 'halt' or other commands while moving. \n\r");
            break;

        case movecm:
            printf("Usage: movecm <direction> [cm] <safemode>\n\r");
            printf("Moves the motor the given number of centimeters in the default direction, unless a direction has been given.\n\r");
            printf("If 'safemode' is present, user can type 'halt' or other commands while moving. \n\r");
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

        case direction:
            printf("Usage: direction [right/left]\n\r");
            printf("Sets the default direction of movement and scanning.\n\r");
            break;

        default:
            printf("That command wasn't found. Here's the whole list: \n\r");
            cli_help();
    }
}


int cli_move(void){

    // This doesn't check if no direction is given, making direction command useless
    char direction_string[5];
    tmparray_from_to_space(user_command, arg_positions[1]);
    strcpy(direction_string, tmparray);
    int dir = 0;
    dir = toNumber(*direction_string);
    printf("[tmparray: %s] [arg_positions[1]: %i] [direction_string: %s] [dir: %i]", tmparray, arg_positions[1], direction_string, dir);
    if(dir == toNumberLeft) {
        directionpin = 1;
    } else if (dir == toNumberRight) {
        directionpin = 0;
    } else {
        badFormatError();
        return -1;
    }

    unsigned long long steps;
    tmparray_from_to_space(user_command, arg_positions[2]);
    steps = parseStringToInt(tmparray, 0);

    tmparray_from_to_space(user_command, arg_positions[3]);
    if(toNumber(tmparray) == 0){
        printf("The program is currently moving in locked mode because 'safe' was not specified in the command. \r\n");
        printf("This mode is faster and more precise, but cannot be halted from the command line.\r\n");

        printf("Starting movement in the ");
        (dir == toNumberLeft) ? printf("left "): printf("right ");
        printf("direction.\r\n");
        setMovement(steps);
        while(steps_taken != steps_to_take) {
            stepperpulsepin = 0;
            ct_delay_us(step_delay);
            stepperpulsepin = 1;
            ct_delay_us(step_delay);
            ++steps_taken;
            doBlinks();
        }
        steps_taken = 0;
        steps_to_take = 0;
        printf("Movement Complete! \r\n");

    } else if(toNumber(tmparray) == toNumberSafemode){
        setMovement(steps);
        printf("Starting movement in the ");
        (dir == toNumberLeft) ? printf("left "): printf("right ");
        printf("direction.\r\n");

    } else {
        badFormatError();
    }

    return 0;

}

int cli_movecm(void){

    // This doesn't check if no direction is given, making direction command useless
    char direction_string[];
    tmparray_from_to_space(user_command, arg_positions[1]);
    strcpy(direction_string, tmparray);
    int dir = 0;
    dir = toNumber(*direction_string);

    if(dir == toNumberLeft) {
        directionpin = 1;
    } else if (dir == toNumberRight) {
        directionpin = 0;
    } else {
        badFormatError();
        return -1;
    }

    unsigned long long cm;
    tmparray_from_to_space(user_command, arg_positions[2]);
    cm = parseStringToInt(tmparray, 0);

    tmparray_from_to_space(user_command, arg_positions[3]);
    if(toNumber(tmparray) == 0){
        printf("The program is currently moving in locked mode because 'safe' was not specified in the command. \r\n");
        printf("This mode is faster and more precise, but cannot be halted from the command line.\r\n");

        printf("Starting movement in the ");
        (dir == toNumberLeft) ? printf("left "): printf("right ");
        printf("direction.\r\n");
        clearQuadRegister();
        while(cm != (getPosition())){
            stepperpulsepin = 0;
            ct_delay_us(step_delay);
            stepperpulsepin = 1;
            ct_delay_us(step_delay);
            if(cm < getPosition()){
                directionpin = ~directionpin;
            }
            doBlinks();
        }
        steps_taken = 0;
        steps_to_take = 0;
        printf("Movement Complete! \r\n");

    } else if(toNumber(tmparray) == toNumberSafemode){
        setMovementCM(cm);
        printf("Starting movement in the ");
        (dir == toNumberLeft) ? printf("left "): printf("right ");
        printf("direction.\r\n");

    } else {
        badFormatError();
    }

    return 0;

}


void cli_scan(void){
    printf("Unimplemented function until the camera gets hooked up.\n\r");
}

void cli_scancm(void) {

        // This doesn't check if no direction is given, making direction command useless
        char direction_string[];
        tmparray_from_to_space(user_command, arg_positions[1]);
        strcpy(direction_string, tmparray);
        int dir = 0;
        dir = toNumber(*direction_string);

        if(dir == toNumberLeft) {
            directionpin = 1;
        } else if (dir == toNumberRight) {
            directionpin = 0;
        } else {
            badFormatError();
            return -1;
        }

        unsigned long long cm;
        tmparray_from_to_space(user_command, arg_positions[2]);
        cm = parseStringToInt(tmparray, 0);

        tmparray_from_to_space(user_command, arg_positions[3]);
        if(toNumber(tmparray) == 0){
            printf("The program is currently scanning in locked mode because 'safe' was not specified in the command. \r\n");
            printf("This mode is faster and more precise, but cannot be halted from the command line.\r\n");

            printf("Starting movement in the ");
            (dir == toNumberLeft) ? printf("left "): printf("right ");
            printf("direction.\r\n");
            clearQuadRegister();
            while(cm != (getPosition())){
                do_handshake();

                stepperpulsepin = 0;
                ct_delay_us(step_delay);
                stepperpulsepin = 1;
                ct_delay_us(step_delay);
                if(cm < getPosition()){
                    directionpin = ~directionpin;
                }
                doBlinks();
            }
            steps_taken = 0;
            steps_to_take = 0;
            printf("Movement Complete! \r\n");

        } else if(toNumber(tmparray) == toNumberSafemode){
            setMovementCM(cm);
            printf("Starting movement in the ");
            (dir == toNumberLeft) ? printf("left "): printf("right ");
            printf("direction.\r\n");

            do_handshake();

        } else {
            badFormatError();
        }

        return 0;

}

//TODO- userCommand doesn't exist anymore
void cli_direction(void){
    tmparray_from_to_space(user_command, arg_positions[1]);
    int dir;
    dir = toNumber(tmparray);

    if(dir == toNumberLeft){
        directionpin = 1;
    } else if(dir == toNumberRight){
        directionpin = 0;
    } else if(dir == 0){
    } else {
        badFormatError();
    }

    printf("Direction pin is %i: ", directionpin);
        (directionpin) ? printf("left"): printf("right");

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
    printf("Steps:      %i of %i\n\r", steps_taken, steps_to_take);
    printf("Direction:  %u\n\r", directionpin);
}

void cli_halt(void){
    printf("Stopping any movement... ");
    steps_to_take   = 0;
    steps_taken     = 0;
    stepperpulsepin = 0;
    printf("Stopped. \r\n");
    printf("Clearing quad register and some arrays... ");
    clearQuadRegister();
    clear_tmparray();
    clear_user_command();
    printf("Stopped.\r\n");
    printf("Entering manual mode. \r\n");
    cli_manual();
}

int cli_quad(void){
    printf("Displaying quadrature reading, press any key to stop: \r\n");

    while(1){
        printf("%u \r", (unsigned int) getPosition());
        ct_delay_ms(10);

        if(waitForKey()) { return 1; }
    }
    return -1; // If this gets returned, everything is horribly wrong
}

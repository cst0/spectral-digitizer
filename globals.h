#ifndef globals_defined
#define globals_defined

    #include "defines.h"
    
    // Version 4: Better file format
    // Revision 0: Tweaked input, split things into headers for conciseness
    char version[] = {"v3.2 (beta) 7-August-2017"};

    /* Defining the ratio of quadrature pulses to centimeters */
    int  stepperPulses_in_cm = 1;   // Variable so that it can be used in control loop

    unsigned char user_command[MAX_COMMAND_SIZE];  // The user's characters, as they're being typed
    unsigned int user_command_pos = 0;    // The position in the userCommand array
    unsigned char tmparray[MAX_COMMAND_SIZE];

    int arg_positions[MAX_ARGS];                // The position of each argument

    unsigned long long steps_to_take = 0;   // The number of steps that need to be taken
    unsigned long long steps_taken   = 0;   // The number of steps that have actually been taken

    unsigned long long pulses_to_get = 0;   // Number of pulses expected to receive (quadrature)
    unsigned long long steps_from_home = 0;          // The number of steps away from the home position
    unsigned long long steps_from_end = 0;           // The number of steps away from the end position

    // Blinking
    unsigned int blinks_taken = 0;

#endif
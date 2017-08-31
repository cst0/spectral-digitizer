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
#include "get_and_set.h"


void clear_tmparray(void){
    memcpy(tmparray, '\0', MAX_COMMAND_SIZE);
    //tmparray[0] = '\0';
}

//TODO- these should be returning, not printing, their output
void tmparray_from_to_space(char *print, int from) {
    clear_tmparray();
    int count = 0;
    for (char *tmp = print + from;
        (*tmp) && ( *(tmp) != ' ' ) ;
        ++tmp) {
            tmparray[count] = *tmp;
            count++;
    }
    tmparray[count] = '\0';
}

unsigned long long parseStringToInt(char toParse[], unsigned int startPoint){
    unsigned long long returnInt = 0;
    unsigned int storeInt;
    unsigned int count = startPoint;
    do{
        storeInt = toParse[count];
        storeInt -= 48;
        if(/*storeInt >= ) && */ storeInt <= 9){ // It's unsigned, no need to check for less than 0?
            returnInt = (returnInt*10)+storeInt;
            ++count;
        }

    } while (/*storeInt >= 0 &&*/ storeInt <= 9);

    return returnInt;
}

void echo(char echoChar){
    while(!TXSTAbits.TRMT);
    TXREG = echoChar;
}

//TODO- userCommand doesn't exist anymore
void clear_user_command(void){
    user_command_pos = 0;
    memcpy(user_command, ' ', MAX_COMMAND_SIZE);
    // TODO- get arg positions filled with 0's
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

void seperate_strings(char *user_command) {
    int string_position = 0;
    int arg_position_count = 0;
    char *tmp = user_command;
    for(; *tmp; ++tmp){
        if(*tmp == ' '){
            ++string_position;
        } else {
            if((string_position > 0) && *(tmp-1) == ' ') {
                arg_positions[arg_position_count] = string_position;
                ++string_position;
                ++arg_position_count;
            } else {
                ++string_position;
            }
        }
    }
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

void doInput(char *command) {
    printf("toNumber is %i, command is %s\n\r", toNumber(command), command);
    switch (toNumber(command)) {
        case goend:
            cli_goend();
            break;
        case move:
            cli_move();
            break;
        case movecm:
            cli_movecm();
            break;
        case help:
            cli_helpCommand();
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
            cli_scan();
            break;
        case sethome:
            cli_sethome();
            break;
        case clear:
            cli_clear();
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
        case direction:
            cli_direction();
            break;
        default:
            printf("\n\rThat command wasn't found.\n\r");
    }
}

void doBackspace(void){
    if(user_command_pos > 0){
        user_command[user_command_pos] = (unsigned char) 0x0;
        --user_command_pos;
        printf("\b \b");

    }
}


void handleUART(void){
    char holderChar = 0x0;

    if(PIR1bits.RCIF){
        holderChar = RCREG;
        if(holderChar == 13 || holderChar == 10){ // Checking for Line Feed or Carriage Return
            if(user_command_pos){
                printf("\n\r");
                tmparray_from_to_space(user_command, 0);
                char command[MAX_COMMAND_SIZE];
                strcpy(command, tmparray);
                doInput(command);
                clear_user_command();
            }
            printPrompt();

        } else if (holderChar == 27) { // Checking for escape key
            printPrompt();
            clear_user_command();

        } else if (holderChar == 8 || holderChar == 127){ // Checking for backspace or delete
            doBackspace();

        } else /* if ( (holderChar >= 'a' && holderChar <= 'z') || (holderChar >= '0' && holderChar <= '9')) */{ // Checking for letters, numbers, or space
            echo(holderChar);
            user_command[user_command_pos] = holderChar;
            ++user_command_pos;
            user_command[user_command_pos] = '\0';
        }
    }
}

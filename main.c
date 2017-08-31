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
 * 
 *       Version 4: Better file format
 *       Revision 0: Tweaked input, split things into headers for conciseness
 */

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
#include "get_and_set.h"

void main(void){
    InitApp();

    ct_delay_ms(100);

    setDefaultPinState();
    clearQuadRegister();
    clear_user_command();

    cli_clear();
    startmsg();

    while(1){
        handleUART();
        if(shouldMove()){ doMoves(); }
        doBlinks();
    }
}

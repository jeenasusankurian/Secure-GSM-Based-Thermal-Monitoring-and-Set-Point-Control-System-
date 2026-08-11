//KeyPd.h
#ifndef _KEYPD_H_
#define _KEYPD_H_

#include "types.h"

// Function Prototypes
void KeyPdInit(void);   // Initialize keypad
u8   ColStat(void);     // Check if any key is pressed
u8   KeyVal(void);      // Get pressed key value

#endif



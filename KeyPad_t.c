//KeyPad.c                           // Source file containing 4x4 keypad driver functions

#include <LPC21xx.h>                 // LPC21xx microcontroller register definitions
#include "KeyPd_t.h"                 // Header file containing keypad function declarations
#include "types_t.h"                 // Header file containing user-defined data types (u8, u16, etc.)

// Row pins (P1.16–P1.19)
#define R0 16                        // Row 0 is connected to Port 1 Pin 16
#define R1 17                        // Row 1 is connected to Port 1 Pin 17
#define R2 18                        // Row 2 is connected to Port 1 Pin 18
#define R3 19                        // Row 3 is connected to Port 1 Pin 19

// Column pins (P1.20–P1.23)
#define C0 20                        // Column 0 is connected to Port 1 Pin 20
#define C1 21                        // Column 1 is connected to Port 1 Pin 21
#define C2 22                        // Column 2 is connected to Port 1 Pin 22
#define C3 23                        // Column 3 is connected to Port 1 Pin 23

// Keypad lookup table
// Stores the character corresponding to each row and column position
const u8 LUT[4][4] =
{
    {'1','2','3','A'},               // Row 0
    {'4','5','6','B'},               // Row 1
    {'7','8','9','C'},               // Row 2
    {'*','0','#','D'}                // Row 3
};

// Function to initialize the keypad
void KeyPdInit(void)
{
    // Configure rows as output
    IODIR1 |= (1<<R0)|(1<<R1)|(1<<R2)|(1<<R3);   // Set row pins (P1.16–P1.19) as output pins

    // Configure columns as input (default input, no need to set)
    // Column pins remain input because GPIO pins are input by default

    // Initialize rows LOW
    IOCLR1 = (1<<R0)|(1<<R1)|(1<<R2)|(1<<R3);    // Drive all row pins LOW initially
}

// Function to check whether any key is pressed
u8 ColStat(void)
{
    // Check if any key is pressed (any column LOW)
    if(((IOPIN1 >> C0) & 0x0F) == 0x0F)          // Read all four column bits
        return 0;                               // All columns HIGH → No key is pressed
    else
        return 1;                               // At least one column is LOW → A key is pressed
}

// Function to identify which key is pressed
u8 KeyVal(void)
{
    u8 row = 0, col = 0;                        // Variables to store detected row and column numbers

    // Scan Row 0
    IOCLR1 = (1<<R0);                           // Make Row 0 LOW (active row)
    IOSET1 = (1<<R1)|(1<<R2)|(1<<R3);           // Make all other rows HIGH
    if(((IOPIN1 >> C0) & 0x0F) != 0x0F)         // Check whether any column becomes LOW
    {
        row = 0;                               // Store detected row number
        goto detect_col;                       // Skip remaining row scanning and detect column
    }

    // Scan Row 1
    IOCLR1 = (1<<R1);                           // Make Row 1 LOW
    IOSET1 = (1<<R0)|(1<<R2)|(1<<R3);           // Keep other rows HIGH
    if(((IOPIN1 >> C0) & 0x0F) != 0x0F)         // Check for key press
    {
        row = 1;                               // Store detected row number
        goto detect_col;                       // Jump to column detection
    }

    // Scan Row 2
    IOCLR1 = (1<<R2);                           // Make Row 2 LOW
    IOSET1 = (1<<R0)|(1<<R1)|(1<<R3);           // Keep remaining rows HIGH
    if(((IOPIN1 >> C0) & 0x0F) != 0x0F)         // Check for key press
    {
        row = 2;                               // Store detected row number
        goto detect_col;                       // Jump to column detection
    }

    // Scan Row 3
    IOCLR1 = (1<<R3);                           // Make Row 3 LOW
    IOSET1 = (1<<R0)|(1<<R1)|(1<<R2);           // Keep all other rows HIGH
    if(((IOPIN1 >> C0) & 0x0F) != 0x0F)         // Check for key press
    {
        row = 3;                               // Store detected row number
    }

detect_col:                                    // Label used after identifying the active row

    if(((IOPIN1 >> C0) & 1) == 0)               // Check whether Column 0 is LOW
        col = 0;                               // Key belongs to Column 0
    else if(((IOPIN1 >> C1) & 1) == 0)          // Check whether Column 1 is LOW
        col = 1;                               // Key belongs to Column 1
    else if(((IOPIN1 >> C2) & 1) == 0)          // Check whether Column 2 is LOW
        col = 2;                               // Key belongs to Column 2
    else                                       // Otherwise
        col = 3;                               // Key belongs to Column 3

    // Reset rows
    IOCLR1 = (1<<R0)|(1<<R1)|(1<<R2)|(1<<R3);   // Drive all rows LOW after scanning

    return LUT[row][col];                      // Return the corresponding key character from the lookup table
}
/*
===============================================================================
File Name   : functions_t.c

Description :
This file contains functions for local configuration and external interrupt
handling in the LPC2148 based GSM temperature and humidity monitoring system.

Main Functions :
- Configures and enables external interrupt EINT0
- Handles the EINT0 interrupt service routine
- Reads a four-digit password through the keypad
- Reads user input through the keypad
- Provides temperature and humidity set-point configuration
- Provides password change functionality
- Handles local configuration menu selection
- Stores updated configuration values in external EEPROM
- Updates configuration values in RAM
===============================================================================
*/

#include <LPC21xx.h>
#include <string.h>
#include "delay.h"
#include "types.h"
#include "defines.h"
#include "PinConnect.h"
#include "lcd.h"
#include "KeyPd.h"
#include "i2c_eeprom_t.h"
#include "functions_t.h"

/* ================= EXTERNAL INTERRUPT CONFIGURATION ================= */

// Port 0 pin used as External Interrupt 0 (EINT0)
#define EINT0_PIN       16

// VIC channel number assigned to EINT0
#define EINT0_VIC_CHNO  14

/* ================= EXTERNAL VARIABLES ================= */

// Mobile number stored in EEPROM
extern char stored_num[11];

// Password stored in EEPROM
extern char stored_pass[5];

// Temperature set-point stored as a two-digit string
extern char stored_temp[3];

// Humidity set-point stored as a two-digit string
extern char stored_hum[3];

// Integer temperature set-point
extern int temp_sp;

// Integer humidity set-point
extern int hum_sp;

/* ================= MENU FLAG ================= */

// Flag set when EINT0 interrupt occurs
volatile unsigned char menu_flag = 0;

/*=============================================================================
Function Name : eint0_isr

Description :
Interrupt Service Routine for External Interrupt 0.

When the configuration switch triggers EINT0, this ISR sets menu_flag.
The main program checks this flag and enters the local configuration menu.

=============================================================================*/

void eint0_isr(void) __irq
{
    // Indicate that the configuration menu has been requested
    menu_flag = 1;

    // Clear EINT0 interrupt flag
    EXTINT = 1 << 0;

    // Signal completion of interrupt servicing to the VIC
    VICVectAddr = 0;
}

/*=============================================================================
Function Name : Enable_EINT0

Description :
Configures Port 0 Pin 16 as EINT0 and enables the EINT0 interrupt
through the Vector Interrupt Controller (VIC).

=============================================================================*/

void Enable_EINT0(void)
{
    // Configure P0.16 as EINT0 function
    CfgPinFunc(PORT0, EINT0_PIN, FUNC2);

    // Enable EINT0 interrupt in the VIC
    VICIntEnable = 1 << EINT0_VIC_CHNO;

    // Assign EINT0 interrupt channel to VIC vector slot 1
    VICVectCntl1 = (1 << 5) | EINT0_VIC_CHNO;

    // Assign EINT0 ISR address to VIC vector slot 1
    VICVectAddr1 = (unsigned int)eint0_isr;

    // Configure EINT0 as an edge-triggered interrupt
    EXTMODE = 1 << 0;
}

/*=============================================================================
Function Name : get_password

Description :
Reads a four-digit password from the 4x4 matrix keypad.

The entered characters are displayed as '*' on the LCD to hide
the password from the user.

Parameters :
pass - Pointer to the buffer used to store the entered password.

=============================================================================*/

void get_password(char *pass)
{
    int i;

    // Read four password digits
    for(i = 0; i < 4; i++)
    {
        // Wait until a key is pressed
        while(ColStat() == 0);

        // Read the pressed key
        pass[i] = KeyVal();

        // Display '*' instead of the actual password character
        CharLCD('*');

        // Wait until the key is released
        while(ColStat() == 1);

        // Key debounce delay
        delay_ms(200);
    }

    // Add string terminator
    pass[4] = '\0';
}

/*=============================================================================
Function Name : get_input

Description :
Reads the required number of characters from the keypad and displays
the entered characters on the LCD.

Parameters :
data - Pointer to the buffer used to store the input.
len  - Number of characters to be read.

=============================================================================*/

void get_input(char *data, int len)
{
    int i;

    // Read the required number of characters
    for(i = 0; i < len; i++)
    {
        // Wait until a key is pressed
        while(ColStat() == 0);

        // Read the pressed key
        data[i] = KeyVal();

        // Display the entered character
        CharLCD(data[i]);

        // Wait until the key is released
        while(ColStat() == 1);

        // Key debounce delay
        delay_ms(200);
    }

    // Add string terminator
    data[len] = '\0';
}

/*=============================================================================
Function Name : setpoint_menu

Description :
Provides the local menu for changing temperature and humidity set-points.

The user must enter the correct four-digit password before accessing
the configuration options.

A maximum of three password attempts is allowed.

=============================================================================*/

void setpoint_menu(void)
{
    char entered_pass[5];
    char new_value[4];
    int attempts = 0;
    char key;

    // Allow a maximum of three password attempts
    while(attempts < 3)
    {
        // Ask the user to enter the password
        CmdLCD(0x01);
        StrLCD("Enter Pass:");

        // Read the four-digit password
        get_password(entered_pass);

        // Verify the entered password
        if(strncmp(entered_pass, stored_pass, 4) == 0)
        {
            // Indicate successful authentication using the green LED
            IOSET0 = (1 << GREEN_LED);
            delay_ms(500);
            IOCLR0 = (1 << GREEN_LED);

            // Display available configuration options
            CmdLCD(0x01);
            StrLCD("1.Temp 2.Hum");

            // Display current set-points
            CmdLCD(0xC0);
            StrLCD("T:");
            IntLCD(temp_sp);
            StrLCD("C  H:");
            IntLCD(hum_sp);
            StrLCD("%");

            // Wait for menu selection
            while(ColStat() == 0);

            // Read selected menu option
            key = KeyVal();

            // Wait until the key is released
            while(ColStat() == 1);

            // Key debounce delay
            delay_ms(200);

            /* ================= TEMPERATURE SET-POINT ================= */

            if(key == '1')
            {
                // Ask the user to enter the new temperature set-point
                CmdLCD(0x01);
                StrLCD("Set Temp:");

                // Read two-digit temperature value
                get_input(new_value, 2);

                // Store new temperature set-point in EEPROM
                i2c_eeprom_pagewrite(0x50, ADDR_TEMP, new_value, 2);

                // Wait for EEPROM write cycle to complete
                delay_ms(10);

                // Update the RAM copy
                strncpy(stored_temp, new_value, 2);
                stored_temp[2] = '\0';

                // Convert ASCII value into integer set-point
                temp_sp = (stored_temp[0] - '0') * 10 +
                          (stored_temp[1] - '0');

                // Display confirmation
                CmdLCD(0x01);
                StrLCD("Temp Updated");
            }

            /* ================= HUMIDITY SET-POINT ================= */

            else if(key == '2')
            {
                // Ask the user to enter the new humidity set-point
                CmdLCD(0x01);
                StrLCD("Set Hum:");

                // Read two-digit humidity value
                get_input(new_value, 2);

                // Store new humidity set-point in EEPROM
                i2c_eeprom_pagewrite(0x50, ADDR_HUM, new_value, 2);

                // Wait for EEPROM write cycle to complete
                delay_ms(10);

                // Update the RAM copy
                strncpy(stored_hum, new_value, 2);
                stored_hum[2] = '\0';

                // Convert ASCII value into integer set-point
                hum_sp = (stored_hum[0] - '0') * 10 +
                         (stored_hum[1] - '0');

                // Display confirmation
                CmdLCD(0x01);
                StrLCD("Hum Updated");
            }

            // Allow the user to view the result
            delay_ms(1000);

            // Exit after successful configuration
            return;
        }

        /* ================= WRONG PASSWORD ================= */

        else
        {
            // Increment failed attempt count
            attempts++;

            // Turn ON red LED and buzzer
            IOSET0 = (1 << RED_LED) | (1 << BUZZER);

            // Keep fault indication active
            delay_ms(500);

            // Turn OFF red LED and buzzer
            IOCLR0 = (1 << RED_LED) | (1 << BUZZER);

            // Display incorrect password message
            CmdLCD(0x01);
            StrLCD("Wrong Pass");
        }
    }

    /* ================= MAXIMUM ATTEMPTS EXCEEDED ================= */

    CmdLCD(0x01);
    StrLCD("Blocked...");

    // Display blocked status for three seconds
    delay_ms(3000);
}

/*=============================================================================
Function Name : password_change

Description :
Allows the authenticated user to change the system password.

The current password is verified first. The new password must be entered
twice and both entries must match before the new password is stored.

A maximum of three attempts is allowed for current-password verification.

=============================================================================*/

void password_change(void)
{
    char current_pass[5];
    char new_pass[5];
    char confirm_pass[5];
    int attempts = 0;

    // Allow a maximum of three password attempts
    while(attempts < 3)
    {
        // Ask the user to enter the current password
        CmdLCD(0x01);
        StrLCD("Curr Pass:");

        // Read current password
        get_password(current_pass);

        // Verify current password
        if(strncmp(current_pass, stored_pass, 4) == 0)
        {
            // Ask for the new password
            CmdLCD(0x01);
            StrLCD("New Pass:");

            // Read new password
            get_password(new_pass);

            // Ask for password confirmation
            CmdLCD(0x01);
            StrLCD("Confirm:");

            // Read confirmation password
            get_password(confirm_pass);

            // Check whether the new password entries match
            if(strncmp(new_pass, confirm_pass, 4) == 0)
            {
                // Store new password in EEPROM
                i2c_eeprom_pagewrite(0x50, ADDR_PASS, new_pass, 4);

                // Wait for EEPROM write cycle to complete
                delay_ms(10);

                // Update the RAM copy
                strncpy(stored_pass, new_pass, 4);
                stored_pass[4] = '\0';

                // Display successful password change
                CmdLCD(0x01);
                StrLCD("Pass Changed");
            }
            else
            {
                // Display password mismatch
                CmdLCD(0x01);
                StrLCD("Mismatch!");
            }

            // Allow the user to view the result
            delay_ms(1500);

            // Exit after processing the password change
            return;
        }

        /* ================= WRONG CURRENT PASSWORD ================= */

        else
        {
            // Increment failed attempt count
            attempts++;

            // Display incorrect password message
            CmdLCD(0x01);
            StrLCD("Wrong Pass");

            // Turn ON red LED and buzzer
            IOSET0 = (1 << RED_LED) | (1 << BUZZER);

            // Keep fault indication active
            delay_ms(500);

            // Turn OFF red LED and buzzer
            IOCLR0 = (1 << RED_LED) | (1 << BUZZER);
        }
    }

    /* ================= MAXIMUM ATTEMPTS EXCEEDED ================= */

    CmdLCD(0x01);
    StrLCD("Blocked...");

    // Display blocked status for three seconds
    delay_ms(3000);
}

/*=============================================================================
Function Name : menu_function

Description :
Displays the local configuration menu and allows the user to select
either set-point configuration or password change.

Menu:
1 - Temperature/Humidity set-point configuration
2 - Password change

=============================================================================*/

void menu_function(void)
{
    char key;

    // Clear LCD
    CmdLCD(0x01);

    // Display menu options
    StrLCD("1.Set 2.Pass");

    // Wait until a key is pressed
    while(ColStat() == 0);

    // Read selected menu option
    key = KeyVal();

    // Wait until the key is released
    while(ColStat() == 1);

    // Key debounce delay
    delay_ms(200);

    // Open set-point configuration menu
    if(key == '1')
        setpoint_menu();

    // Open password change menu
    else if(key == '2')
        password_change();
}

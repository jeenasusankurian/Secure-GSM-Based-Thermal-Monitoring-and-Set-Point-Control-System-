//rtc.c                              // Source file containing RTC (Real Time Clock) driver functions

#include <LPC21xx.h>                 // LPC21xx microcontroller register definitions
#include "rtc_t.h"                   // Header file containing RTC function declarations
#include "rtc_defines_t.h"           // Header file containing RTC register macros and constants
#include "lcd_t.h"                   // Header file containing LCD function declarations
#include "types_t.h"                 // Header file containing user-defined data types (u8, u32, s32, etc.)

extern s32 hour,minute,second,date,month,year;
                                    // Global variables defined in another source file


// Days of week
static char week[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
                                    // Lookup table storing abbreviated names of the seven weekdays

/* RTC Initialization */
void RTC_Init(void)
{
    // Reset RTC
    CCR = RTC_RESET;                // Reset the RTC by writing the reset value into the Clock Control Register

    // Set prescaler values
    PREINT  = PREINT_VAL;           // Load integer prescaler value for 1-second timing
    PREFRAC = PREFRAC_VAL;          // Load fractional prescaler value for accurate RTC clock

    // Enable RTC
    CCR = RTC_ENABLE;               // Enable and start the RTC
}

/* Get Time */
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
{
    *hour   = HOUR;                 // Read current hour from RTC Hour Register
    *minute = MIN;                  // Read current minute from RTC Minute Register
    *second = SEC;                  // Read current second from RTC Second Register
}

/* Display Time on LCD */
void DisplayRTCTime(u32 hour, u32 minute, u32 second)
{
    CmdLCD(0x80);                   // Move LCD cursor to first line, first position

    CharLCD((hour/10) + '0');       // Display tens digit of hour
    CharLCD((hour%10) + '0');       // Display units digit of hour
    CharLCD(':');                   // Display ':' separator

    CharLCD((minute/10) + '0');     // Display tens digit of minute
    CharLCD((minute%10) + '0');     // Display units digit of minute
    CharLCD(':');                   // Display ':' separator

    CharLCD((second/10) + '0');     // Display tens digit of second
    CharLCD((second%10) + '0');     // Display units digit of second
}

/* Get Date */
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year)
{
    *date  = DOM;                   // Read current day of month from RTC
    *month = MONTH;                 // Read current month from RTC
    *year  = YEAR;                  // Read current year from RTC
}

/* Display Date on LCD */
void DisplayRTCDate(u32 date, u32 month, u32 year)
{
    CmdLCD(0xC0);                   // Move LCD cursor to second line, first position

    CharLCD((date/10) + '0');       // Display tens digit of date
    CharLCD((date%10) + '0');       // Display units digit of date
    CharLCD('/');                   // Display '/' separator

    CharLCD((month/10) + '0');      // Display tens digit of month
    CharLCD((month%10) + '0');      // Display units digit of month
    CharLCD('/');                   // Display '/' separator

    IntLCD(year);                   // Display complete year using integer display function
}

/* Set Time */
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
    HOUR = hour;                    // Store hour value into RTC Hour Register
    MIN  = minute;                  // Store minute value into RTC Minute Register
    SEC  = second;                  // Store second value into RTC Second Register
}

/* Set Date */
void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
    DOM   = date;                   // Store day of month into RTC Date Register
    MONTH = month;                  // Store month into RTC Month Register
    YEAR  = year;                   // Store year into RTC Year Register
}

/* Get Day */
void GetRTCDay(s32 *day)
{
    *day = DOW;                     // Read current day of week from RTC Day-of-Week Register
}

/* Display Day */
void DisplayRTCDay(u32 dow)
{
    CmdLCD(0x94);                   // Move cursor to middle position of second LCD line

    StrLCD((u8*)week[dow]);         // Display corresponding weekday name from lookup table
}

/* Set Day */
void SetRTCDay(u32 day)
{
    DOW = day;                      // Store day-of-week value into RTC Day-of-Week Register
}

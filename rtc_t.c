//rtc.c
#include <LPC21xx.h>
#include "rtc_t.h"
#include "rtc_defines_t.h"
#include "lcd_t.h"
#include "types_t.h"
extern s32 hour,minute,second,date,month,year;


// Days of week
static char week[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

/* RTC Initialization */
void RTC_Init(void)
{
    // Reset RTC
    CCR = RTC_RESET;

    // Set prescaler values
    PREINT  = PREINT_VAL;
    PREFRAC = PREFRAC_VAL;

    // Enable RTC
    CCR = RTC_ENABLE;
}

/* Get Time */
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
{
    *hour   = HOUR;
    *minute = MIN;
    *second = SEC;
}

/* Display Time on LCD */
void DisplayRTCTime(u32 hour, u32 minute, u32 second)
{
    CmdLCD(0x80);

    CharLCD((hour/10) + '0');
    CharLCD((hour%10) + '0');
    CharLCD(':');

    CharLCD((minute/10) + '0');
    CharLCD((minute%10) + '0');
    CharLCD(':');

    CharLCD((second/10) + '0');
    CharLCD((second%10) + '0');
}

/* Get Date */
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year)
{
    *date  = DOM;
    *month = MONTH;
    *year  = YEAR;
}

/* Display Date on LCD */
void DisplayRTCDate(u32 date, u32 month, u32 year)
{
    CmdLCD(0xC0);

    CharLCD((date/10) + '0');
    CharLCD((date%10) + '0');
    CharLCD('/');

    CharLCD((month/10) + '0');
    CharLCD((month%10) + '0');
    CharLCD('/');

    IntLCD(year);
}

/* Set Time */
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
    HOUR = hour;
    MIN  = minute;
    SEC  = second;
}

/* Set Date */
void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
    DOM   = date;
    MONTH = month;
    YEAR  = year;
}

/* Get Day */
void GetRTCDay(s32 *day)
{
    *day = DOW;
}

/* Display Day */
void DisplayRTCDay(u32 dow)
{
    CmdLCD(0x94);   // 2nd line, mid position
    StrLCD((u8*)week[dow]);
}

/* Set Day */
void SetRTCDay(u32 day)
{
    DOW = day;
}


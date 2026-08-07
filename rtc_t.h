//rtc.h
#ifndef _RTC_H_
#define _RTC_H_

#include "types_t.h"

// Initialize RTC
void RTC_Init(void);

// Get Time
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second);

// Display Time on LCD
void DisplayRTCTime(u32 hour, u32 minute, u32 second);

// Get Date
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year);

// Display Date on LCD
void DisplayRTCDate(u32 date, u32 month, u32 year);

// Set Time
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second);

// Set Date
void SetRTCDateInfo(u32 date, u32 month, u32 year);

// Get Day of Week
void GetRTCDay(s32 *day);

// Display Day on LCD
void DisplayRTCDay(u32 day);

// Set Day of Week
void SetRTCDay(u32 day);

#endif


//rtc_defines.h
#ifndef _RTC_DEFINES_H_
#define _RTC_DEFINES_H_

//--------------------------------------------------
// System Clock Definitions
//--------------------------------------------------
#define FOSC   12000000UL          // Crystal Frequency (12 MHz)
#define CCLK   (5 * FOSC)          // CPU Clock (PLL = 5 ? 60 MHz)
#define PCLK   (CCLK / 4)          // Peripheral Clock

//--------------------------------------------------
// RTC Prescaler Values (for 1 second increment)
//--------------------------------------------------
#define PREINT_VAL   ((PCLK / 32768) - 1)
#define PREFRAC_VAL  (PCLK - ((PREINT_VAL + 1) * 32768))

//--------------------------------------------------
// CCR (Clock Control Register) Bits
//--------------------------------------------------
#define RTC_ENABLE   (1 << 0)   // Enable RTC
#define RTC_RESET    (1 << 1)   // Reset RTC
#define RTC_CLKSRC   (1 << 4)   // Clock Source (if needed)

//--------------------------------------------------

#endif


```c
/*
===============================================================================
File Name   : main_t.c

Description :
This file contains the main application program for the
GSM Based Temperature and Humidity Monitoring System
using LPC2148 microcontroller.

Main Functions :
- Initializes all required peripherals
- Initializes and communicates with GSM module through UART
- Reads temperature and humidity from DHT11 sensor
- Stores configuration parameters in external EEPROM
- Maintains date and time using RTC
- Compares sensor values with configured setpoints
- Generates LED and buzzer indications during abnormal conditions
- Sends SMS alerts through GSM
- Receives and processes SMS commands
- Provides local configuration through keypad and EINT0

Communication Interfaces :
UART        -> LPC2148 to GSM module
I2C         -> LPC2148 to external EEPROM
Single-wire -> LPC2148 to DHT11 sensor
EINT0       -> External menu button interrupt

EEPROM Power Failure Handling :
The mobile number, password, temperature setpoint and humidity
setpoint are stored in non-volatile EEPROM.

During the first system startup, default values are stored and
an initialization marker is written to EEPROM.

During subsequent startups, the marker is checked. If the marker
is already present, the default values are NOT written again.
Instead, the previously stored configuration values are read.

Therefore, modified configuration values are retained after
power OFF and power ON.
===============================================================================
*/

#include <lpc21xx.h>
#include <string.h>
#include "uart0_t.h"
#include "delay_t.h"
#include "gsm_t.h"
#include "lcd_t.h"
#include "i2c_t.h"
#include "i2c_eeprom_t.h"
#include "dht11_t.h"
#include "rtc_t.h"
#include "functions_t.h"
#include "defines_t.h"
#include "types_t.h"
#include "KeyPd_t.h"

/* ================= RTC VARIABLES ================= */

// Variables used to store current RTC time and date
s32 hour, minute, second, date, month, year;

/* ================= DHT11 VARIABLES ================= */

// Variables used to store DHT11 sensor readings
unsigned char humidity_integer;
unsigned char humidity_decimal;
unsigned char temp_integer;
unsigned char temp_decimal;
unsigned char checksum;

/* ================= ALERT FLAGS ================= */

// Flags prevent repeated SMS alerts while the condition remains abnormal
static int temp_flag = 0;
static int hum_flag = 0;

/* ================= GENERAL VARIABLES ================= */

// General message buffer
char msg[80];

// Menu flag is set by EINT0 ISR
extern volatile unsigned char menu_flag;

/* ================= EEPROM DATA VARIABLES ================= */

// Registered mobile number
char stored_num[11];

// Password
char stored_pass[5];

// Temperature setpoint stored as 2 ASCII digits
char stored_temp[3];

// Humidity setpoint stored as 2 ASCII digits
char stored_hum[3];

/* ================= DEFAULT VALUES ================= */

// Default mobile number
char default_num[11] = "8547858297";

// Default password
char default_pass[5] = "0786";

// Default temperature setpoint
char default_temp[] = "25";

// Default humidity setpoint
char default_hum[] = "60";

/* ================= GSM VARIABLES ================= */

// SMS received flag
unsigned char r_flag = 0;

// UART receive buffer index
extern unsigned char i;

// GSM receive buffer
extern char buff[200];

/* ================= SETPOINT VARIABLES ================= */

// Integer versions of temperature and humidity setpoints
int temp_sp;
int hum_sp;

/* ================= EEPROM INITIALIZATION MARKER ================= */

// EEPROM location used to identify whether default values
// have already been initialized
#define ADDR_INIT 100

// Variable used to read EEPROM initialization marker
char eeprom_init;

/* ================= TWO DIGIT ASCII TO INTEGER ================= */

/*
Function : safe_atoi_2digit()

Description :
Converts a two-digit ASCII string such as "25" into
the integer value 25.

Example :
'2' '5' -> 25
*/

int safe_atoi_2digit(char *data)
{
    // Check whether first character is a digit
    if(data[0] < '0' || data[0] > '9')
        return 0;

    // Check whether second character is a digit
    if(data[1] < '0' || data[1] > '9')
        return 0;

    // Convert ASCII digits into integer value
    return (data[0] - '0') * 10 + (data[1] - '0');
}

/* ================= MAIN FUNCTION ================= */

int main()
{
    /* ================= GPIO INITIALIZATION ================= */

    // Configure buzzer, green LED and red LED as output pins
    IODIR0 |= (1 << BUZZER) |
              (1 << GREEN_LED) |
              (1 << RED_LED);

    /* ================= PERIPHERAL INITIALIZATION ================= */

    // Initialize UART0 for GSM communication
    InitUART0();

    // Initialize LCD
    InitLCD();

    // Initialize keypad
    KeyPdInit();

    // Initialize I2C peripheral
    init_i2c();

    /* ================= GSM BUFFER INITIALIZATION ================= */

    // Reset UART receive buffer index
    i = 0;

    // Clear GSM receive buffer
    memset(buff, '\0', sizeof(buff));

    /* ================= GSM INITIALIZATION ================= */

    // Wait until GSM modem and SIM card become ready
    gsm_wait_startup();

    // Configure GSM module using AT commands
    gsm_init();

    /* ================= EXTERNAL INTERRUPT ================= */

    // Enable EINT0 for keypad/menu button
    Enable_EINT0();

    /* ================= RTC INITIALIZATION ================= */

    // Initialize RTC peripheral
    RTC_Init();

    // Set initial RTC time
    SetRTCTimeInfo(10, 0, 0);

    // Set initial RTC date
    SetRTCDateInfo(30, 5, 2026);

    /* =========================================================
       EEPROM INITIALIZATION CHECK
       ========================================================= */

    // Read initialization marker from EEPROM
    eeprom_init = i2c_eeprom_randomread(0x50, ADDR_INIT);

    /*
    If the marker is not 'Y', this is treated as the first
    initialization of the EEPROM.

    Therefore default values are written only once.
    */

    if(eeprom_init != 'Y')
    {
        /* ================= DEFAULT MOBILE NUMBER ================= */

        // Store default mobile number in EEPROM
        i2c_eeprom_pagewrite(0x50, ADDR_MOB, default_num, 10);

        // Wait for EEPROM write cycle to complete
        delay_ms(10);

        /* ================= DEFAULT PASSWORD ================= */

        // Store default password in EEPROM
        i2c_eeprom_pagewrite(0x50, ADDR_PASS, default_pass, 4);

        // Wait for EEPROM write cycle to complete
        delay_ms(10);

        /* ================= DEFAULT TEMPERATURE ================= */

        // Store default temperature setpoint in EEPROM
        i2c_eeprom_pagewrite(0x50, ADDR_TEMP, default_temp, 2);

        // Wait for EEPROM write cycle to complete
        delay_ms(10);

        /* ================= DEFAULT HUMIDITY ================= */

        // Store default humidity setpoint in EEPROM
        i2c_eeprom_pagewrite(0x50, ADDR_HUM, default_hum, 2);

        // Wait for EEPROM write cycle to complete
        delay_ms(10);

        /* ================= SET INITIALIZATION MARKER ================= */

        // Store 'Y' to indicate EEPROM has been initialized
        i2c_eeprom_bytewrite(0x50, ADDR_INIT, 'Y');

        // Wait for EEPROM write cycle to complete
        delay_ms(10);
    }

    /* =========================================================
       READ CONFIGURATION VALUES FROM EEPROM
       ========================================================= */

    // Read registered mobile number from EEPROM
    i2c_eeprom_seqread(0x50, ADDR_MOB, stored_num, 10);

    // Add string terminator
    stored_num[10] = '\0';

    // Read password from EEPROM
    i2c_eeprom_seqread(0x50, ADDR_PASS, stored_pass, 4);

    // Add string terminator
    stored_pass[4] = '\0';

    // Read temperature setpoint from EEPROM
    i2c_eeprom_seqread(0x50, ADDR_TEMP, stored_temp, 2);

    // Add string terminator
    stored_temp[2] = '\0';

    // Read humidity setpoint from EEPROM
    i2c_eeprom_seqread(0x50, ADDR_HUM, stored_hum, 2);

    // Add string terminator
    stored_hum[2] = '\0';

    /* ================= SYSTEM READY ================= */

    // Clear LCD
    CmdLCD(0x01);

    // Display system ready message
    StrLCD("GSM SYSTEM READY");

    // Wait for one second
    delay_ms(1000);

    /* ================= STARTUP SMS ================= */

    // Send system startup message to registered number
    send_sms(stored_num, "SYSTEM STARTED");

    /* =========================================================
       MAIN MONITORING LOOP
       ========================================================= */

    while(1)
    {
        /* ================= STEP 1 : READ DHT11 ================= */

        // Read temperature and humidity from DHT11
        dht11();

        /* ================= STEP 2 : READ SETPOINTS ================= */

        // Read temperature setpoint from EEPROM
        i2c_eeprom_seqread(0x50, ADDR_TEMP, stored_temp, 2);

        // Read humidity setpoint from EEPROM
        i2c_eeprom_seqread(0x50, ADDR_HUM, stored_hum, 2);

        /* ================= STEP 3 : CONVERT SETPOINTS ================= */

        // Convert temperature ASCII value into integer
        temp_sp = safe_atoi_2digit(stored_temp);

        // Convert humidity ASCII value into integer
        hum_sp = safe_atoi_2digit(stored_hum);

        /* ================= STEP 4 : READ RTC ================= */

        // Read current time
        GetRTCTimeInfo(&hour, &minute, &second);

        // Read current date
        GetRTCDateInfo(&date, &month, &year);

        /* =====================================================
           STEP 5 : TEMPERATURE MONITORING
           ===================================================== */

        // Check whether temperature exceeds configured setpoint
        if(temp_integer > temp_sp && temp_flag == 0)
        {
            // Set flag to prevent repeated SMS alerts
            temp_flag = 1;

            // Turn ON green LED and buzzer
            IOSET0 = (1 << GREEN_LED) |
                     (1 << BUZZER);

            // Send high temperature alert SMS
            send_sms_temp(stored_num,
                          "ALERT:HIGH TEMP",
                          temp_integer,
                          temp_decimal);
        }

        // Check whether temperature has returned to normal
        else if(temp_integer <= temp_sp)
        {
            // Reset flag so another alert can be generated later
            temp_flag = 0;

            // Turn OFF green LED and buzzer
            IOCLR0 = (1 << GREEN_LED) |
                     (1 << BUZZER);
        }

        /* =====================================================
           STEP 6 : HUMIDITY MONITORING
           ===================================================== */

        // Check whether humidity exceeds configured setpoint
        if(humidity_integer > hum_sp && hum_flag == 0)
        {
            // Set flag to prevent repeated SMS alerts
            hum_flag = 1;

            // Turn ON red LED and buzzer
            IOSET0 = (1 << RED_LED) |
                     (1 << BUZZER);

            // Send high humidity alert SMS
            send_sms_humi(stored_num,
                          "ALERT:HIGH HUMIDITY",
                          humidity_integer,
                          humidity_decimal);
        }

        // Check whether humidity has returned to normal
        else if(humidity_integer <= hum_sp)
        {
            // Reset flag
            hum_flag = 0;

            // Turn OFF red LED and buzzer
            IOCLR0 = (1 << RED_LED) |
                     (1 << BUZZER);
        }

        /* =====================================================
           STEP 7 : SMS RECEIVE CHECK
           ===================================================== */

        // Check whether a new SMS has been received
        if(r_flag)
        {
            // Clear SMS received flag
            r_flag = 0;

            // Process received SMS command
            receive_sms();
        }

        /* =====================================================
           STEP 8 : MENU INTERRUPT CHECK
           ===================================================== */

        // Check whether EINT0 generated a menu request
        if(menu_flag)
        {
            // Clear menu flag
            menu_flag = 0;

            // Execute keypad menu
            menu_function();
        }

        /* ================= LOOP DELAY ================= */

        // Small delay before next monitoring cycle
        delay_ms(300);
    }
}
```

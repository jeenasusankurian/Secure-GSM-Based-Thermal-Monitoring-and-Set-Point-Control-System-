/*
===============================================================================
File Name   : gsm.c

Description :
This file contains functions for interfacing the GSM
module with the LPC21xx microcontroller.

It provides:
- GSM module startup detection
- GSM module initialization using AT commands
- SMS transmission to registered mobile numbers
- SMS reception and command processing
- Temperature alert message transmission
- Humidity alert message transmission
- Mobile number verification and management
- Password verification and authentication
- EEPROM parameter update through SMS commands
- UART-based communication with GSM module

The GSM module is connected through UART0 and
is used for sending alert messages, receiving
user commands, and remotely configuring system
parameters such as temperature setpoint,
humidity setpoint, mobile number, and password.
===============================================================================
*/
#include <string.h>              // Header file for string functions like memset(), strstr(), strlen()
#include <stdio.h>               // Header file for standard input/output functions

#include <lpc21xx.h>             // LPC21xx/LPC2148 register definitions

#include "uart0_t.h"             // UART0 communication function declarations
#include "delay_t.h"              // Delay function declarations
#include "gsm_t.h"               // GSM related function declarations
#include "lcd_t.h"               // LCD display function declarations
#include "dht11_t.h"             // DHT11 sensor function declarations
#include "i2c_eeprom_t.h"        // EEPROM read/write function declarations
#include "defines_t.h"            // Project specific macro definitions

// Global buffer used by UART receive interrupt
char buff[200];                   // Stores characters received from GSM module through UART

int sender_len;                   // Stores length of sender mobile number

unsigned char i = 0;              // Index used for storing received UART characters inside buffer

unsigned char cnt = 0;            // General purpose counter variable

int sms_index = 1;                // Stores SMS index/location in SIM memory

// External Variables
extern char stored_num[11];       // Mobile number stored in EEPROM and accessed from another file

extern char stored_pass[5];       // Password stored in EEPROM and accessed from another file

extern int hour,minute,second,date,month,year;
                                  // RTC variables containing current time and date

/* ================= GSM INITIALIZATION ================= */

void gsm_wait_startup(void)
{
    // Reset UART receive buffer index
    i = 0;
    // Clear previous GSM received data
    memset(buff, '\0', sizeof(buff));
    // Display startup message on LCD
    StrLCD("LET'S START");
    delay_ms(1000);               // Wait for GSM module power-up
    // Wait until GSM module sends startup indication
    while(!strstr(buff, "MODEM:STARTUP"));
    CmdLCD(0x01);                 // Clear LCD display
    StrLCD("MODEM READY");       // Display GSM modem ready status
    delay_ms(1000);
    // Reset buffer index after receiving startup message
    i = 0;
    // Clear received startup message
    memset(buff, '\0', sizeof(buff));
    // Wait until SIM card initialization is completed
    while(!strstr(buff, "+PBREADY"));
    CmdLCD(0x01);                // Clear LCD
    StrLCD("SIM READY");         // Display SIM ready message
    delay_ms(1000);
    // Clear buffer for normal GSM communication
    i = 0;
    memset(buff, '\0', sizeof(buff));
}
/*================ GSM INITIALIZATION ================*/
/*
Function : gsm_init()

Description :
Initializes GSM module using AT commands.

Steps:
1. Check GSM communication using AT command
2. Disable command echo
3. Configure SMS text mode
4. Enable new SMS notification
*/
void gsm_init()
{
     //=============== BASIC AT COMMAND =================
    // Reset UART receive buffer
    i = 0;
    memset(buff, '\0', sizeof(buff));
    // Send AT command to check GSM communication
    UART0_Str("AT\r\n");
    // Wait for GSM response "OK"
    if(!gsm_wait_for("OK", 5000))
    {
        // Display error if GSM does not respond
        CmdLCD(0x01);
        StrLCD("AT FAIL");
        return;                  // Exit GSM initialization
    }
    //================ DISABLE COMMAND ECHO =================
    i = 0;                       // Reset buffer index
    memset(buff, '\0', sizeof(buff));
    // Disable GSM command echo
    UART0_Str("ATE0\r\n");
    // Wait for confirmation
    if(!gsm_wait_for("OK", 5000))
    {
        CmdLCD(0x01);
        StrLCD("ATE0 FAIL");
        return;
    }
    //================ SELECT SMS TEXT MODE =================

    i = 0;

    memset(buff, '\0', sizeof(buff));

    // Configure GSM SMS mode as text mode
    UART0_Str("AT+CMGF=1\r\n");

    // Wait for OK response
    if(!gsm_wait_for("OK", 5000))
    {

        CmdLCD(0x01);

        StrLCD("CMGF FAIL");

        return;
    }

    i = 0;

    memset(buff, '\0', sizeof(buff));

    // Configure GSM to notify incoming SMS automatically
    UART0_Str("AT+CNMI=2,1,0,0,0\r\n");

    // Wait for command execution confirmation
    if(!gsm_wait_for("OK", 5000))
    {

        CmdLCD(0x01);

        StrLCD("CNMI FAIL");

        return;
    }

    CmdLCD(0x01);                // Clear LCD

    StrLCD("GSM INIT DONE");     // Display initialization success

    delay_ms(1000);

}

// Function : gsm_wait_for()
// Description:
// Waits until a specific response string is received from GSM module

int gsm_wait_for(char *keyword, int timeout)

{

    int t = 0;                   // Timeout counter

    while(t < timeout)           // Continue checking until timeout expires

    {

        // Check only when UART buffer contains received data
        if(i > 0)

        {

            // Search required keyword inside received GSM response
            if(strstr(buff, keyword) != NULL)

                return 1;        // Keyword found successfully
        }

        delay_ms(1);             // Small delay allowing UART interrupt to receive data

        t++;                     // Increment timeout counter

    }

    return 0;                    // Keyword not received within timeout period

}
/* ================= SEND SMS ================= */

// Function : send_sms()
// Description:
// Sends an SMS message to a given mobile number using GSM module.
// Process:
// 1. Send AT+CMGS command with destination number
// 2. Wait for GSM '>' prompt
// 3. Send SMS message text
// 4. Send CTRL+Z (0x1A) to transmit SMS

void send_sms(/*signed*/ char *num, /*signed*/ char *msg)
{

    // Clear UART receive buffer before sending command
    i = 0;

    memset(buff, '\0', sizeof(buff));

    // Send SMS send command
    UART0_Str("AT+CMGS=");

    // Send opening quotation mark before mobile number
    UART0_Tx('"');

    // Send destination mobile number
    UART0_Str(num);

    // Send closing quotation mark
    UART0_Tx('"');

    // Send carriage return and newline
    UART0_Str("\r\n");

    // Wait for GSM SMS input prompt '>'
    if(!gsm_wait_for(">", 7000))
    {

        // Display error if GSM does not provide prompt
        CmdLCD(0x01);

        StrLCD("NO PROMPT");

        return;                    // Exit function safely
    }

    // Clear buffer before sending SMS message text
    i = 0;

    buff[i] = '\0';

    // Send SMS message content
    UART0_Str(msg);

    // Send CTRL+Z character to indicate SMS completion
    UART0_Tx(0x1A);

    // Wait for GSM confirmation after sending SMS
    if(gsm_wait_for("OK", 25000))
    {

        CmdLCD(0x01);

        StrLCD("SMS SENT");

        delay_ms(2000);
    }

    // Check GSM error response
    else if(strstr(buff, "ERROR"))
    {

        CmdLCD(0x01);

        StrLCD("SMS ERROR");

        delay_ms(1000);
    }

    // If no OK or ERROR received
    else
    {

        CmdLCD(0x01);

        StrLCD("SMS FAIL");

        delay_ms(1000);
    }

    // Clear buffer after SMS operation
    i = 0;

    memset(buff, '\0', sizeof(buff));

}

// Function : receive_sms()
// Description:
// Reads SMS from GSM module, extracts sender number and message,
// verifies sender authentication and password,
// then processes received commands.

void receive_sms()

{

    char *p, *q;                   // Pointers used for searching strings inside GSM response

    char *num_ptr;                 // Pointer to last 10 digits of sender number

    char *msg;                     // Pointer to SMS message body

    char sender[20];               // Buffer to store sender mobile number

    char temp_val[3];              // Temporary buffer for temperature value

    char hum_val[3];               // Temporary buffer for humidity value

    char new_num[11];              // Buffer for new mobile number

    char msg_send[30];             // Buffer for outgoing SMS message

    int j = 0, t = 0, h = 0;        // Loop counters and temporary variables

    // Display SMS received message
    CmdLCD(0x01);

    StrLCD("SMS RECEIVED");

    delay_ms(1000);

    // Reset UART receive buffer
    i = 0;

    memset(buff, '\0', sizeof(buff));

    // Send GSM command to read SMS from SIM memory
    UART0_Str("AT+CMGR=");

    // Send SMS index number
    UART0_Int(sms_index);

    UART0_Str("\r\n");

    // Wait for GSM read response
    if(!gsm_wait_for("OK", 5000))

    {

        CmdLCD(0x01);

        StrLCD("READ FAIL");

        return;
    }

    // Check whether SMS read response contains CMGR header
    if(strstr(buff, "+CMGR:"))

    {

        // ================= SENDER NUMBER EXTRACTION =================

        // Find CMGR response starting position
        p = strstr(buff, "+CMGR:");

        // Initialize sender string as empty
        sender[0] = '\0';

        if(p != NULL)

        {

            // Search for phone number format with country code
            p = strstr(p, ",\"+");

            if(p == NULL)

            {

                // If '+' country code is not present, search normal format
                p = strstr(buff, ",\"");
            }

            if(p != NULL)

            {

                // Move pointer after comma and quotation marks
                p += 2;

                // Find closing quotation mark after phone number
                q = strchr(p, '"');

                if(q != NULL && (q - p) < sizeof(sender))

                {

                    // Calculate sender number length
                    int len = q - p;

                    // Copy sender number into local buffer
                    strncpy(sender, p, len);

                    // Add NULL character at end of string
                    sender[len] = '\0';

                }

            }

        }

        // ================= GET MESSAGE BODY =================

        // Find first newline after CMGR header
        msg = strstr(buff, "\r\n");

        if(msg != NULL)

        {

            // Move pointer to actual SMS message content
            msg = strstr(msg + 2, "\r\n");

            if(msg != NULL)

                msg += 2;          // Skip newline characters

        }

        // Exit if sender number or message is invalid
        if(msg == NULL || strlen(sender) < 10)

            return;

        // ================= CHECK AUTHORIZED NUMBER =================

        // Get sender mobile number length
        sender_len = strlen(sender);

        // Point to last 10 digits of sender number
        num_ptr = sender + (sender_len - 10);

        // Compare received number with registered number
        if(strncmp(num_ptr, stored_num, 10) != 0)

        {

            char alert_msg[30];    // Buffer for unauthorized access alert message

            // Display unauthorized number on LCD
            CmdLCD(0x01);

            StrLCD("WRONG NUM:");

            CmdLCD(0xC0);

            StrLCD((u8*)num_ptr);

            delay_ms(3000);

            // Create alert message "ALERT:XXXXXXXXXX"

            alert_msg[0] = 'A';

            alert_msg[1] = 'L';

            alert_msg[2] = 'E';

            alert_msg[3] = 'R';

            alert_msg[4] = 'T';

            alert_msg[5] = ':';

            // Copy unauthorized number into alert message
            for(j = 0; j < 10; j++)

            {

                alert_msg[6 + j] = num_ptr[j];

            }

            // Add NULL terminator
            alert_msg[16] = '\0';

            // Send alert SMS to registered number
            send_sms(stored_num, alert_msg);

            // Clear UART buffer
            i = 0;

            memset(buff, '\0', sizeof(buff));

            // Delete processed SMS from SIM memory
            UART0_Str("AT+CMGD=");

            UART0_Int(sms_index);

            UART0_Str("\r\n");

            gsm_wait_for("OK", 3000);

            return;

        }

        // ================= CHECK PASSWORD =================

        // Verify first four characters as password
        if(strncmp(msg, stored_pass, 4) != 0)

        {

            // Send wrong password response
            send_sms(stored_num, "WRONG PASSWORD");

            // Clear buffer
            i = 0;

            memset(buff, '\0', sizeof(buff));

            // Delete SMS
            UART0_Str("AT+CMGD=");

            UART0_Int(sms_index);

            UART0_Str("\r\n");

            gsm_wait_for("OK", 3000);

            return;

        }

        // ================= CHECK FORMAT =================

        // Check whether command contains '$' separator
        if(strchr(msg, '$') == NULL)

        {

            // Send format error response
            send_sms(stored_num, "WRONG FORMAT/PASSWORD");

            i = 0;

            memset(buff, '\0', sizeof(buff));

            UART0_Str("AT+CMGD=");

            UART0_Int(sms_index);

            UART0_Str("\r\n");

            gsm_wait_for("OK", 3000);

            return;

        }
              // ================= COMMAND PROCESS =================

        // Check command character after password and separator
        // msg[4] decides which operation needs to be performed

        if(msg[4] == 'T')   // Temperature setpoint change command
        {

            // Copy 2 digit temperature value from SMS
            temp_val[0] = msg[5];

            temp_val[1] = msg[6];

            // Add NULL terminator to make it a string
            temp_val[2] = '\0';

            // Store new temperature setpoint into EEPROM
            i2c_eeprom_pagewrite(0x50, ADDR_TEMP, temp_val, 2);

            // Send confirmation SMS
            send_sms(stored_num, "TEMP UPDATED");

        }

        else if(msg[4] == 'H')   // Humidity setpoint change command
        {

            // Copy 2 digit humidity value from SMS
            hum_val[0] = msg[5];

            hum_val[1] = msg[6];

            // Add NULL terminator
            hum_val[2] = '\0';

            // Store new humidity setpoint into EEPROM
            i2c_eeprom_pagewrite(0x50, ADDR_HUM, hum_val, 2);

            // Send confirmation SMS
            send_sms(stored_num, "HUMIDITY UPDATED");

        }

        else if(msg[4] == 'M')   // Mobile number change command
        {

            // Copy new 10 digit mobile number from SMS
            for(j = 0; j < 10; j++)

                new_num[j] = msg[5 + j];

            // Add NULL terminator
            new_num[10] = '\0';

            // Store new mobile number into EEPROM
            i2c_eeprom_pagewrite(0x50, ADDR_MOB, new_num, 10);

            // Update mobile number in RAM also
            strncpy(stored_num, new_num, 10);

            stored_num[10] = '\0';

            // Send confirmation SMS
            send_sms(stored_num, "NUMBER UPDATED");

        }

        else if(msg[4] == 'I')   // Sensor information request command
        {

            // Send request signal to DHT11 sensor
            dht11_request();

            // Wait for DHT11 response
            dht11_response();

            // Read humidity integer value
            h = dht11_data();

            // Read humidity decimal value (not stored)
            dht11_data();

            // Read temperature integer value
            t = dht11_data();

            // Read temperature decimal value (not stored)
            dht11_data();

            // Read checksum value (not stored)
            dht11_data();

            // Create temperature response SMS

            msg_send[0] = 'T';

            msg_send[1] = ':';

            // Convert temperature tens digit into ASCII
            msg_send[2] = (t / 10) + '0';

            // Convert temperature units digit into ASCII
            msg_send[3] = (t % 10) + '0';

            // Remaining message formatting continues...

            msg_send[9] = '\0';

            // Send sensor information SMS
            send_sms(stored_num, msg_send);

        }

        // ================= DELETE SMS =================

        // Clear UART receive buffer
        i = 0;

        memset(buff, '\0', sizeof(buff));

        // Delete processed SMS from SIM memory
        UART0_Str("AT+CMGD=");

        // Send SMS index to delete
        UART0_Int(sms_index);

        UART0_Str("\r\n");

        // Wait for delete confirmation
        if(gsm_wait_for("OK", 3000))

        {

            // Reset SMS index after successful deletion
            sms_index = 1;

        }

    }

}

// Function : send_sms_temp()
// Description:
// Sends temperature alert SMS along with RTC time and date information.
// The SMS contains:
// - Custom message
// - Current time
// - Current date
// - Temperature value

void send_sms_temp(char *num,char*msg,int t,float tf)
{

    // Combine integer and floating temperature parts
    float temp=t+tf;

    // Clear UART buffer before SMS transmission
    i = 0;

    memset(buff, '\0', sizeof(buff));

    // Send SMS command with destination number
    UART0_Str("AT+CMGS=\"");

    UART0_Str(num);

    UART0_Str("\"\r\n");

    // Wait for GSM SMS input prompt
    if(!gsm_wait_for(">", 7000))
    {

        CmdLCD(0x01);

        StrLCD("NO PROMPT");

        return;

    }

    // Send alert message text
    UART0_Str(msg);

    // Add current time information
    UART0_Str(" TIME:");

    // Send hour
    UART0_Tx((hour/10)+48);

    UART0_Tx((hour%10)+48);

    UART0_Tx(':');

    // Send minute
    UART0_Tx((minute/10)+48);

    UART0_Tx((minute%10)+48);

    UART0_Tx(':');

    // Send second
    UART0_Tx((second/10)+48);

    UART0_Tx((second%10)+48);

    // Add current date information
    UART0_Str(" DATE:");

    UART0_Tx((date/10)+48);

    UART0_Tx((date%10)+48);

    UART0_Tx('/');

    UART0_Tx((month/10)+48);

    UART0_Tx((month%10)+48);

    UART0_Tx('/');

    // Send complete year value
    UART0_Int(year);

    // Add temperature value
    UART0_Str(" TEMP:");

    // Send floating point temperature
    UART0_Float(temp);

    UART0_Str("C");

    UART0_Str("  ");

    // Send CTRL+Z character
    // This tells GSM module that SMS content is completed
    UART0_Tx(26);

    // Wait for SMS transmission confirmation
    if(gsm_wait_for("OK", 20000))

    {

        CmdLCD(0x01);

        StrLCD("TEMP SMS SENT");

        delay_ms(2000);

    }

    // Check GSM ERROR response
    else if(strstr(buff, "ERROR"))

    {
        CmdLCD(0x01);
        StrLCD("TEMP SMS ERROR");
    }
    // No success or error response
    else
    {
        CmdLCD(0x01);

        StrLCD("TEMP SMS FAIL");
    }
    // Clear UART buffer after sending SMS
    i = 0;
    memset(buff, '\0', sizeof(buff));
}

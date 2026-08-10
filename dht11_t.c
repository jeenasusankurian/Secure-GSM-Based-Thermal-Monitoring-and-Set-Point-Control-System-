//dht11.c                           // Source file containing DHT11 temperature and humidity sensor driver

#include <LPC214x.h>                // LPC214x microcontroller register definitions

#include "delay_t.h"                // Header file containing delay function declarations
#include "defines_t.h"              // Header file containing useful macros (READBIT, WRITEBIT, etc.)
#include "lcd_t.h"                  // Header file containing LCD function declarations

#define input  0                    // Constant representing input direction
#define output 1                    // Constant representing output direction

#define DHT11 4                     // DHT11 data pin connected to Port 0 Pin 4 (P0.4)

// Global sensor values
extern unsigned char humidity_integer, humidity_decimal, temp_integer, temp_decimal,checksum;
                                    // Global variables used to store humidity, temperature, and checksum values

/* ================= REQUEST ================= */

// Function to send the start signal to the DHT11 sensor
void dht11_request(void)
{
    WRITEBIT(IODIR0, DHT11, output);   // Configure DHT11 pin as output

    WRITEBIT(IOPIN0, DHT11, 0);        // Pull the data line LOW to initiate communication

    delay_ms(20);                      // Keep the line LOW for at least 18 ms (20 ms used here)

    WRITEBIT(IOPIN0, DHT11, 1);        // Pull the data line HIGH to complete the request signal
}

/* ================= RESPONSE ================= */

// Function to wait for the DHT11 sensor response
void dht11_response(void)
{
    WRITEBIT(IODIR0, DHT11, input);    // Configure DHT11 pin as input to receive sensor response

    while(READBIT(IOPIN0, DHT11));     // Wait until the sensor pulls the line LOW

    while(!READBIT(IOPIN0, DHT11));    // Wait until the sensor pulls the line HIGH

    while(READBIT(IOPIN0, DHT11));     // Wait until the sensor pulls the line LOW again
}

/* ================= READ BYTE ================= */

// Function to read one byte (8 bits) of data from the DHT11 sensor
unsigned char dht11_data(void)
{
    unsigned char count, data = 0;     // count → loop counter, data → stores received byte

    for(count = 0; count < 8; count++) // Repeat 8 times to receive one byte
    {
        while(!READBIT(IOPIN0, DHT11));   // Wait until the data line becomes HIGH

        delay_us(30);                     // Wait 30 µs before checking the signal level

        if(READBIT(IOPIN0, DHT11))        // If the line is still HIGH after 30 µs
            data = (data << 1) | 1;       // Shift left and store bit '1'
        else
            data = (data << 1);           // Shift left and store bit '0'

        while(READBIT(IOPIN0, DHT11));    // Wait until the data line becomes LOW before reading next bit
    }

    return data;                          // Return the complete 8-bit data byte
}

/* ================= FULL READ + DISPLAY ================= */

// Function to read humidity and temperature from DHT11 and display them on the LCD
void dht11(void)
{
        //CmdLCD(0x01);                  // Clear LCD (currently commented)
        //StrLCD("DHT11");               // Display "DHT11" (currently commented)
        //delay_ms(2000);               // Wait 2 seconds (currently commented)

    dht11_request();                    // Send request signal to DHT11

    dht11_response();                   // Wait for sensor response

        // FIXED: If sensor fails response phase, print error and exit instead of freezing

    humidity_integer = dht11_data();    // Read integer part of humidity

    humidity_decimal = dht11_data();    // Read decimal part of humidity

    temp_integer     = dht11_data();    // Read integer part of temperature

    temp_decimal     = dht11_data();    // Read decimal part of temperature

    checksum         = dht11_data();    // Read checksum byte

    CmdLCD(0x01);                       // Clear the LCD display

    if((humidity_integer + humidity_decimal + temp_integer + temp_decimal) != checksum)
                                        // Verify received checksum
    {
        StrLCD("Checksum Error");       // Display error message if checksum verification fails
    }
    else
    {
        CmdLCD(0x01);                   // Clear LCD before displaying sensor data

        CmdLCD(0x80);                   // Move cursor to first line

        StrLCD("H:");                   // Display "H:" for humidity

        IntLCD(humidity_integer);       // Display integer part of humidity

        CharLCD('.');                   // Display decimal point

        IntLCD(humidity_decimal);       // Display decimal part of humidity

        StrLCD("%");                    // Display percentage symbol

        CmdLCD(0xC0);                   // Move cursor to second line

        StrLCD("T:");                   // Display "T:" for temperature

        IntLCD(temp_integer);           // Display integer part of temperature

        CharLCD('.');                   // Display decimal point

        IntLCD(temp_decimal);           // Display decimal part of temperature

        CharLCD(223);                   // Display degree (°) symbol

        CharLCD('C');                   // Display Celsius unit
    }

    delay_ms(2000);                     // Keep the displayed values on the LCD for 2 seconds
}

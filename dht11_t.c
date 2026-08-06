//dht11.c

#include <LPC214x.h>

#include "delay_t.h"
#include "defines_t.h"
#include "lcd_t.h"     // IMPORTANT (was missing)

#define input  0
#define output 1

#define DHT11 4   // P0.4

// Global sensor values
extern unsigned char humidity_integer, humidity_decimal, temp_integer, temp_decimal,checksum;

/* ================= REQUEST ================= */
void dht11_request(void)
{
    WRITEBIT(IODIR0, DHT11, output);   // Set pin as output

    WRITEBIT(IOPIN0, DHT11, 0);        // LOW for start signal
    delay_ms(20);                      // Minimum 18ms

    WRITEBIT(IOPIN0, DHT11, 1);        // HIGH
}

/* ================= RESPONSE ================= */
void dht11_response(void)
{
    WRITEBIT(IODIR0, DHT11, input);    // Set pin as input

    while(READBIT(IOPIN0, DHT11));     // Wait for LOW
    while(!READBIT(IOPIN0, DHT11));    // Wait for HIGH
    while(READBIT(IOPIN0, DHT11));     // Wait for LOW
}
  /*
  int dht11_response(void)
{
    unsigned int timeout;
    WRITEBIT(IODIR0, DHT11, input);    // Set pin as input

    // Safety Timeout 1: Wait for pin to go LOW
    timeout = 10000;
    while(READBIT(IOPIN0, DHT11)) {
        if(--timeout == 0) return 0; // Sensor failed to respond
    }
    
    // Safety Timeout 2: Wait for pin to go HIGH
    timeout = 10000;
    while(!READBIT(IOPIN0, DHT11)) {
        if(--timeout == 0) return 0;
    }
    
    // Safety Timeout 3: Wait for pin to go LOW again
    timeout = 10000;
    while(READBIT(IOPIN0, DHT11)) {
        if(--timeout == 0) return 0;
    }
    
    return 1; // Response OK
}*/
/* ================= READ BYTE ================= */
unsigned char dht11_data(void)
{
    unsigned char count, data = 0;

    for(count = 0; count < 8; count++)
    {
        while(!READBIT(IOPIN0, DHT11));   // Wait for HIGH

        delay_us(30);   // Check after 30us

        if(READBIT(IOPIN0, DHT11))
            data = (data << 1) | 1;
        else
            data = (data << 1);

        while(READBIT(IOPIN0, DHT11));   // Wait for LOW
    }

    return data;
}

/* ================= FULL READ + DISPLAY ================= */
void dht11(void)
{
	//CmdLCD(0x01);
	//StrLCD("DHT11");
	//delay_ms(2000);
    dht11_request();
    dht11_response();
	// FIXED: If sensor fails response phase, print error and exit instead of freezing
    /*if(!dht11_response())
    {
        CmdLCD(0x01);
        CmdLCD(0x80);
        StrLCD("Sensor Offline");
        delay_ms(1000);
        return; 
    }  */
    humidity_integer = dht11_data();
    humidity_decimal = dht11_data();
    temp_integer     = dht11_data();
    temp_decimal     = dht11_data();
    checksum         = dht11_data();

    CmdLCD(0x01);

    if((humidity_integer + humidity_decimal + temp_integer + temp_decimal) != checksum)
    {
        StrLCD("Checksum Error");
    }
    else
    {
	CmdLCD(0x01);
        CmdLCD(0x80);
        StrLCD("H:");
        IntLCD(humidity_integer);
        CharLCD('.');
        IntLCD(humidity_decimal);
        StrLCD("%");

        CmdLCD(0xC0);
        StrLCD("T:");
        IntLCD(temp_integer);
        CharLCD('.');
        IntLCD(temp_decimal);
        CharLCD(223);   // Degree symbol
        CharLCD('C');
    }

    delay_ms(2000);
}
/*

#include <lpc214x.h>
#include "delay.h"
#include "defines.h"

#define input 0
#define output 1

#define DHT11 4 //DHT11 data pin connected to P0.4


void dht11_request(void)
{
        WRITEBIT(IODIR0,DHT11,output);  // Configure DHT11 pin as output (P0.4 used here)
        WRITEBIT(IOPIN0,DHT11,0); // Make DHT11 pin LOW for minimum 18 seconds
        delay_ms(20);
        WRITEBIT(IOPIN0,DHT11,1); // Make DHT11 pin HIGH and wait for response
}

void dht11_response(void)
{
        WRITEBIT(IODIR0,DHT11,input);   // Configure DHT11 pin as input 
        while(READBIT(IOPIN0,DHT11) == 1); // Wait till response is HIGH 
        while(READBIT(IOPIN0,DHT11) == 0); // Wait till response is LOW 
        while(READBIT(IOPIN0,DHT11) == 1); // Wait till response is HIGH  & This is end of response
}

unsigned char dht11_data(void)
{
        unsigned char count;
        unsigned char data = 0;
        for(count = 0; count<8 ; count++)       // 8 bits of data 
        {
                while(READBIT(IOPIN0,DHT11) == 0);      // Wait till response is LOW 
                delay_us(30);   // delay greater than 24 usec 
                if (READBIT(IOPIN0,DHT11)) // If response is HIGH, 1 is received 
                        data = ( (data<<1) | 0x01 );
                else    // If response is LOW, 0 is received 
                        data = (data<<1);
                while(READBIT(IOPIN0,DHT11));   // Wait till response is HIGH (happens if 1 is received)
        }
        return data;
}
  */



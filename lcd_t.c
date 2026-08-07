#include<LPC21xx.h>                 // Contains LPC21xx microcontroller register definitions
#include"delay.h"                   // Header file containing delay function declarations
#include"lcd.h"                     // Header file containing LCD function declarations
#include"types.h"                   // Header file containing user-defined data types (u8, s32, f32, etc.)
#include"defines.h"                 // Header file containing useful macros (e.g., WRITEBYTE)

#define LCD_DAT 0xff                // LCD data lines connected to P0.8–P0.15 (8-bit data bus)
#define RS 5                        // RS (Register Select) pin connected to P0.5
#define RW 7                        // RW (Read/Write) pin connected to P0.7
#define EN 6                        // EN (Enable) pin connected to P0.6

// Function to initialize the LCD
void InitLCD(void)
{
        // P0.0–P0.10 are GPIO pins by default after reset

        IODIR0|=((LCD_DAT<<8)|(1<<RS)|(1<<RW)|(1<<EN));
                                    // Configure LCD data pins (P0.8–P0.15)
                                    // and control pins (RS, RW, EN) as output

        // P0.0–P0.10 pins are configured as output direction

        delay_ms(20);               // Wait at least 15 ms after power-on for LCD stabilization

        CmdLCD(0x30);               // Send Function Set command (8-bit mode)
        delay_ms(10);               // Wait at least 5 ms

        CmdLCD(0x30);               // Send Function Set command again
        delay_ms(1);                // Wait at least 160 µs

        CmdLCD(0x30);               // Send Function Set command third time
        delay_ms(1);                // Wait at least 160 µs

        CmdLCD(0x38);               // Configure LCD:
                                    // 8-bit mode, 2-line display, 5×7 font

        //CmdLCD(0x10);             // Display OFF command (currently not used)

        CmdLCD(0x01);               // Clear the LCD display

        CmdLCD(0x06);               // Cursor moves to the right after each character

        CmdLCD(0x0f);               // Display ON, Cursor ON, Cursor blinking ON
}

// Function to send a command to the LCD
void CmdLCD(u8 cmd)
{
        IOCLR0=1<<RS;               // RS = 0 → Command register selected

        DispLCD(cmd);               // Send command to LCD
}

// Function to send one character to the LCD
void CharLCD(u8 dat)
{
        IOSET0=1<<RS;               // RS = 1 → Data register selected

        DispLCD(dat);               // Send character to LCD
}

// Common function used to transfer command/data to LCD
void DispLCD(u8 val)
{
        IOCLR0=1<<RW;               // RW = 0 → Write operation

        WRITEBYTE(IOPIN0,8,val);    // Write 8-bit value to LCD data pins (P0.8–P0.15)

        IOSET0=1<<EN;               // Make Enable pin HIGH to start data transfer

        delay_ms(2);                // Wait so LCD can latch the data
                                    // (Minimum Enable pulse width is about 450 ns)

        IOCLR0=1<<EN;               // Make Enable pin LOW to complete transfer

        delay_ms(5);                // Wait until LCD finishes processing the command/data
}

// Function to display a string on the LCD
void StrLCD(u8 *ptr)
{
        while(*ptr!='\0')           // Continue until end of string
                CharLCD(*ptr++);    // Display current character and move to next
}

// Function to display an integer number on the LCD
void IntLCD(s32 num)
{
        u8 a[10];                   // Array used to store individual digits
        s8 i=0;                     // Index variable

        // Integer to ASCII conversion (itoa logic)

        if(num==0)                  // Special case when number is zero
                CharLCD('0');       // Display '0'
        else
        {
                if(num<0)           // Check whether number is negative
                {
                        num=-num;   // Convert negative number to positive
                        CharLCD('-'); // Display minus sign
                }

                while(num>0)        // Extract each digit
                {
                        a[i++]=num%10+48;
                                    // Convert digit into ASCII and store
                        num=num/10; // Remove last digit
                }

                for(--i;i>=0;i--)   // Print digits in reverse order
                        CharLCD(a[i]);
        }
}

// Function to display a floating-point number on the LCD
void FltLCD(f32 val)
{
        u32 num,i;                  // Variables for integer part and loop counter

        if(val<0)                   // Check whether value is negative
        {
                val=-val;           // Convert to positive
                CharLCD('-');       // Display minus sign
        }

        num=val;                    // Extract integer part

        IntLCD(num);                // Display integer part

        CharLCD('.');               // Display decimal point

        for(i=0;i<6;i++)            // Display six digits after decimal point
        {
                val=(val-num)*10;   // Shift next decimal digit to integer position

                num=val;            // Extract next digit

                IntLCD(num);        // Display extracted digit
        }
}

// Function to store a custom character pattern into LCD CGRAM
void StoreCustCharFont(void)
{
        u8 i,LUT[]={0x00,0x00,0x04,0x0c,0x1c,0x1c,0x1c,0x00};
                                    // Custom character pixel pattern (8 bytes)

        for(i=0;i<8;i++)            // Write all 8 bytes into CGRAM
                CharLCD(LUT[i]);    // Send one byte of custom character pattern
}
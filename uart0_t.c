//uart0_t.c                         // Source file containing UART0 driver functions for LPC21xx

#include <LPC21xx.h>                // Contains LPC21xx microcontroller register definitions

#include <string.h>                 // Provides string functions like strstr(), strchr()

#include <stdlib.h>                 // Provides standard library functions like atoi()

#include "uart0_t.h"                // Header file containing UART0 function declarations and macros


// External buffer used for storing received GSM/SMS data
extern char buff[200];              // Buffer declared in another file to store received UART characters


// Global variables

unsigned char ch;                   // Variable used to store received UART character

extern unsigned char i;             // External index variable used for buffer position tracking

extern unsigned char r_flag;        // External receive flag used to indicate SMS reception

extern int sms_index;                // External variable used to store SMS memory index


char dummy;                         // Dummy variable used to clear unwanted UART interrupt values



/* ================= UART0 ISR ================= */

// UART0 Interrupt Service Routine
// This function executes automatically whenever UART0 generates an interrupt

void UART0_isr(void) __irq
{
    // Check whether interrupt source is Receive Data Available
    if((U0IIR & 0x0E) == 0x04)
    {
        ch = U0RBR;                 // Read received character from UART Receiver Buffer Register


        // Ensure received data does not exceed buffer size
        if(i < (sizeof(buff) - 1))
        {
            buff[i++] = ch;         // Store received character into buffer and increment index


            buff[i] = '\0';         // Add NULL character to make buffer a valid C string


            // Check whether received data contains GSM SMS indication "+CMTI"
            if(strstr(buff, "+CMTI"))
            {
                char *p;             // Pointer variable used to locate comma position


                p = strchr(buff, ',');
                                     // Find comma character in received SMS notification


                if(p != NULL)        // Check whether comma exists
                {
                    sms_index = atoi(p + 1);
                                     // Convert characters after comma into integer SMS index
                }


                r_flag = 1;          // Set receive flag to indicate new SMS available
            }
        }


        else                         // Buffer is full
        {
            i = 0;                   // Reset buffer index to prevent overflow

            buff[i] = '\0';          // Clear buffer by adding NULL character
        }
    }


    else
    {
        dummy = U0IIR;               // Read interrupt identification register to clear interrupt
    }


    VICVectAddr = 0;                 // Acknowledge interrupt completion to VIC controller
}



/* ================= UART INIT ================= */

// Function to initialize UART0 communication

void InitUART0(void)
{
    // Configure P0.0 as RXD0 and P0.1 as TXD0
    PINSEL0 |= 0x00000005;           // Select UART0 alternate functions for P0.0 and P0.1


    // Configure UART frame format
    // 8 data bits, No parity, 1 stop bit
    U0LCR = 0x83;                    // Enable DLAB bit to configure baud rate


    // Set baud rate to 9600 bps
    U0DLL = DIVISOR & 0xFF;          // Load lower 8 bits of baud divisor

    U0DLM = (DIVISOR >> 8) & 0xFF;   // Load upper 8 bits of baud divisor


    U0LCR = 0x03;                    // Clear DLAB bit and enable normal UART operation


#if UART_INT_ENABLE > 0

    // Configure VIC for UART0 interrupt

    VICIntSelect = 0x00000000;       // Select IRQ mode (not FIQ)


    VICVectAddr0 = (unsigned)UART0_isr;
                                     // Store UART0 ISR address in VIC vector slot 0


    VICVectCntl0 = 0x20 | 6;         // Enable vector slot and assign UART0 interrupt channel number


    VICIntEnable = 1 << 6;           // Enable UART0 interrupt in VIC


    // Enable Receive Data Available interrupt
    U0IER = 0x01;

#endif
}



/* ================= TRANSMIT ================= */

// Function to transmit one character through UART0

void UART0_Tx(char ch)
{
    while(!(U0LSR & 0x20));          // Wait until Transmit Holding Register becomes empty


    U0THR = ch;                      // Load character into Transmit Holding Register
}



/* ================= RECEIVE ================= */

// Function to receive one character from UART0

char UART0_Rx(void)
{
    while(!(U0LSR & 0x01));          // Wait until receive data is available


    return U0RBR;                    // Return received character
}



/* ================= STRING SEND ================= */

// Function to transmit a complete string through UART0

void UART0_Str(char *s)
{
    while(*s)                        // Continue until NULL character is reached
        UART0_Tx(*s++);              // Send each character one by one
}



/* ================= INTEGER SEND ================= */

// Function to transmit an integer value through UART0

void UART0_Int(unsigned int n)
{
    char a[10];                      // Array used to store individual digits

    int i = 0;                       // Index variable


    if(n == 0)                       // Special case when number is zero
    {
        UART0_Tx('0');               // Transmit character '0'

        return;                      // Exit function
    }


    while(n > 0)                     // Extract digits from number
    {
        a[i++] = (n % 10) + '0';     // Convert digit into ASCII and store

        n /= 10;                     // Remove last digit
    }


    for(i = i - 1; i >= 0; i--)      // Send digits in reverse order
        UART0_Tx(a[i]);              // Transmit each digit
}



/* ================= FLOAT SEND ================= */

// Function to transmit floating point values through UART0

void UART0_Float(float f)
{
    int x;                           // Variable for integer part

    float temp;                      // Variable for decimal calculation


    x = (int)f;                      // Extract integer part from float


    UART0_Int(x);                    // Send integer part


    UART0_Tx('.');                   // Send decimal point


    temp = (f - x) * 100;             // Extract two decimal digits


    x = (int)temp;                   // Convert decimal part into integer


    UART0_Int(x);                    // Send decimal digits
}

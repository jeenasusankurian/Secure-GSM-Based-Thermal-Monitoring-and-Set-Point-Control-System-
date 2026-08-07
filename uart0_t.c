//uart0_t.c
#include <LPC21xx.h>
#include <string.h>
#include <stdlib.h>
#include "uart0_t.h"

// External buffer (used in GSM)
extern char buff[200];

// Global variables
unsigned char ch;
extern unsigned char i;
extern unsigned char r_flag;
extern int sms_index;

char dummy;

/* ================= UART0 ISR ================= */
void UART0_isr(void) __irq
{
    // Check if Receive Interrupt
    if((U0IIR & 0x0E) == 0x04)
    {
        ch = U0RBR;   // Read received character

        // Ensure we do not overflow the 200-byte array
        if(i < (sizeof(buff) - 1))
        {
            buff[i++] = ch;
            buff[i] = '\0';   // String null terminator safety

            // Detect new SMS indication safely inside character arrival
            if(strstr(buff, "+CMTI"))
            {
                char *p;
                p = strchr(buff, ',');
                if(p != NULL)
                {
                    sms_index = atoi(p + 1);
                }
                r_flag = 1;
            }
        }
        else
        {
            i = 0;   // CRITICAL: Prevent memory overflow if buffer gets full
            buff[i] = '\0';
        }
    }
    else
    {
        dummy = U0IIR; // Clear interrupt
    }

    VICVectAddr = 0; // Acknowledge interrupt
}
/*void UART0_isr(void) __irq
{
    // Check if Receive Interrupt
    //if((U0IIR & 0x04))
    if((U0IIR & 0x0E) == 0x04)
    {
        ch = U0RBR;   // Read received character

        //if(i < 199)
	if(i < (sizeof(buff) - 1))
        {
            buff[i++] = ch;
            buff[i] = '\0';   // VERY IMPORTANT (string safety)
}	
            
	     if(strstr(buff, "+CMTI"))
	     {
		     char *p;
		     p = strchr(buff, ',');
		     if(p != NULL)
		     {
			     // sms_index = *(p + 1) - '0';
			     sms_index = atoi(p + 1);
		     }
		r_flag = 1;
	     }
    }
    else
    {
        dummy = U0IIR; // Clear interrupt
    }

    VICVectAddr = 0; // Acknowledge interrupt
}
				*/
/* ================= UART INIT ================= */
void InitUART0(void)
{
    // Configure P0.0 (RXD0) and P0.1 (TXD0)
    PINSEL0 |= 0x00000005;

    // 8-bit, No parity, 1 stop bit
    U0LCR = 0x83;

    // Baud rate = 9600
    U0DLL = DIVISOR & 0xFF;
    U0DLM = (DIVISOR >> 8) & 0xFF;

    U0LCR = 0x03; // DLAB = 0

#if UART_INT_ENABLE > 0

    // Configure VIC for UART0
    VICIntSelect = 0x00000000;       // IRQ
    VICVectAddr0 = (unsigned)UART0_isr;
    VICVectCntl0 = 0x20 | 6;         // UART0 interrupt channel
    VICIntEnable = 1 << 6;

    // Enable RX interrupt
    U0IER = 0x01;

#endif
}

/* ================= TRANSMIT ================= */
void UART0_Tx(char ch)
{
    while(!(U0LSR & 0x20));  // Wait for THR empty
    U0THR = ch;
}

/* ================= RECEIVE ================= */
char UART0_Rx(void)
{
    while(!(U0LSR & 0x01));  // Wait for data
    return U0RBR;
}

/* ================= STRING SEND ================= */
void UART0_Str(char *s)
{
    while(*s)
        UART0_Tx(*s++);
}

/* ================= INTEGER SEND ================= */
void UART0_Int(unsigned int n)
{
    char a[10];
    int i = 0;

    if(n == 0)
    {
        UART0_Tx('0');
        return;
    }

    while(n > 0)
    {
        a[i++] = (n % 10) + '0';
        n /= 10;
    }

    for(i = i - 1; i >= 0; i--)
        UART0_Tx(a[i]);
}

/* ================= FLOAT SEND ================= */
void UART0_Float(float f)
{
    int x;
    float temp;

    x = (int)f;
    UART0_Int(x);

    UART0_Tx('.');

    temp = (f - x) * 100;
    x = (int)temp;

    UART0_Int(x);
}

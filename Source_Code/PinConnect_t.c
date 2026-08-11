//PinConnect.c                      // Source file containing pin function configuration routine

#include<LPC21xx.h>                 // Includes LPC21xx microcontroller register definitions

// Function to configure the function of a particular pin
void CfgPinFunc(int PortNo,int PinNo,int Func)
{
        if(PortNo==0)               // Check whether the selected port is Port 0
        {
                if(PinNo<16)        // Check whether the pin number belongs to P0.0–P0.15
                        PINSEL0=((PINSEL0&~(3<<(PinNo*2)))|(Func<<(PinNo*2)));
                                    // Clear the existing 2-bit function field of the selected pin
                                    // and write the new function value into PINSEL0

                else                // Otherwise, the pin belongs to P0.16–P0.31
                        PINSEL1=((PINSEL1&~(3<<((PinNo-16)*2)))|(Func<<((PinNo-16)*2)));
                                    // Clear the existing 2-bit function field of the selected pin
                                    // and write the new function value into PINSEL1
        }
        else                        // If Port 1 is selected
        {
                // Dummy block
                // No implementation is provided because LPC2148 pin function
                // configuration is required only for Port 0 in this program
        }
}

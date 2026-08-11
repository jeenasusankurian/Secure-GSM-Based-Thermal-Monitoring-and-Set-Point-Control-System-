
//delay.c                      // Source file containing delay function implementations

#include "types.h"             // Includes the user-defined header file for required data types (e.g., unsigned int)

// Function to generate a delay in microseconds (µs)
void delay_us(unsigned int tdly)
{
        tdly*=12;              // Convert the given microsecond value into loop iterations
                                // (Approximation based on processor clock speed)

        while(tdly--);         // Execute an empty loop until tdly becomes 0
                                // Each iteration consumes CPU time, creating the delay
}

// Function to generate a delay in milliseconds (ms)
void delay_ms(unsigned int tdly)
{
        tdly*=12000;           // Convert milliseconds into equivalent loop iterations
                                // 1 ms ≈ 12000 iterations (depends on CPU frequency)

        while(tdly--);         // Empty busy-wait loop that runs until tdly reaches 0
}

// Function to generate a delay in seconds (s)
void delay_s(unsigned int tdly)
{
        tdly*=12000000;        // Convert seconds into equivalent loop iterations
                                // 1 second ≈ 12,000,000 iterations (depends on CPU frequency)

        while(tdly--);         // Busy-wait loop to generate the required delay
}


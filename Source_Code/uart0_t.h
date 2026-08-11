//uart0.h
#define PCLK 15000000   // Adjust if different
#define BAUD 9600
#define DIVISOR (PCLK/(16*BAUD))
void InitUART0(void);
void UART0_Tx(char);
char UART0_Rx(void);
void UART0_Str(char *);
void UART0_Int(unsigned int);
void UART0_Float(float);

#define UART_INT_ENABLE 1

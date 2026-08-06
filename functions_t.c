//functions_t.c
#include <LPC21xx.h>
#include <string.h>
/*Pins P0.1 and P0.16 can be selected to
perform EINT0 function
P0.1 - FUNC4
P0.16 - FUNC2
*/
#include "delay.h"
#include "types.h"
#include "defines.h"
#include "PinConnect.h"
#include "lcd.h"
#include "KeyPd.h"
#include "i2c_eeprom_t.h"
#include "functions_t.h"

//select the one port pin which will give support for EINT0
#define EINT0_PIN      16
//assign the EINT0 channel number
#define EINT0_VIC_CHNO 14
//toggle one LED for every interrupt, so select one port pin
#define EINT0_LED      17

extern /*signed*/ char stored_num[11];
extern /*signed*/ char stored_pass[5];
extern char stored_temp[3];   // FIXED: Added explicit size 3 for temp string
extern char stored_hum[3];    // FIXED: Added explicit size 3 for hum string

extern int temp_sp;           // FIXED: Added missing temp setpoint integer link
extern int hum_sp;            // FIXED: Added missing hum setpoint integer link

volatile unsigned char menu_flag = 0;

void eint0_isr(void) __irq
{
    menu_flag = 1;   // trigger menu
    EXTINT = 1<<0;
    VICVectAddr = 0;
}

void Enable_EINT0(void)
{
        //configure P0.1/P0.16 as EINT0 input pin
        CfgPinFunc(PORT0,EINT0_PIN,FUNC2);

        //configure VIC (Vector Interrupt controller)
        //def External interrupts (EINT0/EINT1/EINT2/EINT3) is IRQ types (by using VICIntSelect SFR)

        //enable EINT0 via VIC (by using VICIntEnable SFR)
        VICIntEnable = 1<<EINT0_VIC_CHNO;
        //enable vectored irq slot 1 for EINT0 (0-15 based on requirement ? by using VICVectCntl0 SFR)
        VICVectCntl1 = ((1<<5) | EINT0_VIC_CHNO);
        //load isr address into slot 1 (0-15 based on requirement ? by using VICVectAddr0 SFR)
        VICVectAddr1 = (unsigned int)eint0_isr;

        //configure External Interrupts Peripheral
        //configure EINT0/EINT1/EINT2/EINT3 as edge triggered (use EXTMODE SFR)
        EXTMODE = 1<<0;
        //def EINT0/EINT1/EINT2/EINT3 is Falling Edge Triggerd (use EXTPOLAR SFR)
        //by default falling edge is slected
        //EXTPOLAR = 1<<0;      //for raising edge
}


/*void get_password(char *pass)
{
    int i;

    for(i = 0; i < 4; i++)
    {
        pass[i] = KeyVal();
        CharLCD('*');   // hide input
				delay_ms(200);
    }
    pass[4] = '\0';
}
*/
void get_password(char *pass)
{
    int i;

    for(i = 0; i < 4; i++)
    {
        // Wait until key pressed
        while(ColStat() == 0);

        // Read key
        pass[i] = KeyVal();

        // Display *
        CharLCD('*');

        // Wait until key released
        while(ColStat() == 1);

        // Debounce delay
        delay_ms(200);
    }

    pass[4] = '\0';
}
/*void get_input(char *data, int len)
{
    int i;

    for(i = 0; i < len; i++)
    {
        data[i] = KeyVal();
        CharLCD(data[i]);
    }
    data[len] = '\0';
}

*/


void get_input(char *data, int len)
{
    int i;

    for(i = 0; i < len; i++)
    {
        // Wait until key pressed
        while(ColStat() == 0);

        // Read key
        data[i] = KeyVal();

        // Display entered key
        CharLCD(data[i]);

        // Wait until key released
        while(ColStat() == 1);

        // Debounce delay
        delay_ms(200);
    }

    data[len] = '\0';
}

void setpoint_menu(void)
{
    char entered_pass[5];
    char new_value[4];
    int attempts = 0;
    char key;

    while(attempts < 3)
    {
        CmdLCD(0x01);
        StrLCD("Enter Pass:");

        get_password(entered_pass);   // read 4-digit password

        if(strncmp(entered_pass, stored_pass, 4) == 0)
        {
						IOSET0 = (1<<GREEN_LED);
						delay_ms(500);
						IOCLR0 = (1<<GREEN_LED);
            CmdLCD(0x01);
            StrLCD("1.Temp 2.Hum");


			 // FIXED: Automatically display current active setpoint values on Row 2
            CmdLCD(0xC0); // Move cursor to Row 2
            StrLCD("T:");
            IntLCD(temp_sp);
            StrLCD("C  H:");
            IntLCD(hum_sp);
            StrLCD("%");

            //key = KeyVal();
			// FIXED: Added blocking loop to wait for a clean menu input keypress
            while(ColStat() == 0);
            key = KeyVal();
            while(ColStat() == 1);
            delay_ms(200);
            if(key == '1')
            {
                CmdLCD(0x01);
                StrLCD("Set Temp:");

                get_input(new_value, 2);   // get 2-digit temp

                i2c_eeprom_pagewrite(0x50, ADDR_TEMP, new_value, 2);
				delay_ms(10); // FIXED: Added critical EEPROM write cycle delay
				//lets see work or not?


				strncpy(stored_temp, new_value, 2);
                stored_temp[2] = '\0';
                temp_sp = (stored_temp[0] - '0') * 10 + (stored_temp[1] - '0');

                CmdLCD(0x01);
                StrLCD("Temp Updated");
            }
            else if(key == '2')
            {
                CmdLCD(0x01);
                StrLCD("Set Hum:");

                get_input(new_value, 2);

                i2c_eeprom_pagewrite(0x50, ADDR_HUM, new_value, 2);
				delay_ms(10); // FIXED: Added critical EEPROM write cycle delay
                
//lets see work or not?

				// FIXED: Instantly update active RAM variables for main loop
                strncpy(stored_hum, new_value, 2);
                stored_hum[2] = '\0';
                hum_sp = (stored_hum[0] - '0') * 10 + (stored_hum[1] - '0');				


				CmdLCD(0x01);
                StrLCD("Hum Updated");
            }

            delay_ms(1000);
            return;
        }
        else
        {
            attempts++;

						IOSET0 = (1<<RED_LED) | (1<<BUZZER);
						delay_ms(500);
						IOCLR0 = (1<<RED_LED) | (1<<BUZZER);

            CmdLCD(0x01);
            StrLCD("Wrong Pass");

        }
    }

    // Block system
    CmdLCD(0x01);
    StrLCD("Blocked...");
    delay_ms(3000);
}

void password_change(void)
{
    char current_pass[5];
    char new_pass[5];
    char confirm_pass[5];
    int attempts = 0;

    while(attempts < 3)
    {
        CmdLCD(0x01);
        StrLCD("Curr Pass:");

        get_password(current_pass);

        if(strncmp(current_pass, stored_pass, 4) == 0)
        {
            CmdLCD(0x01);
            StrLCD("New Pass:");

            get_password(new_pass);

            CmdLCD(0x01);
            StrLCD("Confirm:");

            get_password(confirm_pass);
		   //get_password(current_pass); 
            if(strncmp(new_pass, confirm_pass, 4) == 0)
            //if(strncmp(new_pass, current_pass, 4) == 0)

			{
                i2c_eeprom_pagewrite(0x50, ADDR_PASS, new_pass, 4);
				delay_ms(10); // FIXED: Added critical EEPROM write cycle delay

                // update local copy
                //strncpy(stored_pass, new_pass, 4);

				// FIXED: Sync internal password RAM variable instantly
                strncpy(stored_pass, new_pass, 4);
                stored_pass[4] = '\0';


                CmdLCD(0x01);
                StrLCD("Pass Changed");
            }
            else
            {
                CmdLCD(0x01);
                StrLCD("Mismatch!");
            }

            delay_ms(1500);
            return;
        }
        else
        {
            attempts++;

            CmdLCD(0x01);
            StrLCD("Wrong Pass");

            IOSET0 = (1<<RED_LED) | (1<<BUZZER);
						delay_ms(500);
						IOCLR0 = (1<<RED_LED) | (1<<BUZZER);
        }
    }

    CmdLCD(0x01);
    StrLCD("Blocked...");
    delay_ms(3000);
}


void menu_function()
{
		char key;
    CmdLCD(0x01);
    StrLCD("1.Set 2.Pass");

    // Wait until key pressed
    while(ColStat() == 0);

    // Read key
    key = KeyVal();

    // Wait until key released
    while(ColStat() == 1);

    // Debounce
    delay_ms(200);
    if(key == '1')
        setpoint_menu();

    else if(key == '2')
        password_change();
}



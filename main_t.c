//main_t.c

#include <lpc21xx.h>
#include <string.h>

#include "uart0_t.h"
#include "delay_t.h"
#include "gsm_t.h"
#include "lcd_t.h"
#include "i2c_t.h"
#include "i2c_eeprom_t.h"
#include "dht11_t.h"
#include "rtc_t.h"
#include "functions_t.h"
#include "defines_t.h"
#include "types_t.h"
#include "KeyPd_t.h"

s32 hour, minute, second, date, month, year;
unsigned char humidity_integer, humidity_decimal, temp_integer, temp_decimal,checksum;

static int temp_flag =0,hum_flag = 0;

char msg[80];

extern volatile unsigned char menu_flag;

// EEPROM addresses
//#define ADDR_MOB   0x0500
//#define ADDR_PASS  0x0400

// Global variables (used in ISR + GSM)
/*signed*/ char stored_num[11];
/*signed*/ char stored_pass[5];
char stored_temp[3];   // 2 digits + null
char stored_hum[3];

/*signed*/ char default_num[11]  = "8547858297";
/*signed*/ char default_pass[5] = "0786";
char default_temp[] = "25";   // 25°C
char default_hum[]  = "60";   // 60% RH

unsigned char r_flag = 0;
extern unsigned char i;
extern char buff[200];


int temp_sp,hum_sp;

int safe_atoi_2digit(char *data)
{
    if(data[0] < '0' || data[0] > '9') return 0;
    if(data[1] < '0' || data[1] > '9') return 0;

    return (data[0]-'0')*10 + (data[1]-'0');
}

int main()
{
		IODIR0 |= (1<<BUZZER) | (1<<GREEN_LED) | (1<<RED_LED);

    // Initialize peripherals
    InitUART0();
    InitLCD();
    KeyPdInit();
    init_i2c();

   // delay_ms(3000);   // GSM module boot time

    // Enable global interrupts (IMPORTANT)
    //Enable_EINT0();
	i = 0;
   memset(buff, '\0', sizeof(buff));


    // Initialize GSM
	gsm_wait_startup();
    gsm_init();
		//interrupt enable
		Enable_EINT0();

		// RTC INIT
    RTC_Init();
    SetRTCTimeInfo(10, 0, 0);     // Set initial time (HH,MM,SS)
    SetRTCDateInfo(30, 5, 2026);   // Set date (DD,MM,YYYY)

    // Store default mobile number in EEPROM
    i2c_eeprom_pagewrite(0x50, ADDR_MOB, default_num, 10);
    delay_ms(10); 
	i2c_eeprom_seqread(0x50, ADDR_MOB, stored_num, 10);
    stored_num[10] = '\0';

    // Store default password in EEPROM
    i2c_eeprom_pagewrite(0x50, ADDR_PASS, default_pass, 4);
    delay_ms(10); 
	i2c_eeprom_seqread(0x50, ADDR_PASS, stored_pass, 4);
    stored_pass[4] = '\0';
	
		i2c_eeprom_pagewrite(0x50, ADDR_TEMP, default_temp, 2);
		 //delay_ms(10); 
		//i2c_eeprom_seqread(0x50, ADDR_TEMP, stored_temp, 2);
		//stored_temp[2] = '\0';

		i2c_eeprom_pagewrite(0x50, ADDR_HUM,  default_hum,  2);
		 //delay_ms(10); 
		//i2c_eeprom_seqread(0x50, ADDR_HUM, stored_hum, 2);
		//stored_hum[2] = '\0';

    // Display startup message
    CmdLCD(0x01);
    StrLCD("GSM SYSTEM READY");
    delay_ms(1000);

   /* CmdLCD(0x01);
StrLCD("BEFORE SMS");
delay_ms(1000);
	 */
    // Send initial SMS
    send_sms(stored_num, "SYSTEM STARTED");


  /*  CmdLCD(0x01);
StrLCD("AFTER SMS");
delay_ms(1000);
*/
		while(1)
		{
    // ================= STEP 1: READ DHT11 =================
    dht11();
		/*for(int k=0;k<3;k++)
{
    dht11();
    delay_ms(100);
}*/

		//READ SETPOINTS FROM EEPROM...........
    i2c_eeprom_seqread(0x50, ADDR_TEMP, stored_temp, 2);
    i2c_eeprom_seqread(0x50, ADDR_HUM,  stored_hum,  2);

    //temp_sp = (stored_temp[0] - '0') * 10 + (stored_temp[1] - '0');
    //hum_sp  = (stored_hum[0]  - '0') * 10 + (stored_hum[1]  - '0');
		temp_sp = safe_atoi_2digit(stored_temp);
		hum_sp  = safe_atoi_2digit(stored_hum);
    //READ RTC TIME.........
    GetRTCTimeInfo(&hour, &minute, &second);
	GetRTCDateInfo(&date, &month, &year); 
    //char time_str[10];
    //sprintf(time_str, "%02d:%02d:%02d", hour, min, sec);

    //TEMPERATURE CHECK
    //temp_flag = 0;

    if(temp_integer > temp_sp && temp_flag == 0)
    {
        temp_flag = 1;

        IOSET0 = (1 << GREEN_LED) | (1 << BUZZER);

        //char msg[80];
        //sprintf(msg, "TEMP HIGH: %dC TIME:%s", temp_integer, time_str);

        //send_sms(stored_num, msg);
			send_sms_temp(stored_num,"ALERT:HIGH TEMP",temp_integer,temp_decimal);
    }

    else if(temp_integer <= temp_sp)
    {
        temp_flag = 0;
        IOCLR0 = (1 << GREEN_LED) | (1 << BUZZER);
    }
    // ================= STEP 6: HUMIDITY CHECK =================
    //hum_flag = 0;
    if(humidity_integer > hum_sp && hum_flag == 0)
    {
        hum_flag = 1;
        IOSET0 = (1 << RED_LED) | (1 << BUZZER);
        //char msg[80];
       // sprintf(msg, "HUM HIGH: %d%% TIME:%s", humidity_integer, time_str);
        //send_sms(stored_num, msg);
			send_sms_humi(stored_num,"ALERT:HIGH HUMIDITY",humidity_integer,humidity_decimal);
    }
    else if(humidity_integer <= hum_sp)
    {
        hum_flag = 0;
        IOCLR0 = (1 << RED_LED) | (1 << BUZZER);
    }

    // ================= STEP 7: SMS RECEIVE =================
    if(r_flag)
    {
        r_flag = 0;
        receive_sms();
    }

    // ================= STEP 8: MENU INTERRUPT =================
    if(menu_flag)
    {
        menu_flag = 0;
        menu_function();
    }

    delay_ms(300);
}
}


//gsm.c

#include <string.h>
#include <stdio.h>
#include <lpc21xx.h>

#include "uart0_t.h"
#include "delay_t.h"
#include "gsm_t.h"
#include "lcd_t.h"
#include "dht11_t.h"
#include "i2c_eeprom_t.h"
#include "defines_t.h"

// Global buffer (used by UART ISR)
char buff[200];
int sender_len;
unsigned char i = 0;
unsigned char cnt = 0;
int sms_index = 1;

extern /*signed*/ char stored_num[11];
extern /*signed*/ char stored_pass[5];


extern int hour,minute,second,date,month,year;
/* ================= GSM INITIALIZATION ================= */
void gsm_wait_startup(void)
{
	i = 0;
    memset(buff, '\0', sizeof(buff));
    // Wait for MODEM:STARTUP
	StrLCD("LET'S START");
	delay_ms(1000);

    while(!strstr(buff, "MODEM:STARTUP"));

    CmdLCD(0x01);
    StrLCD("MODEM READY");
    delay_ms(1000);

    // Clear buffer
    i = 0;
    memset(buff, '\0', sizeof(buff));

    // Wait for +PBREADY
    while(!strstr(buff, "+PBREADY"));

    CmdLCD(0x01);
    StrLCD("SIM READY");
    delay_ms(1000);

    // Clear again
    i = 0;
    memset(buff, '\0', sizeof(buff));
}

/*
void gsm_wait_startup(void)
{
    if(!gsm_wait_for("MODEM:STARTUP", 15000))
    {
        CmdLCD(0x01);
        StrLCD("MODEM FAIL");
        while(1);
    }

    CmdLCD(0x01);
    StrLCD("MODEM READY");
    delay_ms(1000);

    i = 0;
    memset(buff, '\0', sizeof(buff));

    if(!gsm_wait_for("+PBREADY", 15000))
    {
        CmdLCD(0x01);
        StrLCD("SIM FAIL");
        while(1);
    }

    CmdLCD(0x01);
    StrLCD("SIM READY");
    delay_ms(1000);

    i = 0;
    memset(buff, '\0', sizeof(buff));
}
 */
void gsm_init()
{
    // Basic AT check
    /*
		UART0_Str("AT\r\n");
    delay_ms(1000);
		*/
	/*	UART0_Str("AT\r\n");
    i=0;
    memset(buff,'\0',200);
    while(i<4);
    delay_ms(500);
    buff[i]='\0';
	*/
	i = 0;
memset(buff, '\0', sizeof(buff));

UART0_Str("AT\r\n");

if(!gsm_wait_for("OK", 5000))
{
    CmdLCD(0x01);
    StrLCD("AT FAIL");
    return;
}


    // Echo OFF
    /*
		UART0_Str("ATE0\r\n");
    delay_ms(1000);
		*/
	/*	UART0_Str("ATE0\r\n");
    i=0;
		memset(buff,'\0',200);
    while(i<4);
    delay_ms(500);
    buff[i]='\0';
	*/
i = 0;
memset(buff, '\0', sizeof(buff));

UART0_Str("ATE0\r\n");

if(!gsm_wait_for("OK", 5000))
{
    CmdLCD(0x01);
    StrLCD("ATE0 FAIL");
    return;
}

    // Text mode
		/*
    UART0_Str("AT+CMGF=1\r\n");
    delay_ms(1000);
		*/
/*		UART0_Str("AT+CMGF=1\r\n");
    i=0;
    memset(buff,'\0',200);
    while(i<4);
    delay_ms(500);
    buff[i]='\0';
  */
i = 0;
memset(buff, '\0', sizeof(buff));

UART0_Str("AT+CMGF=1\r\n");

if(!gsm_wait_for("OK", 5000))
{
    CmdLCD(0x01);
    StrLCD("CMGF FAIL");
    return;
}


		// New SMS indication directly
    /*
		UART0_Str("AT+CNMI=2,1,0,0,0\r\n");
    delay_ms(1000);
		*/
/*		UART0_Str("AT+CNMI=2,1,0,0,0\r\n");
    i=0;
    memset(buff,'\0',200);
    while(i<4);
    delay_ms(500);
    buff[i]='\0';
*/
i = 0;
memset(buff, '\0', sizeof(buff));

UART0_Str("AT+CNMI=2,1,0,0,0\r\n");

if(!gsm_wait_for("OK", 5000))
{
    CmdLCD(0x01);
    StrLCD("CNMI FAIL");
    return;
}

    CmdLCD(0x01);
    StrLCD("GSM INIT DONE");
    delay_ms(1000);
}


//int gsm_wait_for(char *buf, int timeout)
  int gsm_wait_for(char *keyword, int timeout)
{
    int t = 0;
    while(t < timeout)
    {
        // Safe check: Only scan if buffer contains data
        if(i > 0)
        {
            if(strstr(buff, keyword) != NULL)
                return 1; // Found the keyword!
        }

        delay_ms(1); // Crucial: gives UART hardware a window to breathe
        t++;
    }
    return 0; // Timeout reached! Safely breaks the freeze
}

/*int gsm_wait_for(char *keyword, int timeout)
{
    int t = 0;
    while(t < timeout)
    {
        //if(strstr(buf, ">"))
				if(strstr(buff, keyword))
            return 1;

        delay_ms(1);
        t++;
    }
    return 0;
}
  */
/* ================= SEND SMS ================= */
void send_sms(/*signed*/ char *num, /*signed*/ char *msg)
{
		i = 0;
    memset(buff, '\0', sizeof(buff));
    /*
		i = 0;

    UART0_Str("AT+CMGS=\"");
    UART0_Str(num);
    UART0_Str("\"\r\n");

    delay_ms(2000);   // Wait for '>'

    UART0_Str(msg);
    UART0_Tx(0x1A);   // CTRL+Z

    delay_ms(3000);

    CmdLCD(0x01);
    StrLCD("SMS SENT");
    delay_ms(500);
		*/
	UART0_Str("AT+CMGS=");
    UART0_Tx('"');
    UART0_Str(num);
    UART0_Tx('"');
    UART0_Str("\r\n");
    //delay_ms(2000);
		// Wait for '>'
    //if(!gsm_wait_for(buff, 5000))
	if(!gsm_wait_for(">", 7000))
    {
        CmdLCD(0x01);
        StrLCD("NO PROMPT");
        return;
    }   // fail safely
	// Clear buffer index again just for the upcoming text response
    i = 0;
    buff[i] = '\0';
    UART0_Str(msg);
    UART0_Tx(0x1A);

	 // Step 5: WAIT FOR SUCCESS ?
    if(gsm_wait_for("OK", 25000))
    {
        CmdLCD(0x01);
        StrLCD("SMS SENT");
		delay_ms(2000);
    }
    else if(strstr(buff, "ERROR"))
    {
        CmdLCD(0x01);
        StrLCD("SMS ERROR");
		delay_ms(1000);
    }
    else
    {
        CmdLCD(0x01);
        StrLCD("SMS FAIL");
		delay_ms(1000);
    }
		i = 0;
		memset(buff, '\0', sizeof(buff));
		/*
    delay_ms(1000);
    i=0;
    memset(buff,'\0',200);
		buff[i]='\0';
		*/
		/*CmdLCD(0x01);
    StrLCD("SMS SENT");
    delay_ms(500);*/
}



void receive_sms()
{
    char *p, *q;
    char *num_ptr;
    char *msg;
    char sender[20];   // to store sender number
    char temp_val[3];
	char hum_val[3];
    char new_num[11];
    char msg_send[30];
    int j = 0, t = 0, h = 0;
	CmdLCD(0x01);
	StrLCD("SMS RECEIVED");
	delay_ms(1000);
    i = 0;
    memset(buff, '\0', sizeof(buff));

    UART0_Str("AT+CMGR=");
    UART0_Int(sms_index);
    UART0_Str("\r\n");

    // Wait for response
    if(!gsm_wait_for("OK", 5000))
    {
        CmdLCD(0x01);
        StrLCD("READ FAIL");
        return;
    }

    if(strstr(buff, "+CMGR:"))
    {
        // ================= NEW BULLETPROOF SENDER NUMBER EXTRACTION =================
        // Find the start of the CMGR header
        p = strstr(buff, "+CMGR:");
        sender[0] = '\0'; // Initialize as empty string

        if(p != NULL)
        {
            // Look for the comma followed by a quote which precedes the phone number
            p = strstr(p, ",\"+"); 
            if(p == NULL)
            {
                // Fallback in case country code '+' is missing in the output
                p = strstr(buff, ",\""); 
            }
            
            if(p != NULL)
            {
                p += 2; // Move past the ," or ,"+ to get to the first digit of the number
                q = strchr(p, '"'); // Find the closing quote at the end of the number

                if(q != NULL && (q - p) < sizeof(sender))
                {
                    int len = q - p;
                    strncpy(sender, p, len);
                    sender[len] = '\0'; // Properly null terminate
                }
            }
        }

        // ================= GET MESSAGE BODY =================
        // The message text starts after the first full line containing +CMGR info
        msg = strstr(buff, "\r\n");
        if(msg != NULL)
        {
            msg = strstr(msg + 2, "\r\n"); // Move past the header newline to the message body
            if(msg != NULL)
                msg += 2;
        }

        if(msg == NULL || strlen(sender) < 10)
            return;

        // ================= CHECK AUTHORIZED NUMBER (LAST 10 DIGITS) =================
        sender_len = strlen(sender);
        num_ptr = sender + (sender_len - 10); // Safe pointer to the last 10 digits

        /*if(strncmp(num_ptr, stored_num, 10) != 0)
        {
            // Debug: Show what it actually parsed on the LCD screen
            CmdLCD(0x01);
            StrLCD("WRONG NUM:");
            CmdLCD(0xC0);
            StrLCD(num_ptr);
            delay_ms(3000);

            send_sms(stored_num, "UNAUTHORIZED ACCESS");
            
            i = 0;
            memset(buff, '\0', sizeof(buff));
            UART0_Str("AT+CMGD=");
            UART0_Int(sms_index);
            UART0_Str("\r\n");
            gsm_wait_for("OK", 3000);
            return;
        } */
		if(strncmp(num_ptr, stored_num, 10) != 0)
{
    char alert_msg[30]; // Buffer to hold our alert message text

    // Debug: Keep showing what it actually parsed on the LCD screen
    CmdLCD(0x01);
    StrLCD("WRONG NUM:");
    CmdLCD(0xC0);
    StrLCD((u8*)num_ptr);
    delay_ms(3000);

    // ================= BUILD CUSTOM SMS MESSAGE =================
    // Copy the text "ALERT:" into the start of the message array
    alert_msg[0] = 'A';
    alert_msg[1] = 'L';
    alert_msg[2] = 'E';
    alert_msg[3] = 'R';
    alert_msg[4] = 'T';
    alert_msg[5] = ':';

    // Copy the 10-digit unauthorized phone number right after the colon
    for(j = 0; j < 10; j++)
    {
        alert_msg[6 + j] = num_ptr[j];
    }
    alert_msg[16] = '\0'; // Add string null-terminator for safety

    // CRITICAL FIX: Send the custom built alert message to your registered number
    send_sms(stored_num, alert_msg);
    
    // ================= CLEANUP AND EXIT =================
    i = 0;
    memset(buff, '\0', sizeof(buff));
    UART0_Str("AT+CMGD=");
    UART0_Int(sms_index);
    UART0_Str("\r\n");
    gsm_wait_for("OK", 3000);
    return;
}


        // ================= CHECK PASSWORD =================
        if(strncmp(msg, stored_pass, 4) != 0)
        {
            send_sms(stored_num, "WRONG PASSWORD");
            i = 0;
            memset(buff, '\0', sizeof(buff));
            UART0_Str("AT+CMGD=");
            UART0_Int(sms_index);
            UART0_Str("\r\n");
            gsm_wait_for("OK", 3000);
            return;
        }

        // ================= CHECK FORMAT =================
        
		if(strchr(msg, '$') == NULL)
        {
			send_sms(stored_num, "WRONG FORMAT/PASSWORD");	
            i = 0;
            memset(buff, '\0', sizeof(buff));
            UART0_Str("AT+CMGD=");
            UART0_Int(sms_index);
            UART0_Str("\r\n");
            gsm_wait_for("OK", 3000);
            return;
        }

        // ================= COMMAND PROCESS =================
        if(msg[4] == 'T')   // TEMP CHANGE
        {
            temp_val[0] = msg[5];
            temp_val[1] = msg[6];
            temp_val[2] = '\0';

            i2c_eeprom_pagewrite(0x50, ADDR_TEMP, temp_val, 2);
            send_sms(stored_num, "TEMP UPDATED");
        }
		else if(msg[4] == 'H')   // TEMP CHANGE
        {
            hum_val[0] = msg[5];
            hum_val[1] = msg[6];
            hum_val[2] = '\0';

            i2c_eeprom_pagewrite(0x50, ADDR_HUM, hum_val, 2);
            send_sms(stored_num, "HUMIDITY UPDATED");
		}
        else if(msg[4] == 'M')   // MOBILE CHANGE
        {
            for(j = 0; j < 10; j++)
                new_num[j] = msg[5 + j];

            new_num[10] = '\0';

            i2c_eeprom_pagewrite(0x50, ADDR_MOB, new_num, 10);
            strncpy(stored_num, new_num, 10);
            stored_num[10] = '\0';

            send_sms(stored_num, "NUMBER UPDATED");
        }
        else if(msg[4] == 'I')   // SENSOR INFO
        {
            dht11_request();
            dht11_response();

            h = dht11_data();
            dht11_data();
            t = dht11_data();
            dht11_data();
            dht11_data();

            msg_send[0] = 'T';
            msg_send[1] = ':';
            msg_send[2] = (t / 10) + '0';
            msg_send[3] = (t % 10) + '0';
            msg_send[4] = ' ';
            msg_send[5] = 'H';
            msg_send[6] = ':';
            msg_send[7] = (h / 10) + '0';
            msg_send[8] = (h % 10) + '0';
            msg_send[9] = '\0';
            send_sms(stored_num, msg_send);
        }

        // ================= DELETE SMS =================
        i = 0;
        memset(buff, '\0', sizeof(buff));
        UART0_Str("AT+CMGD=");
        UART0_Int(sms_index);
        UART0_Str("\r\n");
        if(gsm_wait_for("OK", 3000))
        {
            sms_index = 1;   // Reset safely
        }
    }
}
/*
void receive_sms()
{
		char *p,*q;
		char *num_ptr;
    char *msg;
    char sender[20];   // to store sender number
    char temp_val[3];
    char new_num[11];
    char msg_send[30];

    //int timeout = 0;
    int j = 0, t = 0, h = 0;

    i = 0;
    memset(buff, '\0', sizeof(buff));

   // UART0_Str("AT+CMGR=1\r\n");
	UART0_Str("AT+CMGR=");
UART0_Int(sms_index);
UART0_Str("\r\n");

    // Wait for response
  if(!gsm_wait_for("OK", 5000))
{
    CmdLCD(0x01);
    StrLCD("READ FAIL");
    return;
}
	//  while(timeout < 5000)
    //{
      //  delay_ms(1);
    //    timeout++;
  //  }
//
    if(strstr(buff, "+CMGR"))
    {
        // ================= GET SENDER NUMBER =================
        p = strstr(buff, "+CMGR:");
        if(p != NULL)
        {
            p = strchr(p, '"'); // first "
            if(p != NULL)
            {
                p = strchr(p + 1, '"'); // second "
                if(p != NULL)
                {
                    p++; // move to number
                    q = strchr(p, '"');

                    if(q != NULL)
                    {
                        int len = q - p;
                        strncpy(sender, p, len);
                        sender[len] = '\0';
                    }
                }
            }
        }

        // ================= GET MESSAGE =================
        msg = strstr(buff, "\r\n");
        if(msg != NULL)
        {
            msg = strstr(msg + 2, "\r\n");
            if(msg != NULL)
                msg += 2;
        }

        if(msg == NULL)
            return;

      
				  // ================= CHECK AUTHORIZED NUMBER (ADD HERE) =================
        sender_len = strlen(sender);
        
        // Ensure the extracted number is at least 10 digits long
        if(sender_len >= 10)
        {
            // Point directly to the START of the last 10 digits
            num_ptr = sender + (sender_len - 10);
        }
        else
        {
            num_ptr = sender; // Fallback safety
        }


				if(strncmp(num_ptr, stored_num, 10) != 0)
				{
					send_sms(stored_num, "UNAUTHORIZED ACCESS");
					//UART0_Str("AT+CMGD=1\r\n");
					i = 0;
					memset(buff, '\0', sizeof(buff));
					UART0_Str("AT+CMGD=");
					UART0_Int(sms_index);
					UART0_Str("\r\n");

					gsm_wait_for("OK", 3000);
					return;
				}

        // ================= CHECK PASSWORD =================
        if(strncmp(msg, stored_pass, 4) != 0)
        {
            send_sms(stored_num, "WRONG PASSWORD");

            //UART0_Str("AT+CMGD=1\r\n");
            //delay_ms(1000);
            					i = 0;
					memset(buff, '\0', sizeof(buff));
					UART0_Str("AT+CMGD=");
					UART0_Int(sms_index);
					UART0_Str("\r\n");
					gsm_wait_for("OK", 3000);
					return;
        }

        // ================= CHECK FORMAT =================
        if(strchr(msg, '$') == NULL)
        {
           // UART0_Str("AT+CMGD=1\r\n");
           // delay_ms(1000);
          					i = 0;
					memset(buff, '\0', sizeof(buff));
					UART0_Str("AT+CMGD=");
					UART0_Int(sms_index);
					UART0_Str("\r\n");
  				gsm_wait_for("OK", 3000);
					return;
        }

        // ================= COMMAND PROCESS =================
        if(msg[4] == 'T')   // TEMP CHANGE
        {
            temp_val[0] = msg[5];
            temp_val[1] = msg[6];
            temp_val[2] = '\0';

            i2c_eeprom_pagewrite(0x50, ADDR_TEMP, temp_val, 2);

            send_sms(stored_num, "TEMP UPDATED");
        }
        else if(msg[4] == 'M')   // MOBILE CHANGE
        {
            for(j = 0; j < 10; j++)
                new_num[j] = msg[5 + j];

            new_num[10] = '\0';

            i2c_eeprom_pagewrite(0x50, ADDR_MOB, new_num, 10);

            strncpy(stored_num, new_num, 10);
            stored_num[10] = '\0';

            send_sms(stored_num, "NUMBER UPDATED");
        }
        else if(msg[4] == 'I')   // SENSOR INFO
        {
            dht11_request();
            dht11_response();

            h = dht11_data();
            dht11_data();
            t = dht11_data();
            dht11_data();
            dht11_data();

            //sprintf(msg_send, "T:%02d H:%02d", t, h);
						msg_send[0] = 'T';
						msg_send[1] = ':';
						msg_send[2] = (t / 10) + '0';
						msg_send[3] = (t % 10) + '0';
						msg_send[4] = ' ';
						msg_send[5] = 'H';
						msg_send[6] = ':';
						msg_send[7] = (h / 10) + '0';
						msg_send[8] = (h % 10) + '0';
						msg_send[9] = '\0';
            send_sms(stored_num, msg_send);
        }

        // ================= DELETE SMS =================
        //UART0_Str("AT+CMGD=1\r\n");
        //delay_ms(1000);
				i = 0;
memset(buff, '\0', sizeof(buff));

//UART0_Str("AT+CMGD=1\r\n");
UART0_Str("AT+CMGD=");
UART0_Int(sms_index);
UART0_Str("\r\n");
//gsm_wait_for("OK", 3000);
if(gsm_wait_for("OK", 3000))
{
    sms_index = 1;   // reset safely
}
    }
}
*/


void send_sms_temp(char *num,char*msg,int t,float tf)
{
	float temp=t+tf;
//	float temp = t + (tf / 10.0);
	// Clear buffer before command
    i = 0;
    memset(buff, '\0', sizeof(buff));
	 UART0_Str("AT+CMGS=\"");
   UART0_Str(num);
   UART0_Str("\"\r\n");
		 // ? Wait for '>' prompt
    if(!gsm_wait_for(">", 7000))
    {
        CmdLCD(0x01);
        StrLCD("NO PROMPT");
        return;
    }
   //delay_ms(1000);
   UART0_Str(msg);
   UART0_Str(" TIME:");
	 UART0_Tx((hour/10)+48);
	 UART0_Tx((hour%10)+48);
	UART0_Tx(':');
	UART0_Tx((minute/10)+48);
	UART0_Tx((minute%10)+48);
	UART0_Tx(':');
	UART0_Tx((second/10)+48);
	UART0_Tx((second%10)+48);

	UART0_Str(" DATE:");
	UART0_Tx((date/10)+48);
	UART0_Tx((date%10)+48);
	UART0_Tx('/');
	UART0_Tx((month/10)+48);
	UART0_Tx((month%10)+48);
	UART0_Tx('/');
	//UART0_Tx((year/10)+48);
	//UART0_Tx((year%10)+48);
	UART0_Int(year);

	UART0_Str(" TEMP:");
	UART0_Float(temp);
  UART0_Str("C");
	UART0_Str("  ");

	UART0_Tx(26);//UART0_Tx(0x1A);

  //delay_ms(3000);
	// ? Wait for SMS confirmation
    if(gsm_wait_for("OK", 20000))
    {
        CmdLCD(0x01);
        StrLCD("TEMP SMS SENT");
		delay_ms(2000);
    }
    else if(strstr(buff, "ERROR"))
    {
        CmdLCD(0x01);
        StrLCD("TEMP SMS ERROR");
    }
    else
    {
        CmdLCD(0x01);
        StrLCD("TEMP SMS FAIL");
    }
		i = 0;
memset(buff, '\0', sizeof(buff));
}
void send_sms_humi(char *num,char *msg, int h, float hf)
{
  float humi=h+hf;
	//float humi = h + (hf / 10.0);
	// Clear buffer before command
    i = 0;
    memset(buff, '\0', sizeof(buff));

  UART0_Str("AT+CMGS=\"");
  UART0_Str(num);
  UART0_Str("\"\r\n");
	// ? Wait for '>' prompt
    if(!gsm_wait_for(">", 7000))
    {
        CmdLCD(0x01);
        StrLCD("NO PROMPT");
        return;
    }
  //delay_ms(1000);
  UART0_Str(msg);


    UART0_Str(" TIME:");
    UART0_Tx((hour/10)+'0');
    UART0_Tx((hour%10)+'0');
    UART0_Tx(':');
    UART0_Tx((minute/10)+'0');
    UART0_Tx((minute%10)+'0');
    UART0_Tx(':');
    UART0_Tx((second/10)+'0');
    UART0_Tx((second%10)+'0');

    UART0_Str(" DATE:");
    UART0_Tx((date/10)+'0');
    UART0_Tx((date%10)+'0');
    UART0_Tx('/');
    UART0_Tx((month/10)+'0');
    UART0_Tx((month%10)+'0');
    UART0_Tx('/');
    //UART0_Tx((year/10)+'0');
    //UART0_Tx((year%10)+'0');
		UART0_Int(year);

    UART0_Str(" HUM:");
    UART0_Float(humi);
    UART0_Str("%");

  UART0_Tx(26);//UART0_Tx(0x1A);
  //delay_ms(3000);
	 // ? Wait for confirmation
    if(gsm_wait_for("OK", 20000))
    {
        CmdLCD(0x01);
        StrLCD("HUM SMS SENT");
		delay_ms(2000);
    }
    else if(strstr(buff, "ERROR"))
    {
        CmdLCD(0x01);
        StrLCD("HUM SMS ERROR");
    }
    else
    {
        CmdLCD(0x01);
        StrLCD("HUM SMS FAIL");
    }
		i = 0;
memset(buff, '\0', sizeof(buff));
}


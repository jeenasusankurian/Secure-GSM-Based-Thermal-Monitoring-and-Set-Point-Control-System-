//gsm_t.h

#ifndef _GSM_H
#define _GSM_H

#include "types.h"
void gsm_wait_startup(void);
void gsm_init(void);
void send_sms(/*signed*/ char *num, /*signed*/ char *msg);
void receive_sms(void);
void send_sms_temp(char *num,char*msg,int t,float f);
void send_sms_humi(char *num,char *msg, int t, float tf);
int gsm_wait_for(char *keyword, int timeout);

#endif

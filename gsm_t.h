...................//lcd.h
#ifndef _LCD_H_
#define _LCD_H_

#include "types.h"

// LCD Functions
void InitLCD(void);

void CmdLCD(u8 cmd);
void CharLCD(u8 data);

void DispLCD(u8 val);

void StrLCD(u8 *str);
void IntLCD(s32 num);
void FltLCD(f32 num);

void StoreCustCharFont(void);

#endif

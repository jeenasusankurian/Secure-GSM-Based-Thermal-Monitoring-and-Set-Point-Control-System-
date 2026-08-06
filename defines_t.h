//defines.h
#define SETBIT(WORD,BP) WORD|=1<<BP
#define CLRBIT(WORD,BP) WORD&=~(1<<BP)
#define CPLBIT(WORD,BP) WORD^=(1<<BP)
#define WRITEBIT(WORD,BP,BIT) WORD=((WORD&~(1<<BP))|(BIT<<BP))
#define WRITENIBBLE(WORD,SBP,NIBBLE) WORD=((WORD&~(0xf<<SBP))|(NIBBLE<<SBP))
#define WRITEBYTE(WORD,SBP,BYTE) WORD=((WORD&~(0xff<<SBP))|(BYTE<<SBP))
#define WRITEHWORD(WORD,SBP,HWORD) WORD=((WORD&~(0xffff<<SBP))|(HWORD<<SBP))
#define READBIT(WORD,BP) ((WORD>>BP)&1)


#ifndef _DEFINES_H
#define _DEFINES_H

// LED & BUZZER
#define BUZZER     18
#define GREEN_LED  19
#define RED_LED    20

// EEPROM ADDRESSES
#define ADDR_PASS  0x0400
#define ADDR_MOB   0x0500
#define ADDR_TEMP  0x0600
#define ADDR_HUM   0x0700

#endif


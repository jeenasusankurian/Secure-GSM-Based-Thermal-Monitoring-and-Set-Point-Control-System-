//PinConnect.c
#include<LPC21xx.h>
void CfgPinFunc(int PortNo,int PinNo,int Func)
{
        if(PortNo==0)
        {
                if(PinNo<16)
                        PINSEL0=((PINSEL0&~(3<<(PinNo*2)))|(Func<<(PinNo*2)));
                else
                        PINSEL1=((PINSEL1&~(3<<((PinNo-16)*2)))|(Func<<((PinNo-16)*2)));
        }
        else
        {//dummy
        }
}


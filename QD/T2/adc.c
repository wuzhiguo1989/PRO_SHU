#include "adc.h"
#include "shared.h"
#include "intrins.h"

#define wait_adc_finish() while (!(ADC_CONTR & ADC_FLAG))   //等待ADC转换完成
#define adc_channel 1
#define current_adc_value ((ADC_RES << 2) | ADC_RESL)

static u16 GetADCResult();
static u16 adc_period_max = 0;

/*----------------------------
Get max in this period 
----------------------------*/
u16 read_adc_period_max() {
	return adc_period_max;
}

// start new scan
void refresh_adc_period_max() {
    adc_period_max = 0;
}

void init_adc() {
    P1ASF |= 0x02;                      // P1(ch) -> P1 ^ 1
    low_at_bit(CLK_DIV, ADRJ);          // ADRJ = 0
    ADC_RES = 0;                        // high 8 bit       
    ADC_RESL = 0;                       // low 2 bit
    ADC_CONTR = ADC_POWER | ADC_SPEED | adc_channel;
}

void adc_sampling() {
    u16 adc_value = GetADCResult();
    if (adc_value > adc_period_max)
        adc_period_max = adc_value;
}

static u16 GetADCResult()
{
    ADC_CONTR |= ADC_START;
    
    _nop_();                        
    _nop_();
    _nop_();
    _nop_();
    wait_adc_finish();              
    ADC_CONTR &= ~ADC_FLAG;         
    return current_adc_value;                 
}



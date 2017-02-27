#include    "PWM.h"
#include    "utils.h"


static code u16 const PWMxT1_ADDR[6] = {0xff00, 0xff10, 0xff20, 0xff30, 0xff40, 0xff50};
static code u16 const PWMxT2_ADDR[6] = {0xff02, 0xff12, 0xff22, 0xff32, 0xff42, 0xff52};
static code u16 const PWMxCR_ADDR[6] = {0xff04, 0xff14, 0xff24, 0xff34, 0xff44, 0xff54};

// in-chip
#define PWMx_Init_Level(PWM_id, level)      PWMCFG &= ~(1 << PWM_id), PWMCFG |= (level) << PWM_id

static void PWMx_Set_Edge(const u8 PWM_id, const u16 FirstEdge, const u16 SecondEdge) {
    EAXSFR();       

    (*(unsigned int volatile xdata *)PWMxT1_ADDR[PWM_id]) = FirstEdge;
    (*(unsigned int volatile xdata *)PWMxT2_ADDR[PWM_id]) = SecondEdge;     

    EAXRAM();       
}

static void PWMx_GPIO_Set(const u8 PWM_id, const bool gpio_output) {
    if (PWM_id == PWM3_ID) {
        PWM3 = gpio_output;
    } else if (PWM_id == PWM5_ID) {
        PWM5 = gpio_output;
    } else if (PWM_id == PWM7_ID) {
        PWM7 = gpio_output;
    }
}

void PWM_SourceClk_Duty(const u8 PWM_SourceClk, const u16 PWM_Cycle) {
    EAXSFR();

    (*(unsigned char volatile xdata *)PWMCKS) = PWM_SourceClk;  
    (*(unsigned int volatile xdata *)PWMC) = PWM_Cycle;

    EAXRAM();                       
}

void PWMx_Configuration(const u8 PWM_id, const PWMx_InitDefine xdata *PWMx) {
    EAXSFR();       //访问XFR

    PWMx_Set_Edge(PWM_id, PWMx->FirstEdge, PWMx->SecondEdge);

    (*(unsigned char volatile xdata *)PWMxCR_ADDR[PWM_id])
     = (PWMx->PWMx_IO_Select & 0x08)                    // PWM输出IO选择
        | ((PWMx->PWMx_Interrupt << 2) & 0x04)          // 中断允许
        | ((PWMx->SecondEdge_Interrupt << 1) & 0x02)    // 第二个翻转中断允许
        | (PWMx->FirstEdge_Interrupt & 0x01);           // 第一个翻转中断允许

    EAXRAM();       //恢复访问XRAM

    // // // in-chip
    // PWMx_As_GPIO(PWM_id);
    // in-chip
    PWMx_Init_Level(PWM_id, (PWMx->Start_IO_Level & 0x01));
}


// set width of high / low in one cycle
void PWMx_Setup_Wide(const u8 PWM_id, const u16 width, const bool high) {

    if (!width) {
        PWMx_As_GPIO(PWM_id);
        PWMx_GPIO_Set(PWM_id, !high);
    } else if (width == PWM_FIXED_CYCLE) {
        PWMx_As_GPIO(PWM_id);
        PWMx_GPIO_Set(PWM_id, high);
    } else {
        PWMx_As_PWM(PWM_id);
        if (high)
            PWMx_Set_Edge(PWM_id, 0, width);
        else
            PWMx_Set_Edge(PWM_id, width, 0);     
    }

}

// duty_cycle (percentage)
void PWMx_Setup_DutyCycle(const u8 PWM_id, const u8 duty_cycle) {
    PWMx_Setup_DutyCycle_Based(PWM_id, duty_cycle, 100);
}

// factor * ( TOTAL_CYCLE / base )
void PWMx_Setup_DutyCycle_Based(const u8 PWM_id, const u16 factor, const u16 base) {
    u16 pwm_t2 = factor;

    if (!factor || factor == base) {
        PWMx_As_GPIO(PWM_id);
        PWMx_GPIO_Set(PWM_id, factor);

    } else {
        pwm_t2 *= (PWM_FIXED_CYCLE / base) ;
        PWMx_As_PWM(PWM_id);
        PWMx_Set_Edge(PWM_id, 0, pwm_t2);
    }
}


void PWM_Init()
{
    PWMx_InitDefine xdata PWMx_InitStructure;                             
    // 香薰 IN 
    PWMx_InitStructure.PWMx_IO_Select   = PWM7_P17;      
    PWMx_InitStructure.Start_IO_Level       =   LOW;            
    PWMx_InitStructure.PWMx_Interrupt       = DISABLE;      
    PWMx_InitStructure.FirstEdge            = 1;
    PWMx_InitStructure.SecondEdge           = 0;
    PWMx_InitStructure.FirstEdge_Interrupt  = DISABLE;      
    PWMx_InitStructure.SecondEdge_Interrupt = DISABLE;              
    PWMx_Configuration(PWM7_ID, &PWMx_InitStructure);
   
     // CLK source                                     
    PWM_SourceClk_Duty(PWMCLOCK, PWM_FIXED_CYCLE);  
    
    // IO config

    push_pull(PWM_LED_Mx, PWM_LED_BIT);
    push_pull(PWM_CTL_Mx, PWM_CTL_BIT);
    push_pull(PWM_FENG_Mx, PWM_FENG_BIT);


    PWMCR |= ENPWM; 

    // // DISABLE INT
    PWMCR &= ~ECBI;     
    // PWMCR |=  ECBI;     
    // PWMFDCR = ENFD | FLTFLIO | FDIO;    
}


void PWM_Pin_Init() {
    // PWMx_Setup_DutyCycle(PWM_LED, 80);
    PWMx_Setup_DutyCycle(PWM_CTL, 0);
	PWMx_Setup_DutyCycle(PWM_FENG, 0);
}
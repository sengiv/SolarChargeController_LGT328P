

#include "sysclock.h"
#include <lgt328p.h>

#include "config.h"


int minChargeTime = 300; //ms
const double adcToVolt = 0.005455537;
const int timeForAdc = 5; //ms
const double batteryFullVolt = 15.5; 
bool charging = false; //default not charging
int timer1_counter;
bool continueCharging = false;
bool cellReady = false; //start as cell not ready
bool batteryReady = false; //start as battery not ready
int cellCheckCounter = 0;
const int cellCheckInterval = 6; //6*2 = 12s

//gets the charge level of the battery based on its voltage
static double getBatVolt()
{

	//wait for signal to stabilize
	//delay(timeForAdc);

	double raw = analogRead(A5);

	// convert to VCC (unit: V);
	raw *= adcToVolt;

	return raw;
}

static double getCellVolt()
{

	//wait for signal to stabilize
	//delay(timeForAdc);

	double raw = analogRead(A4);

	// convert to VCC (unit: V);
	raw *= adcToVolt;

	return raw;
}

void stopChargingBattery()
{
	digitalWrite(11, LOW);
	charging = false;
}

void startChargingBattery()
{
	digitalWrite(11, HIGH);
	charging = true;
}


//check cell voltage, should be above bat
ISR(TIMER1_OVF_vect)
{	
	//trigger is called every 2s, count till interval to execute code below
	cellCheckCounter++;
	if (cellCheckCounter < cellCheckInterval) { return; }

	//cell check interval met
	//reset counter
	cellCheckCounter = 0;

	

	//code to execute every interval
	
	stopChargingBattery(); //to get correct volt, disconnect

	//little delay (normal delay not working in isr)
	nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
	
	//get battery volt
	double batteryVolt = getBatVolt();

	//get cell voltage
	double cellVoltage = getCellVolt();

	//if cell is below 6V then something wrong, battery not ready
	if (batteryVolt <= 6) { batteryReady = false; }
	//else battery voltage is ok
	else { batteryReady = true; }

	
	cellVoltage = cellVoltage + 0.5; //+0.5V to rule out noise values
	//if cell lower than battery, mark cell as not ready
	if (cellVoltage < batteryVolt) { cellReady = false; }
	//else cell is above battery, battery ready
	else { cellReady = true; }

	
	//little debug monitor isr firing
	digitalWrite(2, digitalRead(2) ^ 1);
	
}


void setup() {

	//system clock init
	DrvSYS_Init();
	
	//bat & cell volt input
	pinMode(A5, INPUT);
	pinMode(A4, INPUT);
	pinMode(11, OUTPUT);
	pinMode(2, OUTPUT);

	//startup interrupt service
	//to check if cell is ready
	DrvTC1_Init();

	// Global interrupt enable
	sei();
	
}

void loop() {

	double chargingVolt = 0;
	
	chargingVolt = getBatVolt();
	//enable charging only if bat below max volt
	if (chargingVolt < batteryFullVolt){ continueCharging = true; }
	

	//while charging constantly monitor voltage
	//to set below battery max
	while (continueCharging && cellReady && batteryReady)
	{
		//start charging
		startChargingBattery();

		//get current voltage
		chargingVolt = getBatVolt();

		//above max volt stop the charging
		if (chargingVolt > batteryFullVolt) { stopChargingBattery(); continueCharging = false; }
	}

	delayMicroseconds(500000); //0.5s
	
}




/**
 * @enum emCom1Nopwm
 *	Compare Output mode, non-PWM mode,
 *	Use these enumeration type when WGM1 = NORM or CTC
 */
enum emCom1Nopwm
{
	E_COM1_NDIS = 0x0,	/**< disable OC */
	E_COM1_CTOG,		/**< toggle on compare match */
	E_COM1_CCLR,		/**< clear on comapre match */
	E_COM1_CSET			/**< set on compare match */
};


/* Arguments for TC1 initialize */

#if (TC1_CSX == 0x0)
#define	TC1_CS 	0x0
#elif (TC1_CSX == 0x3)
#define	TC1_CS	0x6
#elif (TC1_CSX == 0x4)
#define	TC1_CS	0x7
#else
#define	TC1_CS	TC1_CS1
#endif


void DrvTC1_Init(void)
{
	
	// u8reg is used for storing value of SREG
	u8 u8reg;

	/** 1. Stop timer */
	TCCR1B = 0x0;

	/** 2. Force compare match: OC = 1*/
	// The setup of the OC1x should be performed before setting 
	// the Data Direction Register for the port pin to output
	// set OC1x before DDR_OC1x is set
	TCCR1A = (E_COM1_CSET << COM1A0) | (E_COM1_CSET << COM1B0);
	// force compare match
	TCCR1C = 0xc0;

#if (TC1_CSX == 0x2)
	TCKCSR |= (1 << F2XEN);
#endif

	u8reg = PMX0;
	/** 3. Set PIN OC1x's direction */
#if (TC1_OC1AEN == 1)
#if (TC1_C1AIO == 1)
	DDRF |= (1 << PF5);
	u8reg |= (1 << C1AF5);
#else	
	DDRB |= (1 << PB1);
#endif
#endif

#if (TC1_OC1BEN == 1)
#if (TC1_C1BIO == 1)
	DDRF |= (1 << PF4);
	u8reg |= (1 << C1BF4);
#else
	DDRB |= (1 << PB2);
#endif
#endif

#if (TC1_C1BIO == 1) || (TC1_C1AIO == 1)
	PMX0 = 0x80;
	PMX0 = u8reg;
#endif

	/** 4. Disalble Global Interrupt */
	u8reg = SREG;
	cli();

	/** 5. Initiate TCNT1 */
	TCNT1H = (TC1_TCNT1 >> 8) & 0xff;
	TCNT1L = TC1_TCNT1 & 0xff;

	/** 6. Initiate (OCR1A & OCR1B)output compare register */
	OCR1AH = (TC1_OCR1A >> 8) & 0xff;
	OCR1AL = TC1_OCR1A & 0xff;
	OCR1BH = (TC1_OCR1B >> 8) & 0xff;
	OCR1BL = TC1_OCR1B & 0xff;
	ICR1H = (TC1_ICR1 >> 8) & 0xff;
	ICR1L = TC1_ICR1 & 0xff;

	/** 7. Restore SREG  */
	SREG = u8reg;

	/** 8. Config Interrupt if OCF1A, OCR1B, TOV1 or ICP1 is enabled */
#if ((TC1_TOV1EN == TRUE) || (TC1_OCF1AEN == TRUE) || \
		(TC1_OCF1BEN == TRUE) || (TC1_ICP1EN == TRUE))
	// Clear interrupt flag
	TIFR1 = 0x27;
	TIMSK1 = (TC1_TOV1EN << TOV1) | (TC1_OCF1AEN << OCF1A) | \
		(TC1_OCF1BEN << OCF1B) | (TC1_ICP1EN << ICF1);
#endif

#if (TC1_CSX == 0x2)
	// enable fast clock for timer 1
	TCKCSR |= (1 << TC2XS1);
	//while((TCKCSR & 0x20) == 0x0);
#endif

	/** 9. Initiate TCCR1A and TCCR1B */
	TCCR1A = (TC1_COM1A << COM1A0) | (TC1_COM1B << COM1B0) | \
		((TC1_WGM1 & 0x3) << WGM10);
	TCCR1B = TC1_CS | (((TC1_WGM1 & 0xc) >> 2) << WGM12) | (TC1_ICNC1 << ICNC1) | (TC1_ICES1 << ICES1);
}

/**********************************************************************************
***       MACRO AND DEFINITIONS							***
**********************************************************************************/
/** Argument for Clock Set */
#define MCK_CLKENA ((MCK_OSCKEN << 3) | (MCK_OSCMEN << 2) | (MCK_RCKEN << 1) | MCK_RCMEN)


/**
 * @fn void DrvSYS_Init(void)
 */
void DrvSYS_Init(void)
{
	u8	btmp;

	// step 1. enable clock sources
	btmp = PMCR | MCK_CLKENA;
	PMCR = 0x80;
	PMCR = btmp;

	// wait for clock stable, eg. us
#if (MCK_OSCMEN == 1) && (MCK_MCLKSEL == 1)
	delayms(1);
#endif
#if (MCK_OSCKEN == 1) && (MCK_MCLKSEL == 3)
	delayms(20);
#endif

	// step 2. configure main clock
	btmp = (PMCR & 0x9f) | ((MCK_MCLKSEL & 0x3) << 5);
	PMCR = 0x80;
	PMCR = btmp;

	nop(); nop();

#if (MCK_CLKDIV != 3)
	DrvCLK_SetClockDivider(MCK_CLKDIV);
#endif	

#if (SYS_SWDD == 1)
	DrvMISC_DisableSWD();
#endif

#if (SYS_C6EN == 1) || (SYS_E6EN == 1)
	btmp = PMX2 | (SYS_E6EN << 1) | SYS_C6EN;
	PMX2 = 0x80;
	PMX2 = btmp;
#endif

#if (MCK_CKOSEL == 1)
	CLKPR |= 0x40;
#endif

#if (MCK_CKOSEL == 2)
	CLKPR |= 0x20;
#endif
}


/**
 * @fn void DrvCLK_SetDiv(u8 u8ClkDiv)
 */
void DrvCLK_SetClockDivider(u8 u8ClkDiv)
{
	u8 btmp = 0x80 | (u8ClkDiv & 0xf);

	CLKPR = 0x80;
	CLKPR = btmp;
}




///ARCHIVE CODE
/*


void initTimerInterrupt()
{
	// initialize timer1
	noInterrupts();           // disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;

	// Set timer1_counter to the correct value for our interrupt interval
	//timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
	//timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
	timer1_counter = 0xFFFF;   // preload timer 65536

	TCNT1 = timer1_counter;   // preload timer
	TCCR1B |= (1 << CS32);    // 1024 prescaler
	TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
	interrupts();             // enable all interrupts
}

//checks if cells is producing more voltage then battery
static bool isCellReadyToPower()
{
	//stop all charging before checks
	//stopChargingBattery();

	//get battery volt
	double batteryVolt = getBatVolt();

	//battery volt higher than 16V, end here
	if (batteryVolt >= batteryFullVolt) { return false; }

	//battery not full & charging atm, continue charging
	if (charging) { return true; }

	//get cell voltage
	double cellVoltage = getCellVolt();

	//cell lower than battery, end here
	if (cellVoltage < batteryVolt) { return false; }

	//if battery is lower than cell & not charging atm, return true
	if (batteryVolt <= cellVoltage)
	{
		//good cell voltage & battery not full
		return true;
	}
	else {
		//battery not full & cell under volt
		return false;
	}


}

 */

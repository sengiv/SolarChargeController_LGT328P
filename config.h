// config.h

#ifndef _CONFIG_h
#define _CONFIG_h


// Macro Definition for chip package
#define MMCU_PACKAGE MMCU_SSOP20L

// Macro definition for: System Settings
#define SYS_C6EN 	0
#define MCK_OSCMEN 	0
#define MCK_OSCKEN 	0
#define SYS_E6EN 	0
#define SYS_SWDD 	0
#define MCK_CKOSEL 	0x0  	// disable CLKO output
#define MCK_MCLKSEL 	0x0  	// Internal 32MHz RC/OSC
#define MCK_FOSC 	16000000
#define MCK_CLKDIV 	0x0  	// No scalar
#define MCK_RCMEN 	1
#define MCK_RCKEN 	1

// Macro definition for: Timer/Counter 1
#define TC1_OC1BEN 	0
#define TC1_C1BIO 	0  	// OC1B output to PB3
#define TC1_OC1AEN 	0
#define TC1_C1AIO 	0  	// OC1A output to PB1
#define TC1_CSX 	0x1  	// using T1CLK (= SYSCLK)
#define TC1_ICP1EN 	0
#define TC1_CS1 	0x5  	// T1CLK/1024
#define TC1_WGM1 	0x0  	// NORMAL mode
#define TC1_COM1A 	0x0  	// disable comparator output
#define TC1_COM1B 	0x0  	// disable comparator output
#define TC1_ICES1 	0x0  	// falling edge is used as trigger
#define TC1_ICNC1 	0
#define TC1_OCR1A 	0x0000
#define TC1_OCR1B 	0x0000
#define TC1_ICR1 	0x0000
#define TC1_TCNT1 	0x7A12
#define TC1_TOV1EN 	1
#define TC1_OCF1AEN 	0
#define TC1_OCF1BEN 	0

#endif

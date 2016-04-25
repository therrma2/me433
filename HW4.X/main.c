#include <xc.h>
#include <sys/attribs.h>


// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free    up secondary osc pins  NOT SURE ON NTHIS ONE
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz - 8MHz/2 = 4MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV - 4MHz * 24 = 96 MHz
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz = 96MHz / 2 = 48 MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 13 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define CS LATBbits.LATB7

void initSPI1(){
 //RPB7Rbits.RPB7R = 0b0011; //set RPB7 (pin 16) to SS1
 
    
 TRISBbits.TRISB7 = 0;
 CS = 1;
 
 RPB8Rbits.RPB8R = 0b0011; //set RPB8 (pin 17) to SDO1
 SPI1CON = 0;               //turn off spi module and reset it
 SPI1BUF;                   //clear buffer
 SPI1BRG = 11999;           //set baud
 SPI1STATbits.SPIROV = 0;   // clear overflow bit
 SPI1CONbits.CKE = 1;       //set falling edge
 SPI1CONbits.MSTEN = 1;     // set as master
 SPI1CONbits.ON = 1;        // turn on SPI1
 
 
}

int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0; // Set RA4 to output (green LED)
    TRISBbits.TRISB4 = 1; // Set RB4 to input (user button)
    LATAbits.LATA4 = 0; //Set green LED to 1 to start 


    
    __builtin_enable_interrupts();
    _CP0_SET_COUNT(0);

    while(1) {
        if(_CP0_GET_COUNT()>=24000){      
        LATAbits.LATA4 = !LATAbits.LATA4;
        _CP0_SET_COUNT(0);
        }
	    
        while(PORTBbits.RB4 == 0){
            LATAbits.LATA4 = 0;

        }
    }
    
    
}

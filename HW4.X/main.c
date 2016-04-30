#include <xc.h>
#include <sys/attribs.h>
#include <math.h>
#include "i2c_slave.h"
#include "idc_master.noint.h"


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
 SPI1BRG = 1;           //set baud
 SPI1STATbits.SPIROV = 0;   // clear overflow bit
 SPI1CONbits.CKE = 1;       //set falling edge
 SPI1CONbits.MSTEN = 1;     // set as master
 SPI1CONbits.ON = 1;        // turn on SPI1
 
 
}

unsigned char spi1_io(unsigned char write){
    SPI1BUF = write;
    while(!SPI1STATbits.SPIRBF){
        ;
    }
    return SPI1BUF;
}

void setVoltage(unsigned char channel, unsigned char voltage) {
    
    //volt = 0b0011111111110000;
    unsigned char part1;
    unsigned char part2;
    //channel = 0b0000;
    part1 = channel<<7;
    part1 = part1|0b01110000;
    part1 = part1|(voltage>>4);
    part2 = voltage<<4;
    //channel = channel<<7;
    //channel = channel|0b1010011;
    //unsigned char test;
    //test = 0b10101010;
    CS = 0;
    //spi1_io(test);
    spi1_io(part1);
    spi1_io(part2);
    //spi1_io(voltage);
    CS = 1;
        
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
    initSPI1();

    
    __builtin_enable_interrupts();
    
    unsigned char wave1[100];
    int i = 0;
    float center = 255/2, A = 255/2;
    for (i = 0; i < 100; i++){
        wave1[i] = (unsigned char)center + A*sin( 2 * 3.14 * i / (100));
    }
    
    unsigned char wave2[100];
    i = 0;
    for (i = 0; i < 100; i++){
        wave2[i] = i*255/99;
    }
    
    _CP0_SET_COUNT(0);

    
    
    i = 0;
    while(1) {
        if(_CP0_GET_COUNT()>=24000){      //
            LATAbits.LATA4 = !LATAbits.LATA4;
            setVoltage(0,wave1[i]);
            setVoltage(1,wave2[i]);
            i++;
            _CP0_SET_COUNT(0);
        }
        if(i > 99){
            i = 0;
        }
	    
        while(PORTBbits.RB4 == 0){
            LATAbits.LATA4 = 0;

        }
    }
//    while(1) {
//        if(_CP0_GET_COUNT()>=24000){
//            setVoltage(0,100);
//            _CP0_SET_COUNT(0);
//        }
//    }
    
    
}

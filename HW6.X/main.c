#include <xc.h>
#include <sys/attribs.h>
#include <math.h>
#include "i2c_master_noint.h"
#include "ILI9163C.h"
#include "imu.h"


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
    //initSPI1();
    SPI1_init();
    LCD_init();
    i2c_master_setup();
    imu_init();
    
    //Initialize Timer2 for OC1 and OC3
    T2CONbits.TCKPS=4;
    PR2 = 12000;
    T2CONbits.ON = 1;
    
    //Initialize OC1
    RPA0Rbits.RPA0R = 0b0101;   //Pin 2
    OC1CONbits.OC32=0;
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 0b110;
    OC1R=0;
    OC1RS=1000;
    OC1CONbits.ON = 1;
    
    //Initialize OC2
    RPB8Rbits.RPB8R = 0b0101; //Pin 17
    OC2CONbits.OC32=0;
    OC2CONbits.OCTSEL = 0;
    OC2CONbits.OCM = 0b110;
    OC2R=0;
    OC2RS=1000;
    OC2CONbits.ON =1;
            
   
            
            
    
    __builtin_enable_interrupts();
    
    float accX,accY,accZ=0;
    int temp,roll,pitch,yaw = 0;
    
    unsigned char imu_raw[14];
    short imu_parsed[7];

    
    LCD_clearScreen(WHITE);
    //LATAbits.LATA4 = 1;
    //LCD_Draw_Character(10,10,'d',BLUE);
    
    char c[100];
    //int vari = 1337;
    //sprintf(c,"Hello World %d!",vari);
    
    //LCD_Draw_String(28,32,&c,BLUE);
    //LCD_drawPixel(0,0,RED);

    //sprintf(c,"Hello, %x",data_read);
    //LCD_Draw_String(5,5,&c,RED);
    _CP0_SET_COUNT(0);
    while(1){
        if (_CP0_GET_COUNT()>480000){
            _CP0_SET_COUNT(0);
            i2c_read(IMU,0x20,imu_raw);

            parse_imu(imu_raw,imu_parsed);
        
            temp = imu_parsed[0];
            accX = (float)imu_parsed[4]*2.0/32768.0;
            accY = (float)imu_parsed[5]*2.0/32768.0;
            accZ = (float)imu_parsed[6]*2.0/32768.0;
            roll = imu_parsed[1];
            pitch = imu_parsed[2];
            yaw = imu_parsed[3];
        
//            sprintf(c,"TEMP: %.2i     ",temp);
//            LCD_Draw_String(5,5,&c,RED);
            if (accX > 1){
                OC1RS = 2999;
                }
            else if (accX < -1) {
                OC1RS = 0;
                }
            else {
                OC1RS = accX*PR2/2+PR2/2;
            }
            
            if (accY > 1){
                OC2RS = 2999;
                }
            else if (accY < -1) {
                OC2RS = 0;
                }
            else {
                OC2RS = accY*PR2/2+PR2/2;
            }
            
                    

            sprintf(c,"x %.2f ",accX);
            LCD_Draw_String(5,14,&c,BLUE);
        
            sprintf(c,"y %.2f ",accY);
            LCD_Draw_String(5,23,&c,BLUE);
//        
            sprintf(c,"accZ: %.4f     ",accZ);
            LCD_Draw_String(5,32,&c,BLUE);
        
            sprintf(c,"vROLL: %.2i     ",roll);
            LCD_Draw_String(5,41,&c,RED);
        
            sprintf(c,"vPITCH: %.2i     ",pitch);
            LCD_Draw_String(5,50,&c,RED);
        
            sprintf(c,"vYAW: %.2i     ",yaw);
            LCD_Draw_String(5,59,&c,RED);
        

            LATAbits.LATA4 = !LATAbits.LATA4;
        
        }
    }
}
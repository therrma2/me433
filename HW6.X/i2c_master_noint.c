// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C channel you are using
// I2C pins need pull-up resistors, 2k-10k
#include "i2c_master_noint.h"
#include <xc.h>
void i2c_master_setup(void) {
  ANSELBbits.ANSB2 = 0;
  ANSELBbits.ANSB3 = 0;
  //TRISBbits.TRISB2 = 0;
  //TRISBbits.TRISB3 = 0;
  I2C2BRG = 53; //some number for 100kHz;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2 
                                    // look up PGD for your PIC32 (104ns)  (1/(2*100,000)- .000000104)*48000000 -2
  I2C2CONbits.ON = 1;               // turn on the I2C1 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data

    while(!I2C2STATbits.RBF) {
        ;
//        Nop();
//        Nop();
//        Nop();
    }    // wait to receive the data
    
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}

//void init_exp(void){
//    write_exp(EXPADD,0xF0,0x00); 
//    write_exp(EXPADD,0x00,0x0A); 
//}
void i2c_write(unsigned char addr, unsigned char data, unsigned char regist){
   
    i2c_master_start();
    i2c_master_send(addr<<1|0);
    i2c_master_send(regist);
    i2c_master_send(data);
    i2c_master_stop();
}

void i2c_read(unsigned char addr,unsigned char regist, unsigned char *result){
    
    i2c_master_start();
    i2c_master_send(addr<<1|0);
    i2c_master_send(regist);
    i2c_master_restart();
    i2c_master_send(addr<<1|1);
    int i;
    for(i=0;i<14;i++){
        result[i] = i2c_master_recv();
        if (i==13){
            i2c_master_ack(1);
        }
        else {
            i2c_master_ack(0);
        }
    }
    //i2c_master_ack(1);
    i2c_master_stop();
    
}

void parse_imu(unsigned char *raw,short *parsed){
    //starts with temp low then temp high
    parsed[0] = (raw[1]<<8);//|raw[0]; //temp
    parsed[0] = parsed[0]|raw[0];
    parsed[1] = (raw[3]<<8);//|raw[2]; //gyro x
    parsed[1] = parsed[1]|raw[2];
    parsed[2] = (raw[5]<<8)|raw[4]; //gyro y
    parsed[3] = (raw[7]<<8)|raw[6]; //gyro z
    parsed[4] = (raw[9]<<8)|raw[8]; //acc x
    parsed[5] = (raw[11]<<8)|raw[10]; //acc y
    parsed[6] = (raw[13]<<8)|raw[12]; //acc z
    
    
}


//unsigned char read_exp(unsigned char addr,unsigned char regist){
//    unsigned char result;
//     
//    i2c_master_start();
//    i2c_master_send(addr<<1|0);
//    i2c_master_send(regist);
//    i2c_master_restart();
//    i2c_master_send(addr<<1|1);
//
//    result = i2c_master_recv();
//
//    i2c_master_ack(1);
//    i2c_master_stop();
//    return result;
//}
//
//void set_exp_pin(int pin, int val){
//    unsigned char out = 0x01;
//    LATAbits.LATA4 = 0; 
//    unsigned char current = read_exp(EXPADD,0x09);
//
//    //unsigned char test = read_exp(0x09);
//    out = out << pin;
//    //LATAbits.LATA4 = 1; 
//    if (val == 1){
//        write_exp(EXPADD,current|out,0x0A);
//        //LATAbits.LATA4 = 1;
//    }
//    else if (val == 0){
//        write_exp(EXPADD,current&(~out),0x0A);
//        LATAbits.LATA4 = 1;
//    }
//
//        
//}
//
//unsigned char read_exp_pin(int pin){
//    unsigned char out = 0x01;
//    unsigned char data;
//
//    out = out<<pin;
//    data = read_exp(EXPADD,0x09);
//    
//    data = (data&out)>>pin;
//    //if (data==1){
//    //    LATAbits.LATA4 = 1; 
//    //}
//    return data;
//}
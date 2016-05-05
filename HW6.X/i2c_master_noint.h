#ifndef I2C_MASTER_NOINT_H__
#define I2C_MASTER_NOINT_H__
// Header file for i2c_master_noint.c
// helps implement use I2C1 as a master without using interrupts
#define EXPADD 0b0100000
#define IMU 0b1101010
void i2c_master_setup(void);              // set up I2C 1 as a master, at 100 kHz

void i2c_master_start(void);              // send a START signal
void i2c_master_restart(void);            // send a RESTART signal
void i2c_master_send(unsigned char byte); // send a byte (either an address or data)
unsigned char i2c_master_recv(void);      // receive a byte of data
void i2c_master_ack(int val);             // send an ACK (0) or NACK (1)
void i2c_master_stop(void);               // send a stop
void init_exp(void);
unsigned char read_exp(unsigned char addr,unsigned char regist);
void i2c_write(unsigned char addr, unsigned char data, unsigned char regist);
unsigned char read_exp_pin(int pin);
void set_exp_pin(int pin, int val);
void i2c_read(unsigned char addr,unsigned char regist, char *result);
#endif


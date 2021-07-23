/**
 * @file i2c_handler.c
 *
 * @brief Init I2C0 and transmit-receive.
 
 * @note The accelerometer sensor is always turned on on the TTTW lab-kit. So
 * no power ON or enable has to be done.
 * 
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright ProLab, TTÜ. 2021
 */

#include "em_cmu.h"
#include "em_i2c.h"

#include "i2c_handler.h"

/**
 * @brief Init I2C interface. 
 *
 * Accelerometer sensor is connected to port A pin 2 (SCL) and pin 3 (SDA), I2C0
 * must be routed to those pins.
 */
void i2c_init (void)
{
    static I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    
	// Enable I2C clock.
	CMU_ClockEnable(cmuClock_I2C0, true);

    // Take control of SDC and SCL output pins (GPIO enabled at boot).
	GPIO_PinModeSet(gpioPortA, 2, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(gpioPortA, 3, gpioModeWiredAndPullUpFilter, 1);
	
	// Route I2C SDA and SCL to output pins (efr32mg12-datasheet page 188).
	I2C0->ROUTELOC0 = I2C_ROUTELOC0_SCLLOC_LOC1 | I2C_ROUTELOC0_SDALOC_LOC3;
    I2C0->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN;
    
    // Initialize I2C. 
    init.freq = I2C_FREQ_FAST_MAX;
    init.enable = false;
    I2C_Init(I2C0, &init);
}

void i2c_enable (void)
{
    I2C_Enable(I2C0, true);
}

void i2c_disable (void)
{
    I2C_Enable(I2C0, false);
}

void i2c_reset (void)
{
    I2C_Reset(I2C0);
}

I2C_TransferSeq_TypeDef * i2c_transaction (I2C_TransferSeq_TypeDef * seq)
{
    I2C_TransferReturn_TypeDef ret;
    
    // Do a polled transfer.
    ret = I2C_TransferInit(I2C0, seq);
    
    while (i2cTransferInProgress == ret)
    {
        ret = I2C_Transfer(I2C0);
    }
    
    return seq;
}


/**
 * @file i2c_demo_main.c
 * 
 * @brief Communicates with TTTW labkit accelerometer over I2C protocol.
 *
 * 
 *
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright Thinnect Inc. 2019
 * Copyright ProLab, TTÜ. 2021
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "retargetserial.h"

#include "cmsis_os2.h"

#include "platform.h"

#include "SignatureArea.h"
#include "DeviceSignature.h"

#include "loggers_ext.h"
#include "logger_fwrite.h"

#include "em_i2c.h"
#include "i2c_handler.h"

#include "loglevels.h"
#define __MODUUL__ "main"
#define __LOG_LEVEL__ (LOG_LEVEL_main & BASE_LOG_LEVEL)
#include "log.h"

// Include the information header binary
#include "incbin.h"
INCBIN(Header, "header.bin");

// App loop - periodically print heartbeat
void app_loop ()
{
    for (;;)
    {
        osDelay(10000);
        info1("Heartbeat");
    }
}

void i2c_comm_loop ()
{
    #define TX_BUF_LEN 1
    #define RX_BUF_LEN 1
    #define MMA865FC3_ADDRESS_READ      0x3B
    #define MMA865FC3_ADDRESS_WRITE     0x3A
    #define MMA865FC3_REGADDR_STATUS    0x00
    #define MMA865FC3_REGADDR_WHOAMI    0x0D
    #define MMA865FC3_REGADDR_SYSMOD    0x0B
    
    I2C_TransferSeq_TypeDef rWhoAmI, rStatus, rSysmod, wCtrlReg, *retSeq;
    uint8_t txBuf[TX_BUF_LEN], rxBuf[RX_BUF_LEN];
    
    i2c_init();

    for (;;)
    {
        i2c_enable();
        
        rWhoAmI.addr = MMA865FC3_ADDRESS_READ;
        
        txBuf[0] = MMA865FC3_REGADDR_WHOAMI;
        rWhoAmI.buf[0].data = txBuf;
        rWhoAmI.buf[0].len = TX_BUF_LEN;
        
        rxBuf[0] = 0;
        rWhoAmI.buf[1].data = rxBuf;
        rWhoAmI.buf[1].len = RX_BUF_LEN;

        rWhoAmI.flags = 0x0004; // I2C_FLAG_WRITE_READ
        
        retSeq = i2c_transaction(&rWhoAmI);
        info1("Reg 0x%02x, val 0x%02x", retSeq->buf[0].data[0], retSeq->buf[1].data[0]);
        
        rSysmod.addr = MMA865FC3_ADDRESS_READ;
        
        txBuf[0] = MMA865FC3_REGADDR_SYSMOD;
        rSysmod.buf[0].data = txBuf;
        rSysmod.buf[0].len = TX_BUF_LEN;
        
        rxBuf[0] = 0;
        rSysmod.buf[1].data = rxBuf;
        rSysmod.buf[1].len = RX_BUF_LEN;

        rSysmod.flags = 0x0004; // I2C_FLAG_WRITE_READ
        
        retSeq = i2c_transaction(&rSysmod);
        info1("Reg 0x%02x, val 0x%02x", retSeq->buf[0].data[0], retSeq->buf[1].data[0]);
        
        rStatus.addr = MMA865FC3_ADDRESS_READ;
        
        txBuf[0] = MMA865FC3_REGADDR_STATUS;
        rStatus.buf[0].data = txBuf;
        rStatus.buf[0].len = TX_BUF_LEN;
        
        rxBuf[0] = 0;
        rStatus.buf[1].data = rxBuf;
        rStatus.buf[1].len = RX_BUF_LEN;

        rStatus.flags = 0x0004; // I2C_FLAG_WRITE_READ
        
        retSeq = i2c_transaction(&rStatus);
        info1("Reg 0x%02x, val 0x%02x", retSeq->buf[0].data[0], retSeq->buf[1].data[0]);
        
        i2c_disable();
        
        osDelay(2000);
    }
}

int logger_fwrite_boot (const char *ptr, int len)
{
    fwrite(ptr, len, 1, stdout);
    fflush(stdout);
    return len;
}

int main ()
{
    PLATFORM_Init();

    // LEDs
    PLATFORM_LedsInit(); //this also enables GPIO peripheral
    PLATFORM_ButtonPinInit();

    // Configure debug output
    RETARGET_SerialInit();
    log_init(BASE_LOG_LEVEL, &logger_fwrite_boot, NULL);

    info1("I2C-demo "VERSION_STR" (%d.%d.%d)", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    // Initialize OS kernel
    osKernelInitialize();

    // Create a thread
    const osThreadAttr_t app_thread_attr = { .name = "app" };
    osThreadNew(app_loop, NULL, &app_thread_attr);

    // Create thread for I2C communication
    const osThreadAttr_t i2c_comm_thread_attr = { .name = "i2c_comm" };
    osThreadNew(i2c_comm_loop, NULL, &i2c_comm_thread_attr);

    if (osKernelReady == osKernelGetState())
    {
        // Switch to a thread-safe logger
        logger_fwrite_init();
        log_init(BASE_LOG_LEVEL, &logger_fwrite, NULL);

        // Start the kernel
        osKernelStart();
    }
    else
    {
        err1("!osKernelReady");
    }

    for(;;);
}

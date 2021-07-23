/**
 * @file i2c_handler.h
 *
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright ProLab, TTÜ. 2021
 */

#ifndef I2C_HANDLER_H_
#define I2C_HANDLER_H_

// Public functions
void i2c_init(void);
void i2c_enable(void);
void i2c_disable(void);
void i2c_reset(void);
I2C_TransferSeq_TypeDef * i2c_transaction(I2C_TransferSeq_TypeDef * seq);

#endif // I2C_HANDLER_H_ */

/**
 * @file timer_handler.h
 *
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright ProLab, TTÜ. 15 April 2020
 */

#ifndef TIMER_HANDLER_H_
#define TIMER_HANDLER_H_

// Route LED pins to TIMER0 CC for tsb0 board.
#define LED0_LOC TIMER_ROUTELOC0_CC2LOC_LOC5 // PB12 - red
#define LED1_LOC TIMER_ROUTELOC0_CC1LOC_LOC5 // PB11 - green
#define LED2_LOC TIMER_ROUTELOC0_CC0LOC_LOC5 // PA5  - blue

// Which channel controls which LED.
#define LED0_CC_CHANNEL 2
#define LED1_CC_CHANNEL 1
#define LED2_CC_CHANNEL 0

#define TIMER0_TOP_VAL 100UL

// These dim down LED0 and LED2, change value to 1 if not desired.
#define LED0_POWER_DIV 2UL
#define LED2_POWER_DIV 4UL

// Maximum PWM duty cycle for each LED.
#define LED0_MAX_DC TIMER0_TOP_VAL/LED0_POWER_DIV
#define LED1_MAX_DC TIMER0_TOP_VAL
#define LED2_MAX_DC TIMER0_TOP_VAL/LED2_POWER_DIV

#define TIMER1_TOP_VAL 78

#define BLINK_PWM_LED0_MASK 1U //00000001
#define BLINK_PWM_LED1_MASK 2U //00000010
#define BLINK_PWM_LED2_MASK 4U //00000100

// Public functions
void timer1_init();
void timer0_cc_init();
void start_fading_leds_loop();
uint8_t get_leds_pwm();
void set_leds_pwm(uint8_t val);

#endif // TIMER_HANDLER_H_ */

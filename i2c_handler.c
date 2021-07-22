/**
 * @file timer_handler.c
 *
 * @brief Init TIMER0 and TIMER1 to control LEDs with PWM.
 * 
 * TIMER0 PWM is used to control all three LEDs
 * TIMER1 is used to trigger fading effect of LEDs
 * 
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright ProLab, TTÜ. 15 April 2020
 */

#include "em_cmu.h"
#include "em_timer.h"

#include "timer_handler.h"

typedef enum 
{
	FIRE_UP = 0,
	COOL_DOWN
}led_state_transition_t;

static void change_pwm_dutycycle ();

volatile static uint8_t m_led_state = 0;
static uint32_t m_led0_cnt, m_led1_cnt, m_led2_cnt, m_led0_sd, m_led2_sd;
static led_state_transition_t m_led0_toggle, m_led1_toggle, m_led2_toggle;


/**
 * @brief Init TIMER1 to regulate PWM dutycycle. 
 */
void timer1_init(void)
{
	// Enable clocks.
    CMU_ClockEnable(cmuClock_TIMER1, true);

	// Set TIMER top value.
	TIMER_TopSet(TIMER1, TIMER1_TOP_VAL);

	// TIMER general init
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = timerPrescale1024;
	timerInit.enable = false; // Don't start timer after init.

	// LED0 and LED2 duty cycle count-down is slowed down, init counters.
	m_led0_sd = m_led2_sd = 1;

	TIMER_Init(TIMER1, &timerInit);
}

/**
 * @brief Init TIMER0 for PWM usage on three CC channels. Start TIMER0.
 */
void timer0_cc_init(void)
{
	// Enable clocks.
    CMU_ClockEnable(cmuClock_TIMER0, true);

	// Init CC for PWM on GPIO pins.
	TIMER_InitCC_TypeDef ccInit = TIMER_INITCC_DEFAULT;
	ccInit.mode = timerCCModePWM;
	ccInit.cmoa = timerOutputActionToggle;
	
	// Initilize a CC channels for each LED.
	TIMER_InitCC(TIMER0, LED0_CC_CHANNEL, &ccInit);
	TIMER_InitCC(TIMER0, LED1_CC_CHANNEL, &ccInit);
	TIMER_InitCC(TIMER0, LED2_CC_CHANNEL, &ccInit);

	// Enable GPIO toggling by TIMER and set location of pins to be toggled.
	TIMER0->ROUTEPEN = (TIMER_ROUTEPEN_CC0PEN | TIMER_ROUTEPEN_CC1PEN | TIMER_ROUTEPEN_CC2PEN);
	TIMER0->ROUTELOC0 = (LED0_LOC | LED1_LOC | LED2_LOC);

	// Set same TIMER0 top value for all CC channels.
	TIMER_TopSet(TIMER0, TIMER0_TOP_VAL);

	// Set the PWM duty cycle, init all LEDs to zero.
	TIMER_CompareBufSet(TIMER0, LED0_CC_CHANNEL, 0);
	TIMER_CompareBufSet(TIMER0, LED1_CC_CHANNEL, 0);
	TIMER_CompareBufSet(TIMER0, LED2_CC_CHANNEL, 0);
	m_led0_cnt = m_led1_cnt = m_led2_cnt = 0;
	m_led0_toggle = m_led1_toggle = m_led2_toggle = COOL_DOWN;	

	// TIMER general init
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = timerPrescale256;
	timerInit.enable = true; // Start timer after init.

	TIMER_Init(TIMER0, &timerInit);
}

/**
 * @brief Start TIMER1 and manipulate PWM duty cycle.
 * 
 * Creates LED fading/gradual brightening effect. Each time TIMER1 fires 
 * increase or decrease each LED duty cycle by one step.
 * 
 * @warning There is an infinte loop in this function.
 */
void start_fading_leds_loop()
{
	TIMER_IntClear(TIMER1, TIMER_IFC_OF);
	TIMER_IntEnable(TIMER1, TIMER_IntGet(TIMER1) | TIMER_IEN_OF);
	TIMER_Enable(TIMER1, true);

	//TODO: Perhaps replace with an interrupt handler?
	for(;;)
	{
		if(_TIMER_IF_OF_MASK & TIMER1->IF) // Overflow has occurred.
		{
			change_pwm_dutycycle();
			TIMER_IntClear(TIMER1, TIMER_IFC_OF);
		}
	}
}

/**
 * @brief Sets the state of LEDs. 
 * 
 * State change triggers gradual fading-brightening of LED.
 * 
 * @param val State of each LED as a bitmask. 
 */
void set_leds_pwm(uint8_t val)
{
	if(val & BLINK_PWM_LED0_MASK)m_led0_toggle = FIRE_UP;
	else m_led0_toggle = COOL_DOWN;

	if(val & BLINK_PWM_LED1_MASK)m_led1_toggle = FIRE_UP;
	else m_led1_toggle = COOL_DOWN;

	if(val & BLINK_PWM_LED2_MASK)m_led2_toggle = FIRE_UP;
	else m_led2_toggle = COOL_DOWN;

	m_led_state = val;
}

/**
 * @brief Get current LED state. 
 * 
 * LEDs in the process of change are reported as already changed state (ie if LED
 * is fading from ON to OFF, it is reported as OFF already).
 * 
 * @return Each bit represents LED state, e.g. 00000010 means LED 1 is ON, while
 *         other LEDs are OFF.
 */
uint8_t get_leds_pwm ()
{
	return m_led_state;
}

/**
 * @brief Increase or decrease each LED dutycycle by one step. 
 * 
 * Also keep all LEDs at similar brightness. This means dimming LED0 (red) and LED2
 * (blue) to LED1 (green) level.
 */
static void change_pwm_dutycycle ()
{
	if(LED0_POWER_DIV == m_led0_sd)
	{
		// For LED 0
		if(FIRE_UP == m_led0_toggle)
		{
			if(LED0_MAX_DC >= m_led0_cnt)
			{
				m_led0_cnt++;
				TIMER_CompareBufSet(TIMER0, LED0_CC_CHANNEL, m_led0_cnt);
			}
		}
		else if(COOL_DOWN == m_led0_toggle)
		{
			if(0 < m_led0_cnt)
			{
				m_led0_cnt--;
				TIMER_CompareBufSet(TIMER0, LED0_CC_CHANNEL, m_led0_cnt);
			}
		}
		else ;
	}

	// For LED 1
	if(FIRE_UP == m_led1_toggle)
	{
		if(LED1_MAX_DC >= m_led1_cnt)
		{
			m_led1_cnt++;
			TIMER_CompareBufSet(TIMER0, LED1_CC_CHANNEL, m_led1_cnt);
		}
	}
	else if(COOL_DOWN == m_led1_toggle)
	{
		if(0 < m_led1_cnt)
		{
			m_led1_cnt--;
			TIMER_CompareBufSet(TIMER0, LED1_CC_CHANNEL, m_led1_cnt);
		}
	}
	else ;

	if(LED2_POWER_DIV == m_led2_sd)
	{
		// For LED 2
		if(FIRE_UP == m_led2_toggle)
		{
			if(LED2_MAX_DC >= m_led2_cnt)
			{
				m_led2_cnt++;
				TIMER_CompareBufSet(TIMER0, LED2_CC_CHANNEL, m_led2_cnt);
			}
		}
		else if(COOL_DOWN == m_led2_toggle)
		{
			if(0 < m_led2_cnt)
			{
				m_led2_cnt--;
				TIMER_CompareBufSet(TIMER0, LED2_CC_CHANNEL, m_led2_cnt);
			}
		}
		else ;
	}

	// LED0 and LED2 slow down - dims LED0 and LED2 to LED1 level
	if(LED0_POWER_DIV <= m_led0_sd)m_led0_sd = 1;
	else m_led0_sd++;

	if(LED2_POWER_DIV <= m_led2_sd)m_led2_sd = 1;
	else m_led2_sd++;
}

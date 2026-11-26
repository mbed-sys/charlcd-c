/*
 * Copyright (c) 2026 P Hof
 * 
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for the full text.
 */

/**
 * @file main.c
 * @brief LCD HAL Implementation & Application Example
 * @details Defines the required and optional 'lcd.c' callback functions 
 * for an 8-bit PIC microcontroller, including a sample implementation.
 *
 */

#include "config.h"
#include <xc.h>
#include <stdbool.h>
#include "../../lcd.h"


/* ==========================================================================
 * SECTION: Timing and Delay Management
 * ========================================================================== */

/**
 * @brief Executes hardware-specific delays required by the LCD controller.
 * @param type The specific timing requirement (Power-on, Command, Pulse, etc.)
 * 
 * LCD controllers require specific hold times for instructions to process correctly.
 */
void lcd_delay_execute(lcd_delay_t type) {
    switch (type) {
        case LCD_DELAY_POWER_ON:
            __delay_ms(50);
            break;
        case LCD_DELAY_FUNCTION_SET:
            // The specific 100us delay required during mode switching
            __delay_us(100);
            break;
        case LCD_DELAY_ENABLE_PULSE:
            __delay_us(1);
            break;
        case LCD_DELAY_STANDARD:
            __delay_us(40);
            break;
        case LCD_DELAY_LONG_COMMAND:
            __delay_us(1520);
            break;
        default:
            __delay_us(100);
            break;
    }
}

/* ==========================================================================
 * SECTION: Low-Level Control Pin Drivers
 * ========================================================================== */

/**
 * @brief Sets the Register Select (RS) pin.
 * @param set_pin_state LCD_PIN_STATE_HIGH or LCD_PIN_STATE_LOW
 */
void lcd_rs_set(lcd_pin_state_t set_pin_state) {
    LATBbits.LATB4 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/**
 * @brief Sets the Enable (EN) pin.
 * @param set_pin_state LCD_PIN_STATE_HIGH or LCD_PIN_STATE_LOW
 */
void lcd_en_set(lcd_pin_state_t set_pin_state) {
    LATBbits.LATB6 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/* ==========================================================================
 * SECTION: Low-Level Data Pin Drivers
 * ========================================================================== */

/** @brief Sets Data Bit 4. */
void lcd_D4_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC4 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/** @brief Sets Data Bit 5. */
void lcd_D5_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC5 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/** @brief Sets Data Bit 6. */
void lcd_D6_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC6 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/** @brief Sets Data Bit 7. */
void lcd_D7_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC7 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/* ==========================================================================
 * SECTION: Hardware Configuration (TRIS)
 * ========================================================================== */

/** @brief Configures RS pin as output. */
static inline void rs_driver_output(void) {
    TRISBbits.TRISB4 = 0;
}

/** @brief Configures EN pin as output. */
static inline void en_driver_output(void) {
    TRISBbits.TRISB6 = 0;
}

/* ==========================================================================
 * SECTION: Busy Flag Config - only required when using the busy flag
 * ========================================================================== */

#ifdef LCD_USE_BUSY_FLAG

/** @brief Configures R/W pin as output. */
static inline void rw_driver_output(void) {
    TRISBbits.TRISB5 = 0;
}

/**
 * @brief Sets the Read/Write (RW) pin.
 * @param set_pin_state LCD_PIN_STATE_HIGH or LCD_PIN_STATE_LOW
 */
void lcd_rw_set(lcd_pin_state_t set_pin_state) {
    LATBbits.LATB5 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/**
 * @brief Reads the current state of the D7 pin to check the LCD Busy Flag.
 * @return true if D7 is High, false if Low.
 */
bool lcd_D7_read_busy_flag(void) {
    return (PORTCbits.RC7 != 0);
}

/**
 * @brief Configures the direction of the 4-bit data bus (D4-D7).
 * @param bus_dir LCD_BUS_OUTPUT (Pins as Output) or LCD_BUS_INPUT (Pins as Input)
 */
void lcd_set_data_bus_direction(lcd_bus_dir_t bus_dir) {
    if (bus_dir == LCD_BUS_OUTPUT) {
        TRISC &= 0x0F; // D4 to D7 == outputs
    } else {
        TRISC |= 0xF0; // D4 to D7 == inputs
    }
}

#endif

/* ==========================================================================
 * SECTION: Custom Board Setting
 * ========================================================================== */
/**
 * @brief Vo PWM for negative charge pump on custom board. 
 * @note Not required for the charlcd lib.
 */
static void pwm1_neg_charge_pump_LCD_Vo(void);


/* ==========================================================================
 * SECTION: Main Application
 * ========================================================================== */

/**
 * @brief Application entry point.
 * @details Initializes hardware, displays a multilingual message, and 
 *          performs a scrolling display animation in the main loop.
 */
int main(void) {
    pwm1_neg_charge_pump_LCD_Vo();

    // Pin Configuration: Analog/Digital Selection (ANSEL Registers)
    // Configure RC4-RC7 for digital input (Data Bus D4-D7)
    ANSELC = 0x0F; // Digital Pins: RC4, RC5, RC6, RC7

    /* 1. Peripheral Pin Direction Initialization */
    rs_driver_output();
    en_driver_output();
#ifdef LCD_USE_BUSY_FLAG
    rw_driver_output();
    lcd_set_data_bus_direction(LCD_BUS_OUTPUT);
#else
    TRISC &= 0x0F; // Configure D4-D7 as outputs; bidirectional mode is not required
    TRISBbits.TRISB5 = 0; // Configure RW pin; may be hardwired to GND on custom hardware
    LATBbits.LATB5 = 0; // Drive RW low to prevent a floating state
#endif

    /* 2. LCD Initialization and Content Display */
    lcd_init();
    lcd_cntrl(LCD_CURSOR_STEADY);

    lcd_goto_pos(LCD_LINE1);
    lcd_puts("White roses have thorns");

    lcd_goto_pos(LCD_LINE2);
    lcd_puts("Keine Sch\xEFnheit ist ohne Dornen."); // \xEF == ö

    lcd_home();

    /* 3. Main Application Loop (Animation) */
    while (1) {
        __delay_ms(300);

        // Perform display shifting animation
        lcd_cntrl(LCD_DISPLAY_SHIFT_LEFT);
        lcd_cntrl(LCD_CURSOR_SHIFT_RIGHT);
    }
}

/**
 * @brief Configures PWM1 in 16-bit mode for the Negative Charge Pump (LCD V_o).
 * 
 * Configuration Summary:
 * - Output: RA2 (PWM1_16BIT:PWM11)
 * - Clock: HFINTOSC with Prescaler 2
 * - Mode: Left-aligned, 16-bit Period
 * - Period: 0x013F (319 decimal)
 * - Duty Cycle 1: 0x000C (12 decimal)
 * - Duty Cycle 2: 0x00A0 (160 decimal)
 */
static void pwm1_neg_charge_pump_LCD_Vo(void) {
    // --- 1. Pin Configuration & PPS ---
    TRISAbits.TRISA2 = 0; // Set RA2 as Output
    RA2PPS = 0x0A; // Map RA2 to PWM1_16BIT:PWM11

    // --- 2. Clock & Prescaler Settings ---
    PWM1CLK = 0x03; // Clock Source: HFINTOSC
    PWM1CPRE = 0x01; // Prescaler: 2
    PWM1PIPOS = 0x00; // Postscale: No postscale

    // --- 3. Period & Loading Settings ---
    PWM1LDS = 0x00; // Autoload: Disabled
    PWM1PRH = 0x01; // Period High Byte
    PWM1PRL = 0x3F; // Period Low Byte (Total Period: 0x013F)
    PWM1ERS = 0x00; // External Reset: Disabled

    // --- 4. Duty Cycle (Slice 1) Configuration ---
    // PWM Mode: Left-aligned (PWM1S1CFG = 0)
    PWM1S1CFG = 0x00;

    // Duty Cycle 1 (Output 1)
    PWM1S1P1H = 0x00; // Duty Cycle High
    PWM1S1P1L = 0x0C; // Duty Cycle Low (Value: 12)

    // --- 5. Interrupt Management ---
    // Disable all PWM1 interrupts
    PIE4bits.PWM1IE = 0; // PWM1 Global Interrupt Enable
    PIE4bits.PWM1PIE = 0; // PWM1 Period Interrupt Enable
    PWM1GIE = 0; // PWM1 Slice Interrupt Enable

    // Clear all pending interrupt flags
    PIR4bits.PWM1PIF = 0; // Clear PWM1 Period Interrupt Flag
    PIR4bits.PWM1IF = 0; // Clear PWM1 Global Interrupt Flag
    PWM1GIRbits.S1P1IF = 0; // Clear Slice 1, Output 1 Flag
    PWM1GIRbits.S1P2IF = 0; // Clear Slice 1, Output 2 Flag

    // --- 6. Final Enable ---
    // PWM1CON: Enable PWM, Disable Load, PWMEN enabled
    PWM1CON = 0x80;
}

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


#include <xc.h>
#include "config.h"
#include <stdbool.h>
#include "../../lcd.h"

/* XC8 specific timing requirements */
#ifndef _XTAL_FREQ
#error _XTAL_FREQ not set, required for XC8 compiler delays
#endif

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
    LATAbits.LATA4 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/**
 * @brief Sets the Enable (EN) pin.
 * @param set_pin_state LCD_PIN_STATE_HIGH or LCD_PIN_STATE_LOW
 */
void lcd_en_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC4 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/* ==========================================================================
 * SECTION: Low-Level Data Pin Drivers
 * ========================================================================== */

/** @brief Sets Data Bit 4. */
void lcd_D4_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC3 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/** @brief Sets Data Bit 5. */
void lcd_D5_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC6 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/** @brief Sets Data Bit 6. */
void lcd_D6_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC7 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/** @brief Sets Data Bit 7. */
void lcd_D7_set(lcd_pin_state_t set_pin_state) {
    LATBbits.LATB7 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/* ==========================================================================
 * SECTION: Hardware Configuration (TRIS)
 * ========================================================================== */

/** @brief Configures RS pin as output. */
static inline void rs_driver_output(void) {
    TRISAbits.TRISA4 = 0;
}

/** @brief Configures EN pin as output. */
static inline void en_driver_output(void) {
    TRISCbits.TRISC4 = 0;
}

/* ==========================================================================
 * SECTION: Busy Flag Config - only required when using the busy flag
 * ========================================================================== */

#ifdef LCD_USE_BUSY_FLAG

/** @brief Configures R/W pin as output. */
static inline void rw_driver_output(void) {
    TRISCbits.TRISC5 = 0;
}

/**
 * @brief Sets the Read/Write (RW) pin.
 * @param set_pin_state LCD_PIN_STATE_HIGH or LCD_PIN_STATE_LOW
 */
void lcd_rw_set(lcd_pin_state_t set_pin_state) {
    LATCbits.LATC5 = (set_pin_state == LCD_PIN_STATE_HIGH) ? 1 : 0;
}

/**
 * @brief Reads the current state of the D7 pin to check the LCD Busy Flag.
 * @return true if D7 is High, false if Low.
 */
bool lcd_D7_read_busy_flag(void) {
    return (PORTBbits.RB7 != 0);
}

/**
 * @brief Configures the direction of the 4-bit data bus (D4-D7).
 * @param bus_dir LCD_BUS_OUTPUT (Pins as Output) or LCD_BUS_INPUT (Pins as Input)
 */
void lcd_set_data_bus_direction(lcd_bus_dir_t bus_dir) {
    if (bus_dir == LCD_BUS_OUTPUT) {
        TRISCbits.TRISC3 = 0;
        TRISCbits.TRISC6 = 0;
        TRISCbits.TRISC7 = 0;
        TRISBbits.TRISB7 = 0;
    } else { // input:
        TRISCbits.TRISC3 = 1;
        TRISCbits.TRISC6 = 1;
        TRISCbits.TRISC7 = 1;
        TRISBbits.TRISB7 = 1;
    }
}

#endif


/* ==========================================================================
 * SECTION: Main Application
 * ========================================================================== */

/**
 * @brief Application entry point.
 * @details Initializes hardware, displays a multilingual message, and 
 *          performs a scrolling display animation in the main loop.
 */
int main(void) {
    // Configure the clock
    OSC_INIT();

    // Pin Configuration: Analog/Digital Selection (ANSEL Registers)
    ANSELBbits.ANSB7 = 0; /* Disable analog input buffer */

    /* 1. Peripheral Pin Direction Initialization */
    rs_driver_output();
    en_driver_output();
#ifdef LCD_USE_BUSY_FLAG
    rw_driver_output();
    lcd_set_data_bus_direction(LCD_BUS_OUTPUT);
#else
    // Configure D4-D7 as outputs; bidirectional mode is not required
    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 0;
    TRISBbits.TRISB7 = 0;
    TRISCbits.TRISC5 = 0; // Configure RW pin; may be hardwired to GND on custom hardware
    LATCbits.LATC5 = 0; // Drive RW low to prevent a floating state
#endif

    /* 2. LCD Initialization and Content Display */
    lcd_init();
    lcd_cntrl(LCD_CURSOR_STEADY);

    lcd_goto_pos(LCD_LINE1);
    lcd_puts("White roses have ");

    lcd_goto_pos(LCD_LINE3);
    lcd_puts("Keine Sch\xEFnheit "); // \xEF == ö

    lcd_goto_pos(LCD_LINE2);
    lcd_puts("thorns.");

    lcd_goto_pos(LCD_LINE4);
    lcd_puts("ist ohne Dornen.");

    lcd_home();

    /* 3. Main Application Loop (Animation) */
    while (1) {
        __delay_ms(300);

        // Perform display shifting animation
        //lcd_cntrl(LCD_DISPLAY_SHIFT_LEFT);
        lcd_cntrl(LCD_CURSOR_SHIFT_RIGHT);
    }
}

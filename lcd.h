/*
 * Copyright (c) 2026 P Hof
 * 
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for the full text.
 */

/**
 * @file lcd.h
 * @brief Header file for the 4-bit HD44780 character LCD driver.
 * 
 * This header defines the data types and function prototypes used by the 
 * LCD driver. Hardware-specific pin mappings are abstracted via callbacks 
 * that must be implemented by the user (e.g., in main.c).
 * 
 * @note The driver will not compile unless all functions marked with 'extern' 
 * are provided by the user implementation.
 * 
 * @see https://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller
 * 
 * @author P Hof
 * @date 2026-11-24
 * 
 */

#include <stdbool.h>

#ifndef LCD_H
#define LCD_H


/** 
 * @brief Manual initialization by instructions.
 * 
 * Required if the power supply conditions for the internal reset circuit 
 * are not met. Uncomment to enable.
 */
#define LCD_ENABLE_INIT_BY_INSTRUCTIONS

/** 
 * @brief Busy Flag Configuration.
 * 
 * If defined, the R/W pin must be configured as an output and data pins 
 * must be switchable between output and input modes. Uncomment to enable.
 */
#define LCD_USE_BUSY_FLAG


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief LCD Delay types based on the required instruction timing.
     * Common delays for the HD44780 @ 270 kHz internal clock.
     */
    typedef enum {
        /** Min 15ms: Required after VCC reaches 4.5V to allow internal reset. Recommended: 50ms. */
        LCD_DELAY_POWER_ON,

        /** Min 100us: Required during the Function Set initialization sequence. */
        LCD_DELAY_FUNCTION_SET,

        /** Min 450ns: Required for the Enable (E) pulse width. */
        LCD_DELAY_ENABLE_PULSE,

        /** Min 37us - 40us: Standard command/data execution time. */
        LCD_DELAY_STANDARD,

        /** Min 1.52ms: Required for 'Clear Display' or 'Return Home' commands. */
        LCD_DELAY_LONG_COMMAND
    } lcd_delay_t;

    /** @brief Represents the logic state of a GPIO pin. */
    typedef enum {
        LCD_PIN_STATE_LOW,
        LCD_PIN_STATE_HIGH
    } lcd_pin_state_t;

    /** @brief Represents the direction of the data bus. */
    typedef enum {
        LCD_BUS_INPUT,
        LCD_BUS_OUTPUT
    } lcd_bus_dir_t;


    /** 
     * @name User Callbacks
     * @brief Hardware Abstraction Layer (HAL) callbacks.
     * 
     * @note These functions must be implemented by the user in a .c file 
     * (e.g., main.c) to bridge the driver to the specific microcontroller hardware.
     * @note All GPIOs (RS, EN, D4-D7) must be configured as outputs in the user code.
     * @{
     */

    /**
     * @brief Executes a delay based on the required LCD instruction type.
     * @param type The functional category of the delay required.
     */
    extern void lcd_delay_execute(lcd_delay_t type);

    /* --- Pin Output Controls --- */

    /** @brief Sets the Register Select (RS) pin state. */
    extern void lcd_rs_set(lcd_pin_state_t set_pin_state);

    /** @brief Sets the Enable (EN) pin state. */
    extern void lcd_en_set(lcd_pin_state_t set_pin_state);

    /** @brief Sets the state of the D4 data pin. */
    extern void lcd_D4_set(lcd_pin_state_t set_pin_state);
    /** @brief Sets the state of the D5 data pin. */
    extern void lcd_D5_set(lcd_pin_state_t set_pin_state);
    /** @brief Sets the state of the D6 data pin. */
    extern void lcd_D6_set(lcd_pin_state_t set_pin_state);
    /** @brief Sets the state of the D7 data pin. */
    extern void lcd_D7_set(lcd_pin_state_t set_pin_state);

#ifdef LCD_USE_BUSY_FLAG

    /** 
     * @name Busy Flag Callbacks
     * @note These callbacks are required only if LCD_USE_BUSY_FLAG is defined.
     * @note The R/W pin must be configured as an output in the user code.
     * @{ 
     */

    /** @brief Controls the R/W pin state. */
    extern void lcd_rw_set(lcd_pin_state_t set_pin_state);

    /** 
     * @brief Reads the Busy Flag from the D7 pin. 
     * @return true if the flag is HIGH (busy), false if LOW (ready). 
     */
    extern bool lcd_D7_read_busy_flag(void);

    /** @brief Switches the data bus (D4-D7) between input and output modes. */
    extern void lcd_set_data_bus_direction(lcd_bus_dir_t bus_dir);

    /** @} */
#endif /* LCD_USE_BUSY_FLAG */

    /** @} */ // End User Callbacks

    /**
     * @brief DDRAM addresses for cursor positioning. 
     * Adjust based on specific display properties if necessary.
     * First internal block: Line 1 [and Line 3 @ 0x14 or 0x10]  : 0x00 to 0x27
     * Second internal block: Line 2 [and line 4 @ 0x54 or 0x50] : 0x40 to 0x67
     */
    typedef enum {
        LCD_LINE1 = 0x00, /**< Start of Line 1 */
        LCD_LINE2 = 0x40, /**< Start of Line 2 */
        /* For displays with 4 lines */
        LCD_LINE3 = 0x14, /**< Start of Line 3, also common: 0x10 */
        LCD_LINE4 = 0x54 /**< Start of Line 4, also common: 0x50 */
    } lcd_pos_t;

    /**
     * @brief Entry Mode Set flags.
     */
    typedef enum {
        LCD_MSET_NONE = 0, /**< No shift or direction change */
        LCD_MSET_S = 0x1, /**< Display shift on */
        LCD_MSET_ID = 0x2 /**< Cursor move direction increment */
    } lcd_mset_t;

    /** @brief Default Entry Mode: Increment cursor direction. */
#define LCD_MSET_DEFAULT (LCD_MSET_ID)

    /**
     * @brief Display On/Off Control flags.
     */
    typedef enum {
        LCD_DSET_NONE = 0, /**< All display elements off */
        LCD_DSET_B = 0x1, /**< Blinking cursor on */
        LCD_DSET_C = 0x2, /**< Cursor on */
        LCD_DSET_D = 0x4 /**< Display on */
    } lcd_dset_t;

    /** @brief Default Display Mode: Display on with blinking cursor. */
#define LCD_DSET_DEFAULT (LCD_DSET_D | LCD_DSET_C | LCD_DSET_B)
    //#define LCD_DSET_DEFAULT (LCD_DSET_D) /* no cursor */

    /**
     * @brief Display Cursor or Display Shift flags.
     */
    typedef enum {
        LCD_CSET_NONE = 0, /**< No shift */
        LCD_CSET_SC = 0x8, /**< Display shift */
        LCD_CSET_RL = 0x4 /**< Shift direction: Right */
    } lcd_cset_t;

    /**
     * @brief Function Set flags.
     * @note This implementation does not support 8-bit mode.
     */
    typedef enum {
        LCD_FSET_NONE = 0, /**< No specific function set */
        LCD_FSET_N = 0x8, /**< 2 lines mode */
        LCD_FSET_F = 0x4 /**< 5x10 dot font (works on one line only if supported) */
    } lcd_fset_t;

    /** @brief Default Function Set: 2 lines. */
#define LCD_FSET_DEFAULT (LCD_FSET_N)

    /**
     * @brief High-level control operations for the LCD.
     */
    typedef enum {
        LCD_CURSOR_OFF, /**< Turn cursor off */
        LCD_CURSOR_STEADY, /**< Turn cursor on (no blink) */
        LCD_CURSOR_BLINK, /**< Turn cursor on (blinking) */
        LCD_CURSOR_SHIFT_LEFT, /**< Shift cursor left */
        LCD_CURSOR_SHIFT_RIGHT, /**< Shift cursor right */
        LCD_DISPLAY_SHIFT_LEFT, /**< Shift entire display left */
        LCD_DISPLAY_SHIFT_RIGHT /**< Shift entire display right */
    } lcd_cntrl_t;


    /* --- Main Functions --- */

    /** @brief Initializes the LCD. Must be called before any other function. */
    void lcd_init(void);
    /** @brief Writes a single character to the display. */
    void lcd_putch(char);
    /** @brief Writes a null-terminated string to the display. */
    void lcd_puts(const char *);

    /* --- Support Functions --- 
     * @note These functions accept single enum values. 
     * @warning Do NOT use bitwise 'OR' with these specific functions. 
     */

    /** @brief Clears the display content and returns cursor to home. */
    void lcd_clear(void);
    /** @brief Sets cursor to the home position (0,0). */
    void lcd_home(void);
    /** @brief Moves cursor to a specific DDRAM address. */
    void lcd_goto_pos(lcd_pos_t);
    /** @brief Performs basic cursor and display operations. */
    void lcd_cntrl(lcd_cntrl_t);

    /* --- Low-level Commands --- 
     * @note These functions support bitwise 'OR' operations of multiple values. 
     */

    /** @brief Entry Mode Set. */
    void lcd_mset(lcd_mset_t);
    /** @brief Display On/Off Control. */
    void lcd_dset(lcd_dset_t);
    /** @brief Display Cursor or Display Shift. */
    void lcd_cset(lcd_cset_t);
    /** @brief Function Set (4-bit mode only). */
    void lcd_fset(lcd_fset_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LCD_H */

/**
 * @file lcd.c
 * @brief Driver implementation for the 4-bit HD44780 character LCD.
 * 
 * This driver provides high-level functions to control an HD44780-compatible 
 * LCD in 4-bit mode. 
 * 
 * @author P Hof
 * @date 2026-11-24
 * 
 * @note Device-specific pin configurations are managed via callbacks and must 
 * be implemented in the user code.
 * @see https://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller
 */

#include <stdint.h>
#include "lcd.h"

/** @name HD44780 Instruction Codes
 *  @{
 */
#define LCD_CLEAR   (0b00000001) /**< Clear display command */
#define LCD_HOME    (0b00000010) /**< Return cursor to home position */
#define LCD_MSET    (0b00000100) /**< Entry Mode Set */
#define LCD_DSET    (0b00001000) /**< Display On/Off Control */
#define LCD_CSET    (0b00010000) /**< Display Cursor/Shift Control */
#define LCD_FSET    (0b00100000) /**< Function Set */
#define LCD_DDRAM   (0b10000000) /**< Set DDRAM Address */
/** @} */

static void lcd_write_nibble(char);
static void lcd_put_cmd(char);
static void lcd_wait_for_busy_flag(void);

/**
 * @brief Initializes the LCD controller.
 * 
 * Performs power-on delay, forces the controller into 4-bit mode via a 
 * synchronization sequence, and sets default display parameters.
 */
void lcd_init(void) {

    // Set control pins to low state
    lcd_rs_set(LCD_PIN_STATE_LOW);
    lcd_en_set(LCD_PIN_STATE_LOW);

#ifdef LCD_USE_BUSY_FLAG
    lcd_rw_set(LCD_PIN_STATE_LOW);
#endif /* LCD_USE_BUSY_FLAG */

    // Power-on delay: 50ms recommended for VCC > 4.5V, 40ms for VCC > 2.7V
    lcd_delay_execute(LCD_DELAY_POWER_ON);

#ifdef LCD_ENABLE_INIT_BY_INSTRUCTIONS
    /* 
     * Synchronization Sequence:
     * Forces the LCD into 4-bit mode regardless of its current state.
     * This is necessary if the LCD starts in 8-bit mode or is out of sync.
     */
    lcd_write_nibble(0b0011); // FSET 8-bits
    for (uint8_t dly_cntr = 0; dly_cntr < 42; dly_cntr++) {
        lcd_delay_execute(LCD_DELAY_FUNCTION_SET); // >4.1ms
    }
    lcd_write_nibble(0b0011); // FSET 8-bits
    lcd_delay_execute(LCD_DELAY_FUNCTION_SET); // >100us
    lcd_write_nibble(0b0011); // FSET 8-bits
    lcd_delay_execute(LCD_DELAY_STANDARD);
    lcd_write_nibble(0b0010); // FSET 4-bits
    lcd_delay_execute(LCD_DELAY_STANDARD);
#endif /* LCD_ENABLE_INIT_BY_INSTRUCTRUCTIONS */

    // Set 4-bit mode and default display lines
    lcd_fset(LCD_FSET_DEFAULT);
    // Turn display off initially
    lcd_dset(LCD_DSET_NONE);
    lcd_clear();
    lcd_mset(LCD_MSET_DEFAULT);
    lcd_dset(LCD_DSET_DEFAULT);
}

/**
 * @brief Writes a single character to the LCD.
 * @param c The character to be displayed.
 */
void lcd_putch(char c) {
    lcd_rs_set(LCD_PIN_STATE_HIGH);
    lcd_write_nibble(c >> 4); // Send high nibble
    lcd_write_nibble(c); // Send low nibble
#ifdef LCD_USE_BUSY_FLAG
    lcd_wait_for_busy_flag();
#else
    lcd_delay_execute(LCD_DELAY_STANDARD); // Fixed delay for non-busy flag mode
#endif /* LCD_USE_BUSY_FLAG */
}

/**
 * @brief Writes a null-terminated string to the LCD.
 * @param string Pointer to the character array to be displayed.
 */
void lcd_puts(const char *string) {
    lcd_rs_set(LCD_PIN_STATE_HIGH);
    while (*string != 0) {
        lcd_putch(*string);
        string++;
    }
}

/**
 * @brief Low-level function to send a command to the LCD.
 * @param cmd The command byte to be sent.
 */
static void lcd_put_cmd(char cmd) {
    lcd_rs_set(LCD_PIN_STATE_LOW);
    lcd_write_nibble(cmd >> 4);
    lcd_write_nibble(cmd);

#ifdef LCD_USE_BUSY_FLAG
    lcd_wait_for_busy_flag();
#else
    // If Busy Flag is not used, differentiate between standard commands 
    // and "slow" commands (like Clear or Home) based on the instruction bits.
    if (cmd >> 2) {
        lcd_delay_execute(LCD_DELAY_STANDARD);
    } else {
        // Longer delay required for Clear display (0b1) and Cursor Home (0b1x)
        lcd_delay_execute(LCD_DELAY_LONG_COMMAND);
    }
#endif /* LCD_USE_BUSY_FLAG */
}

/** @brief Clears the display and returns cursor to home position. */
void lcd_clear(void) {
    lcd_put_cmd(LCD_CLEAR);
}

/** @brief Returns the cursor to the home position. */
void lcd_home(void) {
    lcd_put_cmd(LCD_HOME);
}

/**
 * @brief Moves the cursor to a specific DDRAM address.
 * @param addr The target position (use LCD_LINEx constants).
 */
void lcd_goto_pos(lcd_pos_t addr) {
    lcd_put_cmd((char) (LCD_DDRAM | addr));
}

/**
 * @brief Sets the Entry Mode.
 * @param mode The mode to apply (use lcd_mset_t).
 */
void lcd_mset(lcd_mset_t mode) {
    lcd_put_cmd((char) (LCD_MSET | mode));
}

/**
 * @brief Sets the Display On/Off control.
 * @param mode The mode to apply (use lcd_dset_t).
 */
void lcd_dset(lcd_dset_t mode) {
    lcd_put_cmd((char) (LCD_DSET | mode));
}

/**
 * @brief Sets Display Cursor or Display Shift.
 * @param mode The mode to apply (use lcd_cset_t).
 */
void lcd_cset(lcd_cset_t mode) {
    lcd_put_cmd((char) (LCD_CSET | mode));
}

/**
 * @brief Sets the Function Set (e.g., line count, font size).
 * @param mode The mode to apply (use lcd_fset_t).
 */
void lcd_fset(lcd_fset_t mode) {
    lcd_put_cmd((char) (LCD_FSET | mode));
}

/**
 * @brief Provides high-level control for cursor and display operations.
 * @param op The control operation to perform (use lcd_cntrl_t).
 */
void lcd_cntrl(lcd_cntrl_t op) {
    switch (op) {
        case LCD_CURSOR_OFF: lcd_dset(LCD_DSET_D);
            break;
        case LCD_CURSOR_STEADY: lcd_dset(LCD_DSET_D | LCD_DSET_C);
            break;
        case LCD_CURSOR_BLINK: lcd_dset(LCD_DSET_D | LCD_DSET_C | LCD_DSET_B);
            break;
        case LCD_CURSOR_SHIFT_RIGHT: lcd_cset(LCD_CSET_RL);
            break;
        case LCD_CURSOR_SHIFT_LEFT: lcd_cset(LCD_CSET_NONE);
            break;
        case LCD_DISPLAY_SHIFT_RIGHT: lcd_cset(LCD_CSET_SC | LCD_CSET_RL);
            break;
        case LCD_DISPLAY_SHIFT_LEFT: lcd_cset(LCD_CSET_SC);
            break;
        default: lcd_dset(LCD_DSET_D); // default to cursor off
    }
}

/**
 * @brief Low-level function to write a 4-bit nibble to the data pins.
 * @param data The 4-bit value to send.
 */
static void lcd_write_nibble(char data) {
    if (data & 0x1) {
        lcd_D4_set(LCD_PIN_STATE_HIGH);
    } else {
        lcd_D4_set(LCD_PIN_STATE_LOW);
    }

    if (data & 0x2) {
        lcd_D5_set(LCD_PIN_STATE_HIGH);
    } else {
        lcd_D5_set(LCD_PIN_STATE_LOW);
    }

    if (data & 0x4) {
        lcd_D6_set(LCD_PIN_STATE_HIGH);
    } else {
        lcd_D6_set(LCD_PIN_STATE_LOW);
    }

    if (data & 0x8) {
        lcd_D7_set(LCD_PIN_STATE_HIGH);
    } else {
        lcd_D7_set(LCD_PIN_STATE_LOW);
    }

    // Strobe Enable pin to latch data
    lcd_delay_execute(LCD_DELAY_ENABLE_PULSE);
    lcd_en_set(LCD_PIN_STATE_HIGH);
    lcd_delay_execute(LCD_DELAY_ENABLE_PULSE);
    lcd_en_set(LCD_PIN_STATE_LOW);
}


#ifdef LCD_USE_BUSY_FLAG

/**
 * @brief Waits for the LCD busy flag to clear.
 * 
 * Uses the R/W pin and the data pins to poll the LCD status.
 * Because the controller is in 4-bit mode, the status information (Busy Flag 
 * and Address Counter) is split across two nibble reads.
 */
static void lcd_wait_for_busy_flag(void) {
    bool busy = true; // for busy flag monitoring

    lcd_rs_set(LCD_PIN_STATE_LOW);

    lcd_set_data_bus_direction(LCD_BUS_INPUT);

    lcd_rw_set(LCD_PIN_STATE_HIGH);

    lcd_delay_execute(LCD_DELAY_ENABLE_PULSE); // allow R/W to settle
    do {
        // Read first nibble: The Busy Flag (BF) is located in the D7 bit
        lcd_en_set(LCD_PIN_STATE_HIGH);
        lcd_delay_execute(LCD_DELAY_ENABLE_PULSE);
        busy = lcd_D7_read_busy_flag();
        lcd_en_set(LCD_PIN_STATE_LOW);
        lcd_delay_execute(LCD_DELAY_ENABLE_PULSE);

        // Read second nibble: Contains the Address Counter bits (AC3)
        lcd_en_set(LCD_PIN_STATE_HIGH);
        lcd_delay_execute(LCD_DELAY_ENABLE_PULSE);
        lcd_en_set(LCD_PIN_STATE_LOW);
        lcd_delay_execute(LCD_DELAY_ENABLE_PULSE);
    } while (busy);

    lcd_rw_set(LCD_PIN_STATE_LOW);
    lcd_delay_execute(LCD_DELAY_ENABLE_PULSE); // allow LCD to release data lines

    lcd_set_data_bus_direction(LCD_BUS_OUTPUT);
}
#endif /* LCD_USE_BUSY_FLAG */


/**
 * @note DDRAM Address mapping for 4-line displays:
 * 
 * Displays with 4 lines and up to 20 characters per line with one enable pin:
 * Line 1: 0x00, 0x01, 0x02 .. 0x13
 * Line 2: 0x40, 0x41, 0x42 .. 0x53
 * Line 3: 0x14, 0x15, 0x16 .. 0x27
 * Line 4: 0x54, 0x55, 0x56 .. 0x67
 * 
 * Displays with 4 lines and more than 20 characters per line with two enable pins:
 * Line 1: 0x00, 0x01, 0x02 .. 0x27
 * Line 2: 0x40, 0x41, 0x42 .. 0x67
 * Line 3: 0x00, 0x01, 0x02 .. 0x27
 * Line 4: 0x40, 0x41, 0x42 .. 0x67
 */

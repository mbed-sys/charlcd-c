/**
 * @brief LCD HAL Implementation & Application Example
 * @details Defines the required and optional 'lcd.c' callback functions 
 * for a microcontroller, including a sample implementation.
 *
 */

#include "lcd.h"

static enum {
  RS_LCD = 2,
  RW_LCD = 3,
  EN_LCD = 4,
  D4_LCD = 5,
  D5_LCD = 6,
  D6_LCD = 7,
  D7_LCD = 8,
  PIN_COUNT_LCD
} lcd_pins;

// put your setup code here, to run once:
// Hardware-specific delays
void lcd_delay_execute(lcd_delay_t type) {
  switch (type) {
    case LCD_DELAY_POWER_ON: delay(50); break;
    case LCD_DELAY_FUNCTION_SET: delayMicroseconds(100); break;
    case LCD_DELAY_ENABLE_PULSE: delayMicroseconds(1); break;
    case LCD_DELAY_STANDARD: delayMicroseconds(40); break;
    case LCD_DELAY_LONG_COMMAND: delayMicroseconds(1520); break;
    default: delayMicroseconds(100); break;
  }
}

// Control pin manipulation
void lcd_rs_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(RS_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}
void lcd_en_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(EN_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}

// Data bus manipulation (4-bit mode)
void lcd_D4_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(D4_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}
void lcd_D5_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(D5_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}
void lcd_D6_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(D6_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}
void lcd_D7_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(D7_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}

#ifdef LCD_USE_BUSY_FLAG
// with busy flag enabled
// Controls the R/W pin
extern void lcd_rw_set(lcd_pin_state_t set_pin_state) {
  digitalWrite(RW_LCD, (set_pin_state == LCD_PIN_STATE_HIGH) ? HIGH : LOW);
}

// Returns true if the Busy Flag is HIGH, false if LOW
bool lcd_D7_read_busy_flag(void) {
  return digitalRead(D7_LCD) == HIGH;
}

void lcd_set_data_bus_direction(lcd_bus_dir_t bus_dir) {
  if (bus_dir == LCD_BUS_OUTPUT) {
    pinMode(D4_LCD, OUTPUT);
    pinMode(D5_LCD, OUTPUT);
    pinMode(D6_LCD, OUTPUT);
    pinMode(D7_LCD, OUTPUT);
  } else {
    pinMode(D4_LCD, INPUT);
    pinMode(D5_LCD, INPUT);
    pinMode(D6_LCD, INPUT);
    pinMode(D7_LCD, INPUT);
  }
}
#endif

void setup() {
  //configure output
  for (uint8_t pin = 1; pin < PIN_COUNT_LCD; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);  //clear pins
  }

  lcd_init();
  lcd_cntrl(LCD_CURSOR_STEADY);

  lcd_goto_pos(LCD_LINE1);
  lcd_puts("White roses have thorns");

  lcd_goto_pos(LCD_LINE2);
  lcd_puts("Keine Sch\xEFnheit ist ohne Dornen.");  // \xEF == ö

  lcd_home();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(600);
  lcd_cntrl(LCD_DISPLAY_SHIFT_LEFT);
  lcd_cntrl(LCD_CURSOR_SHIFT_RIGHT);
}

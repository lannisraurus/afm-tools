#include <LCD_I2C.h>

const byte lcdAddress = 0x27;
LCD_I2C lcd(lcdAddress);

void setup() {
  lcd.begin();
  lcd.backlight();
  lcd.clear();
}
void loop() {}
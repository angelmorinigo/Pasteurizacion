#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Hola Angel");
  lcd.setCursor(0, 1);
  lcd.print("LCD funcionando");
}

void loop() {
}
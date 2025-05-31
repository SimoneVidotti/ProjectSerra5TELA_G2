#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void stampa_test_lcd(String s1, String s2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(s1);
  lcd.setCursor(0, 1);
  lcd.print(s2);
}

void setup() {
  lcd.init();           // Inizializza il display
  lcd.backlight();      // Accende la retroilluminazione
}

void loop() {
  stampa_test_lcd("HelloWorld!", "Hello");
  delay(1000);
  
  stampa_test_lcd("HelloWorld!", "My");
  delay(1000);
  
  stampa_test_lcd("HelloWorld!", "Friend");
  delay(1000);
}

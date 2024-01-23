// Demo at https://wokwi.com/projects/383989569588126721

#include "HX711.h"
//#include "Wire.h"
//#include "LCD_I2C.h"
#include "LiquidCrystal_I2C.h"
//LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules, change according
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DOUT_PIN_1 4
#define SCK_PIN_1 5
#define DOUT_PIN_2 6
#define SCK_PIN_2 7
#define DOUT_PIN_3 8
#define SCK_PIN_3 9
#define DOUT_PIN_4 11
#define SCK_PIN_4 12
#define BUTTON_PIN 13 // Pin for the push button

HX711 scale1, scale2, scale3, scale4;

float calibration_factor_1 = 420; // Adjust this value for calibration
float calibration_factor_2 = 420; // Adjust this value for calibration
float calibration_factor_3 = 420; // Adjust this value for calibration
float calibration_factor_4 = 420; // Adjust this value for calibration

const int SDA_PIN = A4;  // Replace with the actual SDA pin number
const int SCL_PIN = A5;  // Replace with the actual SCL pin number

void setup() {
  Serial.begin(9600);

  scale1.begin(DOUT_PIN_1, SCK_PIN_1);
  scale2.begin(DOUT_PIN_2, SCK_PIN_2);
  scale3.begin(DOUT_PIN_3, SCK_PIN_3);
  scale4.begin(DOUT_PIN_4, SCK_PIN_4);

  scale1.set_scale(calibration_factor_1);
  scale2.set_scale(calibration_factor_2);
  scale3.set_scale(calibration_factor_3);
  scale4.set_scale(calibration_factor_4);


  // Explicitly set the SDA and SCL pins
  //Wire.begin(SDA_PIN, SCL_PIN);
  lcd.begin(16,2);
  lcd.backlight();
  

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set up the button pin as input with pull-up resistor
}

void loop() {
  // Check if the tare button is pressed
  //if (digitalRead(BUTTON_PIN) == LOW) {
  //  tare();
  //  delay(500); // Optional debounce delay
  //}

  static unsigned long lastDebounceTime = 0;
  static unsigned long debounceDelay = 200;
  static unsigned long lastReadingTime = 0;
  static unsigned long readingInterval = 1000; // Interval between readings in milliseconds

  // Read the state of the button
  int buttonState = digitalRead(BUTTON_PIN);

  // Check for button press with debouncing
  // milis() gibt die Millisekunden seit rogrammstart an 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW) {
      Serial.println("Button Pressed");
      tare();
    }
    lastDebounceTime = millis();
  }

  if (scale1.is_ready() && scale2.is_ready() && scale3.is_ready() && scale4.is_ready()) {
    // Check if it's time for a new reading
    if ((millis() - lastReadingTime) >= readingInterval) {
      // Read weights from each load cell
      float weight1 = scale1.get_units();
      float weight2 = scale2.get_units();
      float weight3 = scale3.get_units();
      float weight4 = scale4.get_units();

      // Calculate the total weight
      float totalWeight = weight1 + weight2 + weight3 + weight4;

      Serial.print("Weight 1: ");
      Serial.print(weight1, 2);
      Serial.println(" kg");

      Serial.print("Weight 2: ");
      Serial.print(weight2, 2);
      Serial.println(" kg");

      Serial.print("Weight 3: ");
      Serial.print(weight3, 2);
      Serial.println(" kg");

      Serial.print("Weight 4: ");
      Serial.print(weight4, 2);
      Serial.println(" kg");

      Serial.print("Total Weight: ");
      Serial.print(totalWeight, 2);
      Serial.println(" kg");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Totales Gewicht: ");
      lcd.setCursor(0, 1);
      lcd.print(totalWeight, 2);
      lcd.println(" kg");

    lastReadingTime = millis(); // Update the last reading time
    //delay(1000); // Optional delay to control the rate of readings
    }
  }    
  //} else {
    //Serial.println("Error: Unable to detect one or more HX711 modules.");
    //lcd.print("Error: Unable to detect one or more HX711 modules.");
  

  
}

void tare() {
  Serial.println("updating.");
  scale1.tare();
  scale2.tare();
  scale3.tare();
  scale4.tare();

  delay(100); // Add a small delay before re-taring
  Serial.println("updating..");
  scale1.tare();
  scale2.tare();
  scale3.tare();
  scale4.tare();
  Serial.println("updating...");
}


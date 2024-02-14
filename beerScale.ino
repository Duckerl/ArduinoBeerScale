// Erstellt für das Erstsemesterseminar "Einführung in die Verfahrenstechnik" der HSD, WiSe 2023
// Demo unter https://wokwi.com/projects/383989569588126721
// Exzellente Anleitung über die Funktionsweise von Buttons https://starthardware.org/arduino-und-taster/
// Kallibrierungscode der offiziellen HX711 Bibliothek https://github.com/sparkfun/HX711-Load-Cell-Amplifier/tree/master/firmware/SparkFun_HX711_Calibration 

#include "HX711.h"                    // Bibliothek der Wägezellen
#include "LiquidCrystal_I2C.h"        // Bibliothek des LCDs
#include "SmoothProgress.h"           // Bibliothek der Progressbar https://github.com/Gjorgjevikj/SmoothProgress?tab=readme-ov-file
LiquidCrystal_I2C lcd(0x27, 20, 4);   // Standard Addresse der meisten PCF8574 Module, hier mit 4x20 Zeichen
#include <BarStyle4.h>                // Progressbar Style 4

// !häufige Fehlerquelle: Pins aus dem Programm stimmen nicht mit den aus den Kommentaren überein
#define DOUT_PIN_1 4  //Zelle 1 auf 4 & 5;
#define SCK_PIN_1 5

#define DOUT_PIN_2 6  //Zelle 2 auf 6 & 7;
#define SCK_PIN_2 7

#define DOUT_PIN_3 8  //Zelle 3 auf 8 & 9;
#define SCK_PIN_3 9

#define DOUT_PIN_4 10 //Zelle 4 auf 10 & 11;
#define SCK_PIN_4 11

#define BUTTON_PIN 13  // Pin für den Button;

HX711 scale1, scale2, scale3, scale4;
LCD dispA(lcd, barStyle4);

// Erstelle das Object zur Darstellung der Progressbar
SmoothProgressBar spb(dispA, 16, 2, 3); // Progressbar 16 Zeichen lang, Anfang 3-te Spalte (zählt von 0 an),  4-te Zeile (zählt von 0 an)

float calibration_factor_1 = -104500;  // erfolgreich -105000;
float calibration_factor_2 = -105000;  // erfolgreich -105000;
float calibration_factor_3 = 105000;   // erfolgreich 105000;
float calibration_factor_4 = 105000;   // erfolgreich 105000;

const int SDA_PIN = A4;  // am Arduino Mega gibts speziell zugewiesene PINs für SDA, daher eigentlich redundant
const int SCL_PIN = A5;  // am Arduino Mega gibts speziell zugewiesene PINs für SCL, daher eigentlich redundant

bool buttonPressedDown = true;  // speichert den letzten Zustand des Buttons; theoretisch weiß der Button bei der Initailisierung nicht ob der Zustand gedrückt (=true) ist oder nicht,
                                //ist praktisch aber egal und soagar hilfreich, da beim Programmstart automatisch ein Nullsetzen erfolgt
int progressPercentage = 0;

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

  lcd.init();
  lcd.backlight();

  dispA.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Initialisiert den Button, INPUT_PULLUP initiallisiert einen digitalen Resistor zum Entstören des Signals
}

void loop() {

  static unsigned long lastDebounceTime = 0;    // speichert den Zeitpunkt des letzten Buttonpress
  static unsigned long debounceDelay = 1000;    // Warteintervall für den Button in Milisec; 1000 milisec = 1 sec
  static unsigned long lastReadingTime = 0;     // speichert den Zeitpunkt der letzten Waagenlesung
  static unsigned long readingInterval = 1000;  // Warteintervall zur nächsten Waagenlesung in Millisec

  // Variable zum Auslesen des Button-Status (HIGH oder LOW)
  int buttonState = digitalRead(BUTTON_PIN);

  // Prüft Button-Press mit Entprellung --> Button kann eine Warteperiode lang nicht gedrückt werden
  // milis() gibt die Millisekunden seit Programmstart an
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Unser Button verbleibt in seiner Position, daher:
    // Zustandsabfrage: ist der Button LOW und war vorher HIGH?
    if (buttonState == LOW && buttonPressedDown == false) {
      Serial.println("Button Pressed");
      tare();
      buttonPressedDown = true;
    }
    // ist der Button HIGH und war vorher LOW?
    if (buttonState == HIGH && buttonPressedDown == true) {
      Serial.println("Button Pressed");
      tare();
      buttonPressedDown = false;
    }
    lastDebounceTime = millis();  // Updatet zur aktuelle Buttonauslesezeit
  }

  if (scale1.is_ready() && scale2.is_ready() && scale3.is_ready() && scale4.is_ready()) {
    // Ist das letzte Warteintervall überschritten?
    if ((millis() - lastReadingTime) >= readingInterval) {
      // Liest die Gewichte aus
      float weight1 = scale1.get_units(5); // Durchschnitt aus 5 Messungen
      float weight2 = scale2.get_units(5);
      float weight3 = scale3.get_units(5);
      float weight4 = scale4.get_units(5);

      // Kalkuliert das totale Gewicht
      float totalWeight = weight1 + weight2 + weight3 + weight4;

      // Setze den Wert auf 0, wenn er sehr nah an 0 liegt
      if (abs(totalWeight) < 0.001) {
        totalWeight = 0.0; 
      }
      // 80kg auf 100% umrechnen
      progressPercentage = (totalWeight / 80)*100;

      // Ausgabe der Werte im Seriellen Monitor (am PC)
      Serial.print("Weight 1: ");
      Serial.print(weight1, 3);     // 3 Dezimalstellen anzeigen
      Serial.println(" kg");

      Serial.print("Weight 2: ");
      Serial.print(weight2, 3);
      Serial.println(" kg");

      Serial.print("Weight 3: ");
      Serial.print(weight3, 3);
      Serial.println(" kg");

      Serial.print("Weight 4: ");
      Serial.print(weight4, 3);
      Serial.println(" kg");

      Serial.print("Total Weight: ");
      Serial.print(totalWeight, 3);
      Serial.println(" kg");

      // Ausgabe am LCD der Waage
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Totales Gewicht: ");
      lcd.setCursor(0, 1);
      lcd.print(totalWeight, 2);     // 2 Dezimalstellen anzeigen
      lcd.print(" kg");

      spb.showProgressPct(progressPercentage);

      lastReadingTime = millis();  // Updatet zur aktuellen Waageauslesungszeit
    }
  }
}

// Funktion fürs  Nullsetzen, tare wird 2x ausgeführt für ein genaueres Ergebnis
void tare() {
  Serial.println("updating.");
  lcd.clear();
  lcd.print("===tare===");
  scale1.tare();
  scale2.tare();
  scale3.tare();
  scale4.tare();
  lcd.setCursor(0, 1);
  lcd.print("updating.");

  delay(100);  // kleiner Delay
  Serial.println("updating..");
  lcd.setCursor(0, 2);
  lcd.print("updating..");
  scale1.tare();
  scale2.tare();
  scale3.tare();
  scale4.tare();
  Serial.println("updating...");
  lcd.setCursor(0, 3);
  lcd.print("updating...");
}

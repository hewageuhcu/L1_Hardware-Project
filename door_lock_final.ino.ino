#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Setups for LCD


LiquidCrystal_I2C lcd(0x27, 16, 4);
char password[10];
char initial_password[10], new_password[10];
int arraySize = sizeof(initial_password);
int vcc = 12;
int i = 0;
int relay_pin = 12;
char key_pressed = 0;
const byte rows = 4;
const byte columns = 4;
char hexaKeys[rows][columns] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte row_pins[rows] = { 31, 33, 35, 37 };
byte column_pins[columns] = { 39, 41, 43, 45 };

Keypad keypad_key = Keypad(makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);

boolean isPasswordAccepting = false;  // This turns to true when key 'D'is pressed

// Setups for humidity
const int buzzerPin = 16;  // Digital pin connected to the buzzer
const int switchPin = 5;
const int buttonPin = 51;   // Digital pin connected to the switch

#define DHTPIN 2           // what pin we're connected to
#define DHTTYPE DHT22      // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);  // set the LCD address to 0x27 for a 16 chars and 2 line display

float hum;
float tem;

void setup() {
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  pinMode(relay_pin, OUTPUT);
  pinMode(vcc, OUTPUT);

  // Initialize Humidity
  dht.begin();

  // Initialize buzzer and switch
  pinMode(buzzerPin, OUTPUT);        // Set the buzzer pin as an output
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);  // Set the switch pin as an input
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(relay_pin,LOW);


  printLCD(0, 0, "Track Master", true);
  lcd.setCursor(0, 1);
  lcd.print("Electronic Lock ");
  printLCD(0, 1, "Electronic Lock :", false);
  delay(500);
  initialpassword();
  printLCD(0, 0, "Enter D to Unlock", true);
  delay(500);
  lcd.clear();
}

void printLCD(int cursorX, int cursorY, String text, boolean isClear) {
  if (isClear == true) {
    lcd.clear();
  }
  lcd.setCursor(cursorX, cursorY);
  lcd.print(text);
}

void loop() {
  
  if(isPasswordAccepting == false){
    if(digitalRead(buttonPin) == LOW){
      isPasswordAccepting = true;
      printLCD(0, 0, "Enter Password", true);
      lcd.setCursor(0, 1);
      Serial.print("Enter Password");
    }
  }
  /*if (digitalRead(buttonPin) == LOW) {
    if (isPasswordAccepting == false) {
      isPasswordAccepting = true;
      printLCD(0, 0, "Enter Password", true);
      lcd.setCursor(0, 1);
      Serial.print("Enter Password");
    }
    key_pressed = keypad_key.getKey();
  } else {
    isPasswordAccepting = false;
  }*/

  key_pressed = keypad_key.getKey();
  if (key_pressed) {
    if (isPasswordAccepting) {
      if (key_pressed == 'D') {
        // Check password
        Serial.println("Password");
        Serial.println(password);
        Serial.println(initial_password);

        if (!(strcmp(password, initial_password))) {
          if (arraySize==10){
          printLCD(0, 0, "Pass Accepted", true);
          digitalWrite(relay_pin, HIGH);
          i = 0;
          };
          isPasswordAccepting = false;
          delay(5000);
          digitalWrite(relay_pin, LOW);
          lcd.clear();
        } else {
          digitalWrite(relay_pin, LOW);
          printLCD(0, 0, "Wrong Password", true);
          delay(2000);
          printLCD(0, 0, "Press # to", true);
          delay(1000);
          printLCD(0, 1, "Change the pass", false);

          delay(2000);
          i = 0;
          lcd.clear();
          isPasswordAccepting = false;
        }
      } else {
        // Get next char of the password
        Serial.println("Press key");
        password[i++] = key_pressed;
        lcd.print(key_pressed);
        Serial.print(key_pressed);
      }
    } else if (key_pressed == '#') {
      // Change password
      change();
    } else if (key_pressed == 'D') {
      isPasswordAccepting = true;
      printLCD(0, 0, "Enter Password", true);
      lcd.setCursor(0, 1);
      Serial.print("Enter Password");
    }
    //key_pressed = NULL;
  }
  if (!isPasswordAccepting) {
    hum = dht.readHumidity();
    tem = dht.readTemperature();

    if (hum >= 70.00 && hum <= 80.00) {
      printLCD(1, 2, "Charge the unit!", true);
      Serial.println("Charge the unit!");
      if (digitalRead(switchPin) == LOW) {
        digitalWrite(buzzerPin, LOW);
      } else {
        digitalWrite(buzzerPin, HIGH);
      }
    } else {
      printLCD(0, 0, "HUMIDITY:", true);
      printLCD(0, 1, String(hum), false);
      printLCD(0, 2, "Temperature:", false);
      printLCD(0, 3, String(tem), false);

      digitalWrite(buzzerPin, HIGH);
    }
    delay(1000);
  }
}

void change() {
  int j = 0;
  lcd.clear();
  lcd.print("Current Password");
  lcd.setCursor(0, 1);
  while (j < 9) {
    char key = keypad_key.getKey();
    if (key) {
      new_password[j++] = key;
      lcd.print(key);
    }
    key = 0;
  }
  delay(500);

  if ((strncmp(new_password, initial_password, 9))) {
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(1000);
  }

  else {
    j = 0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(0, 1);

    while (j < 9) {
      char key = keypad_key.getKey();
      if (key) {
        initial_password[j] = key;
        lcd.print(key);
        EEPROM.write(j, key);
        j++;
      }
    }
    lcd.print("Pass Changed");
    delay(1000);
  }

  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0, 1);
  key_pressed = 0;
}

void initialpassword() {
  int j;
  for (j = 0; j < 9; j++) {
    EEPROM.write(j, j + 49);
  }

  for (j = 0; j < 9; j++) {
    initial_password[j] = EEPROM.read(j);
  }
  

  //lcd.print(initial_password);
}

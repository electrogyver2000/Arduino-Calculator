/* 
  Arduino Calculator

  This sketch uses an Arduino to calculate an expression from a console application on a computer.
  The Arduino only communicates with the computer via a FT232 serial module. 
  The Arduino receives and parses the values, calculates the two arguments, and sends the
  answer back to the LCD display and the console.

  Wrong formats and divide-by-zero errors are shown.
  LEDs show green for successful communication and red for errors.

  Schematic:
  - I2C LCD display connected via SDA (A4) and SCL (A5)
  - Green LED connected to pin 7 via a 1k resistor to ground
  - Red LED connected to pin 8 via a 1k resistor to ground
  - Common ground for all components
  - Serial communication via USB-to-Serial FT232 module on RX/TX
*/

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// LCD Screen
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED lights
const int grnLED = 7;                             // green LED
const int redLED   = 8;                           // red LED

// Make sure we return to home screen after displaying answer of calculation.
unsigned long prevhometime = 0;
const unsigned long displayDuration = 5000;       //5 seconds
bool Home = true;

// set led's as output pins
void setup() {
  pinMode(grnLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  lcd.init();
  lcd.backlight();

// set baudrate here, currently set to 4800 to show communication at a different baudrate
  Serial.begin(4800);
  HomeScr();
}

void loop() {
  //check if we have a message in the serial buffer, to process.
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    //when we start a new calculation, reset the led status
    digitalWrite(grnLED, HIGH);
    digitalWrite(redLED, LOW);

    Serial.print("RAW: [");
    Serial.print(input);
    Serial.println("]");

    // Ignore any command that does not start with CALC.
    if (!input.startsWith("CALC ")) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wrong Cmd");
      lcd.setCursor(0, 1);
      lcd.print("Use CALC");

      digitalWrite(redLED, HIGH);
      digitalWrite(grnLED, LOW);

      Serial.print("ERROR: INVALID COMMAND -> ");
      Serial.println(input);

      prevhometime = millis();
      Home = false;
      return;
    }

    // Start removing CALC from string.
    String expr = input.substring(5);
    expr.replace(" ", "");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(expr);

    // Look for the operator(PARSE)
    int opPos = -1;
    char op = 0;
    for (int i = 0; i < expr.length(); i++) {
      if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
        op = expr[i];
        opPos = i;
        break;
      }
    }
    
    //If we don't see an operator, something is wrong on input.
    if (opPos <= 0) {
      lcd.setCursor(0, 1);
      lcd.print("Error Format");
      digitalWrite(redLED, HIGH);
      Serial.println("ERROR: FORMAT");

      prevhometime = millis();
      Home = false;
      return;
    }

    // Divide the string into the right and left arguments.
    String left = expr.substring(0, opPos);
    String right = expr.substring(opPos + 1);

    float a = left.toFloat();
    float b = right.toFloat();
    float result = 0;
    bool errorFlag = false;

    // Calculate.
    if (op == '+') result = a + b;
    else if (op == '-') result = a - b;
    else if (op == '*') result = a * b;
    else if (op == '/') {
      if (b == 0) errorFlag = true; // No divide by zero!!!!
      else result = a / b;
    } else errorFlag = true;

    // Display result on the LCD
    lcd.setCursor(0, 1);
    if (errorFlag) {
      if (op == '/' && b == 0) lcd.print("Error! Div by 0!");
      else lcd.print("Error!");
      digitalWrite(redLED, HIGH);
    } else {
      lcd.print("= ");
      //If it is a whole number, clean result
      if (result == (int)result) lcd.print((int)result);
      else lcd.print(result, 4);

      Serial.print("RESULT: ");
      Serial.println(result, 4);
    }

    prevhometime = millis();
    Home = false;
    digitalWrite(grnLED, LOW);
  }

  // When 5 seconds has passed, reset to home screen
  if (!Home && (millis() - prevhometime >= displayDuration)) {
    HomeScr();
    Home = true;
  }
}

// Home screen
void HomeScr() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Arduino Calc");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}

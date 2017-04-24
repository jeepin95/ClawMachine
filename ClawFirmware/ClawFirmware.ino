#include <Wire.h>

#include <LiquidCrystal.h>
String VERSION = "0.1";
//
// Limit Switches
//
#define X_MIN_PIN           3
#define X_MAX_PIN         2
#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

//
// Joystick
//
#define X_JOY_PIN          13
#define X_JOY_CENTER       677
#define Y_JOY_PIN          14
#define Y_JOY_CENTER       666
#define JOY_PLAY           20

//
// Steppers
//
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_CS_PIN           53

#define Z_STEP_PIN         60
#define Z_DIR_PIN          61
#define Z_ENABLE_PIN       56
#define Z_CS_PIN           49

#define Y_STEP_PIN         46
#define Y_DIR_PIN          48
#define Y_ENABLE_PIN       62
#define Y_CS_PIN           40

//
// LCD
//
#define BEEPER_PIN         37
#define BTN_EN1            31
#define BTN_EN2            33
#define BTN_ENC            35
#define SD_DETECT_PIN      49
#define KILL_PIN           41
#define LCD_BACKLIGHT_PIN  39
#define LCD_I2C_TYPE_PCF8575
#define LCD_I2C_ADDRESS 0x27   // I2C Address of the port expander
#define ULTIPANEL
#define NEWPANEL
#define LCD_PINS_RS 16
#define LCD_PINS_ENABLE 17
#define LCD_PINS_D4 23
#define LCD_PINS_D5 25
#define LCD_PINS_D6 27
#define LCD_PINS_D7 29

#define X_AXIS    0
#define Y_AXIS    1
#define Z_AXIS    2

#define X_MAX_POSITION 128000
#define Z_MAX_POSITION 5000
#define Y_MAX_POSITION 128000
#define STEPS         1500


unsigned long x_max_position = X_MAX_POSITION;
unsigned long y_max_position = Y_MAX_POSITION;
unsigned long z_max_position = Z_MAX_POSITION;

unsigned long x_current_position = 0;
unsigned long y_current_position = 0;
unsigned long z_current_position = 0;

unsigned int x_joy;
unsigned int y_joy;

String line0 = "";
String line1 = "";
String line2 = "";
String line3 = "";

static const unsigned long REFRESH_INTERVAL = 500;
static unsigned long lastRefreshTime = 0;

LiquidCrystal lcd(LCD_PINS_RS,LCD_PINS_ENABLE,LCD_PINS_D4,LCD_PINS_D5,LCD_PINS_D6,LCD_PINS_D7);

void setup() {
  // put your setup code here, to run once:
  pinMode(X_STEP_PIN,OUTPUT);
  pinMode(X_DIR_PIN,OUTPUT);
  pinMode(X_ENABLE_PIN,OUTPUT);
  pinMode(Y_STEP_PIN,OUTPUT);
  pinMode(Y_DIR_PIN,OUTPUT);
  pinMode(Y_ENABLE_PIN,OUTPUT);
  pinMode(Z_STEP_PIN,OUTPUT);
  pinMode(Z_DIR_PIN,OUTPUT);
  pinMode(Z_ENABLE_PIN,OUTPUT);
  pinMode(X_JOY_PIN,INPUT);
  pinMode(Y_JOY_PIN,INPUT);
  
  pinMode(3,INPUT);
  
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ellie's Claw Machine");
  lcd.setCursor(0,1);
  lcd.print("Version: " + VERSION);
  delay(5000);
  digitalWrite(X_ENABLE_PIN,LOW);
  digitalWrite(Y_ENABLE_PIN,LOW);
  digitalWrite(Z_ENABLE_PIN,LOW);


}

void move_stepper(int axis,int dir) {
  int pin;
  if(dir == 0) {
    dir = -1;
  }
  int dir_pin;
  long next_position;
  
  bool continue_move = false;
  switch(axis) {
    case X_AXIS:
      pin = X_STEP_PIN;
      dir_pin = X_DIR_PIN;
      next_position = x_current_position + (dir*STEPS);
      if(next_position >= 0 && next_position <= x_max_position) {
        continue_move = true;
      } else {
        continue_move = false;
      }

      if (next_position < 0) {
        x_current_position = 0;
      } else if (next_position > x_max_position) {
        x_current_position = x_max_position;
      } else {
        x_current_position = next_position;
      }
      break;
    case Y_AXIS:
      pin = Y_STEP_PIN;
      dir_pin = Y_DIR_PIN;
      next_position = y_current_position + (dir*STEPS);
      if(next_position >= 0 && next_position <= y_max_position) {
        continue_move = true;
      } else {
        continue_move = false;
      }

      if (next_position < 0) {
        y_current_position = 0;
      } else if (next_position > y_max_position) {
        y_current_position = y_max_position;
      } else {
        y_current_position = next_position;
      }
      break;
    case Z_AXIS:
      pin = Z_STEP_PIN;
      dir_pin = Z_DIR_PIN;
      next_position = z_current_position + (dir*STEPS);
      if( next_position >= 0 && next_position <= z_max_position ) {
        continue_move = true;
      } else { continue_move = false; }
      if(next_position < 0) {
        z_current_position = 0;
      } else if (next_position > z_max_position) {
        z_current_position = z_max_position;
      }
      else {
        z_current_position = next_position;
      }
      break;
  }
  if(continue_move == true) {
    if(dir == 1) {
      digitalWrite(dir_pin,HIGH);
    } else {
      digitalWrite(dir_pin,LOW);
    }
    for(int x = 0; x < STEPS; x++) {
      digitalWrite(pin,HIGH);
      delayMicroseconds(50);
      digitalWrite(pin,LOW);
      delayMicroseconds(50);
    }
  }
}

void updateDisplay() {
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL) {
    lcd.clear();
    line0 = "X:" + String(x_current_position) + " Y:" + String(y_current_position) + " Z:" + String(z_current_position);
    lastRefreshTime += REFRESH_INTERVAL;
    lcd.setCursor(0,0);
    lcd.print(line0);
    //lcd.setCursor(0,1);
    //lcd.print(line1);
    lcd.setCursor(0,2);
    lcd.print("GO");
    //lcd.setCursor(0,3);
    //lcd.print(line3);
  }
}
int getDirection(int current, int center) {
  if(current < (center-JOY_PLAY)) {
    return -1;
  } else if (current > center+JOY_PLAY) {
    return 1;
  } else {
    return 0;
  }
}

int x_dir;
int y_dir;
int z_dir;

void loop() {
  // put your main code here, to run repeatedly:
  x_joy = analogRead(X_JOY_PIN);
  y_joy = analogRead(Y_JOY_PIN);
  x_dir = getDirection(x_joy,X_JOY_CENTER);
  y_dir = getDirection(y_joy,Y_JOY_CENTER);
  if(x_dir == 1) { move_stepper(Z_AXIS,HIGH); } else if (x_dir == -1) { move_stepper(Z_AXIS,LOW); }
  if(y_dir == 1) { move_stepper(Y_AXIS,HIGH); } else if (y_dir == -1) { move_stepper(Y_AXIS,LOW); }
  if(x_dir == 0 && y_dir == 0) {
    updateDisplay();
  }
  
}

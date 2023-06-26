#include <Arduino.h>
#include <TM1637TinyDisplay.h>
#include <Bounce2.h>

// =========== HAL (Pins) =========== 
#define IDX_RED     0
#define PIN_RED_PWM 5
#define PIN_RED_CLK 2
#define PIN_RED_DAT 3
#define PIN_RED_BTN 4

#define IDX_YELLOW     1
#define PIN_YELLOW_PWM 6
#define PIN_YELLOW_CLK 7
#define PIN_YELLOW_DAT 8
#define PIN_YELLOW_BTN 15

#define IDX_GREEN     2
#define PIN_GREEN_PWM 9
#define PIN_GREEN_CLK 20  // A2
#define PIN_GREEN_DAT 21  // A3
#define PIN_GREEN_BTN 19  // A1

#define IDX_BLUE     3
#define PIN_BLUE_PWM 10
#define PIN_BLUE_CLK 16
#define PIN_BLUE_DAT 14
#define PIN_BLUE_BTN 18  // A0
// ================================== 

const byte pins_clk[] = {PIN_RED_CLK, PIN_YELLOW_CLK, PIN_GREEN_CLK, PIN_BLUE_CLK};
const byte pins_dat[] = {PIN_RED_DAT, PIN_YELLOW_DAT, PIN_GREEN_DAT, PIN_BLUE_DAT};
//TM1637TinyDisplay timers[4];


// Serial1 uses TX/RX pins and Serial is for debugging/testing
//#define COMM_CHAN Serial1 // <==== for running production units
#define COMM_CHAN Serial // <==== for testing purposes

// Displays
TM1637TinyDisplay displays[] = {
  TM1637TinyDisplay(PIN_RED_CLK, PIN_RED_DAT),
  TM1637TinyDisplay(PIN_YELLOW_CLK, PIN_YELLOW_DAT),
  TM1637TinyDisplay(PIN_GREEN_CLK, PIN_GREEN_DAT),
  TM1637TinyDisplay(PIN_BLUE_CLK, PIN_BLUE_DAT)
};

// Buttons
#define DEBOUNCE_INTERVAL 10 // set global 10 ms debounce value
const byte pins_btn[] = {PIN_RED_BTN, PIN_YELLOW_BTN, PIN_GREEN_BTN, PIN_BLUE_BTN};
Bounce2::Button buttons[4];


// LED PWM stuff
const byte pins_led[] = {PIN_RED_PWM, PIN_YELLOW_PWM, PIN_GREEN_PWM, PIN_BLUE_PWM};
byte led_brightness[] = {0, 0, 0, 0};
byte disp_brightness[] = {0, 0, 0, 0};
long blink_timer = millis();
byte blink_leds[] = {1,0,0,0};
byte blink_disp[] = {1,0,0,0};
boolean blink_state = true;
int blink_cycle = 100;


// COMM MSGs
const char msg_start[] PROGMEM = ">>";
const char msg_upper_colors[] = "RYGB";
const char msg_lower_colors[] = "rygb";
const char msg_end[] PROGMEM = "<<";
const char disp_lines[] PROGMEM = "----";


// misc
bool sent_init = false;

void setButtonBrightness(byte color, byte brightness, bool save=true) {
  if (color > 3) return;
  if (brightness > 7) brightness = 7;
  if (save) led_brightness[color] = brightness;
  if (brightness > 0) {
    byte pwm_level = 255;
    pwm_level = pwm_level >> (7 - brightness);
    analogWrite(pins_led[color], pwm_level);
  } else {
    digitalWrite(pins_led[color], LOW);
  }  
}

void setDispBrightness(byte color, byte brightness, bool save=true) {
  if (color > 3) return;
  if (brightness > 7) brightness = 7;
  if (save) disp_brightness[color] = brightness;
  if (brightness > 0) {
    displays[color].setBrightness(brightness, true);
  } else {
    displays[color].setBrightness(brightness, false);
  }
  
}

void doBlink() {
  if ((millis() - blink_timer) > blink_cycle) {
    blink_timer = millis();
    // toggle
    blink_state = !blink_state;
    for (byte i=0; i<4; i++) {
      if (blink_state) {
        if (blink_leds[i]) setButtonBrightness(i, led_brightness[i], false);
        if (blink_disp[i]) setDispBrightness(i, disp_brightness[i], false);
      } else {
        if (blink_leds[i]) setButtonBrightness(i, 0, false);
        if (blink_disp[i]) setDispBrightness(i, 0, false);
      }
    }
  }
}


void setup() {
  // setup the communications channel
  COMM_CHAN.begin(9600);
  // main setup loop
  for (byte i=0; i<4; i++) {
    // setup the buttons  
    buttons[i].attach(pins_btn[i], INPUT_PULLUP);
    buttons[i].interval(DEBOUNCE_INTERVAL);
    buttons[i].setPressedState(LOW);
    // setup the displays
    displays[i].begin();
    setDispBrightness(i, 0);
    displays[i].showString_P(disp_lines);
    for (int k = 0; k < 8; k++) {
      setButtonBrightness(i, k);
      setDispBrightness(i, k);
      delay(50);
    }
  }  
  COMM_CHAN.println("4-Team Counter by @hackerceo");
} 

void loop() {
  // send init message?
  //if (!sent_init) {
  //  if (millis() > 500) {
  //    sent_init = true;
  //    COMM_CHAN.println("4-Team Counter by @hackerceo");
  //  }
  //}


  // ===== Deal with blinking =====
  doBlink();

  
  // ===== React to any button changes =====
  // ">>R+<<" means red was pressed
  // ">>R-<<" means red was released
  // ">>Y+<<" means yellow was pressed
  // ">>Y-<<" means yellow was released
  // ">>G+<<" means green was pressed
  // ">>G-<<" means green was released
  // ">>B+<<" means blue was pressed
  // ">>B-<<" means blue was released
  for (byte i=0; i<4; i++) {
    buttons[i].update();
    if (buttons[i].changed()) {
      COMM_CHAN.print((const __FlashStringHelper *) msg_start);
      COMM_CHAN.print(msg_upper_colors[i]);
      if (buttons[i].isPressed()) {
        COMM_CHAN.print("+"); // pressed
      } else {
        COMM_CHAN.print("-"); // released
      }
      COMM_CHAN.println((const __FlashStringHelper *) msg_end);
    }
  }
  



}



// ">>SryGb<<" means status result with red,yellow,and blue being released and green being pressed

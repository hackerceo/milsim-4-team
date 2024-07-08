#include <Arduino.h>
#include <TM1637TinyDisplay.h>
#include <Bounce2.h>


// WITH DEBUG:
// Sketch uses 11638 bytes (40%) of program storage space. Maximum is 28672 bytes.
// Global variables use 783 bytes (30%) of dynamic memory, leaving 1777 bytes for local variables. Maximum is 2560 bytes.

// WITHOUT DEBUG:
// Sketch uses 11288 bytes (39%) of program storage space. Maximum is 28672 bytes.
// Global variables use 571 bytes (22%) of dynamic memory, leaving 1989 bytes for local variables. Maximum is 2560 bytes.

// PRE-BUFFER
// Sketch uses 12650 bytes (44%) of program storage space. Maximum is 28672 bytes.
// Global variables use 865 bytes (33%) of dynamic memory, leaving 1695 bytes for local variables. Maximum is 2560 bytes.

// POST-BUFFER
// Sketch uses 12562 bytes (43%) of program storage space. Maximum is 28672 bytes.
// Global variables use 920 bytes (35%) of dynamic memory, leaving 1640 bytes for local variables. Maximum is 2560 bytes.


#define XXX_DEBUG

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
#define COMM_CHAN Serial1 // <==== for running production units
//#define COMM_CHAN Serial // <==== for testing purposes

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

extern void setButtonBrightness(byte color, byte brightness, bool save);
extern void setDispBrightness(byte color, byte brightness, bool save);


// LED PWM stuff
const byte pins_led[] = {PIN_RED_PWM, PIN_YELLOW_PWM, PIN_GREEN_PWM, PIN_BLUE_PWM};
byte led_brightness[] = {0, 0, 0, 0};
byte disp_brightness[] = {0, 0, 0, 0};
bool disp_loop_animation[] = {0, 0, 0, 0};

long blink_timer = millis();
byte blink_leds[] = {0,0,0,0};
byte blink_disp[] = {0,0,0,0};
boolean blink_state = true;
int blink_cycle = 100;
byte blink_ratio = 50;


// COMM MSGs
const char msg_start[] PROGMEM = ">>";
const char msg_upper_colors[] = "RYGB";
const char msg_lower_colors[] = "rygb";
const char msg_end[] PROGMEM = "<<";
const char disp_lines[] PROGMEM = "----";

byte master_buffer[35];


// uses the "arduino-fsm" library by Jon Black
#include "Fsm.h"
extern State state_Idle;
Fsm core_fsm(&state_Idle);

#define SIG_GO_NEXT           0
#define SIG_ABORT             1
#define SIG_START_BRIGHTNESS  2
#define SIG_START_BLINK       3
#define SIG_START_ANIMATE     4
#define SIG_START_DISPLAY     5
#define SIG_BLINK_RATIO       6
#define SIG_BLINK_CYCLE       7
#define SIG_BLINK_ACTIVATE    8
#define SIG_BLINK_DEACTIVATE  9
#define SIG_DISPLAY_SCROLL    10
#define SIG_DISPLAY_TEXT      11
#define SIG_ANI_BUILTIN       12
#define SIG_ANI_UPLOADED      13
#define SIG_ANI_


byte cnt_OpenClose = 0;
byte targetColor = 0;
byte brightnessLevel = 0;


// State which checks for a start sequence ">>"
// ********************************************************************************** //
State state_Idle(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: idle");
        #endif        
        cnt_OpenClose = 0;
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read();
        if (cnt_OpenClose == 2) {
          switch (data) {
            case 'B':
              core_fsm.trigger(SIG_START_BRIGHTNESS);
              break;
            case 'K':
              core_fsm.trigger(SIG_START_BLINK);
              break;
            case 'A':
              core_fsm.trigger(SIG_START_ANIMATE);
              break;
            case 'D':
              core_fsm.trigger(SIG_START_DISPLAY);
              break;
            default:
              cnt_OpenClose = 0;
              break;
          }
        } else {
          if (data == '>') {
            cnt_OpenClose++;
          } else {
            cnt_OpenClose = 0;
          }
        }
      }
    },
    NULL
);



// *  B = BRIGHTNESS
// *    R/Y/G/B
// *       0-7 = BRIGHTNESS LEVEL
// *  K = BLINK
// *    C = BLINK CYCLE TIME
// *       00-99 (?+1 * 10 ms) per cycle
// *    R = BLINK RATIO
// *       00-99 (??% of time it is on)
// *    A = ACTIVATE
// *         R/Y/G/B (color)
// *         D/B (optional) Display or Button  
// *    D = DEACTIVATE    
// *         R/Y/G/B (color)
// *         D/B (optional) Display or Button  
//    A = ANIMATE
//      D = FROM DEFAULT
//        1-5 (animation number)
//          R/Y/G/B (color)
//          01-99 (?*10 ms) per frame
//          L = LOOP?
//      U = FROM UPLOADED
//        1-5 (animation number)
//          R/Y/G/B (color)
//          01-99 (?*10 ms) per frame
//          L = LOOP?
//      T = TRANSFER
//        [full contents of the animation array to fill the EEPROM]
// *  D = DISPLAY
// *     S = scroll speed (if more than 4 chars)
// *         000 - 999 ms of scroll speed
// *     T = text string
// *       R/Y/G/B (color)
// *         :/_ (display colon or "_" if it is off) [this only works if 4 chars or less of input]
// *           [up to 25 chars to display]

// ">>SryGb<<" means status result with red,yellow,and blue being released and green being pressed

void nullFunction() {
}

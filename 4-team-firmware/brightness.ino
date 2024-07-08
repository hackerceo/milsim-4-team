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




// State BRIGHTNESS
// ********************************************************************************** //
State state_BrightnessColor(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: brightness color");
        #endif        
        targetColor = 0;
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read();
        switch (data) {
          case 'B':
            targetColor++;
          case 'G':
            targetColor++;
          case 'Y':
            targetColor++;
          case 'R':
            core_fsm.trigger(SIG_GO_NEXT);
            break;
          default:
            core_fsm.trigger(SIG_ABORT);
            break;
        }
      }
    },
    NULL
);
State state_BrightnessLevel(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: brightness level");
        #endif
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read();
        if (data >= '0' && data <= '7') {
          brightnessLevel = data - '0';  
          core_fsm.trigger(SIG_GO_NEXT);
        } else {
          core_fsm.trigger(SIG_ABORT);
        }
      }
    },
    NULL
);
State state_BrightnessClose(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: brightness close");
        #endif
        cnt_OpenClose = 0;
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read();
        if (data == '<') {
          cnt_OpenClose++;
          if (cnt_OpenClose == 2) {
            setButtonBrightness(targetColor, brightnessLevel, true);
            setDispBrightness(targetColor, brightnessLevel, true);
            core_fsm.trigger(SIG_GO_NEXT);
          }
        } else {
          core_fsm.trigger(SIG_ABORT);
        }
      }
    },
    NULL
);


void doBlink() {
  if ((millis() - blink_timer) > blink_cycle) {
    // restart blink
    blink_timer = millis();
    blink_state = true;
    for (byte i=0; i<4; i++) {
      if (blink_leds[i] == 1) setButtonBrightness(i, led_brightness[i], false);
      if (blink_disp[i] == 1) setDispBrightness(i, disp_brightness[i], false);
    }
  } else {
    // check ratio
    if (blink_state && ((int)((((float)(millis() - blink_timer) / (float)blink_cycle)) * 100)) > blink_ratio) {      
      // we are over our ratio level, turn off blink
      blink_state = false;
      for (byte i=0; i<4; i++) {
        if (blink_leds[i] == 1) setButtonBrightness(i, 0, false);
        if (blink_disp[i] == 1) setDispBrightness(i, 0, false);
      }
    }
  }
}



// State BLINK
// ********************************************************************************** //
State state_BlinkCommand(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: blink action command");
        #endif
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read();
        switch (data) {
          case 'C':
            core_fsm.trigger(SIG_BLINK_CYCLE);
            break;
          case 'R':
            core_fsm.trigger(SIG_BLINK_RATIO);
            break;
          case 'A':
            core_fsm.trigger(SIG_BLINK_ACTIVATE);
            break;
          case 'D':
            core_fsm.trigger(SIG_BLINK_DEACTIVATE);
            break;
          default:
            core_fsm.trigger(SIG_ABORT);
            break;
        }
      }
    },
    NULL
);
State state_BlinkRatio(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: blink ratio");
        #endif
    },
    [](){
        size_t data = COMM_CHAN.readBytesUntil('<', master_buffer, 3);
        if (data == 2) {
          signed int validate = (master_buffer[0] - '0');
          if (validate > 9 || validate < 0) {
            core_fsm.trigger(SIG_ABORT);
          } else {
            blink_ratio = validate * 10; 
            validate = (master_buffer[1] - '0');
            if (validate > 9 || validate < 0) {
              core_fsm.trigger(SIG_ABORT);
            } else {
              blink_ratio += validate;     
              #ifdef DEBUG
              Serial.print("ratio: ");
              Serial.print(blink_ratio);
              Serial.println("%");
              #endif
              core_fsm.trigger(SIG_GO_NEXT);
            }
          }
        } else {
          core_fsm.trigger(SIG_ABORT);
        }
    },
    NULL
);
State state_BlinkCycle(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: blink cycle");
        #endif
    },
    [](){
        size_t data = COMM_CHAN.readBytesUntil('<', master_buffer, 3);
        if (data == 2) {
          signed int validate = (master_buffer[0] - '0');
          if (validate > 9 || validate < 0) {
            core_fsm.trigger(SIG_ABORT);
          } else {
            blink_cycle = validate * 10; // Seconds
            validate = (master_buffer[1] - '0');
            if (validate > 9 || validate < 0) {
              core_fsm.trigger(SIG_ABORT);
            } else {              
              blink_cycle += validate; // 0.1 Seconds
              blink_cycle *= 100; // change to MS
              #ifdef DEBUG
              Serial.print("cycle time: ");
              Serial.print(blink_cycle);
              Serial.println(" ms");
              #endif
              core_fsm.trigger(SIG_GO_NEXT);            
            }
          }
        } else {
          core_fsm.trigger(SIG_ABORT);
        }
    },
    NULL
);

State state_BlinkActivate(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: blink activate");
        #endif
        targetColor = 0;
    },
    [](){
        size_t data = COMM_CHAN.readBytesUntil('<', master_buffer, 3);
        if (data != 0) {
          #ifdef DEBUG
          Serial.print("Color: ");
          Serial.println((char)master_buffer[0]);
          #endif
          switch(master_buffer[0]) {
            case 'B':
              targetColor++;
            case 'G':
              targetColor++;
            case 'Y':
              targetColor++;
            case 'R':
              if (data == 1 || (data > 1 && master_buffer[1] == 'B')) blink_leds[targetColor] = 1;
              if (data == 1 || (data > 1 && master_buffer[1] == 'D')) blink_disp[targetColor] = 1;
              core_fsm.trigger(SIG_GO_NEXT);
              break;
            default:
              core_fsm.trigger(SIG_ABORT);
              break;
          }
        } else {
          core_fsm.trigger(SIG_ABORT);
        }
    },
    NULL
);
State state_BlinkDeactivate(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: blink deactivate");
        #endif
        targetColor = 0;
    },
    [](){
        size_t data = COMM_CHAN.readBytesUntil('<', master_buffer, 3);
        if (data != 0) {
          #ifdef DEBUG
          Serial.print("Color: ");
          Serial.println((char)master_buffer[0]);
          #endif
          switch(master_buffer[0]) {
            case 'B':
              targetColor++;
            case 'G':
              targetColor++;
            case 'Y':
              targetColor++;
            case 'R':              
              if (data == 1 || (data > 1 && master_buffer[1] == 'B')) {
                blink_leds[targetColor] = 0;
                setButtonBrightness(targetColor, led_brightness[targetColor], false);
              }
              if (data == 1 || (data > 1 && master_buffer[1] == 'D')) {
                blink_disp[targetColor] = 0;
                setDispBrightness(targetColor, disp_brightness[targetColor], false);
              }
              core_fsm.trigger(SIG_GO_NEXT);
              break;
            default:
              core_fsm.trigger(SIG_ABORT);
              break;
          }
        } else {
          core_fsm.trigger(SIG_ABORT);
        }
    },
    NULL
);

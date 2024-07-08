int master_scroll_speed = 100;
byte master_disp_text[4][25];


// State DISPLAY
// ********************************************************************************** //
State state_DisplayCommand(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: display command");
        #endif
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read(); 
        switch (data) {
          case 'S':
            core_fsm.trigger(SIG_DISPLAY_SCROLL);
            break;
          case 'T':
            core_fsm.trigger(SIG_DISPLAY_TEXT);
            break;
          default:
            core_fsm.trigger(SIG_ABORT);
            break;
        }
      }
    },
    NULL
);


State state_DisplayScroll(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: display scroll");
        #endif
    },
    [](){
        size_t data = COMM_CHAN.readBytesUntil('<', master_buffer, 3);
        if (data != 3) {
              core_fsm.trigger(SIG_ABORT);
        } else {
          byte temp;
          master_scroll_speed = 0;
          for (byte i=0; i<3; i++) {
            temp = master_buffer[i] - '0';
            if (temp > 9) {
              core_fsm.trigger(SIG_ABORT);
              break;
            } else {
              switch(i) {
                case 0:
                  master_scroll_speed += (temp * 100);
                  break;
                case 1:
                  master_scroll_speed += (temp * 10);
                  break;
                case 2:
                  master_scroll_speed += temp;
                  break;
              }
            }
          }
  
          #ifdef DEBUG
          Serial.print("Scroll Speed: ");
          Serial.println(master_scroll_speed);
          #endif
          
          // update all the scroll speeds
          for (byte i=0; i<4; i++) {
            displays[i].setScrolldelay(master_scroll_speed);
          }
          
          core_fsm.trigger(SIG_GO_NEXT);
        }
    },
    NULL
);

State state_DisplayText(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: display text");
        #endif
        targetColor = 0;
    },
    [](){
        size_t data = COMM_CHAN.readBytesUntil('<', master_buffer, 27);
        if (data < 2) {
          core_fsm.trigger(SIG_ABORT);          
        } else {
          #ifdef DEBUG
          Serial.print("Color: ");
          Serial.println((char)master_buffer[0]);
          #endif
          // get color
          switch(master_buffer[0]) {
            case 'B':
              targetColor++;
            case 'G':
              targetColor++;
            case 'Y':
              targetColor++;
            case 'R':
              break;
            default:
              core_fsm.trigger(SIG_ABORT);
              return;
              break;
          }
          // display colon
          byte dots = 0;
          if (master_buffer[1] == ':') dots = 0b01000000;

          // prepare display string
          for (byte i=0; i<25; i++) {
            if (i < data - 2) {
              master_disp_text[targetColor][i] = master_buffer[i+2];
            } else {
              master_disp_text[targetColor][i] = 0;
            }
          }
          data = data - 2;
          // display the text
          if (data > 4) {
            displays[targetColor].startStringScroll(master_disp_text[targetColor], master_scroll_speed);
          } else {
            displays[targetColor].showString(master_disp_text[targetColor], 4, 0, dots);
          }
          core_fsm.trigger(SIG_GO_NEXT);
        }
    },
    NULL
);

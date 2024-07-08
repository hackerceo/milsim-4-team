// Animate
void fuckshit() {}


State state_AnimateCommand(
    [](){
        #ifdef DEBUG
        Serial.println("STATE: animate action command");
        #endif
    },
    [](){
      if (COMM_CHAN.available() > 0) {
        char data = COMM_CHAN.read();
        switch (data) {
          case 'D':
            // display build-in animation
            core_fsm.trigger(SIG_BLINK_CYCLE);
            break;
          case 'U':
            // display an animation that was previously uploaded
            core_fsm.trigger(SIG_BLINK_RATIO);
            break;
          case 'T':
            // start transfer of new animations into the EEPROM for later use by the 'U' subcommand
            core_fsm.trigger(SIG_BLINK_ACTIVATE);
            break;
          default:
            core_fsm.trigger(SIG_ABORT);
            break;
        }
      }
    },
    NULL
);

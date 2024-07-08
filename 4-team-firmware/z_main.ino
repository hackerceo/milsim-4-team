void setup() {

  // setup the communications channel
  COMM_CHAN.begin(19200);
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


  // create FSM transitions
// wait 2 second upon startup for a button to be pressed, timeout to the GameModeSelect state
//  core__fsm.add_timed_transition(&core__state_CheckReset, &core__state_GameModeSelect, 2000, NULL);
  
  core_fsm.add_transition(&state_Idle, &state_BrightnessColor, SIG_START_BRIGHTNESS, NULL);
  core_fsm.add_transition(&state_BrightnessColor, &state_BrightnessLevel, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_BrightnessColor, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_BrightnessLevel, &state_BrightnessClose, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_BrightnessLevel, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_BrightnessClose, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_BrightnessClose, &state_Idle, SIG_GO_NEXT, NULL);

  core_fsm.add_transition(&state_Idle, &state_BlinkCommand, SIG_START_BLINK, NULL);
  core_fsm.add_transition(&state_BlinkCommand, &state_BlinkCycle, SIG_BLINK_CYCLE, NULL);
  core_fsm.add_transition(&state_BlinkCommand, &state_BlinkRatio, SIG_BLINK_RATIO, NULL);
  core_fsm.add_transition(&state_BlinkCommand, &state_BlinkActivate, SIG_BLINK_ACTIVATE, NULL);
  core_fsm.add_transition(&state_BlinkCommand, &state_BlinkDeactivate, SIG_BLINK_DEACTIVATE, NULL);
  core_fsm.add_transition(&state_BlinkCycle, &state_Idle, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_BlinkCycle, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_BlinkRatio, &state_Idle, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_BlinkRatio, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_BlinkActivate, &state_Idle, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_BlinkActivate, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_BlinkDeactivate, &state_Idle, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_BlinkDeactivate, &state_Idle, SIG_ABORT, NULL);

  core_fsm.add_transition(&state_Idle, &state_DisplayCommand, SIG_START_DISPLAY, NULL);
  core_fsm.add_transition(&state_DisplayCommand, &state_DisplayScroll, SIG_DISPLAY_SCROLL, NULL);
  core_fsm.add_transition(&state_DisplayCommand, &state_DisplayText, SIG_DISPLAY_TEXT, NULL);
  core_fsm.add_transition(&state_DisplayCommand, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_DisplayScroll, &state_Idle, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_DisplayScroll, &state_Idle, SIG_ABORT, NULL);
  core_fsm.add_transition(&state_DisplayText, &state_Idle, SIG_GO_NEXT, NULL);
  core_fsm.add_transition(&state_DisplayText, &state_Idle, SIG_ABORT, NULL);

  core_fsm.add_transition(&state_Idle, &state_AnimateCommand, SIG_START_ANIMATE, NULL);


  
//  core_fsm.add_transition(&, &, , NULL);
//  core_fsm.add_transition(&, &, , NULL);
//  core_fsm.add_transition(&, &, , NULL);
  








  
  COMM_CHAN.println("4-Team Counter by @hackerceo");
  COMM_CHAN.println("http://github.com/hackerceo");
  COMM_CHAN.println(">>4-TEAM_START<<");
} 

void loop() {
  // ===== Deal with blinking =====
  doBlink();

  // ===== Deal with running the requested functions =====
  core_fsm.run_machine();

  // ===== Deal with display animations/scrolling =====
  bool isDone;
  for (byte i=0; i<4; i++) {
    isDone = displays[i].Animate(disp_loop_animation[i]);
  }
  
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

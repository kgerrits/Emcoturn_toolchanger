void StateMachineToolchange() {
  switch (state) {

    case STATE_INIT:
      // no tool engaged
      MotorPWM = 0;

      if (toolRequest != toolConfirm && toolRequest != -1 && homingRequired == false && toolChangerEnabled == true) {
        toolchangeStartTime = millis(); // monitor time taken for toolchange
        state = STATE_ADVANCE_TOOL;
      }

      if (homingRequired == true && toolChangerEnabled == true && startHoming == true) {
        toolchangeStartTime = millis(); // monitor time taken for toolchange
        state = STATE_HOMING;
      }

      break;

    case STATE_ADVANCE_TOOL:
      // move tool forwards
      MotorPWM = motorAdvancePWM;

      if (toolRequest == toolNo) {
        backingDelayStart = millis();
        state = STATE_BACKING_DELAY;
      }
      break;

    case STATE_BACKING_DELAY:
      // wait to fully pass ratched before backing up
      MotorPWM = motorAdvancePWM;

      if (millis() - backingDelayStart >= backingDelay && toolRequest == toolNo) {
        backingTimeoutStart = millis();
        state = STATE_ENGAGE_RATCHET;
      }
      break;

    case STATE_ENGAGE_RATCHET:
      // move backwards until ratchet engaged
      MotorPWM = motorBackupPWM;

      if (millis() - backingTimeoutStart >= backingTimeout) {
        Serial.println("Toolchange error!");
        state = STATE_INIT;
      }

      if (filteredCurrent < -110) {
        // ratchet engaged
        state = STATE_HOLDING_CURRENT;
      }
      break;

    case STATE_HOLDING_CURRENT:
      // lower PWM to keep ratchet engaged
      {
        MotorPWM = motorHoldingPWM;
        long timeTaken = millis() - toolchangeStartTime;

        toolConfirm = toolRequest;
        Serial.print("completed in: ");
        Serial.print(timeTaken);
        Serial.print("[ms] | Active tool: ");
        Serial.println(toolConfirm);
        state = STATE_CONFIRM_TOOL;

      }
      break;

    case STATE_CONFIRM_TOOL:
      // desired tool position reached
      MotorPWM = motorHoldingPWM;

      if (toolRequest != toolConfirm && homingRequired == false) {
        toolchangeStartTime = millis(); // monitor time taken for toolchange
        state = STATE_ADVANCE_TOOL;
      }

      if (toolConfirm == 1 && homingRequired == true) {
        // confirmed tool equals homing tool. Set homing required flag to false
        homingRequired = false;
        startHoming  = false;
        Serial.println("Homing complete");
      }

      break;

    case STATE_HOMING:
      // perform homing routine
      // homing --> goto tool 1
      Serial.println("Start homing...");
      toolRequest = 1;
      state = STATE_ADVANCE_TOOL;
      break;

  }

  // check toolchanger enable conditions

  if (toolChangerEnabled == true && _toolChangerEnabled != toolChangerEnabled) {
    MotorPWM = 0;
    digitalWrite(MSLEEP, HIGH);  // enable motor driver
    digitalWrite(LEDPIN, HIGH);
    Serial.println("Toolchanger enabled");
  }
  else if (toolChangerEnabled == false && _toolChangerEnabled != toolChangerEnabled) {
    state = STATE_INIT;
    MotorPWM = 0;
    digitalWrite(MSLEEP, LOW);  // disable motor driver
    digitalWrite(LEDPIN, LOW);
    Serial.println("Toolchanger disabled");
  }

  if (toolChangerEnabled == true && _toolChangerEnabled == false) {
    Serial.println("Homing required");
    homingRequired = true;
  }
  _toolChangerEnabled = toolChangerEnabled; // remember previous state of toolchanger enable
}

boolean validateToolRequest(int request) {
  // Toolchanger has 8 positions, if toolrequest >8 is received, it will be modulo 8.
  // If request is valid, tool position 1-8 will be returned
  // If request is invalid, -1 will be returned.

  if (request <= 0) {
    return false;
  }
  else {
    int toolRangeMapped = request % 8;

    if (toolRangeMapped == 0) {
      toolRangeMapped = 8; //--> multiple of 8 % 8 = 0 --> meaning if answer is 0, tool 8 must be called
    }

    if (toolRangeMapped > 0 && toolRangeMapped <= 8) {
      // double check if tool is in range of toolchanger
      toolRequest = toolRangeMapped;
      return true;
    }
    else {
      return false;
    }

  }

}

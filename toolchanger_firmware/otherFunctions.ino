void signalConditioning() {
  rawCurrent = analogRead(MFB) * sign(MotorPWM);
  filteredCurrent = rawCurrent * alphaCurrent + (1 - alphaCurrent) * lastCurrentFilt;
  lastCurrentFilt = filteredCurrent;


  // check for enable signal
  toolChangerEnabled = digitalRead(ENAPIN);

}

int sign(int var) {
  if (var < 0) {
    return -1;
  }
  if (var > 0) {
    return 1;
  }
  if (var == 0) {
    return 0;
  }
}

void displayValues() {
  Serial.print(MotorPWM);
  Serial.print(", ");
  Serial.print(filteredCurrent);
  Serial.print(", ");
  Serial.print(positionState[0]);
  Serial.print(positionState[1]);
  Serial.print(positionState[2]);
  Serial.print(positionState[3]);
  Serial.print(", ");
  Serial.print("Pos:");
  Serial.print(toolNo);
  Serial.print(", ");
  Serial.print("State:");
  Serial.println(state);
}

void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  //  sprintf(format, "0x%%.%dX", precision);
  sprintf(format, "%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}

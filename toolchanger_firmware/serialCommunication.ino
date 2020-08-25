void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  integerFromPC = atoi(strtokIndx);     // convert this part to an integer

  strtokIndx = strtok(NULL, ",");
  floatFromPC = atof(strtokIndx);     // convert this part to a float

}

void doParsedData() {
  if (strcmp(messageFromPC, "ENA") == 0) {
    if (integerFromPC == 1) {
      toolChangerEnabled = true;
    }
    else {
      toolChangerEnabled = false;
    }
  }
  if (strcmp(messageFromPC, "MOT") == 0) {
    MotorPWM = integerFromPC;
    Serial.print("MOTOR: ");
    Serial.println(MotorPWM);
  }
  if (strcmp(messageFromPC, "TOOL") == 0) {
    int rawToolRequest = integerFromPC;

    if (homingRequired == false) {
      boolean requestValid = validateToolRequest(rawToolRequest);

      if (requestValid == true) {
        Serial.println("Toolchange starting.");
      }
      else {
        Serial.println("Please request tool > 0.");
      }
    }
    else {
      Serial.println("Please home toolchanger first!");
    }
  }
  if (strcmp(messageFromPC, "HOME") == 0) {
    if (integerFromPC == 1) {
      startHoming  = true;
    }

  }

}

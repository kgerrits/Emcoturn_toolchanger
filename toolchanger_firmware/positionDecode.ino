void readSensor0(){
  positionState[0] = digitalRead(S1);
  positionDecode(positionState); // update tool position
}

void readSensor1(){
  positionState[1] = digitalRead(S2);
  positionDecode(positionState); // update tool position
}

void readSensor2(){
  positionState[2] = digitalRead(S3);
  positionDecode(positionState); // update tool position
}

void readSensor3(){
  positionState[3] = digitalRead(S4);
  positionDecode(positionState); // update tool position
}


void positionDecode(boolean switchState[4]) {

  if (switchState[0] == 1 && switchState[1] == 1 && switchState[2] == 1 && switchState[3] == 0) {
    toolNo = 1;
  }
  else if (switchState[0] == 1 && switchState[1] == 1 && switchState[2] == 0 && switchState[3] == 0) {
    toolNo = 2;
  }
  else if (switchState[0] == 1 && switchState[1] == 1 && switchState[2] == 0 && switchState[3] == 1) {
    toolNo = 3;
  }
  else if (switchState[0] == 1 && switchState[1] == 0 && switchState[2] == 0 && switchState[3] == 1) {
    toolNo = 4;
  }
  else if (switchState[0] == 1 && switchState[1] == 0 && switchState[2] == 1 && switchState[3] == 1) {
    toolNo = 5;
  }
  else if (switchState[0] == 0 && switchState[1] == 0 && switchState[2] == 1 && switchState[3] == 1) {
    toolNo = 6;
  }
  else if (switchState[0] == 0 && switchState[1] == 1 && switchState[2] == 1 && switchState[3] == 1) {
    toolNo = 7;
  }
  else if (switchState[0] == 0 && switchState[1] == 1 && switchState[2] == 1 && switchState[3] == 0) {
    toolNo = 8;
  }
}

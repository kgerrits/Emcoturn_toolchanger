void TurretMotor(int PWM) {

  if (PWM > 0) {
    //forward motion
    int PWMConstrain = constrain(PWM, 0 , motorPWMLimit);

    digitalWrite(MIN2, HIGH);
    analogWrite(MIN1, PWMConstrain);
  }
  else if (PWM < 0) {
    // backwards motion
    int absPWM = abs(PWM);
    int PWMConstrain = constrain(absPWM, 0 , motorPWMLimit);

    digitalWrite(MIN2, LOW);
    analogWrite(MIN1, PWMConstrain);

  }
  else {
    // free rotation (disable motor)
    digitalWrite(MIN1, LOW);
    digitalWrite(MIN2, LOW);
  }

}

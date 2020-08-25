#include <Metro.h>

// Firmware version
const char FirmVer[] = "V1.00";

// --------------------------------------------------------------------------------------------------------------------
// Pin definition
// --------------------------------------------------------------------------------------------------------------------
const int MIN1 = 5;
const int MIN2 = 6;
const int MSLEEP = 7;
const int MFB = A4;
const int MFAULT = 8;
const int LEDPIN = 13;
const int S4 = 17;
const int S3 = 16;
const int S2 = 15;
const int S1 = 14;
const int TX2PIN = 10; // serial 2 transmit pin
const int RX2PIN = 9; // serial 2 receive pin
const int TEPIN = 11; // transmit enable
const int ENAPIN = 23;

// --------------------------------------------------------------------------------------------------------------------
// Timer variables
// --------------------------------------------------------------------------------------------------------------------
const int SampleTimeSignalConditioning = 5; // [ms]
const int SampleTimeDisplayValues = 100; // [ms]
const int SampleTimeStateMachine = 5; // [ms]
unsigned long backingDelayStart;
int backingDelay = 120; // [ms]
unsigned long backingTimeoutStart;
int backingTimeout = 1000; // [ms] maximum time of full backwards motor current
long toolchangeStartTime = 0;

// declare timer objects
Metro signalConditioningTimer = Metro(SampleTimeSignalConditioning);
Metro displayValuesTimer = Metro(SampleTimeDisplayValues);
Metro stateMachineTimer = Metro(SampleTimeStateMachine);


// declare toolchange state variables
int state;
#define STATE_INIT 0
#define STATE_ADVANCE_TOOL 1
#define STATE_BACKING_DELAY 2
#define STATE_ENGAGE_RATCHET 3
#define STATE_HOLDING_CURRENT 4
#define STATE_CONFIRM_TOOL 5
#define STATE_HOMING 6

// --------------------------------------------------------------------------------------------------------------------
// declare variables
// --------------------------------------------------------------------------------------------------------------------
unsigned int motorMode = 0;
int motorCurrent = 0;
int motorPWMLimit = 2000;
int MotorPWM = 0;
int rawCurrent = 0;
float alphaCurrent = 0.01;
float filteredCurrent = 0;
float lastCurrentFilt = 0;
boolean positionState[4];
const int positionSensor[4] = {S1, S2, S3, S4};
int toolNo = 0;
int toolRequest;
int lastToolRequest;
int toolConfirm;
boolean toolChangerEnabled = false;
boolean _toolChangerEnabled = false; // previous toolChangerEnabled state
boolean homingRequired = true;  // when enable is toggled, homing of the toolchanger is required.
boolean startHoming = false;

// motor PWM commands
const int motorAdvancePWM = 2000;
const int motorBackupPWM = -2000;
const int motorHoldingPWM = -200;


// serial communication variables
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing
// variables to hold the parsed data
char messageFromPC[numChars] = {0};
int integerFromPC = 0;
float floatFromPC = 0.0;
boolean newData = false;

// modbus communication variables
#define ID   1                // slave ID
#define HWSERIAL Serial2

byte frameBuffer[256];
const unsigned int t35 = 750; // [us] specified in modbus protocol for baud > 19200
const unsigned int t15 = 550;  // [us] specified in modbus protocol for baud > 19200
unsigned long modbusT35TimerStart;
unsigned long modbusT15TimerStart;
byte frameLength = 0;
boolean frameOK = false;
boolean frameChecked = false;
const byte modbusResponseWMRLength = 8;
byte modbusResponseWMR[modbusResponseWMRLength]; // buffer containing response for modbus Write Multiple Registers
byte modbusResponseRHR[256]; // buffer containing response for modbus Read Holding Registers (length is depending on request, so set to max message length)
byte modbusResponseRDI[256]; // buffer containing response for modbus Read Discrete Inputs (length is depending on request, so set to max message length)
byte modbusResponseWSR[8]; 
byte modbusResponseWSC[8];
boolean RDI_states[32]; // array containing discrete states to be read through modbus 

// modbus receive function state variables
byte modbusReceiveState;
#define MB_STATE_INIT 0
#define MB_STATE_IDLE 1
#define MB_STATE_RECEPTION 2
#define MB_STATE_CONTROL_WAITING 3

// modbus process function state variables
byte modbusFunctionCode;
#define FC_READ_MULTIPLE_COILS 1
#define FC_READ_DISCRETE_INPUTS 2
#define FC_READ_HOLDING_REGISTERS 3
#define FC_READ_INPUT_REGISTER 4
#define FC_WRITE_SINGLE_COIL 5
#define FC_WRITE_SINGLE_REGISTER 6
#define FC_WRITE_MULTIPLE_COILS 15
#define FC_WRITE_MULTIPLE_REGISTERS 16
#define FC_REPORT_SLAVEID 17

void setup() {
  // set pinmodes
  pinMode(MIN1, OUTPUT);
  pinMode(MIN2, OUTPUT);
  pinMode(MSLEEP, OUTPUT);
  pinMode(MFB, INPUT);
  pinMode(MFAULT, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(ENAPIN, INPUT);

  // ADC setup
  analogWriteFrequency(5, 20000); // change PWM to inaudible 20 kHz
  analogWriteFrequency(6, 20000); // change PWM to inaudible 20 kHz
  analogWriteResolution(11);  // analogWrite value 0 to 1023, or 1024 for high

  // disable motor
  digitalWrite(MIN1, LOW);
  digitalWrite(MIN2, LOW);
  analogWrite(MIN1, 0);
  digitalWrite(MSLEEP, LOW);

  // attach interrupts for sensor pins
  attachInterrupt(S1, readSensor0, CHANGE);
  attachInterrupt(S2, readSensor1, CHANGE);
  attachInterrupt(S3, readSensor2, CHANGE);
  attachInterrupt(S4, readSensor3, CHANGE);

  // get initial sensor reading
  positionState[0] = digitalRead(S1);
  positionState[1] = digitalRead(S2);
  positionState[2] = digitalRead(S3);
  positionState[3] = digitalRead(S4);
  positionDecode(positionState);

  // initial tool request
  toolRequest = -1; // state machine init state (wait for command)

  // Setup serial communication
  Serial.begin(115200);

  // Setup modbus communication
  Serial2.setTX(TX2PIN);
  Serial2.setRX(RX2PIN);
  Serial2.transmitterEnable(TEPIN);
  HWSERIAL.begin(115200, SERIAL_8N2); // 115200 baud, 8-data-bits, no-parity, 2 stop bits

  // fill known data for modbus responses
  modbusResponseWMR[0] = ID;
  modbusResponseWMR[1] = FC_WRITE_MULTIPLE_REGISTERS;
  modbusResponseRHR[0] = ID;
  modbusResponseRHR[1] = FC_READ_HOLDING_REGISTERS;
  modbusResponseWSR[0] = ID;
  modbusResponseWSR[1] = FC_WRITE_SINGLE_REGISTER;
  modbusResponseWSC[0] = ID;
  modbusResponseWSC[1] = FC_WRITE_SINGLE_COIL;
  modbusResponseRDI[0] = ID;
  modbusResponseRDI[1] = FC_READ_DISCRETE_INPUTS;
  

  modbusReceiveState = STATE_INIT;
  modbusT35TimerStart = micros();

  Serial.println("Setup complete.");
}

void loop() {

  // serial communication
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    doParsedData();
    newData = false;
  }

  // modbus communication
  modbusReceive(); // watch for incoming messages

  // display message when available
  if (frameLength > 0 && modbusReceiveState == MB_STATE_IDLE) {
    // process modbus message
    modbusProcess();
  }

  // set motorPWM on each loop cycle
  TurretMotor(MotorPWM);

  // check for ellapsed timers
  if (displayValuesTimer.check() == 1) {
//    displayValues();
  }
  if (signalConditioningTimer.check() == 1) {
    signalConditioning();
    updateRDI_states();
  }
  if (stateMachineTimer.check() == 1) {
    StateMachineToolchange();
  }

}

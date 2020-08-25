void modbusProcess() {

  if (frameBuffer[0] == ID) {
    // message directed to this slave
    Serial.print("Received: ");

    for ( int ii = 0; ii < frameLength; ii++) {
      printHex(frameBuffer[ii], 2);
      Serial.print(" ");
    }
    Serial.println();
    modbusFunctionCode = frameBuffer[1];

    switch (modbusFunctionCode) {
      case FC_READ_MULTIPLE_COILS:
        {
          Serial.println("FC_READ_MULTIPLE_COILS");
        }
        break;

      case FC_READ_DISCRETE_INPUTS:
        {
          int RDI_starting_address = frameBuffer[2] * 256 + frameBuffer[3];
          int RDI_num_inputs = frameBuffer[4] * 256 + frameBuffer[5];


          byte RDI_process = ProcessReadDiscreteInputs(RDI_starting_address, RDI_num_inputs); // processes request, if succesfull places read values in RDI buffer

          if (RDI_process == 0) {

            byte num_RDI_bytes = 1 + (RDI_num_inputs / 8); // number of bytes needed to pack values
            byte RDI_response_length = 5 + num_RDI_bytes; // ID, FC, bytecount, statusbytes, CRC

            // pack discrete inputs into bytes
            byte RDI_bytes[num_RDI_bytes];

            for (int j = 0; j < num_RDI_bytes; j++) {
              for (int i = 0; i < 8; i++) {
                bitWrite(RDI_bytes[j], i, RDI_states[RDI_starting_address + i]);
              }
              modbusResponseRDI[3 + j] = RDI_bytes[j]; // put completed bytes in send buffer
            }

            // create response message
            modbusResponseRDI[2] = num_RDI_bytes;
            int CRC_RDI = ModRTU_CRC(modbusResponseRDI, RDI_response_length - 2);
            modbusResponseRDI[RDI_response_length - 2] = highByte(CRC_RDI);
            modbusResponseRDI[RDI_response_length - 1] = lowByte(CRC_RDI);

            for (int i = 0; i < RDI_response_length; i++) {
              HWSERIAL.write(modbusResponseRDI[i]);
            }

            Serial.print("Reply: ");
            for (int i = 0; i < RDI_response_length; i++) {
              printHex(modbusResponseRDI[i], 2);
              Serial.print(" ");
            }
            Serial.println();
          }

          if (RDI_process != 0) {
            // request not accepted, respond with error
            byte modbusResponseRDI_error[5];

            modbusResponseRDI_error[0] = ID;
            modbusResponseRDI_error[1] = 0x82; // error code = FC + 80
            modbusResponseRDI_error[2] = RDI_process; // exception code 01, 02, 03 or 04
            int CRC_RDI_error = ModRTU_CRC(modbusResponseRDI_error, 3);
            modbusResponseRDI_error[3] = highByte(CRC_RDI_error);
            modbusResponseRDI_error[4] = lowByte(CRC_RDI_error);

            for (int i = 0; i < 5; i++) {
              HWSERIAL.write(modbusResponseRDI_error[i]);
            }
          }

        }
        break;

      case FC_READ_INPUT_REGISTER:
        {
          Serial.println("Read input register");
        }
        break;

      case FC_WRITE_SINGLE_COIL:
        {
          int WSC_address = frameBuffer[2] * 256 + frameBuffer[3];

          int WSC_value;

          if (frameBuffer[4] == 0xFF && frameBuffer[5] == 0x00) {
            WSC_value = 1;
          }
          else if (frameBuffer[4] == 0x00 && frameBuffer[5] == 0x00) {
            WSC_value = 0;
          }

          byte WSC_process = ProcessWriteSingleCoil(WSC_address, WSC_value);

          if (WSC_process == 0) {
            // request processed succesfully. Respond accordingly
            modbusResponseWSC[2] = frameBuffer[2];
            modbusResponseWSC[3] = frameBuffer[3];
            modbusResponseWSC[4] = frameBuffer[4];
            modbusResponseWSC[5] = frameBuffer[5];
            int CRC_WSC = ModRTU_CRC(modbusResponseWSC, 6);
            modbusResponseWSC[6] = highByte(CRC_WSC);
            modbusResponseWSC[7] = lowByte(CRC_WSC);

            for (int i = 0; i < 8; i++) {
              HWSERIAL.write(modbusResponseWSC[i]);
            }

          }
          if (WSC_process != 0) {
            // request not accepted, respond with error
            byte modbusResponseWSC_error[5];

            modbusResponseWSC_error[0] = ID;
            modbusResponseWSC_error[1] = 0x85; // error code = FC + 80
            modbusResponseWSC_error[2] = WSC_process; // exception code 01, 02, 03 or 04
            int CRC_WSC_error = ModRTU_CRC(modbusResponseWSC_error, 3);
            modbusResponseWSC_error[3] = highByte(CRC_WSC_error);
            modbusResponseWSC_error[4] = lowByte(CRC_WSC_error);

            for (int i = 0; i < 5; i++) {
              HWSERIAL.write(modbusResponseWSC_error[i]);
            }
          }

          Serial.println("Write single coil");
          Serial.print("Address: ");
          Serial.println(WSC_address);
          Serial.print("Value: ");
          Serial.println(WSC_value);
          Serial.print("Process return: ");
          Serial.println(WSC_process, HEX);

        }
        break;

      case FC_WRITE_SINGLE_REGISTER:
        {
          Serial.println("FC_WRITE_SINGLE_REGISTER");
          int WSR_address = frameBuffer[2] * 256 + frameBuffer[3];
          int WSR_value = frameBuffer[4] * 256 + frameBuffer[5];

          byte WSR_process = ProcessWriteSingleRegister(WSR_address, WSR_value);

          if (WSR_process == 0) {

            // create response
            modbusResponseWSR[2] = frameBuffer[2];
            modbusResponseWSR[3] = frameBuffer[3];
            modbusResponseWSR[4] = frameBuffer[4];
            modbusResponseWSR[5] = frameBuffer[5];
            int CRC_WSR = ModRTU_CRC(modbusResponseWSR, 6);
            modbusResponseWSR[6] = highByte(CRC_WSR);
            modbusResponseWSR[7] = lowByte(CRC_WSR);

            for (int i = 0; i < 8; i++) {
              HWSERIAL.write(modbusResponseWSR[i]);
            }
            Serial.print("Reply: ");
            for (int i = 0; i < 8; i++) {
              printHex(modbusResponseWSR, 2);
              Serial.print(" ");
            }
            Serial.println();
          }

          if (WSR_process != 0) {
            // request not accepted, respond with error
            byte modbusResponseWSR_error[5];

            modbusResponseWSR_error[0] = ID;
            modbusResponseWSR_error[1] = 0x86; // error code = FC + 80
            modbusResponseWSR_error[2] = WSR_process; // exception code 01, 02, 03 or 04
            int CRC_WSR_error = ModRTU_CRC(modbusResponseWSR_error, 3);
            modbusResponseWSR_error[3] = highByte(CRC_WSR_error);
            modbusResponseWSR_error[4] = lowByte(CRC_WSR_error);

            for (int i = 0; i < 5; i++) {
              HWSERIAL.write(modbusResponseWSR_error[i]);
            }
            Serial.print("Reply: ");
            for (int i = 0; i < 5; i++) {
              printHex(modbusResponseWSR_error, 2);
              Serial.print(" ");
            }
            Serial.println();
          }

          Serial.println("Write single register");
          Serial.print("Address: ");
          Serial.println(WSR_address);
          Serial.print("Value: ");
          Serial.println(WSR_value);
          Serial.print("Process return: ");
          Serial.println(WSR_process, HEX);

        }
        break;

      case FC_WRITE_MULTIPLE_COILS:
        {
          Serial.println("Write multiple coils");
        }
        break;

      case FC_WRITE_MULTIPLE_REGISTERS:
        {
          Serial.println("FC_WRITE_MULTIPLE_REGISTERS");
          int startAddress = word(frameBuffer[2], frameBuffer[3]); // get highbyte and lowbyte of starting address
          int quantityRegisters = frameBuffer[4] * 256 + frameBuffer[5];   // get quantity of addresses to write
          int byteCount = frameBuffer[6];

          if (startAddress == 2 && quantityRegisters == 1) {
            // for testing only, do toolrequest
            int registerValue = frameBuffer[7] * 256 + frameBuffer[8];
            boolean requestValid = validateToolRequest(registerValue);

            if (requestValid == true) {
              Serial.print("Requested tool: ");
              Serial.println(registerValue);
            }
            else {
              Serial.print("Invalid request. ");
              Serial.print("Requested tool: ");
              Serial.println(registerValue);
            }
          }

          // create response
          // modbusResponseWMR[0] = ID; --> already done in setup() function
          // modbusResponseWMR[1] = FC_WRITE_MULTIPLE_REGISTERS; --> already done in setup() function
          modbusResponseWMR[2] = frameBuffer[2];
          modbusResponseWMR[3] = frameBuffer[3];
          modbusResponseWMR[4] = frameBuffer[4];
          modbusResponseWMR[5] = frameBuffer[5];
          int responseCRC = ModRTU_CRC(modbusResponseWMR, modbusResponseWMRLength - 2);
          modbusResponseWMR[6] = highByte(responseCRC);
          modbusResponseWMR[7] = lowByte(responseCRC);

          for (int i = 0; i < modbusResponseWMRLength; i++) {
            HWSERIAL.write(modbusResponseWMR[i]);
          }

          Serial.println("Write multiple registers");
          Serial.print("Start address: ");
          Serial.println(startAddress);
          Serial.print("Number of registers: ");
          Serial.println(quantityRegisters);

          int index = 7;
          for (int i = 0; i < quantityRegisters; i++) {
            int value = (int) (frameBuffer[index] << 8) + frameBuffer[index + 1];
            Serial.print("Value ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(value);
            index += 2;
          }

          Serial.print("Reply: ");
          for (int i = 0; i < modbusResponseWMRLength; i++) {
            printHex(modbusResponseWMR[i], 2);
            Serial.print(" ");
          }
          Serial.println();
        }
        break;

      case FC_REPORT_SLAVEID:
        {
          Serial.println("Report slave ID");

          byte modbusReponseRSID[7] = {ID, 0x11, 0x00, 0x01, 0xFF, 0x00, 0x00};

          int responseCRC_RSID = ModRTU_CRC(modbusReponseRSID, 5);
          modbusReponseRSID[5] = highByte(responseCRC_RSID);
          modbusReponseRSID[6] = lowByte(responseCRC_RSID);

          for (int i = 0; i < 7; i++) {
            HWSERIAL.write(modbusReponseRSID[i]);
          }

          for (int i = 0; i < 7; i++) {
            printHex(modbusReponseRSID[i], 2);
            Serial.print(" ");
          }
          Serial.println();
        }
        break;

      case FC_READ_HOLDING_REGISTERS:
        {
          int startAddressRHR = frameBuffer[2] * 256 + frameBuffer[3]; // get highbyte and lowbyte of starting address
          int quantityRegistersRHR = frameBuffer[4] * 256 + frameBuffer[5];   // get quantity of addresses to read

          int byteCountRHR = 2 * quantityRegistersRHR;

          int valToSend = 0;
  
          if (startAddressRHR == 2 && quantityRegistersRHR == 1) {
            // for testing purposes only
            valToSend = toolConfirm; // confirmed tool
          }

          // create response
          // modbusResponseRHR[0] = ID; --> already done in setup() function
          // modbusResponseRHR[1] = FC_READ_HOLDING_REGISTERS; --> already done in setup() function
          modbusResponseRHR[2] = byteCountRHR;
          modbusResponseRHR[3] = highByte(valToSend);
          modbusResponseRHR[4] = lowByte(valToSend);

          // compute CRC
          int responseCRC_RHR = ModRTU_CRC(modbusResponseRHR, 5);
          modbusResponseRHR[5] = highByte(responseCRC_RHR);
          modbusResponseRHR[6] = lowByte(responseCRC_RHR);

          for (int i = 0; i < 7; i++) {
            HWSERIAL.write(modbusResponseRHR[i]);
          }

          Serial.println("Read holding registers");
          Serial.print("Start address: ");
          Serial.println(startAddressRHR);
          Serial.print("Number of registers: ");
          Serial.println(quantityRegistersRHR);

          Serial.print("Reply: ");
          for (int i = 0; i < 7; i++) {
            printHex(modbusResponseRHR[i], 2);
            Serial.print(" ");
          }
          Serial.println(" ");
        }
        break;

      default:
        Serial.println("Error: unknown function code");
        break;
    }

    // message processed at this point. Set framelength to zero to indicate empty framebuffer.
    frameLength = 0;
  }
}


void modbusReceive() {

  switch (modbusReceiveState) {
    case MB_STATE_INIT:
      // wait for t35 to pass, prevent reception of incomplete messages after slave startup
      // if character is received during this waiting period, restart timer and wait longer.

      if ( micros() - modbusT35TimerStart >= t35) {
        // no characters received in t35, goto idle state
        modbusReceiveState = MB_STATE_IDLE;
      }
      if (HWSERIAL.available() > 0) {
        // characters received, reset t35 timer
        modbusT35TimerStart = micros();
      }
      break;

    case MB_STATE_IDLE:
      // ready to receive or to emit
      frameOK = false;
      frameChecked = false;

      if (HWSERIAL.available() > 0) {
        // start t15 and t35 and goto reception
        modbusT15TimerStart = micros();
        modbusT35TimerStart = modbusT15TimerStart;
        frameLength = 0; // start new framelength counter for new message
        modbusReceiveState = MB_STATE_RECEPTION;
      }
      break;

    case MB_STATE_RECEPTION:

      // enter state when first character is received
      if (frameLength == 0) {
        frameBuffer[frameLength] = HWSERIAL.read();
        frameLength++;
      }

      if (HWSERIAL.available() > 0) {
        // receive new character and reset timers
        modbusT15TimerStart = micros();
        modbusT35TimerStart = modbusT15TimerStart;
        frameBuffer[frameLength] = HWSERIAL.read();
        frameLength++;
      }

      // check t15 timer
      if ( micros() - modbusT15TimerStart >= t15) {
        // no characters received in t15 goto Control and Waiting state
        modbusReceiveState = MB_STATE_CONTROL_WAITING;
      }
      break;

    case MB_STATE_CONTROL_WAITING:
      // check integrity of message and wait for t35 to ellapse. If character is received in this state, flag frame NOK

      if (frameChecked == false) {
        // validate checksum (checksum in last two bytes of frame)
        int CRC = ModRTU_CRC(frameBuffer, frameLength - 2); // calculated CRC of received frame
        int messageCRC = frameBuffer[frameLength - 2] * 256 +  frameBuffer[frameLength - 1]; // CRC of received frame

        if (messageCRC - CRC == 0) {
          // checksum valid
          frameOK = true;
          frameChecked = true;
        }
        else {
          frameOK = false;
          frameChecked = true;
        }
      }

      if (HWSERIAL.available() > 0) {
        frameOK = false;
      }

      if ( micros() - modbusT35TimerStart >= t35 && frameOK == true) {
        // t35 timer ellapsed and message approved, goto IDLE
        modbusReceiveState = MB_STATE_IDLE;
      }

      if (frameChecked == true && frameOK == false) {
        // invalid frame, raise error
        Serial.println("Invalid frame received");
        frameLength = 0;
        modbusReceiveState = MB_STATE_IDLE;
      }

      break;
  }

}

byte ProcessWriteSingleRegister(int WSR_address, int WSR_value) {
  // process received data request write single coil
  // return 0x00 if request is handled sucessfully
  // return 0x02 for illegal address
  // return 0x03 for illegal data value
  // return 0x04 for server device failure

  if (WSR_address == 2) {
    // toolrequest
    boolean requestValid = validateToolRequest(WSR_value);
    if (requestValid == true) {
      return 0x00;
    }
    else {
      return 0x03; // officially not allowed as value out of range not in the scope of modbus protocol
    }
  }
  else {
    // illegal address
    return 0x02;
  }

}

byte ProcessWriteSingleCoil(int WSC_address, int WSC_value) {
  // process received data request write single coil
  // return 0x00 if request is handled sucessfully
  // return 0x02 for illegal address
  // return 0x03 for illegal data value
  // return 0x04 for server device failure

  if (WSC_address == 0) {

    if (WSC_value == 0 || WSC_value == 1) {
      // set bit
      return 0x00;
    }
    else {
      // value is not 1 or 0, raise illegal data value error
      return 0x03;
    }

  }
  else if (WSC_address == 23) {
    // set bit

    if (WSC_value == 1) {
      startHoming = true;
      return 0x00;
    }
    else if (WSC_value == 0) {
      startHoming = false;
      return 0x00;
    }
    else {
      // value is not 1 or 0, raise illegal data value error
      return 0x03;
    }
  }
  else {
    // address unknown
    return 0x02;
  }
}

byte ProcessReadDiscreteInputs(int RDI_starting_address, int RDI_num_inputs) {
  // process request to read discrete inputs
  // return 0x00 if request is handled sucessfully
  // return 0x02 for illegal address
  // return 0x03 for illegal data value
  // return 0x04 for server device failure

  if ( (RDI_starting_address + RDI_num_inputs) > 31) {
    // request out of range, return illegal address exception code
    return 0x02;
  }
  else {
    return 0x00;
  }
}


void updateRDI_states() {
  // updates the array containing the discrete states that can be requested through modbus

  RDI_states[23] = !homingRequired; // true indicates homing is completed
}

uint16_t ModRTU_CRC(char * buf, int len)
// Compute the MODBUS RTU CRC
// https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
{
  uint16_t crc = 0xFFFF;
  uint16_t crcSwapped = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }

  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  crcSwapped = lowByte(crc) * 256 + highByte(crc);


  return crcSwapped;
}

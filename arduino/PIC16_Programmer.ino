// ATMega328p docs: http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-42735-8-bit-AVR-Microcontroller-ATmega328-328P_Datasheet.pdf
// PIC16F57 programming docs: http://ww1.microchip.com/downloads/en/DeviceDoc/41208C.pdf

// Design limits: 9600 baud transfer rate over pic serial protocol
// Meaning: clock speed of 9.6KHz.
// This gives us 1667 cycles (or 104us) between each clock edge
// to use for computation and whatever else.
//
// Special NOTE: PIC16F57 is a 12-bit core. The ICSP protocol dictates that we write a total of
//   16 bits per write command (a start bit, a stop bit, and 14 data bits). It should be noted
//   that the 2 most-significant bits written are ignored by the chip (i.e. the last 2 bits).
//
//   Similarly, the read command has a similar format. From our perspective, the 2 most significant
//   bits returned are ignored values.

#define SERIAL_CLOCK_PORT 2
#define SERIAL_DATA_PORT 4

unsigned short input_data = 0;
// TODO: Need to make some adjustments to allow for full memory programming. Perhaps a stream-based approach is best here
// Max size to fit on PIC16F57:
// 0x000 : 0x7ff = 2048 words for address space
// 0x800 : 0x803 = 4 words for user ids
// 0xfff         = 1 word for configuration
// ------------------------------------
//               = 2053 total words
//               = 2053 * 12 bits / word = 24,636 bits
//               = 24,636 / 8 bits / byte = 3079 bytes minimally.
//               = 2053 * 2 bytes (i.e. 16 bits/byte) = 4106 16-bit short types (with wasted high bytes) for simplest approach.
// Arduino uno has 2048 bytes of dynamic memory. As a result, storing full programs in memory is _not_ an option.
// Instead, we should have a streaming serial buffer (that communicates with arduino) when arduino needs more data.
unsigned short program_data[256] = { 0 };
unsigned validWords = 0;
unsigned writtenWords = 0;
// Max line length:
//  2 + 4 + 2 + 255*2 + 2 = 520
unsigned char program_line[520] = { 0 };
unsigned short lineBytes = 0;
short baseAddress = 0;

bool thold2 = false;
bool loaded = false;
bool bulkErased = false;
bool eraseDelay = false;
bool tprog = false;
bool tdis = false;

bool isDumping = false;
bool isProgramming = false;
bool start = false;
bool initialInc = false;
bool failed = false;
bool done = false;
// Upon entering programming mode, currentAddress == 0xfff based on docs.
short currentAddress = 0xfff;

unsigned long debounce = 0;
int currentStep = 0;

enum MODE {
  WRITE,
  BEGIN_PROG,
  END_PROG,
  INC_ADDR,
  READ,
  RESET_PC,
  DUMP
};

// TODO: Write configuration word before increment.
MODE mode = WRITE;

inline void mclr(unsigned sig) {
  if (sig == HIGH) {
    analogWrite(13, 0);
  } else {
    digitalWrite(13, HIGH);
  }
}

inline bool is_clock_low() {
  return !(PORTD & _BV(SERIAL_CLOCK_PORT));
}

// True when bit flipped to high
inline bool clock_tick() {
  if (!is_clock_low()) {
    PORTD &= ~_BV(SERIAL_CLOCK_PORT);
    return false;
  }
  PORTD |= _BV(SERIAL_CLOCK_PORT);
  return true;
}

inline void clock_low() {
  PORTD &= _BV(SERIAL_CLOCK_PORT);
}

inline void data_low() {
  PORTD &= ~_BV(SERIAL_DATA_PORT);
}

inline void data_high() {
  PORTD |= _BV(SERIAL_DATA_PORT);
}

inline void waitClockPeriod() {
  delayMicroseconds(104);
}

void setup() {
  // Enable pins 2 & 4 as output. Pin 2 is clock, pin 4 is data.
  DDRD = 0x14;
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(9600);
  // Zero out program_data
  for (unsigned i = 0 ; i < sizeof(program_data) / sizeof(short) ; ++i) {
    program_data[i] = 0;
  }
}

void loop() {
//  mclr(HIGH);
//  return;
  if (failed | done) {
    start = false;
  }
  if (!start && is_clock_low()) {
    checkAndToggleStart();
    PORTD &= ~(_BV(SERIAL_DATA_PORT) | _BV(SERIAL_CLOCK_PORT));
  } else if (clock_tick()) {
    // TODO: The control flow should be done here rather than in the global. The actions should
    //       return a bool as to whether or not their actions have completed.
    if (!initialInc) {
      incrementAddress();
//      Serial.write(0x06);
//      writeShort(currentAddress);
    } else if (!bulkErased) {
      bulkErase(); // Disable code protection
//      exitProgrammingMode();
//      enterProgrammingMode(); // Re-enter programming mode.
    } else {
      switch (mode) {
        case WRITE:
          doWrite();
          break;
        case BEGIN_PROG:
          beginProgramming();
          break;
        case END_PROG:
          endProgramming();
          break;
        case INC_ADDR:
//        mode = WRITE;
          incrementAddress();
          break;
        case READ:
          doRead();
          break;
        case RESET_PC:
          resetPC();
          break;
        case DUMP:
          dumpData();
          break;
      }
    }
  } else {
    if (tprog) {
      tprog = false;
      tdis = false;
      thold2 = false;
      // Ensure we're holding clock low if necessary
      delay(2); // Min delay is 2ms
    } else if (tdis) {
      tdis = false;
      thold2 = false;
      tprog = false;
      // Ensure we're holding clock low if necessary
      delayMicroseconds(100); // minimum 100us delay required
    } else if (thold2) {
      // Hold the clock low for 2 cycles
      thold2 = false;
      // Ensure we're holding clock low if necessary
      waitClockPeriod();
    } else if (eraseDelay) {
      eraseDelay = false;
      bulkErased = true;
      // Ensure we're holding clock low if necessary
      clock_low();
      delay(10); // at least 10 ms delay for bulk erase
    }
  }
  // Kill the remaining cycles to get to roughly 9.6KHz frequency
  waitClockPeriod();
}

inline void parseLine() {
  // Hex file is in intel hex format:
  // https://en.wikipedia.org/wiki/Intel_HEX
  unsigned char numBytes = (program_line[0] << 4) | program_line[1];
  unsigned short address = ((program_line[2] << 12) | (program_line[3] << 8) | (program_line[4] << 4) | program_line[5]);
  unsigned char recordType = (program_line[6] << 4) | program_line[7];
  unsigned char checksum = (program_line[lineBytes - 2] << 4) | program_line[lineBytes - 1];
  unsigned calculatedChecksum = numBytes + (address & 0xff) + ((address & 0xff00) >> 8) + recordType;
  int idx = 0;
//  for (unsigned int i = 0, iter = 0 ; i < 2 * numBytes ; i += 2, iter = ((iter + 1) % 3)) {
//    //    int idx = (i / 2);
//    // Data bytes start at byte offset 8
//    // Data word for PIC16F57 is 12-bytes.
//    unsigned char topNybble = program_line[8 + i];
//    unsigned char botNybble = program_line[8 + i + 1];
//    // This is a bit painful... But we're processing here to make tx easier. This is a product of a 12-bit word:
//    switch (iter) {
//      case 0:
//        program_data[idx] = (topNybble << 8) | (botNybble << 4);
//        break;
//      case 1:
//        program_data[idx] |= topNybble;
//        idx++;
//        program_data[idx] = (botNybble << 8);
//        break;
//      case 2:
//        program_data[idx] |= (topNybble << 4) | botNybble;
//        idx++;
//        break;
//    }
//    // Take the low 12 bits
//    program_data[idx] &= 0xfff;
//    calculatedChecksum += ((topNybble << 4) | botNybble) & 0xff;
//    // Remember the number of valid bytes (based on the index)
//    validWords = idx + 1;
//  }
  for (unsigned int i = 0, iter = 0 ; i < 2 * numBytes ; i += 4) {
    //    int idx = (i / 2);
    int idx = i / 4;
    // Data bytes start at byte offset 8
    // Data word for PIC16F57 is 12-bytes.
    // NOTE: Words are stored with LOWER BYTE FIRST.
    unsigned char topNybble = program_line[8 + i];
    unsigned char botNybble = program_line[8 + i + 1];
    unsigned char upperTNybble = program_line[8 + i + 2];
    unsigned char upperBNybble = program_line[8 + i + 3];
    unsigned char lowerByte = (topNybble << 4) | botNybble;
    unsigned char upperByte = (upperTNybble << 4) | upperBNybble;
    // Take the low 12 bits
    program_data[idx] = ((upperByte << 8) | lowerByte) & 0xfff;
    calculatedChecksum += lowerByte + upperByte;
    // Remember the number of valid bytes (based on the index)
    validWords = idx + 1;
  }
  calculatedChecksum = ((~calculatedChecksum) + 1) & 0xff;
  // TODO: Hack.
  if ((address & 0xfff) == 0xffe) {
    // Configuration word?
    address = 0xfff;
  }
  baseAddress = address;
  if (calculatedChecksum != checksum) {
    Serial.write(0x04);
    Serial.write(checksum);
    Serial.write(calculatedChecksum);
    failed = true;
  }
  if (recordType == 0x01) {
    done = true;
    Serial.write(0x02);
  } else if (recordType == 0x04) {
    // 12 bit core... So these records can simply be ignored since the data represents the upper
    // 16 bits of an address line (we only have 12...)
  } else {
    writtenWords = 0;
    start = true;
    mode = WRITE;
    if (!isProgramming) {
      // TReset is 10ms. Make sure we wait.
      delay(10);
      enterProgrammingMode();
    }
    if (address == 0xfff) {
      // Write config word. just re-enter programming mode to reset counter
      exitProgrammingMode();
      enterProgrammingMode();
      initialInc = true;
    }
  }
}

inline void checkAndToggleStart() {
  static bool hasResetAfterDone = false;
  
  // NOTE: To start/stop send any signal over USB serial.
  if (failed) {
    if (Serial.available() > 0) {
      Serial.write(0x03); // Failure byte.
      while (Serial.available() > 0) {
        Serial.read(); // Throw away buffer bytes
      }
    }
    mclr(LOW);
    return;
  }
  if (done) {
    if (!hasResetAfterDone) {
      mclr(LOW);
      delay(20);
      mclr(HIGH);
      hasResetAfterDone = true;
    }
    return;
  }
  if (Serial.available() > 0) {
    char inByte;
    while (Serial.available() > 0) {
      // It's possible the controlling process has sent many ready responses.
      // Just in case, pop all of the consecutive 'R's off.
      inByte = Serial.read();
      if (inByte == 'R' && Serial.peek() == 'R') {
        continue;
      } else {
        break;
      }
    }
    // NOTE: Non-hex ASCII characters are ok for control signals (except :) since we're sending
    // the lines as ASCII characters and converting to their numeric representation.
    if (inByte == 'R') {
      // Controller said it's ready to send, let controller know we're also ready to receive.
      Serial.write(0x01);
      isProgramming = false;
//      exitProgrammingMode();
    } else if (inByte == 'X') {
      mode = DUMP;
      enterProgrammingMode();
      initialInc = bulkErased = start = true;
      return;
    } else if (inByte == ':') {
      lineBytes = 0;
    } else if (inByte == '\n' || inByte == '\r') {
      parseLine();
    } else if (inByte == ' ' || inByte == '\t') {
      // Skip.
    } else {
      char newByte[2] = { 0 };
      newByte[0] = inByte;
      program_line[lineBytes] = strtol(newByte, NULL, 16);
      lineBytes++;
    }
    if (inByte != 'R' && inByte != '\n' && inByte != '\r') {
      // Echo the byte back for verification
      Serial.write(inByte);
    }
    if (!start) {
      if (!isProgramming) {
        mclr(LOW);
      }
    } else {
      if (!done && !failed && isProgramming) {
        mclr(HIGH);
      } else if (done) {
        Serial.write(0x02); // Succes
        start = false;
      } else {
        start = false;
      }
    }
  } else if (!start && !isProgramming) {
    mclr(LOW);
  }
}

inline void enterProgrammingMode() {
  // Drive clock and data low to re-enter programming mode
  clock_low();
  data_low();
  // Drive MCLR low for 5 microseconds (transistor will disable)
  mclr(LOW);
  delayMicroseconds(5);
  mclr(HIGH);
  // Hold clock and data low for 5 microseconds
  delayMicroseconds(5);
  initialInc = false;
  currentAddress = 0xfff;
  isProgramming = true;
}

inline void bulkErase() {
  DDRD |= 0x10;
  if (currentStep == 0) {
    data_high();
  } else if (currentStep < 3) {
    data_low();
  } else if (currentStep == 3) {
    data_high();
  } else if (currentStep < 5) {
    data_low();
  } else {
    eraseDelay = true;
    currentStep = -1;
  }
  currentStep++;
}

inline void dumpData() {
  static bool hasDumped = false;
  static bool hasRead = false;
  static short reportedAddress = 0;
  // Everything has been dumped that's not code protected
  if (hasDumped && currentAddress == 0x404) {
    hasDumped = false;
    mode = WRITE;
    start = false;
    isDumping = false;
    Serial.write(0x08);
    return;
  }
  
  isDumping = true;

  // NOTE: We need to clean this code up... a lot...
  // But currentStep relies on the artifact that incAddr will update.. doh.
  if (!hasRead) {
    doRead();
    // Ick. Artifact :( write mode means it finished...
    if (mode == WRITE) {
      mode = DUMP;
      hasRead = true;
      hasDumped = true;
      reportedAddress = currentAddress;
    }
  } else {
    incrementAddress();
    if (mode == WRITE) {
      mode = DUMP;
      hasRead = false;
      clock_low(); // Hold clock low during transmission
      // Send back to controller
      Serial.write(0x07);
      sendShort(reportedAddress);
      sendShort(input_data);
    }
  }
}

// Write data to an address
inline void doWrite() {
  // Make sure pin 4 is output
  int currentIndex = writtenWords;
  DDRD |= 0x10;
  if (currentStep == 0) {
    // Make sure our address is in range to be written, otherwise increment towards it.
    // The final condition is to make sure we're writing the correct word.
    if (currentAddress != ((baseAddress + writtenWords) % 0x1000)) {
      if (writtenWords >= validWords) {
        // We're done writing bits sent from this line
        start = false;
        return;
      }
      incrementAddress();
      mode = INC_ADDR;
      return;
    }
    data_low();
  } else if (currentStep == 1) {
    data_high();
  } else if (currentStep < 4) {
    data_low();
  } else if (currentStep < 5) {
    data_low(); // don't care value
  } else if (currentStep == 5) {
    data_low(); // don't care value
    thold2 = true; // Hold clock signal low before start bit
  } else if (currentStep == 6) {
    // Start bit
    data_low();
  } else if (currentStep > 6 && currentStep < 21) {
    // Data bits.
    int checkBit = currentStep - 7;
    if (program_data[currentIndex] & (0x01 << checkBit)) {
      data_high();
    } else {
      data_low();
    }
  } else {
    // Stop bit
    data_low();
    currentStep = -1;
    mode = BEGIN_PROG;
  }
  currentStep++;
}

// Read data
inline void doRead() {
  // Make sure pin 4 is in read mode
  if (currentStep < 2) {
    DDRD |= 0x10; // Writeable data
    input_data = 0; // Reset input data
    data_low();
  } else if (currentStep == 2) {
    DDRD |= 0x10;
    data_high();
  } else if (currentStep < 5) {
    DDRD |= 0x10;
    data_low();
  } else if (currentStep == 5) {
    DDRD |= 0x10;
    data_low(); // don't care value
    DDRD &= ~0x10;
    thold2 = true; // Hold clock signal
  } else if (currentStep > 5 && currentStep < 21) {
    DDRD &= ~0x10; // Read pin 4
    // Ignore stop/start bits
    if (currentStep != 6 && currentStep != 20) {
      int value = PIND & 0x10;
      int checkBit = currentStep - 7;
      if (value) {
        input_data |= 0x01 << checkBit;
      }
    }
  } else {
    currentStep = -1;
    mode = WRITE;
    // Verify result - skip configuration word for now...
    if (!isDumping && input_data != program_data[writtenWords]) {
      writtenWords = 0;
      failed = true;
      Serial.write(0x05);
      writeShort(currentAddress);
      writeShort(baseAddress);
      Serial.write((currentAddress - baseAddress) & 0xff);
      writeShort(input_data);
      writeShort(program_data[currentAddress - baseAddress]);
    }
    // Keep track of the fact that we have successfully written a word
    writtenWords++;
  }
  currentStep++;
}

inline void writeShort(short value) {
  Serial.write((value & 0xf000) >> 12);
  Serial.write((value & 0x0f00) >> 8);
  Serial.write((value & 0x00f0) >> 4);
  Serial.write((value & 0x000f) >> 0);
}

// Enter programming mode
inline void beginProgramming() {
  DDRD |= 0x10;
  if (currentStep < 3) {
    data_low();
  } else if (currentStep == 3) {
    data_high();
  } else if (currentStep < 5) {
    data_low(); // Don't care
  } else if (currentStep == 5) {
    data_low(); // don't care
    tprog = true; // Hold clock cycle-- this time for tprog.
    currentStep = -1;
    mode = END_PROG;
  }
  currentStep++;
}

// End programming mode
inline void endProgramming() {
  DDRD |= 0x10;
  if (currentStep == 0) {
    data_low();
  } else if (currentStep < 5) {
    data_high();
  } else if (currentStep == 5) {
    data_low(); // don't care
    tdis = true; // Hold clock cycle-- this time for tprog.
    currentStep = -1;
    mode = READ;
  }
  currentStep++;
}

// Reset PC to base address
inline void resetPC() {
  if (currentAddress == baseAddress) {
    mode = WRITE;
    doWrite();
  } else {
    incrementAddress();
    mode = RESET_PC;
  }
}

inline void exitProgrammingMode() {
  mclr(LOW);
  delay(10);
}

inline void sendShort(short value) {
  writeShort(value);
}

// Increment PC address
inline void incrementAddress() {
  DDRD |= 0x10;
  if (currentStep == 0) {
    data_low();
  } else if (currentStep < 3) {
    data_high();
  } else if (currentStep < 5) {
    data_low();
  } else if (currentStep == 5) {
    data_low(); // don't care
    currentAddress++;
    // Programmer for PIC57 which is a 12-bit core. Rollover at max address.
    if (currentAddress > 0xfff) {
      currentAddress = 0;
    }

    // TODO: Remove this limit.
    // TODO: Should only program sections we know about.
    //    if (initialInc && currentAddress >= 0x400) {
    //      start = false;
    //      done = true;
    //      Serial.println("Programming completed successfully!");
    //    }

    thold2 = true; // Hold clock cycle-- this time for tprog.
    initialInc = true;
    currentStep = -1;
    // After inc, go to write mode
    mode = WRITE;
  }
  currentStep++;
}


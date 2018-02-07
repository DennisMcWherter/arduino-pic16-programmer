/**
 * ReadCommand.cpp
 * 
 * Read command implementation.
 * 
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#include "ReadCommand.h"
#include "utility.h"

CommandResult ReadCommand::executeStep()
{
    uint8_t currentStep = getCurrentStep();
    bool tdly2 = false;
    if (currentStep < 2) {
        // Pin is in write mode
        data_write_mode();
        data_low();
    } else if (currentStep == 2) {
        data_write_mode();
        data_high();
    } else if (currentStep < 5) {
        data_write_mode();
        data_low();
    } else if (currentStep == 5) {
        data_write_mode();
        data_low(); // don't care value
        data_read_mode();
        tdly2 = true; // Hold clock signal
    } else if (currentStep > 5 && currentStep < 21) {
        // Read mode
        data_read_mode();
        // Ignore stop/start bits
        if (currentStep != 6 && currentStep != 20) {
            bool value = read_data_bit();
            int checkBit = currentStep - 7;
            if (value) {
                input_data |= 0x01 << checkBit;
            }
        }
    } else {
#if 0
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
#endif
        return COMPLETED_NODLY;
    }
    step();
    if (tdly2) {
        return IN_PROGRESS_TDLY2;
    }
    return IN_PROGRESS_NODLY;
}


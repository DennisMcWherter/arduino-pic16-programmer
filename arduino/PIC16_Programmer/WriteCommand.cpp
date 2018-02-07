/**
 * WriteCommand.cpp
 * 
 * Write command implementation.
 * 
 * (C) Copyright 2018 Dennis J. McWherter, Jr.
 */

#include "WriteCommand.h"
#include "utility.h"

CommandResult WriteCommand::executeStep()
{
    // Ensure pin is in write mode
    data_write_mode();
    bool tdly2 = false;
    const uint8_t currentStep = getCurrentStep();
    if (currentStep == 0) {
        data_low();
    } else if (currentStep == 1) {
        data_high();
    } else if (currentStep < 4) {
        data_low();
    } else if (currentStep < 5) {
        data_low(); // don't care value
    } else if (currentStep == 5) {
        data_low(); // don't care value
        tdly2 = true; // Hold clock signal low before start bit
    } else if (currentStep == 6) {
        // Start bit
        data_low();
    } else if (currentStep > 6 && currentStep < 21) {
        // Data bits.
        int checkBit = currentStep - 7;
        if (data & (0x01 << checkBit)) {
            data_high();
        } else {
            data_low();
        }
    } else {
        // Stop bit
        data_low();
        return COMPLETED_NODLY;
    }
    step();
    if (tdly2) {
        return IN_PROGRESS_TDLY2;
    }
    return IN_PROGRESS_NODLY;
}

/*
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#include "IncrementAddressCommand.h"
#include "utility.h"

CommandResult IncrementAddressCommand::executeStep()
{
    data_write_mode();
    uint8_t currentStep = getCurrentStep();
    if (currentStep == 0) {
        data_low();
    } else if (currentStep < 3) {
        data_high();
    } else if (currentStep < 5) {
        data_low();
    } else if (currentStep == 5) {
        data_low(); // don't care
        return COMPLETED_TDLY2;
    }
    step();
    return IN_PROGRESS_NODLY;
}

/*
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#include "BeginProgCommand.h"
#include "utility.h"

CommandResult BeginProgCommand::executeStep()
{
    data_write_mode();
    uint8_t currentStep = getCurrentStep();
    if (currentStep < 3) {
        data_low();
    } else if (currentStep == 3) {
        data_high();
    } else if (currentStep < 5) {
        data_low(); // Don't care
    } else if (currentStep == 5) {
        data_low(); // don't care
        // Hold clock cycle for tprog
        return CommandResult(true, false, TPROG_MICROS);
    }
    return IN_PROGRESS_NODLY;
}

/*
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#include "EndProgCommand.h"
#include "utility.h"

CommandResult EndProgCommand::executeStep()
{
    data_write_mode();
    uint8_t currentStep = 0;
    if (currentStep == 0) {
        data_low();
    } else if (currentStep < 5) {
        data_high();
    } else if (currentStep == 5) {
        data_low(); // don't care
        // Hold clock cycle low for Tprog.
        return CommandResult(true, false, TDIS_MICROS);
    }
    step();
    return IN_PROGRESS_NODLY;
}

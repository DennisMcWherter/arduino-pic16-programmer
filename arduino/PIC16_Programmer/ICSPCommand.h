/**
 * ICSPCommand.h
 * 
 * Interface for executing an ICSP command.
 * 
 * (C) Copyright 2018 Dennis J. McWherter, Jr.
 */

#ifndef ICSPCOMMAND_H__
#define ICSPCOMMAND_H__

#include <stdint.h>
#include "constants.h"

/**
 * Structure containing information related to the result of a command
 */
struct CommandResult {
    CommandResult(bool completed, bool failed, uint32_t delayMicros)
      : completed(completed), failed(failed), delayMicros(delayMicros)
      {
      }
      bool completed;
      bool failed;
      uint32_t delayMicros;
};

/**
 * Common results
 */
const CommandResult IN_PROGRESS_NODLY(false, false, 0);
const CommandResult IN_PROGRESS_TDLY2(false, false, TDLY2_MICROS);
const CommandResult COMPLETED_NODLY(true, false, 0);
const CommandResult COMPLETED_TDLY2(true, false, TDLY2_MICROS);
const CommandResult COMPLETED_FAILED_NODLY(true, true, 0);

class ICSPCommand {
public:    
    ICSPCommand()
      : currentStep(0)
      {
      }

    virtual ~ICSPCommand() {}
    
    /**
     * Execute a step in the ICSP command
     * 
     * @return Result of the command.
     */
    virtual CommandResult executeStep() = 0;
    
protected:
    /**
     * Get the current step
     * 
     * @return Current step.
     */
    inline uint8_t getCurrentStep() const {
        return currentStep;
    }
    
    /**
     * Take a single step through command
     */
    inline void step() {
        currentStep++;
    }
    
    /**
     * Reset the step counter
     */
    inline void resetSteps() {
        currentStep = 0;
    }

private:
    uint8_t currentStep;
};

#endif /* ICSPCOMMAND_H__ */

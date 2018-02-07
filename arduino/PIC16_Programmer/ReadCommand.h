/**
 * ReadCommand.h
 * 
 * Header for the ICSP read command.
 * 
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#ifndef READ_COMMAND_H__
#define READ_COMMAND_H__

#include "ICSPCommand.h"

class ReadCommand : public ICSPCommand {
public:
    ReadCommand(const uint16_t expectedWord)
      : hasExpectedWord(true), expectedWord(expectedWord), input_data(0)
      {
      }

    ReadCommand()
      : hasExpectedWord(false), expectedWord(0), input_data(0)
      {
      }
    
    virtual ~ReadCommand() {}
    
    CommandResult executeStep() override;

private:
    bool hasExpectedWord;
    const uint16_t expectedWord;
    uint16_t input_data;
};

#endif /* READ_COMMAND_H__ */

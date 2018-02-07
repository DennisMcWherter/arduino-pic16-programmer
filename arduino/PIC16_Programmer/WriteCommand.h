/**
 * WriteCommand.h
 * 
 * ICSP write command.
 * 
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#ifndef __WRITE_COMMAND_H__
#define __WRITE_COMMAND_H__

#include "ICSPCommand.h"

class WriteCommand : public ICSPCommand {
public:
    WriteCommand(const uint16_t data)
      : data(data)
      {
      }
      
    virtual ~WriteCommand() {}

    CommandResult executeStep() override;

private:
    const uint16_t data;
};

#endif /* __WRITE_COMMAND_H__ */

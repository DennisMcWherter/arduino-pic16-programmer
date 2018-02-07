/*
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#ifndef BEGINPROGCOMMAND_H__
#define BEGINPROGCOMMAND_H__

#include "ICSPCommand.h"

class BeginProgCommand : public ICSPCommand
{
public:
    virtual CommandResult executeStep() override;

};

#endif // BEGINPROGCOMMAND_H__

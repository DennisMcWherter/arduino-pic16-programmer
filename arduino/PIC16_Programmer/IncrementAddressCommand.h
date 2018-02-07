/*
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#ifndef INCREMENTADDRESSCOMMAND_H__
#define INCREMENTADDRESSCOMMAND_H__

#include "ICSPCommand.h"

class IncrementAddressCommand : public ICSPCommand
{
public:
    virtual CommandResult executeStep() override;

};

#endif // INCREMENTADDRESSCOMMAND_H__

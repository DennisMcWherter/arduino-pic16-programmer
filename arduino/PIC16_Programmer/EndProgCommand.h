/*
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#ifndef ENDPROGRAMMINGCOMMAND_H__
#define ENDPROGRAMMINGCOMMAND_H__

#include "ICSPCommand.h"

class EndProgCommand : public ICSPCommand
{
public:
    virtual CommandResult executeStep() override;

};

#endif // ENDPROGRAMMINGCOMMAND_H__

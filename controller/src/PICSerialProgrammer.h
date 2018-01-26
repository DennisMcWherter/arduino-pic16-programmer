/**
 * PICSerialProgrammer.h
 *
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */
#ifndef __PICSerialProgrammer_H__
#define __PICSerialProgrammer_H__

#include <algorithm>
#include <cstring>
#include <deque>
#include <experimental/filesystem>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#define HEX_EOF ":00000001FF"
#define MAX_RETRIES 100

namespace fs = std::experimental::filesystem;
using namespace std;

const string CONFIG_WORD_START(":021FFE00");

class PICSerialProgrammer {
public:
    /**
     * Constructor
     */
    PICSerialProgrammer(const char* devicePath, const char* hexPath);
    
    /**
     * Destructor
     */
    virtual ~PICSerialProgrammer();
    
    /**
     * Dump chip memory
     */
    void dumpChip();
    
    /**
     * Flash program onto device
     */
    void flashDevice();

private:
    int serialCommFd;
    deque<string> hexLines;
    
    enum CONTROL_CODES {
        READY_FOR_LINE = 0x01,
        COMPLETE_SUCCESS = 0x02,
        COMPLETE_FAILURE = 0x03,
        CHECKSUM_FAILED = 0x04,
        VERIFY_FAILED = 0x05,
        DEBUG = 0x06,
        DUMP_ADDRESS = 0x07,
        DUMP_COMPLETE = 0x08
    };
    
    void openAndConfigureDevice(const char* devicePath);
    
    void loadHexLines(const char* hexPath);
    
    void sendDumpSignal();
    
    void sendReadySignal();
    
    void sendSignal(const char signal);
    
    bool waitForData();
    
    unsigned char loadByte();
    
    short loadShort();
    
    string byteToHex(const char byte);
    
    string shortToHex(const short value);
};

#endif // __PICSerialProgrammer_H__

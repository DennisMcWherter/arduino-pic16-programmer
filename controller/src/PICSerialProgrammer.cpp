/**
 * PICSerialProgrammer.cpp
 *
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

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

#include "PICSerialProgrammer.h"

#define HEX_EOF ":00000001FF"
#define MAX_RETRIES 100

namespace fs = std::experimental::filesystem;
using namespace std;

PICSerialProgrammer::PICSerialProgrammer(const char* devicePath, const char* hexPath)
{
    if (!fs::exists(devicePath)) {
        throw invalid_argument("Device path does not exist: " + string(devicePath));
    }

    if (!fs::exists(hexPath)) {
        throw invalid_argument("Hex file does not exist: " + string(hexPath));
    }
    
    loadHexLines(hexPath);
    openAndConfigureDevice(devicePath);
}

void PICSerialProgrammer::dumpChip() {
    sendDumpSignal();        
    while (1) {
        char input = loadByte();
        switch (input) {
            case DUMP_ADDRESS:
            {
                short address = loadShort();
                short data = loadShort();
                cout<< hex << address << ": " << data << endl;
            }
            break;
            case DUMP_COMPLETE:
                return;
            default:
                throw runtime_error("Unexpected status code received while dumping chip data: " + byteToHex(input));
        }
    }
}

void PICSerialProgrammer::flashDevice() {
    unsigned lineNumber = 0;
    unsigned totalLines = hexLines.size();
    
    // Send line-by-line to let the microcontroller program
    while (1) {
        sendReadySignal();
        char input = loadByte();
        switch (input) {
            case READY_FOR_LINE:
            {
                if (totalLines <= lineNumber) {
                    throw runtime_error("Did not get termination signal after completing programming.");
                }

                // Copy the line so we can add a newline character to it
                string line = hexLines[lineNumber];
                line += '\n';
                const char* txData = line.c_str();
                size_t totalBytes = line.size();
                size_t totalWritten = 0;
                while (totalWritten < totalBytes) {
                    char expected = txData[totalWritten];
                    size_t written = write(serialCommFd, &expected, 1);
                    if (written == -1) {
                        throw runtime_error("An error occurred writing to the device: " + string(strerror(errno)));
                    }
                    totalWritten += written;
                    // Deliberately skip the new line (just in case a checksum failure occurs)
                    if (totalWritten != totalBytes) {
                        char actual = loadByte();
                        if (expected != actual) {
                            throw runtime_error("Transmission failed on line: " + to_string(lineNumber + 1)
                                + " for byte number: " + to_string(totalWritten)
                                + "\n\tExpected: " + byteToHex(expected)
                                + "\n\tActual: " + byteToHex(actual) + "\nTotal bytes: " + to_string(totalBytes) + "\nTotal bytes written: " + to_string(totalWritten) + "\nFull line: " + line);
                        }
                        cout<< actual;
                        flush(cout);
                    }
                }
                cout<< endl;
                lineNumber++;
            }
                break;
            case COMPLETE_SUCCESS:
                return;
            case COMPLETE_FAILURE:
                throw runtime_error("Programming failed. Please reset arduino & try again.");
            case CHECKSUM_FAILED:
            {                    
                // Should also send the expected and actual bytes:
                const string& expected = byteToHex(loadByte());
                const string& actual = byteToHex(loadByte());
                throw runtime_error("Checksum failed for line number: " + to_string(lineNumber)
                    + "\n\tExpected: " + expected
                    + "\n\tActual: " + actual);
            }
            case VERIFY_FAILED:
            {
                short currentAddress = loadShort();
                short baseAddress = loadShort();
                char bufferIndex = loadByte();
                short actualData = loadShort() & 0xfff;
                short expectedData = loadShort() & 0xfff;
                throw runtime_error(string("Failed to verify writing data to PIC chip.")
                    + "\n\tFailed to write to address: " + shortToHex(currentAddress & 0xfff)
                    + "\n\tBase address: " + shortToHex(baseAddress)
                    + "\n\tAt buffer index: " + byteToHex(bufferIndex)
                    + "\n\tActual data: " + shortToHex(actualData)
                    + "\n\tExpected data: " + shortToHex(expectedData)
                );
            }
            case DEBUG:
            {
                short data = loadShort() & 0xfff;
                cout<< "Data: " << shortToHex(data) << endl;
            }
                break;
            default:
                throw runtime_error("Unexpected control code sent from device: " + byteToHex(input));
        }
    }
}

PICSerialProgrammer::~PICSerialProgrammer() {
    close(serialCommFd);
}

void PICSerialProgrammer::openAndConfigureDevice(const char* devicePath) {
    cout<< "Connecting to device... ";
    flush(cout);
    serialCommFd = open(devicePath, O_RDWR | O_NOCTTY);
    if (serialCommFd == -1) {
        throw invalid_argument("Could not open device: " + string(strerror(errno)));
    }
    
    // Configure IO settings
    struct termios options;
    tcgetattr(serialCommFd, &options);
    
    // Set comms rate to 9600 baud
    cfsetspeed(&options, B9600);
    options.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
    options.c_cflag |= CREAD | CS8 | CLOCAL;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(IXON | IXANY);
    options.c_iflag |= IGNBRK;
    options.c_cc[VMIN] = options.c_cc[VTIME] = 0;
    
    tcsetattr(serialCommFd, TCSANOW, &options);
    
    // Sleep 2 seconds to allow arduino to reset on serial connect
    // Rather than disable, we'll just cope with it:
    // https://playground.arduino.cc/Main/DisablingAutoResetOnSerialConnection
    sleep(2);
    
    tcflush(serialCommFd, TCIOFLUSH);
    
    cout<< "Connected!" << endl;
}

void PICSerialProgrammer::loadHexLines(const char* hexPath) {
    // Load microcontroller code into memory
    ifstream hexFile;
    hexFile.open(hexPath);
    if (!hexFile.is_open()) {
        throw invalid_argument("Could not open hex file: " + string(strerror(errno)));
    }
    string line;
    while (getline(hexFile, line)) {
        hexLines.push_back(line);
    }
    hexFile.close();
    
    // Validate the end at least looks correct:
    if (hexLines.empty()) {
        throw invalid_argument("Hex file is empty.");
    }
    if (hexLines[hexLines.size() - 1] != HEX_EOF) {
        throw invalid_argument("Hex file format is invalid.");
    }
}

void PICSerialProgrammer::sendDumpSignal() {
    sendSignal('X');
}

void PICSerialProgrammer::sendReadySignal() {
    sendSignal('R');
}

void PICSerialProgrammer::sendSignal(const char signal) {
    unsigned retries = 0;

    // Signal to microcontroller that we're ready to write!
    while (write(serialCommFd, &signal, 1) != 1 && retries < MAX_RETRIES) {
        retries++;
    }
    
    if (retries >= MAX_RETRIES) {
        throw runtime_error("Communication with device failed.");
    }
}

bool PICSerialProgrammer::waitForData() {
    fd_set checkFds;

    FD_ZERO(&checkFds);
    FD_SET(serialCommFd, &checkFds);

    // 5s timeout
    struct timeval delay;
    delay.tv_sec = 30;
    delay.tv_usec = 0;
    
    // Was having trouble getting a blocking read call, so just force a select until we have data
    return select(serialCommFd + 1, &checkFds, NULL, NULL, &delay) > 0;
}

unsigned char PICSerialProgrammer::loadByte() {
    if (!waitForData()) {
        throw runtime_error("Connection to device lost.");
    }
    unsigned char byte = 0;
    while (read(serialCommFd, &byte, 1) != 1);
    return byte;
}

short PICSerialProgrammer::loadShort() {
    char firstNybble = loadByte();
    char secondNybble = loadByte();
    char thirdNybble = loadByte();
    char fourthNybble = loadByte();
    return (firstNybble << 12) | (secondNybble << 8) | (thirdNybble << 4) | fourthNybble;
}

string PICSerialProgrammer::byteToHex(const char byte) {
    stringstream stream;
    stream << "0x" << hex << (((int) byte) & 0xff);
    return stream.str();
}

string PICSerialProgrammer::shortToHex(const short value) {
    stringstream stream;
    stream << "0x" << hex << value;
    return stream.str();
}

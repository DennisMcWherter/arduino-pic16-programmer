/**
 * main.cpp
 *
 * (C) Copyright 2018 Dennis J. McWherter, Jr. All rights reserved.
 */

#include <iostream>
#include <stdexcept>

#include "PICSerialProgrammer.h"

using namespace std;

int main(int argc, char** argv) {
    using namespace std;

    if (argc != 3) {
        cout<< "Usage: " << argv[0] << " <path_to_serial_device> <path_to_hex_file>" << endl;
        return 1;
    }
    
    try {
        PICSerialProgrammer programmer(argv[1], argv[2]);
        cout<< "Programming device..." << endl;
        programmer.flashDevice();
        //programmer.dumpChip();
        cout<< "Programming completed successfully!" << endl << endl;
    } catch (invalid_argument& e) {
        cerr<< endl << e.what() << endl << endl;
        return 1;
    } catch (runtime_error& e) {
        cerr<< endl << e.what() << endl << endl;
        return 1;
    }
    
    return 0;
}

// This file is part Floppy I/O, a Virtual Machine - Hypervisor intercommunication system.
// Copyright (C) 2011 Ioannis Charalampidis 
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// -------------------------------------------------------------------
// File:   errorbase.cpp
// Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
// License: GNU Lesser General Public License - Version 3.0
// -------------------------------------------------------------------
//
// Base class that provides error handling
//

#include "../includes/errorbase.h"

using namespace fpio;
using namespace std;

// FloppyIO Exception singleton
static ioexception   __FloppyIOExceptionSingleton;

//
// Set last error.
// This is a short-hand function to update the error variables.
//
// @param   code    The error code
// @param   message The error message
// @return          The error code
//
int  errorbase::setError(const string message, int code) {
    this->errorCode = code;

    // Chain errors
    if (this->errorStr.empty()) {
        this->errorStr = message;
    } else {
        this->errorStr = message + " (" + this->errorStr + ")";
    }

    // Should we raise an exception?
    if (this->useExceptions) 
        throw *__FloppyIOExceptionSingleton.set(code, message);

    // Otherwise return code
    // (Useful for using single-lined: return this->setError(-1, "message..');
    return code;
}

//
// Clear error state flags
//
void errorbase::clear() {
    this->errorCode = 0;
    this->errorStr = "";
}

//
// Check if everything is in ready state
// @return Returns true if there are no errors and stream hasn't failed
//
bool errorbase::ready() {
    return (this->errorCode==0);
}


//
// Check if there was an error
// @return Returns true if there was an error
//
bool errorbase::error() {
    return (this->errorCode!=0);
}




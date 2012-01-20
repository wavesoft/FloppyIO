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
// File:   errorbase.h
// Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
// License: GNU Lesser General Public License - Version 3.0
// -------------------------------------------------------------------
//
// Base class that provides error handling
//

#ifndef ERRORBASE_H
#define	ERRORBASE_H

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

namespace fpio {

    //
    // Common error codes for FPIO
    //
    const int   ERR_NONE        = 0;        // No error occured
    const int   ERR_IO          = -1;       // An I/O error occured
    const int   ERR_TIMEOUT     = -2;       // An operation timed out
    const int   ERR_CREATE      = -3;       // Unable to create the floppy file
    const int   ERR_NOTREADY    = -4;       // The I/O object is not ready
    const int   ERR_INPUT       = -5;       // An error occured while processing ipnut
    const int   ERR_ABORTED     = -6;       // Operation aborted
    const int   ERR_INVALID     = -7;       // Invalid usage

    // Error levels
    const int   ERL_MINOR       = 1;        // Minor error    : Does not raise exceptions
    const int   ERL_ERROR       = 2;        // Normal error   : Raises exception
    const int   ERL_CRITICAL    = 3;        // Critical error : Raises exception, needs to stop execution

    //
    // Base class that provides error reporting functionality
    //
    class errorbase {
    public:

        // Properties
        int             errorCode;          // The error code of the last error
        string          errorStr;           // The error message of the last occured error
        bool            useExceptions;      // Set to TRUE to enable exceptions

        // Functions
        bool            error();            // Returns TRUE if an error has occured
        virtual bool    ready();            // Returns TRUE if the class is ready for operation
        virtual void    clear();            // Clear the error state

    protected:

        // Trigger an error 
        int         setError(const string message, int code, int level=0);
        int         setError(const string message, const string details, int code, int level=0);

    };


    //
    // Floppy I/O Exceptions
    //
    class ioexception: public exception {
    public:

        int         code;
        string      message;

        // Default constructor/destructor
        ioexception() { this->code=0; this->message=""; };
        virtual ~ioexception() throw() { };

        // Get description
        virtual const char* what() const throw() {
            static ostringstream oss (ostringstream::out);
            oss << this->message << ". Error code = " << this->code;
            return oss.str().c_str();
        }

        // Change the message and return my instance
        // (Used for singleton format)
        virtual ioexception * set(int code, string message) {
            this->code = code;
            this->message = message;
            return this;
        }
        
    };
    
};

#endif  // ERRORBASE_H


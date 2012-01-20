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
// File:   FloppyIO.h
// Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
// License: GNU Lesser General Public License - Version 3.0
// -------------------------------------------------------------------
//
// Hypervisor-Virtual machine bi-directional communication
// through floppy disk.
//
// This class provides the hypervisor-side of the script.
// For the guest-side, check the perl scripts that
// were available with this code.
//


#ifndef FLOPPYIO_H
#define FLOPPYIO_H

#include <iostream>

#include "flpdisk.h"
#include "errorbase.h"

using namespace std;

namespace fpio {

    // Additional open flags
    const int   O_SYNCHRONIZED  = 64;    // Use synchronized I/O

    // Default synchronization timeout
    const int   SYNC_TIMEOUT    = 4;     // 4 Seconds

    //
    // FloppyIO Class
    //
    class floppyIO:
        public flpdisk 
    {
    public:

        // Constructor/Destructor
        floppyIO(const char * file, int flags = 0) ;
        virtual             ~floppyIO();

        // Send/Receive data from stream
        int                 send(istream * stream, unsigned short id = 0);
        int                 receive(ostream * stream, unsigned short id = 0);

        // Send/Receive data without I/O Sync
        int                 send(string buffer);
        int                 send(char * buffer, int size, int streamID = 0);
        int                 receive(string buffer);
        int                 receive(char * buffer, int size, int streamID = 0);

        // Synchronization
        int                 waitForSyncIn(unsigned short streamID, int timeout = 0);
        int                 waitForSyncOut(unsigned short streamID, int timeout = 0);

        // Variables
        int                 syncTimeout;
        bool                useSynchronization;

    private:

        ctrlbyte            inCB, outCB;
        extended_header     inHDR, outHDR;

    };

};

#endif  // FLPDISK_H

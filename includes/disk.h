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
// File:   disk.h
// Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
// License: GNU Lesser General Public License - Version 3.0
// -------------------------------------------------------------------
//
// Virtual disk I/O Header file
// 

#ifndef DISK_H
#define	DISK_H

#include "errorbase.h"

namespace fpio {

    // The size of the floppy disk (1.44Mb)
    const int   SZ_FLOPPY = 1474560;

    // Flags for the fpio::disk constructor
    const int   O_DEVICE = 1;     // The file is a device and not a normal file
    const int   O_CREATE = 2;     // Create file if it doesn't exist
    const int   O_NORESET = 4;    // Do not reset the file contents
    
    //
    // Structure of the synchronization control byte.
    //
    // This byte usually resides at the beginning of the
    // floppy file for the receive buffer and at the end
    // of the file for the sending buffer.
    //
    // It's purpose is to force the entire floppy image
    // to be re-written/re-read by the hypervisor/guest OS and
    // to synchronize the I/O in case of large ammount of
    // data being exchanged.
    //
    struct fpio_ctlbyte {
        unsigned char bDataPresent   : 1;
        unsigned char bEndOfData     : 1;
        unsigned char bLengthPrefix  : 1;
        unsigned char bAborted       : 1;
        unsigned char usID           : 4;
    };

    // 
    // The entire floppy disk structure
    //
    // This structure is memory mapped to the i/o file
    // specified. It tries to override system caching
    // and access directly the other end.
    //
    struct disk_map {
        char            cBufferIn[SZ_FLOPPY/2-1];
        union {
            unsigned char   value;
            fpio_ctlbyte    flags;
        } bControlOut;
        union {
            unsigned char   value;
            fpio_ctlbyte    flags;
        } bControlIn;
        char            cBufferOut[SZ_FLOPPY/2-1];
    };


    //
    // Disk I/O Class
    //
    // This class provides the required I/O interface with the floppy disk
    // file or block device. It provides a memory-mapped structure with 
    // real-time communication with the other end.
    //
    class disk: 
        public errorbase 
    {
    public:

        // Constructor/Destructor
        disk(const char * file, int flags = 0);
        virtual             ~disk();

        // Utility functions
        void                reset();
        void                sync();
        void                update();
        virtual bool        ready();

        // Memory map
        disk_map *          map;

    private:

        int                 fd;         // File descriptor
        bool                useDevice;  // Use device I/O (ioctl when needed) instead of file I/O
            
    };


};

#endif  // DISK_H


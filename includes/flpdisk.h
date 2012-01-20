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
// File:   flpdisk.h
// Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
// License: GNU Lesser General Public License - Version 3.0
// -------------------------------------------------------------------
//
// Virtual disk I/O Header file
// 

#ifndef FLPDISK_H
#define FLPDISK_H

#include "errorbase.h"

#define FPIO_VERSION  0,3

namespace fpio {

    // The size of the floppy disk (1.44Mb)
    const int   SZ_FLOPPY = 1474560;

    // Open flags
    const int   O_DEVICE        = 1;     // Notifies FloppyIO that we are about to open a block device
    const int   O_CREATE        = 2;     // Create file if it doesn't exist
    const int   O_NORESET       = 4;     // Do not reset the file contents
    const int   O_EXCEPTIONS    = 8;     // Throw exceptions on errors
    const int   O_CLIENT        = 16;    // Swap in/out buffers
    const int   O_EXTENDED      = 32;    // Use extended protocol

    //
    // FloppyIO disk file layout information
    //
    // This structure provides the location and sizes
    // of all the data location inside the file.
    //
    struct disk_layout {
    
        unsigned int ofsControlIn;
        unsigned int ofsControlOut;
        unsigned int ofsBufferIn;
        unsigned int ofsBufferOut;

        unsigned int szControlByte;
        unsigned int szBufferIn;
        unsigned int szBufferOut;
        
    };

    // The default floppyIO structure
    const disk_layout FPIO_DEFAULT_STRUCTURE = {
        0,              // ofsControlIn
        1,              // ofsControlOut
        2,              // ofsBufferIn
        SZ_FLOPPY/2+2,  // ofsBufferOut
        
        1,              // szControlByte   Control byte is a byte (Duh!)
        SZ_FLOPPY/2,    // szBufferIn      Half of the block goes to input
        SZ_FLOPPY/2     // szBufferOut     The other half goes to output
    };
    
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
    union ctrlbyte {
        struct {
            unsigned char bDataPresent   : 1;   // 1=Data pending in the buffer
            unsigned char bStreaming     : 1;   // 1=The data in buffer is a part of a stream
            unsigned char bEndOfData     : 1;   // 1=That's the last buffer of a streaming sequence
            unsigned char bAborted       : 1;   // 1=The stream was aborted. Pending data are in buffer
            unsigned char bExtended      : 1;   // 1=There is an extended header present in the buffer
            unsigned char usID           : 3;   // Stream ID (0~7) for multiple stream I/O
        } flags;
        unsigned char     value;                // RAW Representation for simplified I/O
    };

    //
    // Extended header that might exist in the buffer
    //
    // This is commonly used for binary data transmittions, where you need
    // the actual size of the buffer rather than using null-terminated string.
    //
    // The size of the extended header is 16 bytes.
    //
    union extended_header {
        struct {
            unsigned int   szBuffer;            // The pending buffer size
            unsigned char  reserved[12];        // Reserved bytes for future use
        } data;
        unsigned char      value[16];           // RAW Representation for simplified I/O
    };

    //
    // Floppy Disk I/O Class
    //
    // This class provides the required I/O interface with the floppy disk
    // file or block device. It provides a memory-mapped structure with 
    // real-time communication with the other end.
    //
    class flpdisk: 
        public errorbase 
    {
    public:

        // Constructor/Destructor
        flpdisk(const char * file, int flags = 0);
        virtual             ~flpdisk();

        // Get/Set control bytes
        int                 get_in_cb( ctrlbyte * cb);
        int                 set_in_cb( ctrlbyte * cb);
        int                 get_out_cb( ctrlbyte * cb);
        int                 set_out_cb( ctrlbyte * cb);

        // Get/Set extended headers
        int                 get_in_xhdr( extended_header * hdr );
        int                 set_in_xhdr( extended_header * hdr );
        int                 get_out_xhdr( extended_header * hdr );
        int                 set_out_xhdr( extended_header * hdr );

        // Read/Write buffers
        int                 read_in( char * buffer, int szLen );
        int                 read_out( char * buffer, int szLen );
        int                 write_in( char * buffer, int szLen );
        int                 write_out( char * buffer, int szLen );

        // Utility functions
        int                 reset();
        int                 sync();
        virtual bool        ready();

        // Layout
        disk_layout         layout;

    private:

        int                 fd;          // File descriptor
        bool                useDevice;   // Use device I/O (ioctl when needed) instead of file I/O
        bool                useExtended; // Use extended version of the protocol
            
    };
    
};


#endif  // FLPDISK_H

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
// File:   flpdisk.cpp
// Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
// License: GNU Lesser General Public License - Version 3.0
// -------------------------------------------------------------------
//
// Virtual disk I/O class
// 

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#if defined __linux__
#include <sys/ioctl.h>
#include <linux/fd.h>
#include <linux/fs.h>
#endif

#include "../includes/flpdisk.h"

using namespace fpio;
using namespace std;

// The size of the extended header
const int SZ_EXTENDED_HEADER = sizeof( extended_header );


//
// FloppyIO Constructor
//
//
//
flpdisk::flpdisk(const char * file, int flags) {
    int lRet;

    // Use exceptions
    this->clear();
    this->fd=0;
    this->useDevice = false;

    // Update flags
    this->useExceptions=((flags & O_EXCEPTIONS) != 0);
    this->useExtended=((flags & O_EXTENDED) != 0);

    // Prepare open flags
    int oflags = O_RDWR | O_SYNC;
    if ((flags & fpio::O_DEVICE)==0) {
        // Create file if missing
        if ((flags & fpio::O_CREATE)!=0) oflags |= O_CREAT | O_TRUNC;
    } else {
        // Enable device access
        this->useDevice=true;
    }

    // Open the file
    this->fd = open(file, oflags, (mode_t)0600);
    if (this->fd<0) {
        this->setError("Unable to open the floppy file", strerror(errno), ERR_IO, ERL_ERROR);
        return;
    }
    
    // Make sure file is long enough
    int fSize = lseek(this->fd, 0, SEEK_END);
    if (fSize < SZ_FLOPPY) {

        // Go to the end
        lRet=lseek(this->fd, SZ_FLOPPY-1, SEEK_SET);
        if (lRet == -1) {
            this->setError("Unable to stretch floppy file",strerror(errno), ERR_IO, ERL_ERROR);
            return;
        }

        // And write one byte to stretch it
        lRet=write(this->fd, "", 1);
        if (lRet != 1) {
            this->setError("Unable to stretch floppy file",strerror(errno), ERR_IO, ERL_ERROR);
            return;
        }
    }

    // Initialize layout
    this->layout = FPIO_DEFAULT_STRUCTURE;
    if ((flags & O_CLIENT) != 0) {
        unsigned int tmp;

        // Swap control byte positions
        tmp = this->layout.ofsControlOut;
        this->layout.ofsControlOut = this->layout.ofsControlIn;
        this->layout.ofsControlIn = tmp;
        
        // Swap buffer positions
        tmp = this->layout.ofsBufferOut;
        this->layout.ofsBufferOut = this->layout.ofsBufferIn;
        this->layout.ofsBufferIn = tmp;

        // Swap buffer sizes
        tmp = this->layout.szBufferOut;
        this->layout.szBufferOut = this->layout.szBufferIn;
        this->layout.szBufferIn = tmp;
        
    }

    // Check if we have to reset this file
    if ((flags & fpio::O_NORESET)==0) this->reset();

}

//
// Reset the FloppyIO disk file
//
int flpdisk::reset() {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Go to the beginning
    if (lseek(this->fd, 0, SEEK_SET) == -1)
        return this->setError("Unable to reset floppy file",strerror(errno), ERR_IO, ERL_ERROR);

    // Write zeroes
    char * buf = new char[SZ_FLOPPY+1];
    memset(buf, 0, SZ_FLOPPY);
    if (write(this->fd, buf, SZ_FLOPPY) != SZ_FLOPPY)
        return this->setError("Unable to reset floppy file",strerror(errno), ERR_IO, ERL_ERROR);

    // Synchronize
    return this->sync();
};


//
// Synchronize I/O
//
// This function flushes buffers, resets floppy devices
// and 
//
int flpdisk::sync() {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Sync changes
    if (fsync(this->fd) == -1)
        return this->setError("Unable to synchronize floppy file",strerror(errno), ERR_IO, ERL_ERROR);

    // Flush buffers
#if defined __linux__
    ioctl(this->fd, FDFLUSH);
    ioctl(this->fd, BLKFLSBUF);
#endif

    // No error
    return ERR_NONE;

}

// 
// A bit more extended ready() function
//
bool flpdisk::ready() {
    if (this->fd<=0) return false;
    return errorbase::ready();
};

//
// FloppyIO Destructor
//
flpdisk::~flpdisk() {
    if (this->error()) return;
    close(this->fd);
};

//
// ==[ I/O Functions ]================================================
//

//
// Read the INPUT Extended header
// 
int flpdisk::get_in_xhdr(extended_header * hdr) {

    // If we are not using extended protocol raise error
    if (!this->useExtended) return this->setError("You asked for XHDR operations, but you are not using extended protocol", "Usage error", ERR_INVALID, ERL_ERROR);

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsBufferIn, SEEK_SET) == -1)
        return this->setError("Unable to read input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (read(this->fd, hdr->value, SZ_EXTENDED_HEADER) != SZ_EXTENDED_HEADER) 
        return this->setError("Unable to read input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Read the OUTPUT Extended header
// 
int flpdisk::get_out_xhdr(extended_header * hdr) {

    // If we are not using extended protocol raise error
    if (!this->useExtended) return this->setError("You asked for XHDR operations, but you are not using extended protocol", "Usage error", ERR_INVALID, ERL_ERROR);

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsBufferOut, SEEK_SET) == -1)
        return this->setError("Unable to read input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (read(this->fd, hdr->value, SZ_EXTENDED_HEADER) != SZ_EXTENDED_HEADER) 
        return this->setError("Unable to read input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Write the INPUT Extended header
// 
int flpdisk::set_in_xhdr(extended_header * hdr) {

    // If we are not using extended protocol raise error
    if (!this->useExtended) return this->setError("You asked for XHDR operations, but you are not using extended protocol", "Usage error", ERR_INVALID, ERL_ERROR);

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsBufferIn, SEEK_SET) == -1)
        return this->setError("Unable to write input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (write(this->fd, hdr->value, SZ_EXTENDED_HEADER) != SZ_EXTENDED_HEADER) 
        return this->setError("Unable to write input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Write the OUTPUT Extended header
// 
int flpdisk::set_out_xhdr(extended_header * hdr) {

    // If we are not using extended protocol raise error
    if (!this->useExtended) return this->setError("You asked for XHDR operations, but you are not using extended protocol", "Usage error", ERR_INVALID, ERL_ERROR);

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsBufferOut, SEEK_SET) == -1)
        return this->setError("Unable to read input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (write(this->fd, hdr->value, SZ_EXTENDED_HEADER) != SZ_EXTENDED_HEADER) 
        return this->setError("Unable to read input extended header",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Read the INPUT Control byte
// 
int flpdisk::get_in_cb(ctrlbyte * cb) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsControlIn, SEEK_SET) == -1)
        return this->setError("Unable to read input control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (read(this->fd, &cb->value, 1) != 1) 
        return this->setError("Unable to read input control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Read the OUTPUT Control byte
// 
int flpdisk::get_out_cb(ctrlbyte * cb) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsControlIn, SEEK_SET) == -1)
        return this->setError("Unable to read output control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (read(this->fd, &cb->value, 1) != 1) 
        return this->setError("Unable to read output control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Write the INPUT Control byte
// 
int flpdisk::set_in_cb(ctrlbyte * cb) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsControlIn, SEEK_SET) == -1)
        return this->setError("Unable to write input control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (write(this->fd, &cb->value, 1) != 1) 
        return this->setError("Unable to write input control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Write the OUTPUT Control byte
// 
int flpdisk::set_out_cb(ctrlbyte * cb) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Seek to the input control byte
    if (lseek(this->fd, this->layout.ofsControlIn, SEEK_SET) == -1)
        return this->setError("Unable to write output control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (write(this->fd, &cb->value, 1) != 1) 
        return this->setError("Unable to write output control byte",strerror(errno), ERR_IO, ERL_ERROR);

    // No error
    return ERR_NONE;
}

//
// Read the input buffer
//
int flpdisk::read_in(char * buffer, int szLen) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Prevent overflow
    if (szLen > this->layout.szBufferIn)
        szLen = this->layout.szBufferIn;

    // If we are using extended version, skip extended header
    int szOffset = this->layout.ofsBufferIn;
    if (this->useExtended) {

        // Check for overflow if we shift the output
        if (szLen + SZ_EXTENDED_HEADER > this->layout.szBufferIn)
            szLen = this->layout.szBufferIn-SZ_EXTENDED_HEADER;

        // Seek a bit further
        szOffset += SZ_EXTENDED_HEADER;
        
    }

    // Seek to the input control byte
    if (lseek(this->fd, szOffset, SEEK_SET) == -1)
        return this->setError("Unable to read input buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (read(this->fd, buffer, szLen) != szLen) 
        return this->setError("Unable to read input buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // No error = the size of the data read
    return szLen;

}

//
// Read the out buffer
//
int flpdisk::read_out(char * buffer, int szLen) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Prevent overflow
    if (szLen > this->layout.szBufferOut)
        szLen = this->layout.szBufferOut;

    // If we are using extended version, skip extended header
    int szOffset = this->layout.ofsBufferOut;
    if (this->useExtended) {

        // Check for overflow if we shift the output
        if (szLen + SZ_EXTENDED_HEADER > this->layout.szBufferOut)
            szLen = this->layout.szBufferOut-SZ_EXTENDED_HEADER;

        // Seek a bit further
        szOffset += SZ_EXTENDED_HEADER;
        
    }

    // Seek to the input control byte
    if (lseek(this->fd, szOffset, SEEK_SET) == -1)
        return this->setError("Unable to read output buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (read(this->fd, buffer, szLen) != szLen) 
        return this->setError("Unable to read output buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // No error = the size of the data read
    return szLen;

}


//
// Write the input buffer
//
int flpdisk::write_in(char * buffer, int szLen) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Prevent overflow
    if (szLen > this->layout.szBufferIn)
        szLen = this->layout.szBufferIn;

    // If we are using extended version, skip extended header
    int szOffset = this->layout.ofsBufferIn;
    if (this->useExtended) {

        // Check for overflow if we shift the output
        if (szLen + SZ_EXTENDED_HEADER > this->layout.szBufferIn)
            szLen = this->layout.szBufferIn-SZ_EXTENDED_HEADER;

        // Seek a bit further
        szOffset += SZ_EXTENDED_HEADER;
        
    }

    // Seek to the input control byte
    if (lseek(this->fd, szOffset, SEEK_SET) == -1)
        return this->setError("Unable to write input buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (write(this->fd, buffer, szLen) != szLen) 
        return this->setError("Unable to write input buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // No error = the size of the data read
    return szLen;

}

//
// Read the out buffer
//
int flpdisk::write_out(char * buffer, int szLen) {

    // Make sure floppy is ready    
    if (!this->ready()) return ERR_NOTREADY;

    // Prevent overflow
    if (szLen > this->layout.szBufferOut)
        szLen = this->layout.szBufferOut;

    // If we are using extended version, skip extended header
    int szOffset = this->layout.ofsBufferOut;
    if (this->useExtended) {

        // Check for overflow if we shift the output
        if (szLen + SZ_EXTENDED_HEADER > this->layout.szBufferOut)
            szLen = this->layout.szBufferOut-SZ_EXTENDED_HEADER;

        // Seek a bit further
        szOffset += SZ_EXTENDED_HEADER;
        
    }

    // Seek to the input control byte
    if (lseek(this->fd, szOffset, SEEK_SET) == -1)
        return this->setError("Unable to write output buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // Try to read input
    this->sync();
    if (write(this->fd, buffer, szLen) != szLen) 
        return this->setError("Unable to write output buffer",strerror(errno), ERR_IO, ERL_ERROR);

    // No error = the size of the data read
    return szLen;

}


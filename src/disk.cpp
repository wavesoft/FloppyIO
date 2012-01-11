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
// File:   disk.cpp
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

#include "../includes/disk.h"

using namespace fpio;
using namespace std;

disk::disk(const char * file, int flags) {
    // Use exceptions
    this->useExceptions=true;
    this->clear();
    this->fd=0;

    // Prepare the open flags
    int oflags = O_RDWR | O_SYNC;
    if ((flags & fpio::O_DEVICE)==0) {

        // Create file if missing
        if ((flags & fpio::O_CREATE)!=0) oflags |= O_CREAT;
        
    } else {

        // Avoid buffering when accessing the block device
        #ifdef _GNU_SOURCE
        oflags |= O_DIRECT; // Non-posix
        #endif

    }


    // Open the file
    this->fd = open(file, oflags);
    if (this->fd<0) {
        this->setError(strerror(errno), ERR_IO);
        this->setError("Unable to open the floppy file", ERR_IO, 2);
        return;
    }

    // Check if we have to reset this file
    if ((flags & fpio::O_NORESET)==0) this->reset();

    // Map FD
    this->map = (disk_map*) mmap(0, SZ_FLOPPY, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0);
    if (this->map == MAP_FAILED) {
        close(this->fd);
        this->fd=0;
        this->setError(strerror(errno), ERR_IO);
        this->setError("Unable to map memory region", ERR_IO, 2);
        return;
    }

    // Allocate the map
    this->map = new disk_map;

};

void disk::reset() {
    if (!this->ready()) return;
    memset(&this->map, 0, SZ_FLOPPY);
    this->sync();
};

void disk::sync() {
    if (!this->ready()) return;
    msync(&this->map, SZ_FLOPPY, MS_SYNC | MS_INVALIDATE);
};

bool disk::ready() {
    if (this->fd<=0) return false;
    return errorbase::ready();
};

disk::~disk() {
    if (this->error()) return;
    if (this->map != MAP_FAILED) munmap(this->map, SZ_FLOPPY);
    close(this->fd);
};


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
// File:   FloppyIO.cpp
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

#include <string.h>
#include <iostream>

#include "../includes/floppyIO.h"

using namespace std;
using namespace fpio;

//
// Constructor
//
floppyIO::floppyIO(const char * file, int flags) : flpdisk(file, flags) {

    this->syncTimeout = SYNC_TIMEOUT;
    this->useSynchronization = (( flags & O_SYNCHRONIZED) != 0);

}

//
// Destructor
//
floppyIO::~floppyIO() {

}

//
// Wait for data to be available on input
//
int floppyIO::waitForSyncIn(unsigned short streamID, int timeout) {
    int lRet = ERR_NONE;
    time_t tExpired = time (NULL) + timeout;

    // Get last status of inCB
    this->get_in_cb(&inCB);

    // Wait until expired, error, or forever.
    while (((timeout == 0) || ( time(NULL) <= tExpired)) && (lRet == ERR_NONE)) {

        // Wait until we have data available
        if (inCB.bDataPresent != 0) {
            if (inCB.sID == streamID) break; // If the streamID matches the one specified, quit
        }

        // Update inCB
        lRet = this->get_in_cb(&inCB);

        // Sleep for a while, not to overload CPU
        usleep(1000);

    }

    // Check for timeout
    if ((timeout != 0) && ( time(NULL) > tExpired)) 
        lRet = this->setError("Timeout while waiting for input!", ERR_TIMEOUT, ERL_ERROR);

    return lRet;    
}

//
// Wait for data to be available on output
//
int floppyIO::waitForSyncOut(unsigned short streamID, int timeout) {
    int lRet = ERR_NONE;
    time_t tExpired = time (NULL) + timeout;

    // Wait until expired, error, or forever.
    while (((timeout == 0) || ( time(NULL) <= tExpired)) && (lRet == ERR_NONE)) {
        lRet = this->get_out_cb(&outCB);

        // Wait until we have data available
        if (outCB.bDataPresent == 0) {
            if (outCB.sID == streamID) break; // If the streamID matches the one specified, quit
        }

        // Sleep for a while, not to overload CPU
        usleep(1000);

    }

    // Check for timeout
    if ((timeout != 0) && ( time(NULL) > tExpired)) 
        lRet = this->setError("Timeout while waiting for output to be read!", ERR_TIMEOUT, ERL_ERROR);

    return lRet;    
}

//
// Send Data
//
// If you want extended functionality, setup outCB
// before calling this.
//
int floppyIO::send(char * buffer, int size, int streamID) {
    int lRet;

    // Write the output data
    lRet = write_out(buffer, size);
    if (lRet<0) return lRet;

    // If we have extended information, write extended header
    if (this->useExtended) {
        outHDR.szLength = size;
        set_out_xhdr(&outHDR);
    }

    // Write output control byte
    outCB.sID = streamID;
    outCB.bDataPresent = 1;
    outCB.bExtended = this->useExtended ? 1 : 0;
    set_out_cb(&outCB);

    // Wait for sync output
    if (this->useSynchronization)
        lRet = waitForSyncOut(streamID, this->syncTimeout);

    // Return the bytes sent
    return lRet;
    
}

//
// Receive Data
//
// If you want extended functionality, setup outCB
// before calling this.
//
int floppyIO::receive(char * buffer, int size, int streamID) {
    int lRet;

    // Wait for sync input
    if (this->useSynchronization)
        lRet = waitForSyncIn(streamID, this->syncTimeout);

    // Read the input data
    lRet = read_in(buffer, size);
    if (lRet<0) return lRet;

    // Locate the end of the data
    if (this->useExtended) {
        get_in_xhdr(&inHDR);
        lRet = inHDR.szLength;
    } else {
        lRet = strlen(buffer);
    }

    // Data are no more present
    inCB.bDataPresent=0;
    set_in_cb(&inCB);

    // Return the bytes sent
    return lRet;
    
}

//
// Streaming Sending Data
//
int floppyIO::send(istream * stream, unsigned short id) {
    int sz_chunk = this->layout.szBufferOut;
    int sentLength, rd, lRet;

    // Resize chunk if we are using extended header
    if (this->useExtended)
        sz_chunk -= SZ_EXTENDED_HEADER;

    // Check if stream is not good
    if (!stream->good()) return this->setError("Unable to open input stream!", ERR_INPUT, ERL_ERROR);

    // While stream is good, start processing
    char * inBuffer = new char[sz_chunk+1];
    while (stream->good()) {

        // Zero the input buffer (Plus one null-termination byte)
        memset(inBuffer, 0, sz_chunk+1);

        // Read data 
        stream->read(inBuffer, sz_chunk);
        rd = stream->gcount();

        // Check status
        if (stream->eof() || (stream->tellg() < 0)) {
            // EOF? Mark end-of-data on the current block
            outCB.bEndOfData = 1;
            outCB.bAborted = 0;

        } else if (stream->fail()) {
            // Got fail without getting eof? Something went wrong

            // Notify the remote end that we failed the transmittion
            outCB.bEndOfData = 1;
            outCB.bAborted = 1;
            lRet = this->send((char*)"", 1, id); // Send Zero data and the appropriate control bits

            // Return error            
            return this->setError("Unable to open input stream!", ERR_INPUT, ERL_ERROR);
            
        } else {

            // Not end of data yet
            outCB.bEndOfData = 0;
            outCB.bAborted = 0;

        }

        // Count bytes written
        if (rd > 0) {
            lRet = this->send(inBuffer, rd, id);
            if (lRet<0) return lRet; // Error occured
        }
                
        sentLength+=lRet;

    }

    // Completed
    return sentLength;

}

// 
// Streaming Receiving Data
//
int floppyIO::receive(ostream * stream, unsigned short id) {
    int sz_chunk = this->layout.szBufferOut;
    int receivedLength, rd, lRet;

    // Resize chunk if we are using extended header
    if (this->useExtended)
        sz_chunk -= SZ_EXTENDED_HEADER;

    // Prepare and start reading loop
    char * inBuffer = new char[sz_chunk+1];
    receivedLength = 0;
    while (1) {

        // Try to read chunk
        lRet = receive( inBuffer, sz_chunk, id );
        if (lRet < 0) { // Error
            stream->setstate(ostream::badbit);
            break;
        }

        // Push the data to the stream
        if (lRet > 0) {
            stream->write(inBuffer, lRet);
            stream->flush();
            receivedLength += lRet;
        }

        // Check if stream was aborted or finished
        if (inCB.bAborted == 1) {
            stream->setstate(ostream::badbit | ostream::eofbit);
            break;
            
        } else if (inCB.bEndOfData == 1) {
            stream->setstate(ostream::eofbit);
            break;
        }

    }

    // Return the bytes sent
    return receivedLength;
}




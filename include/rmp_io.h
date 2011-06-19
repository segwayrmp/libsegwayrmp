/**
 * @file rmp_io.h
 * @author  William Woodall <wjwwood@gmail.com>
 * @version 0.1
 *
 * @section LICENSE
 *
 * The BSD License
 *
 * Copyright (c) 2011 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This provides an abstract I/O interface for communicating with the RMP.
 * 
 * This library depends on Boost: http://www.boost.org/
 */

#ifndef RMP_IO_H
#define RMP_IO_H

#include <vector>

#include <boost/thread.hpp>

namespace segwayrmp {

typedef struct {
    unsigned short id;
    unsigned char channel;
    unsigned char data[8];
} Packet;

static unsigned int BUFFER_SIZE = 36;

class RMPIO {
public:
    // Must be implemented by child class
    virtual void connect() = 0;
    
    // Must be implemented by child class
    virtual void disconnect() = 0;
    
    // Must be implemented by child class
    virtual int read(unsigned char* buffer, int size) = 0;
    
    // Must be implemented by child class
    virtual int write(unsigned char* buffer, int size) = 0;
    
    void getPacket(Packet &packet);
    
    void sendPacket(Packet &packet);
    
    bool isConnected() {return this->connected;}
    
protected:
    void fillBuffer();
    unsigned char computeChecksum(unsigned char* usb_packet);
    
    bool connected;
    
    std::vector<unsigned char> data_buffer;
};

class PacketRetrievalException : public std::exception {
    int _error_number;
    const char * e_what;
public:
    PacketRetrievalException(int _error_number, const char * e_what) {
        this->_error_number = _error_number;
        this->e_what = e_what;
    }
    
    int error_number() {
        return this->_error_number;
    }
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Error retrieving a packet from the SegwayRMP: " << this->e_what;
        return ss.str().c_str();
    }
};

} // Namespace segwayrmp

#endif
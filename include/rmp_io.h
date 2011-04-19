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
 * This provides different I/O methods for communicating with the RMP.
 */

#ifndef RMP_IO_H
#define RMP_IO_H

#include <vector>

#include "serial.h"

namespace segwayrmp {

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
    
    virtual void configure(std::string port, int baudrate) = 0;
    
    void getPacket(unsigned char* packet);
    
    bool isConnected() {return this->connected;}
    
protected:
    bool connected;
    
private:
    void fillBuffer();
    
    std::vector<unsigned char> data_buffer;
    char * current_index;
};

class SerialRMPIO : public RMPIO {
public:
    SerialRMPIO();
    ~SerialRMPIO();
    
    void connect();
    
    void disconnect();
    
    int read(unsigned char* buffer, int size);
    
    int write(unsigned char* buffer, int size);
    
    void configure(std::string port, int baudrate);
protected:
    bool configured;
    
    std::string port;
    int baudrate;
    
    serial::Serial serial_port;
};

class PacketRetrievalException : public std::exception {
    const char * e_what;
public:
    PacketRetrievalException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Error retrieving a packet from the SegwayRMP: " << this->e_what;
        return ss.str().c_str();
    }
};

} // Namespace segwayrmp

#endif
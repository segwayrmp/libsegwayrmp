/**
 * @file segwayrmp.h
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
 * This provides a cross platform interface for the Segway RMP's.
 */
 
#ifndef SEGWAYRMP_H
#define SEGWAYRMP_H

#include "serial.h"

namespace segwayrmp {

class SegwayRMP {
public:
    SegwayRMP(std::string port);
    ~SegwayRMP();
    
    void connect();
    
    bool go();
    
    // Getter and Setters
    
private:
    bool validatePacket(unsigned char* usb_packet);
    unsigned char computeChecksum(unsigned char* usb_packet);
    
    std::string port;
    
    serial::Serial serial_port;
};

class ConnectionFailedException : public std::exception {
    const char * e_what;
public:
    ConnectionFailedException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Error connecting to SegwayRMP: " << this->e_what;
        return ss.str().c_str();
    }
};

} // Namespace segwayrmp

#endif

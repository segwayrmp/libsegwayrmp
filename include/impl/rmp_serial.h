/**
 * @file rmp_serial.h
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
 * This provides a Serial based implementation of the rmp_io interface.
 * 
 * This library depends on a Serial library: https://github.com/wjwwood/serial
 * and Boost: http://www.boost.org/
 */

#include "../rmp_io.h"

#include "serial.h"

namespace segwayrmp {

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

}
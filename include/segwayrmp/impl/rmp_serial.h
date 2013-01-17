/*!
 * \file rmp_serial.h
 * \author  William Woodall <wjwwood@gmail.com>
 * \version 0.1
 *
 * \section LICENSE
 *
 * The BSD License
 *
 * Copyright (c) 2013 William Woodall
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
 * \section DESCRIPTION
 *
 * This provides a Serial based implementation of the rmp_io interface.
 * 
 * This library depends on a Serial library: https://github.com/wjwwood/serial
 */

#ifndef RMP_SERIAL_H
#define RMP_SERIAL_H

#include "rmp_io.h"

#include "serial/serial.h"

namespace segwayrmp {

/*!
 * Provides a serial based interface for reading and writing packets.
 */
class SerialRMPIO : public RMPIO {
public:
    /*!
     * Constructs the SerialRMPIO object.
     */
    SerialRMPIO();
    ~SerialRMPIO();
    
    /*!
     * Connects to the serial port if it has been configured. Can throw ConnectionFailedException.
     */
    void connect();
    
    /*!
     * Disconnects from the serial port if it is open.
     */
    void disconnect();
    
    /*!
     * Read Function, reads from the serial port.
     * 
     * \param buffer An unsigned char array for data to be read into.
     * \param size The amount of data to be read.
     * \return int Bytes read.
     */
    int read(unsigned char* buffer, int size);
    
    /*!
     * Write Function, writes to the serial port.
     * 
     * \param buffer An unsigned char array of data to be written.
     * \param size The amount of data to be written.
     * \return int Bytes written.
     */
    int write(unsigned char* buffer, int size);
    
    /*!
     * Configures the serial port.
     * 
     * \param port The com port identifier like '/dev/ttyUSB0' on POSIX and like 'COM1' on windows.
     * \param baudrate The speed of the serial communication.
     */
    void configure(std::string port, int baudrate);
    
private:
    bool configured;
    
    std::string port;
    int baudrate;
    
    serial::Serial serial_port;
};

}

#endif
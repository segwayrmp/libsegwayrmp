/*!
 * \file rmp_ftd2xx.h
 * \author  William Woodall <wjwwood@gmail.com>
 * \version 0.1
 *
 * \section LICENSE
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
 * \section DESCRIPTION
 *
 * This provides a FTD2XX based usb implementation of the rmp_io interface.
 * 
 * This library depends on the FTD2XX driver: http://www.ftdichip.com/Drivers/D2XX.htm
 */

#ifndef RMP_FTD2XX_H
#define RMP_FTD2XX_H

#include "../rmp_io.h"

#include "ftd2xx.h"

namespace segwayrmp {

/*!
 * Defines the possible methods of configuring the usb port.
 */
typedef enum {
    by_serial_number   = 0, /*!< This method requires a serial number like "FT000001". */
    by_description     = 1, /*!< This method requires a description like "Robotic Mobile Platform". */
    by_index           = 2, /*!< This method requires an index like 0 or 1.  This selects amoung all FTD2XX devices. */
    by_none            = 3  /*!< Means it hasn't been set yet. */
} ConfigurationType;

class FTD2XXRMPIO : public RMPIO {
public:
    FTD2XXRMPIO();
    ~FTD2XXRMPIO();
    
    void connect();
    
    void disconnect();
    
    int read(unsigned char* buffer, int size);
    
    int write(unsigned char* buffer, int size);
    
    void configureUSBBySerial(std::string serial_number, int baudrate = 460800);
    
    void configureUSBByDescription(std::string description, int baudrate = 460800);
    
    void configureUSBByIndex(unsigned int device_index, int baudrate = 460800);
    
private:
    void enumerateUSBDevices();
    void connectBySerial();
    void connectByDescription();
    void connectByIndex();
    std::string getErrorMessageByFT_STATUS(FT_STATUS result, std::string what);
    
    bool configured;
    
    bool is_open;
    
    ConfigurationType config_type;
    
    std::string port_serial_number;
    std::string port_description;
    unsigned int port_index;
    
    int baudrate;
    
    FT_HANDLE usb_port_handle;
};

}

#endif
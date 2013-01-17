/*!
 * \file rmp_ftd2xx.h
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
 * This provides a FTD2XX based usb implementation of the rmp_io interface.
 * 
 * This library depends on the FTD2XX driver:
 * http://www.ftdichip.com/Drivers/D2XX.htm
 */

#ifndef RMP_FTD2XX_H
#define RMP_FTD2XX_H

#include "segwayrmp/impl/rmp_io.h"

#include "segwayrmp/ftd2xx.h"

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

std::vector<FT_DEVICE_LIST_INFO_NODE> enumerateUSBDevices();

class FTD2XXRMPIO : public RMPIO {
public:
    /*!
     * Constructs the FTD2XXRMPIO object.
     */
    FTD2XXRMPIO();
    ~FTD2XXRMPIO();
    
    /*!
     * Connects to the usb port if it has been configured. Can throw ConnectionFailedException.
     */
    void connect();
    
    /*!
     * Disconnects from the usb port if it is open.
     */
    void disconnect();
    
    /*!
     * Read Function, reads from the usb port.
     * 
     * \param buffer An unsigned char array for data to be read into.
     * \param size The amount of data to be read.
     * \return int Bytes read.
     */
    int read(unsigned char* buffer, int size);
    
    /*!
     * Write Function, writes to the usb port.
     * 
     * \param buffer An unsigned char array of data to be written.
     * \param size The amount of data to be written.
     * \return int Bytes written.
     */
    int write(unsigned char* buffer, int size);
    
    /*!
     * Configures the usb port using the devices Serial Number.
     * 
     * \param serial_number The serial number of the RMP device, something like '00000056'.
     * \param baudrate The speed of the usb communication.
     */
    void configureUSBBySerial(std::string serial_number, int baudrate);
    
    /*!
     * Configures the usb port using the devices Description.
     * 
     * \param description The description of the RMP device, something like 'Robotic Mobile Platform'.
     * \param baudrate The speed of the usb communication.
     */
    void configureUSBByDescription(std::string description, int baudrate);
    
    /*!
     * Configures the usb port using the devices index.
     * 
     * \param device_index The index of the FTDI, something like 0. 
     * This is its index amoung all FTD2XX devices on the system.
     * \param baudrate The speed of the usb communication.
     */
    void configureUSBByIndex(unsigned int device_index, int baudrate);
    
private:
    std::vector<FT_DEVICE_LIST_INFO_NODE> enumerateUSBDevices_();
    void connectBySerial();
    void connectByDescription();
    void connectByIndex();
    
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

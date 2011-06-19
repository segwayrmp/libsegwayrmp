#include "segwayrmp.h"
#include "rmp_serial.h"

using namespace segwayrmp;

/////////////////////////////////////////////////////////////////////////////
// SerialRMPIO

SerialRMPIO::SerialRMPIO() : configured(false), baudrate(115200), port("") {
    this->connected = false;
}

SerialRMPIO::~SerialRMPIO() {
    this->disconnect();
}

void SerialRMPIO::configure(std::string port, int baudrate) {
    this->port = port;
    this->baudrate = baudrate;
    this->configured = true;
}

void SerialRMPIO::connect() {
    if(!this->configured) {
        throw(ConnectionFailedException("The serial port must be configured before connecting."));
    }
    try {
        // Configure and open the serial port
        this->serial_port.setPort(this->port);
        this->serial_port.setBaudrate(this->baudrate);
        this->serial_port.setTimeoutMilliseconds(1000);
        this->serial_port.open();
    } catch(std::exception &e) {
        throw(ConnectionFailedException(e.what()));
    }
    this->connected = true;
}

void SerialRMPIO::disconnect() {
    if(this->connected) {
        if(this->serial_port.isOpen())
            this->serial_port.close();
        this->connected = false;
    }
}

int SerialRMPIO::read(unsigned char* buffer, int size) {
    return this->serial_port.read(reinterpret_cast<char*>(buffer), size);
}

int SerialRMPIO::write(unsigned char* buffer, int size) {
    return this->serial_port.write(reinterpret_cast<char*>(buffer), size);
}

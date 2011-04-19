#include "segwayrmp.h"
#include "rmp_io.h"

using namespace segwayrmp;

/////////////////////////////////////////////////////////////////////////////
// RMPIO

void RMPIO::getPacket(unsigned char* packet) {
    // if(!this->connected)
        // throw(PacketRetrievalException("Not connected."));
    
    bool packet_complete = false;
    int packet_index = 0;
    
    while(!packet_complete) {
        if(this->data_buffer.size() < 18)
            this->fillBuffer();
        
        // If looking for start of packet and start of packet
        if(packet_index == 0 && this->data_buffer[0] == 0xF0) {
            // Put the 0xF0 in the packet
            packet[packet_index] = this->data_buffer[0];
            // Remove the 0xF0 from the buffer
            this->data_buffer.erase(this->data_buffer.begin());
            // Look for next packet byte
            packet_index += 1;
        } else if(packet_index == 0) { // If we were looking for the first byte, but didn't find it
            // Remove the invalid byte from the buffer
            this->data_buffer.erase(this->data_buffer.begin());
        }
        
        // If looking for second byte of packet and second byte of packet
        if(packet_index == 1 && this->data_buffer[0] == 0x55) {
            // Put the 0x55 in the packet
            packet[packet_index] = this->data_buffer[0];
            // Remove the 0x55 from the buffer
            this->data_buffer.erase(this->data_buffer.begin());
            // Look for next packet byte
            packet_index += 1;
        } else if(packet_index == 1) { // Else were looking for second byte but didn't find it
            // Reset the packet index to start search for packet over
            packet_index = 0;
        }
        
        // If looking for channel byte and channel A or B
        if(packet_index == 2 && (this->data_buffer[0] == 0xAA || this->data_buffer[0] == 0xBB)) {
            // Put the channel in the packet
            packet[packet_index] = this->data_buffer[0];
            // Remove the channel from the buffer
            this->data_buffer.erase(this->data_buffer.begin());
            // Look for next packet byte
            packet_index += 1;
        } else if(packet_index == 2) { // Else were looking for channel byte but didn't find it
            // Reset the packet index to start search for packet over
            packet_index = 0;
        }
        
        // If packet_index >= 3 then we just need to collect the rest of the bytes
        // (we assume that if the previous three bytes were recieved then this is a valid packet, 
        //  if it isn't the checksum will fail)
        if(packet_index >= 3) {
            // Put the next btye in the packet
            packet[packet_index] = this->data_buffer[0];
            // Remove the byte from the buffer
            this->data_buffer.erase(this->data_buffer.begin());
            // Look for next packet byte
            packet_index += 1;
        }
        
        // If packet_index is 18 then we have a full packet
        if(packet_index == 18)
            packet_complete = true;
    }
    return;
}

void RMPIO::fillBuffer() {
    unsigned char buffer[BUFFER_SIZE];
    // Read up to BUFFER_SIZE what ever is needed to fill the vector to BUFFER_SIZE
    int bytes_read = this->read(buffer, BUFFER_SIZE-this->data_buffer.size());
    // Append the buffered data to the vector
    this->data_buffer.insert(this->data_buffer.end(), buffer, buffer+bytes_read);
}

/////////////////////////////////////////////////////////////////////////////
// SerialRMPIO

SerialRMPIO::SerialRMPIO() : configured(false), baudrate(115200), port("") {
    this->connected = true;
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
        throw(ConnectionFailedException("The serial port must be configured before connecting!"));
    }
    try {
        // Configure and open the serial port
        this->serial_port.setPort(this->port);
        this->serial_port.setBaudrate(this->baudrate);
        this->serial_port.setTimeoutMilliseconds(250);
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
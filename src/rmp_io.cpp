#include "segwayrmp.h"
#include "rmp_io.h"

inline void printHex(char * data, int length) {
    for(int i = 0; i < length; ++i) {
        printf("0x%.2X ", (unsigned)(unsigned char)data[i]);
    }
    printf("\n");
}

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
        throw(ConnectionFailedException("The serial port must be configured before connecting!"));
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

void SerialRMPIO::getPacket(Packet &packet) {
    if(!this->connected)
        throw(PacketRetrievalException(1, "Not connected."));
    
    unsigned char usb_packet[18];
    bool packet_complete = false;
    int packet_index = 0;
    
    while(!packet_complete) {
        // Top the buffer off
        if(this->data_buffer.size() < 18)
            this->fillBuffer();
        
        // Ensure that there is data in the buffer...
        if(this->data_buffer.size() < 1) {
            throw(PacketRetrievalException(3, "No data received from Segway."));
        }
        
        // If looking for start of packet and start of packet
        if(packet_index == 0 && this->data_buffer[0] == 0xF0) {
            // Put the 0xF0 in the packet
            usb_packet[packet_index] = this->data_buffer[0];
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
            usb_packet[packet_index] = this->data_buffer[0];
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
            usb_packet[packet_index] = this->data_buffer[0];
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
            usb_packet[packet_index] = this->data_buffer[0];
            // Remove the byte from the buffer
            this->data_buffer.erase(this->data_buffer.begin());
            // Look for next packet byte
            packet_index += 1;
        }
        
        // If packet_index is 18 then we have a full packet
        if(packet_index == 18)
            packet_complete = true;
    }
    
    // Check the Checksum
    if(usb_packet[17] != this->computeChecksum(usb_packet))
        throw(PacketRetrievalException(2, "Checksum mismatch."));
    
    // Convert to the packet type
    packet.channel = usb_packet[2];
    packet.id = ((usb_packet[4] << 3) | ((usb_packet[5] >> 5) & 7)) & 0x0fff;
    for (int i = 0; i < 8; i++)  {
        packet.data[i] = usb_packet[i + 9];
    }
    
    return;
}

void SerialRMPIO::sendPacket(Packet &packet) {
    unsigned char usb_packet[18] = {0xF0, 0x55, 0x00, 0x00, 0x00, 0x00, 0x04, 0x13, 0x00, 
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // Set the desitnation channel, 0x01 for 0xAA and 0x02 for 0xBB
    usb_packet[2] = packet.channel;
    // Copy movement and configuration commands
    for(int i = 0; i < 8; ++i) {
        usb_packet[9+i] = packet.data[i];
    }
    // Compute and set the checksum
    usb_packet[17] = this->computeChecksum(usb_packet);
    // Write the data
    this->write(usb_packet, 18);
}

void SerialRMPIO::fillBuffer() {
    unsigned char buffer[BUFFER_SIZE];
    // Read up to BUFFER_SIZE what ever is needed to fill the vector to BUFFER_SIZE
    int bytes_read = this->read(buffer, BUFFER_SIZE-this->data_buffer.size());
    // Append the buffered data to the vector
    this->data_buffer.insert(this->data_buffer.end(), buffer, buffer+bytes_read);
}

unsigned char SerialRMPIO::computeChecksum(unsigned char* usb_packet) {
    unsigned short checksum = 0;
    unsigned short checksum_hi = 0;
    
    for(int i = 0; i < 17; i++) {
        checksum += (short)usb_packet[i];
    }
    
    checksum_hi = (unsigned short)(checksum >> 8);
    checksum &= 0xff;
    checksum += checksum_hi;
    checksum_hi = (unsigned short)(checksum >> 8);
    checksum &= 0xff;
    checksum += checksum_hi;
    checksum = (~checksum + 1) & 0xff;
    return (unsigned char)checksum;
}
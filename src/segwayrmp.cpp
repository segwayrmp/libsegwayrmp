#include "segwayrmp.h"

using namespace segwayrmp;

SegwayRMP::SegwayRMP(std::string port) {
    this->port = port;
}

SegwayRMP::~SegwayRMP() {
    ;
}

inline void printHex(char * data, int length) {
    for(int i = 0;
    i < length;
    ++i) {
        printf("0x%.2X ", (unsigned)(unsigned char)data[i]);
    }
    printf("\n");
}

inline void printHexFromString(std::string str) {
    printHex(const_cast<char*>(str.c_str()), str.length());
}

void SegwayRMP::connect() {
    try {
        // Configure and open the serial port
        this->serial_port.setPort(this->port);
        this->serial_port.setBaudrate(921600);
        this->serial_port.setTimeoutMilliseconds(250);
        this->serial_port.open();
    } catch(std::exception &e) {
        throw(ConnectionFailedException(e.what()));
    }
    while(this->serial_port.read(1).find(0xF0) == std::string::npos)
        continue;
    this->serial_port.read(17);
}

bool SegwayRMP::go() {
    char usb_packet[18];
    int bytes_read = this->serial_port.read(usb_packet, 18);
    if(bytes_read != 18) {
        printf("Did not read 18 bytes!\n");
        return false;
    }
    printHex(usb_packet, bytes_read);
    return this->validatePacket(reinterpret_cast<unsigned char*>(usb_packet));
    // std::string usb_packet = this->serial_port.read(18);
    //     printHexFromString(usb_packet);
    //     return this->validatePacket(usb_packet);
}

bool SegwayRMP::validatePacket(unsigned char* usb_packet) {
    if(usb_packet[0] != 0xF0) {
        printf("First byte not 0xF0");
        return false;
    }
    
    unsigned char checksum = this->computeChecksum(usb_packet);
    if(usb_packet[17] != checksum) {
        printf("Checksum does not match.");
        return false;
    }
    
    return true;
}

unsigned char SegwayRMP::computeChecksum(unsigned char* usb_packet) {
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

// bool SegwayRMP::validatePacket(std::string usb_packet) {
//     // Look for USB Message Header (Start Byte)
//     if(usb_packet.substr(0,1).find(0xF0) == std::string::npos) {
//         printf("VP: First byte not 0xF0\n");
//         return false;
//     }
//     
//     char* data = (const_cast<char*>(usb_packet.c_str()));
//     
//     // Check the USB Message Checksum
//     unsigned short checksum = 0;
//     unsigned short checksum_hi = 0;
//     
//     for(int i = 0; i < 17; i++) {
//         checksum += (short)((unsigned)(unsigned char)data[i]);
//     }
//     
//     checksum_hi = (unsigned short)(checksum >> 8);
//     checksum &= 0xff;
//     checksum += checksum_hi;
//     checksum_hi = (unsigned short)(checksum >> 8);
//     checksum &= 0xff;
//     checksum += checksum_hi;
//     checksum = (~checksum + 1) & 0xff;
//     printf("0x%.2X ", checksum);
//     printf("\n");
//     if(((unsigned)(unsigned char)data[17]) != (unsigned char)checksum) {
//         printf("VP: Checksum does not check\n");
//         return false;
//     }
//     
//     return true;
// }











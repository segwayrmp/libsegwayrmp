#include "segwayrmp.h"

using namespace segwayrmp;

SegwayRMP::SegwayRMP(std::string port) {
    this->port = port;
    this->rmp_io = NULL;
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
    this->rmp_io = new SerialRMPIO();
    this->rmp_io->configure(this->port, 460800);
    this->rmp_io->connect();
}

bool SegwayRMP::go() {
    unsigned char packet[18];
    this->rmp_io->getPacket(packet);
    
    printHex(reinterpret_cast<char *>(packet), 18);
    
    this->validatePacket(packet);
}

bool SegwayRMP::validatePacket(unsigned char* usb_packet) {
    if(usb_packet[0] != 0xF0) {
        printf("First byte not 0xF0\n");
        return false;
    }
    
    unsigned char checksum = this->computeChecksum(usb_packet);
    if(usb_packet[17] != checksum) {
        printf("Checksum does not match.\n");
        return true;
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











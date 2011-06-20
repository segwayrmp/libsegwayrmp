#include <iostream>
#include <string.h>

#include "segwayrmp.h"

void handleSegwayStatus(segwayrmp::SegwayStatus &ss) {
    std::cout << ss.str() << std::endl << std::endl;
}

int run_segway(segwayrmp::InterfaceType interface_type, std::string port = "/dev/ttyUSB0") {
    segwayrmp::SegwayRMP rmp(interface_type);
    if (interface_type == segwayrmp::serial) {
        rmp.configureSerial(port);
    }
    rmp.setStatusCallback(handleSegwayStatus);
    rmp.connect();
    while(true) {
        rmp.move(0.3, 0);
        usleep(100000);
    }
}

void print_usage() {
    std::cout << "Usage: segwayrmp_example usb" << std::endl;
    std::cout << "       or" << std::endl;
    std::cout << "       segwayrmp_example serial <serial port>" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 0;
    }
    if (std::strcmp(argv[1], "serial") == 0) {
        if (argc < 3) {
            print_usage();
            return 0;
        }
        run_segway(segwayrmp::serial, std::string(argv[2]));
    } else {
        run_segway(segwayrmp::usb);
    }
}

#include "segwayrmp.h"

void handleSegwayStatus(segwayrmp::SegwayStatus &ss) {
    std::cout << ss.str() << std::endl << std::endl;
}

int main(void) {
    segwayrmp::SegwayRMP rmp(segwayrmp::serial);
    rmp.configureSerial("/dev/ttyUSB0");
    rmp.setStatusCallback(handleSegwayStatus);
    rmp.connect();
    while(true) {
        rmp.move(1.0, 0);
        usleep(100000);
    }
}

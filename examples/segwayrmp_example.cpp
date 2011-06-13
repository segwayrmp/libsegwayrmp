#include "segwayrmp.h"

void handleSegwayStatus(segwayrmp::SegwayStatus &ss) {
    ;
}

int main(void) {
    segwayrmp::SegwayRMP rmp(segwayrmp::serial);
    rmp.configure("/dev/ttyUSB0");
    rmp.setStatusCallback(handleSegwayStatus);
    rmp.connect();
    while(true) {
        rmp.move(1.0, 0);
        usleep(100000);
    }
}

#include "segwayrmp.h"

int main(void) {
    segwayrmp::SegwayRMP rmp(segwayrmp::serial);
    rmp.configure("/dev/ttyUSB0");
    rmp.connect();
    while(true) {
        rmp.move(0.1, 0);
        usleep(100000);
    }
}
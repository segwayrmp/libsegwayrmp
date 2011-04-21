#include "segwayrmp.h"

int main(void) {
    segwayrmp::SegwayRMP rmp(segwayrmp::serial);
    rmp.configure("/dev/ttyUSB0");
    rmp.connect();
    while(true) {
        rmp.move(0.1, 0);
        std::cout << "here" << std::endl;
        sleep(1);
    }
    // while(rmp.go()) {
    //     continue;
    // }
}
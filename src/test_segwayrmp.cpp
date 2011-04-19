#include "segwayrmp.h"

int main(void) {
    segwayrmp::SegwayRMP rmp("/dev/ttyUSB0");
    rmp.connect();
    while(rmp.go()) {
        continue;
    }
    while(rmp.go()) {
        continue;
    }
}
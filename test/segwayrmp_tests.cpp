#include "gtest/gtest.h"

// OMG this is so nasty...
#define private public
#define protected public
#include "segwayrmp.h"

using namespace segwayrmp;

namespace {

class PacketTests : public ::testing::Test {
protected:
    virtual void SetUp() {
        pck.channel = 0xAA;
        pck.id = 0x0400;
        pck.data[0] = 0x00;
        pck.data[1] = 0x00;
        pck.data[2] = 0x00;
        pck.data[3] = 0x00;
        pck.data[4] = 0x00;
        pck.data[5] = 0x00;
        pck.data[6] = 0x00;
        pck.data[7] = 0x00;
    }
    
    // virtual void TearDown() {
    //     
    // }
    
    SegwayRMP segway_rmp;
    Packet pck;
    SegwayStatus ss;
};

TEST_F(PacketTests, IgnoresChannelB) {
    pck.channel = 0xBB;
    pck.id = 0x0401;
    segway_rmp._parsePacket(pck, ss);
    
    ASSERT_FALSE(ss.touched);
}

TEST_F(PacketTests, IgnoresCommandRequest) {
    segway_rmp._parsePacket(pck,ss);
    
    ASSERT_FALSE(ss.touched);
}

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

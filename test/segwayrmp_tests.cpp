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

TEST_F(PacketTests, ParsesPitchRollCorrectly) {
    pck.id = 0x0401;
    pck.data[0] = 0xE8;
    pck.data[1] = 0xFF;
    pck.data[2] = 0x20;
    pck.data[3] = 0x00;
    pck.data[4] = 0x30;
    pck.data[5] = 0x00;
    pck.data[6] = 0x10;
    pck.data[7] = 0x00;
    
    segway_rmp._parsePacket(pck,ss);
    EXPECT_TRUE(ss.touched);
    EXPECT_NEAR(-3.0,ss.pitch,0.13);
    EXPECT_NEAR(4.1,ss.pitch_rate,0.13);
    EXPECT_NEAR(6.15,ss.roll,0.13);
    EXPECT_NEAR(2.05,ss.roll_rate,0.13);
}

TEST_F(PacketTests, ParsesWheelSpeedsYawRatesServoFramesCorrectly) {
    pck.id = 0x0402;
    pck.data[0] = 0xa0;
    pck.data[1] = 0x01;
    pck.data[2] = 0x3e;
    pck.data[3] = 0x03;
    pck.data[4] = 0x30;
    pck.data[5] = 0x00;
    pck.data[6] = 0x78;
    pck.data[7] = 0x00;
    
    segway_rmp._parsePacket(pck,ss);
    EXPECT_TRUE(ss.touched);
    EXPECT_NEAR(1.25,ss.left_wheel_speed,0.003);
    EXPECT_NEAR(2.5,ss.right_wheel_speed,0.003);
    EXPECT_NEAR(6.15,ss.yaw_rate,0.13);
    EXPECT_NEAR(1.2,ss.servo_frames,0.01);
}

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

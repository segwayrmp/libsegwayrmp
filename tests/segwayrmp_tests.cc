#include "gtest/gtest.h"

// OMG this is so nasty...
#define private public
#define protected public
#include "segwayrmp/segwayrmp.h"
#include "segwayrmp/impl/rmp_io.h"

using namespace segwayrmp;

namespace {

class PacketTests : public ::testing::Test {
protected:
    virtual void SetUp() {
        segway_rmp = new SegwayRMP(no_interface);
        ss = SegwayStatus::Ptr(new SegwayStatus);
        
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
    
    SegwayRMP *segway_rmp;
    Packet pck;
    SegwayStatus::Ptr ss;
};

TEST_F(PacketTests, IgnoresChannelB) {
    pck.channel = 0xBB;
    pck.id = 0x0401;
    segway_rmp->ParsePacket_(pck, ss);
    
    ASSERT_FALSE(ss->touched);
}

TEST_F(PacketTests, IgnoresCommandRequest) {
    segway_rmp->ParsePacket_(pck,ss);
    
    ASSERT_FALSE(ss->touched);
}

TEST_F(PacketTests, ParsesPitchRollCorrectly) {
    pck.id = 0x0401;
    pck.data[0] = 0xFF;
    pck.data[1] = 0xF9;
    pck.data[2] = 0x00;
    pck.data[3] = 0x0F;
    pck.data[4] = 0xFF;
    pck.data[5] = 0xFF;
    pck.data[6] = 0x00;
    pck.data[7] = 0x09;
    
    segway_rmp->ParsePacket_(pck,ss);
    EXPECT_TRUE(ss->touched);
    EXPECT_NEAR(-0.897,ss->pitch,0.128);
    EXPECT_NEAR(1.923,ss->pitch_rate,0.128);
    EXPECT_NEAR(-0.128,ss->roll,0.128);
    EXPECT_NEAR(1.154,ss->roll_rate,0.128);
}

TEST_F(PacketTests, ParsesWheelSpeedsYawRatesServoFramesCorrectly) {
    pck.id = 0x0402;
    pck.data[0] = 0x00;
    pck.data[1] = 0x0C;
    pck.data[2] = 0xFF;
    pck.data[3] = 0xF2;
    pck.data[4] = 0x00;
    pck.data[5] = 0x4B;
    pck.data[6] = 0x18;
    pck.data[7] = 0xD4;
    
    segway_rmp->ParsePacket_(pck,ss);
    EXPECT_TRUE(ss->touched);
    EXPECT_NEAR(0.0361,ss->left_wheel_speed,0.003);
    EXPECT_NEAR(-0.0422,ss->right_wheel_speed,0.003);
    EXPECT_NEAR(9.6154,ss->yaw_rate,0.128);
    EXPECT_NEAR(63.56,ss->servo_frames,0.01);
}

TEST_F(PacketTests, ParsesIntegratedWheelsCorrectly) {
    pck.id = 0x0403;
    pck.data[0] = 0xFE;
    pck.data[1] = 0x06;
    pck.data[2] = 0xFF;
    pck.data[3] = 0xFF;
    pck.data[4] = 0x00;
    pck.data[5] = 0x4F;
    pck.data[6] = 0x00;
    pck.data[7] = 0x00;
    
    segway_rmp->ParsePacket_(pck,ss);
    EXPECT_TRUE(ss->touched);
    EXPECT_NEAR(-0.0152341,ss->integrated_left_wheel_position,0.00003);
    EXPECT_NEAR(0.0023784,ss->integrated_right_wheel_position,0.00003);
}

TEST_F(PacketTests, ParsesIntegratedDistanceTurnCorrectly) {
    pck.id = 0x0404;
    pck.channel = 0xAA;
    // Packet id: 404, Packet Channel: AA,
    // Packet Data: 0xFC 0x19 0xFF 0xFF
    //              0xFF 0xAC 0xFF 0xFF
    pck.data[0] = 0xFC;
    pck.data[1] = 0x19;
    pck.data[2] = 0xFF;
    pck.data[3] = 0xFF;
    pck.data[4] = 0xFF;
    pck.data[5] = 0xAC;
    pck.data[6] = 0xFF;
    pck.data[7] = 0xFF;
    
    segway_rmp->ParsePacket_(pck,ss);
    EXPECT_TRUE(ss->touched);
    EXPECT_NEAR(-0.0300768,ss->integrated_forward_position,0.00003);
    EXPECT_NEAR(-0.268452,ss->integrated_turn_position,0.00001);
}

TEST_F(PacketTests, ParsesMotorTorqueCorrectly) {
    pck.id = 0x0405;
    // Packet id: 405, Packet Channel: AA, Packet Data: 0xFF 0xF9 0x00 0xA7 0x00 0x80 0x00 0x00
    // 1094
    pck.data[0] = 0xFF;
    pck.data[1] = 0xF9;
    pck.data[2] = 0x00;
    pck.data[3] = 0xA7;
    pck.data[4] = 0x00;
    pck.data[5] = 0x80;
    pck.data[6] = 0x00;
    pck.data[7] = 0x00;
    
    segway_rmp->ParsePacket_(pck,ss);
    EXPECT_TRUE(ss->touched);
    EXPECT_NEAR(-0.006399,ss->left_motor_torque,0.0009);
    EXPECT_NEAR(0.152651,ss->right_motor_torque,0.0009);
}

TEST_F(PacketTests, ParsesStatusesCorrectly) {
    pck.id = 0x0406;
    // Packet id: 406, Packet Channel: AA, Packet Data: 0x00 0x01 0x00 0x00 0x02 0x7A 0x01 0x38
    pck.data[0] = 0x00;
    pck.data[1] = 0x01;
    pck.data[2] = 0x00;
    pck.data[3] = 0x00;
    pck.data[4] = 0x02;
    pck.data[5] = 0x7A;
    pck.data[6] = 0x01;
    pck.data[7] = 0x38;
    
    segway_rmp->ParsePacket_(pck,ss);
    EXPECT_TRUE(ss->touched);
    // EXPECT_EQ(1,(int)ss.operational_mode);
    // EXPECT_EQ(0,(int)ss.controller_gain_schedule);
    EXPECT_NEAR(9.325,ss->ui_battery_voltage,0.25);
    EXPECT_NEAR(78.0,ss->powerbase_battery_voltage,0.25);
}

// TODO: Add tests for motor enabled/disabled and commanded velocity and yaw rate

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

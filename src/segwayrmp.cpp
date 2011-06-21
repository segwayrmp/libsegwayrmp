#include "segwayrmp.h"

inline void defaultSegwayStatusCallback(segwayrmp::SegwayStatus &segway_status) {
    std::cout << segway_status.str() << std::endl << std::endl;
}

inline void defaultDebugMsgCallback(const std::string &msg) {
    std::cerr << "SegwayRMP Debug: " << msg << std::endl;
}

inline void defaultInfoMsgCallback(const std::string &msg) {
    std::cerr << "SegwayRMP Info: " << msg << std::endl;
}

inline void defaultErrorMsgCallback(const std::string &msg) {
    std::cerr << "SegwayRMP Error: " << msg << std::endl;
}

inline void printHex(char * data, int length) {
    for(int i = 0; i < length; ++i) {
        printf("0x%.2X ", (unsigned)(unsigned char)data[i]);
    }
    printf("\n");
}

inline void printHexFromString(std::string str) {
    printHex(const_cast<char*>(str.c_str()), str.length());
}

using namespace segwayrmp;

SegwayStatus::SegwayStatus() {
    pitch, pitch_rate, roll, roll_rate, left_wheel_speed, right_wheel_speed,
    yaw_rate, servo_frames, integrated_left_wheel_position,
    integrated_right_wheel_position, integrated_forward_position,
    integrated_turn_position, left_motor_torque, right_motor_torque,
    ui_battery_voltage, powerbase_battery_voltage, commanded_velocity,
    commanded_yaw_rate = 0.0;
    operational_mode, controller_gain_schedule, motor_status = 0;
    touched = false;
}

std::string SegwayStatus::str() {
    std::stringstream ss;
    ss << "Segway Status: ";
    ss << "\nPitch: " << pitch << "\nPitch Rate: " << pitch_rate << "\nRoll: " << roll;
    ss << "\nRoll Rate: " << roll_rate << "\nLeft Wheel Speed: " << left_wheel_speed;
    ss << "\nRight Wheel Speed: " << right_wheel_speed << "\nYaw Rate: " << yaw_rate;
    ss << "\nServo Frames: " << servo_frames << "\nIntegrated Left Wheel Position: ";
    ss << integrated_left_wheel_position << "\nIntegrated Right Wheel Position: ";
    ss << integrated_right_wheel_position << "\nIntegrated Forward Displacement: ";
    ss << integrated_forward_position << "\nIntegrated Turn Position: ";
    ss << integrated_turn_position << "\nLeft Motor Torque: " << left_motor_torque;
    ss << "\nRight Motor Torque: " << right_motor_torque << "\nUI Battery Voltage: ";
    ss << ui_battery_voltage << "\nPowerbase Battery Voltage: " << powerbase_battery_voltage;
    ss << "\nOperational Mode: " << operational_mode << "\nController Gain Schedule: ";
    ss << controller_gain_schedule << "\nCommanded Velocity: " << commanded_velocity;
    ss << "\nCommanded Yaw Rate: " << commanded_yaw_rate << "\nMotor Status: ";
    if(motor_status)
        ss << "Motors Enabled";
    else
        ss << "E-Stopped";
    return ss.str();
}

SegwayRMP::SegwayRMP(InterfaceType interface_type) {
    this->interface_type = interface_type;
    if (this->interface_type == serial) {
        #ifdef SEGWAYRMP_SERIAL_SUPPORT
        this->rmp_io = new SerialRMPIO();
        #else
        throw(ConfigurationException("InterfaceType", "The segwayrmp library is not built with serial support, but serial was specified."));
        #endif
    } else if (this->interface_type == usb) {
        #ifdef SEGWAYRMP_FTD2XX_SUPPORT
        this->rmp_io = new FTD2XXRMPIO();
        #else
        throw(ConfigurationException("InterfaceType", "The segwayrmp library is not built with ftd2xx usb support, but usb was specified."));
        #endif
    } else if (this->interface_type == no_interface) {
        ;
    } else {
        throw(ConfigurationException("InterfaceType", "The specified interface type is not supported or invalid."));
    }
    this->callback_execution_thread_status = false;
    
    this->status_callback = defaultSegwayStatusCallback;
    this->debug = defaultDebugMsgCallback;
    this->info = defaultInfoMsgCallback;
    this->error = defaultErrorMsgCallback;
}

SegwayRMP::~SegwayRMP() {
    if(this->continuous)
        this->stopContinuousRead();
    delete this->rmp_io;
}

void SegwayRMP::configureSerial(std::string port, int baudrate) {
    #ifdef SEGWAYRMP_SERIAL_SUPPORT
    if(this->interface_type == serial) {
        SerialRMPIO * serial_rmp = (SerialRMPIO*)(this->rmp_io);
        serial_rmp->configure(port, baudrate);
        serial_rmp = NULL;
    } else {
        throw(ConfigurationException("configureSerial", "Cannot configure serial when the InterfaceType is not serial."));
    }
    #else
    throw(ConfigurationException("configureSerial", "The segwayrmp library is not build with serial support, not implemented."));
    #endif
}

void SegwayRMP::configureUSBBySerial(std::string serial_number, int baudrate) {
    #ifdef SEGWAYRMP_FTD2XX_SUPPORT
    if(this->interface_type == usb) {
        FTD2XXRMPIO * ftd2xx_rmp = (FTD2XXRMPIO*)(this->rmp_io);
        ftd2xx_rmp->configureUSBBySerial(serial_number, baudrate);
        ftd2xx_rmp = NULL;
    } else {
        throw(ConfigurationException("configureUSBBySerial", "Cannot configure ftd2xx usb when the InterfaceType is not usb."));
    }
    #else
    throw(ConfigurationException("configureUSBBySerial", "The segwayrmp library is not build with ftd2xx usb support, not implemented."));
    #endif
}

void SegwayRMP::configureUSBByDescription(std::string description, int baudrate) {
    #ifdef SEGWAYRMP_FTD2XX_SUPPORT
    if(this->interface_type == usb) {
        FTD2XXRMPIO * ftd2xx_rmp = (FTD2XXRMPIO*)(this->rmp_io);
        ftd2xx_rmp->configureUSBByDescription(description, baudrate);
        ftd2xx_rmp = NULL;
    } else {
        throw(ConfigurationException("configureUSBByDescription", "Cannot configure ftd2xx usb when the InterfaceType is not usb."));
    }
    #else
    throw(ConfigurationException("configureUSBByDescription", "The segwayrmp library is not build with ftd2xx usb support, not implemented."));
    #endif
}

void SegwayRMP::configureUSBByIndex(int device_index, int baudrate) {
    #ifdef SEGWAYRMP_FTD2XX_SUPPORT
    if(this->interface_type == usb) {
        FTD2XXRMPIO * ftd2xx_rmp = (FTD2XXRMPIO*)(this->rmp_io);
        ftd2xx_rmp->configureUSBByIndex(device_index, baudrate);
        ftd2xx_rmp = NULL;
    } else {
        throw(ConfigurationException("configureUSBByIndex", "Cannot configure ftd2xx usb when the InterfaceType is not usb."));
    }
    #else
    throw(ConfigurationException("configureUSBByIndex", "The segwayrmp library is not build with ftd2xx usb support, not implemented."));
    #endif
}

void SegwayRMP::connect(OperationalMode operational_mode, ControllerGainSchedule controller_gain_schedule) {
    // Connect to the interface
    this->rmp_io->connect();
    
    this->connected = true;
    
    // Kick off the read thread
    this->startContinuousRead();
    
    // Lock or unlock balancing depending on the mode.
    if(operational_mode == balanced) {
        this->setBalanceModeLocking(false);
    } else {
        this->setBalanceModeLocking(true);
    }
    
    // Set the operational mode and controller gain schedule
    this->setOperationalMode(operational_mode);
    this->setControllerGainSchedule(controller_gain_schedule);
    
    // Set the scale factor to 1.0 by default
    this->setMaxVelocityScaleFactor();

    // Reset all the integrators
    this->resetAllIntegrators();
}

void SegwayRMP::move(float linear_velocity, float angular_velocity) {
    // Ensure we are connected
    if(!this->connected)
        throw(MoveFailedException("Not Connected."));
    try {
        short int lv = (short int)(linear_velocity*MPS_TO_COUNTS);
        short int av = (short int)(angular_velocity*1024.0);
        
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = (unsigned char)((lv & 0xFF00) >> 8);
        packet.data[1] = (unsigned char)(lv & 0x00FF);
        packet.data[2] = (unsigned char)((av & 0xFF00) >> 8);
        packet.data[3] = (unsigned char)(av & 0x00FF);
        packet.data[4] = 0x00;
        packet.data[5] = 0x00;
        packet.data[6] = 0x00;
        packet.data[7] = 0x00;
        
        this->rmp_io->sendPacket(packet);
    } catch(std::exception &e) {
        throw(MoveFailedException(e.what()));
    }
}

void SegwayRMP::setOperationalMode(OperationalMode operational_mode) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Operational Mode", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x10;
        packet.data[6] = 0x00;
        packet.data[7] = (unsigned char)operational_mode;
        
        this->rmp_io->sendPacket(packet);
        
        while(this->segway_status.operational_mode != operational_mode) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(10)); // Check again in 10 ms
        }
    } catch(std::exception &e) {
        throw(ConfigurationException("Operational Mode", e.what()));
    }
}

void SegwayRMP::setControllerGainSchedule(ControllerGainSchedule controller_gain_schedule) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Controller Gain Schedule", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x0D;
        packet.data[6] = 0x00;
        packet.data[7] = (unsigned char)controller_gain_schedule;
        
        this->rmp_io->sendPacket(packet);
        
        // while(this->segway_status.controller_gain_schedule != controller_gain_schedule) {
        //     boost::this_thread::sleep(boost::posix_time::milliseconds(10)); // Check again in 10 ms
        // }
    } catch(std::exception &e) {
        throw(ConfigurationException("Controller Gain Schedule", e.what()));
    }
}

void SegwayRMP::setBalanceModeLocking(bool state) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Balance Mode Lock", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x0F;
        packet.data[6] = 0x00;
        if(state)
            packet.data[7] = 0x01;
        else
            packet.data[7] = 0x00;
        
        this->rmp_io->sendPacket(packet);
    } catch(std::exception &e) {
        throw(ConfigurationException("Balance Mode Lock", e.what()));
    }
}

void SegwayRMP::resetAllIntegrators() {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Integrators", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x32;
        packet.data[6] = 0x00;
        packet.data[7] = 0x01;
        
        this->rmp_io->sendPacket(packet);
        
        packet.data[7] = 0x02;
        
        this->rmp_io->sendPacket(packet);
        
        packet.data[7] = 0x04;
        
        this->rmp_io->sendPacket(packet);
        
        packet.data[7] = 0x08;
        
        this->rmp_io->sendPacket(packet);
        
    } catch(std::exception &e) {
        throw(ConfigurationException("Integrators", e.what()));
    }
}

void SegwayRMP::setMaxVelocityScaleFactor(double scalar) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Max Velocity Scale Factor", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x0A;
        packet.data[6] = 0x00;
        
        if (scalar < 0.0)
            scalar = 0.0;
        if (scalar > 1.0)
            scalar = 1.0;
        scalar *= 16.0;
        scalar = floor(scalar);

        short int scalar_int = (short int)scalar;

        packet.data[7] = (unsigned char)(scalar_int & 0x00FF);
        
        this->rmp_io->sendPacket(packet);
    } catch(std::exception &e) {
        throw(ConfigurationException("Max Velocity Scale Factor", e.what()));
    }
}

void SegwayRMP::setStatusCallback(void (*status_callback)(SegwayStatus &segway_status)) {
    this->status_callback = status_callback;
}

void SegwayRMP::setDebugMsgCallback(void (*debug_callback)(const std::string &msg)) {
    this->debug = f;
}

void SegwayRMP::setInfoMsgCallback(void (*info_callback)(const std::string &msg)) {
    this->info = f;
}

void SegwayRMP::setErrorMsgCallback(void (*error_callback)(const std::string &msg)) {
    this->error = f;
}

void SegwayRMP::readContinuously() {
    Packet packet;
    while(this->continuous) {
        try {
            this->rmp_io->getPacket(packet);
            this->parsePacket(packet);
        } catch(PacketRetrievalException &e) {
            if(e.error_number() == 2) // Failed Checksum
                this->error("Checksum mismatch...");
            else if(e.error_number() == 3) // No packet received
                this->error("No data from Segway...");
            else
                throw(e);
        }
    }
}

void SegwayRMP::startContinuousRead() {
    this->continuous = true;
    this->continuous_read_thread = boost::thread(&SegwayRMP::readContinuously, this);
}

void SegwayRMP::stopContinuousRead() {
    this->continuous = false;
    this->continuous_read_thread.join();
}

inline short int getShortInt(unsigned char high, unsigned char low) {
    return (short int)(((unsigned short int)high<<8)|(unsigned short int)low);
}

inline int getInt(unsigned char lhigh, unsigned char llow, unsigned char hhigh, unsigned char hlow) {
    int result = 0;
    char data[4] = {llow,lhigh,hlow,hhigh};
    memcpy(&result, data, 4);
    return result;
}

bool SegwayRMP::_parsePacket(Packet &packet, SegwayStatus &_segway_status) {
    bool status_updated = false;
    if(packet.channel == 0xBB) // Ignore Channel B messages
        return status_updated;
    
    // This section comes largerly from the Segway example code
    switch (packet.id) {
        case 0x0400: // COMMAND REQUEST
            break;
        case 0x0401:
            _segway_status.pitch      = getShortInt(packet.data[0], packet.data[1])/7.8;
            _segway_status.pitch_rate = getShortInt(packet.data[2], packet.data[3])/7.8;
            _segway_status.roll       = getShortInt(packet.data[4], packet.data[5])/7.8;
            _segway_status.roll_rate  = getShortInt(packet.data[6], packet.data[7])/7.8;
            _segway_status.touched = true;
            break;
        case 0x0402:
            _segway_status.left_wheel_speed  = getShortInt(packet.data[0], packet.data[1])/332.0;
            _segway_status.right_wheel_speed = getShortInt(packet.data[2], packet.data[3])/332.0;
            _segway_status.yaw_rate          = getShortInt(packet.data[4], packet.data[5])/7.8;
            _segway_status.servo_frames      = ((((short unsigned int)packet.data[6])<<8) | 
                                                 ((short unsigned int)packet.data[7]))*0.01;
            _segway_status.touched = true;
            break;
        case 0x0403:
            _segway_status.integrated_left_wheel_position  = 
                                            getInt(packet.data[0], packet.data[1], packet.data[2], packet.data[3])/33215.0;
            _segway_status.integrated_right_wheel_position = 
                                            getInt(packet.data[4], packet.data[5], packet.data[6], packet.data[7])/33215.0;
            _segway_status.touched = true;
            break;
        case 0x0404:
            _segway_status.integrated_forward_position = 
                                            getInt(packet.data[0], packet.data[1], packet.data[2], packet.data[3])/33215.0;
            _segway_status.integrated_turn_position    = 
                                            getInt(packet.data[4], packet.data[5], packet.data[6], packet.data[7])/112644.0;
            _segway_status.touched = true;
            break;
        case 0x0405:
            _segway_status.left_motor_torque  = getShortInt(packet.data[0], packet.data[1])/1094.0;
            _segway_status.right_motor_torque = getShortInt(packet.data[2], packet.data[3])/1094.0;
            _segway_status.touched = true;
            break;
        case 0x0406:
            _segway_status.operational_mode          = OperationalMode(getShortInt(packet.data[0], packet.data[1]));
            _segway_status.controller_gain_schedule  = ControllerGainSchedule(getShortInt(packet.data[2], packet.data[3]));
            _segway_status.ui_battery_voltage        = ((((short unsigned int)packet.data[4])<<8) | 
                                                         ((short unsigned int)packet.data[5]))*0.0125 + 1.4;
            _segway_status.powerbase_battery_voltage = ((((short unsigned int)packet.data[6])<<8) | 
                                                         ((short unsigned int)packet.data[7]))/4.0;
            _segway_status.touched = true;
            break;
        case 0x0407:
            _segway_status.commanded_velocity = (float)getShortInt(packet.data[0], packet.data[1])/MPS_TO_COUNTS;
            _segway_status.commanded_yaw_rate = (float)getShortInt(packet.data[2], packet.data[3])/1024.0;
            status_updated = true;
            _segway_status.touched = true;
            break;
        case 0x0680:
            if(packet.data[3] == 0x80) // Motors Enabled
                _segway_status.motor_status = 1;
            else // E-Stopped
                _segway_status.motor_status = 0;
            _segway_status.touched = true;
            break;
        default: // Unknown/Unhandled Message
            break;
    };
    return status_updated;
}

void SegwayRMP::parsePacket(Packet &packet) {
    bool status_updated = false;
    
    //printf("Packet id: %X, Packet Channel: %X, Packet Data: ", packet.id, packet.channel);
    //printHex(reinterpret_cast<char *>(packet.data), 8);
    
    status_updated = this->_parsePacket(packet, this->segway_status);
    
    // Messages come in order 0x0400, 0x0401, ... 0x0407 so a complete "cycle" of information has been sent every time we get an 0x0407
    if(status_updated) {
        if(this->callback_execution_thread_status) {
            this->error("Callback Falling behind, skipping packet report...");
        } else {
            this->callback_execution_thread.join(); // Should be instant
            this->callback_execution_thread = boost::thread(&SegwayRMP::executeCallback, this, this->segway_status);
        }
    }
}

void SegwayRMP::executeCallback(SegwayStatus segway_status) {
    this->callback_execution_thread_status = true;
    this->status_callback(this->segway_status);
    this->callback_execution_thread_status = false;
}

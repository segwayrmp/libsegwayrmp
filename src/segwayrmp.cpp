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

// This is from ROS's walltime function
// http://www.ros.org/doc/api/rostime/html/time_8cpp_source.html
inline segwayrmp::SegwayTime defaultTimestampCallback()
#ifndef WIN32
    throw(segwayrmp::NoHighPerformanceTimersException)
#endif
  {
    segwayrmp::SegwayTime st;
#ifndef WIN32
#if HAS_CLOCK_GETTIME
    timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    st.sec  = start.tv_sec;
    st.nsec = start.tv_nsec;
#else
    struct timeval timeofday;
    gettimeofday(&timeofday,NULL);
    st.sec  = timeofday.tv_sec;
    st.nsec = timeofday.tv_usec * 1000;
#endif
#else
    // Win32 implementation
    // unless I've missed something obvious, the only way to get high-precision
    // time on Windows is via the QueryPerformanceCounter() call. However,
    // this is somewhat problematic in Windows XP on some processors, especially
    // AMD, because the Windows implementation can freak out when the CPU clocks
    // down to save power. Time can jump or even go backwards. Microsoft has
    // fixed this bug for most systems now, but it can still show up if you have
    // not installed the latest CPU drivers (an oxymoron). They fixed all these
    // problems in Windows Vista, and this API is by far the most accurate that
    // I know of in Windows, so I'll use it here despite all these caveats
    static LARGE_INTEGER cpu_freq, init_cpu_time;
    uint32_t start_sec = 0;
    uint32_t start_nsec = 0;
    if ( ( start_sec == 0 ) && ( start_nsec == 0 ) )
      {
        QueryPerformanceFrequency(&cpu_freq);
        if (cpu_freq.QuadPart == 0) {
          throw segwayrmp::NoHighPerformanceTimersException();
        }
        QueryPerformanceCounter(&init_cpu_time);
        // compute an offset from the Epoch using the lower-performance timer API
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        LARGE_INTEGER start_li;
        start_li.LowPart = ft.dwLowDateTime;
        start_li.HighPart = ft.dwHighDateTime;
        // why did they choose 1601 as the time zero, instead of 1970?
        // there were no outstanding hard rock bands in 1601.
#ifdef _MSC_VER
        start_li.QuadPart -= 116444736000000000Ui64;
#else
        start_li.QuadPart -= 116444736000000000ULL;
#endif
        start_sec = (uint32_t)(start_li.QuadPart / 10000000); // 100-ns units. odd.
        start_nsec = (start_li.LowPart % 10000000) * 100;
      }
    LARGE_INTEGER cur_time;
    QueryPerformanceCounter(&cur_time);
    LARGE_INTEGER delta_cpu_time;
    delta_cpu_time.QuadPart = cur_time.QuadPart - init_cpu_time.QuadPart;
    // todo: how to handle cpu clock drift. not sure it's a big deal for us.
    // also, think about clock wraparound. seems extremely unlikey, but possible
    double d_delta_cpu_time = delta_cpu_time.QuadPart / (double) cpu_freq.QuadPart;
    uint32_t delta_sec = (uint32_t) floor(d_delta_cpu_time);
    uint32_t delta_nsec = (uint32_t) boost::math::round((d_delta_cpu_time-delta_sec) * 1e9);
    
    int64_t sec_sum  = (int64_t)start_sec  + (int64_t)delta_sec;
    int64_t nsec_sum = (int64_t)start_nsec + (int64_t)delta_nsec;
    
    // Throws an exception if we go out of 32-bit range
    normalizeSecNSecUnsigned(sec_sum, nsec_sum);
    
    st.sec = sec_sum;
    st.nsec = nsec_sum;
#endif
    return st;
}

inline void defaultExceptionCallback(const std::exception &error) {
    std::cerr << "SegwayRMP Unhandled Exception: " << error.what() << std::endl;
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
    timestamp = SegwayTime(0,0);
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
    ss << "\n  Seconds: " << timestamp.sec;
    ss << "\n  Nanoseconds: " << timestamp.nsec;
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

SegwayRMP::SegwayRMP(InterfaceType interface_type, SegwayRMPType segway_rmp_type) {
    this->interface_type = interface_type;
    this->segway_type = segway_rmp_type;
    if (this->interface_type == serial) {
        #if SEGWAYRMP_SERIAL_SUPPORT
        this->rmp_io = new SerialRMPIO();
        #else
        throw(ConfigurationException("InterfaceType", "The segwayrmp library is not built with serial support, but serial was specified."));
        #endif
    } else if (this->interface_type == usb) {
        #if SEGWAYRMP_FTD2XX_SUPPORT
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
    this->get_time = defaultTimestampCallback;
    this->handle_exception = defaultExceptionCallback;
    
    this->configureSegwayType();
}

SegwayRMP::~SegwayRMP() {
    if(this->continuous)
        this->stopContinuousRead();
    delete this->rmp_io;
}

void SegwayRMP::configureSerial(std::string port, int baudrate) {
    #if SEGWAYRMP_SERIAL_SUPPORT
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
    #if SEGWAYRMP_FTD2XX_SUPPORT
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
    #if SEGWAYRMP_FTD2XX_SUPPORT
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
    #if SEGWAYRMP_FTD2XX_SUPPORT
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
        short int lv = (short int)(linear_velocity*this->mps_to_counts);
        short int av = (short int)(angular_velocity*this->dps_to_counts);
        
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
        
        // while(this->segway_status.operational_mode != operational_mode) {
        //     boost::this_thread::sleep(boost::posix_time::milliseconds(10)); // Check again in 10 ms
        // }
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

void SegwayRMP::setMaxAccelerationScaleFactor(double scalar) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Max Acceleration Scale Factor", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x0B;
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
        throw(ConfigurationException("Max Acceleration Scale Factor", e.what()));
    }
}

void SegwayRMP::setMaxTurnScaleFactor(double scalar) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Max Turn Scale Factor", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x0C;
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
        throw(ConfigurationException("Max Turn Scale Factor", e.what()));
    }
}

void SegwayRMP::setCurrentLimitScaleFactor(double scalar) {
    // Ensure we are connected
    if(!this->connected)
        throw(ConfigurationException("Current Limit Scale Factor", "Not Connected."));
    try {
        Packet packet;
        
        packet.id = 0x0413;
        
        packet.data[0] = 0x00;
        packet.data[1] = 0x00;
        packet.data[2] = 0x00;
        packet.data[3] = 0x00;
        packet.data[4] = 0x00;
        packet.data[5] = 0x0E;
        packet.data[6] = 0x00;
        
        if (scalar < 0.0)
            scalar = 0.0;
        if (scalar > 1.0)
            scalar = 1.0;
        scalar *= 256.0;
        scalar = floor(scalar);
        
        short int scalar_int = (short int)scalar;
        
        packet.data[7] = (unsigned char)(scalar_int & 0x00FF);
        
        this->rmp_io->sendPacket(packet);
    } catch(std::exception &e) {
        throw(ConfigurationException("Current Limit Scale Factor", e.what()));
    }
}

void SegwayRMP::setStatusCallback(SegwayStatusCallback status_callback) {
    this->status_callback = status_callback;
}

void SegwayRMP::setDebugMsgCallback(DebugMsgCallback debug_callback) {
    this->debug = debug_callback;
}

void SegwayRMP::setInfoMsgCallback(InfoMsgCallback info_callback) {
    this->info = info_callback;
}

void SegwayRMP::setErrorMsgCallback(ErrorMsgCallback error_callback) {
    this->error = error_callback;
}

void SegwayRMP::setTimestampCallback(TimestampCallback timestamp_callback) {
    this->get_time = timestamp_callback;
}

void SegwayRMP::setExceptionCallback(ExceptionCallback exception_callback) {
  this->handle_exception = exception_callback;
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
                this->handle_exception(e);
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

void SegwayRMP::configureSegwayType() {
    if (this->segway_type == rmp200 || this->segway_type == rmp400) {
        this->dps_to_counts = 7.8;
        this->mps_to_counts = 332.0;
        this->meters_to_counts = 33215.0;
        this->rev_to_counts = 112644.0;
        this->torque_to_counts = 1094.0;
    } else if (this->segway_type == rmp50 || this->segway_type == rmp100) {
        this->dps_to_counts = 7.8;
        this->mps_to_counts = 401.0;
        this->meters_to_counts = 40181.0;
        this->rev_to_counts = 117031.0;
        this->torque_to_counts = 1463.0;
    } else {
        throw(ConfigurationException("SegwayType", "Inavlid Type"));
    }
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
            // This is the first packet of a msg series, timestamp here.
            _segway_status.timestamp  = this->get_time();
            break;
        case 0x0401:
            _segway_status.pitch      = getShortInt(packet.data[0], packet.data[1])/this->dps_to_counts;
            _segway_status.pitch_rate = getShortInt(packet.data[2], packet.data[3])/this->dps_to_counts;
            _segway_status.roll       = getShortInt(packet.data[4], packet.data[5])/this->dps_to_counts;
            _segway_status.roll_rate  = getShortInt(packet.data[6], packet.data[7])/this->dps_to_counts;
            _segway_status.touched = true;
            break;
        case 0x0402:
            _segway_status.left_wheel_speed  = getShortInt(packet.data[0], packet.data[1])/this->mps_to_counts;
            _segway_status.right_wheel_speed = getShortInt(packet.data[2], packet.data[3])/this->mps_to_counts;
            _segway_status.yaw_rate          = getShortInt(packet.data[4], packet.data[5])/this->dps_to_counts;
            _segway_status.servo_frames      = ((((short unsigned int)packet.data[6])<<8) | 
                                                 ((short unsigned int)packet.data[7]))*0.01;
            _segway_status.touched = true;
            break;
        case 0x0403:
            _segway_status.integrated_left_wheel_position  = 
                                            getInt(packet.data[0], packet.data[1], packet.data[2], packet.data[3])/this->meters_to_counts;
            _segway_status.integrated_right_wheel_position = 
                                            getInt(packet.data[4], packet.data[5], packet.data[6], packet.data[7])/this->meters_to_counts;
            _segway_status.touched = true;
            break;
        case 0x0404:
            _segway_status.integrated_forward_position = 
                                            getInt(packet.data[0], packet.data[1], packet.data[2], packet.data[3])/this->meters_to_counts;
            _segway_status.integrated_turn_position    = 
                                            getInt(packet.data[4], packet.data[5], packet.data[6], packet.data[7])/this->rev_to_counts;
            _segway_status.integrated_turn_position *= 360.0; // convert from revolutions to degrees
            _segway_status.touched = true;
            break;
        case 0x0405:
            _segway_status.left_motor_torque  = getShortInt(packet.data[0], packet.data[1])/this->torque_to_counts;
            _segway_status.right_motor_torque = getShortInt(packet.data[2], packet.data[3])/this->torque_to_counts;
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
            _segway_status.commanded_velocity = (float)getShortInt(packet.data[0], packet.data[1])/this->mps_to_counts;
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
                return;
        }
        this->callback_execution_thread.join(); // Should be instant
        this->callback_execution_thread = boost::thread(&SegwayRMP::executeCallback, this, this->segway_status);
    }
}

void SegwayRMP::executeCallback(SegwayStatus this_segway_status) {
    this->callback_execution_thread_status = true;
    try {
      this->status_callback(this_segway_status);
    } catch (std::exception &e) {
      this->callback_execution_thread_status = false;
      this->handle_exception(e);
    }
    this->callback_execution_thread_status = false;
}

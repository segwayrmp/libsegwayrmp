#include <algorithm>
#include <iostream>

#include <segwayrmp/segwayrmp.h>
#include <segwayrmp/impl/rmp_io.h>
#include <segwayrmp/impl/rmp_ftd2xx.h>
#if defined(SEGWAYRMP_USE_SERIAL)
# include <segwayrmp/impl/rmp_serial.h>
#endif

#if !HAS_CLOCK_GETTIME
# include <sys/time.h>
#endif

inline void
defaultSegwayStatusCallback(segwayrmp::SegwayStatus::Ptr &segway_status)
{
  std::cout << "Segway Status:" << std::endl << std::endl
            << segway_status->str() << std::endl << std::endl;
}

inline void defaultDebugMsgCallback(const std::string &msg)
{
  std::cerr << "SegwayRMP Debug: " << msg << std::endl;
}

inline void defaultInfoMsgCallback(const std::string &msg)
{
  std::cerr << "SegwayRMP Info: " << msg << std::endl;
}

inline void defaultErrorMsgCallback(const std::string &msg)
{
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
# if HAS_CLOCK_GETTIME
  timespec start;
  clock_gettime(CLOCK_REALTIME, &start);
  st.sec  = start.tv_sec;
  st.nsec = start.tv_nsec;
# else
  struct timeval timeofday;
  gettimeofday(&timeofday, NULL);
  st.sec  = timeofday.tv_sec;
  st.nsec = timeofday.tv_usec * 1000;
# endif
#else
  // Win32 implementation
  // unless I've missed something obvious, the only way to get high-precision
  // time on Windows is via the QueryPerformanceCounter() call. However,
  // this is somewhat problematic in Windows XP on some processors,
  // especially AMD, because the Windows implementation can freak out when
  // the CPU clocks down to save power. Time can jump or even go backwards.
  // Microsoft has fixed this bug for most systems now, but it can still
  // show up if you have not installed the latest CPU drivers (an oxymoron).
  // They fixed all these problems in Windows Vista, and this API is by far
  // the most accurate that I know of in Windows, so I'll use it here
  // despite all these caveats
  static LARGE_INTEGER cpu_freq, init_cpu_time;
  uint32_t start_sec = 0;
  uint32_t start_nsec = 0;
  if ( ( start_sec == 0 ) && ( start_nsec == 0 ) ) {
    QueryPerformanceFrequency(&cpu_freq);
    if (cpu_freq.QuadPart == 0) {
      throw segwayrmp::NoHighPerformanceTimersException();
    }
    QueryPerformanceCounter(&init_cpu_time);
    // compute an offset from the Epoch using the lower-performance
    // timer API
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    LARGE_INTEGER start_li;
    start_li.LowPart = ft.dwLowDateTime;
    start_li.HighPart = ft.dwHighDateTime;
    // why did they choose 1601 as the time zero, instead of 1970?
    // there were no outstanding hard rock bands in 1601.
# ifdef _MSC_VER
    start_li.QuadPart -= 116444736000000000Ui64;
# else
    start_li.QuadPart -= 116444736000000000ULL;
# endif
    // 100-ns units. odd.
    start_sec = (uint32_t)(start_li.QuadPart / 10000000);
    start_nsec = (start_li.LowPart % 10000000) * 100;
  }
  LARGE_INTEGER cur_time;
  QueryPerformanceCounter(&cur_time);
  LARGE_INTEGER delta_cpu_time;
  delta_cpu_time.QuadPart = cur_time.QuadPart - init_cpu_time.QuadPart;
  // todo: how to handle cpu clock drift. not sure it's a big deal for us.
  // also, think about clock wraparound. seems extremely unlikey,
  // but possible
  double d_delta_cpu_time =
    delta_cpu_time.QuadPart / (double) cpu_freq.QuadPart;
  uint32_t delta_sec = (uint32_t) floor(d_delta_cpu_time);
  uint32_t delta_nsec =
    (uint32_t) boost::math::round((d_delta_cpu_time - delta_sec) * 1e9);

  int64_t sec_sum  = (int64_t)start_sec  + (int64_t)delta_sec;
  int64_t nsec_sum = (int64_t)start_nsec + (int64_t)delta_nsec;

  // Throws an exception if we go out of 32-bit range
  normalizeSecNSecUnsigned(sec_sum, nsec_sum);

  st.sec = sec_sum;
  st.nsec = nsec_sum;
#endif
  return st;
}

inline void defaultExceptionCallback(const std::exception &error)
{
  std::cerr << "SegwayRMP Unhandled Exception: " << error.what()
            << std::endl;
}

inline void printHex(char *data, int length)
{
  for (int i = 0; i < length; ++i) {
    printf("0x%.2X ", (unsigned)(unsigned char)data[i]);
  }
  printf("\n");
}

inline void printHexFromString(std::string str)
{
  printHex(const_cast<char *>(str.c_str()), str.length());
}

using namespace segwayrmp;

SegwayStatus::SegwayStatus()
  : timestamp(SegwayTime(0, 0)), pitch(0.0f), pitch_rate(0.0f), roll(0.0f),
    roll_rate(0.0f), left_wheel_speed(0.0f), right_wheel_speed(0.0f),
    yaw_rate(0.0f), servo_frames(0.0f), integrated_left_wheel_position(0.0f),
    integrated_right_wheel_position(0.0f), integrated_forward_position(0.0f),
    integrated_turn_position(0.0f), left_motor_torque(0.0f),
    right_motor_torque(0.0f), ui_battery_voltage(0.0f),
    powerbase_battery_voltage(0.0f), commanded_velocity(0.0f),
    commanded_yaw_rate(0.0f), operational_mode(disabled),
    controller_gain_schedule(light), motor_status(0), touched(false)
{}

std::string SegwayStatus::str()
{
  std::stringstream ss;
  ss << "Time Stamp: "
     << "\n  Seconds: " << timestamp.sec
     << "\n  Nanoseconds: " << timestamp.nsec
     << "\nPitch: " << pitch << "\nPitch Rate: " << pitch_rate
     << "\nRoll: " << roll  << "\nRoll Rate: " << roll_rate
     << "\nLeft Wheel Speed: " << left_wheel_speed
     << "\nRight Wheel Speed: " << right_wheel_speed
     << "\nYaw Rate: " << yaw_rate
     << "\nServo Frames: " << servo_frames
     << "\nIntegrated Left Wheel Position: " << integrated_left_wheel_position
     << "\nIntegrated Right Wheel Position: "
     << integrated_right_wheel_position
     << "\nIntegrated Forward Displacement: " << integrated_forward_position
     << "\nIntegrated Turn Position: " << integrated_turn_position
     << "\nLeft Motor Torque: " << left_motor_torque
     << "\nRight Motor Torque: " << right_motor_torque
     << "\nUI Battery Voltage: " << ui_battery_voltage
     << "\nPowerbase Battery Voltage: " << powerbase_battery_voltage
     << "\nOperational Mode: ";
  switch (operational_mode) {
    case disabled:
      ss << "disabled";
      break;
    case tractor:
      ss << "tractor";
      break;
    case balanced:
      ss << "balanced";
      break;
    case power_down:
      ss << "power down";
      break;
    default:
      ss << "unknown";
      break;
  }
  ss << "\nController Gain Schedule: ";
  switch (controller_gain_schedule) {
    case light:
      ss << "light";
      break;
    case tall:
      ss << "tall";
      break;
    case heavy:
      ss << "heavy";
      break;
    default:
      ss << "unknown";
      break;
  }
  ss << "\nCommanded Velocity: " << commanded_velocity
     << "\nCommanded Yaw Rate: " << commanded_yaw_rate
     << "\nMotor Status: ";
  if (motor_status) {
    ss << "Motors Enabled";
  } else {
    ss << "E-Stopped";
  }
  return ss.str();
}

SegwayRMP::SegwayRMP(InterfaceType interface_type,
                     SegwayRMPType segway_rmp_type)
: interface_type_(no_interface), segway_rmp_type_(segway_rmp_type),
  connected_(false),
  continuously_reading_(false),
  status_callback_(defaultSegwayStatusCallback),
  get_time_(defaultTimestampCallback),
  debug_(defaultDebugMsgCallback),
  info_(defaultInfoMsgCallback),
  error_(defaultErrorMsgCallback),
  handle_exception_(defaultExceptionCallback)
{
  this->segway_status_ = SegwayStatus::Ptr(new SegwayStatus());
  this->interface_type_ = interface_type;
  switch (interface_type) {
    case can:
      RMP_THROW_MSG(ConfigurationException, "CAN is not supported currently");
      break;
    case usb:
      this->rmp_io_ = new FTD2XXRMPIO();
      break;
    case serial:
#if defined(SEGWAYRMP_USE_SERIAL)
      this->rmp_io_ = new SerialRMPIO();
#else
      RMP_THROW_MSG(ConfigurationException, "Library is not built with Serial "
        "support");
#endif
      break;
    case ethernet:
      RMP_THROW_MSG(ConfigurationException, "Ethernet is not currently "
        "supported");
      break;
    case no_interface:
      // do nothing
      break;
    default:
      RMP_THROW_MSG(ConfigurationException, "Invalid InterfaceType specified");
      break;
  }

  // Set the constants based on the segway type
  this->SetConstantsBySegwayType_(this->segway_rmp_type_);
}

SegwayRMP::~SegwayRMP()
{
  if (this->continuously_reading_) {
    this->StopReadingContinuously_();
  }
  if (this->interface_type_ == serial) {
#if defined(SEGWAYRMP_USE_SERIAL)
    SerialRMPIO * ptr = (SerialRMPIO *)(this->rmp_io_);
    delete ptr;
#endif
  }
  if (this->interface_type_ == usb) {
    FTD2XXRMPIO * ptr = (FTD2XXRMPIO *)(this->rmp_io_);
    delete ptr;
  }
}

void SegwayRMP::configureSerial(std::string port, int baudrate)
{
#if SEGWAYRMP_USE_SERIAL
  if (this->interface_type_ == serial) {
    SerialRMPIO *serial_rmp = (SerialRMPIO *)(this->rmp_io_);
    serial_rmp->configure(port, baudrate);
  } else {
    RMP_THROW_MSG(ConfigurationException, "configureSerial: Cannot configure "
      "serial when the InterfaceType is not serial.");
  }
#else
  RMP_THROW_MSG(ConfigurationException, "configureSerial: The segwayrmp "
    "library is not build with serial support, not implemented.");
#endif
}

void SegwayRMP::configureUSBBySerial(std::string serial_number, int baudrate)
{
  if (this->interface_type_ == usb) {
    FTD2XXRMPIO *ftd2xx_rmp = (FTD2XXRMPIO *)(this->rmp_io_);
    ftd2xx_rmp->configureUSBBySerial(serial_number, baudrate);
  } else {
    RMP_THROW_MSG(ConfigurationException, "configureUSBBySerial: Cannot "
      "configure ftd2xx usb when the InterfaceType is not usb.");
  }
}

void SegwayRMP::configureUSBByDescription(std::string description,
                                          int baudrate)
{
  if (this->interface_type_ == usb) {
    FTD2XXRMPIO *ftd2xx_rmp = (FTD2XXRMPIO *)(this->rmp_io_);
    ftd2xx_rmp->configureUSBByDescription(description, baudrate);
  } else {
    RMP_THROW_MSG(ConfigurationException, "configureUSBByDescription: "
      "Cannot configure ftd2xx usb when the InterfaceType is not usb.");
  }
}

void SegwayRMP::configureUSBByIndex(int device_index, int baudrate)
{
  if (this->interface_type_ == usb) {
    FTD2XXRMPIO *ftd2xx_rmp = (FTD2XXRMPIO *)(this->rmp_io_);
    ftd2xx_rmp->configureUSBByIndex(device_index, baudrate);
  } else {
    RMP_THROW_MSG(ConfigurationException, "configureUSBByIndex: "
      "Cannot configure ftd2xx usb when the InterfaceType is not usb.");
  }
}

void SegwayRMP::connect(bool reset_integrators)
{
  // Connect to the interface
  this->rmp_io_->connect();

  this->connected_ = true;

  if (reset_integrators) {
    // Reset all the integrators
    this->resetAllIntegrators();
  }

  // Kick off the read thread
  this->StartReadingContinuously_();

}

void SegwayRMP::shutdown()
{
  // Ensure we are connected
  if (!this->connected_)
      RMP_THROW_MSG(ConfigurationException, "Cannot send shutdown: "
        "Not Connected.");
  try {
    Packet packet;

    packet.id = 0x0412;

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
      std::stringstream ss;
      ss << "Cannot send shutdown: " << e.what();
      RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::moveCounts(short int linear_counts, short int angular_counts)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(MoveFailedException, "Not Connected.");
  try {
    short int lc = linear_counts;
    short int ac = angular_counts;
    Packet packet;

    packet.id = 0x0413;

    packet.data[0] = (unsigned char)((lc & 0xFF00) >> 8);
    packet.data[1] = (unsigned char)(lc & 0x00FF);
    packet.data[2] = (unsigned char)((ac & 0xFF00) >> 8);
    packet.data[3] = (unsigned char)(ac & 0x00FF);
    packet.data[4] = 0x00;
    packet.data[5] = 0x00;
    packet.data[6] = 0x00;
    packet.data[7] = 0x00;

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    RMP_THROW_MSG(MoveFailedException, e.what());
  }
}

void SegwayRMP::move(float linear_velocity, float angular_velocity)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(MoveFailedException, "Not Connected.");
  try {
    short int lv = (short int)(linear_velocity * this->mps_to_counts_);
    short int av = (short int)(angular_velocity * this->dps_to_counts_);

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

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    RMP_THROW_MSG(MoveFailedException, e.what());
  }
}

void SegwayRMP::setOperationalMode(OperationalMode operational_mode)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set operational mode: "
      "Not Connected.");
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

    this->rmp_io_->sendPacket(packet);

//    while(this->segway_status_->operational_mode != operational_mode) {
//      // Check again in 10 ms
//      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
//    }
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set operational mode: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setControllerGainSchedule(
  ControllerGainSchedule controller_gain_schedule)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set controller gain "
      "schedule: Not Connected.");
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

    this->rmp_io_->sendPacket(packet);

//    while(this->segway_status_->controller_gain_schedule
//          != controller_gain_schedule)
//    {
//      // Check again in 10 ms
//      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
//    }
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set controller gain schedule: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setBalanceModeLocking(bool state)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set balance mode lock: "
      "Not Connected.");
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
    if (state)
      packet.data[7] = 0x01;
    else
      packet.data[7] = 0x00;

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set balance mode lock: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::resetAllIntegrators()
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot reset Integrators: Not "
      "Connected.");
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

    this->rmp_io_->sendPacket(packet);

    packet.data[7] = 0x02;

    this->rmp_io_->sendPacket(packet);

    packet.data[7] = 0x04;

    this->rmp_io_->sendPacket(packet);

    packet.data[7] = 0x08;

    this->rmp_io_->sendPacket(packet);

  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot reset Integrators: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setMaxVelocityScaleFactor(double scalar)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set max velocity scale "
      "factor: Not Connected.");
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

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set max velocity scale factor: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setMaxAccelerationScaleFactor(double scalar)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set max acceleration scale "
      "factor: Not Connected.");
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

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set max acceleration scale factor: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setMaxTurnScaleFactor(double scalar)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set max turn scale factor: "
      "Not Connected.");
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

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set max turn scale factor: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setCurrentLimitScaleFactor(double scalar)
{
  // Ensure we are connected
  if (!this->connected_)
    RMP_THROW_MSG(ConfigurationException, "Cannot set current limit scale "
      "factor: Not Connected.");
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

    this->rmp_io_->sendPacket(packet);
  } catch (std::exception &e) {
    std::stringstream ss;
    ss << "Cannot set current limit scale factor: " << e.what();
    RMP_THROW_MSG(ConfigurationException, ss.str().c_str());
  }
}

void SegwayRMP::setStatusCallback(SegwayStatusCallback callback) {
  this->status_callback_ = callback;
}

void SegwayRMP::setLogMsgCallback(std::string log_level,
                                    LogMsgCallback callback)
{
  // Convert to lower case
  std::transform(log_level.begin(), log_level.end(),
                 log_level.begin(), ::tolower);
  if (log_level == "debug") {
    this->debug_ = callback;
  }
  if (log_level == "info") {
    this->info_ = callback;
  }
  if (log_level == "error") {
    this->error_ = callback;
  }
}

void SegwayRMP::setTimestampCallback(GetTimeCallback callback) {
  this->get_time_ = callback;
}

void SegwayRMP::setExceptionCallback(ExceptionCallback exception_callback) {
  this->handle_exception_ = exception_callback;
}

void SegwayRMP::ReadContinuously_() {
  Packet packet;
  while (this->continuously_reading_) {
    try {
      this->rmp_io_->getPacket(packet);
      this->ProcessPacket_(packet);
    } catch (PacketRetrievalException &e) {
      if (e.error_number() == 2) // Failed Checksum
        this->error_("Checksum mismatch...");
      else if (e.error_number() == 3) // No packet received
        this->error_("No data from Segway...");
      else
        this->handle_exception_(e);
    }
  }
}

void SegwayRMP::ExecuteCallbacks_() {
  while (this->continuously_reading_) {
    SegwayStatus::Ptr ss = this->ss_queue_.dequeue();
    if (this->continuously_reading_) {
      try {
        if (ss) {
          if (this->status_callback_) {
            this->status_callback_(ss);
          } // if this->status_callback_
        } // if ss
      } catch (std::exception &e) {
        this->handle_exception_(e);
      }// try callback
    }// if continuous
  }// while continuous
}

void SegwayRMP::StartReadingContinuously_() {
  this->continuously_reading_ = true;
  this->read_thread_ =
    boost::thread(&SegwayRMP::ReadContinuously_, this);
  this->callback_execution_thread_ =
    boost::thread(&SegwayRMP::ExecuteCallbacks_, this);
}

void SegwayRMP::StopReadingContinuously_()
{
  this->continuously_reading_ = false;
  this->rmp_io_->cancel();
  this->read_thread_.join();
  this->ss_queue_.cancel();
  this->callback_execution_thread_.join();
}

void SegwayRMP::SetConstantsBySegwayType_(SegwayRMPType &rmp_type) {
  if (rmp_type == rmp200 || rmp_type == rmp400) {
    this->dps_to_counts_ = 7.8;
    this->mps_to_counts_ = 332.0;
    this->meters_to_counts_ = 33215.0;
    this->rev_to_counts_ = 112644.0;
    this->torque_to_counts_ = 1094.0;
  } else
  if (rmp_type == rmp50 || rmp_type == rmp100) {
    this->dps_to_counts_ = 7.8;
    this->mps_to_counts_ = 401.0;
    this->meters_to_counts_ = 40181.0;
    this->rev_to_counts_ = 117031.0;
    this->torque_to_counts_ = 1463.0;
  } else {
    RMP_THROW_MSG(ConfigurationException, "Invalid Segway RMP Type");
  }
}

inline short int getShortInt(unsigned char high, unsigned char low)
{
  return (short int)(((unsigned short int)high << 8)
                   | (unsigned short int)low);
}

inline int
getInt(unsigned char lhigh, unsigned char llow,
       unsigned char hhigh, unsigned char hlow)
{
  int result = 0;
  char data[4] = {llow, lhigh, hlow, hhigh};
  memcpy(&result, data, 4);
  return result;
}

bool SegwayRMP::ParsePacket_(Packet &packet, SegwayStatus::Ptr &ss_ptr)
{
  bool status_updated = false;
  if (packet.channel == 0xBB) // Ignore Channel B messages
    return status_updated;

  // This section comes largerly from the Segway example code
  switch (packet.id) {
  case 0x0400: // COMMAND REQUEST
    // This is the first packet of a msg series, timestamp here.
    ss_ptr->timestamp  = this->get_time_();
    break;
  case 0x0401:
    ss_ptr->pitch      = getShortInt(packet.data[0], packet.data[1])
                       / this->dps_to_counts_;
    ss_ptr->pitch_rate = getShortInt(packet.data[2], packet.data[3])
                       / this->dps_to_counts_;
    ss_ptr->roll       = getShortInt(packet.data[4], packet.data[5])
                       / this->dps_to_counts_;
    ss_ptr->roll_rate  = getShortInt(packet.data[6], packet.data[7])
                       / this->dps_to_counts_;
    ss_ptr->touched = true;
    break;
  case 0x0402:
    ss_ptr->left_wheel_speed  = getShortInt(packet.data[0], packet.data[1])
                              / this->mps_to_counts_;
    ss_ptr->right_wheel_speed = getShortInt(packet.data[2], packet.data[3])
                              / this->mps_to_counts_;
    ss_ptr->yaw_rate          = getShortInt(packet.data[4], packet.data[5])
                              / this->dps_to_counts_;
    ss_ptr->servo_frames      = (
                                 (((short unsigned int)packet.data[6]) << 8)
                               | ((short unsigned int)packet.data[7])
                                ) * 0.01;
    ss_ptr->touched = true;
    break;
  case 0x0403:
    ss_ptr->integrated_left_wheel_position  =
      getInt(packet.data[0], packet.data[1], packet.data[2], packet.data[3])
    / this->meters_to_counts_;
    ss_ptr->integrated_right_wheel_position =
      getInt(packet.data[4], packet.data[5], packet.data[6], packet.data[7])
    / this->meters_to_counts_;
    ss_ptr->touched = true;
    break;
  case 0x0404:
    ss_ptr->integrated_forward_position =
      getInt(packet.data[0], packet.data[1], packet.data[2], packet.data[3])
    / this->meters_to_counts_;
    ss_ptr->integrated_turn_position    =
      getInt(packet.data[4], packet.data[5], packet.data[6], packet.data[7])
    / this->rev_to_counts_;
    // convert from revolutions to degrees
    ss_ptr->integrated_turn_position *= 360.0;
    ss_ptr->touched = true;
    break;
  case 0x0405:
    ss_ptr->left_motor_torque  = getShortInt(packet.data[0], packet.data[1])
                               / this->torque_to_counts_;
    ss_ptr->right_motor_torque = getShortInt(packet.data[2], packet.data[3])
                               / this->torque_to_counts_;
    ss_ptr->touched = true;
    break;
  case 0x0406:
    ss_ptr->operational_mode          =
      OperationalMode(getShortInt(packet.data[0], packet.data[1]));
    ss_ptr->controller_gain_schedule  =
      ControllerGainSchedule(getShortInt(packet.data[2], packet.data[3]));
    ss_ptr->ui_battery_voltage        =
      (
        (((short unsigned int)packet.data[4]) << 8)
      | ((short unsigned int)packet.data[5])
      ) * 0.0125 + 1.4;
    ss_ptr->powerbase_battery_voltage =
      (
        (((short unsigned int)packet.data[6]) << 8)
      | ((short unsigned int)packet.data[7])
      ) / 4.0;
    ss_ptr->touched = true;
    break;
  case 0x0407:
    ss_ptr->commanded_velocity =
      (float)getShortInt(packet.data[0], packet.data[1])
    / this->mps_to_counts_;
    ss_ptr->commanded_yaw_rate =
      (float)getShortInt(packet.data[2], packet.data[3])
    / 1024.0;
    status_updated = true;
    ss_ptr->touched = true;
    break;
  case 0x0680:
    if (packet.data[3] == 0x80) // Motors Enabled
      ss_ptr->motor_status = 1;
    else // E-Stopped
      ss_ptr->motor_status = 0;
    ss_ptr->touched = true;
    break;
  default: // Unknown/Unhandled Message
    break;
  };
  return status_updated;
}

void SegwayRMP::ProcessPacket_(Packet &packet)
{
  bool status_updated = false;

  status_updated = this->ParsePacket_(packet, this->segway_status_);

  // Messages come in order 0x0400, 0x0401, ... 0x0407 so a
  //  complete "cycle" of information has been sent every
  //  time we get an 0x0407
  if (status_updated) {
    if (this->ss_queue_.enqueue(this->segway_status_)) {
      this->error_("Falling behind, SegwayStatus Queue Full, skipping packet "
        "report...");
    }
    this->segway_status_ = SegwayStatus::Ptr(new SegwayStatus());
  }
}


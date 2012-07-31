/*!
 * \file segwayrmp.h
 * \author William Woodall <wjwwood@gmail.com>
 * \version 1.0
 *
 * \section LICENSE
 *
 * The BSD License
 *
 * Copyright (c) 2012 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * \section DESCRIPTION
 *
 * This provides a cross platform interface for the Segway RMP's.
 * 
 * This library depends on Boost: http://www.boost.org/
 * and possibly depends on a Serial library: https://github.com/wjwwood/serial
 * depending on the library build configuration.
 */
 
#ifndef SEGWAYRMP_H
#define SEGWAYRMP_H

#include <exception>
#include <sstream>
#include <queue>
#include <typeinfo>

#include <boost/function.hpp>
#include <boost/thread.hpp>

/* Setup (u)int*_t types if not UNIX. */
#if defined(_WIN32) && !defined(__MINGW32__)
  typedef unsigned int uint32_t;
#else
# include <stdint.h>
#endif

// ClassName - The name of the exception class
// Prefix    - The common prefix
// Message   - The default message, if one is not specified at Throw
// The message takes the form of:
// ClassName occurred at line # of file `<file>`: Prefix+Message
#define DEFINE_EXCEPTION(ClassName, Prefix, Message) \
  class ClassName : public std::exception { \
    void operator=(const ClassName &); \
    const ClassName & operator=( ClassName ); \
    const char * what_; \
    const int id_; \
  public: \
    ClassName(const char * file, const int ln, \
              const char * msg = Message, const int id = -1) : id_(id) { \
      std::stringstream ss; \
      ss << #ClassName " occurred at line " << ln \
         << " of `" << file << "`: " << Prefix << msg; \
      what_ = ss.str().c_str(); \
    } \
    virtual const char* what () const throw () { return what_; } \
    const int error_number() { return this->id_; } \
  };

#define RMP_THROW(ExceptionClass) \
  throw ExceptionClass(__FILE__, __LINE__)

#define RMP_THROW_MSG(ExceptionClass, Message) \
  throw ExceptionClass(__FILE__, __LINE__, (Message) )

#define RMP_THROW_MSG_AND_ID(ExceptionClass, Message, Id) \
  throw ExceptionClass(__FILE__, __LINE__, (Message), (Id) )

/*!
 * Defines the number of buffered SegwayStatus structures to be held.
 */
#define MAX_SEGWAYSTATUS_QUEUE_SIZE 100

namespace segwayrmp {

/*!
 * Defines the possible modes of communication for the Segway Interface.
 */
typedef enum {
  /*!
   * This method is unsupported currently.
   */
  can     = 0,
  /*!
   * This method communicates to the Segway via usb using the FTD2XX library.
   */
  usb     = 1,
  /*!
   * This method communicates to the Segway via a virtual serial port.
   */
  serial  = 2,
  /*!
   * This method communicates to the Segway via a UDP/IP ethernet interface.
   * Note: This is only allowed with the rmpx440
   */
  ethernet  = 3,
  no_interface = -1
} InterfaceType;

/*!
 * Defines the type of Segway RMP you are using.
 */
typedef enum {
  /*!
   * This indicates you have an RMP50 vehicle.
   */
  rmp50   = 0,
  /*!
   * This indicates you have an RMP100 vehicle.
   */
  rmp100  = 1,
  /*!
   * This indicates you have an RMP200 vehicle.
   */
  rmp200  = 2,
  /*!
   * You still need to run two instances of SegwayRMP, this is the same as
   * setting SegwayRMPType as rmp200.
   */
  rmp400  = 3,
  /*!
   * This is the new rmp400, dubbed the rmpx440, which allows ethernet control.
   */
  rmpx440  = 4
} SegwayRMPType;

/*!
 * Defines the operational modes of the Segway vehicle.
 */
typedef enum {
  /*!
   * This means the Segway has not been set to a mode yet.
   */
  disabled   = 0,
  /*!
   * This mode means the Segway will move without balancing, i.e. it has a
   * caster.
   */
  tractor    = 1,
  /*!
   * This mode means the Segway will balance on its two drive wheels.
   */
  balanced   = 2,
  /*!
   * This mode means the Segway will power down.
   */
  power_down = 3
} OperationalMode;

/*!
 * Defines the controller gain schedule options for the Segway vehicle.
 */
typedef enum {
  /*!
   * For relatively small weights (20 kg) near the platform
   */
  light = 0,
  /*!
   * For relatively small weights (20 kg) spread out vertically on the
   * platform
   */
  tall  = 1,
  /*!
   * For heavy weights (40 kg) near the platform
   */
  heavy = 2
} ControllerGainSchedule;

/*!
 * Represents the time of a timestamp using seconds and nanoseconds.
 */
class SegwayTime
{
public:
  SegwayTime (uint32_t sec = 0, uint32_t nsec = 0) {
    this->sec = sec;
    this->nsec = nsec;
  }
  
  uint32_t sec; /*!< Seconds since the epoch (UNIX time) */
  uint32_t nsec; /*!< Nanoseconds since the last second */
};

template<typename T>
class FiniteConcurrentSharedQueue {
  std::queue<boost::shared_ptr<T> > queue_;
  boost::mutex mutex_;
  boost::condition_variable condition_variable_;
  size_t size_;
  bool canceled_;
public:
  FiniteConcurrentSharedQueue(size_t size = 1024)
    : size_(size), canceled_(false) {}
  ~FiniteConcurrentSharedQueue() {}
  
  size_t size() {
    boost::mutex::scoped_lock(mutex_);
    return queue_.size();
  }
  
  bool empty() {
    return this->size() == 0;
  }
  
  bool enqueue(boost::shared_ptr<T> element) {
    bool dropped_element = false;
    {
      boost::lock_guard<boost::mutex> lock(mutex_);
      if (queue_.size() == size_) {
        queue_.pop();
        dropped_element = true;
      }
      queue_.push(element);
    }
    condition_variable_.notify_one();
    return dropped_element;
  }
  
  boost::shared_ptr<T> dequeue() {
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (queue_.empty()) {
      condition_variable_.wait(lock);
      if (this->canceled_) {
        return boost::shared_ptr<T>();
      }
    }
    boost::shared_ptr<T> element = queue_.front();
    queue_.pop();
    return element;
  }
  
  void cancel() {
    {
      boost::lock_guard<boost::mutex> lock(mutex_);
      this->canceled_ = true;
    }
    condition_variable_.notify_all();
  }
  
  void reset() {
    {
      boost::lock_guard<boost::mutex> lock(mutex_);
      this->canceled_ = false;
    }
    condition_variable_.notify_all();
  }
};

// Forward declarations
class RMPIO;
class Packet;

/*!
 * Contains Status Information returned by the Segway RMP.
 */
class SegwayStatus {
public:
  SegwayTime timestamp; /*!< Time that this data was received. */
  float pitch; /*!< Integrated Pitch in degrees. */
  float pitch_rate; /*!< Current Pitch Aungular Velocity in degrees/second. */
  float roll; /*!< Integrated Roll in degrees. */
  float roll_rate; /*!< Current Roll Angular Velocity, degrees/second. */
  float left_wheel_speed; /*!< Current Left Wheel Velocity, meters/second. */
  float right_wheel_speed; /*!< Current Right Wheel Velocity, meters/second. */
  float yaw_rate; /*!< Current Yaw Angular Velocity, degrees/second. */
  float servo_frames; /*!< Current Servo Time in seconds (0.01s intervals). */
  /*! Integrated Left Wheel Position in meters. */
  float integrated_left_wheel_position;
  /*! Integrated Right Wheel Position in meters. */
  float integrated_right_wheel_position;
  /*! Integrated Forward/Aft Displacement in meters. */
  float integrated_forward_position;
  /*! Integrated Yaw Angle in degrees. */
  float integrated_turn_position;
  /*! Current Left Motor Torque, Newton-meters. */
  float left_motor_torque;
  /*! Current Right Motor Torque, Newton-meters. */
  float right_motor_torque;
  /*! Current UI Battery Voltage in Volts. */
  float ui_battery_voltage;
  /*! Current Powerbase Battery Voltage in Volts. */
  float powerbase_battery_voltage;
  /*!
   * Current Operational Mode one of {disabled, tractor, balanced, power_down}.
   */
  OperationalMode operational_mode;
  /*!
   * Current Controller Gain Schedule (balance mode only) one of {light, tall,
   * heavy}.
   */
  ControllerGainSchedule controller_gain_schedule;
  /*! Current Commanded Velocity, meters/second. */
  float commanded_velocity;
  /*! Current Commanded Angular Velocity in degrees/second. */
  float commanded_yaw_rate; 
  /*! Current Motor Status one of {Enabled = 1, Emergency-Stopped = 0}. */
  int motor_status;
  /*! For Testing Only. */
  bool touched;
  
  SegwayStatus();
  
  std::string str();

  typedef boost::shared_ptr<SegwayStatus> Ptr;
};

typedef boost::function<void(SegwayStatus::Ptr)> SegwayStatusCallback;
typedef boost::function<SegwayTime(void)> GetTimeCallback;
typedef boost::function<void(const std::exception&)> ExceptionCallback;
typedef boost::function<void(const std::string&)> LogMsgCallback;

/*!
 * Provides an interface for the Segway RMP.
 */
class SegwayRMP {
public:
  /*!
   * Constructs the SegwayRMP object given the interface type.
   * 
   * \param interface_type This must be can, usb, or serial. Default is usb.
   * \param segway_rmp_type This can be rmp50, rmp100, rmp200, or rmp400.
   *  Default is rmp200.
   */
  SegwayRMP(InterfaceType interface_type = serial,
            SegwayRMPType segway_rmp_type = rmp200);
  ~SegwayRMP();

  /*!
   * Configures the serial interface, if the library is built with serial
   * support, otherwise throws ConfigurationException.
   * 
   * \param port Defines which serial port to connect to in serial mode.
   * \param baudrate Defines the speed of the serial port in baud's.
   *  Defaults to 460800 (recommended).
   */
  void
  configureSerial(std::string port, int baudrate = 460800);

  /*!
   * Configures the FTD2XX usb interface, if the library is built with usb
   * support, otherwise throws ConfigurationException.
   * 
   * \param serial_number Defines which usb port to connect to based on the
   *  devices serial number.
   * \param baudrate Defines the speed of the usb port in baud's.  Defaults
   *  to 460800 (recommended).
   */
  void
  configureUSBBySerial(std::string serial_number,
                       int baudrate = 460800);

  /*!
   * Configures the FTD2XX usb interface, if the library is built with usb
   * support, otherwise throws ConfigurationException.
   * 
   * \param description Defines which usb port to connect to based on the
   *  devices description.
   * \param baudrate Defines the speed of the usb port in baud's.  Defaults
   *  to 460800 (recommended).
   */
  void
  configureUSBByDescription(std::string description, int baudrate = 460800);

  /*!
   * Configures the FTD2XX usb interface, if the library is built with usb
   * support, otherwise throws ConfigurationException.
   * 
   * \param device_index Defines which usb port to connect to based on the
   *  devices index.
   *              Note: This is indexed by all ftd2xx devices on the system.
   * \param baudrate Defines the speed of the usb port in baud's.  Defaults
   *  to 460800 (recommended).
   */
  void
  configureUSBByIndex(int device_index, int baudrate = 460800);

  /*!
   * Connects to the Segway. Ensure it has been configured first.
   *
   * \param reset_integrators If this is true, the integrators are reset.
   */
  void
  connect(bool reset_integrators = true);

  /*!
   * Sends a shutdown command to the RMP that immediately shuts it down.
   */
  void
  shutdown();

  /*!
   * This command moves the base at a given linear and angular count.
   *
   * \param linear_counts Forward/Reverse effort, in range [-1176, 1176].
   *  Which maps approximately to [-8, 8] mph (assuming a max velocity
   *  scalar of 1.0)
   * \param angular_counts Angular effort, in range [-1024, 1024].
   */
  void
  moveCounts(short int linear_counts, short int angular_counts);

  /*!
   * This command moves the base at a given linear and angular velocity.
   * This assumes the max velocity scalar is set to 1.0.
   * 
   * \param linear_velocity Forward/Reverse desired velocity of the vehicle
   *  in m/s.
   * \param angular_velocity Desired angular velocity of the vehicle in
   *  degrees/s, positive to is left.
   */
  void
  move(float linear_velocity, float angular_velocity);

  /************ Getter and Setters ************/
  
  /*!
   * Sets the operational mode.
   * 
   * \param operational_mode This must be disabled, tractor, or balanced.
   */
  void
  setOperationalMode(OperationalMode operational_mode);
   
  /*!
   * Sets the controller gain schedule.
   * 
   * \param controller_gain_schedule This sets the contoller gain schedule,
   *  possible values are light, tall, and heavy.
   */
  void
  setControllerGainSchedule(ControllerGainSchedule controller_gain_schedule);
  
  /*!
   * Locks or unlocks the balancing mode.
   * 
   * \param state This allows you to specify whether you want lock or unlock
   *  balancing mode. 
   * True for locked and False for unlocked.  The default state is True.
   */
  void
  setBalanceModeLocking(bool state = true);
  
  /*!
   * Resets all of the integrators.
   * 
   * \todo Add individual functions for reseting each integrator.
   */
  void
  resetAllIntegrators();
  
  /*!
   * Sets the Max Velocity Scale Factor
   *
   * \param scalar This is a value between 0.0 and 1.0 which will set the
   * scale factor on the segway internally for all velocity commands.
   * Values larger than 1.0 will round down to 1.0 and values < 0 will round
   * up to 0.0. Parameter defaults to 1.0.
   */
  void
  setMaxVelocityScaleFactor(double scalar = 1.0);
  
  /*!
   * Sets the Max Acceleration Scale Factor
   *
   * \param scalar This is a value between 0.0 and 1.0 which will set the
   *  acceleration scale factor on the segway internally for all velocity
   *  commands. Values larger than 1.0 will round down to 1.0 and values < 0 
   *  will round up to 0.0. Parameter defaults to 1.0.
   */
  void
  setMaxAccelerationScaleFactor(double scalar = 1.0);
  
  /*!
   * Sets the Max Turn Scale Factor
   *
   * \param scalar This is a value between 0.0 and 1.0 which will set the
   * scale factor on the segway internally for all turn commands.
   * Values larger than 1.0 will round down to 1.0 and values < 0 will round
   * up to 0.0. Parameter defaults to 1.0.
   */
  void
  setMaxTurnScaleFactor(double scalar = 1.0);
  
  /*!
   * Sets the Current Limit Scale Factor
   *
   * \param scalar This is a value between 0.0 and 1.0 which will set the
   * current limit for the drive motors, limiting torque.
   * Values larger than 1.0 will round down to 1.0 and values < 0 will round
   * up to 0.0. Parameter defaults to 1.0.
   */
  void
  setCurrentLimitScaleFactor(double scalar = 1.0);
  
  /*!
   * Sets the Callback Function to be called on new Segway Status Updates.
   * 
   * The provided function must follow this prototype:
   * <pre>
   *    void yourSegwayStatusCallback(segwayrmp::SegwayStatus &segway_status)
   * </pre>
   * Here is an example:
   * <pre>
   *    void handleSegwayStatus(segwayrmp::SegwayStatus &ss) {
   *        std::cout << ss.str() << std::endl << std::endl;
   *    }
   * </pre>
   * And the resulting call to make it the callback:
   * <pre>
   *    segwayrmp::SegwayRMP my_segway_rmp;
   *    my_segway_rmp.setStatusCallback(handleSegwayStatus);
   * </pre>
   * Alternatively you can use a class method as a callback
   * using boost::bind:
   * <pre>
   *    #include <boost/bind.hpp>
   *    
   *    #include "segwayrmp.h"
   *    
   *    class MySegwayWrapper
   *    {
   *    public:
   *     MySegwayWrapper () {
   *      my_segway_rmp.setStatusCallback(
   *        boost::bind(&MySegwayWrapper::handleSegwayStatus, this, _1));
   *     }
   *    
   *     void handleSegwayStatus(segwayrmp::SegwayStatus &ss) {
   *       std::cout << ss.str() << std::endl << std::endl;
   *     }
   *    
   *    private:
   *     segwayrmp::SegwayRMP my_segway_rmp;
   *    };
   * </pre>
   * \param status_callback A function pointer to the callback to handle new 
   * SegwayStatus updates.
   * \todo Make all the callbacks capable of taking class methods
   */
  void
  setStatusCallback(SegwayStatusCallback callback);
  
  /*!
   * Sets the Callback Function to be called when a log message occurs.
   * 
   * This allows you to hook into the message reporting of the library and
   * use your own logging facilities.
   * 
   * The provided function must follow this prototype:
   * <pre>
   *    void yourLogMsgCallback(const std::string &msg)
   * </pre>
   * Here is an example (Using Debug in this case):
   * <pre>
   *    void yourDebugMsgCallback(const std::string &msg) {
   *        std::cerr << "SegwayRMP Debug: " << msg << std::endl;
   *    }
   * </pre>
   * And the call to make it the callback (Again using Debug for example):
   * <pre>
   *    segwayrmp::SegwayRMP my_segway_rmp;
   *    my_segway_rmp.setLogMsgCallback("debug", yourDebugMsgCallback);
   * </pre>
   * Alternatively you can use a class method as a callback
   * using boost::bind:
   * <pre>
   *    #include <boost/bind.hpp>
   *    
   *    #include "segwayrmp.h"
   *    
   *    class MySegwayWrapper
   *    {
   *    public:
   *     MySegwayWrapper () {
   *      my_segway_rmp.setLogMsgCallback("debug"
   *        boost::bind(&MySegwayWrapper::handleDebugMsg, this, _1));
   *     }
   *    
   *     void handleDebugMsg(const std::string &msg) {
   *       std::cerr << "SegwayRMP Debug: " << msg << std::endl;
   *     }
   *    
   *    private:
   *     segwayrmp::SegwayRMP my_segway_rmp;
   *    };
   * </pre>
   * \param log_level A std::string which indicates which log level to 
   *  assign a callback to.  Must be one of "debug", "info", or "error".
   * \param callback A LogMsgCallback type function which will be called 
   *  when a log of level log_level occurs.
   */
  void
  setLogMsgCallback(std::string log_level, LogMsgCallback callback);
  
  /*!
   * Sets the Callback Function to be called on when a timestamp is made.
   * 
   * This allows you to provide your own time stamp method for time stamping 
   * the segway data.  You must return a SegwayTimeStruct, but you can return
   * an empty time struct and manually store your time stamp and use your 
   * timestamp when processing the segway status callback.
   * 
   * The provided function must follow this prototype:
   * <pre>
   *    SegwayTime yourTimestampCallback()
   * </pre>
   * Here is an example:
   * <pre>
   *    SegwayTime yourTimestampCallback() {
   *        SegwayTime st;
   *        timespec start;
   *        clock_gettime(CLOCK_REALTIME, &start);
   *        st.sec  = start.tv_sec;
   *        st.nsec = start.tv_nsec;
   *        return st;
   *    }
   * </pre>
   * And the resulting call to make it the callback:
   * <pre>
   *    segwayrmp::SegwayRMP my_segway_rmp;
   *    my_segway_rmp.setTimestampCallback(yourTimestampCallback);
   * </pre>
   * Alternatively you can use a class method as a callback
   * using boost::bind:
   * <pre>
   *    #include <boost/bind.hpp>
   *    #include <ctime>
   *    
   *    #include "segwayrmp.h"
   *    
   *    class MySegwayWrapper
   *    {
   *    public:
   *     MySegwayWrapper () {
   *       my_segway_rmp.setTimestampCallback(
   *         boost::bind(&MySegwayWrapper::handleTimestamp, this, _1));
   *     }
   *     SegwayTime handleTimestamp() {
   *        SegwayTime st;
   *        timespec start;
   *        clock_gettime(CLOCK_REALTIME, &start);
   *        st.sec  = start.tv_sec;
   *        st.nsec = start.tv_nsec;
   *        return st;
   *     }
   *    
   *    private:
   *     segwayrmp::SegwayRMP my_segway_rmp;
   *    };
   * </pre>
   * \param callback A function pointer to the callback to handle 
   *  Timestamp creation.
   */
  void
  setTimestampCallback(GetTimeCallback callback);
  
  /*!
   * Sets the Callback Function to be called on when an unhandled exception
   * occurs.
   * 
   * This allows you to catch and handle otherwise unhandled exceptions that
   * might occur in an internal thread to the segwayrmp library.
   * 
   * The provided function must follow this prototype:
   * <pre>
   *    void yourExceptionCallback(const std::exception&)
   * </pre>
   * Here is an example:
   * <pre>
   *    SegwayTime yourTimestampCallback() {
   *        std::cerr << "SegwayRMP Unhandled Exception: " << error.what()
   *                  << std::endl;
   *    }
   * </pre>
   * And the resulting call to make it the callback:
   * <pre>
   *    segwayrmp::SegwayRMP my_segway_rmp;
   *    my_segway_rmp.setExceptionCallback(yourExceptionCallback);
   * </pre>
   * Alternatively you can use a class method as a callback
   * using boost::bind:
   * <pre>
   *    #include <boost/bind.hpp>
   *    #include <ctime>
   *    
   *    #include "segwayrmp.h"
   *    
   *    class MySegwayWrapper
   *    {
   *    public:
   *     MySegwayWrapper () {
   *       my_segway_rmp.setExceptionCallback(
   *         boost::bind(&MySegwayWrapper::handleException, this, _1));
   *     }
   *     SegwayTime handleException() {
   *        std::cerr << "SegwayRMP Unhandled Exception: " << error.what()
   *                  << std::endl;
   *     }
   *    
   *    private:
   *     segwayrmp::SegwayRMP my_segway_rmp;
   *    };
   * </pre>
   * \param callback A function pointer to the callback to handle
   *  otherwise unhandled exceptions that occur.
   */
  void
  setExceptionCallback(ExceptionCallback callback);
private:
  // Disable Copy Constructor
  void operator=(const SegwayRMP &);
  const SegwayRMP & operator=(SegwayRMP);

  // Interface implementation (pimpl idiom)
  RMPIO * rmp_io_;

  // Configuration Variables
  InterfaceType interface_type_;
  SegwayRMPType segway_rmp_type_;

  // Status Variables
  bool connected_;
  SegwayStatus::Ptr segway_status_;

  // Constants
  void SetConstantsBySegwayType_(SegwayRMPType &rmp_type);
  double dps_to_counts_;
  double mps_to_counts_;
  double meters_to_counts_;
  double rev_to_counts_;
  double torque_to_counts_;

  // Callbacks
  SegwayStatusCallback status_callback_;
  GetTimeCallback get_time_;
  LogMsgCallback debug_, info_, error_;
  ExceptionCallback handle_exception_;
  FiniteConcurrentSharedQueue<SegwayStatus> ss_queue_;

  // Continuous Read Functions and Variables
  void ReadContinuously_();
  void ExecuteCallbacks_();
  void StartReadingContinuously_();
  void StopReadingContinuously_();
  bool continuously_reading_;
  boost::thread read_thread_;
  boost::thread callback_execution_thread_;

  // Parsing Functions and Variables
  void ProcessPacket_(Packet &packet);
  bool ParsePacket_(Packet &packet, SegwayStatus::Ptr &ss_ptr);
};

DEFINE_EXCEPTION(NoHighPerformanceTimersException, "", "This system does not "
  "appear to have a High Precision Event Timer (HPET) device.");

DEFINE_EXCEPTION(ConnectionFailedException, "Error connecting to the "
  "SegwayRMP: ", "Unspecified");

DEFINE_EXCEPTION(ReadFailedException, "Error reading from the SegwayRMP: ",
  "Unspecified");

DEFINE_EXCEPTION(WriteFailedException, "Error writing to the SegwayRMP: ",
  "Unspecified");

DEFINE_EXCEPTION(MoveFailedException, "Error moving the SegwayRMP: ",
  "Unspecified");

DEFINE_EXCEPTION(ConfigurationException, "Error configuring the SegwayRMP: ",
  "Unspecified");

} // Namespace segwayrmp

#endif

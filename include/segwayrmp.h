/*!
 * \file segwayrmp.h
 * \author William Woodall <wjwwood@gmail.com>
 * \version 0.1
 *
 * \section LICENSE
 *
 * The BSD License
 *
 * Copyright (c) 2011 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \section DESCRIPTION
 *
 * This provides a cross platform interface for the Segway RMP's.
 */
 
#ifndef SEGWAYRMP_H
#define SEGWAYRMP_H

#include <sstream>

#include "rmp_io.h"

namespace segwayrmp {

/*!
 * Converts Meters per Second to Counts to be sent to the segway.
 * 
 *      X meters   1 kilometer    1 mile     3600 seconds
 * X' = -------- * ----------- * --------- * ------------
 *       second    1000 meters   1.609 km        1 hr
 * 
 *           X' milesperhr    1176 counts
 * Command = ------------- * -------------  (From the datasheet)
 *                 1          8 milesperhr
 */
static double MPS_TO_COUNTS = (1.0/1000.0)*(1.0/1.609)*(3600.0/1.0)*(1176.0/8.0);

/*!
 * Defines the possible modes of communication for the Segway Interface.
 */
typedef enum {
    can     = 0, /*!< This method is unsupported currently. */
    usb     = 1, /*!< This method is unsupported currently. */
    serial  = 2  /*!< This method communicates to the Segway via a virtual serial port. */
} InterfaceType;

/*!
 * Defines the operational modes of the Segway vehicle.
 */
typedef enum {
    disabled   = 0, /*!< This means the Segway has no been set to a mode yet. (This must be done before use) */
    tractor    = 1, /*!< This mode means the Segway will move without balancing, i.e. it has a caster. */
    balanced   = 2, /*!< This mode means the Segway will balance on its two drive wheels. */
    power_down = 3  /*!< This mode means the Segway will power down. */
} OperationalMode;

/*!
 * Defines the controller gain schedule options for the Segway vehicle.
 */
typedef enum {
    light = 0, /*!< For relatively small weights (20 kg) near the platform */
    tall  = 1, /*!< For relatively small weights (20 kg) spread out vertically on the platform */
    heavy = 2  /*!< For heavy weights (40 kg) near the platform */
} ControllerGainSchedule;

/*!
 * Contains Information returned by the Segway RMP.
 */
class SegwayStatus {
public:
    float pitch;
    float pitch_rate;
    float roll;
    float roll_rate;
    float left_wheel_speed;
    float right_wheel_speed;
    float yaw_rate;
    float servo_frames;
    float integrated_left_wheel_position;
    float integrated_right_wheel_position;
    float integrated_forward_position;
    float integrated_turn_position;
    float left_motor_torque;
    float right_motor_torque;
    float ui_battery_voltage;
    float powerbase_battery_voltage;
    OperationalMode operational_mode;
    ControllerGainSchedule controller_gain_schedule;
    float commanded_velocity;
    float commanded_yaw_rate;
    
    int motor_status;
    
    SegwayStatus();
    
    std::string str();
};

/*!
 * Provides an interface for the Segway RMP.
 */
class SegwayRMP {
public:
    /*!
     * Constructs the SegwayRMP object give the interface type.
     * 
     * \param interface_type This must be can, usb, or serial.
     */
    SegwayRMP(InterfaceType interface_type);
    ~SegwayRMP();
    
    /*!
     * Configures the interface, for serial only ATM.
     * 
     * In the future this function will accomodate or change based on the connection type (serial, usb, or can).
     * 
     * \param port Defines which serial port to connect to in serial mode.
     * \param baudrate Defines the speed of the serial port in baud's.  Defaults to 460800 (recommended).
     */
    void configure(std::string port, int baudrate = 460800);
    
    /*!
     * Connects to the Segway.
     * 
     * \param operational_mode Defines the operation mode of the segway, this must be tractor or balanced to 
     * have a successful connection.  If you want to connect to the Segway but not put it in a mode, use disabled (0).
     * This will resulting in connecting to the Segway interface, but not setting the mode or controller gain schedule.
     * Note: If you use default, you must manually set the operational mode and controller gain schedule. The default is tractor.
     * \param controller_gain_schedule This is the controller gain schedule for the Segway vehicle, which is only
     * used in balanced mode.  The default is light and is the appropriate option if using tractor mode.
     */
    void connect(OperationalMode operational_mode = tractor, ControllerGainSchedule controller_gain_schedule = light);
    
    /*!
     * This command moves the base.
     * 
     * \param linear Forward/Reverse desired velocity of the vehicle in m/s.
     * \param angular Desired angular velocity of the vehicle in rad/s, positive to is left.
     */
    void move(float linear_velocity, float angular_velocity);
    
    /************ Getter and Setters ************/
    
    /*!
     * Sets the operational mode.
     * 
     * \param operational_mode This must be disabled, tractor, or balanced.
     */
    void setOperationalMode(OperationalMode operational_mode);
     
    /*!
    * Sets the controller gain schedule.
    * 
    * \param controller_gain_schedule This sets the contoller gain schedule, possible values are light, tall, and heavy.
    */
    void setControllerGainSchedule(ControllerGainSchedule controller_gain_schedule);
    
    /*!
     * Locks or unlocks the balancing mode.
     * 
     * \param state This allows you to specify whether you want lock or unlock balancing mode. 
     * True for locked and False for unlocked.  The default state is True.
     */
     void setBalanceModeLocking(bool state = true);
     
     /*!
      * Resets all of the integrators.
      * 
      * \todo Add individual functions for reseting each integrator.
      */
     void resetAllIntegrators();
     
     void setStatusCallback(void (*status_callback)(SegwayStatus &segway_status));
     
     void setDebugMsgCallback(void (*f)(const std::string &msg));
     void setInfoMsgCallback(void (*f)(const std::string &msg));
     void setErrorMsgCallback(void (*f)(const std::string &msg));
private:
    void readContinuously();
    void startContinuousRead();
    void stopContinuousRead();
    void parsePacket(Packet &packet);
    void executeCallback(SegwayStatus segway_status);
    void (*status_callback)(SegwayStatus &segway_status);
    void (*debug)(const std::string &msg);
    void (*info)(const std::string &msg);
    void (*error)(const std::string &msg);
    
    // Interface Type
    InterfaceType interface_type;
    
    // Communication interface
    RMPIO * rmp_io;
    
    // Segway Status Object
    SegwayStatus segway_status;
    
    // Connection Status
    bool connected;
    
    // Continuous Read Thread
    boost::thread continuous_read_thread;
    
    // Continuous Read status
    bool continuous;
    
    // SegwayStatus Callback Execution Thread
    boost::thread callback_execution_thread;
    
    // SegwayStatus Callback Execution Thread Status
    bool callback_execution_thread_status;
};

class ConnectionFailedException : public std::exception {
    const char * e_what;
public:
    ConnectionFailedException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Error connecting to SegwayRMP: " << this->e_what;
        return ss.str().c_str();
    }
};

class ConfigurationException : public std::exception {
    const char * e_who;
    const char * e_what;
public:
    ConfigurationException(const char* e_who, const char * e_what) {this->e_who = e_who; this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Error configuring the " << this->e_who << " of the SegwayRMP: " << this->e_what;
        return ss.str().c_str();
    }
};

class MoveFailedException : public std::exception {
    const char * e_what;
public:
    MoveFailedException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Error moving the SegwayRMP: " << this->e_what;
        return ss.str().c_str();
    }
};

} // Namespace segwayrmp

#endif

#include "segwayrmp/segwayrmp.h"
#include "segwayrmp/impl/rmp_ftd2xx.h"

#include <iostream>
#include <sstream>

using namespace segwayrmp;

static const bool ftd2xx_devices_debug = false;

inline std::string
getErrorMessageByFT_STATUS(FT_STATUS result, std::string what)
{
  std::stringstream msg;
  msg << "FTD2XX error while " << what << ": ";
  switch (result) {
    case FT_INVALID_HANDLE:
        msg << "FT_INVALID_HANDLE";
        break;
    case FT_DEVICE_NOT_FOUND:
        msg << "FT_DEVICE_NOT_FOUND";
        break;
    case FT_DEVICE_NOT_OPENED:
        msg << "FT_DEVICE_NOT_OPENED";
        break;
    case FT_IO_ERROR:
        msg << "FT_IO_ERROR";
        break;
    case FT_INSUFFICIENT_RESOURCES:
        msg << "FT_INSUFFICIENT_RESOURCES";
        break;
    case FT_INVALID_PARAMETER:
        msg << "FT_INVALID_PARAMETER";
        break;
    case FT_INVALID_BAUD_RATE:
        msg << "FT_INVALID_BAUD_RATE";
        break;
    case FT_DEVICE_NOT_OPENED_FOR_ERASE:
        msg << "FT_DEVICE_NOT_OPENED_FOR_ERASE";
        break;
    case FT_DEVICE_NOT_OPENED_FOR_WRITE:
        msg << "FT_DEVICE_NOT_OPENED_FOR_WRITE";
        break;
    case FT_FAILED_TO_WRITE_DEVICE:
        msg << "FT_FAILED_TO_WRITE_DEVICE";
        break;
    case FT_EEPROM_READ_FAILED:
        msg << "FT_EEPROM_READ_FAILED";
        break;
    case FT_EEPROM_WRITE_FAILED:
        msg << "FT_EEPROM_WRITE_FAILED";
        break;
    case FT_EEPROM_ERASE_FAILED:
        msg << "FT_EEPROM_ERASE_FAILED";
        break;
    case FT_EEPROM_NOT_PRESENT:
        msg << "FT_EEPROM_NOT_PRESENT";
        break;
    case FT_EEPROM_NOT_PROGRAMMED:
        msg << "FT_EEPROM_NOT_PROGRAMMED";
        break;
    case FT_INVALID_ARGS:
        msg << "FT_INVALID_ARGS";
        break;
    case FT_NOT_SUPPORTED:
        msg << "FT_NOT_SUPPORTED";
        break;
    case FT_OTHER_ERROR:
        msg << "FT_OTHER_ERROR";
        break;
    default:
        msg << "Unknown FTD2XX Error.";
        break;
  }
  return msg.str();
}

std::vector<FT_DEVICE_LIST_INFO_NODE> segwayrmp::enumerateUSBDevices() {
    FT_STATUS result;
    FT_DEVICE_LIST_INFO_NODE *device_info;
    DWORD number_of_devices;

    // If mac or linux you must set VID/PID
    #ifndef _WIN32
    DWORD FTDI_VID = 0x403;
    DWORD SEGWAY_PID = 0xe729;

    try {
      result = FT_SetVIDPID(FTDI_VID, SEGWAY_PID);
    } catch(std::exception &e) {
      RMP_THROW_MSG(ReadFailedException, e.what());
    }
    if (result != FT_OK) {
      RMP_THROW_MSG(ReadFailedException,
        getErrorMessageByFT_STATUS(result, "setting vid and pid").c_str());
    }
    #endif

    try {
      result = FT_CreateDeviceInfoList(&number_of_devices);
    } catch(std::exception &e) {
      RMP_THROW_MSG(ReadFailedException, e.what());
    }
    if (result != FT_OK) {
      RMP_THROW_MSG(ReadFailedException,
        getErrorMessageByFT_STATUS(result, "enumerating devices").c_str());
    }

    if (ftd2xx_devices_debug)
      std::cout << "Number of devices is: " << number_of_devices << std::endl;

    std::vector<FT_DEVICE_LIST_INFO_NODE> devices;
    if (number_of_devices > 0) {
      device_info = (FT_DEVICE_LIST_INFO_NODE*)malloc(
          sizeof(FT_DEVICE_LIST_INFO_NODE)*number_of_devices
        );
      try {
        result = FT_GetDeviceInfoList(device_info, &number_of_devices);
      } catch(std::exception &e) {
        RMP_THROW_MSG(ReadFailedException, e.what());
      }
      if (result != FT_OK) {
        RMP_THROW_MSG(ReadFailedException,
          getErrorMessageByFT_STATUS(result, "enumerating devices").c_str());
      }
      for (int i = 0; i < number_of_devices; i++) {
          devices.push_back(device_info[i]);
        if (ftd2xx_devices_debug) {
          printf("Dev %d:\n", i);
          printf(" Flags=0x%x\n", device_info[i].Flags);
          printf(" Type=0x%x\n", device_info[i].Type);
          printf(" ID=0x%x\n", device_info[i].ID);
          printf(" LocId=0x%x\n", device_info[i].LocId);
          printf(" SerialNumber=%s\n", device_info[i].SerialNumber);
          printf(" Description=%s\n", device_info[i].Description);
        }
      }
    }
    return devices;
}

/////////////////////////////////////////////////////////////////////////////
// FTD2XXRMPIO

FTD2XXRMPIO::FTD2XXRMPIO()
: configured(false), config_type(by_none), baudrate(460800), is_open(false)
{
  this->port_serial_number = "";
  this->port_description = "";
  this->port_index = 0;
  this->connected = false;
  this->usb_port_handle = NULL;
}

FTD2XXRMPIO::~FTD2XXRMPIO() {
  this->disconnect();
}

void FTD2XXRMPIO::connect() {
  if(!this->configured) {
    RMP_THROW_MSG(ConnectionFailedException, "The usb port must be "
      "configured before connecting.");
  }
  
  FT_STATUS result;
  
  this->enumerateUSBDevices_();
  
  // Select connection method and open
  switch (this->config_type) {
    case by_serial_number:
      this->connectBySerial();
      break;
    case by_description:
      this->connectByDescription();
      break;
    case by_index:
      this->connectByIndex();
      break;
    case by_none:
    default:
      RMP_THROW_MSG(ConnectionFailedException, "The usb port must be "
        "configured before connecting.");
        break;
  }
  
  this->is_open = true;
  
  // Configure the Baudrate
  try {
    result = FT_SetBaudRate(this->usb_port_handle, this->baudrate);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "setting the baudrate").c_str());
  }
  
  // Set default timeouts
  try {
    // 1 sec read and 1 sec write
    result = FT_SetTimeouts(this->usb_port_handle, 1000, 1000);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "setting timeouts").c_str());
  }
  
  // Set Latency Timer
  try {
    result = FT_SetLatencyTimer(this->usb_port_handle, 1);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "setting latency timer").c_str());
  }
  
  // Set flowcontrol
  try {
    result = FT_SetFlowControl(this->usb_port_handle, FT_FLOW_NONE, 0, 0);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "setting flowcontrol").c_str());
  }
  
  // Purge the I/O buffers of the usb device
  try {
    result = FT_Purge(this->usb_port_handle, FT_PURGE_RX | FT_PURGE_TX);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "purging the buffers").c_str());
  }
  
  this->connected = true;
}

void FTD2XXRMPIO::disconnect() {
  if(this->connected) {
    if (this->is_open) {
      FT_Close(this->usb_port_handle);
    }
    this->connected = false;
  }
}

int FTD2XXRMPIO::read(unsigned char* buffer, int size) {
  FT_STATUS result;
  DWORD bytes_read;
  
  try {
    result = FT_Read(this->usb_port_handle, buffer, size, &bytes_read);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ReadFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ReadFailedException,
      getErrorMessageByFT_STATUS(result, "reading").c_str());
  }
  
  return bytes_read;
}

int FTD2XXRMPIO::write(unsigned char* buffer, int size) {
  FT_STATUS result;
  DWORD bytes_written;
  
  try {
    result = FT_Write(this->usb_port_handle, buffer, size, &bytes_written);
  } catch(std::exception &e) {
    RMP_THROW_MSG(ReadFailedException, e.what());
  }
  if (result != FT_OK) {
    RMP_THROW_MSG(ReadFailedException,
      getErrorMessageByFT_STATUS(result, "reading").c_str());
  }
  
  return bytes_written;
}

std::vector<FT_DEVICE_LIST_INFO_NODE> FTD2XXRMPIO::enumerateUSBDevices_() {
  return enumerateUSBDevices();
}

void
FTD2XXRMPIO::configureUSBBySerial(std::string serial_number, int baudrate)
{
  this->config_type = by_serial_number;
  this->port_serial_number = serial_number;
  this->configured = true;
  this->baudrate = baudrate;
}

void FTD2XXRMPIO::connectBySerial() {
  FT_STATUS result;
  DWORD number_of_devices;
  
  try {
    // Open the usb port
    result = FT_OpenEx((PVOID)this->port_serial_number.c_str(),
                       (DWORD)FT_OPEN_BY_SERIAL_NUMBER,
                       &(this->usb_port_handle));
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (this->usb_port_handle == NULL) {
    // Failed to open port
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "opening the usb port").c_str());
  }
}

void
FTD2XXRMPIO::configureUSBByDescription(std::string description, int baudrate)
{
  this->config_type = by_description;
  this->port_description = description;
  this->configured = true;
  this->baudrate = baudrate;
}

void FTD2XXRMPIO::connectByDescription() {
  FT_STATUS result;
  
  try {
    // Open the usb port
    result = FT_OpenEx(const_cast<char *>(this->port_description.c_str()), 
                       FT_OPEN_BY_DESCRIPTION, 
                       &(this->usb_port_handle));
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (result != FT_OK) {
    // Failed to open port
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "opening the usb port").c_str());
  }
}

void
FTD2XXRMPIO::configureUSBByIndex(unsigned int device_index, int baudrate)
{
  this->config_type = by_index;
  this->port_index = device_index;
  this->configured = true;
  this->baudrate = baudrate;
}

void FTD2XXRMPIO::connectByIndex() {
  FT_STATUS result;
  
  try {
    // Open the usb port
    result = FT_Open(this->port_index, &(this->usb_port_handle));
  } catch(std::exception &e) {
    RMP_THROW_MSG(ConnectionFailedException, e.what());
  }
  if (this->usb_port_handle == NULL) {
    // Failed to open port
    RMP_THROW_MSG(ConnectionFailedException,
      getErrorMessageByFT_STATUS(result, "opening the usb port").c_str());
  }
}


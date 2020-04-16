#include <CyUSBSerial/CyController.hpp>
#include <mutex>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>

#define CY_MAX_DEVICES 30
#define CY_MAX_INTERFACES 4
#define I2C_TIMEOUT_MILLISECONDS 500

namespace cyusb
{
namespace detail
{
struct RefCounter
{
    size_t ref{0};
    mutable std::mutex mutex;
    RefCounter &operator++()
    {
        std::unique_lock<std::mutex> ul(mutex);
        if (ref == 0)
        {
            auto rStatus = CyLibraryInit();
            if (rStatus != CY_SUCCESS)
            {
                std::ostringstream str;
                str << "Unable to initialize CyUSBSerial Library: " << CyController::error_message(rStatus) << " " << __FILE__ << " : " << __LINE__ << std::endl;
                throw std::runtime_error(str.str().c_str());
            }
            ref++;
        }
        return *this;
    }

    RefCounter &operator--()
    {
        std::unique_lock<std::mutex> ul(mutex);
        if (ref != 0)
        {
            ref--;
            if (ref == 0)
            {
                auto rStatus = CyLibraryExit();
                if (rStatus != CY_SUCCESS)
                {
                    std::ostringstream str;
                    str << "Unable to properly exit CyUSBSerial Library: " << CyController::error_message(rStatus) << " " << __FILE__ << " : " << __LINE__ << std::endl;
                    throw std::runtime_error(str.str().c_str());
                }
            }
        }
        return *this;
    }
};
} // namespace detail

static detail::RefCounter g_cy_object_counter;

/* -------------------------------------------------------------------------- */
/*                                Cy Controller                               */
/* -------------------------------------------------------------------------- */

CyController::CyController() : mWorkingDevice{nullptr},
                               mIsInitialized{false}
{
    ++g_cy_object_counter;
}

CyController::~CyController()
{
    --g_cy_object_counter;
}

bool CyController::initialize()
{
    int numDevices = get_number_of_devices();
    if (numDevices > 0)
    {

        for (uint8_t DeviceIdx = 0; DeviceIdx < numDevices; DeviceIdx++)
        {
            mDeviceList.emplace_back();
            mDeviceList.back().device_number = DeviceIdx;
            auto rStatus = CyGetDeviceInfo(DeviceIdx, &mDeviceList.back().info);
            if (rStatus != CY_SUCCESS)
            {
                // std::cerr << "CyUSBSerial Error: " << error_message(rStatus) << " " << __FILE__ << " : " << __LINE__ << std::endl;
                mDeviceList.pop_back();
                continue;
            }
            bool good_device{false};
            for (uint8_t InterfaceIdx = 0; InterfaceIdx < mDeviceList.back().info.numInterfaces; InterfaceIdx++)
            {
                if (mDeviceList.back().info.deviceType[InterfaceIdx] == CY_TYPE_I2C)
                {
                    good_device = true;
                    mDeviceList.back().interface_number = InterfaceIdx;
                    mDeviceList.back().serial = std::string(reinterpret_cast<const char *>(mDeviceList.back().info.serialNum));
                    // std::cout << " Found Device Serial : " << mDeviceList.back().serial << std::endl;
                    // printf("found I2C DeviceIndex: %d, InterfaceIndex: %d  Serial_number: %s \n", DeviceIdx, InterfaceIdx, DeviceInfo.serialNum);
                }
            }
            if (!good_device)
                mDeviceList.pop_back();
        }
        return mIsInitialized = !mDeviceList.empty();
    }
    std::cout << " No devices found :"
              << " " << __FILE__ << " : " << __LINE__ << std::endl;
    return mIsInitialized = false;
}

std::unique_ptr<CyController::Device> CyController::get_working_device()
{
    return std::move(mWorkingDevice);
}

void CyController::set_working_device(std::unique_ptr<Device> wd)
{
    mWorkingDevice = std::move(wd);
}

void CyController::set_desired_serial(const std::string &desired_serial)
{
    mDesiredSerial = desired_serial;
}

const std::string &CyController::get_desired_serial() const
{
    return mDesiredSerial;
}
bool CyController::set_working_device()
{
    auto result1 = std::find_if(mDeviceList.begin(), mDeviceList.end(), [&](const auto &device) -> bool { return mDesiredSerial.compare(device.serial) == 0 ? true : false; });
    if (result1 != mDeviceList.end())
    {
        if (!close_working_device())
            return false;
        mWorkingDevice = std::make_unique<Device>(*result1);
        return true;
    }
    std::ostringstream str;
    str << "Could not find device with serial:  " << mDesiredSerial << __FILE__ << " : " << __LINE__ << std::endl;
    std::cout << str.str() << std::endl;
    return false;
}

bool CyController::set_working_device_by_serial(const std::string &serial)
{
    auto result1 = std::find_if(mDeviceList.begin(), mDeviceList.end(), [serial](const auto &device) -> bool { return serial.compare(device.serial) == 0 ? true : false; });
    if (result1 != mDeviceList.end())
    {
        if (!close_working_device())
            return false;
        mWorkingDevice = std::make_unique<Device>(*result1);
        mDesiredSerial = serial;
        return true;
    }
    std::ostringstream str;
    str << "Could not find device with serial:  " << serial << __FILE__ << " : " << __LINE__ << std::endl;
    std::cout << str.str() << std::endl;
    return false;
}
bool CyController::open_working_device() const
{
    if (!mWorkingDevice)
    {
        std::ostringstream str;
        str << "Working device is not set! " << __FILE__ << " : " << __LINE__ << std::endl;
        std::cout << str.str() << std::endl;
        return false;
    }
    if (mWorkingDevice->is_open)
        return true;

    auto rStatus = CyOpen(mWorkingDevice->device_number, mWorkingDevice->interface_number, &mWorkingDevice->handle);
    if (rStatus != CY_SUCCESS)
    {
        std::ostringstream str;
        str << "Unable to open working device :: CyUSBSerial error (No): " << rStatus << "\n"
            << " at " << __FILE__ << " : " << __LINE__ << std::endl;
        std::cout << str.str() << std::endl;
        return false;
    }
    mWorkingDevice->is_open = true;
    return true;
}

bool CyController::close_working_device() const
{
    if (!is_initialized())
        return false;
    if (mWorkingDevice)
    {
        if (is_working_device_open())
        {
            auto rStatus = CyClose(mWorkingDevice->handle);
            if (rStatus != CY_SUCCESS)
            {
                std::ostringstream str;
                str << "Unable to close working device :: CyUSBSerial error (No): " << rStatus << "\n"
                    << " at " << __FILE__ << " : " << __LINE__ << std::endl;
                std::cout << str.str() << std::endl;
                return false;
            }
            mWorkingDevice->is_open = false;
        }
    }
    return true;
}

bool CyController::is_initialized() const
{
    return mIsInitialized;
}

bool CyController::is_working_device_open() const
{
    if (mWorkingDevice)
        return mWorkingDevice->is_open;
    return false;
}

bool CyController::is_working_device_set() const
{
    return mWorkingDevice ? true : false;
}

int CyController::number_of_stored_device() const
{
    return mDeviceList.size();
}

void CyController::clear_stored_devices()
{
    mDeviceList.clear();
    mIsInitialized = false;
    close_working_device();
    mWorkingDevice.reset();
}

int CyController::get_number_of_devices() const
{
    uint8_t numDevices_;
    auto rStatus = CyGetListofDevices(&numDevices_);
    if (rStatus != CY_SUCCESS)
    {
        std::ostringstream str;
        str << "CyUSBSerial error (No): " << rStatus << "\n"
            << " at " << __FILE__ << " : " << __LINE__ << std::endl;
        std::cout << str.str() << std::endl;
        return -1;
    }
    return static_cast<int>(numDevices_);
}

size_t CyController::get_gpio_value(const size_t &gpioNumber) const
{
    if (!open_working_device())
    {
        std::ostringstream str;
        str << "Unable to open working device: "
            << " at " << __FILE__ << " : " << __LINE__ << std::endl;
        throw std::runtime_error(str.str().c_str());
    }
    uint8_t value;
    auto rStatus = CyGetGpioValue(mWorkingDevice->handle, static_cast<uint8_t>(gpioNumber), &value);
    if (rStatus != CY_SUCCESS)
    {
        std::ostringstream str;
        str << "Unable to get GPIO value: "
            << " at " << __FILE__ << " : " << __LINE__ << std::endl;
        throw std::runtime_error(str.str().c_str());
    }
    return static_cast<size_t>(value);
}

bool CyController::set_gpio_value(const size_t &gpioNumber, const size_t &value) const
{
    if (!open_working_device())
    {
        std::ostringstream str;
        str << "Unable to open working device: "
            << " at " << __FILE__ << " : " << __LINE__ << std::endl;
        throw std::runtime_error(str.str().c_str());
    }
    auto rStatus = CySetGpioValue(mWorkingDevice->handle, static_cast<uint8_t>(gpioNumber), static_cast<uint8_t>(value));
    if (rStatus != CY_SUCCESS)
    {
        std::ostringstream str;
        str << "Unable to get GPIO value: "
            << " at " << __FILE__ << " : " << __LINE__ << std::endl;
        throw std::runtime_error(str.str().c_str());
    }
    return true;
}

bool CyController::get_gpio_state(const size_t &gpioNumber) const
{
    get_gpio_value(gpioNumber) == 0 ? false : true;
}

bool CyController::switch_gpio_state(const size_t &gpioNumber) const
{
    return set_gpio_state(gpioNumber, !get_gpio_state(gpioNumber));
}

bool CyController::set_gpio_state(const size_t &gpioNumber, bool newState) const
{
    set_gpio_value(gpioNumber, newState ? 1 : 0);
    return newState;
}

std::string CyController::error_message(CY_RETURN_STATUS status)
{
    switch (status)
    {
    case CY_SUCCESS:
        return "CY_SUCCESS";
    case CY_ERROR_ACCESS_DENIED:
        return "CY_ERROR_ACCESS_DENIED";
    case CY_ERROR_DRIVER_INIT_FAILED:
        return "CY_ERROR_DRIVER_INIT_FAILED";
    case CY_ERROR_DEVICE_INFO_FETCH_FAILED:
        return "CY_ERROR_DEVICE_INFO_FETCH_FAILED";
    case CY_ERROR_DRIVER_OPEN_FAILED:
        return "CY_ERROR_DRIVER_OPEN_FAILED";
    case CY_ERROR_INVALID_PARAMETER:
        return "CY_ERROR_INVALID_PARAMETER";
    case CY_ERROR_REQUEST_FAILED:
        return "CY_ERROR_REQUEST_FAILED";
    case CY_ERROR_DOWNLOAD_FAILED:
        return "CY_ERROR_DOWNLOAD_FAILED";
    case CY_ERROR_FIRMWARE_INVALID_SIGNATURE:
        return "CY_ERROR_FIRMWARE_INVALID_SIGNATURE";
    case CY_ERROR_INVALID_FIRMWARE:
        return "CY_ERROR_INVALID_FIRMWARE";
    case CY_ERROR_DEVICE_NOT_FOUND:
        return "CY_ERROR_DEVICE_NOT_FOUND";
    case CY_ERROR_IO_TIMEOUT:
        return "CY_ERROR_IO_TIMEOUT";
    case CY_ERROR_PIPE_HALTED:
        return "CY_ERROR_PIPE_HALTED";
    case CY_ERROR_BUFFER_OVERFLOW:
        return "CY_ERROR_BUFFER_OVERFLOW";
    case CY_ERROR_INVALID_HANDLE:
        return "CY_ERROR_INVALID_HANDLE";
    case CY_ERROR_ALLOCATION_FAILED:
        return "CY_ERROR_ALLOCATION_FAILED";
    case CY_ERROR_I2C_DEVICE_BUSY:
        return "CY_ERROR_I2C_DEVICE_BUSY";
    case CY_ERROR_I2C_NAK_ERROR:
        return "CY_ERROR_I2C_NAK_ERROR";
    case CY_ERROR_I2C_ARBITRATION_ERROR:
        return "CY_ERROR_I2C_ARBITRATION_ERROR";
    case CY_ERROR_I2C_BUS_ERROR:
        return "CY_ERROR_I2C_BUS_ERROR";
    case CY_ERROR_I2C_BUS_BUSY:
        return "CY_ERROR_I2C_BUS_BUSY";
    case CY_ERROR_I2C_STOP_BIT_SET:
        return "CY_ERROR_I2C_STOP_BIT_SET";
    case CY_ERROR_STATUS_MONITOR_EXIST:
        return "CY_ERROR_STATUS_MONITOR_EXIST";
    default:
        return "Unknown error";
    }
}

} // namespace cyusb

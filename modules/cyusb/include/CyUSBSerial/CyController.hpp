
#ifndef PATH_CYPRESS_USB_SDK_CY_CONTRTOLLER_HPP
#define PATH_CYPRESS_USB_SDK_CY_CONTRTOLLER_HPP

#include <CyUSBSerial/CyUSBCommon.hpp>
#include <string>
#include <memory>
#include <vector>
#include <mutex>

namespace cyusb {


    struct Device {
        Device() = default;
        uint8_t device_number;
        uint8_t interface_number;
        std::string serial;
        std::vector<int> interface_functions;
        bool is_i2c{false};
        bool is_spi{false};
        CY_DEVICE_INFO info;
        CY_HANDLE handle;
        bool is_open{false};
        mutable std::mutex device_mutex;
    };


    class CyController
    {
    public:
    using Ptr = std::shared_ptr<CyController>;                     
	using ConstPtr = std::shared_ptr<CyController>;        
	using UniquePtr = std::unique_ptr<CyController>;             
	using ConstUniquePtr = std::unique_ptr<CyController>;  
                                                                          
	template <typename... Args>                                          
	static Ptr Create(Args &&... args)                                    
	{                                                                     
		return std::make_shared<CyController>(std::forward<Args>(args)...);
	}                                                                     
	template <typename Alloc, typename... Args>                           
	static Ptr CreateAlloc(const Alloc &alloc, Args &&... args)           
	{                                                                     
		return std::allocate_shared<CyController>(                         
			alloc, std::forward<Args>(args)...);                          
	}                                                                    
	template <typename... Args>                                           
	static UniquePtr CreateUnique(Args &&... args)                        
	{                                                                     
		return std::make_unique<CyController>(std::forward<Args>(args)...); 
	}                                                                     

    public:

    protected:
        bool mIsInitialized;
        std::string mDesiredSerial;
        std::shared_ptr<Device> mWorkingDevice;
    public:
        CyController();

        CyController(std::shared_ptr<Device> device);

        ~CyController();

        bool initialize();

        std::shared_ptr<Device> get_working_device();

        void set_working_device(std::shared_ptr<Device> wd);

        void set_desired_serial(const std::string &desired_serial);

        const std::string &get_desired_serial() const;

        bool set_working_device();

        bool set_working_device_by_serial(const std::string &serial);

        bool open_working_device() const;

        bool close_working_device() const;

        bool is_initialized() const;

        bool is_working_device_open() const;

        bool is_working_device_set() const;

        int number_of_stored_device() const;

        void clear_stored_devices();

        int get_number_of_devices() const;

        size_t get_gpio_value(const size_t &gpioNumber) const;

        bool set_gpio_value(const size_t &gpioNumber, const size_t &value) const;

        bool get_gpio_state(const size_t &gpioNumber) const;

        bool switch_gpio_state(const size_t  &gpioNumber) const;

        bool set_gpio_state(const size_t &gpioNumber, bool newState) const;

        static std::string error_message(CY_RETURN_STATUS);
    };
}

#endif
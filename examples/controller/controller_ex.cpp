#include <CyUSBSerial/CyController.hpp>
#include <iostream>

int main (int argc, char **agrv)
{
    cyusb::CyController controller;

    controller.initialize();

    std::string serial = "RTF001";
    controller.set_working_device_by_serial(serial);

    std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << controller.get_gpio_value(2) <<std::endl;
    
    controller.switch_gpio_state(5);
    std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << controller.get_gpio_value(2) <<std::endl;

    return 0;
}
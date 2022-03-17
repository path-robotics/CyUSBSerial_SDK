#include <CyUSBSerial/CyController.hpp>
#include <iostream>

int main(int argc, char** agrv)
{
  cyusb::CyController controller, controller2;

  if (!controller.initialize() || controller2.initialize())
  {
    std::cout << "Failed to initialize CyUSB controllers--Aborting!";
    return -1;
  }

  const std::string str_serial = "RTF001";
  controller.set_working_device_by_serial(str_serial);
  controller2.set_working_device_by_serial(str_serial);

  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << controller.get_gpio_value(5) << std::endl;

  controller.switch_gpio_state(5);
  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << controller.get_gpio_value(5) << std::endl;

  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << controller2.get_gpio_value(2) << std::endl;

  controller2.switch_gpio_state(2);
  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << controller2.get_gpio_value(2) << std::endl;

  return 0;
}

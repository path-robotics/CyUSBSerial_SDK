/*
 * Test utility
 * Copyright (C) 2013  Cypress Semiconductor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include <CyUSBSerial/CyController.hpp>
#include <iostream>

int main (int argc, char **agrv)
{
    cyusb::CyController controller;

    controller.initialize();

    std::string serial = "RTF001";
    controller.set_working_device_by_serial(serial);

    std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << controller.get_gpio_value(5) <<std::endl;
    
    controller.switch_gpio_state(5);
    std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << controller.get_gpio_value(5) <<std::endl;

    return 0;
}
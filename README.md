# CyUSBSerial_SDK
Installer repository for Cypress USB serial controller drivers.

Specifically creates a CMake debian installer, but there's no reason this project can't be used on other platforms with some modification.

To make an installable linux package

```shell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) package
```

This should create a `*.deb` file in the folder. To install, just run
`sudo apt install ./*.deb`

You should now be able to find the package in CMake using
```cmake
find_package(CyUSBSerial 1.2 REQUIRED COMPONENTS cyusb)
```
the version number is optional and the version used in this example may not reflect the version you install.

To link against the library in CMake, use

```cmake
target_link_libraries(name_of_library PRIVATE CyUSBSerial::cyusb)
```

After doing so you should be able to include and use files such as 
```c++
#include <CyUSBSerial/CyUSBSerial.hpp>
```

To uninstall
`sudo apt remove cyusbserial`

# CyUsb Tools

This project installs the following tools alongside the Cypress drivers:
- `lscygpio`
- `cygpioget`
- `cygpioset`
- `cygpio-hammer`

## CyUsb Tool Usage
### lscygpio
```
Usage: lscygpio [OPTIONS]...

    List CyUSB GPIO chips, lines and states

    Options:
        -n <name>    List GPIOs on a named device (gpiochip number)
        -h, --help   This helptext
```
### cygpioget
```
Usage: cygpioget [OPTIONS] <gpiochip-num> <offset1> [<offset2>]...

    Read line value(s) from a GPIO chip

    Options:
        -h, --help        Display this message and exit

    Example:
        cygpioget gpiochip0 2 3 4 5
```
### cygpioset
```
Usage: cygpioset [OPTIONS] <gpiochip-num> <offset1>=[0|1] [<offset2>=[0|1]]...

    Set GPIO line values of a GPIO chip and maintain the state until the process exits

    Options:
        -h, --help        Display this message and exit

    Example:
        cygpioset gpiochip0 2=0 3=1 4=1 5=0
```

### cygpio-hammer
```
Usage: cygpio-hammer [options]...
    Hammer CyUSB GPIO lines, 0->1->0->1...
        -n <name>    Hammer CyUSB GPIOs on a named device (must be stated)
        -o <n>       Offset[s] to hammer, at least one, several can be stated
    [-c <n>]      Do <n> loops (optional, infinite loop if not stated)
        -h, --help   This helptext\n"

    Example:
        cygpio-hammer -n gpiochip0 -o 4
```
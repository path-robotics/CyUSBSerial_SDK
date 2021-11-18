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

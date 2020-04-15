/*
 * Test utility
 * Copyright (C) 2013 Cypress Semiconductor
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


#include <CyUSBSerial/CyUSBSerial.h>
#include <string>
#include <memory>

namespace cyusb {

    class CyController
    {
    public:
        struct Device {
            Device() = default;

            int device_number;
            std::vector<int> interface_functions;
            bool is_i2c{false};
            bool is_spi{false};
            int numInterface;
            CY_DEVICE_INFO info;
        };

    protected:
        std::string mDesiredSerial;
        D


    public:
        CyController();

        ~CyController();

        bool initialize();

        bool get_device_by_serial

        int get_number_of_devices();

        size_t get_gpio_value(const size_t &gpioNumber) const;

        bool set_gpio_value(const size_t &gpioNumber, const size_t &value);






    };
}


#define CY_MAX_DEVICES 30
#define CY_MAX_INTERFACES 4
#define I2C_TIMEOUT_MILLISECONDS   500


CY_DEVICE_STRUCT *glDevice;
int i2cDeviceIndex[CY_MAX_DEVICES][CY_MAX_INTERFACES];
unsigned char *deviceNumber = NULL;
int cyDevices, i2cDevices = 0, numDevices = 0;
int selectedDeviceNum = -1, selectedInterfaceNum = -1;
bool exitApp = false;
unsigned short pageAddress = -1;
short readWriteLength = -1;
bool deviceAddedRemoved = false;



void passNumber (int devicenumber, int interfacenumber) {

    uint8_t value=5;
    CY_RETURN_STATUS rStatus;
    CY_HANDLE handle;

    rStatus = CyOpen (devicenumber, interfacenumber, &handle);
    if (rStatus != CY_SUCCESS)
    {
        printf("I2C open fail \n");
    }

    rStatus=CyGetGpioValue(handle, 5, &value);
    if (rStatus != CY_SUCCESS)
    {
        printf("GPIO reading failed \n");
    }

    printf("Device 2 GPIO 5 data: %d  \n", value);

    CyClose(handle);

}

void passHandle (CY_HANDLE handle) {

    uint8_t value=5;
    CY_RETURN_STATUS rStatus;

    rStatus=CyGetGpioValue(handle, 5, &value);
    if (rStatus != CY_SUCCESS)
    {
        printf("GPIO reading failed \n");
    }

    printf("Pass Device 2 GPIO 5 data: %d  \n", value);

    CyClose(handle);

}

int main (int argc, char **agrv)
{
    int index = 0, i, j, userInput;
    int tempSelectedDeviceNum, tempSelectedInterfaceNum, tempPageAddress, tempLength;
    CY_RETURN_STATUS rStatus;
    signal (SIGUSR1, deviceHotPlug);
    glDevice = (CY_DEVICE_STRUCT *)malloc (CY_MAX_DEVICES *sizeof (CY_DEVICE_STRUCT));
    if (glDevice == NULL){
        printf ("Memory allocation failed ...!! \n");
        return -1;
    }
    rStatus = CyLibraryInit ();
    if (rStatus != CY_SUCCESS) {
        printf ("CY:Error in Doing library init Error NO:<%d> \n", rStatus);
        return rStatus;
    }
    uint8_t numDevices_;
    rStatus = CyGetListofDevices (&numDevices_);
    if (rStatus != CY_SUCCESS) {
        printf ("CY:Error in Getting List of Devices: Error NO:<%d> \n", rStatus);
        return rStatus;
    }
    numDevices = static_cast<int>(numDevices_);
    printf("Number of Decice:  %d ",numDevices);
    printListOfDevices(true);

    printf ("-------------------------------------------------------------------\n");



    CY_HANDLE handle, handle2;

    CY_DEVICE_INFO   DeviceInfo;



    int          DeviceIdx;
    int          InterfaceIdx;
    int          devicenumber[2], interfacenum[2];

    int	     indexNumber=0;


    rStatus = CyGetListofDevices(&numDevices_);
    numDevices = static_cast<int>(numDevices_);
    printf("Number of Decice:  %d  \n:",numDevices);
    if ((rStatus != CY_SUCCESS) || (numDevices == 0))
    {
        printf(" Did not get it");
    }

    for (DeviceIdx = 0; DeviceIdx < numDevices; DeviceIdx++)
    {
        rStatus = CyGetDeviceInfo(DeviceIdx, &DeviceInfo);
        if (rStatus != CY_SUCCESS)
        {
            continue;
        }


        for (InterfaceIdx = 0; InterfaceIdx < DeviceInfo.numInterfaces; InterfaceIdx++)
        {
            if (DeviceInfo.deviceType[InterfaceIdx] == CY_TYPE_I2C)
            {
                devicenumber[indexNumber]=DeviceIdx;
                interfacenum[indexNumber]=InterfaceIdx;
                printf("found I2C DeviceIndex: %d, InterfaceIndex: %d  Serial_number: %s \n", DeviceIdx, InterfaceIdx, DeviceInfo.serialNum);
                indexNumber++;

            }
        }
    }



    rStatus = CyOpen (devicenumber[0], interfacenum[0], &handle);
    if (rStatus != CY_SUCCESS)
    {
        std::cout << "I2C one open fail:  " << rStatus<<std::endl;
    }


    uint8_t value=5;

    rStatus=CyGetGpioValue(handle, 5, &value);
    if (rStatus != CY_SUCCESS)
    {
        printf("GPIO reading failed \n");
    }

    printf("Device 1 GPIO 5 data: %d  \n", value);


    if(value==0)
    {
        rStatus=CySetGpioValue(handle, 5, 1);
        if (rStatus != CY_SUCCESS)
        {
            printf("GPIO setting failed \n");
        }
    }

    if(value==1)
    {
        rStatus=CySetGpioValue(handle, 5, 0);
        if (rStatus != CY_SUCCESS)
        {
            printf("GPIO setting failed \n");
        }
    }

    rStatus=CyGetGpioValue(handle, 5, &value);
    if (rStatus != CY_SUCCESS)
    {
        printf("GPIO reading failed \n");
    }

    printf(" GPIO 5 data: %d  \n", value);

/*
        // liquid lens I2C address 0x18

        CY_I2C_CONFIG cyI2CConfig;
        CY_DATA_BUFFER cyDataBuffer;
        CY_I2C_DATA_CONFIG cyI2CDataConfig;
        unsigned char readBuffer[4096], writeBuffer[4096];
        UINT32 timeout = 5000;

        cyI2CDataConfig.slaveAddress = 0x18;
        cyI2CDataConfig.isNakBit = true;
        cyI2CDataConfig.isStopBit = true;


        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x00; // register 0
        cyDataBuffer.buffer[1] = 0xff; //
        cyDataBuffer.buffer[2] = 0x55; //

        cyDataBuffer.length = 3;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting liquid lens \n");
        }

   */

/*
       // Open second device, test!!

       rStatus = CyOpen (devicenumber[1], interfacenum[1], &handle2);
       if (rStatus != CY_SUCCESS)
       {
           printf("I2C open fail \n");
       }


       printf("test \n");

       passNumber(devicenumber[1], interfacenum[1]);



       rStatus = CyOpen (devicenumber[1], interfacenum[1], &handle2);
       if (rStatus != CY_SUCCESS)
       {
           printf("I2C two open fail here: %d  \n", rStatus);
       }

       passHandle(handle2);

*/



/*

       //This section is to setup I2C communication

       int value=5;

       rStatus=CyGetGpioValue(handle, 5, &value);
       if (rStatus != CY_SUCCESS)
       {
           printf("GPIO reading failed \n");
       }

       printf("Device 1 GPIO 5 data: %d  \n", value);


       if(value==0)
       {
            rStatus=CySetGpioValue(handle, 5, 1);
            if (rStatus != CY_SUCCESS)
            {
                printf("GPIO setting failed \n");
            }
        }

        rStatus=CyGetGpioValue(handle, 5, &value);
        if (rStatus != CY_SUCCESS)
        {
           printf("GPIO reading failed \n");
        }

        printf(" GPIO 5 data: %d  \n", value);

        time_t  StartTime = time(NULL);

        value=0;

        while(value==0)
        {
          rStatus=CyGetGpioValue(handle, 6, &value);
          if (rStatus != CY_SUCCESS)
           {
           printf("GPIO reading failed \n");
           }
        }
        printf(" GPIO 6 data: %d  \n", value);

        rStatus=CyGetGpioValue(handle, 9, &value);
        if (rStatus != CY_SUCCESS)
        {
           printf("GPIO reading failed \n");
        }

        printf(" GPIO 9 data: %d  \n", value);

        if(value==0)
        {
            rStatus=CySetGpioValue(handle, 9, 1);
            if (rStatus != CY_SUCCESS)
            {
                printf("GPIO setting failed \n");
            }
        }

        rStatus=CyGetGpioValue(handle, 9, &value);
        if (rStatus != CY_SUCCESS)
        {
           printf("GPIO reading failed \n");
        }

        printf(" GPIO 9 data: %d  \n", value);

*/


/*
        //Trigger setting

        CY_I2C_CONFIG cyI2CConfig;
        CY_DATA_BUFFER cyDataBuffer;
        CY_I2C_DATA_CONFIG cyI2CDataConfig;
        unsigned char readBuffer[4096], writeBuffer[4096];
        UINT32 timeout = 5000;

        cyI2CDataConfig.slaveAddress = 0x1B;
        cyI2CDataConfig.isNakBit = true;
        cyI2CDataConfig.isStopBit = true;


        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x92; // trigger out setting
        cyDataBuffer.buffer[1] = 0x03; // selete trigger 2 (bit0=1)  enable (bit1=1) Not inverted (bit2=0) 00000-011
        cyDataBuffer.buffer[2] = 0x00; // Not delay 4 bytes
        cyDataBuffer.buffer[3] = 0x00;
        cyDataBuffer.buffer[4] = 0x00;
        cyDataBuffer.buffer[5] = 0x00;

        cyDataBuffer.length = 6;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting trigger \n");
        }

        sleep(0.5);
*/


    /*
           // read LED current setting

           memset(writeBuffer, 0x00, 4096);
           cyDataBuffer.buffer = writeBuffer;

           memset(readBuffer, 0x00, 4096);

           cyDataBuffer.buffer[0] = 0x55; // read
           cyDataBuffer.length = 1;

           rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

           if (rStatus != CY_SUCCESS)
           {
              printf("Read LED Current \n");
           }

           cyDataBuffer.buffer = readBuffer;
           cyDataBuffer.buffer[0] = 0;
           cyDataBuffer.buffer[1] = 0;
           cyDataBuffer.buffer[2] = 0;
           cyDataBuffer.buffer[3] = 0;
           cyDataBuffer.buffer[4] = 0;
           cyDataBuffer.buffer[5] = 0;

           cyDataBuffer.length = 6;

           rStatus = CyI2cRead(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

           if (rStatus != CY_SUCCESS)
           {
              printf("Read LED Current \n");
           }
           printf("LED Current: R= %d , %d \n", cyDataBuffer.buffer[0], cyDataBuffer.buffer[1]);
           printf("LED Current: G= %d , %d \n", cyDataBuffer.buffer[2], cyDataBuffer.buffer[3]);
           printf("LED Current: B= %d , %d \n", cyDataBuffer.buffer[4], cyDataBuffer.buffer[5]);

           // Write LED enabel, may not need to do it

           memset(writeBuffer, 0x00, 4096);
           cyDataBuffer.buffer = writeBuffer;

           cyDataBuffer.buffer[0] = 0x52; // Write LED enable
           cyDataBuffer.buffer[1] = 0x07; //

           cyDataBuffer.length = 2;

           rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

           if (rStatus != CY_SUCCESS)
           {
              printf("Write LED Enable \n");
           }

           sleep(1);


           // For new projectors, they are already set to Max current in default

           memset(writeBuffer, 0x00, 4096);
           cyDataBuffer.buffer = writeBuffer;

           cyDataBuffer.buffer[0] = 0x5C; // Setting LED max current
           cyDataBuffer.buffer[1] = 0xFF; // R 0x03ff = 1023 low byte first then high byte
           cyDataBuffer.buffer[2] = 0x03; // R
           cyDataBuffer.buffer[3] = 0xFF; // G
           cyDataBuffer.buffer[4] = 0x03; // G
           cyDataBuffer.buffer[5] = 0xFF; // B
           cyDataBuffer.buffer[6] = 0x03; // B

           cyDataBuffer.length = 7;

           rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

           if (rStatus != CY_SUCCESS)
           {
              printf("Setting LED Max Current \n");
           }

           sleep(1);


           // not used

           memset(writeBuffer, 0x00, 4096);
           cyDataBuffer.buffer = writeBuffer;

           cyDataBuffer.buffer[0] = 0x54; // LED current setting to tested stable value
           cyDataBuffer.buffer[1] = 0x20; // R 0x02BC=700, low byte first then high byte
           cyDataBuffer.buffer[2] = 0x03; // R
           cyDataBuffer.buffer[3] = 0x20; // G 0x0320=800
           cyDataBuffer.buffer[4] = 0x03; // G
           cyDataBuffer.buffer[5] = 0x20; // B 0x02BC=700
           cyDataBuffer.buffer[6] = 0x03; // B

           cyDataBuffer.length = 7;

           rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

           if (rStatus != CY_SUCCESS)
           {
              printf("Setting LED Current \n");
           }

           sleep(1);
   */

/*      test run here

        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x05;  // Choose test pattern
        cyDataBuffer.buffer[1] = 0x01;  // TestPatternGenerator

        cyDataBuffer.length = 2;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting solid image \n");
        }

        sleep(1);

        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x0B; // Display solid image
        cyDataBuffer.buffer[1] = 0x00; // No boarder
        cyDataBuffer.buffer[2] = 0x60; // white

        cyDataBuffer.length = 3;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting solid white image \n");
        }

        sleep(5);


        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x05;
        cyDataBuffer.buffer[1] = 0x04; //display internal patterns
        cyDataBuffer.length = 2;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

         if (rStatus != CY_SUCCESS)
        {
           printf("display internal pattern \n");
        }

        cyDataBuffer.buffer[0] = 0x9E;
        cyDataBuffer.buffer[1] = 0x00;
        cyDataBuffer.buffer[2] = 0x00; //display once
        cyDataBuffer.length = 3;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

         if (rStatus != CY_SUCCESS)
        {
           printf("display once \n");
        }

        sleep(3);

        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x05;  // Choose test pattern
        cyDataBuffer.buffer[1] = 0x01;  // TestPatternGenerator

        cyDataBuffer.length = 2;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting solid image \n");
        }


        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x0B; // Display solid image
        cyDataBuffer.buffer[1] = 0x00; // No boarder
        cyDataBuffer.buffer[2] = 0x70; // white


        cyDataBuffer.length = 3;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting solid blue image \n");
        }



        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x05; // Display HDMI
        cyDataBuffer.buffer[1] = 0x00; //

        cyDataBuffer.length = 2;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

        if (rStatus != CY_SUCCESS)
        {
           printf("Setting HDMI \n");
        }

        sleep(5);


        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;

        cyDataBuffer.buffer[0] = 0x05;
        cyDataBuffer.buffer[1] = 0x04; //display internal patterns
        cyDataBuffer.length = 2;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

         if (rStatus != CY_SUCCESS)
        {
           printf("display internal pattern \n");
        }

        memset(writeBuffer, 0x00, 4096);
        cyDataBuffer.buffer = writeBuffer;
        cyDataBuffer.buffer[0] = 0x9E;
        cyDataBuffer.buffer[1] = 0x00;
        cyDataBuffer.buffer[2] = 0x00; //display once
        cyDataBuffer.length = 3;

        rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDataBuffer, timeout);

         if (rStatus != CY_SUCCESS)
        {
           printf("display once \n");
        }


        // finish job, close I2C communcation to projector

        rStatus=CySetGpioValue(handle, 5, 0);
        rStatus=CySetGpioValue(handle, 9, 0);

 */
    CyLibraryExit ();
    free (glDevice);
    CyClose (handle);
    CyClose (handle2);




}
int spiWriteEnable (CY_HANDLE handle)
{
    unsigned char wr_data,rd_data;
    CY_RETURN_STATUS status = CY_SUCCESS;
    CY_DATA_BUFFER writeBuf;
    CY_DATA_BUFFER readBuf;

    writeBuf.buffer = &wr_data;
    writeBuf.length = 1;

    readBuf.buffer = &rd_data;
    readBuf.length = 1;

    wr_data = 0x06; /* Write enable */
    status = CySpiReadWrite (handle, &readBuf, &writeBuf, 5000);
    if (status != CY_SUCCESS)
    {
        return status;
    }
    return status;
}
//Helper functions for doing data transfer to/from SPI flash
int spiWaitForIdle (CY_HANDLE handle)
{
    char rd_data[2], wr_data[2];
    CY_DATA_BUFFER writeBuf, readBuf;
    writeBuf.length = 2;
    writeBuf.buffer = (unsigned char *)wr_data;
    int timeout = 0xFFFF;
    CY_RETURN_STATUS status;

    readBuf.length = 2;
    readBuf.buffer = (unsigned char *)rd_data;
    do
    {
        wr_data[0] = 0x05; /* Status */
        status = CySpiReadWrite (handle, &readBuf, &writeBuf, 5000);
        if (status != CY_SUCCESS)
        {
            break;
        }
        timeout--;
        if (timeout == 0)
        {
            status = CY_ERROR_IO_TIMEOUT;
            return status;
        }
    } while (rd_data[1] & 0x01);
    return status;
}

int spiVerifyData (int deviceNumber, int interfaceNum)
{
    CY_DATA_BUFFER dataBufferWrite,dataBufferRead;
    CY_HANDLE handle;
    bool isVerify = true;
    unsigned char wbuffer[256 + 4], rbuffer[256 + 4];
    int rStatus, length;

    memset (rbuffer, 0x00, 256);
    memset (wbuffer, 0x00, 256);

    rStatus = CyOpen (deviceNumber, interfaceNum, &handle);
    if (rStatus != CY_SUCCESS){
        printf ("CY_SPI: Open failed \n");
        return rStatus;
    }
    dataBufferWrite.buffer = wbuffer;
    dataBufferRead.buffer = rbuffer;

    rStatus = spiWaitForIdle (handle);
    if (rStatus){
        printf("Error in Waiting for EEPOM active state %d \n", rStatus);
        CyClose (handle);

        return CY_ERROR_REQUEST_FAILED;
    }
    printf ("Calling spi write enable \n");
    rStatus = spiWriteEnable (handle);
    if (rStatus){
        printf("Error in setting Write Enable %d \n", rStatus);
        CyClose (handle);
        return CY_ERROR_REQUEST_FAILED;
    }
    //Write SPI write command
    wbuffer[0] = 0x02;
    //SPI flash address
    wbuffer[1] = (pageAddress >> 8);
    wbuffer[2] = (pageAddress) & 0x00FF;
    wbuffer[3] = 0;

    printf ("The Data written is ...\n");
    printf ("\n--------------------------------------------------------------------\n");
    for (rStatus = 4; rStatus < (readWriteLength + 4); rStatus++){
        wbuffer[rStatus] = rand() % 256;
        printf ("%x ",wbuffer[rStatus]);
    }
    printf ("\n--------------------------------------------------------------------\n");

    dataBufferRead.length = (4 + readWriteLength);
    dataBufferWrite.length = (4 + readWriteLength);

    rStatus = CySpiReadWrite (handle , &dataBufferRead, &dataBufferWrite, 5000);
    if (rStatus != CY_SUCCESS){
        CyClose (handle);
        printf ("Error in doing SPI data write data Write is %d data read is %d\n" , dataBufferWrite.transferCount,dataBufferRead.transferCount);
        return CY_ERROR_REQUEST_FAILED;
    }

    spiWaitForIdle (handle);
    //Write SPI read command
    wbuffer[0] = 0x03;
    dataBufferRead.length =  (4 + readWriteLength);
    dataBufferWrite.length = (4 + readWriteLength);

    rStatus = CySpiReadWrite (handle, &dataBufferRead, &dataBufferWrite, 5000);
    if (rStatus != CY_SUCCESS){
        CyClose (handle);
        printf ("The Error is %d \n", rStatus);
        printf ("Error in doing SPI data write data Write is %d data read is %d\n" , dataBufferWrite.transferCount,dataBufferRead.transferCount);
        return CY_ERROR_REQUEST_FAILED;
    }
    printf ("Data Read back is \n");
    printf ("\n---------------------------------------------------------------------\n");
    for (rStatus = 4; rStatus < (readWriteLength + 4); rStatus++){
        printf ("%x ",rbuffer[rStatus]);
        if (rbuffer[rStatus] != wbuffer[rStatus]){
            isVerify = false;
        }
    }
    printf ("\n---------------------------------------------------------------------\n");
    if (isVerify)
        printf ("Data verified successfully \n");
    else
        printf ("Data corruption occured!!\n");

    CyClose (handle);
    return CY_SUCCESS;
}

int i2cVerifyData (int deviceNumber, int interfaceNum)
{
    CY_DATA_BUFFER dataBufferWrite, dataBufferRead;
    CY_HANDLE handle;
    int length = 0;
    bool isVerify = true;
    int loopCount = 100, i;
    CY_RETURN_STATUS rStatus;
    unsigned char bytesPending = 0, address[2], wbuffer[66], rbuffer[66];
    CY_I2C_DATA_CONFIG i2cDataConfig;

    memset (wbuffer, 0, 66);
    memset (rbuffer, 0, 66);

    i2cDataConfig.isStopBit = true;
    i2cDataConfig.slaveAddress = 0x51;

    rStatus = CyOpen (deviceNumber, interfaceNum, &handle);
    if (rStatus != CY_SUCCESS){
        printf("CY_I2C: Open failed \n");
        return rStatus;
    }
    loopCount = 100;
    length = readWriteLength;
    wbuffer[0]= pageAddress;
    wbuffer[1] = 0;
    dataBufferWrite.buffer = wbuffer;
    i2cDataConfig.isStopBit = true;
    dataBufferWrite.length = (length + 2);
    printf ("\n Data that is written on to i2c ...\n");
    printf ("\n-----------------------------------------------------------------\n");
    for (i = 2; i < (length +2); i++){
        wbuffer[i] = rand() % 256;
        printf ("%x ", wbuffer[i]);
    }
    printf ("\n-----------------------------------------------------------------\n");
    rStatus = CyI2cWrite (handle, &i2cDataConfig, &dataBufferWrite, 5000);
    if (rStatus != CY_SUCCESS){
        printf ("Error in doing i2c write \n");
        CyClose (handle);
        return -1;
    }
    //We encountered a error in I2C read repeat the procedure again
    //Loop here untill Read vendor command passes
    i2cDataConfig.isStopBit = false;
    dataBufferWrite.length = 2;

    do {
        rStatus = CyI2cWrite (handle, &i2cDataConfig, &dataBufferWrite, 5000);
        loopCount--;
    }while (rStatus != CY_SUCCESS && loopCount != 0);

    if (loopCount == 0 && rStatus != CY_SUCCESS){
        printf ("Error in sending read command \n");
        CyClose (handle);
        return -1;
    }

    dataBufferRead.buffer = rbuffer;
    rbuffer[0]= address[0];
    rbuffer[1] = 0;
    i2cDataConfig.isStopBit = true;
    i2cDataConfig.isNakBit = true;
    dataBufferRead.length = length;
    dataBufferRead.buffer = rbuffer;

    memset (rbuffer, 0, 64);

    rStatus = CyI2cRead (handle, &i2cDataConfig, &dataBufferRead, 5000);
    if (rStatus != CY_SUCCESS){
        printf ("Error in doing i2c read ... Error is %d \n", rStatus);
        CyClose (handle);
        return -1;
    }

    printf ("\n Data that is read from i2c ...\n");
    printf ("\n-----------------------------------------------------------------\n");
    for (rStatus = 0; rStatus < length; rStatus++){
        printf ("%x ", rbuffer[rStatus]);
        if (rbuffer[rStatus] != wbuffer[rStatus + 2]){
            isVerify = false;
        }
    }
    printf ("\n-----------------------------------------------------------------\n");
    if (!isVerify)
        printf ("Data corruption occured ..!!!\n");
    else
        printf ("Data verified successfully \n");

    CyClose (handle);
}
bool isCypressDevice (int deviceNum) {
    CY_HANDLE handle;
    unsigned char interfaceNum = 0;
    unsigned char sig[6];
    CY_RETURN_STATUS rStatus;
    rStatus = CyOpen (deviceNum, interfaceNum, &handle);
    if (rStatus == CY_SUCCESS){
        rStatus = CyGetSignature (handle, sig);
        if (rStatus == CY_SUCCESS){
            CyClose (handle);
            return true;
        }
        else {
            CyClose (handle);
            return false;
        }
    }
    else
        return false;
}
void printListOfDevices (bool isPrint)
{
    int  index_i = 0, index_j = 0, i, j, countOfDevice = 0, devNum;
    int length, index = 0, numInterfaces, interfaceNum;
    bool set1 = false;

    unsigned char deviceID[CY_MAX_DEVICES];
    unsigned char functionality[64];
    CY_DEVICE_INFO deviceInfo;
    CY_DEVICE_CLASS deviceClass[CY_MAX_INTERFACES];
    CY_DEVICE_TYPE  deviceType[CY_MAX_INTERFACES];
    CY_RETURN_STATUS rStatus;

    deviceAddedRemoved = false;
    CyGetListofDevices (&numDevices);
    //printf ("The number of devices is %d \n", numDevices);
    for (i = 0; i < numDevices; i++){
        for (j = 0; j< CY_MAX_INTERFACES; j++)
            glDevice[i].interfaceFunctionality[j] = -1;
    }
    if (isPrint){
        printf ("\n\n---------------------------------------------------------------------------------\n");
        printf ("Device Number | VID | PID | INTERFACE NUMBER | FUNCTIONALITY \n");
        printf ("---------------------------------------------------------------------------------\n");
    }
    cyDevices = 0;
    for (devNum = 0; devNum < numDevices; devNum++){
        rStatus = CyGetDeviceInfo (devNum, &deviceInfo);
        interfaceNum = 0;
        if (!rStatus)
        {
            if (!isCypressDevice (devNum)){
                continue;
            }
            strcpy (functionality, "NA");
            numInterfaces = deviceInfo.numInterfaces;
            glDevice[index].numInterface = numInterfaces;
            cyDevices++;

            while (numInterfaces){
                if (deviceInfo.deviceClass[interfaceNum] == CY_CLASS_VENDOR)
                {
                    glDevice[index].deviceNumber = devNum;
                    switch (deviceInfo.deviceType[interfaceNum]){
                        case CY_TYPE_I2C:
                            glDevice[index].interfaceFunctionality[interfaceNum] = CY_TYPE_I2C;
                            strcpy (functionality, "VENDOR_I2C");
                            glDevice[index].isI2c = true;
                            break;
                        case CY_TYPE_SPI:
                            glDevice[index].interfaceFunctionality[interfaceNum] = CY_TYPE_SPI;
                            strcpy (functionality, "VENDOR_SPI");
                            glDevice[index].isSpi = true;
                            break;
                        default:
                            strcpy (functionality, "NA");
                            break;
                    }
                }
                else if (deviceInfo.deviceClass[interfaceNum] == CY_CLASS_CDC){
                    strcpy (functionality, "NA");
                }
                if (isPrint) {
                    printf ("%d             |%x  |%x    | %d     | %s\n", \
                            index, \
                            deviceInfo.vidPid.vid, \
                            deviceInfo.vidPid.pid,  \
                            interfaceNum, \
                            functionality \
                            );
                }
                interfaceNum++;
                numInterfaces--;
            }
            index++;
        }
    }
    if (isPrint){
        printf ("---------------------------------------------------------------------------------\n\n");
    }
}

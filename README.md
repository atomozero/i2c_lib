# I2C Driver and Library for Haiku OS

This project provides an I2C driver and accompanying C++ library for Haiku OS. The driver enables interaction with I2C devices via `/dev/i2c/*` paths, and the library offers a user-friendly interface for reading and writing data over the I2C bus.

## Features

- Basic I2C communication (read/write).
- Simple user-mode interface.
- Example driver to register an I2C device on `/dev/i2c`.
- Designed for ease of integration with hardware using the I2C protocol on Haiku OS.

---

## 1. I2C Driver Overview

The I2C driver registers an I2C bus as a device under `/dev/i2c`. This driver allows reading and writing data over the I2C bus, which is then used by user-mode applications or other kernel modules.

### Key Functions

- **init_driver**: Initializes the I2C driver, registers the device under `/dev/i2c/test`.
- **uninit_driver**: Cleans up the driver, unregistering the device.
- **i2c_open**: Opens a connection to the I2C bus.
- **i2c_close**: Closes the connection.
- **i2c_read**: Reads data from the I2C bus.
- **i2c_write**: Writes data to the I2C bus.
- **i2c_ioctl**: Handles specific operations on the I2C bus, such as setting the I2C slave address.

### Directory Structure

- `/dev/i2c/*`: Path where the I2C devices are registered.
- `/boot/system/non-packaged/add-ons/kernel/drivers/bin/`: Path where the compiled I2C driver is placed.

---

## 2. I2C Library Overview

The I2C library is a C++ wrapper around the driver functionality, offering a simple API for userspace applications to communicate with I2C devices. It abstracts the low-level I/O operations, making it easier to interact with I2C devices.

### Example Usage

```cpp
#include "i2c.h"

int main() {
    I2CBus bus(0); // Create an I2CBus object for bus 0

    if (bus.init() != B_OK) {
        // Handle initialization error
        return 1;
    }

    uint8 address = 0x50; // Example I2C device address
    uint8 data_to_write[] = {0x00}; // Data to write
    uint8 data_to_read[2]; // Buffer for reading data

    // Write data to the I2C device
    if (bus.write(address, 1, data_to_write) != B_OK) {
        // Handle write error
        bus.deinit();
        return 1;
    }

    // Read data from the I2C device
    if (bus.read(address, 2, data_to_read) != B_OK) {
        // Handle read error
        bus.deinit();
        return 1;
    }

    bus.deinit();
    return 0;
}
```

### Library API

#### `class I2CBus`

Represents an I2C bus and provides methods to interact with devices on the bus.

- **`I2CBus(int bus_number)`**: Constructor that initializes the bus object with the given bus number.
- **`status_t init()`**: Initializes the I2C bus by opening the corresponding device under `/dev/i2c-*`.
- **`void deinit()`**: Closes the I2C bus and cleans up resources.
- **`status_t write(uint8 address, size_t length, const uint8* data)`**: Writes data to a device on the I2C bus.
- **`status_t read(uint8 address, size_t length, uint8* data)`**: Reads data from a device on the I2C bus.

---

## 3. Installation and Compilation

### 3.1 Driver Installation

1. **Compile the driver**:
   ```bash
   g++ -o i2c_driver i2c_driver.cpp -Wall -lroot
   ```

2. **Copy the driver to the appropriate directory**:
   ```bash
   cp i2c_driver /boot/system/non-packaged/add-ons/kernel/drivers/bin/
   ```

3. **Create the `/dev/i2c` directory** (if it doesn’t exist):
   ```bash
   mkdir -p /dev/i2c
   ```

4. **Load the driver** by either restarting the system or by using Haiku’s tools to manually load the driver.

### 3.2 Library Installation

1. **Compile the library**:
   Ensure that `i2c.h` and `i2c.cpp` are correctly placed in your project, then compile your project using:

   ```bash
   g++ -o your_application your_application.cpp i2c.cpp -lroot
   ```

---

## 4. Future Improvements

1. **Support for Multiple Buses**: Extend the driver to automatically detect and support multiple I2C buses.
2. **Interrupt Handling**: Implement interrupt-based data transfer for more efficient I2C communication.
3. **Advanced Features**: Add more ioctl commands for controlling bus speed, error handling, and advanced I2C features.

---

## 5. License

This I2C driver and library are open-source and licensed under the MIT License. Feel free to modify and distribute as needed.

---

## 6. Contributing

If you would like to contribute to this project, feel free to open issues or submit pull requests on the repository.

For questions, contact the maintainers via email or on the Haiku OS forums.

---

This documentation outlines the structure and usage of the I2C driver and library for Haiku OS. If you encounter any issues, refer to the `syslog` for debugging information, or consult the Haiku OS development guides.

--- 

This should cover all aspects of the project in a concise and clear manner for users and developers working with I2C on Haiku OS.

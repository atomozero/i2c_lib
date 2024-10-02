#ifndef I2C_H
#define I2C_H

#include <OS.h>
#include <stdint.h>

// Definizioni delle costanti I2C
#define I2C_SLAVE 0x0703

class I2CBus {
public:
    I2CBus(int bus_number);
    ~I2CBus();

    status_t init();
    status_t deinit();

    status_t write(uint8 address, const uint8* data, size_t length);
    status_t read(uint8 address, uint8* buffer, size_t length);

    status_t set_speed(uint32 speed);

    status_t write_register(uint8 address, uint8 reg, uint8 data);
    status_t read_register(uint8 address, uint8 reg, uint8* data);

    status_t write_registers(uint8 address, uint8 reg, const uint8* data, size_t length);
    status_t read_registers(uint8 address, uint8 reg, uint8* data, size_t length);

private:
    int f_bus_number;
    bool f_initialized;
    uint32 f_speed;
    int f_fd;

    status_t open_bus();
    void close_bus();
    status_t set_slave_address(uint8 address);
};

#endif  // I2C_H

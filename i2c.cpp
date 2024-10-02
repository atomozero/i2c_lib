#include "i2c.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

I2CBus::I2CBus(int bus_number)
    : f_bus_number(bus_number), f_initialized(false), f_speed(100000), f_fd(-1) {
}

I2CBus::~I2CBus() {
    if (f_initialized) {
        deinit();
    }
}

status_t I2CBus::init() {
    if (f_initialized) {
        return B_OK;
    }

    status_t status = open_bus();
    if (status == B_OK) {
        f_initialized = true;
    }
    return status;
}

status_t I2CBus::deinit() {
    if (!f_initialized) {
        return B_OK;
    }

    close_bus();
    f_initialized = false;
    return B_OK;
}

status_t I2CBus::open_bus() {
    char path[20];
    snprintf(path, sizeof(path), "/dev/i2c-%d", f_bus_number);
    f_fd = open(path, O_RDWR);
    if (f_fd < 0) {
        fprintf(stderr, "Errore nell'aprire %s: %s\n", path, strerror(errno));
        return B_IO_ERROR;
    }
    return B_OK;
}

void I2CBus::close_bus() {
    if (f_fd >= 0) {
        close(f_fd);
        f_fd = -1;
    }
}

status_t I2CBus::set_slave_address(uint8 address) {
    // Passiamo l'indirizzo di 'address' invece di 'address' direttamente
    if (ioctl(f_fd, I2C_SLAVE, (void*)(uintptr_t)address) < 0) {
        fprintf(stderr, "Errore nel settare l'indirizzo slave: %s\n", strerror(errno));
        return B_IO_ERROR;
    }
    return B_OK;
}

status_t I2CBus::write(uint8 address, const uint8* data, size_t length) {
    if (!f_initialized || f_fd < 0) {
        return B_NO_INIT;
    }

    status_t status = set_slave_address(address);
    if (status != B_OK) {
        return status;
    }

    ssize_t bytes_written = ::write(f_fd, data, length);
    if (bytes_written != static_cast<ssize_t>(length)) {
        fprintf(stderr, "Errore nella scrittura dei dati: %s\n", strerror(errno));
        return B_IO_ERROR;
    }

    return B_OK;
}

status_t I2CBus::read(uint8 address, uint8* buffer, size_t length) {
    if (!f_initialized || f_fd < 0) {
        return B_NO_INIT;
    }

    status_t status = set_slave_address(address);
    if (status != B_OK) {
        return status;
    }

    ssize_t bytes_read = ::read(f_fd, buffer, length);
    if (bytes_read != static_cast<ssize_t>(length)) {
        fprintf(stderr, "Errore nella lettura dei dati: %s\n", strerror(errno));
        return B_IO_ERROR;
    }

    return B_OK;
}

status_t I2CBus::set_speed(uint32 speed) {
    if (!f_initialized) {
        return B_NO_INIT;
    }

    f_speed = speed;
    
    // Nota: l'implementazione effettiva potrebbe richiedere un ioctl specifico di Haiku
    // per impostare la velocità del bus

    return B_OK;
}

status_t I2CBus::write_register(uint8 address, uint8 reg, uint8 data) {
    uint8 buffer[2] = {reg, data};
    return write(address, buffer, sizeof(buffer));
}

status_t I2CBus::read_register(uint8 address, uint8 reg, uint8* data) {
    status_t status = write(address, &reg, 1);
    if (status != B_OK) {
        return status;
    }
    return read(address, data, 1);
}

status_t I2CBus::write_registers(uint8 address, uint8 reg, const uint8* data, size_t length) {
    uint8* buffer = new uint8[length + 1];
    buffer[0] = reg;
    memcpy(buffer + 1, data, length);

    status_t status = write(address, buffer, length + 1);
    delete[] buffer;
    return status;
}

status_t I2CBus::read_registers(uint8 address, uint8 reg, uint8* data, size_t length) {
    status_t status = write(address, &reg, 1);
    if (status != B_OK) {
        return status;
    }
    return read(address, data, length);
}

// La funzione transfer è stata rimossa

#include "i2c.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <KernelExport.h>

I2CBus::I2CBus(int bus_number)
    : f_bus_number(bus_number), f_initialized(false), f_speed(100000) {
}

I2CBus::~I2CBus() {
    if (f_initialized) {
        deinit();
    }
}

status_t I2CBus::init() {
    char path[20];
    snprintf(path, sizeof(path), "/dev/i2c-%d", f_bus_number);

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Errore nell'aprire %s: %s\n", path, strerror(errno));
        return B_ERROR;
    }

    // Altre operazioni di inizializzazione

    f_initialized = true;
    close(fd);
    return B_OK;
}

status_t I2CBus::deinit() {
    if (!f_initialized) {
        return B_ERROR;
    }

    // Operazioni di chiusura del bus
    f_initialized = false;
    return B_OK;
}

status_t I2CBus::write(uint8 address, const uint8* data, size_t length) {
    if (!f_initialized) {
        return B_ERROR;
    }

    char path[20];
    snprintf(path, sizeof(path), "/dev/i2c-%d", f_bus_number);
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Errore nell'aprire il bus I2C per scrittura: %s\n", strerror(errno));
        return B_ERROR;
    }

    // Configurazione dell'indirizzo del dispositivo slave
    if (ioctl(fd, I2C_SET_SLAVE, &address) < 0) {
        printf("Errore nel settare l'indirizzo slave: %s\n", strerror(errno));
        close(fd);
        return B_ERROR;
    }

    // Scrittura dei dati
    if (write(fd, data, length) != length) {
        printf("Errore nella scrittura dei dati: %s\n", strerror(errno));
        close(fd);
        return B_ERROR;
    }

    close(fd);
    return B_OK;
}

status_t I2CBus::read(uint8 address, uint8* buffer, size_t length) {
    if (!f_initialized) {
        return B_ERROR;
    }

    char path[20];
    snprintf(path, sizeof(path), "/dev/i2c-%d", f_bus_number);
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Errore nell'aprire il bus I2C per lettura: %s\n", strerror(errno));
        return B_ERROR;
    }

    // Configurazione dell'indirizzo del dispositivo slave
    if (ioctl(fd, I2C_SET_SLAVE, &address) < 0) {
        printf("Errore nel settare l'indirizzo slave: %s\n", strerror(errno));
        close(fd);
        return B_ERROR;
    }

    // Lettura dei dati
    if (read(fd, buffer, length) != length) {
        printf("Errore nella lettura dei dati: %s\n", strerror(errno));
        close(fd);
        return B_ERROR;
    }

    close(fd);
    return B_OK;
}

status_t I2CBus::set_speed(uint32 speed) {
    if (!f_initialized) {
        return B_ERROR;
    }

    f_speed = speed;
    // Qui si potrebbe configurare la velocità del bus con un ioctl, se supportato
    // ioctl(fd, I2C_SET_SPEED, &f_speed);

    return B_OK;
}

status_t I2CBus::write_register(uint8 address, uint8 reg, uint8 data) {
    uint8 buffer[2] = {reg, data};
    return write(address, buffer, sizeof(buffer));
}

status_t I2CBus::read_register(uint8 address, uint8 reg, uint8* data) {
    if (write(address, &reg, 1) != B_OK) {
        return B_ERROR;
    }
    return read(address, data, 1);
}

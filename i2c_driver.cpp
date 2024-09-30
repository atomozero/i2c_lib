#include <KernelExport.h>
#include <drivers/Drivers.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define I2C_DEVICE_PATH "i2c/test"

// Definizione di alcuni comandi base I2C
#define I2C_READ  0x01
#define I2C_WRITE 0x00

// Struttura del dispositivo I2C
typedef struct {
    int bus_number;
    bool active;
} i2c_device;

// Dichiarazioni delle funzioni del driver
static status_t i2c_open(const char* name, uint32 flags, void** cookie);
static status_t i2c_close(void* cookie);
static status_t i2c_read(void* cookie, off_t pos, void* buffer, size_t* length);
static status_t i2c_write(void* cookie, off_t pos, const void* buffer, size_t* length);
static status_t i2c_ioctl(void* cookie, uint32 op, void* buffer, size_t length);

// Struttura con le funzioni del driver
static device_hooks i2c_hooks = {
    &i2c_open,
    &i2c_close,
    &i2c_ioctl,
    &i2c_read,
    &i2c_write,
    NULL,
    NULL,
    NULL,
    NULL,
};

// Funzione di inizializzazione del driver
status_t
init_hardware() {
    // Funzione chiamata al caricamento del driver per fare un check dell'hardware
    dprintf("i2c_driver: init_hardware()\n");
    return B_OK;
}

// Funzione chiamata quando il driver viene inizializzato
status_t
init_driver() {
    dprintf("i2c_driver: init_driver()\n");

    // Aggiunge il dispositivo I2C al filesystem di Haiku
    if (devfs_publish_device(I2C_DEVICE_PATH, NULL, &i2c_hooks)) {
        dprintf("i2c_driver: impossibile registrare il dispositivo %s\n", I2C_DEVICE_PATH);
        return B_ERROR;
    }

    return B_OK;
}

// Funzione chiamata per chiudere il driver
void
uninit_driver() {
    dprintf("i2c_driver: uninit_driver()\n");

    // Rimuove il dispositivo I2C dal filesystem di Haiku
    devfs_unpublish_device(I2C_DEVICE_PATH, NULL);
}

// Funzione di apertura del dispositivo I2C
static status_t
i2c_open(const char* name, uint32 flags, void** cookie) {
    dprintf("i2c_driver: i2c_open()\n");

    // Alloca memoria per il dispositivo e inizializza i campi
    i2c_device* device = (i2c_device*)malloc(sizeof(i2c_device));
    device->bus_number = 0;  // Imposta il bus I2C 0
    device->active = true;
    *cookie = device;

    return B_OK;
}

// Funzione di chiusura del dispositivo I2C
static status_t
i2c_close(void* cookie) {
    dprintf("i2c_driver: i2c_close()\n");

    // Dealloca la memoria del dispositivo
    i2c_device* device = (i2c_device*)cookie;
    free(device);

    return B_OK;
}

// Funzione di lettura da un dispositivo I2C
static status_t
i2c_read(void* cookie, off_t pos, void* buffer, size_t* length) {
    dprintf("i2c_driver: i2c_read() - Posizione: %lld, Lunghezza: %zu\n", pos, *length);

    // Simula una lettura, puoi sostituire questo blocco con codice I2C reale
    uint8_t* read_buffer = (uint8_t*)buffer;
    for (size_t i = 0; i < *length; i++) {
        read_buffer[i] = i;  // Dati di test
    }

    return B_OK;
}

// Funzione di scrittura su un dispositivo I2C
static status_t
i2c_write(void* cookie, off_t pos, const void* buffer, size_t* length) {
    dprintf("i2c_driver: i2c_write() - Posizione: %lld, Lunghezza: %zu\n", pos, *length);

    // Simula una scrittura, puoi sostituire questo blocco con codice I2C reale
    const uint8_t* write_buffer = (const uint8_t*)buffer;
    for (size_t i = 0; i < *length; i++) {
        dprintf("i2c_driver: Dato scritto %02x\n", write_buffer[i]);
    }

    return B_OK;
}

// Funzione di gestione di operazioni specifiche con ioctl
static status_t
i2c_ioctl(void* cookie, uint32 op, void* buffer, size_t length) {
    dprintf("i2c_driver: i2c_ioctl() - Operazione: %u\n", op);

    // Aggiungi qui la gestione di operazioni specifiche come impostazione della velocità, ecc.
    switch (op) {
        case I2C_SET_SLAVE:
            dprintf("i2c_driver: I2C_SET_SLAVE chiamato\n");
            // Gestione dell'impostazione del dispositivo slave
            break;
        default:
            return B_DEV_INVALID_IOCTL;
    }

    return B_OK;
}

// Dichiarazione delle funzioni principali del driver
const char** publish_devices() {
    static const char* devices[] = { I2C_DEVICE_PATH, NULL };
    return devices;
}

device_hooks* find_device(const char* name) {
    return &i2c_hooks;
}

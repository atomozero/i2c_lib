#include <KernelExport.h>
#include <Drivers.h>
#include <Errors.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>       // Per STDOUT_FILENO e STDERR_FILENO
#include <module.h>       // Per module_info e operazioni del modulo

#define I2C_DEVICE_PATH "i2c/my_i2c_device"
#define I2C_SET_SLAVE 0x0703

// Funzioni Hook del driver
status_t i2c_open(const char* name, uint32 flags, void** cookie);
status_t i2c_close(void* cookie);
status_t i2c_read(void* cookie, off_t pos, void* buffer, size_t* length);
status_t i2c_write(void* cookie, off_t pos, const void* buffer, size_t* length);
status_t i2c_ioctl(void* cookie, uint32 op, void* buffer, size_t length);

// Tabella dei "device hooks"
static device_hooks i2c_hooks = {
    &i2c_open,
    &i2c_close,
    &i2c_ioctl,  // Ioctl è trattato come "control" hook, quindi il tipo corretto è "device_control_hook"
    &i2c_read,   // Read hook
    &i2c_write,  // Write hook
    NULL,        // No free hook
    NULL,        // No select hook
    NULL         // No deselect hook
};

// Funzione di apertura del dispositivo
status_t i2c_open(const char* name, uint32 flags, void** cookie) {
    dprintf(STDOUT_FILENO, "i2c_driver: i2c_open() - Nome dispositivo: %s\n", name);
    *cookie = NULL; // Non usiamo il "cookie" in questo esempio
    return B_OK;
}

// Funzione di chiusura del dispositivo
status_t i2c_close(void* cookie) {
    dprintf(STDOUT_FILENO, "i2c_driver: i2c_close()\n");
    return B_OK;
}

// Funzione di lettura dal dispositivo
status_t i2c_read(void* cookie, off_t pos, void* buffer, size_t* length) {
    dprintf(STDOUT_FILENO, "i2c_driver: i2c_read() - Posizione: %ld, Lunghezza: %zu\n", pos, *length);
    memset(buffer, 0, *length); // Legge dati fittizi (tutti 0)
    return B_OK;
}

// Funzione di scrittura sul dispositivo
status_t i2c_write(void* cookie, off_t pos, const void* buffer, size_t* length) {
    dprintf(STDOUT_FILENO, "i2c_driver: i2c_write() - Posizione: %ld, Lunghezza: %zu\n", pos, *length);
    const uint8_t* write_buffer = (const uint8_t*)buffer;
    for (size_t i = 0; i < *length; i++) {
        dprintf(STDOUT_FILENO, "i2c_driver: Dato scritto %02x\n", write_buffer[i]);
    }
    return B_OK;
}

// Funzione di gestione delle operazioni speciali (ioctl)
status_t i2c_ioctl(void* cookie, uint32 op, void* buffer, size_t length) {
    dprintf(STDOUT_FILENO, "i2c_driver: i2c_ioctl() - Operazione: %u\n", op);
    switch (op) {
        case I2C_SET_SLAVE:
            dprintf(STDOUT_FILENO, "i2c_driver: I2C_SET_SLAVE chiamato\n");
            // Imposta il dispositivo slave I2C (simulato)
            break;
        default:
            dprintf(STDOUT_FILENO, "i2c_driver: Operazione non supportata\n");
            return B_BAD_VALUE;
    }
    return B_OK;
}

// Funzione di inizializzazione dell'hardware (opzionale)
status_t init_hardware() {
    dprintf(STDOUT_FILENO, "i2c_driver: init_hardware()\n");
    return B_OK;
}

// Funzione di inizializzazione del driver
status_t init_driver() {
    dprintf(STDOUT_FILENO, "i2c_driver: init_driver()\n");
    if (publish_devices(I2C_DEVICE_PATH, &i2c_hooks) != B_OK) {
        dprintf(STDOUT_FILENO, "i2c_driver: impossibile registrare il dispositivo %s\n", I2C_DEVICE_PATH);
        return B_ERROR;
    }
    return B_OK;
}

// Funzione di disinstallazione del driver
void uninit_driver() {
    dprintf(STDOUT_FILENO, "i2c_driver: uninit_driver()\n");
    publish_devices(I2C_DEVICE_PATH);  // Correzione della funzione
}

// Funzione di gestione dei comandi per il modulo del kernel
status_t i2c_std_ops(int32 op, ...) {
    switch (op) {
        case B_MODULE_INIT:
            return init_driver();
        case B_MODULE_UNINIT:
            uninit_driver();
            return B_OK;
        default:
            return B_BAD_VALUE;
    }
}

// Definizione del modulo driver
module_info i2c_driver_module = {
    "drivers/i2c/my_i2c_device",
    0,
    i2c_std_ops
};

// Esportazione del modulo
_EXPORT module_info* modules[] = {
    &i2c_driver_module,
    NULL
};

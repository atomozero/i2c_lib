#ifndef I2C_TOUCHPAD_H
#define I2C_TOUCHPAD_H

#include <OS.h>
#include "i2c_driver.h"
#include <drivers/device_manager.h>
#include <Drivers.h>
#include <drivers/Drivers.h> 
#include <KernelExport.h>

// Struttura per il descrittore HID
typedef struct {
    uint16 wHIDDescLength;
    uint16 bcdVersion;
    uint16 wReportDescLength;
    uint16 wReportDescRegister;
    uint16 wInputRegister;
    uint16 wMaxInputLength;
    uint16 wOutputRegister;
    uint16 wMaxOutputLength;
    uint16 wCommandRegister;
    uint16 wDataRegister;
    uint16 wVendorID;
    uint16 wProductID;
    uint16 wVersionID;
    uint8 reserved[4];
} hid_descriptor;

// Struttura per le informazioni del touchpad
typedef struct {
    uint16 max_x;
    uint16 max_y;
    uint8 max_touch_points;
    bool supports_pressure;
    // Aggiungi altri parametri specifici del touchpad secondo necessità
} touchpad_info;

// Prototipi delle funzioni
status_t init_touchpad(i2c_device_info* device);
status_t get_hid_report(i2c_device_info* device, uint8* report, uint16 length);
status_t parse_report_descriptor(uint8* report_descriptor, uint16 length);

// Funzioni hook del dispositivo
status_t touchpad_open(const char* name, uint32 flags, void** cookie);
status_t touchpad_close(void* cookie);
status_t touchpad_free(void* cookie);
status_t touchpad_read(void* cookie, off_t position, void* buffer, size_t* numBytes);
status_t touchpad_write(void* cookie, off_t position, const void* buffer, size_t* numBytes);
status_t touchpad_control(void* cookie, uint32 op, void* arg, size_t len);

// Funzioni hook del dispositivo di input
status_t touchpad_device_control(void* cookie, uint32 op, void* arg, size_t len);
status_t touchpad_device_read(void* cookie, off_t position, void* buffer, size_t* numBytes);

// Definizioni per i comandi IOCTL specifici del touchpad
enum {
    TOUCHPAD_IOCTL_GET_INFO = B_DEVICE_OP_CODES_END + 1000,
    TOUCHPAD_IOCTL_SET_PARAMETERS,
    // Aggiungi altri codici IOCTL secondo necessità
};

// Struttura per i parametri del touchpad
typedef struct {
    bool tap_to_click_enabled;
    uint8 scroll_speed;
    uint8 sensitivity;
    // Aggiungi altri parametri configurabili secondo necessità
} touchpad_parameters;

// Funzioni di utilità
status_t touchpad_set_parameters(i2c_device_info* device, touchpad_parameters* params);
status_t touchpad_get_parameters(i2c_device_info* device, touchpad_parameters* params);
status_t touchpad_reset(i2c_device_info* device);

// Funzioni per la gestione degli eventi
void process_touchpad_event(i2c_device_info* device, uint8* event_data, size_t event_size);

#endif // I2C_TOUCHPAD_H

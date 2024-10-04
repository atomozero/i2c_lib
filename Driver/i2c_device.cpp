#include "i2c_device.h"
#include "i2c_controller.h"
#include "i2c_util.h"
#include <string.h>
#include <stdlib.h>

static i2c_device_info* sDeviceList = NULL;
static uint32 sDeviceCount = 0;

status_t probe_i2c_devices() {
    pci_info info;
    uint32 index = 0;

    while (sPCIModule->get_nth_pci_info(index++, &info) == B_OK) {
        if (info.vendor_id == INTEL_VENDOR_ID &&
            (info.device_id == INTEL_TIGER_LAKE_I2C_CONTROLLER_0 ||
             info.device_id == INTEL_TIGER_LAKE_I2C_CONTROLLER_1)) {
            
            i2c_device_info* new_devices = (i2c_device_info*)realloc(sDeviceList, (sDeviceCount + 1) * sizeof(i2c_device_info));
            if (new_devices == NULL) {
                I2C_DEBUG_PRINT("Failed to allocate memory for new device\n");
                return B_NO_MEMORY;
            }
            
            sDeviceList = new_devices;
            i2c_device_info* device = &sDeviceList[sDeviceCount];
            
            memset(device, 0, sizeof(i2c_device_info));
            device->base_addr = info.u.h0.base_registers[0];
            device->irq = info.u.h0.interrupt_line;
            device->vendor_id = info.vendor_id;
            device->device_id = info.device_id;
            
            status_t status = init_i2c_controller(device);
            if (status != B_OK) {
                I2C_DEBUG_PRINT("Failed to initialize I2C controller\n");
                return status;
            }
            
            sDeviceCount++;
            
            I2C_DEBUG_PRINT("Found I2C controller at %02x:%02x.%x\n",
                    info.bus, info.device, info.function);
        }
    }

    if (sDeviceCount == 0) {
        I2C_DEBUG_PRINT("No compatible I2C controllers found\n");
        return B_ERROR;
    }

    return B_OK;
}

void free_i2c_devices() {
    for (uint32 i = 0; i < sDeviceCount; i++) {
        if (sDeviceList[i].register_area >= B_OK) {
            delete_area(sDeviceList[i].register_area);
        }
    }
    free(sDeviceList);
    sDeviceList = NULL;
    sDeviceCount = 0;
}

i2c_device_info* find_i2c_device(const char* name) {
    // Per ora, restituisci semplicemente il primo dispositivo
    // In futuro, potresti voler implementare una logica più sofisticata
    if (sDeviceCount > 0) {
        return &sDeviceList[0];
    }
    return NULL;
}

status_t i2c_device_init(i2c_device_info* device, uint8 slave_address) {
    if (device == NULL) {
        return B_BAD_VALUE;
    }

    device->slave_addr = slave_address;
    
    // Inizializza qui eventuali altre configurazioni specifiche del dispositivo

    return B_OK;
}

status_t i2c_device_read(i2c_device_info* device, uint8* buffer, size_t length) {
    if (device == NULL || buffer == NULL) {
        return B_BAD_VALUE;
    }

    return i2c_transfer(device, device->slave_addr, NULL, 0, buffer, length);
}

status_t i2c_device_write(i2c_device_info* device, const uint8* buffer, size_t length) {
    if (device == NULL || buffer == NULL) {
        return B_BAD_VALUE;
    }

    return i2c_transfer(device, device->slave_addr, buffer, length, NULL, 0);
}

status_t i2c_device_read_register(i2c_device_info* device, uint8 reg, uint8* value) {
    if (device == NULL || value == NULL) {
        return B_BAD_VALUE;
    }

    return i2c_transfer(device, device->slave_addr, &reg, 1, value, 1);
}

status_t i2c_device_write_register(i2c_device_info* device, uint8 reg, uint8 value) {
    if (device == NULL) {
        return B_BAD_VALUE;
    }

    uint8 buffer[2] = {reg, value};
    return i2c_transfer(device, device->slave_addr, buffer, 2, NULL, 0);
}

status_t i2c_device_transfer(i2c_device_info* device, i2c_transfer* transfers, size_t count) {
    if (device == NULL || transfers == NULL || count == 0) {
        return B_BAD_VALUE;
    }

    status_t status = B_OK;
    for (size_t i = 0; i < count; i++) {
        status = i2c_transfer(device, device->slave_addr, 
                              transfers[i].send_buffer, transfers[i].send_length,
                              transfers[i].recv_buffer, transfers[i].recv_length);
        if (status != B_OK) {
            break;
        }
    }

    return status;
}

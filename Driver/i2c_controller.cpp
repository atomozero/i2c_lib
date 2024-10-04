#include "i2c_controller.h"
#include "i2c_driver.h"
#include <drivers/device_manager.h>
#include <PCI.h>
#include <string.h>

#define INTEL_VENDOR_ID 0x8086
#define TIGER_LAKE_I2C_CONTROLLER_0 0xa0e8
#define TIGER_LAKE_I2C_CONTROLLER_1 0xa0e9

// Registri del controller I2C (esempio, potrebbero variare a seconda del controller specifico)
#define I2C_CON         0x00 // Control Register
#define I2C_TAR         0x04 // Target Address Register
#define I2C_DATA_CMD    0x10 // Data Command Register
#define I2C_SS_SCL_HCNT 0x14 // Standard Speed SCL High Count Register
#define I2C_SS_SCL_LCNT 0x18 // Standard Speed SCL Low Count Register
#define I2C_FS_SCL_HCNT 0x1C // Fast Speed SCL High Count Register
#define I2C_FS_SCL_LCNT 0x20 // Fast Speed SCL Low Count Register
#define I2C_INTR_STAT   0x2C // Interrupt Status Register
#define I2C_INTR_MASK   0x30 // Interrupt Mask Register
#define I2C_RAW_INTR_STAT 0x34 // Raw Interrupt Status Register
#define I2C_RX_TL       0x38 // Receive FIFO Threshold Register
#define I2C_TX_TL       0x3C // Transmit FIFO Threshold Register
#define I2C_CLR_INTR    0x40 // Clear Combined and Individual Interrupt Register
#define I2C_STATUS      0x70 // Status Register

static pci_module_info* sPCIModule;
static i2c_device_info* sDeviceList = NULL;
static uint32 sDeviceCount = 0;

status_t probe_i2c_devices() {
    pci_info info;
    uint32 index = 0;

    while (sPCIModule->get_nth_pci_info(index++, &info) == B_OK) {
        if (info.vendor_id == INTEL_VENDOR_ID &&
            (info.device_id == TIGER_LAKE_I2C_CONTROLLER_0 ||
             info.device_id == TIGER_LAKE_I2C_CONTROLLER_1)) {
            
            i2c_device_info* new_devices = (i2c_device_info*)realloc(sDeviceList, (sDeviceCount + 1) * sizeof(i2c_device_info));
            if (new_devices == NULL) {
                dprintf(DRIVER_NAME ": Failed to allocate memory for new device\n");
                return B_NO_MEMORY;
            }
            
            sDeviceList = new_devices;
            i2c_device_info* device = &sDeviceList[sDeviceCount];
            
            device->base_addr = info.u.h0.base_registers[0];
            device->irq = info.u.h0.interrupt_line;
            device->vendor_id = info.vendor_id;
            device->device_id = info.device_id;
            
            status_t status = init_i2c_controller(device);
            if (status != B_OK) {
                dprintf(DRIVER_NAME ": Failed to initialize I2C controller\n");
                return status;
            }
            
            sDeviceCount++;
            
            dprintf(DRIVER_NAME ": Found I2C controller at %02x:%02x.%x\n",
                    info.bus, info.device, info.function);
        }
    }

    if (sDeviceCount == 0) {
        dprintf(DRIVER_NAME ": No compatible I2C controllers found\n");
        return B_ERROR;
    }

    return B_OK;
}

status_t init_i2c_controller(i2c_device_info* device) {
    device->register_area = map_physical_memory("i2c_regs", device->base_addr, 
                                                B_PAGE_SIZE, B_IO_MEMORY,
                                                (void**)&device->mapped_registers);
    if (device->register_area < B_OK) {
        dprintf(DRIVER_NAME ": Failed to map I2C registers\n");
        return device->register_area;
    }

    // Inizializzazione del controller I2C
    // Questo è un esempio e potrebbe dover essere adattato al tuo controller specifico
    write32(device->mapped_registers + I2C_CON, 0x0); // Disabilita il controller
    write32(device->mapped_registers + I2C_SS_SCL_HCNT, 0x190); // Configura la temporizzazione
    write32(device->mapped_registers + I2C_SS_SCL_LCNT, 0x1D6);
    write32(device->mapped_registers + I2C_FS_SCL_HCNT, 0x3C);
    write32(device->mapped_registers + I2C_FS_SCL_LCNT, 0x82);
    write32(device->mapped_registers + I2C_INTR_MASK, 0); // Disabilita tutti gli interrupt
    write32(device->mapped_registers + I2C_CON, 0x1); // Abilita il controller

    return B_OK;
}

status_t i2c_transfer(i2c_device_info* device, int addr, const uint8* write_buf, size_t write_len, uint8* read_buf, size_t read_len) {
    if (!device || (!write_buf && write_len > 0) || (!read_buf && read_len > 0)) {
        return B_BAD_VALUE;
    }

    // Imposta l'indirizzo del dispositivo slave
    write32(device->mapped_registers + I2C_TAR, addr);

    // Scrittura
    for (size_t i = 0; i < write_len; i++) {
        while (!(read32(device->mapped_registers + I2C_STATUS) & 0x2)) { // Attendi che il TX FIFO non sia pieno
            snooze(1);
        }
        write32(device->mapped_registers + I2C_DATA_CMD, write_buf[i]);
    }

    // Lettura
    for (size_t i = 0; i < read_len; i++) {
        while (!(read32(device->mapped_registers + I2C_STATUS) & 0x2)) { // Attendi che il TX FIFO non sia pieno
            snooze(1);
        }
        write32(device->mapped_registers + I2C_DATA_CMD, 0x100); // Comando di lettura

        while (!(read32(device->mapped_registers + I2C_STATUS) & 0x8)) { // Attendi che il RX FIFO non sia vuoto
            snooze(1);
        }
        read_buf[i] = read32(device->mapped_registers + I2C_DATA_CMD) & 0xFF;
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

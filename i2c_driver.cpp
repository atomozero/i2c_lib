#include <Drivers.h>
#include <KernelExport.h>
#include <PCI.h>
#include <string.h>
#include <stdlib.h>

#define DRIVER_NAME "i2c"
#define DEVICE_NAME "bus/i2c/0"

// Vendor e Device ID per alcuni controller I2C comuni
#define INTEL_VENDOR_ID 0x8086
#define INTEL_LYNXPOINT_DEVICE_ID 0x9c61
#define INTEL_SUNRISEPOINT_DEVICE_ID 0x9d61

// Vendor e Device ID per touchpad comuni nei Huawei MateBook Pro
#define SYNAPTICS_VENDOR_ID 0x06CB
#define ELAN_VENDOR_ID 0x04F3

// Questi sono esempi e potrebbero dover essere aggiornati
#define SYNAPTICS_DEVICE_ID 0x122E  // Esempio di ID Synaptics
#define ELAN_DEVICE_ID 0x3022       // Esempio di ID ELAN

int32 api_version = B_CUR_DRIVER_API_VERSION;

struct i2c_device {
    uint32 base_addr;
    pci_info pci;
    bool is_touchpad;
    uint16 touchpad_vendor;
    uint16 touchpad_device;
};

static i2c_device* gDeviceList = NULL;
static uint32 gDeviceCount = 0;

static pci_module_info* gPCIModule;

// Prototipi delle funzioni del driver
status_t init_hardware(void);
status_t init_driver(void);
void uninit_driver(void);
const char** publish_devices(void);
device_hooks* find_device(const char* name);

// Funzioni del dispositivo
static status_t i2c_open(const char* name, uint32 flags, void** cookie);
static status_t i2c_close(void* cookie);
static status_t i2c_free(void* cookie);
static status_t i2c_read(void* cookie, off_t position, void* buffer, size_t* numBytes);
static status_t i2c_write(void* cookie, off_t position, const void* buffer, size_t* numBytes);
static status_t i2c_control(void* cookie, uint32 op, void* arg, size_t len);

static device_hooks gDeviceHooks = {
    i2c_open,
    i2c_close,
    i2c_free,
    i2c_control,
    i2c_read,
    i2c_write
};

// Funzione per rilevare i dispositivi I2C
static status_t probe_i2c_devices() {
    pci_info info;
    uint32 index = 0;
    uint32 found_devices = 0;

    while (gPCIModule->get_nth_pci_info(index++, &info) == B_OK) {
        bool is_i2c_controller = (info.vendor_id == INTEL_VENDOR_ID) &&
                                 ((info.device_id == INTEL_LYNXPOINT_DEVICE_ID) ||
                                  (info.device_id == INTEL_SUNRISEPOINT_DEVICE_ID));

        bool is_touchpad = (info.vendor_id == SYNAPTICS_VENDOR_ID && info.device_id == SYNAPTICS_DEVICE_ID) ||
                           (info.vendor_id == ELAN_VENDOR_ID && info.device_id == ELAN_DEVICE_ID);

        if (is_i2c_controller || is_touchpad) {
            dprintf(DRIVER_NAME ": Found %s at %02x:%02x.%x\n",
                    is_touchpad ? "touchpad" : "I2C controller",
                    info.bus, info.device, info.function);
            
            // Alloca memoria per il nuovo dispositivo
            i2c_device* new_devices = (i2c_device*)realloc(gDeviceList, (gDeviceCount + 1) * sizeof(i2c_device));
            if (new_devices == NULL) {
                dprintf(DRIVER_NAME ": Failed to allocate memory for new device\n");
                return B_NO_MEMORY;
            }
            
            gDeviceList = new_devices;
            gDeviceList[gDeviceCount].base_addr = info.u.h0.base_registers[0];
            memcpy(&gDeviceList[gDeviceCount].pci, &info, sizeof(pci_info));
            gDeviceList[gDeviceCount].is_touchpad = is_touchpad;
            if (is_touchpad) {
                gDeviceList[gDeviceCount].touchpad_vendor = info.vendor_id;
                gDeviceList[gDeviceCount].touchpad_device = info.device_id;
            }
            gDeviceCount++;
            found_devices++;
        }
    }

    if (found_devices == 0) {
        dprintf(DRIVER_NAME ": No compatible I2C controllers or touchpads found\n");
        return B_ERROR;
    }

    return B_OK;
}

// Implementazione delle funzioni del driver
status_t init_hardware(void)
{
    dprintf(DRIVER_NAME ": init_hardware()\n");
    return B_OK;
}

status_t init_driver(void)
{
    dprintf(DRIVER_NAME ": init_driver()\n");
    
    if (get_module(B_PCI_MODULE_NAME, (module_info**)&gPCIModule) != B_OK) {
        dprintf(DRIVER_NAME ": Failed to get PCI module\n");
        return B_ERROR;
    }

    status_t status = probe_i2c_devices();
    if (status != B_OK) {
        put_module(B_PCI_MODULE_NAME);
        return status;
    }

    return B_OK;
}

void uninit_driver(void)
{
    dprintf(DRIVER_NAME ": uninit_driver()\n");
    free(gDeviceList);
    put_module(B_PCI_MODULE_NAME);
}

const char** publish_devices(void)
{
    static const char* devices[2] = {
        DEVICE_NAME,
        NULL
    };
    return devices;
}

device_hooks* find_device(const char* name)
{
    if (!strcmp(name, DEVICE_NAME))
        return &gDeviceHooks;
    return NULL;
}

// Implementazione delle funzioni del dispositivo
static status_t i2c_open(const char* name, uint32 flags, void** cookie)
{
    if (gDeviceCount > 0) {
        *cookie = (void*)&gDeviceList[0];
        return B_OK;
    }
    return B_ERROR;
}

static status_t i2c_close(void* cookie)
{
    return B_OK;
}

static status_t i2c_free(void* cookie)
{
    return B_OK;
}

static status_t i2c_read(void* cookie, off_t position, void* buffer, size_t* numBytes)
{
    i2c_device* device = (i2c_device*)cookie;
    if (device->is_touchpad) {
        dprintf(DRIVER_NAME ": Reading from touchpad (vendor: 0x%04x, device: 0x%04x)\n",
                device->touchpad_vendor, device->touchpad_device);
        // Implementa la lettura specifica per il touchpad
    } else {
        dprintf(DRIVER_NAME ": Reading from I2C controller\n");
        // Implementa la lettura generica dal controller I2C
    }
    return B_OK;
}

static status_t i2c_write(void* cookie, off_t position, const void* buffer, size_t* numBytes)
{
    i2c_device* device = (i2c_device*)cookie;
    if (device->is_touchpad) {
        dprintf(DRIVER_NAME ": Writing to touchpad (vendor: 0x%04x, device: 0x%04x)\n",
                device->touchpad_vendor, device->touchpad_device);
        // Implementa la scrittura specifica per il touchpad
    } else {
        dprintf(DRIVER_NAME ": Writing to I2C controller\n");
        // Implementa la scrittura generica al controller I2C
    }
    return B_OK;
}

static status_t i2c_control(void* cookie, uint32 op, void* arg, size_t len)
{
    i2c_device* device = (i2c_device*)cookie;
    if (device->is_touchpad) {
        dprintf(DRIVER_NAME ": Control operation on touchpad (vendor: 0x%04x, device: 0x%04x)\n",
                device->touchpad_vendor, device->touchpad_device);
        // Implementa le operazioni di controllo specifiche per il touchpad
    } else {
        dprintf(DRIVER_NAME ": Control operation on I2C controller\n");
        // Implementa le operazioni di controllo generiche per il controller I2C
    }
    return B_OK;
}

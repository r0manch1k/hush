#ifndef HARDWARE_KEY_H
#define HARDWARE_KEY_H

#include <string>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#endif

namespace hardware_key {

struct USBDevice {
    std::string name;
    std::string fingerprint;  // Уникальный идентификатор устройства
    std::string vendor_id;
    std::string product_id;
    std::string serial_number;
};

#ifdef __APPLE__
// Получить строковое свойство USB устройства из IOKit
inline std::string get_usb_string_property(io_service_t device, CFStringRef property) {
    CFTypeRef property_ref = IORegistryEntryCreateCFProperty(device, property, kCFAllocatorDefault, 0);
    if (!property_ref) return "";

    std::string result;
    if (CFGetTypeID(property_ref) == CFStringGetTypeID()) {
        CFStringRef string_ref = (CFStringRef)property_ref;
        char        buffer[256];
        if (CFStringGetCString(string_ref, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
            result = buffer;
        }
    } else if (CFGetTypeID(property_ref) == CFNumberGetTypeID()) {
        CFNumberRef number_ref = (CFNumberRef)property_ref;
        int         value;
        if (CFNumberGetValue(number_ref, kCFNumberIntType, &value)) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%04x", value);
            result = buffer;
        }
    }

    CFRelease(property_ref);
    return result;
}

// Получить список всех подключенных USB устройств (съёмных носителей)
inline std::vector<USBDevice> get_usb_devices() {
    std::vector<USBDevice> devices;

    // Создаём словарь для поиска USB устройств
    CFMutableDictionaryRef matching_dict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matching_dict) return devices;

    io_iterator_t iterator;
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    kern_return_t kr =
        IOServiceGetMatchingServices(kIOMasterPortDefault, matching_dict, &iterator);
    #pragma clang diagnostic pop
    if (kr != KERN_SUCCESS) return devices;

    io_service_t device;
    while ((device = IOIteratorNext(iterator))) {
        USBDevice usb_device;

        // Получаем основные свойства
        usb_device.vendor_id  = get_usb_string_property(device, CFSTR("idVendor"));
        usb_device.product_id = get_usb_string_property(device, CFSTR("idProduct"));
        usb_device.serial_number = get_usb_string_property(device, CFSTR("USB Serial Number"));
        usb_device.name          = get_usb_string_property(device, CFSTR("USB Product Name"));

        // Если имени нет, пробуем другие варианты
        if (usb_device.name.empty()) {
            usb_device.name = get_usb_string_property(device, CFSTR("kUSBProductString"));
        }
        if (usb_device.name.empty()) {
            usb_device.name = "Unknown USB Device";
        }

        // Создаём fingerprint из vendor_id, product_id и serial_number
        // Это уникальный идентификатор конкретного устройства
        if (!usb_device.vendor_id.empty() && !usb_device.product_id.empty()) {
            usb_device.fingerprint =
                usb_device.vendor_id + ":" + usb_device.product_id;

            // Если есть серийный номер, добавляем его для большей уникальности
            if (!usb_device.serial_number.empty()) {
                usb_device.fingerprint += ":" + usb_device.serial_number;
            }

            // Фильтруем встроенные устройства (hub, bluetooth, камеры и т.д.)
            // Оставляем только массовые накопители
            std::string name_lower = usb_device.name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

            bool is_storage_device =
                name_lower.find("storage") != std::string::npos ||
                name_lower.find("disk") != std::string::npos ||
                name_lower.find("flash") != std::string::npos ||
                name_lower.find("card reader") != std::string::npos ||
                name_lower.find("mass storage") != std::string::npos;

            bool is_excluded =
                name_lower.find("hub") != std::string::npos ||
                name_lower.find("bluetooth") != std::string::npos ||
                name_lower.find("camera") != std::string::npos ||
                name_lower.find("keyboard") != std::string::npos ||
                name_lower.find("mouse") != std::string::npos;

            // Добавляем только потенциальные накопители или устройства с серийником
            if ((is_storage_device || !usb_device.serial_number.empty()) && !is_excluded) {
                devices.push_back(usb_device);
            }
        }

        IOObjectRelease(device);
    }

    IOObjectRelease(iterator);
    return devices;
}

// Проверить, подключено ли устройство с данным fingerprint
inline bool is_device_connected(const std::string& fingerprint) {
    if (fingerprint.empty()) return false;

    std::vector<USBDevice> devices = get_usb_devices();
    for (const auto& device : devices) {
        if (device.fingerprint == fingerprint) {
            return true;
        }
    }
    return false;
}

// Получить имя устройства по fingerprint (если подключено)
inline std::string get_device_name(const std::string& fingerprint) {
    if (fingerprint.empty()) return "";

    std::vector<USBDevice> devices = get_usb_devices();
    for (const auto& device : devices) {
        if (device.fingerprint == fingerprint) {
            return device.name;
        }
    }
    return "";
}

#else
// Заглушки для не-macOS платформ
inline std::vector<USBDevice> get_usb_devices() {
    return {};
}

inline bool is_device_connected(const std::string& fingerprint) {
    return true;  // На других платформах не проверяем
}

inline std::string get_device_name(const std::string& fingerprint) {
    return "";
}
#endif

}  // namespace hardware_key

#endif

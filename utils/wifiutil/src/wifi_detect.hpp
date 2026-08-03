#pragma once
#include <string>
#include <vector>

enum class WiFiVendor {
    INTEL,
    REALTEK,
    BROADCOM,
    QUALCOMM,   // Qualcomm Atheros (ath9k/ath10k/ath11k)
    MEDIATEK,   // MediaTek/Ralink mt76 family
    RALINK,     // Older Ralink (rt2800)
    APPLE,      // Apple Silicon (Asahi Linux) built-in WiFi
    UNKNOWN
};

enum class WiFiBus {
    PCI,
    USB,
    UNKNOWN_BUS
};

enum class KernelSupport {
    IN_KERNEL,      // driver built into mainline kernel, no extra packages
    NEEDS_FIRMWARE, // in-kernel driver but needs linux-firmware blobs
    DKMS_AUR,       // out-of-tree DKMS driver (from AUR or drv/)
    PROPRIETARY,    // proprietary driver (broadcom-wl)
    NONE
};

// Apple hardware context — affects which firmware packages are needed
struct AppleContext {
    bool        isApple;         // running on Apple hardware
    bool        isAppleSilicon;  // M1/M2/M3/M4 (Asahi Linux)
    std::string macModel;        // e.g. "MacBookPro18,3"
    std::string macSysVendor;    // "Apple Inc."
};

struct WiFiAdapter {
    std::string    ifname;
    std::string    chipName;
    std::string    vendorId;
    std::string    deviceId;
    std::string    pciAddr;
    std::string    currentDriver;
    WiFiBus        bus;
    WiFiVendor     vendor;
    bool           hasInterface;
};

struct WiFiDetectResult {
    std::vector<WiFiAdapter> adapters;
    AppleContext             apple;
};

class WiFiDetector {
public:
    WiFiDetectResult detect();

private:
    std::vector<WiFiAdapter> detectPCI();
    std::vector<WiFiAdapter> detectUSB();
    void                     enrichWithNetdev(std::vector<WiFiAdapter>& adapters);
    AppleContext             detectApple();
    WiFiVendor   resolveVendor(const std::string& vendorId);
    std::string  readFile(const std::string& path);
    std::string  trim(const std::string& s);
    std::string  currentDriver(const std::string& pciAddr);
};

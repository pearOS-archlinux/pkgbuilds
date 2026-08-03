#pragma once
#include <string>
#include <vector>

enum class GPUVendor {
    NVIDIA,
    AMD,
    INTEL,
    APPLE,       // Apple Silicon (M1/M2/M3/M4) via Asahi Linux
    S3,          // S3 Graphics (Virge / Savage / Chrome series)
    VIA,         // VIA Technologies (UniChrome / Chrome9 integrated GPUs)
    VMWARE,
    VIRTUALBOX,
    QEMU,
    UNKNOWN      // PCI vendor resolved from pci.ids, no known driver recipe
};

enum class VMType {
    NONE,
    VIRTUALBOX,
    VMWARE,
    QEMU_KVM,
    OTHER
};

struct GPUInfo {
    std::string pciAddr;
    std::string name;
    std::string vendorName;   // human-readable, from pci.ids if unknown
    std::string boardMaker;   // e.g. "Zotac" — from lspci subvendor (informational)
    std::string vendorId;     // hex e.g. "10de"
    std::string deviceId;     // hex e.g. "2204"
    GPUVendor   vendor;
    bool        isVirtual;
};

struct SystemInfo {
    VMType               vmType;
    std::string          vmName;
    std::vector<GPUInfo> gpus;
};

class GPUDetector {
public:
    SystemInfo detect();

private:
    VMType      detectVM();
    std::vector<GPUInfo> detectFromSysfs();
    std::vector<GPUInfo> detectFromLspci();
    std::string readFile(const std::string& path);
    std::string trim(const std::string& s);
    GPUVendor   resolveVendor(const std::string& vendorId);
    std::string vendorName(GPUVendor v, const std::string& rawId);
    // Look up vendor/device name from /usr/share/hwdata/pci.ids
    std::string lookupPciVendor(const std::string& vendorId);
    std::string lookupPciDevice(const std::string& vendorId, const std::string& deviceId);
    // Extract board manufacturer (subvendor) from lspci -v
    std::string detectBoardMaker(const std::string& pciAddr);
};

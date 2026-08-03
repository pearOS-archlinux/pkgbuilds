#include "gpu_detect.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <sstream>
#include <unistd.h>

// ── helpers ───────────────────────────────────────────────────────────────────

std::string GPUDetector::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::string s;
    std::getline(f, s);
    return s;
}

std::string GPUDetector::trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\n\r");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\n\r");
    return s.substr(a, b - a + 1);
}

static std::string execCmd(const std::string& cmd) {
    std::array<char, 512> buf;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buf.data(), buf.size(), pipe.get()))
        result += buf.data();
    return result;
}

// ── pci.ids lookup ────────────────────────────────────────────────────────────
// Format of /usr/share/hwdata/pci.ids:
//   VVVV  Vendor Name
//   \tDDDD  Device Name
//   \t\tSVVV SDDD  Subsystem Name

static const char* PCI_IDS_PATH = "/usr/share/hwdata/pci.ids";

std::string GPUDetector::lookupPciVendor(const std::string& vendorId) {
    std::ifstream f(PCI_IDS_PATH);
    if (!f) return "";
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '\t') continue;
        // vendor line: "VVVV  Name"
        if (line.size() >= 4 && line.substr(0, 4) == vendorId) {
            size_t sp = line.find("  ");
            if (sp != std::string::npos)
                return trim(line.substr(sp + 2));
        }
    }
    return "";
}

std::string GPUDetector::lookupPciDevice(const std::string& vendorId,
                                          const std::string& deviceId) {
    std::ifstream f(PCI_IDS_PATH);
    if (!f) return "";
    std::string line;
    bool inVendor = false;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line[0] != '\t') {
            // new vendor section
            inVendor = (line.size() >= 4 && line.substr(0, 4) == vendorId);
            continue;
        }
        if (!inVendor) continue;
        if (line.size() >= 5 && line[0] == '\t' && line[1] != '\t') {
            // device line: "\tDDDD  Name"
            if (line.substr(1, 4) == deviceId) {
                size_t sp = line.find("  ");
                if (sp != std::string::npos)
                    return trim(line.substr(sp + 2));
            }
        }
    }
    return "";
}

// ── board maker (subvendor) from lspci -v ────────────────────────────────────
// lspci -v shows "Subsystem: Zotac ..." for AIB cards

std::string GPUDetector::detectBoardMaker(const std::string& pciAddr) {
    std::string out = execCmd("lspci -v -s " + pciAddr + " 2>/dev/null");
    if (out.empty()) return "";
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find("Subsystem:");
        if (pos == std::string::npos) continue;
        std::string sub = trim(line.substr(pos + 10));
        // sub might be "Zotac Engineering Inc. GeForce RTX ..."
        // extract the manufacturer part (before the product name)
        // We'll just return the first word/company
        size_t sp = sub.find(' ');
        if (sp != std::string::npos) {
            // check for multi-word company names
            std::string first = sub.substr(0, sp);
            // common AIB brands
            static const std::vector<std::string> brands = {
                "ZOTAC", "ASUS", "MSI", "Gigabyte", "EVGA", "Palit",
                "Sapphire", "PowerColor", "XFX", "Galax", "Inno3D",
                "PNY", "Colorful", "AORUS", "Gainward", "Leadtek",
                "Club3D", "HIS", "Biostar", "ASRock"
            };
            for (const auto& b : brands) {
                std::string slow = sub, blow = b;
                std::transform(slow.begin(), slow.end(), slow.begin(), ::tolower);
                std::transform(blow.begin(), blow.end(), blow.begin(), ::tolower);
                if (slow.find(blow) != std::string::npos)
                    return b;
            }
            return first;  // return whatever the first word is
        }
        return sub;
    }
    return "";
}

// ── vendor mapping ────────────────────────────────────────────────────────────

GPUVendor GPUDetector::resolveVendor(const std::string& id) {
    if (id == "10de") return GPUVendor::NVIDIA;
    if (id == "1002") return GPUVendor::AMD;
    if (id == "8086") return GPUVendor::INTEL;
    if (id == "106b") return GPUVendor::APPLE;    // Apple Inc. (Asahi Linux)
    if (id == "5333") return GPUVendor::S3;       // S3 Graphics Ltd.
    if (id == "1106") return GPUVendor::VIA;      // VIA Technologies (UniChrome/Chrome9)
    if (id == "15ad") return GPUVendor::VMWARE;
    if (id == "80ee") return GPUVendor::VIRTUALBOX;
    if (id == "1af4" || id == "1234") return GPUVendor::QEMU;
    return GPUVendor::UNKNOWN;
}

std::string GPUDetector::vendorName(GPUVendor v, const std::string& rawId) {
    switch (v) {
        case GPUVendor::NVIDIA:      return "NVIDIA";
        case GPUVendor::AMD:         return "AMD";
        case GPUVendor::INTEL:       return "Intel";
        case GPUVendor::APPLE:       return "Apple";
        case GPUVendor::S3:          return "S3 Graphics";
        case GPUVendor::VIA:         return "VIA Technologies";
        case GPUVendor::VMWARE:      return "VMware";
        case GPUVendor::VIRTUALBOX:  return "VirtualBox";
        case GPUVendor::QEMU:        return "QEMU/KVM";
        default: {
            // Try to resolve from pci.ids
            std::string name = const_cast<GPUDetector*>(this)->lookupPciVendor(rawId);
            return name.empty() ? ("Unknown [" + rawId + "]") : name;
        }
    }
}

// ── VM detection ──────────────────────────────────────────────────────────────

VMType GPUDetector::detectVM() {
    std::string product = trim(readFile("/sys/class/dmi/id/product_name"));
    std::string sysVend = trim(readFile("/sys/class/dmi/id/sys_vendor"));
    std::string board   = trim(readFile("/sys/class/dmi/id/board_vendor"));

    auto ciContains = [](const std::string& haystack, const std::string& needle) {
        std::string h = haystack, n = needle;
        std::transform(h.begin(), h.end(), h.begin(), ::tolower);
        std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        return h.find(n) != std::string::npos;
    };

    if (ciContains(product, "virtualbox") || ciContains(sysVend, "innotek") ||
        ciContains(sysVend, "virtualbox") || ciContains(board, "virtualbox"))
        return VMType::VIRTUALBOX;

    if (ciContains(product, "vmware") || ciContains(sysVend, "vmware"))
        return VMType::VMWARE;

    if (ciContains(product, "qemu") || ciContains(sysVend, "qemu") ||
        ciContains(sysVend, "red hat") || ciContains(product, "kvm"))
        return VMType::QEMU_KVM;

    std::ifstream cpu("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpu, line)) {
        if (line.find("hypervisor") != std::string::npos)
            return VMType::OTHER;
    }

    return VMType::NONE;
}

// ── sysfs GPU detection ───────────────────────────────────────────────────────

std::vector<GPUInfo> GPUDetector::detectFromSysfs() {
    std::vector<GPUInfo> gpus;
    const char* base = "/sys/bus/pci/devices";
    DIR* dir = opendir(base);
    if (!dir) return gpus;

    auto stripHex = [](const std::string& s) -> std::string {
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            return s.substr(2);
        return s;
    };

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);
        if (name == "." || name == "..") continue;

        std::string devPath = std::string(base) + "/" + name;
        std::string classStr = trim(readFile(devPath + "/class"));
        if (classStr.empty()) continue;

        unsigned long pciClass = 0;
        try { pciClass = std::stoul(classStr, nullptr, 16); }
        catch (...) { continue; }

        unsigned long baseClass = (pciClass >> 8) & 0xFFFF;
        if (baseClass != 0x0300 && baseClass != 0x0302 && baseClass != 0x0380)
            continue;

        GPUInfo g;
        g.pciAddr  = name;
        g.vendorId = stripHex(trim(readFile(devPath + "/vendor")));
        g.deviceId = stripHex(trim(readFile(devPath + "/device")));
        std::transform(g.vendorId.begin(), g.vendorId.end(), g.vendorId.begin(), ::tolower);
        std::transform(g.deviceId.begin(), g.deviceId.end(), g.deviceId.begin(), ::tolower);

        g.vendor     = resolveVendor(g.vendorId);
        g.vendorName = vendorName(g.vendor, g.vendorId);
        g.isVirtual  = (g.vendor == GPUVendor::VMWARE    ||
                        g.vendor == GPUVendor::VIRTUALBOX ||
                        g.vendor == GPUVendor::QEMU);

        // Try to get device name from pci.ids
        std::string devName = lookupPciDevice(g.vendorId, g.deviceId);
        g.name = devName.empty()
            ? (g.vendorName + " GPU [" + g.vendorId + ":" + g.deviceId + "]")
            : (g.vendorName + " " + devName);

        // Append currently loaded kernel driver
        char drvBuf[512] = {};
        std::string drvLink = devPath + "/driver";
        if (readlink(drvLink.c_str(), drvBuf, sizeof(drvBuf) - 1) > 0) {
            std::string dl(drvBuf);
            size_t pos = dl.rfind('/');
            if (pos != std::string::npos)
                g.name += " (driver: " + dl.substr(pos + 1) + ")";
        }

        gpus.push_back(g);
    }
    closedir(dir);
    return gpus;
}

// ── lspci-based detection ─────────────────────────────────────────────────────

std::vector<GPUInfo> GPUDetector::detectFromLspci() {
    std::string out = execCmd("lspci -nn 2>/dev/null");
    if (out.empty()) return {};

    std::vector<GPUInfo> gpus;
    std::istringstream ss(out);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.find("VGA")     == std::string::npos &&
            line.find("3D")      == std::string::npos &&
            line.find("Display") == std::string::npos)
            continue;

        GPUInfo g;
        size_t sp = line.find(' ');
        if (sp == std::string::npos) continue;
        g.pciAddr = line.substr(0, sp);

        // Extract [vendor:device] at end
        size_t lb = line.rfind('[');
        size_t rb = line.rfind(']');
        if (lb != std::string::npos && rb != std::string::npos && lb < rb) {
            std::string ids = line.substr(lb + 1, rb - lb - 1);
            size_t col = ids.find(':');
            if (col != std::string::npos) {
                g.vendorId = ids.substr(0, col);
                g.deviceId = ids.substr(col + 1);
            }
        }

        // Device description from lspci (between ": " and "[ids]")
        size_t colon2 = line.find(": ");
        if (colon2 != std::string::npos) {
            size_t nameEnd = lb != std::string::npos ? lb : line.size();
            g.name = trim(line.substr(colon2 + 2, nameEnd - colon2 - 2));
        } else {
            g.name = line;
        }

        g.vendor     = resolveVendor(g.vendorId);
        g.vendorName = vendorName(g.vendor, g.vendorId);
        g.isVirtual  = (g.vendor == GPUVendor::VMWARE    ||
                        g.vendor == GPUVendor::VIRTUALBOX ||
                        g.vendor == GPUVendor::QEMU);

        // For UNKNOWN vendors: try to get a better name from pci.ids
        if (g.vendor == GPUVendor::UNKNOWN && !g.vendorId.empty()) {
            std::string pciVend = lookupPciVendor(g.vendorId);
            if (!pciVend.empty()) g.vendorName = pciVend;
            std::string pciDev  = lookupPciDevice(g.vendorId, g.deviceId);
            if (!pciDev.empty())
                g.name = g.vendorName + " " + pciDev;
        }

        // Detect board manufacturer (Zotac, ASUS, Sapphire, etc.)
        g.boardMaker = detectBoardMaker(g.pciAddr);

        gpus.push_back(g);
    }
    return gpus;
}

// ── public entry point ────────────────────────────────────────────────────────

SystemInfo GPUDetector::detect() {
    SystemInfo info;
    info.vmType = detectVM();

    switch (info.vmType) {
        case VMType::VIRTUALBOX: info.vmName = "VirtualBox";        break;
        case VMType::VMWARE:     info.vmName = "VMware";             break;
        case VMType::QEMU_KVM:   info.vmName = "QEMU/KVM";           break;
        case VMType::OTHER:      info.vmName = "Unknown hypervisor"; break;
        default:                 info.vmName = "";                   break;
    }

    info.gpus = detectFromLspci();
    if (info.gpus.empty())
        info.gpus = detectFromSysfs();

    // Finalize vendor/virtual flags after lspci enrichment
    for (auto& g : info.gpus) {
        if (!g.vendorId.empty()) {
            g.vendor    = resolveVendor(g.vendorId);
            g.isVirtual = (g.vendor == GPUVendor::VMWARE    ||
                           g.vendor == GPUVendor::VIRTUALBOX ||
                           g.vendor == GPUVendor::QEMU);
        }
    }

    return info;
}

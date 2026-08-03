#include "wifi_detect.hpp"
#include <algorithm>
#include <array>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <sstream>
#include <unistd.h>

// ── helpers ───────────────────────────────────────────────────────────────────

std::string WiFiDetector::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::string s;
    std::getline(f, s);
    return s;
}

std::string WiFiDetector::trim(const std::string& s) {
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

// ── vendor mapping ────────────────────────────────────────────────────────────

WiFiVendor WiFiDetector::resolveVendor(const std::string& id) {
    if (id == "8086") return WiFiVendor::INTEL;
    if (id == "10ec") return WiFiVendor::REALTEK;   // PCI
    if (id == "0bda") return WiFiVendor::REALTEK;   // USB (0x0bda = Realtek USB)
    if (id == "14e4") return WiFiVendor::BROADCOM;
    if (id == "168c") return WiFiVendor::QUALCOMM;  // Atheros/Qualcomm
    if (id == "0cf3") return WiFiVendor::QUALCOMM;  // Atheros USB
    if (id == "14c3") return WiFiVendor::MEDIATEK;  // PCIe MediaTek
    if (id == "1814") return WiFiVendor::RALINK;    // Ralink (legacy)
    if (id == "148f") return WiFiVendor::RALINK;    // Ralink USB
    // Other common USB WiFi vendors
    if (id == "0846") return WiFiVendor::REALTEK;   // Netgear (often Realtek)
    if (id == "0e8d") return WiFiVendor::MEDIATEK;  // MediaTek USB
    return WiFiVendor::UNKNOWN;
}

// ── Apple hardware detection ──────────────────────────────────────────────────

AppleContext WiFiDetector::detectApple() {
    AppleContext ctx{};
    ctx.isApple        = false;
    ctx.isAppleSilicon = false;

    ctx.macSysVendor = trim(readFile("/sys/class/dmi/id/sys_vendor"));
    ctx.macModel     = trim(readFile("/sys/class/dmi/id/product_name"));

    auto ciContains = [](const std::string& h, const std::string& n) {
        std::string lh = h, ln = n;
        std::transform(lh.begin(), lh.end(), lh.begin(), ::tolower);
        std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
        return lh.find(ln) != std::string::npos;
    };

    if (!ciContains(ctx.macSysVendor, "apple") && !ciContains(ctx.macModel, "apple"))
        return ctx;

    ctx.isApple = true;

    // Apple Silicon: Asahi Linux sets board vendor/product differently;
    // also check for ARM architecture and asahi kernel naming.
    std::string kernelArch = trim(execCmd("uname -m 2>/dev/null"));
    std::string kernelVer  = trim(execCmd("uname -r 2>/dev/null"));
    bool isARM = (kernelArch == "aarch64" || kernelArch == "arm64");
    bool hasAsahi = (kernelVer.find("asahi") != std::string::npos);

    // Also check if running on Apple Silicon by looking for apple,arm-platform
    // in device tree (present on Asahi Linux systems)
    bool hasAppleDT = (access("/proc/device-tree/compatible", F_OK) == 0);

    ctx.isAppleSilicon = isARM || hasAsahi || hasAppleDT;

    return ctx;
}

static std::string WiFiDetector_vendorNameImpl(WiFiVendor v, const std::string& rawId) {
    switch (v) {
        case WiFiVendor::INTEL:     return "Intel";
        case WiFiVendor::REALTEK:   return "Realtek";
        case WiFiVendor::BROADCOM:  return "Broadcom";
        case WiFiVendor::QUALCOMM:  return "Qualcomm Atheros";
        case WiFiVendor::MEDIATEK:  return "MediaTek";
        case WiFiVendor::RALINK:    return "Ralink";
        default:                    return "Unknown [" + rawId + "]";
    }
}

// Unused now (replaced by static impl above), keep to satisfy the header decl
// The header still declares vendorName but we removed it from .cpp
// Actually let's just drop the old one — the header no longer declares it.

// ── loaded kernel driver via sysfs symlink ────────────────────────────────────

std::string WiFiDetector::currentDriver(const std::string& pciAddr) {
    std::string link = "/sys/bus/pci/devices/" + pciAddr + "/driver";
    char buf[512] = {};
    if (readlink(link.c_str(), buf, sizeof(buf) - 1) > 0) {
        std::string s(buf);
        size_t p = s.rfind('/');
        return (p != std::string::npos) ? s.substr(p + 1) : s;
    }
    return "";
}

// ── PCI WiFi detection ────────────────────────────────────────────────────────
// PCI class 0x028000 = Network controller (covers WiFi, Bluetooth, etc.)
// We filter by known WiFi vendor IDs to avoid matching Ethernet/BT.

std::vector<WiFiAdapter> WiFiDetector::detectPCI() {
    std::string out = execCmd("lspci -nn 2>/dev/null");
    std::vector<WiFiAdapter> adapters;
    if (out.empty()) return adapters;

    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        // PCI class for WiFi network controllers
        if (line.find("Network controller") == std::string::npos &&
            line.find("Wireless")           == std::string::npos &&
            line.find("802.11")             == std::string::npos)
            continue;

        WiFiAdapter a;
        a.bus = WiFiBus::PCI;

        size_t sp = line.find(' ');
        if (sp == std::string::npos) continue;
        a.pciAddr = line.substr(0, sp);

        // Extract [vendor:device]
        size_t lb = line.rfind('[');
        size_t rb = line.rfind(']');
        if (lb != std::string::npos && rb != std::string::npos && lb < rb) {
            std::string ids = line.substr(lb + 1, rb - lb - 1);
            size_t col = ids.find(':');
            if (col != std::string::npos) {
                a.vendorId = ids.substr(0, col);
                a.deviceId = ids.substr(col + 1);
            }
        }

        a.vendor = resolveVendor(a.vendorId);

        // Skip Ethernet controllers (Broadcom/Realtek make both; filter by name)
        if (line.find("Ethernet") != std::string::npos &&
            line.find("Wireless") == std::string::npos &&
            line.find("802.11")   == std::string::npos)
            continue;

        // Chip description
        size_t colon2 = line.find(": ");
        if (colon2 != std::string::npos) {
            size_t nameEnd = lb != std::string::npos ? lb : line.size();
            a.chipName = trim(line.substr(colon2 + 2, nameEnd - colon2 - 2));
        } else {
            a.chipName = line;
        }

        a.currentDriver  = currentDriver(a.pciAddr);
        a.hasInterface   = false;
        adapters.push_back(a);
    }
    return adapters;
}

// ── USB WiFi detection ────────────────────────────────────────────────────────

std::vector<WiFiAdapter> WiFiDetector::detectUSB() {
    std::string out = execCmd("lsusb 2>/dev/null");
    std::vector<WiFiAdapter> adapters;
    if (out.empty()) return adapters;

    // Known USB WiFi vendor IDs (we only pick up devices with these)
    // lsusb format: Bus 001 Device 002: ID 0bda:8179 Realtek ...
    static const std::vector<std::string> wifiUsbVendors = {
        "0bda",  // Realtek
        "148f",  // Ralink/MediaTek
        "0cf3",  // Qualcomm Atheros
        "0e8d",  // MediaTek
        "0846",  // Netgear
        "2357",  // TP-Link (many are Realtek/MediaTek inside)
        "0411",  // Buffalo (Ralink inside)
        "7392",  // Edimax (Realtek)
        "050d",  // Belkin (Ralink/Realtek)
        "1286",  // Marvell
        "0a5c",  // Broadcom USB
    };

    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        // Find "ID VVVV:DDDD"
        size_t idPos = line.find(" ID ");
        if (idPos == std::string::npos) continue;
        std::string idStr = line.substr(idPos + 4);
        size_t colon = idStr.find(':');
        if (colon == std::string::npos || colon != 4) continue;

        std::string vid = idStr.substr(0, 4);
        std::string did = idStr.substr(5, 4);

        // Check if it's a known WiFi USB vendor
        bool isWifiVendor = false;
        for (const auto& v : wifiUsbVendors) {
            if (vid == v) { isWifiVendor = true; break; }
        }
        if (!isWifiVendor) continue;

        WiFiAdapter a;
        a.bus      = WiFiBus::USB;
        a.vendorId = vid;
        a.deviceId = did;
        a.vendor   = resolveVendor(vid);

        // Description: rest of line after "ID VVVV:DDDD "
        size_t descPos = idPos + 4 + 10; // "ID VVVV:DDDD "
        if (descPos < line.size())
            a.chipName = trim(line.substr(descPos));
        else
            a.chipName = "USB WiFi [" + vid + ":" + did + "]";

        // Skip obviously non-WiFi descriptions
        if (a.chipName.find("Hub") != std::string::npos ||
            a.chipName.find("hub") != std::string::npos ||
            a.chipName.find("Keyboard") != std::string::npos ||
            a.chipName.find("Mouse") != std::string::npos)
            continue;

        a.hasInterface  = false;
        adapters.push_back(a);
    }
    return adapters;
}

// ── enrich with netdev info (interface name, loaded driver) ───────────────────

void WiFiDetector::enrichWithNetdev(std::vector<WiFiAdapter>& adapters) {
    // Read /sys/class/net/ to find wireless interfaces
    DIR* dir = opendir("/sys/class/net");
    if (!dir) return;

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        std::string iface(ent->d_name);
        if (iface == "." || iface == "..") continue;

        // A wireless interface has /sys/class/net/<iface>/wireless/ directory
        std::string wirelessPath = "/sys/class/net/" + iface + "/wireless";
        if (access(wirelessPath.c_str(), F_OK) != 0) continue;

        // Get PCI device symlink
        std::string devLink = "/sys/class/net/" + iface + "/device";
        char devBuf[512] = {};
        if (readlink(devLink.c_str(), devBuf, sizeof(devBuf) - 1) <= 0) continue;

        std::string devPath(devBuf);
        // devPath is something like "../../devices/pci0000:00/0000:00:14.3/..."
        // Extract the last PCI address component
        size_t lastSlash = devPath.rfind('/');
        std::string pciAddr = (lastSlash != std::string::npos)
                              ? devPath.substr(lastSlash + 1) : devPath;

        // Get driver
        std::string drvLink = "/sys/class/net/" + iface + "/device/driver";
        char drvBuf[512] = {};
        std::string driver;
        if (readlink(drvLink.c_str(), drvBuf, sizeof(drvBuf) - 1) > 0) {
            std::string dl(drvBuf);
            size_t p = dl.rfind('/');
            driver = (p != std::string::npos) ? dl.substr(p + 1) : dl;
        }

        // Match to existing adapter by PCI address.
        // lspci gives "35:00.0"; sysfs uses "0000:35:00.0" (with domain).
        // We match if one is a suffix of the other.
        auto addrMatch = [](const std::string& a, const std::string& b) {
            if (a == b) return true;
            if (a.size() > b.size()) return a.rfind(b) == a.size() - b.size();
            if (b.size() > a.size()) return b.rfind(a) == b.size() - a.size();
            return false;
        };
        bool found = false;
        for (auto& a : adapters) {
            if (addrMatch(a.pciAddr, pciAddr)) {
                a.ifname        = iface;
                a.hasInterface  = true;
                if (!driver.empty()) a.currentDriver = driver;
                found = true;
                break;
            }
        }

        if (!found) {
            // Interface exists but wasn't detected by lspci (e.g. USB dongles)
            for (auto& a : adapters) {
                if (a.bus == WiFiBus::USB && a.ifname.empty() && !a.hasInterface) {
                    a.ifname       = iface;
                    a.hasInterface = true;
                    if (!driver.empty()) a.currentDriver = driver;
                    found = true;
                    break;
                }
            }
        }

        // If still not matched, create a minimal entry
        if (!found) {
            WiFiAdapter a;
            a.ifname       = iface;
            a.hasInterface = true;
            a.currentDriver = driver;
            a.bus          = WiFiBus::UNKNOWN_BUS;
            a.vendor       = WiFiVendor::UNKNOWN;
            a.chipName     = iface + " (details unavailable)";
            adapters.push_back(a);
        }
    }
    closedir(dir);
}

// ── public entry point ────────────────────────────────────────────────────────

WiFiDetectResult WiFiDetector::detect() {
    WiFiDetectResult result;

    result.adapters = detectPCI();
    auto usb = detectUSB();
    result.adapters.insert(result.adapters.end(), usb.begin(), usb.end());
    enrichWithNetdev(result.adapters);
    result.apple = detectApple();

    // Apple Silicon: WiFi isn't on a standard PCI bus, so lspci won't find it.
    // Inject a synthetic entry so the installer knows what to do.
    if (result.apple.isAppleSilicon) {
        bool alreadyHasApple = false;
        for (const auto& a : result.adapters)
            if (a.vendor == WiFiVendor::APPLE) { alreadyHasApple = true; break; }
        if (!alreadyHasApple) {
            WiFiAdapter a;
            a.chipName     = "Apple Silicon WiFi (" + result.apple.macModel + ")";
            a.vendorId     = "106b";
            a.deviceId     = "0000";
            a.bus          = WiFiBus::PCI;
            a.vendor       = WiFiVendor::APPLE;
            a.hasInterface = false;
            // Check for an active wlan interface even without lspci entry
            if (result.adapters.empty()) {
                result.adapters.push_back(a);
            } else {
                result.adapters.insert(result.adapters.begin(), a);
            }
        }
    }

    return result;
}

#include "driver_installer.hpp"
#include "wifi_detect.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"
#define C_CYAN   "\033[36m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_RED    "\033[31m"
#define C_DIM    "\033[2m"

static void printBanner() {
    std::cout << C_CYAN C_BOLD
              << "  _      _  ___ _      _   _ _\n"
                 " | |    (_)/ __(_)_   _| |_(_) |\n"
                 " | | /| | | (__ | | | | __| | |\n"
                 " | |/ |/ | |\\__ \\ | |_| | |_| | |\n"
                 " |__/|__/_||___/_|\\__,_|\\__|_|_|\n"
                 "  WiFi Driver Manager\n"
              << C_RESET << "\n";
}

static void printUsage(const char* prog) {
    std::cout
        << C_BOLD << "Usage:\n" << C_RESET
        << "  " << prog << " " C_CYAN "--detect" C_RESET
        << "\n      Detect WiFi adapter(s) and current driver status\n\n"
        << "  " << prog << " " C_CYAN "--list" C_RESET
        << "\n      List the driver package(s) for the detected chip, whether\n"
        << "      they're installed/available, and warn about conflicting packages\n\n"
        << "  " << prog << " " C_CYAN "--install" C_RESET " [--noconfirm]\n"
        << "      Install the recommended driver package via pacman/AUR helper.\n"
        << "      Run as a regular sudo-capable user (not via `sudo " << prog << " ...`)\n"
        << "      — pacman calls self-escalate with sudo, AUR builds need a non-root user.\n\n"
        << C_BOLD << "Supported chips:\n" << C_RESET
        << "  PCIe: Intel iwlwifi, Realtek (8821CE/8852BE/8723DE/8822BE...),\n"
        << "         Broadcom (wl/b43/brcmfmac), Qualcomm Atheros, MediaTek, Ralink\n"
        << "  USB:  Realtek (8821AU/8812AU/8188EU...), Ralink, MediaTek, Atheros\n\n";
}

static void printDetect(const std::vector<WiFiAdapter>& adapters) {
    if (adapters.empty()) {
        std::cout << C_RED << "[wifiutil] No WiFi adapters detected.\n" << C_RESET;
        return;
    }

    std::cout << C_BOLD << "[wifiutil] Detected WiFi adapter(s):\n" << C_RESET;
    int idx = 1;
    for (const auto& a : adapters) {
        std::string vendorStr;
        switch (a.vendor) {
            case WiFiVendor::INTEL:    vendorStr = "Intel";             break;
            case WiFiVendor::REALTEK:  vendorStr = "Realtek";           break;
            case WiFiVendor::BROADCOM: vendorStr = "Broadcom";          break;
            case WiFiVendor::QUALCOMM: vendorStr = "Qualcomm Atheros";  break;
            case WiFiVendor::MEDIATEK: vendorStr = "MediaTek";          break;
            case WiFiVendor::RALINK:   vendorStr = "Ralink";            break;
            case WiFiVendor::APPLE:    vendorStr = "Apple";             break;
            default:                   vendorStr = "Unknown";           break;
        }

        std::cout << "\n  " C_BOLD << idx++ << ". " << a.chipName << C_RESET << "\n";
        std::cout << "     Vendor  : " << vendorStr
                  << "  [" << a.vendorId << ":" << a.deviceId << "]\n";
        std::cout << "     Bus     : " << (a.bus == WiFiBus::PCI ? "PCIe" : "USB") << "\n";

        if (!a.pciAddr.empty())
            std::cout << "     PCI addr: " << a.pciAddr << "\n";
        if (a.hasInterface)
            std::cout << "     Netdev  : " C_GREEN << a.ifname << C_RESET << "\n";
        else
            std::cout << "     Netdev  : " C_YELLOW "none (driver not loaded?)" C_RESET "\n";
        if (!a.currentDriver.empty())
            std::cout << "     Driver  : " C_CYAN << a.currentDriver << C_RESET << "\n";
        else
            std::cout << "     Driver  : " C_RED "none loaded" C_RESET "\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    printBanner();

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string flag = argv[1];

    if (flag == "--help" || flag == "-h") {
        printUsage(argv[0]);
        return 0;
    }

    if (flag == "--detect") {
        WiFiDetector detector;
        auto adapters = detector.detect().adapters;
        printDetect(adapters);
        return 0;
    }

    if (flag == "--list") {
        WiFiDetector detector;
        auto adapters = detector.detect().adapters;

        if (adapters.empty()) {
            std::cout << C_RED << "[wifiutil] No WiFi adapters detected.\n" << C_RESET;
            return 0;
        }

        DriverInstaller installer;
        for (const auto& a : adapters)
            installer.listDrivers(a);

        return 0;
    }

    if (flag == "--install") {
        bool noConfirm = false;
        for (int i = 2; i < argc; ++i)
            if (std::string(argv[i]) == "--noconfirm") noConfirm = true;

        if (geteuid() == 0)
            std::cerr << C_YELLOW
                      << "[wifiutil] Warning: running as root. AUR-backed drivers\n"
                         "  (Realtek DKMS, broadcom-wl-dkms) will fail to build — run\n"
                         "  wifiutil as a regular (sudo-capable) user instead.\n"
                      << C_RESET;

        WiFiDetector detector;
        auto adapters = detector.detect().adapters;

        if (adapters.empty()) {
            std::cout << C_RED << "[wifiutil] No WiFi adapters detected.\n" << C_RESET;
            return 1;
        }

        printDetect(adapters);

        DriverInstaller installer;
        bool allOk = true;

        for (const auto& a : adapters) {
            std::cout << C_BOLD << "\n[wifiutil] Processing: " << a.chipName << C_RESET << "\n";
            auto result = installer.install(a, noConfirm);

            if (result.success) {
                std::cout << C_GREEN C_BOLD
                          << "  OK — " << result.message << "\n" << C_RESET;
            } else {
                std::cerr << C_RED
                          << "  FAIL — " << result.message << "\n" << C_RESET;
                allOk = false;
            }
        }

        return allOk ? 0 : 1;
    }

    std::cerr << C_RED << "[wifiutil] Unknown option: " << flag << C_RESET << "\n\n";
    printUsage(argv[0]);
    return 1;
}

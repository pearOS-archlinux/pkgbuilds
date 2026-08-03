#include "driver_manager.hpp"
#include "gpu_detect.hpp"
#include <algorithm>
#include <cstring>
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
              << "  _               _\n"
                 " | |__  _   _ _ _| |__  _ __ __   ___  ___  _ __\n"
                 " | '_ \\| | | | '_ \\ '__| '_ \\ \\ / / |/ __|| '__|\n"
                 " | | | | |_| | |_) | |  | | | \\ V /| |\\__ \\| |\n"
                 " |_| |_|\\__, | .__/|_|  |_| |_|\\_/ |_||___/|_|\n"
                 "        |___/|_|   GPU Driver Manager\n"
              << C_RESET << "\n";
}

static void printUsage(const char* prog) {
    std::cout
        << C_BOLD << "Usage:\n" << C_RESET
        << "  " << prog << " " C_CYAN "--detect" C_RESET
        << "\n      Detect GPU(s) and current drivers\n\n"
        << "  " << prog << " " C_CYAN "--list" C_RESET
        << " [--prefer <open|proprietary>] [--json]\n"
        << "      List available driver packages (--json for machine-readable output)\n\n"
        << "  " << prog << " " C_CYAN "--simulate" C_RESET " \"<GPU model>\""
        << " [--prefer <open|proprietary>] [--json]\n"
        << "      Show what would be installed for a typed GPU name (no real hardware needed)\n\n"
        << "  " << prog << " " C_CYAN "--clean" C_RESET " [--noconfirm]\n"
        << "      Remove EVERY known GPU driver package (any vendor, any generation)\n"
        << "      regardless of what's detected — for a clean slate. Prompts for\n"
        << "      confirmation unless --noconfirm is given. Chain with --install:\n"
        << "        " << prog << " --clean --noconfirm && " << prog << " --install --noconfirm\n\n"
        << "  " << prog << " " C_CYAN "--install" C_RESET
        << " [--prefer <open|proprietary>] [--noconfirm]\n"
        << "      Install the recommended driver(s). Run as a regular sudo-capable\n"
        << "      user (NOT via `sudo " << prog << " ...`) — pacman calls self-escalate\n"
        << "      with sudo, while AUR builds need a non-root user. --noconfirm skips\n"
        << "      all pacman/AUR prompts and, for DKMS packages, rebuilds the initramfs\n"
        << "      afterwards (mkinitcpio -P) — fully unattended, for install scripts.\n"
        << "      Also removes any driver package installed but known-wrong for this GPU\n"
        << "      or a different vendor entirely (e.g. leftover plain 'nvidia' next to\n"
        << "      nvidia-580xx-dkms, or an AMD driver on an NVIDIA-only machine). --list\n"
        << "      always warns about that in red, --noconfirm additionally removes it.\n"
        << "      For DKMS packages, auto-detects and adds the matching kernel headers\n"
        << "      package (any kernel naming scheme), and warns loudly (does NOT auto-fix)\n"
        << "      if the installed headers version doesn't match the running kernel — that\n"
        << "      always needs a manual kernel/headers upgrade, never done automatically.\n"
        << "      For NVIDIA Turing+, prefers a precompiled '<kernel-package>-nvidia-open'\n"
        << "      module over the generic nvidia-open-dkms build when the repo has one.\n\n"
        << C_BOLD << "Options:\n" << C_RESET
        << "  --prefer proprietary   Force proprietary drivers where available:\n"
        << "                           NVIDIA Turing+        → nvidia (full closed driver)\n"
        << "                           NVIDIA Kepler/Fermi/8-9xxx → legacy AUR driver (470xx/390xx/340xx)\n"
        << "                           AMD / Intel            → already open-source, no effect\n"
        << "  --prefer open           Force open-source (nouveau/mesa) even where an\n"
        << "                           open-kernel or legacy driver would be default\n"
        << C_DIM
        << "  (default, no flag)      NVIDIA Turing+                 → nvidia-open-dkms (open kernel)\n"
           "                           NVIDIA Pascal/Maxwell/Volta    → nvidia-580xx-dkms (AUR)\n"
           "                           NVIDIA Kepler/Fermi/8-9xxx     → xf86-video-nouveau + mesa\n"
           "                           AMD / Intel                    → mesa + open-source stack\n"
        << C_RESET << "\n";
}

// Guess a GPUVendor from a free-typed model name (used by --simulate, where
// there is no real PCI device to read a vendor ID from).
static GPUVendor guessVendorFromName(const std::string& name) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    auto has = [&](const char* s) { return n.find(s) != std::string::npos; };

    if (has("nvidia") || has("geforce") || has("gtx") || has("rtx") ||
        has("titan") || has("quadro") || has("tesla"))
        return GPUVendor::NVIDIA;
    if (has("amd") || has("radeon") || has("rx ") || has("rx5") || has("rx6") ||
        has("rx7") || has("rx9") || has("vega") || has("firepro"))
        return GPUVendor::AMD;
    if (has("intel") || has("iris") || has("uhd") || has("arc a") ||
        has("arc b") || has("hd graphics") || has("gma"))
        return GPUVendor::INTEL;
    if (has("s3") || has("virge") || has("savage") || has("chrome"))
        return GPUVendor::S3;
    if (has("via") || has("unichrome"))
        return GPUVendor::VIA;
    return GPUVendor::UNKNOWN;
}

static const char* vendorLabel(GPUVendor v) {
    switch (v) {
        case GPUVendor::NVIDIA: return "NVIDIA";
        case GPUVendor::AMD:    return "AMD";
        case GPUVendor::INTEL:  return "Intel";
        case GPUVendor::S3:     return "S3 Graphics";
        case GPUVendor::VIA:    return "VIA Technologies";
        default:                return "Unknown";
    }
}

// Parse --prefer <open|proprietary> from argv, starting at index `start`
static DriverPreference parsePreference(int argc, char* argv[], int start) {
    for (int i = start; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--prefer" && i + 1 < argc) {
            std::string val = argv[i + 1];
            if (val == "open")        return DriverPreference::OPEN_SOURCE;
            if (val == "proprietary") return DriverPreference::PROPRIETARY;
            std::cerr << C_YELLOW << "[hyprvisor] Unknown --prefer value: " << val
                      << " (use 'open' or 'proprietary')\n" << C_RESET;
        }
        // also support --prefer=open style
        if (arg.rfind("--prefer=", 0) == 0) {
            std::string val = arg.substr(9);
            if (val == "open")        return DriverPreference::OPEN_SOURCE;
            if (val == "proprietary") return DriverPreference::PROPRIETARY;
        }
    }
    return DriverPreference::AUTO;
}

static void printDetect(const SystemInfo& info) {
    if (!info.vmName.empty()) {
        std::cout << C_YELLOW C_BOLD
                  << "[hyprvisor] Virtualized environment: " << info.vmName
                  << C_RESET << "\n";
    }

    if (info.gpus.empty()) {
        std::cout << C_RED << "[hyprvisor] No GPU detected.\n" << C_RESET;
        return;
    }

    std::cout << C_BOLD << "[hyprvisor] Detected GPU(s):\n" << C_RESET;
    int idx = 1;
    for (const auto& g : info.gpus) {
        std::cout << "\n  " C_BOLD << idx++ << ". " << g.name << C_RESET;
        if (g.isVirtual)
            std::cout << C_YELLOW << "  [Virtual GPU]" << C_RESET;
        std::cout << "\n";
        std::cout << "     Vendor  : " << g.vendorName << "\n"
                  << "     PCI addr: " << g.pciAddr    << "\n"
                  << "     IDs     : " << g.vendorId << ":" << g.deviceId << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    bool jsonMode = false;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--json") jsonMode = true;

    if (!jsonMode) printBanner();

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
        GPUDetector detector;
        SystemInfo  info = detector.detect();
        printDetect(info);
        return 0;
    }

    if (flag == "--list") {
        DriverPreference pref = parsePreference(argc, argv, 2);
        bool wantJson = false;
        for (int i = 2; i < argc; ++i)
            if (std::string(argv[i]) == "--json") wantJson = true;

        GPUDetector   detector;
        SystemInfo    info = detector.detect();
        DriverManager mgr(info, pref);
        if (wantJson) mgr.listDriversJson();
        else          mgr.listDrivers();
        return 0;
    }

    if (flag == "--simulate") {
        if (argc < 3) {
            std::cerr << C_RED << "[hyprvisor] Usage: " << argv[0]
                      << " --simulate \"<GPU model name>\" [--prefer open|proprietary] [--json]\n"
                      << C_RESET;
            return 1;
        }
        std::string name = argv[2];

        GPUInfo g;
        g.name       = name;
        g.vendor     = guessVendorFromName(name);
        g.vendorName = vendorLabel(g.vendor);
        g.isVirtual  = false;

        SystemInfo info;
        info.vmType = VMType::NONE;
        info.gpus.push_back(g);

        DriverPreference pref = parsePreference(argc, argv, 3);
        bool wantJson = false;
        for (int i = 3; i < argc; ++i)
            if (std::string(argv[i]) == "--json") wantJson = true;

        if (g.vendor == GPUVendor::UNKNOWN)
            std::cerr << C_YELLOW
                      << "[hyprvisor] Could not guess a vendor from \"" << name
                      << "\" — try including NVIDIA/AMD/Intel/model keywords "
                         "(e.g. \"RTX\", \"Radeon\", \"Iris\").\n" << C_RESET;

        DriverManager mgr(info, pref);
        if (wantJson) mgr.listDriversJson();
        else          mgr.listDrivers();
        return 0;
    }

    if (flag == "--clean") {
        bool noConfirm = false;
        for (int i = 2; i < argc; ++i)
            if (std::string(argv[i]) == "--noconfirm") noConfirm = true;

        GPUDetector   detector;
        SystemInfo    info = detector.detect();
        DriverManager mgr(info, DriverPreference::AUTO);
        bool ok = mgr.cleanAllDrivers(noConfirm);
        return ok ? 0 : 1;
    }

    if (flag == "--install") {
        // No root requirement here on purpose: runPacman()/regenerateInitramfs()
        // escalate with `sudo` themselves as needed, while AUR packages must be
        // built as a regular user (AUR helpers refuse to run as root). Run this
        // as the normal user with sudo rights, not via `sudo hyprvisor ...`.
        if (geteuid() == 0)
            std::cerr << C_YELLOW
                      << "[hyprvisor] Warning: running as root. Packages needing an AUR\n"
                         "  helper (legacy NVIDIA branches, some S3/VIA drivers) will fail —\n"
                         "  run hyprvisor as a regular (sudo-capable) user instead.\n"
                      << C_RESET;

        DriverPreference pref = parsePreference(argc, argv, 2);
        bool noConfirm = false;
        for (int i = 2; i < argc; ++i)
            if (std::string(argv[i]) == "--noconfirm") noConfirm = true;

        if (noConfirm)
            std::cout << C_YELLOW
                      << "[hyprvisor] --noconfirm: installing unattended, no prompts.\n"
                      << C_RESET;

        GPUDetector   detector;
        SystemInfo    info = detector.detect();
        printDetect(info);
        DriverManager mgr(info, pref);
        bool ok = mgr.installBestDriver(noConfirm);
        return ok ? 0 : 1;
    }

    std::cerr << C_RED << "[hyprvisor] Unknown option: " << flag << C_RESET << "\n\n";
    printUsage(argv[0]);
    return 1;
}

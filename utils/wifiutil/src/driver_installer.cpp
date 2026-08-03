#include "driver_installer.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_CYAN    "\033[36m"
#define C_RED     "\033[31m"
#define C_MAGENTA "\033[35m"

static std::string execCmd(const std::string& cmd) {
    std::array<char, 256> buf;
    std::string out;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buf.data(), buf.size(), pipe.get()))
        out += buf.data();
    return out;
}

static bool strContains(const std::string& h, const std::string& n) {
    std::string lh = h, ln = n;
    std::transform(lh.begin(), lh.end(), lh.begin(), ::tolower);
    std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
    return lh.find(ln) != std::string::npos;
}

// ── system checks ─────────────────────────────────────────────────────────────

bool DriverInstaller::isModuleLoaded(const std::string& module) const {
    if (module.empty()) return false;
    std::string out = execCmd("lsmod 2>/dev/null");
    return out.find(module) != std::string::npos;
}

// Same reasoning as hyprvisor's isInstalled(): don't trust "pacman printed
// something" alone (a wrapped/aliased pacman could print a banner regardless
// of the real result) — require a line shaped like "<exact pkg name> <version>".
bool DriverInstaller::isPackageInstalled(const std::string& pkg) const {
    if (pkg.empty()) return false;
    std::string out = execCmd("pacman -Q " + pkg + " 2>/dev/null");
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        std::istringstream ls(line);
        std::string name, version;
        ls >> name >> version;
        if (name == pkg && !version.empty()) return true;
    }
    return false;
}

std::string DriverInstaller::detectAurHelper() const {
    if (!execCmd("which paru 2>/dev/null").empty())        return "paru";
    if (!execCmd("which yay 2>/dev/null").empty())          return "yay";
    if (!execCmd("which trizen 2>/dev/null").empty())       return "trizen";
    return "";
}

bool DriverInstaller::isPackageAvailable(const std::string& pkg, bool isAUR) const {
    if (pkg.empty()) return false;

    if (!isAUR) {
        // un-indented "repo/pkgname version" line = real pacman -Ss match
        std::string out = execCmd("pacman -Ss '^" + pkg + "$' 2>/dev/null");
        std::istringstream ss(out);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty() || std::isspace(static_cast<unsigned char>(line[0]))) continue;
            size_t slash = line.find('/');
            if (slash == std::string::npos) continue;
            std::istringstream ls(line.substr(slash + 1));
            std::string name, version;
            ls >> name >> version;
            if (name == pkg && !version.empty()) return true;
        }
        return false;
    }

    std::string helper = detectAurHelper();
    if (helper.empty()) return false;
    std::string out = execCmd(helper + " -Ss '^" + pkg + "$' 2>/dev/null");
    return out.find(pkg) != std::string::npos;
}

// ── package installation ──────────────────────────────────────────────────────

bool DriverInstaller::runPacman(const std::vector<std::string>& pkgs, bool noConfirm) const {
    if (pkgs.empty()) return false;
    std::string cmd = (geteuid() == 0) ? "pacman -S --needed" : "sudo pacman -S --needed";
    if (noConfirm) cmd += " --noconfirm";
    for (const auto& p : pkgs) cmd += " " + p;
    std::cout << C_CYAN << "  [+] " << cmd << C_RESET << "\n";
    return system(cmd.c_str()) == 0;
}

bool DriverInstaller::runAUR(const std::vector<std::string>& pkgs, bool noConfirm) const {
    if (pkgs.empty()) return false;

    if (geteuid() == 0) {
        std::cerr << C_RED
                  << "  [!] Can't build AUR packages while running as root.\n"
                     "      Run wifiutil as a regular user with sudo rights instead of\n"
                     "      via `sudo wifiutil ...` for AUR-backed drivers.\n"
                  << C_RESET;
        return false;
    }

    std::string helper = detectAurHelper();
    if (helper.empty()) {
        std::cerr << C_RED
                  << "  [!] No AUR helper found (paru/yay/trizen).\n"
                     "      Install paru: https://aur.archlinux.org/packages/paru\n"
                  << C_RESET;
        return false;
    }

    std::string cmd = helper + " -S --needed";
    if (noConfirm) cmd += " --noconfirm";
    for (const auto& p : pkgs) cmd += " " + p;
    std::cout << C_CYAN << "  [+] (AUR) " << cmd << C_RESET << "\n";
    return system(cmd.c_str()) == 0;
}

// Package that owns the running kernel's module directory, with "-headers"
// appended — works for any kernel naming scheme (linux, linux-zen,
// linux-cachyos-lts, ...) since it asks pacman directly instead of guessing.
std::string DriverInstaller::detectKernelHeadersPackage() const {
    std::string kernelRelease = execCmd("uname -r");
    while (!kernelRelease.empty() && std::isspace(static_cast<unsigned char>(kernelRelease.back())))
        kernelRelease.pop_back();
    if (kernelRelease.empty()) return "";

    std::string out = execCmd("pacman -Qo /usr/lib/modules/" + kernelRelease + " 2>/dev/null");
    const std::string marker = " is owned by ";
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        size_t pos = line.find(marker);
        if (pos == std::string::npos) continue;
        std::istringstream ls(line.substr(pos + marker.size()));
        std::string pkg;
        ls >> pkg;
        if (!pkg.empty() && pkg.find("-headers") == std::string::npos)
            return pkg + "-headers";
    }
    return "";
}

// ── package list builder ──────────────────────────────────────────────────────

// Realtek needs per-device-ID selection (many incompatible chips)
std::vector<DriverPackage> DriverInstaller::realtekPackages(const WiFiAdapter& a) {
    std::vector<DriverPackage> pkgs;

    auto add = [&](const std::string& name, const std::string& pkgName,
                   const std::string& mod, KernelSupport ks,
                   bool needsHdr = false, bool aur = false) {
        pkgs.push_back({name, pkgName, mod, needsHdr, aur, false, ks});
    };

    const std::string& id   = a.deviceId;
    const std::string& chip = a.chipName;

    // ══ PCIe (laptop-integrated, "xxxE" suffix) ════════════════════════════════
    // USB-suffix checks ("xxxU"/"xxxC" combo chips) are matched below BEFORE
    // any broad digit-only substring, so e.g. "RTL8852BU" never falls into the
    // 8852BE branch just because both contain "8852".

    // ── RTL8821CE — most common on laptops (2018-2022) ────────────────────────
    if (id == "c821" || strContains(chip, "8821ce")) {
        add("rtl8821ce-dkms", "rtl8821ce-dkms", "8821ce", KernelSupport::DKMS_AUR, true, true);
        add("linux-firmware (fallback rtw88_8821ce)", "linux-firmware", "rtw88_8821ce",
            KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }
    // ── RTL8723DE (laptop, common 2017-2019) ──────────────────────────────────
    if (id == "d723" || strContains(chip, "8723de")) {
        add("linux-firmware (rtl8723de in-kernel)", "linux-firmware", "rtl8723de",
            KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }
    // ── RTL8822BE (rtw88, in kernel 5.2+) ────────────────────────────────────
    if (id == "b822" || strContains(chip, "8822be")) {
        add("linux-firmware (rtw88_8822be in-kernel)", "linux-firmware", "rtw88_8822be",
            KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }
    // ── RTL8822CE (Wi-Fi 5, newer than 8822BE) ────────────────────────────────
    if (strContains(chip, "8822ce")) {
        add("rtl88x2ce-dkms-git", "rtl88x2ce-dkms-git", "rtl8822ce",
            KernelSupport::DKMS_AUR, true, true);
        add("linux-firmware (rtw88_8822ce in-kernel, kernel >= 5.9)", "linux-firmware",
            "rtw88_8822ce", KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }
    // ── RTL8851BE (Wi-Fi 6E, 2023+ laptops) ────────────────────────────────────
    if (strContains(chip, "8851be")) {
        add("linux-firmware (rtw89_8851be, kernel >= 6.2)", "linux-firmware",
            "rtw89_8851be", KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }
    // ── RTL8852AE / RTL8852BE / RTL8852CE (Wi-Fi 6/6E, 2021+ laptops) ────────
    if (strContains(chip, "8852ae") || strContains(chip, "8852be") ||
        strContains(chip, "8852ce") || id == "b852" || id == "8852") {
        add("rtl8852be-dkms", "rtl8852be-dkms", "rtl8852be", KernelSupport::DKMS_AUR, true, true);
        add("linux-firmware (rtw89_8852be/ce, kernel >= 5.18)", "linux-firmware",
            "rtw89_8852be", KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }

    // ══ USB dongles ════════════════════════════════════════════════════════════

    // ── RTL8188EU / RTL8188GU / RTL8188FU (cheap legacy N-only dongles) ──────
    if (id == "8179" || strContains(chip, "8188eu") || strContains(chip, "RTL8188EE")) {
        add("rtl8188eu-dkms", "rtl8188eu-dkms", "r8188eu", KernelSupport::DKMS_AUR, true, true);
        add("linux-firmware (rtl8188ee in-kernel)", "linux-firmware", "rtl8188ee",
            KernelSupport::NEEDS_FIRMWARE, false, false);
        return pkgs;
    }
    if (strContains(chip, "8188gu")) {
        add("rtl8188gu-dkms-git", "rtl8188gu-dkms-git", "8188gu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    if (strContains(chip, "8188fu")) {
        add("8188fu-dkms-git", "8188fu-dkms-git", "8188fu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8192CU / RTL8192EU (AC600-era dongles) ──────────────────────────────
    if (strContains(chip, "8192cu")) {
        add("8192cu-dkms", "8192cu-dkms", "8192cu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    if (strContains(chip, "8192eu")) {
        add("rtl8192eu-dkms", "rtl8192eu-dkms", "8192eu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8723BU / RTL8723DU (combo WiFi+BT, budget laptops/dongles) ────────
    if (strContains(chip, "8723bu")) {
        add("rtl8723bu-dkms-git", "rtl8723bu-dkms-git", "8723bu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    if (strContains(chip, "8723du")) {
        add("rtl8723du-dkms-git", "rtl8723du-dkms-git", "8723du", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8821AU (USB dongle) ─────────────────────────────────────────────────
    if (id == "0811" || strContains(chip, "8821au")) {
        add("rtl8821au-dkms", "rtl8821au-dkms", "8821au", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8821CU (USB, Wi-Fi 5, common TP-Link/Archer dongles) ──────────────
    if (strContains(chip, "8821cu")) {
        add("rtl8821cu-morrownr-dkms-git", "rtl8821cu-morrownr-dkms-git", "8821cu",
            KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8812AU (USB, AC1200 dongles) ──────────────────────────────────────
    if (id == "8812" || strContains(chip, "8812au")) {
        add("rtl8812au-dkms", "rtl8812au-dkms", "8812au", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8814AU (USB, AC1900, high-power adapters) ──────────────────────────
    if (strContains(chip, "8814au")) {
        add("rtl8814au-dkms-git", "rtl8814au-dkms-git", "8814au", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8822BU (USB, AC1200/AC1900, e.g. some Alfa/TP-Link) ────────────────
    if (strContains(chip, "8822bu")) {
        add("rtl8822bu-dkms", "rtl8822bu-dkms", "8822bu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8812BU / RTL8822BU combo silicon (88x2bu — Alfa AWUS036ACM etc.) ──
    if (strContains(chip, "88x2bu") || strContains(chip, "8812bu")) {
        add("rtl88x2bu-dkms-git", "rtl88x2bu-dkms-git", "88x2bu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8852AU / RTL8852BU (USB, Wi-Fi 6 dongles) ───────────────────────────
    if (strContains(chip, "8852au")) {
        add("rtl8852au-dkms-git", "rtl8852au-dkms-git", "8852au", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    if (strContains(chip, "8852bu")) {
        add("rtl8852bu-dkms-git", "rtl8852bu-dkms-git", "8852bu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8852CU (USB, Wi-Fi 6E dongles, 2023+) ───────────────────────────────
    if (strContains(chip, "8852cu")) {
        add("rtl8852cu-dkms-morrownr-git", "rtl8852cu-dkms-morrownr-git", "8852cu",
            KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── RTL8851BU (USB, Wi-Fi 6E dongles) ──────────────────────────────────────
    if (strContains(chip, "8851bu")) {
        add("rtl8851bu-dkms-git", "rtl8851bu-dkms-git", "8851bu", KernelSupport::DKMS_AUR, true, true);
        return pkgs;
    }
    // ── Generic Realtek fallback ───────────────────────────────────────────────
    // Unrecognized chip: try the in-kernel rtl8xxxu driver first (covers most
    // older RTL81xx/RTL8192-family USB chips out of the box), rtw88/rtw89
    // upstream backports as a second opinion for anything newer.
    add("linux-firmware (rtl8xxxu/rtw88/rtw89 in-kernel)", "linux-firmware", "",
        KernelSupport::NEEDS_FIRMWARE, false, false);
    add("rtw88-dkms-git (upstream backport, alternative to in-kernel)",
        "rtw88-dkms-git", "", KernelSupport::DKMS_AUR, true, true);
    add("rtw89-dkms-git (upstream backport, alternative to in-kernel)",
        "rtw89-dkms-git", "", KernelSupport::DKMS_AUR, true, true);
    return pkgs;
}

std::vector<DriverPackage> DriverInstaller::buildPackageList(const WiFiAdapter& a) {
    std::vector<DriverPackage> pkgs;

    auto add = [&](const std::string& name, const std::string& pkgName,
                   const std::string& mod, KernelSupport ks, bool needsHdr = false,
                   bool aur = false, bool fw = false) {
        pkgs.push_back({name, pkgName, mod, needsHdr, aur, fw, ks});
    };

    switch (a.vendor) {

    // ── Intel (iwlwifi in kernel, needs firmware) ──────────────────────────────
    case WiFiVendor::INTEL:
        add("linux-firmware", "linux-firmware", "iwlwifi",
            KernelSupport::NEEDS_FIRMWARE, false, false, true);
        break;

    // ── Realtek (per-chip logic) ──────────────────────────────────────────────
    case WiFiVendor::REALTEK:
        pkgs = realtekPackages(a);
        break;

    // ── Broadcom ──────────────────────────────────────────────────────────────
    // Three mutually-exclusive options; which one depends on the chip.
    // broadcom-wl: most BCM4xxx chips (proprietary, widest compat)
    // b43: older chips BCM43xx (open-source, needs b43-firmware)
    // brcmfmac: newer chips (in-kernel, needs linux-firmware)
    case WiFiVendor::BROADCOM: {
        bool useB43 = (a.deviceId == "4301" || a.deviceId == "4306" ||
                       a.deviceId == "4311" || a.deviceId == "4312" ||
                       a.deviceId == "4318" || a.deviceId == "4319" ||
                       a.deviceId == "4321" || a.deviceId == "4322" ||
                       strContains(a.chipName, "BCM431") ||
                       strContains(a.chipName, "BCM430"));
        bool useBrcmfmac = (a.deviceId == "43ba" || a.deviceId == "43a0" ||
                            strContains(a.chipName, "BCM43602") ||
                            strContains(a.chipName, "BCM43455") ||
                            strContains(a.chipName, "BCM4356"));
        if (useBrcmfmac) {
            add("linux-firmware (brcmfmac in-kernel)",
                "linux-firmware", "brcmfmac",
                KernelSupport::NEEDS_FIRMWARE, false, false, true);
        } else if (useB43) {
            add("b43-firmware",
                "b43-firmware", "b43",
                KernelSupport::DKMS_AUR, false, true, false);
            add("broadcom-wl-dkms (fallback)",
                "broadcom-wl-dkms", "wl",
                KernelSupport::PROPRIETARY, true, true, false);
        } else {
            add("broadcom-wl-dkms",
                "broadcom-wl-dkms", "wl",
                KernelSupport::PROPRIETARY, true, true, false);
            add("b43-firmware (open-source alternative)",
                "b43-firmware", "b43",
                KernelSupport::DKMS_AUR, false, true, false);
            add("linux-firmware (brcmfmac fallback)",
                "linux-firmware", "brcmfmac",
                KernelSupport::NEEDS_FIRMWARE, false, false, true);
        }
        break;
    }

    // ── Qualcomm Atheros (ath9k/ath10k/ath11k all in-kernel) ─────────────────
    case WiFiVendor::QUALCOMM:
        add("linux-firmware (ath10k/ath11k firmware)",
            "linux-firmware", "ath10k_pci",
            KernelSupport::NEEDS_FIRMWARE, false, false, true);
        break;

    // ── MediaTek (mt76 in-kernel since 5.12) ──────────────────────────────────
    case WiFiVendor::MEDIATEK:
        add("linux-firmware (mt76 firmware)",
            "linux-firmware", "mt7921e",
            KernelSupport::NEEDS_FIRMWARE, false, false, true);
        break;

    // ── Ralink (rt2800pci in-kernel) ──────────────────────────────────────────
    case WiFiVendor::RALINK:
        add("linux-firmware (rt2800 firmware)",
            "linux-firmware", "rt2800pci",
            KernelSupport::NEEDS_FIRMWARE, false, false, true);
        break;

    default:
        add("linux-firmware (generic firmware package)",
            "linux-firmware", "",
            KernelSupport::NEEDS_FIRMWARE, false, false, true);
        break;
    }

    return pkgs;
}

// ── vendor package universes ──────────────────────────────────────────────────
// Every real DKMS/proprietary package name that shows up in ANY chip branch
// of buildPackageList()/realtekPackages() for that vendor. Plain "linux-firmware"
// is left out — it's shared/always-needed, not a per-chip driver choice, so its
// presence alone never signals a conflict.

std::vector<std::string> DriverInstaller::vendorPackageUniverse(WiFiVendor vendor) const {
    switch (vendor) {
        case WiFiVendor::REALTEK:
            return {"rtl8821ce-dkms", "rtl8852be-dkms", "rtl8821au-dkms",
                    "rtl8812au-dkms", "rtl8188eu-dkms",
                    "rtl88x2ce-dkms-git", "rtl8188gu-dkms-git", "8188fu-dkms-git",
                    "8192cu-dkms", "rtl8192eu-dkms", "rtl8723bu-dkms-git",
                    "rtl8723du-dkms-git", "rtl8821cu-morrownr-dkms-git",
                    "rtl8814au-dkms-git", "rtl8822bu-dkms", "rtl88x2bu-dkms-git",
                    "rtl8852au-dkms-git", "rtl8852bu-dkms-git",
                    "rtl8852cu-dkms-morrownr-git", "rtl8851bu-dkms-git",
                    "rtw88-dkms-git", "rtw89-dkms-git"};
        case WiFiVendor::BROADCOM:
            return {"broadcom-wl-dkms", "b43-firmware"};
        default:
            return {};
    }
}

// ── installed packages + conflict detection ──────────────────────────────────

void DriverInstaller::printInstalledAndConflicts(const WiFiAdapter& adapter,
                                                   const std::vector<DriverPackage>& pkgs) const {
    std::string primary = pkgs.empty() ? "" : pkgs[0].pkgName;

    std::vector<std::string> candidates = vendorPackageUniverse(adapter.vendor);
    for (const auto& dp : pkgs) {
        if (dp.isFirmware || dp.pkgName == "linux-firmware") continue;
        if (std::find(candidates.begin(), candidates.end(), dp.pkgName) == candidates.end())
            candidates.push_back(dp.pkgName);
    }

    std::vector<std::string> installed;
    std::vector<std::string> conflicts;
    for (const auto& name : candidates) {
        if (!isPackageInstalled(name)) continue;
        installed.push_back(name);
        if (name != primary) conflicts.push_back(name);
    }

    if (installed.empty()) return;

    std::cout << "\n" C_BOLD "  Installed packages (vendor-relevant):\n" C_RESET;
    for (const auto& name : installed) {
        bool isPrimary = (name == primary);
        std::cout << "    " << (isPrimary ? C_GREEN : C_YELLOW) << name
                   << (isPrimary ? "  [recommended]" : "  [not the recommended driver]")
                   << C_RESET << "\n";
    }

    if (!conflicts.empty()) {
        std::cout << "\n" C_RED C_BOLD "  [!] Conflicting driver package(s) installed:\n" C_RESET;
        for (const auto& name : conflicts) {
            std::cout << C_RED << "    " << name
                       << " — installed but not the recommended driver for "
                       << adapter.chipName << ".\n"
                          "      Either a leftover from a different chip, or a "
                          "mutually-exclusive alternative\n"
                          "      driver bound to the same hardware — having both "
                          "installed can cause the\n"
                          "      wrong kernel module to load or a load-time clash. Remove with:\n"
                          "        sudo pacman -R " << name << "\n"
                       << C_RESET;
        }
    }
}

// ── pretty-print ──────────────────────────────────────────────────────────────

void DriverInstaller::printDriverRow(const DriverPackage& dp) const {
    bool installed = isPackageInstalled(dp.pkgName);
    bool available  = isPackageAvailable(dp.pkgName, dp.isAUR);
    bool loaded     = isModuleLoaded(dp.module);

    std::string typeStr;
    switch (dp.support) {
        case KernelSupport::IN_KERNEL:      typeStr = "in-kernel";   break;
        case KernelSupport::NEEDS_FIRMWARE: typeStr = "firmware";    break;
        case KernelSupport::DKMS_AUR:       typeStr = "dkms-aur";    break;
        case KernelSupport::PROPRIETARY:    typeStr = "proprietary"; break;
        default:                            typeStr = "unknown";     break;
    }

    std::string ann;
    if (dp.needsHeaders) ann += " [needs headers]";
    if (dp.isAUR)        ann += " [AUR]";

    printf("  %-36s %-11s  %-12s  %-12s  %s\n",
           (dp.name + ann).c_str(),
           typeStr.c_str(),
           installed ? (C_GREEN + std::string("installed") + C_RESET).c_str()
                     : (C_DIM   + std::string("not installed") + C_RESET).c_str(),
           available ? (C_GREEN + std::string("yes") + C_RESET).c_str()
                     : (C_RED   + std::string("no")  + C_RESET).c_str(),
           (loaded ? (std::string(C_GREEN) + "loaded" + C_RESET)
                   : (std::string(C_DIM)   + "not loaded" + C_RESET)).c_str());
}

// ── public listDrivers ────────────────────────────────────────────────────────

void DriverInstaller::listDrivers(const WiFiAdapter& adapter) {
    auto pkgs = buildPackageList(adapter);

    std::cout << "\n" C_BOLD "Adapter: " << adapter.chipName << C_RESET;
    if (adapter.hasInterface)
        std::cout << C_GREEN << "  [iface: " << adapter.ifname << "]" << C_RESET;
    if (!adapter.currentDriver.empty())
        std::cout << C_CYAN  << "  [driver: " << adapter.currentDriver << "]" << C_RESET;
    std::cout << "\n";
    std::cout << C_DIM
              << "  Bus: " << (adapter.bus == WiFiBus::PCI ? "PCIe" : "USB")
              << "  Vendor: " << adapter.vendorId
              << ":" << adapter.deviceId << C_RESET << "\n\n";

    printf("  %-36s %-11s  %-12s  %-12s  %s\n",
           "Package / Driver", "Type", "Installed", "Available", "System status");
    printf("  %s\n", std::string(90, '-').c_str());

    for (const auto& dp : pkgs)
        printDriverRow(dp);

    printInstalledAndConflicts(adapter, pkgs);
}

// ── public install ────────────────────────────────────────────────────────────

InstallResult DriverInstaller::install(const WiFiAdapter& adapter, bool noConfirm) {
    auto pkgs = buildPackageList(adapter);

    // Check if already working
    for (const auto& dp : pkgs) {
        if (!dp.module.empty() && isModuleLoaded(dp.module)) {
            return {true, true,
                    "Driver module '" + dp.module + "' already loaded — OK"};
        }
    }

    if (pkgs.empty())
        return {false, false, "No driver package known for this adapter"};

    // Only install the primary (first) recommendation.
    const DriverPackage& dp = pkgs[0];

    if (dp.needsHeaders) {
        std::string headers = detectKernelHeadersPackage();
        if (headers.empty()) {
            std::cerr << C_YELLOW
                      << "  [!] Could not detect kernel headers package — DKMS build may fail.\n"
                      << C_RESET;
        } else {
            std::cout << C_CYAN << "  Installing kernel headers: " << headers << "\n" << C_RESET;
            runPacman({headers, "dkms"}, noConfirm);
        }
    }

    bool ok = dp.isAUR ? runAUR({dp.pkgName}, noConfirm)
                        : runPacman({dp.pkgName}, noConfirm);

    if (!ok)
        return {false, false, "Failed to install " + dp.pkgName};

    std::string msg = "Installed " + dp.pkgName + ".";
    if (dp.needsHeaders)
        msg += " Reboot or run 'modprobe " + dp.module + "' to load the new driver.";
    return {true, false, msg};
}

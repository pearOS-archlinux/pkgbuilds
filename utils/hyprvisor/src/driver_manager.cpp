#include "driver_manager.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <cstdint>
#include <set>
#include <sstream>
#include <unistd.h>

// ── ANSI colours ──────────────────────────────────────────────────────────────
#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_CYAN   "\033[36m"
#define C_RED    "\033[31m"
#define C_DIM    "\033[2m"
#define C_MAGENTA "\033[35m"

static std::string execCmd(const std::string& cmd) {
    std::array<char, 256> buf;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buf.data(), buf.size(), pipe.get()))
        result += buf.data();
    return result;
}

// Package that owns the running kernel's module directory — e.g. "linux",
// "linux-zen", or a distro-custom name like "linux-cachyos-lts"/"linux618".
// Works for any kernel package naming scheme since it asks pacman directly
// instead of guessing from a fixed name list.
static std::string detectRunningKernelPackage() {
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
        // Skip the headers package itself if it's already installed and thus
        // also shows up in this query — we want the base kernel package name.
        if (!pkg.empty() && pkg.find("-headers") == std::string::npos)
            return pkg;
    }
    return "";
}

static bool strContains(const std::string& haystack, const std::string& needle) {
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

// Driver list entries for a known-wrong-for-this-GPU package (e.g. plain
// "nvidia" on a Pascal/Kepler/Fermi card) are always built with
// recommended=false and a description containing "NOT compatible" — if one
// of those is actually installed, it's a real problem, not just a suboptimal
// choice (see buildNvidiaList's Pascal/Volta/Maxwell/Kepler/Fermi/Tesla-G cases).
static bool isIncompatibleInstalled(const DriverInfo& d) {
    return d.installed && d.description.find("NOT compatible") != std::string::npos;
}

// ── full per-vendor package universes, for orphaned-driver detection ──────────
// Every package name that appears in ANY generation's buildXList() for that
// vendor, across all of buildNvidiaList/buildAmdList/buildIntelList/
// buildS3List/buildViaList. Packages shared across every vendor (mesa, the
// generic vesa/fbdev fallback DDX) are never flagged, so they're left out.

static const std::vector<std::string>& nvidiaPackageUniverse() {
    static const std::vector<std::string> v = {
        "nvidia", "nvidia-utils", "nvidia-dkms", "lib32-nvidia-utils",
        "nvidia-open", "nvidia-open-dkms",
        "nvidia-580xx-dkms", "nvidia-580xx-utils", "lib32-nvidia-580xx-utils",
        "nvidia-470xx-dkms", "nvidia-470xx-utils",
        "nvidia-390xx-dkms", "nvidia-390xx-utils",
        "nvidia-340xx-dkms", "nvidia-340xx-utils",
        "xf86-video-nouveau",
    };
    return v;
}

static const std::vector<std::string>& amdPackageUniverse() {
    static const std::vector<std::string> v = {
        "xf86-video-amdgpu", "vulkan-radeon", "lib32-vulkan-radeon",
        "rocm-opencl-runtime", "xf86-video-ati",
    };
    return v;
}

static const std::vector<std::string>& intelPackageUniverse() {
    static const std::vector<std::string> v = {
        "vulkan-intel", "lib32-vulkan-intel", "intel-compute-runtime",
        "intel-media-driver", "level-zero-loader", "intel-gpu-tools",
        "libva-intel-driver", "xf86-video-intel",
    };
    return v;
}

static const std::vector<std::string>& s3PackageUniverse() {
    static const std::vector<std::string> v = {
        "xf86-video-openchrome", "xf86-video-savage", "xf86-video-s3virge",
    };
    return v;
}

static const std::vector<std::string>& viaPackageUniverse() {
    static const std::vector<std::string> v = { "xf86-video-openchrome" };
    return v;
}

// ── constructor ───────────────────────────────────────────────────────────────

DriverManager::DriverManager(const SystemInfo& info, DriverPreference pref)
    : sysInfo_(info), pref_(pref) {}

// ── package helpers ───────────────────────────────────────────────────────────

bool DriverManager::isInstalled(const std::string& pkg) {
    // Don't just check "did pacman print anything" — some setups wrap/alias
    // pacman with something that prints a banner/MOTD to stdout on every
    // call regardless of the actual result, which would make every package
    // look "installed" and then `pacman -R <pkg>` fail with "target not
    // found" since it never really existed. Require a line that actually
    // looks like a real match: "<exact pkg name> <version>".
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

bool DriverManager::isAvailable(const std::string& pkg) {
    // Same reasoning as isInstalled(): don't trust "pacman printed something"
    // on its own (a wrapped/aliased pacman could print a banner regardless
    // of the real result) — require a line shaped like real `pacman -Ss`
    // output: an un-indented "repo/pkgname version" line (description lines
    // are indented) whose name matches exactly.
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

bool DriverManager::runPacman(const std::vector<std::string>& packages) {
    if (packages.empty()) return false;
    // Prefix with sudo so this works whether hyprvisor itself runs as root or
    // as a regular user (required for AUR builds — see runAUR below). Assumes
    // the invoking user has (passwordless, in unattended/--noconfirm contexts)
    // sudo rights, as is standard on an Arch live/install-script environment.
    std::string cmd = (geteuid() == 0) ? "pacman -S --needed" : "sudo pacman -S --needed";
    if (noConfirm_) cmd += " --noconfirm";
    for (const auto& p : packages) cmd += " " + p;
    std::cout << C_CYAN << "[hyprvisor] Running: " << cmd << C_RESET << "\n";
    return system(cmd.c_str()) == 0;
}

bool DriverManager::runPacmanRemove(const std::vector<std::string>& packages) {
    if (packages.empty()) return false;
    bool isRoot  = (geteuid() == 0);
    auto sudoCmd = [&](const std::string& c) { return isRoot ? c : ("sudo " + c); };

    std::string cmd = sudoCmd("pacman -R");
    if (noConfirm_) cmd += " --noconfirm";
    for (const auto& p : packages) cmd += " " + p;
    std::cout << C_CYAN << "[hyprvisor] Running: " << cmd << C_RESET << "\n";
    if (system(cmd.c_str()) == 0) return true;

    // First attempt failed (e.g. "target not found"). Re-check each package
    // against the local db with the same strict match isInstalled() uses —
    // if it's genuinely already gone, that's success, not failure. Anything
    // still genuinely installed gets one retry with -Rdd (skip dependency/
    // conflict checks), which resolves some local-db bookkeeping edge cases
    // plain -R rejects.
    std::vector<std::string> stillInstalled;
    for (const auto& p : packages)
        if (isInstalled(p)) stillInstalled.push_back(p);

    if (stillInstalled.empty()) {
        std::cout << C_DIM
                  << "  (already gone from the local package database — nothing to remove)\n"
                  << C_RESET;
        return true;
    }

    std::string retryCmd = sudoCmd("pacman -Rdd");
    if (noConfirm_) retryCmd += " --noconfirm";
    for (const auto& p : stillInstalled) retryCmd += " " + p;
    std::cout << C_YELLOW << "[hyprvisor] Retrying: " << retryCmd << C_RESET << "\n";
    return system(retryCmd.c_str()) == 0;
}

bool DriverManager::runAUR(const std::vector<std::string>& packages) {
    if (packages.empty()) return false;

    // AUR helpers (paru/yay/trizen) refuse to run as root — they need to be a
    // regular user and call `sudo` themselves for the final pacman -U step.
    // Run hyprvisor itself as a normal user (not via sudo) for this to work;
    // runPacman() above already adds its own sudo prefix as needed.
    if (geteuid() == 0) {
        std::cerr << C_RED
                  << "[hyprvisor] Can't build AUR packages while running as root.\n"
                     "  Run hyprvisor as a regular user with sudo rights instead of\n"
                     "  via `sudo hyprvisor ...` — it escalates only the pacman calls\n"
                     "  it actually needs, and AUR builds require a non-root user.\n"
                  << C_RESET;
        return false;
    }

    // detect available AUR helper
    std::string helper;
    if (!execCmd("which paru 2>/dev/null").empty())       helper = "paru";
    else if (!execCmd("which yay 2>/dev/null").empty())   helper = "yay";
    else if (!execCmd("which trizen 2>/dev/null").empty())helper = "trizen";

    if (helper.empty()) {
        std::cerr << C_RED
                  << "[hyprvisor] No AUR helper found (paru/yay/trizen).\n"
                     "  Install paru: https://aur.archlinux.org/packages/paru\n"
                     "  Then re-run: hyprvisor --install\n"
                  << C_RESET;
        return false;
    }

    std::string cmd = helper + " -S --needed";
    if (noConfirm_) cmd += " --noconfirm";
    for (const auto& p : packages) cmd += " " + p;
    std::cout << C_CYAN << "[hyprvisor] Running (AUR): " << cmd << C_RESET << "\n";
    return system(cmd.c_str()) == 0;
}

// ── AMD generation detection ──────────────────────────────────────────────────
//
// AMD PCI device ID ranges (approximate):
//   0x4c57–0x5c61  R100/R200 (Radeon 7000–8500/9200)       → TOO_OLD / R300_R500
//   0x5964–0x5c6b  R200 continued                          → TOO_OLD
//   0x4136–0x4c64  RV100/RV200 (Radeon 7000 PCI etc.)      → TOO_OLD
//   0x5144–0x5157  R100 desktop (Radeon DDR/SDR)           → TOO_OLD
//   0x4964–0x4966  RV250 (Radeon 9000)                     → R300_R500
//   0x5c61–0x5e4b  R300/R420 (Radeon 9500–X800)            → R300_R500
//   0x5e48–0x5f57  R420/R480 (Radeon X700/X800 XT)         → R300_R500
//   0x7100–0x71ff  R520/R580 (Radeon X1xxx)                → R300_R500
//   0x7200–0x7291  R600/RV610 (Radeon HD 2000)             → R600_R700
//   0x9400–0x9710  R700 (Radeon HD 4xxx)                   → R600_R700
//   0x6720–0x6799  Evergreen (Radeon HD 5xxx)              → EVERGREEN
//   0x6720–0x6820  Northern Islands (Radeon HD 6xxx)       → EVERGREEN
//   0x6780–0x683f  Southern Islands GCN 1.0 (HD 7xxx)      → GCN_OLD
//   0x6600–0x666f  Sea Islands GCN 1.1 (R7/R9 2xx)        → GCN_OLD
//   0x6900–0x697f  Volcanic Islands GCN 1.2 (R9 285/380)  → GCN_NEW
//   0x7300         Fiji (R9 Fury/Nano)                     → GCN_NEW
//   0x67c0–0x6fdf  Polaris (RX 4xx/5xx)                   → POLARIS
//   0x6860–0x687f  Vega 10/20 (RX Vega/Radeon VII)        → VEGA
//   0x15dd–0x15df  Raven/Picasso APU (Vega iGPU)          → VEGA
//   0x7310–0x73ff  Navi 10/14 RDNA 1 (RX 5xxx)            → RDNA1
//   0x73a0–0x73ff  Navi 21/22/23/24 RDNA 2 (RX 6xxx)      → RDNA2
//   0x744c–0x747f  Navi 31/32/33 RDNA 3 (RX 7xxx)         → RDNA3
//   0x7480–0x75ff  Navi 44/48 RDNA 4 (RX 9xxx)            → RDNA4

AmdGen DriverManager::detectAmdGen(const GPUInfo& gpu) {
    uint32_t id = 0;
    bool hasId = false;
    if (!gpu.deviceId.empty()) {
        try { id = static_cast<uint32_t>(std::stoul(gpu.deviceId, nullptr, 16)); hasId = true; }
        catch (...) {}
    }

    // ── device ID ranges (authoritative — checked BEFORE name matching) ───────
    if (hasId) {
        if (id >= 0x7480 && id <= 0x75ff) return AmdGen::RDNA4;
        if (id >= 0x744c && id <= 0x747f) return AmdGen::RDNA3;
        if (id >= 0x73a0 && id <= 0x73af) return AmdGen::RDNA2;
        if (id >= 0x7310 && id <= 0x739f) return AmdGen::RDNA1;
        if (id >= 0x6860 && id <= 0x687f) return AmdGen::VEGA;
        if (id >= 0x67c0 && id <= 0x6fdf) return AmdGen::POLARIS;
        if (id == 0x7300)                  return AmdGen::GCN_NEW;
        if (id >= 0x6900 && id <= 0x697f) return AmdGen::GCN_NEW;
        if (id >= 0x6780 && id <= 0x683f) return AmdGen::GCN_OLD;
        if (id >= 0x6600 && id <= 0x666f) return AmdGen::GCN_OLD;
        if (id >= 0x6720 && id <= 0x682f) return AmdGen::EVERGREEN;
        if (id >= 0x9400 && id <= 0x9710) return AmdGen::R600_R700;
        if (id >= 0x7100 && id <= 0x72ff) return AmdGen::R300_R500;
        if (id >= 0x5144 && id <= 0x6fdf) return AmdGen::R300_R500;
        if (id < 0x5144)                   return AmdGen::TOO_OLD;
        // id didn't land in any known range — fall through to name matching below
    }

    // ── name-based fallback (used when there's no PCI ID, or an ID outside
    // every known range) ────────────────────────────────────────────────────
    const std::string& n = gpu.name;
    // RDNA 4: RX 9000 series
    if (strContains(n, "RX 9") || strContains(n, "RX9") ||
        strContains(n, "9070") || strContains(n, "9060") || strContains(n, "9080"))
        return AmdGen::RDNA4;
    // RDNA 3: RX 7000 series
    if (strContains(n, "RX 7") || strContains(n, "7900") || strContains(n, "7800") ||
        strContains(n, "7700") || strContains(n, "7600"))
        return AmdGen::RDNA3;
    // RDNA 2: RX 6000 series
    if (strContains(n, "RX 6") || strContains(n, "6900") || strContains(n, "6800") ||
        strContains(n, "6700") || strContains(n, "6600") || strContains(n, "6500") ||
        strContains(n, "6400"))
        return AmdGen::RDNA2;
    // RDNA 1: RX 5000 series
    if (strContains(n, "RX 5") || strContains(n, "5700") || strContains(n, "5600") ||
        strContains(n, "5500") || strContains(n, "5300"))
        return AmdGen::RDNA1;
    // Vega
    if (strContains(n, "Vega") || strContains(n, "Radeon VII"))
        return AmdGen::VEGA;
    // Mac "Radeon Pro M###X" mobile naming (2013-2015 MacBook Pro dGPUs) —
    // the "M" between the tier and the number breaks the desktop "R9 3"/"R9 2"
    // substring checks below, and the model numbers alone are genuinely
    // ambiguous between GCN generations, so list known Mac SKUs explicitly.
    if (strContains(n, "M395"))                                return AmdGen::GCN_NEW;  // Tonga
    if (strContains(n, "M370") || strContains(n, "M290"))       return AmdGen::GCN_OLD;  // Cape Verde/Pitcairn rebadge
    // Polaris: RX 400/500 series (incl. Mac "Radeon Pro 450/455/555/555X/560/560X")
    if (strContains(n, "RX 4") || strContains(n, "RX 5") ||
        strContains(n, "590") || strContains(n, "580") || strContains(n, "570") ||
        strContains(n, "560") || strContains(n, "555") || strContains(n, "480") ||
        strContains(n, "470") || strContains(n, "460") || strContains(n, "455") ||
        strContains(n, "450"))
        return AmdGen::POLARIS;
    // GCN new: R9 285/380/390/Fury
    if (strContains(n, "R9 3") || strContains(n, "R9 2") || strContains(n, "Fury") ||
        strContains(n, "Nano") || strContains(n, "R9 285"))
        return AmdGen::GCN_NEW;
    // GCN old: HD 7xxx, R7/R9 2xx older
    if (strContains(n, "HD 7") || strContains(n, "R7 2") || strContains(n, "R9 2") ||
        strContains(n, "HD 8"))
        return AmdGen::GCN_OLD;
    // Evergreen/NI: HD 5xxx/6xxx
    if (strContains(n, "HD 5") || strContains(n, "HD 6"))
        return AmdGen::EVERGREEN;
    // r600/r700: HD 2xxx/3xxx/4xxx
    if (strContains(n, "HD 2") || strContains(n, "HD 3") || strContains(n, "HD 4"))
        return AmdGen::R600_R700;
    // R300-R500: Radeon 9xxx, X series
    if (strContains(n, "Radeon 9") || strContains(n, "Radeon X") ||
        strContains(n, "X1") || strContains(n, "X8") || strContains(n, "X7"))
        return AmdGen::R300_R500;
    // Very old: Radeon 7xxx/8xxx (not HD)
    if (strContains(n, "Radeon 7") || strContains(n, "Radeon 8") ||
        strContains(n, "7000") || strContains(n, "7200") || strContains(n, "7500") ||
        strContains(n, "8500"))
        return AmdGen::TOO_OLD;

    return AmdGen::UNKNOWN_AMD;
}

static const char* amdGenName(AmdGen g) {
    switch (g) {
        case AmdGen::RDNA4:      return "RDNA 4 (RX 9000 series, 2025)";
        case AmdGen::RDNA3:      return "RDNA 3 (RX 7000 series, 2022-2023)";
        case AmdGen::RDNA2:      return "RDNA 2 (RX 6000 series, 2020-2022)";
        case AmdGen::RDNA1:      return "RDNA 1 (RX 5000 series, 2019)";
        case AmdGen::VEGA:       return "Vega/GCN 5 (RX Vega/Radeon VII, 2017-2019)";
        case AmdGen::POLARIS:    return "Polaris/GCN 4 (RX 400/500, 2016-2018)";
        case AmdGen::GCN_NEW:    return "GCN 1.2-1.3 (R9 285/380/390/Fury, 2015-2016)";
        case AmdGen::GCN_OLD:    return "GCN 1.0-1.1 (HD 7xxx/R7-R9 2xx, 2012-2014)";
        case AmdGen::EVERGREEN:  return "Evergreen/NI (HD 5000-6000, 2009-2011)";
        case AmdGen::R600_R700:  return "r600/r700 (HD 2000-4000, 2007-2009)";
        case AmdGen::R300_R500:  return "R300-R500 (Radeon 9500-X1950, 2002-2007)";
        case AmdGen::TOO_OLD:    return "R100/R200 (Radeon 7000-8500, pre-2004)";
        default:                 return "Unknown AMD generation";
    }
}

// ── AMD driver list builder ───────────────────────────────────────────────────

std::vector<DriverInfo> DriverManager::buildAmdList(const GPUInfo& gpu) {
    std::vector<DriverInfo> list;
    AmdGen gen = detectAmdGen(gpu);

    auto add = [&](const std::string& pkg, const std::string& desc,
                   const std::string& type, bool rec,
                   bool aur = false, bool open = true) {
        DriverInfo d;
        d.packageName = pkg;
        d.description = desc;
        d.type        = type;
        d.installed   = isInstalled(pkg);
        d.recommended = rec;
        d.fromAUR     = aur;
        d.openSource  = open;
        list.push_back(d);
    };

    std::cout << C_DIM << "  AMD architecture : " << amdGenName(gen) << C_RESET << "\n";

    switch (gen) {

    // ── RDNA 4 / RDNA 3 / RDNA 2 / RDNA 1 / Vega / Polaris / GCN new ────────
    // Full amdgpu stack with Vulkan
    case AmdGen::RDNA4:
    case AmdGen::RDNA3:
    case AmdGen::RDNA2:
    case AmdGen::RDNA1:
    case AmdGen::VEGA:
    case AmdGen::POLARIS:
    case AmdGen::GCN_NEW:
        add("mesa",               "Mesa OpenGL/Vulkan (amdgpu kernel driver)",  "open-source", true);
        add("xf86-video-amdgpu",  "AMDGPU DDX Xorg driver",                    "open-source", true);
        add("vulkan-radeon",      "RADV Vulkan driver (part of Mesa)",          "open-source", true);
        add("lib32-mesa",         "32-bit Mesa (Steam/Wine/Proton)",            "open-source", false);
        add("lib32-vulkan-radeon","32-bit RADV Vulkan",                         "open-source", false);
        if (gen == AmdGen::RDNA3 || gen == AmdGen::RDNA4)
            add("rocm-opencl-runtime","ROCm OpenCL for RDNA3/4 compute",        "open-source", false);
        break;

    // ── GCN 1.0-1.1 (HD 7xxx, R7/R9 2xx) ───────────────────────────────────
    // amdgpu supported on kernels ≥ 4.9 with amdgpu.si_support=1 or amdgpu.cik_support=1
    // ati DDX still works as fallback; no hardware Vulkan
    case AmdGen::GCN_OLD:
        add("mesa",               "Mesa OpenGL (amdgpu or ati kernel driver)", "open-source", true);
        add("xf86-video-amdgpu",  "AMDGPU DDX (kernel param: amdgpu.si_support=1)", "open-source", true);
        add("xf86-video-ati",     "Legacy ATI DDX fallback",                   "open-source", false);
        add("lib32-mesa",         "32-bit Mesa",                               "open-source", false);
        std::cout << C_YELLOW
                  << "  [!] GCN 1.0/1.1 needs kernel param to enable amdgpu:\n"
                     "      For SI (HD 7xxx): amdgpu.si_support=1\n"
                     "      For CIK (R7/R9 2xx): amdgpu.cik_support=1\n"
                     "      Add to GRUB_CMDLINE_LINUX in /etc/default/grub\n"
                  << C_RESET;
        break;

    // ── Evergreen / Northern Islands (HD 5000-6000) ──────────────────────────
    // radeon kernel driver; xf86-video-ati DDX
    case AmdGen::EVERGREEN:
        add("mesa",              "Mesa OpenGL (radeon kernel driver)",         "open-source", true);
        add("xf86-video-ati",   "ATI DDX Xorg driver (Evergreen support)",    "open-source", true);
        add("lib32-mesa",        "32-bit Mesa",                               "open-source", false);
        std::cout << C_YELLOW
                  << "  [!] Uses 'radeon' kernel module (not amdgpu). No Vulkan support.\n"
                  << C_RESET;
        break;

    // ── r600/r700 (HD 2000-4000) ─────────────────────────────────────────────
    case AmdGen::R600_R700:
        add("mesa",              "Mesa OpenGL (r600 Gallium3D driver)",       "open-source", true);
        add("xf86-video-ati",   "ATI DDX Xorg driver",                       "open-source", true);
        add("lib32-mesa",        "32-bit Mesa",                               "open-source", false);
        std::cout << C_YELLOW
                  << "  [!] Uses 'radeon' kernel module. Basic 3D via r600 Gallium3D.\n"
                     "      No Vulkan support on this generation.\n"
                  << C_RESET;
        break;

    // ── R300-R500 (Radeon 9500–X1950) ────────────────────────────────────────
    case AmdGen::R300_R500:
        add("xf86-video-ati",   "ATI DDX — main driver for this generation",  "open-source", true);
        add("mesa",              "Mesa with r300 Gallium3D (basic OpenGL)",    "open-source", true);
        std::cout << C_YELLOW
                  << "  [!] R300-R500 era GPU. 'radeon' kernel module, r300 Gallium3D.\n"
                     "      Limited 3D performance. No Vulkan support.\n"
                  << C_RESET;
        break;

    // ── R100/R200 — Radeon 7000 PCI, 8500, etc. ──────────────────────────────
    case AmdGen::TOO_OLD:
        add("xf86-video-ati",   "ATI DDX — the only real option",             "open-source", true);
        add("mesa",              "Mesa with r100/r200 driver (very basic)",    "open-source", true);
        add("xf86-video-vesa",  "VESA DDX fallback",                           "open-source", false);
        std::cout << C_YELLOW
                  << "  [!] Very old ATI GPU (R100/R200 era, pre-2004).\n"
                     "      Uses 'radeon' kernel module + r100/r200 Mesa driver.\n"
                     "      No 3D acceleration in modern sense. No Vulkan.\n"
                     "      Consider using a newer GPU for any modern workload.\n"
                  << C_RESET;
        break;

    default:
        add("mesa",              "Mesa OpenGL/Vulkan stack",                   "open-source", true);
        add("xf86-video-amdgpu","AMDGPU DDX (for GCN+ cards)",                "open-source", false);
        add("xf86-video-ati",   "ATI DDX (for older cards)",                  "open-source", false);
        add("vulkan-radeon",    "RADV Vulkan (for GCN 1.2+ cards)",           "open-source", false);
        break;
    }

    return list;
}

// ── Intel generation detection ────────────────────────────────────────────────
//
// All Intel GPUs share vendor ID 0x8086, so we distinguish by device ID + name.
//
// Arc Alchemist (Xe-HPG) device IDs: 0x56a0–0x56cf
//   A770=0x56a0, A750=0x56a1, A580=0x56a5, A380=0x56a6
//   A770M=0x56c0, A730M=0x56c1, Arc Pro A40=0x56b0
//
// Arc Battlemage (Xe2-HPG) device IDs: ~0xe200–0xe2ff
//   B580=0xe202, B570=0xe20b
//
// Iris Xe iGPU (Xe-LP): Tiger Lake 0x9a40–0x9a7f, Alder Lake 0x46xx, Raptor 0xa7xx
// UHD / HD modern (Skylake–Comet Lake): 0x1900–0x9bxx (excluding Arc/Xe ranges)
// HD legacy (Sandy/Ivy/Haswell/Broadwell): 0x0100–0x17ff
// GMA (pre-HD): <0x0100 and 0x2500–0x2bxx

IntelGen DriverManager::detectIntelGen(const GPUInfo& gpu) {
    // ── device ID ranges (authoritative — checked BEFORE name matching) ───────
    uint32_t id = 0;
    bool hasId = false;
    if (!gpu.deviceId.empty()) {
        try { id = static_cast<uint32_t>(std::stoul(gpu.deviceId, nullptr, 16)); hasId = true; }
        catch (...) {}
    }

    if (hasId) {
        // Arc Battlemage (Xe2-HPG): B580/B570
        if (id >= 0xe200 && id <= 0xe2ff) return IntelGen::ARC_BATTLEMAGE;
        // Arc Alchemist (Xe-HPG): A-series discrete
        if (id >= 0x56a0 && id <= 0x56cf) return IntelGen::ARC_ALCHEMIST;
        // Iris Xe iGPU: Tiger Lake, Alder Lake, Raptor Lake
        if ((id >= 0x9a40 && id <= 0x9a7f) ||   // Tiger Lake Iris Xe
            (id >= 0x4680 && id <= 0x46d0) ||   // Alder Lake iGPU
            (id >= 0xa780 && id <= 0xa7bf))      // Raptor Lake iGPU
            return IntelGen::IRIS_XE;
        // Meteor Lake, etc. — newer iGPUs
        if (id >= 0x7d40 && id <= 0x7d7f) return IntelGen::IRIS_XE;
        // Skylake through Comet Lake UHD/HD 500-600 series
        if ((id >= 0x1900 && id <= 0x1940) ||   // Skylake
            (id >= 0x5900 && id <= 0x5940) ||   // Kaby Lake
            (id >= 0x3e90 && id <= 0x3ef0) ||   // Coffee Lake
            (id >= 0x9b20 && id <= 0x9bff) ||   // Comet Lake
            (id >= 0x8a50 && id <= 0x8a71))     // Ice Lake
            return IntelGen::HD_MODERN;
        // Sandy/Ivy/Haswell/Broadwell HD Graphics
        if ((id >= 0x0100 && id <= 0x0166) ||   // Sandy/Ivy Bridge
            (id >= 0x0400 && id <= 0x0416) ||   // Haswell
            (id >= 0x1600 && id <= 0x1626))     // Broadwell
            return IntelGen::HD_LEGACY;
        // GMA era
        if ((id >= 0x2560 && id <= 0x2b00) ||
            (id >= 0x35e0 && id <= 0x35ef))
            return IntelGen::GMA;
        // id didn't land in any known range — fall through to name matching below
    }

    // ── name-based fallback (used when there's no PCI ID, or an ID outside
    // every known range) ────────────────────────────────────────────────────
    const std::string& n = gpu.name;
    if (strContains(n, "Arc B") || strContains(n, "Battlemage"))
        return IntelGen::ARC_BATTLEMAGE;
    if (strContains(n, "Arc A") || strContains(n, "Arc Pro") || strContains(n, "Alchemist"))
        return IntelGen::ARC_ALCHEMIST;
    if (strContains(n, "Iris Xe") || strContains(n, "Iris(R) Xe"))
        return IntelGen::IRIS_XE;
    if (strContains(n, "UHD"))
        return IntelGen::HD_MODERN;  // UHD 600–770 series
    // "Iris Graphics 540/550/6100" (Skylake, e.g. 2016 MacBook Pro 13") — plain
    // "Iris Graphics", no "Plus"/"Pro"/"Xe" suffix, same driver tier as HD 530.
    if (strContains(n, "Iris Plus") || strContains(n, "Iris Pro") ||
        strContains(n, "Iris Graphics"))
        return IntelGen::HD_MODERN;
    // Bare "Intel HD Graphics" with no model number anywhere in the name at
    // all (Arrandale/Ironlake, e.g. 2010 MacBook Pro 13") predates Sandy
    // Bridge — far too old for Vulkan/VA-API. Every later "HD Graphics ####"
    // name carries a digit (after "Graphics", not "HD", so a positional
    // "HD 4"/"HD 5"/... check wouldn't catch it) — check for any digit at all.
    bool hasAnyDigit = std::any_of(n.begin(), n.end(), [](unsigned char c) { return std::isdigit(c); });
    if (strContains(n, "HD Graphics") && !hasAnyDigit)
        return IntelGen::HD_LEGACY;
    // Sandy Bridge "HD Graphics 2000/3000" (2011 MacBook Pro/Air) — no Vulkan,
    // no VA-API iHD support; the "HD Graphics" catch-all below would otherwise
    // wrongly bucket it as HD_MODERN since its model number follows "Graphics",
    // not "HD", so a positional "HD 2"/"HD 3" substring check can't catch it.
    if (strContains(n, "Graphics 2000") || strContains(n, "Graphics 3000"))
        return IntelGen::HD_LEGACY;
    if (strContains(n, "HD Graphics") || strContains(n, "HD 4") ||
        strContains(n, "HD 5") || strContains(n, "HD 6"))
        return IntelGen::HD_MODERN;
    if (strContains(n, "HD 2") || strContains(n, "HD 3"))
        return IntelGen::HD_LEGACY;
    if (strContains(n, "GMA") || strContains(n, "G45") || strContains(n, "G41"))
        return IntelGen::GMA;

    return IntelGen::UNKNOWN_INTEL;
}

static const char* intelGenName(IntelGen g) {
    switch (g) {
        case IntelGen::ARC_BATTLEMAGE: return "Arc Battlemage (B580/B570, Xe2-HPG, 2024-2025)";
        case IntelGen::ARC_ALCHEMIST:  return "Arc Alchemist (A380/A580/A750/A770, Xe-HPG, 2022-2023)";
        case IntelGen::IRIS_XE:        return "Iris Xe (Xe-LP iGPU, Tiger/Alder/Raptor Lake)";
        case IntelGen::HD_MODERN:      return "UHD / HD 500-600 Graphics (Skylake–Comet Lake)";
        case IntelGen::HD_LEGACY:      return "HD Graphics 2000-6000 (Sandy/Ivy/Haswell/Broadwell)";
        case IntelGen::GMA:            return "Intel GMA (pre-HD era, 2004-2008)";
        default:                       return "Unknown Intel GPU generation";
    }
}

// ── Intel driver list builder ─────────────────────────────────────────────────

std::vector<DriverInfo> DriverManager::buildIntelList(const GPUInfo& gpu) {
    std::vector<DriverInfo> list;
    IntelGen gen = detectIntelGen(gpu);

    auto add = [&](const std::string& pkg, const std::string& desc,
                   const std::string& type, bool rec,
                   bool aur = false, bool open = true) {
        DriverInfo d;
        d.packageName = pkg;
        d.description = desc;
        d.type        = type;
        d.installed   = isInstalled(pkg);
        d.recommended = rec;
        d.fromAUR     = aur;
        d.openSource  = open;
        list.push_back(d);
    };

    std::cout << C_DIM << "  Intel generation : " << intelGenName(gen) << C_RESET << "\n";

    switch (gen) {

    // ── Arc Battlemage — Xe2-HPG discrete (B580/B570) ─────────────────────────
    // Kernel: xe driver (default since kernel 6.8 for Battlemage)
    // Mesa ANV Vulkan + intel-compute-runtime for OpenCL/oneAPI
    case IntelGen::ARC_BATTLEMAGE:
        add("mesa",                  "Mesa OpenGL + ANV Vulkan (Xe2 support)",    "open-source", true);
        add("vulkan-intel",          "Intel ANV Vulkan driver (Battlemage)",       "open-source", true);
        add("intel-compute-runtime", "Intel OpenCL/oneAPI (GPGPU on Arc)",         "open-source", true);
        add("intel-media-driver",    "Intel iHD VA-API (hardware decode/encode)",  "open-source", true);
        add("lib32-mesa",            "32-bit Mesa (Steam/Wine/Proton)",            "open-source", false);
        add("lib32-vulkan-intel",    "32-bit ANV Vulkan",                          "open-source", false);
        add("level-zero-loader",     "Level Zero API (oneAPI compute framework)",  "open-source", false);
        add("intel-gpu-tools",       "Intel GPU debug/monitor tools",              "open-source", false);
        std::cout << C_CYAN
                  << "  [i] Arc Battlemage uses the 'xe' kernel driver (included in\n"
                     "      Linux kernel >= 6.8). No additional kernel package needed.\n"
                     "      For cutting-edge Xe2 Vulkan features, ensure mesa >= 24.2.\n"
                  << C_RESET;
        break;

    // ── Arc Alchemist — Xe-HPG discrete (A380/A580/A750/A770) ────────────────
    // Kernel: xe (6.8+, preferred) or i915 with GuC (older kernels)
    // Notable: GuC/HuC firmware loaded from linux-firmware; no proprietary blob
    case IntelGen::ARC_ALCHEMIST:
        add("mesa",                  "Mesa OpenGL + ANV Vulkan (Arc support)",     "open-source", true);
        add("vulkan-intel",          "Intel ANV Vulkan driver (Alchemist)",         "open-source", true);
        add("intel-compute-runtime", "Intel OpenCL/oneAPI (GPGPU — key for Arc)",  "open-source", true);
        add("intel-media-driver",    "Intel iHD VA-API (hardware QSV encode)",      "open-source", true);
        add("lib32-mesa",            "32-bit Mesa (Steam/Wine/Proton)",             "open-source", false);
        add("lib32-vulkan-intel",    "32-bit ANV Vulkan",                           "open-source", false);
        add("level-zero-loader",     "Level Zero API (oneAPI compute framework)",   "open-source", false);
        add("intel-gpu-tools",       "Intel GPU debug/monitor (intel_gpu_top)",     "open-source", false);
        add("linux-firmware",        "Firmware blobs (GuC/HuC for Arc scheduling)", "open-source", false);
        std::cout << C_CYAN
                  << "  [i] Arc Alchemist uses the 'xe' kernel driver on Linux >= 6.8,\n"
                     "      or 'i915' with GuC on older kernels. Both are open-source.\n"
                     "      Resizable BAR (Above 4G Decoding) recommended in UEFI for\n"
                     "      full VRAM visibility (especially A750/A770).\n"
                  << C_RESET;
        break;

    // ── Iris Xe iGPU (Tiger/Alder/Raptor Lake) ────────────────────────────────
    case IntelGen::IRIS_XE:
        add("mesa",               "Mesa OpenGL + ANV Vulkan (Iris Xe)",          "open-source", true);
        add("vulkan-intel",       "Intel ANV Vulkan driver",                      "open-source", true);
        add("intel-media-driver", "Intel iHD VA-API driver (Xe iGPU)",           "open-source", true);
        add("lib32-mesa",         "32-bit Mesa (Steam/Wine/Proton)",              "open-source", false);
        add("lib32-vulkan-intel", "32-bit ANV Vulkan",                            "open-source", false);
        break;

    // ── UHD / HD 500-600 series (Skylake–Comet Lake) ─────────────────────────
    case IntelGen::HD_MODERN:
        add("mesa",               "Mesa OpenGL/Vulkan stack",                    "open-source", true);
        add("vulkan-intel",       "Intel ANV Vulkan (Skylake+ required)",         "open-source", true);
        add("intel-media-driver", "Intel iHD VA-API (Broadwell+ recommended)",   "open-source", true);
        add("lib32-mesa",         "32-bit Mesa (Steam/Wine)",                     "open-source", false);
        add("lib32-vulkan-intel", "32-bit ANV Vulkan",                            "open-source", false);
        add("libva-intel-driver", "Legacy VA-API fallback (pre-Broadwell)",       "open-source", false);
        break;

    // ── HD Graphics 2000-6000 (Sandy/Ivy/Haswell/Broadwell) ──────────────────
    case IntelGen::HD_LEGACY:
        add("mesa",               "Mesa OpenGL (i915 kernel driver)",             "open-source", true);
        add("libva-intel-driver", "Intel legacy VA-API (pre-Broadwell)",          "open-source", true);
        add("lib32-mesa",         "32-bit Mesa",                                  "open-source", false);
        add("vulkan-intel",       "ANV Vulkan (Broadwell min; may not apply)",    "open-source", false);
        add("xf86-video-intel",   "Intel DDX (SNA accel, older setups)",          "open-source", false);
        std::cout << C_YELLOW
                  << "  [!] HD 2000-6000: modesetting DDX (default) recommended over\n"
                     "      xf86-video-intel unless you have a specific reason.\n"
                  << C_RESET;
        break;

    // ── GMA (pre-HD, 2004-2008) ───────────────────────────────────────────────
    case IntelGen::GMA:
        add("xf86-video-intel",   "Intel DDX (required for GMA UXA accel)",      "open-source", true);
        add("mesa",               "Mesa (software OpenGL fallback)",              "open-source", true);
        add("xf86-video-vesa",    "VESA DDX last-resort fallback",                "open-source", false);
        std::cout << C_YELLOW
                  << "  [!] Intel GMA era GPU. Very limited support in modern Linux.\n"
                     "      3D acceleration is basic or unavailable.\n"
                  << C_RESET;
        break;

    default:
        add("mesa",               "Mesa OpenGL/Vulkan stack",                    "open-source", true);
        add("vulkan-intel",       "Intel ANV Vulkan",                             "open-source", false);
        add("intel-media-driver", "Intel iHD VA-API",                             "open-source", false);
        break;
    }

    return list;
}

// ── S3 Graphics generation detection ─────────────────────────────────────────
//
// S3 device ID families (vendor 0x5333):
//   0x5631, 0x883d, 0x88xx, 0x8800–0x88ff  ViRGE/Vision/Trio  → xf86-video-s3virge (AUR)
//   0x8a20–0x9102                           Savage series       → xf86-video-savage (AUR)
//   0x8e00–0x90ff                           Chrome/DeltaChrome  → xf86-video-openchrome (AUR)
//
// Note: Chrome 500 series (Chrome 530 GT=0x9060, Chrome 5400E=0x9060,
//       Chrome 540 GTX=0x9045, Chrome 430=0x9043) uses openchrome.

S3Gen DriverManager::detectS3Gen(const GPUInfo& gpu) {
    const std::string& n = gpu.name;
    if (strContains(n, "chrome") || strContains(n, "deltachrome") ||
        strContains(n, "prosavage") || strContains(n, "matrix"))
        return S3Gen::CHROME;
    if (strContains(n, "savage"))
        return S3Gen::SAVAGE;
    if (strContains(n, "virge") || strContains(n, "trio") || strContains(n, "vision"))
        return S3Gen::VIRGE;

    uint32_t id = 0;
    if (gpu.deviceId.empty()) return S3Gen::UNKNOWN_S3;
    try { id = static_cast<uint32_t>(std::stoul(gpu.deviceId, nullptr, 16)); }
    catch (...) { return S3Gen::UNKNOWN_S3; }

    if (id >= 0x8e00 && id <= 0x90ff) return S3Gen::CHROME;
    if (id >= 0x8a20 && id <= 0x9102) return S3Gen::SAVAGE;
    if ((id >= 0x5631 && id <= 0x5632) ||
        (id >= 0x8800 && id <= 0x88ff) ||
        (id >= 0x883d && id <= 0x883f))  return S3Gen::VIRGE;

    return S3Gen::UNKNOWN_S3;
}

static const char* s3GenName(S3Gen g) {
    switch (g) {
        case S3Gen::CHROME:  return "S3 Chrome / DeltaChrome / ProSavage (2002-2008)";
        case S3Gen::SAVAGE:  return "S3 Savage (1998-2002)";
        case S3Gen::VIRGE:   return "S3 ViRGE / Trio / Vision (1995-1999)";
        default:             return "S3 Graphics (unknown sub-family)";
    }
}

std::vector<DriverInfo> DriverManager::buildS3List(const GPUInfo& gpu) {
    std::vector<DriverInfo> list;
    S3Gen gen = detectS3Gen(gpu);

    auto add = [&](const std::string& pkg, const std::string& desc,
                   const std::string& type, bool rec, bool aur = false) {
        DriverInfo d;
        d.packageName = pkg;
        d.description = desc;
        d.type        = type;
        d.installed   = isInstalled(pkg);
        d.recommended = rec;
        d.fromAUR     = aur;
        d.openSource  = true;   // all S3 drivers are open-source
        list.push_back(d);
    };

    std::cout << C_DIM << "  S3 sub-family : " << s3GenName(gen) << C_RESET << "\n";

    switch (gen) {

    // ── Chrome 400/430/440/460/530/540/5400 ──────────────────────────────────
    // xf86-video-openchrome is the only Xorg DDX for Chrome series.
    // No hardware 3D on Linux; mesa software rendering only.
    // Package dropped from official Arch repos → AUR only.
    case S3Gen::CHROME:
        add("xf86-video-openchrome", "OpenChrome DDX — only Xorg driver for Chrome",  "open-source", true,  true);
        add("mesa",                  "Mesa software OpenGL (no hw 3D on Chrome/Linux)","open-source", true,  false);
        add("xf86-video-vesa",       "VESA DDX fallback (display only, no accel)",     "open-source", false, false);
        std::cout << C_YELLOW
                  << "  [!] S3 Chrome series has no hardware 3D acceleration on Linux.\n"
                     "      xf86-video-openchrome provides 2D accel (EXA) only.\n"
                     "      OpenGL is purely software-rendered via Mesa llvmpipe/softpipe.\n"
                     "      No Vulkan support.\n"
                  << C_RESET;
        break;

    // ── Savage 3D / Savage 4 / Savage 2000 / Savage MX/IX ───────────────────
    case S3Gen::SAVAGE:
        add("xf86-video-savage",     "S3 Savage DDX — Xorg driver for Savage cards",  "open-source", true,  true);
        add("mesa",                  "Mesa software OpenGL fallback",                   "open-source", false, false);
        add("xf86-video-vesa",       "VESA DDX fallback",                              "open-source", false, false);
        std::cout << C_YELLOW
                  << "  [!] S3 Savage: basic 3D via DRI1 on older kernels only.\n"
                     "      Modern kernels (>= 5.x) dropped DRI1 support.\n"
                     "      Software OpenGL via Mesa is the practical option.\n"
                  << C_RESET;
        break;

    // ── ViRGE / Trio / Vision (very old, 1995-1998) ──────────────────────────
    case S3Gen::VIRGE:
        add("xf86-video-s3virge",    "S3 ViRGE DDX — Xorg driver for ViRGE/Trio",     "open-source", true,  true);
        add("xf86-video-vesa",       "VESA DDX fallback",                               "open-source", false, false);
        std::cout << C_YELLOW
                  << "  [!] S3 ViRGE/Trio era (pre-2000). No usable 3D on Linux.\n"
                     "      The ViRGE's 3D engine was infamously slow even in 1997.\n"
                     "      Use VESA DDX if openchrome fails to load.\n"
                  << C_RESET;
        break;

    default:
        add("xf86-video-openchrome", "OpenChrome DDX (best guess for S3)",             "open-source", true,  true);
        add("mesa",                  "Mesa software OpenGL",                            "open-source", false, false);
        add("xf86-video-vesa",       "VESA DDX fallback",                              "open-source", false, false);
        break;
    }

    return list;
}

// ── VIA Technologies GPU support ──────────────────────────────────────────────
//
// VIA integrated GPUs (vendor 0x1106) used in VIA chipsets (CLE266, K8M800,
// P4M890, VX800, etc.). These are always integrated/soldered, not discrete.
//
// Driver: xf86-video-openchrome — same project as S3 Chrome (VIA acquired S3).
// Kernel: 'via' DRM module (basic KMS), or just framebuffer.
//
// Known device IDs:
//   0x3122  CLE266 UniChrome (VT8623)
//   0x3108  VT8025 / CN333/CN400 UniChrome Pro
//   0x3118  K8M800 / PM800 UniChrome Pro
//   0x3230  K8M890 UniChrome Pro II
//   0x3343  P4M890 Chrome 9
//   0x3371  P4M900 / VN896 Chrome 9
//   0x1122  VX800/VX820 Chrome 9 HC3
//   0x5122  VX855/VX875 Chrome 9 HCM

std::vector<DriverInfo> DriverManager::buildViaList(const GPUInfo& gpu) {
    std::vector<DriverInfo> list;

    auto add = [&](const std::string& pkg, const std::string& desc,
                   const std::string& type, bool rec, bool aur = false) {
        DriverInfo d;
        d.packageName = pkg;
        d.description = desc;
        d.type        = type;
        d.installed   = isInstalled(pkg);
        d.recommended = rec;
        d.fromAUR     = aur;
        d.openSource  = true;
        list.push_back(d);
    };

    // Identify UniChrome vs Chrome9 from name/ID for the info message
    bool isChrome9 = strContains(gpu.name, "chrome 9") ||
                     strContains(gpu.name, "vx800")     ||
                     strContains(gpu.name, "vx820")     ||
                     strContains(gpu.name, "vx855")     ||
                     strContains(gpu.name, "p4m8")      ||
                     strContains(gpu.name, "k8m89");

    std::cout << C_DIM << "  VIA GPU : "
              << (isChrome9 ? "Chrome 9 (VX800/VX820/P4M890/K8M890 era)"
                            : "UniChrome / UniChrome Pro (CLE266/K8M800 era)")
              << C_RESET << "\n";

    add("xf86-video-openchrome", "OpenChrome DDX — the only Xorg driver for VIA GPUs", "open-source", true,  true);
    add("mesa",                  "Mesa software OpenGL (no hardware 3D on VIA/Linux)",  "open-source", true,  false);
    add("xf86-video-vesa",       "VESA DDX fallback (display only)",                    "open-source", false, false);

    std::cout << C_YELLOW
              << "  [!] VIA integrated GPUs share the OpenChrome driver with S3 Chrome\n"
                 "      (VIA acquired S3 Graphics in 1999).\n"
                 "      No hardware 3D acceleration on Linux. No Vulkan. No VA-API.\n"
                 "      2D acceleration (EXA) only via xf86-video-openchrome.\n"
                 "      xf86-video-openchrome is in the AUR — needs paru/yay.\n"
              << C_RESET;

    return list;
}

// ── NVIDIA generation detection ───────────────────────────────────────────────
//
// Primary method: PCI device ID ranges (most reliable).
// Fallback: GPU name string matching for edge cases.
//
// Device ID ranges (hex, lower bound inclusive):
//   0x0000 – 0x017f  NV1/TNT/TNT2/GeForce2  → TOO_OLD
//   0x0180 – 0x018f  GeForce4 MX/Ti          → TOO_OLD
//   0x0190 – 0x07ff  G7x/G8x/G9x (8xxx/9xxx) → TESLA_G  (340xx AUR)
//   0x0800 – 0x0dff  GT2xx/GT3xx             → TESLA_G  (340xx AUR)
//   0x0dc0 – 0x1057  GF1xx Fermi (400/500)   → FERMI    (390xx AUR)
//   0x1058 – 0x11ff  GF1xx late Fermi         → FERMI
//   0x0fc0 – 0x12ff  GK1xx Kepler (600/700)  → KEPLER   (470xx)
//   0x1300 – 0x13bf  GK2xx Kepler            → KEPLER
//   0x13c0 – 0x17ff  GM1xx/GM2xx Maxwell      → MAXWELL  (current)
//   0x1b00 – 0x1dff  GP1xx Pascal (10xx)      → PASCAL
//   0x1d80 – 0x1dbf  GV1xx Volta              → VOLTA
//   0x1e00 – 0x1fff  TU1xx Turing (20xx/16xx) → TURING
//   0x2200 – 0x25ff  GA1xx Ampere (30xx)      → AMPERE
//   0x2600 – 0x2bff  AD1xx Ada (40xx)         → ADA
//   0x2c00 +         GB2xx Blackwell (50xx)   → BLACKWELL
//
// NOTE: some ranges overlap (Fermi/Kepler around 0x0dc0-0x12ff).
// We resolve overlaps by also checking the GPU name when ambiguous.

NvidiaGen DriverManager::detectNvidiaGen(const GPUInfo& gpu) {
    uint32_t id = 0;
    bool hasId = false;
    if (!gpu.deviceId.empty()) {
        try {
            id = static_cast<uint32_t>(std::stoul(gpu.deviceId, nullptr, 16));
            hasId = true;
        } catch (...) {}
    }

    // ── device ID ranges (authoritative — checked BEFORE name matching) ───────
    // Marketing names get rebadged across generations (e.g. "GT 630" ships as
    // both Fermi GF108 [10de:0f00] and Kepler GK208 [10de:1284] depending on
    // OEM/region); a PCI device ID is unambiguous, so it must win over any
    // name-based guess whenever we actually have one.
    if (hasId) {
        if (id >= 0x2c00)                   return NvidiaGen::BLACKWELL;
        if (id >= 0x2600 && id <= 0x2bff)   return NvidiaGen::ADA;
        if (id >= 0x2200 && id <= 0x25ff)   return NvidiaGen::AMPERE;
        if (id >= 0x1e00 && id <= 0x1fff)   return NvidiaGen::TURING;
        if (id >= 0x1d80 && id <= 0x1dbf)   return NvidiaGen::VOLTA;
        if (id >= 0x1b00 && id <= 0x1d7f)   return NvidiaGen::PASCAL;
        if (id >= 0x13c0 && id <= 0x17ff)   return NvidiaGen::MAXWELL;
        // Kepler: 0x0fc0–0x13bf (some overlap with late Fermi 0x0dc0–0x0fbf)
        if (id >= 0x0fc0 && id <= 0x13bf)   return NvidiaGen::KEPLER;
        // Fermi: 0x0dc0–0x0fbf (includes GF108 "GT 630" = 0x0f00)
        if (id >= 0x0dc0 && id <= 0x0fbf)   return NvidiaGen::FERMI;
        // Tesla G8x/G9x/GT2xx: 0x0190–0x0dbf
        if (id >= 0x0190 && id <= 0x0dbf)   return NvidiaGen::TESLA_G;
        // Anything older: NV30/NV40/GeForce 6/7
        if (id < 0x0190)                     return NvidiaGen::TOO_OLD;
        // id didn't land in any known range — fall through to name matching below
    }

    // ── name-based fallback (used when there's no PCI ID — e.g. --simulate,
    // or an ID outside every known range) ──────────────────────────────────
    const std::string& n = gpu.name;

    // Datacenter/compute cards (Tesla-branded T4/P100/... and A100/H100/B200
    // generation names) — same driver rules as their consumer-GPU counterparts,
    // just checked first since they don't carry a "GTX"/"RTX" substring.
    if (strContains(n, "B100") || strContains(n, "B200") || strContains(n, "GB200"))
        return NvidiaGen::BLACKWELL;
    if (strContains(n, "H100") || strContains(n, "H200") ||
        strContains(n, "L40")  || strContains(n, "L4"))
        return NvidiaGen::ADA;
    if (strContains(n, "A100") || strContains(n, "A30") || strContains(n, "A10") ||
        strContains(n, "A40")  || strContains(n, "A16"))
        return NvidiaGen::AMPERE;
    if (strContains(n, "T4"))
        return NvidiaGen::TURING;
    if (strContains(n, "V100"))
        return NvidiaGen::VOLTA;
    if (strContains(n, "P100") || strContains(n, "P40") || strContains(n, "P4"))
        return NvidiaGen::PASCAL;
    if (strContains(n, "M60") || strContains(n, "M40") || strContains(n, "M4"))
        return NvidiaGen::MAXWELL;
    if (strContains(n, "K80") || strContains(n, "K40") ||
        strContains(n, "K20") || strContains(n, "K10"))
        return NvidiaGen::KEPLER;

    // Workstation "Quadro RTX ####" (Turing, 2018) vs "RTX #### Ada Generation"
    // (Ada, 2023) — both use plain "RTX 4000"/"RTX 5000"/etc. numbering, so the
    // generic "RTX 40"/"RTX 30"/... consumer checks below would misread a
    // Quadro RTX 4000 (Turing) as an RTX 40xx (Ada) card. Disambiguate first.
    if (strContains(n, "ada generation"))  return NvidiaGen::ADA;
    if (strContains(n, "quadro rtx"))      return NvidiaGen::TURING;
    // Dual-GPU Kepler flagship — doesn't carry a "GTX 7"/"GTX 6" substring.
    if (strContains(n, "titan z"))         return NvidiaGen::KEPLER;

    if (strContains(n, "RTX 50") || strContains(n, "RTX50"))  return NvidiaGen::BLACKWELL;
    if (strContains(n, "RTX 40") || strContains(n, "RTX40"))  return NvidiaGen::ADA;
    if (strContains(n, "RTX 30") || strContains(n, "RTX30"))  return NvidiaGen::AMPERE;
    if (strContains(n, "RTX 20") || strContains(n, "RTX20"))  return NvidiaGen::TURING;
    if (strContains(n, "GTX 16") || strContains(n, "GTX16"))  return NvidiaGen::TURING;
    if (strContains(n, "GTX 10") || strContains(n, "GTX10"))  return NvidiaGen::PASCAL;
    if (strContains(n, "Titan V"))                             return NvidiaGen::VOLTA;

    // Rebadged low/mid-range parts that NVIDIA shipped under the SAME marketing
    // name on both Fermi (GF108-derived) and Kepler (GK208-derived) silicon —
    // e.g. GT 630 exists as both 10de:0f00 (Fermi) and 10de:1284 (Kepler). Name
    // alone can't tell them apart; a real PCI device ID always can (see the
    // device-ID block above, which runs BEFORE this fallback and handles real
    // hardware correctly). Default to Fermi here only as a same-either-way
    // placeholder: AUTO recommends nouveau for both generations regardless, so
    // this only matters for --prefer proprietary, where the AUR branch guess
    // (390xx vs 470xx) may be wrong for the actual chip.
    static const char* ambiguousRebadges[] = {
        "gt 610", "gt 620", "gt 630", "gt 635", "gt 640", "gt 730"
    };
    for (const char* model : ambiguousRebadges) {
        if (strContains(n, model)) {
            std::cerr << "\033[33m[hyprvisor] \"" << n << "\" was sold as both Fermi "
                         "and Kepler silicon under this exact name — the name alone "
                         "can't tell them apart. Defaulting to Fermi/nouveau; if you "
                         "need the proprietary legacy branch, check the real PCI "
                         "device ID (`lspci -nn`) instead of trusting this guess.\n"
                         "\033[0m";
            return NvidiaGen::FERMI;
        }
    }
    // GTX 900 series = Maxwell (GTX 950/960/970/980)
    if (strContains(n, "GTX 9") && !strContains(n, "9600") &&
        !strContains(n, "9800") && !strContains(n, "9500") &&
        !strContains(n, "9400") && !strContains(n, "9300") &&
        !strContains(n, "9200") && !strContains(n, "9100"))    return NvidiaGen::MAXWELL;
    // GTX 750 / GTX 745 = Maxwell; rest of 700 series = Kepler
    if (strContains(n, "GTX 750") || strContains(n, "GTX 745")) return NvidiaGen::MAXWELL;
    if (strContains(n, "GTX 7") || strContains(n, "GTX 6"))   return NvidiaGen::KEPLER;
    // Fermi: GTX 400 / GTX 500 series
    if (strContains(n, "GTX 5") || strContains(n, "GTX 4") ||
        strContains(n, "GTS 4") || strContains(n, "GT 5"))    return NvidiaGen::FERMI;
    // Tesla G era: GeForce 8xxx, 9xxx, 100, 200, 300 series
    if (strContains(n, "geforce 9") || strContains(n, "geforce 8") ||
        strContains(n, "geforce 3") || strContains(n, "geforce 2") ||
        strContains(n, "geforce 1") || strContains(n, "gt 2") ||
        strContains(n, "gt 3")      || strContains(n, "9600") ||
        strContains(n, "9800")      || strContains(n, "8800") ||
        strContains(n, "8600")      || strContains(n, "8500"))
        return NvidiaGen::TESLA_G;

    return NvidiaGen::UNKNOWN_GEN;
}

static const char* nvidiaGenName(NvidiaGen g) {
    switch (g) {
        case NvidiaGen::BLACKWELL: return "Blackwell (RTX 50xx)";
        case NvidiaGen::ADA:       return "Ada Lovelace (RTX 40xx)";
        case NvidiaGen::AMPERE:    return "Ampere (RTX 30xx)";
        case NvidiaGen::TURING:    return "Turing (RTX 20xx / GTX 16xx)";
        case NvidiaGen::VOLTA:     return "Volta (Titan V)";
        case NvidiaGen::PASCAL:    return "Pascal (GTX 10xx)";
        case NvidiaGen::MAXWELL:   return "Maxwell (GTX 750 / GTX 9xx)";
        case NvidiaGen::KEPLER:    return "Kepler (GTX 600 / GTX 700)";
        case NvidiaGen::FERMI:     return "Fermi (GTX 400 / GTX 500)";
        case NvidiaGen::TESLA_G:   return "Tesla G (GeForce 8xxx / 9xxx / GT2xx)";
        case NvidiaGen::TOO_OLD:   return "Pre-Tesla (GeForce 6/7 and older)";
        default:                   return "Unknown generation";
    }
}

// ── NVIDIA driver list builder ────────────────────────────────────────────────
//
// Driver taxonomy for NVIDIA:
//   proprietary   – full NVIDIA blob (best perf, CUDA, DLSS)
//   open-kernel   – nvidia-open: open kernel module, proprietary userspace (Turing+)
//   open-source   – xf86-video-nouveau + mesa: 100% free, no blobs, limited 3D
//
// --prefer proprietary  → default; best performance
// --prefer open         → Turing+: nvidia-open (open kernel, still needs nvidia-utils)
//                         Maxwell/Pascal/Volta: nouveau only (nvidia-open unsupported)
//                         Kepler/Fermi/Tesla: nouveau only
// AUTO                  → same as proprietary for NVIDIA

std::vector<DriverInfo> DriverManager::buildNvidiaList(const GPUInfo& gpu) {
    std::vector<DriverInfo> list;
    NvidiaGen gen = detectNvidiaGen(gpu);

    // Three-way preference: AUTO auto-selects the best default per generation
    // (open kernel module for Turing+, nouveau for pre-Kepler-legacy tiers);
    // an explicit --prefer only overrides that default when given.
    bool explicitProprietary = (pref_ == DriverPreference::PROPRIETARY);
    bool explicitOpen        = (pref_ == DriverPreference::OPEN_SOURCE);

    auto add = [&](const std::string& pkg, const std::string& desc,
                   const std::string& type, bool rec,
                   bool aur = false, bool open = false) {
        DriverInfo d;
        d.packageName = pkg;
        d.description = desc;
        d.type        = type;
        d.installed   = isInstalled(pkg);
        d.recommended = rec;
        d.fromAUR     = aur;
        d.openSource  = open;
        list.push_back(d);
    };

    std::cout << C_DIM << "  NVIDIA architecture : " << nvidiaGenName(gen) << "\n" << C_RESET;

    switch (gen) {

    // ── Blackwell / Ada / Ampere / Turing — auto-selects nvidia-open-dkms ─────
    // Default (AUTO): open kernel module (GPL-licensed, still needs nvidia-utils
    // for the proprietary userspace/OpenGL/Vulkan bits, but no closed kernel blob).
    // --prefer proprietary switches to the full closed driver.
    case NvidiaGen::BLACKWELL:
    case NvidiaGen::ADA:
    case NvidiaGen::AMPERE:
    case NvidiaGen::TURING: {
        bool propRec = explicitProprietary;
        bool openRec = !explicitProprietary;

        // Some distros (e.g. pearOS on its CachyOS-based kernel) publish a
        // precompiled "<kernel-package>-nvidia-open" module built for that
        // EXACT kernel — no DKMS build, no kernel headers, no compile time.
        // Prefer it over the generic DKMS package whenever it exists.
        std::string kernelPkg = detectRunningKernelPackage();
        std::string prebuiltOpen = kernelPkg.empty() ? "" : (kernelPkg + "-nvidia-open");
        bool hasPrebuiltOpen = !prebuiltOpen.empty() && isAvailable(prebuiltOpen);

        std::cout << C_DIM << "  Driver preference   : "
                  << (propRec ? "proprietary"
                     : hasPrebuiltOpen ? ("open kernel module (" + prebuiltOpen + ", prebuilt for this kernel)")
                     : "open kernel module (nvidia-open-dkms, default)")
                  << "\n" << C_RESET;

        if (hasPrebuiltOpen)
            add(prebuiltOpen, "NVIDIA open kernel module — precompiled for the running kernel "
                               "(" + kernelPkg + "), no DKMS build/headers needed",
                "open-kernel", openRec, false, false);
        // open kernel module — requires nvidia-utils for userspace (not 100% open)
        add("nvidia-open-dkms",   "NVIDIA open kernel module DKMS (GPL kernel"
                                   + std::string(hasPrebuiltOpen ? ", fallback if the prebuilt module doesn't match your kernel" : ", default")
                                   + ")", "open-kernel", openRec && !hasPrebuiltOpen, false, false);
        add("nvidia-open",        "NVIDIA open kernel module (per-kernel build variant)","open-kernel", false,   false, false);
        add("nvidia-utils",       "NVIDIA userspace utilities & OpenGL/Vulkan (needed either way)", "proprietary", true, false, false);
        add("nvidia",             "NVIDIA full proprietary driver",                   "proprietary", propRec, false, false);
        add("nvidia-dkms",        "NVIDIA proprietary DKMS (custom kernels)",         "proprietary", false,   false, false);
        add("lib32-nvidia-utils", "32-bit NVIDIA libs (Steam/Wine)",                  "proprietary", false,   false, false);
        // fully open — no blobs, limited 3D (reclocking/power mgmt limited on newer GPUs)
        add("xf86-video-nouveau", "Nouveau — 100% open-source, limited 3D accel",    "open-source", false,   false, true);
        add("mesa",               "Mesa (required by nouveau for OpenGL/Vulkan)",      "open-source", false,   false, true);
        break;
    }

    // ── Volta / Pascal / Maxwell — dropped by current driver as of 590.xx ─────
    // Nvidia removed Maxwell/Pascal/Volta support starting with the 590 series.
    // Current "nvidia"/"nvidia-utils" packages track that series, so installing
    // them here leaves the kernel module unable to bind (fallback 800x600).
    // Last driver branch with support is 580.xx → nvidia-580xx-dkms/-utils.
    case NvidiaGen::VOLTA:
    case NvidiaGen::PASCAL:
    case NvidiaGen::MAXWELL: {
        bool propRec = !explicitOpen;
        bool nv2Rec  =  explicitOpen;   // nouveau is the only fully open option here

        std::cout << C_DIM << "  Driver preference   : "
                  << (propRec ? "proprietary (nvidia-580xx-dkms, default)" : "open-source (nouveau)")
                  << "\n" << C_RESET;

        add("nvidia-580xx-dkms",  "NVIDIA legacy 580xx — last branch with Maxwell/Pascal/Volta support", "aur-legacy", propRec, true, false);
        add("nvidia-580xx-utils", "NVIDIA 580xx userspace utilities & OpenGL",                            "aur-legacy", propRec, true, false);
        add("lib32-nvidia-580xx-utils", "32-bit NVIDIA 580xx libs (Steam/Wine)",                          "aur-legacy", false,   true, false);
        add("xf86-video-nouveau", "Nouveau — 100% open-source (good Pascal/Maxwell)", "open-source", nv2Rec,  false, true);
        add("mesa",               "Mesa (required by nouveau for OpenGL)",             "open-source", nv2Rec,  false, true);
        add("nvidia",             "CURRENT nvidia (590+) — NOT compatible, Maxwell/Pascal/Volta support removed", "proprietary", false, false, false);
        std::cout << C_YELLOW
                  << "  [!] Nvidia dropped Maxwell/Pascal/Volta support in driver 590.\n"
                     "      Use nvidia-580xx-dkms, not 'nvidia' (will fail to load → 800x600 fallback).\n"
                  << C_RESET;
        if (explicitOpen)
            std::cout << C_YELLOW
                      << "  [!] nvidia-open unsupported on this GPU (requires Turing+). "
                         "Using nouveau.\n" << C_RESET;
        break;
    }

    // ── Kepler / Fermi / Tesla G — auto-selects nouveau + mesa ────────────────
    // These legacy proprietary branches (470xx/390xx/340xx) are AUR-only,
    // unmaintained by Nvidia, and increasingly fragile to build against current
    // kernels. Default (AUTO): leave nouveau + mesa active. Only an explicit
    // --prefer proprietary pulls in the AUR legacy DKMS package.
    case NvidiaGen::KEPLER: {
        bool propRec = explicitProprietary;
        bool nv2Rec  = !explicitProprietary;

        std::cout << C_DIM << "  Driver preference   : "
                  << (propRec ? "proprietary (nvidia-470xx-dkms, AUR)" : "open-source (nouveau, default)")
                  << "\n" << C_RESET;

        add("xf86-video-nouveau", "Nouveau — excellent Kepler support",               "open-source", nv2Rec,  false, true);
        add("mesa",               "Mesa (required by nouveau)",                        "open-source", nv2Rec,  false, true);
        add("nvidia-470xx-dkms",  "NVIDIA legacy 470xx — last branch for Kepler",    "aur-legacy",  propRec, true, false);
        add("nvidia-470xx-utils", "NVIDIA 470xx userspace utilities",                 "aur-legacy",  propRec, true, false);
        add("nvidia",             "CURRENT nvidia — NOT compatible with Kepler",       "proprietary", false,   false, false);
        break;
    }

    // ── Fermi → nvidia-390xx (AUR, only with --prefer proprietary) ────────────
    case NvidiaGen::FERMI: {
        bool propRec = explicitProprietary;
        bool nv2Rec  = !explicitProprietary;

        std::cout << C_DIM << "  Driver preference   : "
                  << (propRec ? "proprietary (nvidia-390xx-dkms, AUR)" : "open-source (nouveau, default)")
                  << "\n" << C_RESET;

        add("xf86-video-nouveau", "Nouveau — decent Fermi support",                   "open-source", nv2Rec,  false, true);
        add("mesa",               "Mesa (required by nouveau)",                        "open-source", nv2Rec,  false, true);
        add("nvidia-390xx-dkms",  "NVIDIA legacy 390xx — last branch for Fermi",     "aur-legacy",  propRec, true,  false);
        add("nvidia-390xx-utils", "NVIDIA 390xx userspace utilities",                 "aur-legacy",  propRec, true,  false);
        add("nvidia",             "CURRENT nvidia — NOT compatible with Fermi",        "proprietary", false,   false, false);
        break;
    }

    // ── Tesla G 8xxx/9xxx → nvidia-340xx (AUR, only with --prefer proprietary) ─
    case NvidiaGen::TESLA_G: {
        bool propRec = explicitProprietary;
        bool nv2Rec  = !explicitProprietary;

        std::cout << C_DIM << "  Driver preference   : "
                  << (propRec ? "proprietary (nvidia-340xx-dkms, AUR)" : "open-source (nouveau, default)")
                  << "\n" << C_RESET;

        add("xf86-video-nouveau", "Nouveau — recommended for this era",               "open-source", nv2Rec,  false, true);
        add("mesa",               "Mesa (required by nouveau)",                        "open-source", nv2Rec,  false, true);
        add("nvidia-340xx-dkms",  "NVIDIA legacy 340xx — last branch for 8/9xxx",    "aur-legacy",  propRec, true,  false);
        add("nvidia-340xx-utils", "NVIDIA 340xx userspace utilities",                 "aur-legacy",  propRec, true,  false);
        add("nvidia",             "CURRENT nvidia — NOT compatible with 8/9xxx",       "proprietary", false,   false, false);
        break;
    }

    // ── GeForce 6/7 — no proprietary option ───────────────────────────────────
    case NvidiaGen::TOO_OLD:
        add("xf86-video-nouveau", "Nouveau — only driver option for this GPU",        "open-source", true,   false, true);
        add("mesa",               "Mesa software rendering",                           "open-source", true,   false, true);
        add("xf86-video-vesa",    "VESA generic DDX fallback",                         "open-source", false,  false, true);
        break;

    default:
        add("nvidia",             "NVIDIA proprietary (generation unknown)",           "proprietary", !explicitOpen, false, false);
        add("nvidia-utils",       "NVIDIA userspace utilities",                        "proprietary", !explicitOpen, false, false);
        add("xf86-video-nouveau", "Nouveau open-source fallback",                      "open-source",  explicitOpen, false, true);
        add("mesa",               "Mesa (required by nouveau)",                        "open-source",  explicitOpen, false, true);
        break;
    }

    return list;
}

// ── generic driver database ───────────────────────────────────────────────────

std::string DriverManager::detectKernelHeadersPackage() {
    std::string kernelPkg = detectRunningKernelPackage();
    return kernelPkg.empty() ? "" : (kernelPkg + "-headers");
}

// Version of `pkg` — installed version if it's installed, else whatever the
// repo currently has (so we can catch a mismatch BEFORE downloading/building
// anything, not just after a failed build).
static std::string queryPackageVersion(const std::string& pkg) {
    std::string out = execCmd("pacman -Qi " + pkg + " 2>/dev/null");
    if (out.empty()) out = execCmd("pacman -Si " + pkg + " 2>/dev/null");
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.rfind("Version", 0) != 0) continue;
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string v = line.substr(colon + 1);
        size_t start = v.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        size_t end = v.find_last_not_of(" \t\r\n");
        return v.substr(start, end - start + 1);
    }
    return "";
}

std::vector<DriverInfo> DriverManager::buildList(const GPUInfo& gpu) {
    std::vector<DriverInfo> list;
    if (gpu.vendor == GPUVendor::NVIDIA) list = buildNvidiaList(gpu);
    else if (gpu.vendor == GPUVendor::AMD)    list = buildAmdList(gpu);
    else if (gpu.vendor == GPUVendor::INTEL)  list = buildIntelList(gpu);
    else if (gpu.vendor == GPUVendor::S3)     list = buildS3List(gpu);
    else if (gpu.vendor == GPUVendor::VIA)    list = buildViaList(gpu);

    if (!list.empty()) {
        bool needsDkms = false;
        for (const auto& d : list)
            if (d.recommended && d.packageName.find("dkms") != std::string::npos)
                needsDkms = true;

        if (needsDkms) {
            std::string headersPkg = detectKernelHeadersPackage();
            if (!headersPkg.empty()) {
                std::string kernelPkg = detectRunningKernelPackage();
                std::string kernelVer  = queryPackageVersion(kernelPkg);
                std::string headersVer = queryPackageVersion(headersPkg);

                DriverInfo h;
                h.packageName = headersPkg;
                h.description = "Kernel headers for the running kernel — required to build the DKMS module";
                h.type        = "kernel-headers";
                h.installed   = isInstalled(headersPkg);
                h.recommended = true;
                h.fromAUR     = false;
                h.openSource  = true;

                if (!kernelVer.empty() && !headersVer.empty() && kernelVer != headersVer) {
                    h.description += " [VERSION MISMATCH]";
                    h.versionMismatch = true;
                    std::cerr << C_RED << C_BOLD
                              << "\n  [!] Kernel/headers version mismatch — the DKMS build WILL FAIL:\n"
                              << C_RESET << C_RED
                              << "      " << kernelPkg  << " (kernel)  = " << kernelVer  << "\n"
                              << "      " << headersPkg << " (headers) = " << headersVer << "\n"
                              << "      These must be the exact same version. Rebuild/republish "
                                 "them together in your repo, or run `sudo pacman -Syu` to bring "
                                 "both up to whatever the repo currently has in sync.\n"
                              << C_RESET;
                }

                list.push_back(h);
            } else {
                std::cerr << C_YELLOW
                          << "  [!] Could not auto-detect the kernel headers package for "
                             "the running kernel — install it manually (e.g. linux-headers, "
                             "linux-zen-headers, ...) before the DKMS build runs.\n"
                          << C_RESET;
            }
        }
        return list;
    }

    auto add = [&](const std::string& pkg, const std::string& desc,
                   const std::string& type, bool rec,
                   bool aur = false, bool open = true) {
        DriverInfo d;
        d.packageName = pkg;
        d.description = desc;
        d.type        = type;
        d.installed   = isInstalled(pkg);
        d.recommended = rec;
        d.fromAUR     = aur;
        d.openSource  = open;
        list.push_back(d);
    };

    switch (gpu.vendor) {

    case GPUVendor::VIRTUALBOX:
        add("virtualbox-guest-utils",        "VirtualBox guest utilities & drivers","virtual", true);
        add("virtualbox-guest-modules-arch", "VirtualBox kernel modules",           "virtual", true);
        add("mesa",                          "Mesa (software rendering fallback)",  "open-source",false);
        break;

    case GPUVendor::VMWARE:
        add("open-vm-tools",     "VMware open tools (recommended)",          "virtual",     true);
        add("xf86-video-vmware", "VMware SVGA DDX Xorg driver",             "virtual",     true);
        add("mesa",              "Mesa (software rendering)",                "open-source", false);
        add("gtkmm3",            "GTK GUI for vmware-user-suid-wrapper",     "virtual",     false);
        break;

    case GPUVendor::QEMU:
        add("mesa",              "Mesa (virtio-gpu / QXL rendering)",        "open-source", true);
        add("spice-vdagent",     "SPICE guest agent (clipboard, resize)",    "virtual",     true);
        add("xf86-video-qxl",   "QXL DDX Xorg driver",                      "virtual",     false);
        add("xf86-video-fbdev", "Framebuffer DDX fallback",                  "open-source", false);
        break;

    // ── Apple Silicon (M1/M2/M3/M4) via Asahi Linux ───────────────────────────
    // The Apple GPU chip appears as vendor 0x106b on the Asahi DRM subsystem.
    // The driver stack is "asahi" (kernel) + Mesa Asahi Gallium3D (userspace).
    // Users need the Asahi Linux kernel and optionally the asahi-edge Mesa for
    // cutting-edge GPU support. Official Arch packages land in [extra]/[community];
    // bleeding-edge ones come from the Asahi Linux pacman repo.
    case GPUVendor::APPLE:
        add("mesa",              "Mesa with Asahi Gallium3D (OpenGL/Vulkan)",  "open-source", true);
        add("linux-asahi",       "Asahi Linux kernel (Apple Silicon support)", "open-source", true,  true);
        add("asahi-scripts",     "Apple Silicon boot/firmware helper scripts", "open-source", true,  true);
        add("mesa-asahi-edge",   "Mesa Asahi edge (bleeding-edge GPU perf)",   "open-source", false, true);
        add("xf86-video-fbdev",  "Framebuffer DDX fallback (Xorg)",            "open-source", false);
        std::cout << "\033[33m"
                  << "  [!] Apple Silicon GPU detected.\n"
                     "      Recommended: Asahi Linux (https://asahilinux.org/)\n"
                     "      Add the Asahi pacman repo before installing:\n"
                     "        [asahi] Server = https://cdn.asahilinux.org/$arch/$repo\n"
                  << "\033[0m";
        break;

    // ── Unknown / obscure GPU ─────────────────────────────────────────────────
    // The vendor was not NVIDIA/AMD/Intel/Apple/VM. We resolved the vendor name
    // from pci.ids and show it. Generic Mesa + xf86 fallbacks are suggested.
    // Note: "Zotac", "ASUS", "MSI" etc. are BOARD makers — the PCI vendor ID
    // always belongs to the chip (NVIDIA/AMD/Intel), so those are never "unknown".
    default:
        std::cout << "\033[33m"
                  << "  [!] GPU vendor not recognized in driver database.\n"
                     "      Generic/fallback drivers suggested below.\n"
                     "      Check https://wiki.archlinux.org/title/Xorg#Driver_installation\n"
                     "      for manual driver selection.\n"
                  << "\033[0m";
        add("mesa",              "Mesa generic software OpenGL/Vulkan",      "open-source", true);
        add("xf86-video-fbdev", "Framebuffer DDX — works on almost any GPU", "open-source", true);
        add("xf86-video-vesa",  "VESA DDX — universal last-resort fallback", "open-source", false);
        break;
    }

    return list;
}

// ── pretty-print ──────────────────────────────────────────────────────────────

void DriverManager::printDriverTable(const GPUInfo& gpu,
                                     const std::vector<DriverInfo>& drivers) {
    std::cout << "\n" << C_BOLD << "GPU: " << gpu.name << C_RESET;
    if (gpu.isVirtual)
        std::cout << C_YELLOW << " [VIRTUAL]" << C_RESET;
    std::cout << "\n";
    std::cout << C_DIM
              << "  PCI: " << gpu.pciAddr
              << "  Vendor ID: " << gpu.vendorId
              << "  Device ID: " << gpu.deviceId
              << C_RESET << "\n\n";

    printf("  %-36s %-12s %-7s %-13s %s\n",
           "Package", "Type", "Blobs", "Status", "Description");
    printf("  %s\n", std::string(100, '-').c_str());

    for (const auto& d : drivers) {
        const char* statusColor;
        const char* statusStr;
        if (d.installed) {
            statusStr   = "installed";
            statusColor = isIncompatibleInstalled(d) ? C_RED : C_GREEN;
        } else {
            statusStr   = "not installed";
            statusColor = C_DIM;
        }

        // Blobs column: shows whether driver contains proprietary blobs
        const char* blobStr;
        const char* blobColor;
        if (d.openSource) {
            blobStr   = "none";
            blobColor = C_GREEN;
        } else if (d.type == "open-kernel") {
            blobStr   = "utils";   // kernel open, userspace proprietary
            blobColor = C_YELLOW;
        } else {
            blobStr   = "yes";
            blobColor = C_RED;
        }

        std::string pkg = d.packageName;
        if (d.recommended) pkg += " *";
        if (d.fromAUR)     pkg += " [AUR]";

        const char* pkgColor = d.recommended ? C_CYAN
                             : (d.fromAUR    ? C_MAGENTA : "");

        printf("  %s%-36s%s %-12s %s%-7s%s %s%-13s%s %s\n",
               pkgColor, pkg.c_str(), C_RESET,
               d.type.c_str(),
               blobColor, blobStr, C_RESET,
               statusColor, statusStr, C_RESET,
               d.description.c_str());
    }

    std::cout << "\n"
              << "  " C_CYAN "* " C_RESET "= recommended   "
              << C_MAGENTA "[AUR]" C_RESET " = AUR helper (paru/yay)   "
              << "Blobs: " C_GREEN "none" C_RESET "=100% open  "
              << C_YELLOW "utils" C_RESET "=open kernel+prop userspace  "
              << C_RED "yes" C_RESET "=proprietary\n";

    for (const auto& d : drivers) {
        if (isIncompatibleInstalled(d)) {
            std::cout << C_RED << C_BOLD
                      << "\n  [!] Incompatible driver installed: " << d.packageName
                      << C_RESET << C_RED << " — " << d.description
                      << "\n      Remove it: sudo pacman -R " << d.packageName
                      << C_RESET << "\n";
        }
    }
}

// ── JSON output (machine-readable, for GUIs/scripts) ──────────────────────────

static std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            default:   out += c;
        }
    }
    return out;
}

void DriverManager::listDriversJson() {
    // buildList() prints human-readable diagnostics (architecture, warnings)
    // straight to std::cout; redirect those to a throwaway buffer so stdout
    // stays pure JSON.
    std::ostringstream discard;
    std::streambuf* realCout = std::cout.rdbuf();

    std::set<GPUVendor> presentVendors;
    for (const auto& g : sysInfo_.gpus) presentVendors.insert(g.vendor);

    std::cout << "{\"vmName\":\"" << jsonEscape(sysInfo_.vmName) << "\",\"gpus\":[";
    bool firstGpu = true;
    for (const auto& gpu : sysInfo_.gpus) {
        if (!firstGpu) std::cout << ",";
        firstGpu = false;

        std::cout << "{"
                   << "\"name\":\""       << jsonEscape(gpu.name)       << "\","
                   << "\"vendorName\":\"" << jsonEscape(gpu.vendorName) << "\","
                   << "\"pciAddr\":\""    << jsonEscape(gpu.pciAddr)    << "\","
                   << "\"vendorId\":\""   << jsonEscape(gpu.vendorId)   << "\","
                   << "\"deviceId\":\""   << jsonEscape(gpu.deviceId)   << "\","
                   << "\"isVirtual\":"    << (gpu.isVirtual ? "true" : "false") << ","
                   << "\"drivers\":[";

        std::cout.rdbuf(discard.rdbuf());
        auto drivers = buildList(gpu);
        std::cout.rdbuf(realCout);

        bool firstDrv = true;
        for (const auto& d : drivers) {
            if (!firstDrv) std::cout << ",";
            firstDrv = false;
            std::cout << "{"
                       << "\"package\":\""     << jsonEscape(d.packageName) << "\","
                       << "\"description\":\"" << jsonEscape(d.description) << "\","
                       << "\"type\":\""        << jsonEscape(d.type)        << "\","
                       << "\"installed\":"     << (d.installed   ? "true" : "false") << ","
                       << "\"recommended\":"   << (d.recommended ? "true" : "false") << ","
                       << "\"fromAUR\":"       << (d.fromAUR     ? "true" : "false") << ","
                       << "\"openSource\":"    << (d.openSource  ? "true" : "false") << ","
                       << "\"incompatible\":"  << (isIncompatibleInstalled(d) ? "true" : "false") << ","
                       << "\"versionMismatch\":" << (d.versionMismatch ? "true" : "false")
                       << "}";
        }
        std::cout << "],\"orphanedDrivers\":[";

        std::cout.rdbuf(discard.rdbuf());
        auto orphans = findOrphanedDrivers(gpu, drivers, presentVendors);
        std::cout.rdbuf(realCout);

        bool firstOrphan = true;
        for (const auto& o : orphans) {
            if (!firstOrphan) std::cout << ",";
            firstOrphan = false;
            std::cout << "{\"package\":\"" << jsonEscape(o.first) << "\","
                       << "\"reason\":\""   << jsonEscape(o.second) << "\"}";
        }
        std::cout << "]}";
    }
    std::cout << "]}\n";
}

// ── orphaned-driver detection ──────────────────────────────────────────────────

std::vector<std::pair<std::string, std::string>> DriverManager::findOrphanedDrivers(
    const GPUInfo& gpu, const std::vector<DriverInfo>& relevantList,
    const std::set<GPUVendor>& presentVendors) {

    std::vector<std::pair<std::string, std::string>> orphans;
    std::set<std::string> whitelist;
    for (const auto& d : relevantList) whitelist.insert(d.packageName);

    std::string thisVendor = gpu.vendorName;

    struct VendorUniverse { GPUVendor vendor; const char* label; const std::vector<std::string>& pkgs; };
    std::vector<VendorUniverse> universes = {
        {GPUVendor::NVIDIA, "NVIDIA", nvidiaPackageUniverse()},
        {GPUVendor::AMD,    "AMD",    amdPackageUniverse()},
        {GPUVendor::INTEL,  "Intel",  intelPackageUniverse()},
        {GPUVendor::S3,     "S3 Graphics", s3PackageUniverse()},
        {GPUVendor::VIA,    "VIA Technologies", viaPackageUniverse()},
    };

    std::set<std::string> seen;
    for (const auto& u : universes) {
        bool sameVendor = (u.vendor == gpu.vendor);
        // A vendor whose GPU is ALSO present elsewhere in this system (hybrid
        // Intel+NVIDIA laptops etc.) gets checked against ITS OWN matching
        // GPU entry when the loop reaches it — skip it here entirely so it's
        // never flagged as "wrong vendor" just for legitimately coexisting.
        if (!sameVendor && presentVendors.count(u.vendor)) continue;

        for (const auto& pkg : u.pkgs) {
            if (seen.count(pkg)) continue;
            if (sameVendor && whitelist.count(pkg)) continue;  // belongs here, fine
            if (!isInstalled(pkg)) continue;
            seen.insert(pkg);
            std::string reason = sameVendor
                ? ("wrong generation for this " + thisVendor + " GPU")
                : ("belongs to " + std::string(u.label) + ", not " + thisVendor);
            orphans.emplace_back(pkg, reason);
        }
    }
    return orphans;
}

// ── public API ────────────────────────────────────────────────────────────────

void DriverManager::listDrivers() {
    if (sysInfo_.gpus.empty()) {
        std::cout << C_RED << "[hyprvisor] No GPUs detected.\n" << C_RESET;
        return;
    }
    if (!sysInfo_.vmName.empty())
        std::cout << C_YELLOW << "[hyprvisor] Running inside "
                  << sysInfo_.vmName << " VM\n" << C_RESET;

    std::set<GPUVendor> presentVendors;
    for (const auto& g : sysInfo_.gpus) presentVendors.insert(g.vendor);

    for (const auto& gpu : sysInfo_.gpus) {
        auto drivers = buildList(gpu);
        printDriverTable(gpu, drivers);

        for (const auto& orphan : findOrphanedDrivers(gpu, drivers, presentVendors)) {
            std::cout << C_RED << C_BOLD
                      << "\n  [!] Unrelated driver installed: " << orphan.first
                      << C_RESET << C_RED << " — " << orphan.second
                      << "\n      Remove it: sudo pacman -R " << orphan.first
                      << C_RESET << "\n";
        }
    }
}

void DriverManager::regenerateInitramfs() {
    if (execCmd("which mkinitcpio 2>/dev/null").empty()) return;
    std::string cmd = (geteuid() == 0) ? "mkinitcpio -P" : "sudo mkinitcpio -P";
    std::cout << C_CYAN << "[hyprvisor] Running: " << cmd << "\n" << C_RESET;
    if (system(cmd.c_str()) != 0)
        std::cerr << C_RED << "  [!] mkinitcpio -P failed — check the output above.\n" << C_RESET;
}

// ── kernel cmdline params (GCN 1.0/1.1 amdgpu.si_support/cik_support) ─────────

static bool allTokensPresent(const std::string& haystack, const std::string& tokens) {
    std::istringstream ps(tokens);
    std::string tok;
    while (ps >> tok)
        if (haystack.find(tok) == std::string::npos) return false;
    return true;
}

// Pipes `content` into `path` via `sudo tee` (or a plain shell redirect if
// we're already root) — needed because the process itself runs unprivileged
// (see runAUR()), so a direct ofstream to a root-owned /etc or /boot file
// would just fail with EACCES.
static bool writeFileAsRoot(const std::string& path, const std::string& content) {
    std::string cmd = (geteuid() == 0) ? ("cat > " + path)
                                        : ("sudo tee " + path + " > /dev/null");
    FILE* p = popen(cmd.c_str(), "w");
    if (!p) return false;
    fwrite(content.data(), 1, content.size(), p);
    return pclose(p) == 0;
}

static std::string readWholeFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void DriverManager::applyKernelParams(const std::string& params) {
    bool isRoot   = (geteuid() == 0);
    auto sudoCmd  = [&](const std::string& c) { return isRoot ? c : ("sudo " + c); };
    bool appliedAny = false;

    // ── GRUB ── /etc/default/grub → GRUB_CMDLINE_LINUX_DEFAULT="..."
    std::string grubConf = readWholeFile("/etc/default/grub");
    if (!grubConf.empty()) {
        const std::string key = "GRUB_CMDLINE_LINUX_DEFAULT=\"";
        size_t pos = grubConf.find(key);
        if (pos != std::string::npos) {
            size_t valStart = pos + key.size();
            size_t valEnd   = grubConf.find('"', valStart);
            if (valEnd != std::string::npos) {
                std::string existing = grubConf.substr(valStart, valEnd - valStart);
                if (!allTokensPresent(existing, params)) {
                    std::cout << C_CYAN
                              << "[hyprvisor] Adding kernel params to GRUB_CMDLINE_LINUX_DEFAULT: "
                              << params << "\n" << C_RESET;
                    system(sudoCmd("cp /etc/default/grub /etc/default/grub.hyprvisor.bak").c_str());

                    std::string newVal = existing;
                    if (!newVal.empty() && newVal.back() != ' ') newVal += " ";
                    newVal += params;
                    std::string newContent =
                        grubConf.substr(0, valStart) + newVal + grubConf.substr(valEnd);

                    if (writeFileAsRoot("/etc/default/grub", newContent)) {
                        std::string regen = sudoCmd("grub-mkconfig -o /boot/grub/grub.cfg");
                        std::cout << C_CYAN << "[hyprvisor] Running: " << regen << "\n" << C_RESET;
                        if (system(regen.c_str()) != 0)
                            std::cerr << C_RED << "  [!] grub-mkconfig failed.\n" << C_RESET;
                        appliedAny = true;
                    } else {
                        std::cerr << C_RED << "  [!] Failed to write /etc/default/grub "
                                             "(backup at /etc/default/grub.hyprvisor.bak).\n" << C_RESET;
                    }
                }
            }
        }
    }

    // ── systemd-boot ── /boot/loader/entries/*.conf → "options ..." line
    std::string entries = execCmd("find /boot/loader/entries -maxdepth 1 -name '*.conf' 2>/dev/null");
    std::istringstream entryList(entries);
    std::string path;
    while (std::getline(entryList, path)) {
        if (path.empty()) continue;
        std::string content = readWholeFile(path);
        if (content.empty()) continue;

        size_t lineStart = std::string::npos;
        if (content.rfind("options ", 0) == 0) {
            lineStart = 0;
        } else {
            size_t nlOpt = content.find("\noptions ");
            if (nlOpt != std::string::npos) lineStart = nlOpt + 1;
        }
        if (lineStart == std::string::npos) continue;

        size_t lineEnd = content.find('\n', lineStart);
        if (lineEnd == std::string::npos) lineEnd = content.size();
        std::string line = content.substr(lineStart, lineEnd - lineStart);
        if (allTokensPresent(line, params)) continue;

        std::string newLine = line;
        if (!newLine.empty() && newLine.back() != ' ') newLine += " ";
        newLine += params;
        std::string newContent = content.substr(0, lineStart) + newLine + content.substr(lineEnd);

        std::cout << C_CYAN << "[hyprvisor] Adding kernel params to " << path << ": "
                  << params << "\n" << C_RESET;
        system(sudoCmd("cp " + path + " " + path + ".hyprvisor.bak").c_str());
        if (writeFileAsRoot(path, newContent))
            appliedAny = true;
        else
            std::cerr << C_RED << "  [!] Failed to write " << path
                                 << " (backup at " << path << ".hyprvisor.bak).\n" << C_RESET;
    }

    if (!appliedAny)
        std::cout << C_DIM
                  << "  (no GRUB or systemd-boot config found/changed for kernel params — "
                     "add \"" << params << "\" to your bootloader's kernel cmdline manually)\n"
                  << C_RESET;
}

bool DriverManager::cleanAllDrivers(bool noConfirm) {
    noConfirm_ = noConfirm;

    std::vector<std::string> universe;
    auto add = [&](const std::vector<std::string>& u) {
        universe.insert(universe.end(), u.begin(), u.end());
    };
    add(nvidiaPackageUniverse());
    add(amdPackageUniverse());
    add(intelPackageUniverse());
    add(s3PackageUniverse());
    add(viaPackageUniverse());
    // mesa/lib32-mesa deliberately excluded: they're the shared OpenGL
    // foundation nearly everything on the system depends on (window
    // managers, browsers, etc.), not a vendor-specific driver choice —
    // removing them has a huge, unrelated blast radius. --install will
    // ensure they're present again regardless.

    std::vector<std::string> toRemove;
    for (const auto& pkg : universe)
        if (isInstalled(pkg)) toRemove.push_back(pkg);

    if (toRemove.empty()) {
        std::cout << C_GREEN
                  << "[hyprvisor] No known GPU driver packages installed — nothing to clean.\n"
                  << C_RESET;
        return true;
    }

    std::cout << C_YELLOW << C_BOLD
              << "[hyprvisor] --clean will remove ALL of the following GPU driver "
                 "packages, regardless of vendor:\n" << C_RESET;
    for (const auto& p : toRemove) std::cout << "    - " << p << "\n";

    if (!noConfirm_) {
        std::cout << C_BOLD << "Proceed? [y/N] " << C_RESET;
        std::string answer;
        std::getline(std::cin, answer);
        if (answer != "y" && answer != "Y") {
            std::cout << "[hyprvisor] Aborted — nothing removed.\n";
            return false;
        }
    }

    bool ok = runPacmanRemove(toRemove);
    if (ok)
        std::cout << C_GREEN << C_BOLD
                  << "[hyprvisor] Clean complete. Run --install to set up the correct "
                     "driver for your detected GPU.\n" << C_RESET;
    return ok;
}

bool DriverManager::installBestDriver(bool noConfirm) {
    noConfirm_ = noConfirm;

    if (sysInfo_.gpus.empty()) {
        std::cerr << C_RED << "[hyprvisor] No GPUs detected.\n" << C_RESET;
        return false;
    }

    bool allOk = true;
    bool installedDkms = false;
    int  gpuIndex = 0;

    std::set<GPUVendor> presentVendors;
    for (const auto& g : sysInfo_.gpus) presentVendors.insert(g.vendor);

    for (const auto& gpu : sysInfo_.gpus) {
        ++gpuIndex;
        std::cout << "\n" << C_BOLD
                  << "[GPU " << gpuIndex << "] " << gpu.name
                  << C_RESET << "\n";

        auto drivers = buildList(gpu);

        // Check recommended packages
        std::vector<std::string> pacmanPkgs, aurPkgs;
        std::vector<std::string> alreadyOk;

        for (const auto& d : drivers) {
            if (!d.recommended) continue;
            if (d.installed) {
                alreadyOk.push_back(d.packageName);
            } else {
                if (d.fromAUR) aurPkgs.push_back(d.packageName);
                else           pacmanPkgs.push_back(d.packageName);
            }
        }

        bool nothingToInstall = pacmanPkgs.empty() && aurPkgs.empty();
        if (nothingToInstall) {
            std::cout << C_GREEN << C_BOLD
                      << "  OK — all recommended drivers already installed:\n"
                      << C_RESET;
            for (const auto& p : alreadyOk)
                std::cout << C_GREEN << "       ✓ " << p << C_RESET << "\n";
        } else {
            // Show what's already good
            if (!alreadyOk.empty()) {
                for (const auto& p : alreadyOk)
                    std::cout << C_GREEN << "  ✓ already installed: " << p << C_RESET << "\n";
            }

            auto hasDkms = [](const std::vector<std::string>& pkgs) {
                for (const auto& p : pkgs)
                    if (p.find("dkms") != std::string::npos) return true;
                return false;
            };

            // Install missing packages
            if (!pacmanPkgs.empty()) {
                std::cout << C_CYAN << "  Packages to install (pacman):\n" << C_RESET;
                for (const auto& p : pacmanPkgs) std::cout << "    + " << p << "\n";
                if (!runPacman(pacmanPkgs)) {
                    std::cerr << C_RED << "  [!] pacman failed.\n" << C_RESET;
                    allOk = false;
                } else if (hasDkms(pacmanPkgs)) {
                    installedDkms = true;
                }
            }

            if (!aurPkgs.empty()) {
                std::cout << C_MAGENTA << "  Packages to install (AUR):\n" << C_RESET;
                for (const auto& p : aurPkgs) std::cout << "    + " << p << "\n";
                if (!runAUR(aurPkgs)) {
                    std::cerr << C_RED << "  [!] AUR install failed.\n" << C_RESET;
                    allOk = false;
                } else if (hasDkms(aurPkgs)) {
                    installedDkms = true;
                }
            }

            if (allOk)
                std::cout << C_GREEN << C_BOLD
                          << "  OK — driver installation complete.\n" << C_RESET;
        }

        // Remove any driver package that's installed but known-wrong for this
        // GPU: same-vendor-wrong-generation leftovers (e.g. plain "nvidia"
        // alongside the correct nvidia-580xx-dkms on a Pascal card) AND
        // anything from a totally unrelated vendor's stack (e.g. an AMD
        // driver on an NVIDIA-only machine, or nvidia-open-dkms on a Pascal
        // card that will never use it) — both are real conflict/orphan risk,
        // not just clutter.
        if (noConfirm_ && allOk) {
            std::vector<std::string> toRemove;
            for (const auto& d : drivers)
                if (isIncompatibleInstalled(d)) toRemove.push_back(d.packageName);
            for (const auto& orphan : findOrphanedDrivers(gpu, drivers, presentVendors))
                toRemove.push_back(orphan.first);

            if (!toRemove.empty()) {
                std::cout << C_RED << C_BOLD << "  Removing unrelated/incompatible driver(s): "
                          << C_RESET;
                for (const auto& p : toRemove) std::cout << p << " ";
                std::cout << "\n";
                if (!runPacmanRemove(toRemove))
                    std::cerr << C_RED << "  [!] Failed to remove some driver(s) — "
                                         "remove manually: sudo pacman -R "
                              << toRemove[0] << (toRemove.size() > 1 ? " ..." : "")
                              << "\n" << C_RESET;
            }
        }

        // GCN 1.0/1.1 (HD 7xxx, R7/R9 2xx) needs a kernel param to enable amdgpu
        // at all (otherwise it silently stays on the far weaker 'radeon' driver)
        // — apply it for real instead of just printing the warning, so a script
        // driving --noconfirm actually gets a working GPU after reboot.
        if (noConfirm_ && allOk && gpu.vendor == GPUVendor::AMD &&
            detectAmdGen(gpu) == AmdGen::GCN_OLD) {
            applyKernelParams("amdgpu.si_support=1 amdgpu.cik_support=1");
        }
    }

    // DKMS modules need the initramfs rebuilt to actually take effect on the
    // next boot. Official nvidia/nvidia-dkms ship a pacman hook that already
    // does this; AUR legacy packages (470xx/390xx/340xx/580xx) may not, so do
    // it ourselves whenever --noconfirm is driving a fully unattended install.
    if (noConfirm_ && installedDkms && allOk)
        regenerateInitramfs();

    return allOk;
}

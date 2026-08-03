#pragma once
#include "gpu_detect.hpp"
#include <set>
#include <string>
#include <utility>
#include <vector>

enum class DriverPreference {
    AUTO,         // default: proprietary for NVIDIA, open-source for AMD/Intel
    PROPRIETARY,  // force proprietary drivers where available
    OPEN_SOURCE   // prefer open-source (nouveau, nvidia-open, mesa...)
};

struct DriverInfo {
    std::string packageName;
    std::string description;
    std::string type;        // "proprietary", "open-source", "open-kernel", "legacy", "virtual", "aur-legacy", "kernel-headers"
    bool        installed;
    bool        recommended;
    bool        fromAUR;     // requires AUR helper (yay/paru)
    bool        openSource;  // true = no proprietary blobs
    bool        versionMismatch = false;  // kernel-headers entry whose version doesn't match the running kernel
};

// AMD GPU generations
enum class AmdGen {
    TOO_OLD,    // Radeon 7000/8500 (R100/R200, pre-2004)       → xf86-video-ati only
    R300_R500,  // Radeon 9500–X1950 (R300-R500, 2002-2007)     → xf86-video-ati
    R600_R700,  // Radeon HD 2000–4870 (r600/r700, 2007-2009)   → xf86-video-ati + mesa
    EVERGREEN,  // Radeon HD 5000–6000 (Evergreen/NI, 2009-2011)→ xf86-video-ati + mesa
    GCN_OLD,    // Radeon HD 7000 / R7-R9 2xx (GCN 1.0-1.1)    → amdgpu/ati + mesa (no Vulkan)
    GCN_NEW,    // R9 285/380/390/Fury (GCN 1.2-1.3, 2015-2016) → amdgpu + mesa + vulkan-radeon
    POLARIS,    // RX 460/470/480/570/580/590 (Polaris, 2016)   → amdgpu + mesa + vulkan-radeon
    VEGA,       // RX Vega 56/64/VII (Vega, 2017-2019)          → amdgpu + mesa + vulkan-radeon
    RDNA1,      // RX 5000 series (Navi 10/14, 2019)            → amdgpu + mesa + vulkan-radeon
    RDNA2,      // RX 6000 series (Navi 21/22/23/24, 2020-2022) → amdgpu + mesa + vulkan-radeon
    RDNA3,      // RX 7000 series (Navi 31/32/33, 2022-2023)    → amdgpu + mesa + vulkan-radeon
    RDNA4,      // RX 9000 series (Navi 48/44, 2025)            → amdgpu + mesa + vulkan-radeon
    UNKNOWN_AMD
};

// S3 Graphics sub-families
enum class S3Gen {
    VIRGE,    // ViRGE/Trio/Vision (1995-1998)       → xf86-video-s3virge (AUR)
    SAVAGE,   // Savage 3D/4/2000/MX/IX (1998-2001)  → xf86-video-savage (AUR)
    CHROME,   // DeltaChrome/ProSavage/Chrome 4xx/5xx → xf86-video-openchrome (AUR)
    UNKNOWN_S3
};

// Intel GPU generations
enum class IntelGen {
    GMA,            // GMA 900/950/3000/3500 (pre-HD, 2004-2008)     → xf86-video-intel + mesa
    HD_LEGACY,      // HD Graphics 2000-6000 (Sandy/Ivy/Haswell/BW)  → mesa (modesetting DDX)
    HD_MODERN,      // HD 510-620, UHD 620-770 (Skylake–Tiger Lake)   → mesa + vulkan-intel
    IRIS_XE,        // Iris Xe (Xe-LP iGPU, Tiger/Alder/Raptor Lake)  → mesa + vulkan-intel
    ARC_ALCHEMIST,  // Arc A380/A580/A750/A770 (Xe-HPG, 2022)        → xe driver + mesa + compute
    ARC_BATTLEMAGE, // Arc B570/B580 (Xe2-HPG, 2024-2025)            → xe driver + mesa + compute
    UNKNOWN_INTEL
};

// NVIDIA architecture generations, ordered oldest→newest
enum class NvidiaGen {
    TOO_OLD,     // pre-8xxx: only nouveau
    TESLA_G,     // 8xxx/9xxx/GT2xx  → nvidia-340xx-dkms (AUR)
    FERMI,       // GTX 400/500      → nvidia-390xx-dkms (AUR)
    KEPLER,      // GTX 600/700      → nvidia-470xx-dkms
    MAXWELL,     // GTX 750/9xx      → nvidia (current)
    PASCAL,      // GTX 10xx         → nvidia (current)
    VOLTA,       // Titan V          → nvidia (current)
    TURING,      // RTX 20xx/GTX 16xx→ nvidia + nvidia-open supported
    AMPERE,      // RTX 30xx         → nvidia + nvidia-open
    ADA,         // RTX 40xx         → nvidia + nvidia-open
    BLACKWELL,   // RTX 50xx         → nvidia + nvidia-open
    UNKNOWN_GEN
};

class DriverManager {
public:
    DriverManager(const SystemInfo& info, DriverPreference pref = DriverPreference::AUTO);

    void listDrivers();
    void listDriversJson();
    bool installBestDriver(bool noConfirm = false);
    // Removes every known GPU driver package (any vendor, any generation)
    // regardless of what's actually detected — for a clean slate before a
    // fresh --install. Prompts for confirmation unless noConfirm is set.
    bool cleanAllDrivers(bool noConfirm = false);

private:
    SystemInfo       sysInfo_;
    DriverPreference pref_;
    bool             noConfirm_ = false;

    std::vector<DriverInfo> buildList(const GPUInfo& gpu);
    std::vector<DriverInfo> buildNvidiaList(const GPUInfo& gpu);
    std::vector<DriverInfo> buildAmdList(const GPUInfo& gpu);
    std::vector<DriverInfo> buildIntelList(const GPUInfo& gpu);
    std::vector<DriverInfo> buildS3List(const GPUInfo& gpu);
    std::vector<DriverInfo> buildViaList(const GPUInfo& gpu);
    NvidiaGen               detectNvidiaGen(const GPUInfo& gpu);
    AmdGen                  detectAmdGen(const GPUInfo& gpu);
    IntelGen                detectIntelGen(const GPUInfo& gpu);
    S3Gen                   detectS3Gen(const GPUInfo& gpu);

    bool isInstalled(const std::string& pkg);
    bool isAvailable(const std::string& pkg);
    // Package that owns the currently running kernel's module directory,
    // with "-headers" appended (e.g. "linux" -> "linux-headers", or this
    // distro's custom-numbered kernels: "linux618" -> "linux618-headers").
    // Works for any kernel package naming scheme since it asks pacman which
    // package actually owns /usr/lib/modules/$(uname -r) instead of guessing
    // from a fixed name list. Returns "" if it can't be determined.
    std::string detectKernelHeadersPackage();
    // Packages from OTHER vendors' driver stacks, or from a different
    // generation of the SAME vendor, that are installed but don't belong to
    // the detected GPU at all (not even as an optional/non-recommended
    // candidate) — e.g. nvidia-open-dkms installed on a Pascal card, or an
    // AMD driver stack present on an NVIDIA-only machine. `presentVendors` is
    // every vendor detected ANYWHERE in this system (hybrid/Optimus laptops
    // have both Intel and NVIDIA legitimately) — packages for a vendor that's
    // also present are skipped here and left to be checked against their own
    // matching GPU entry instead, so hybrid setups don't get flagged for
    // simply having both vendors' drivers installed.
    std::vector<std::pair<std::string, std::string>> findOrphanedDrivers(
        const GPUInfo& gpu, const std::vector<DriverInfo>& relevantList,
        const std::set<GPUVendor>& presentVendors);
    bool runPacman(const std::vector<std::string>& packages);
    bool runPacmanRemove(const std::vector<std::string>& packages);
    bool runAUR(const std::vector<std::string>& packages);
    void regenerateInitramfs();
    void applyKernelParams(const std::string& params);
    void printDriverTable(const GPUInfo& gpu, const std::vector<DriverInfo>& drivers);
};

#pragma once
#include "wifi_detect.hpp"
#include <string>
#include <vector>

struct DriverPackage {
    std::string name;         // human-readable description
    std::string pkgName;      // real pacman/AUR package name
    std::string module;       // kernel module name once loaded
    bool        needsHeaders; // requires linux-headers (DKMS build)
    bool        isAUR;        // needs an AUR helper (yay/paru/trizen)
    bool        isFirmware;   // is a firmware package (linux-firmware)
    KernelSupport support;
};

struct InstallResult {
    bool        success;
    bool        alreadyLoaded; // driver module already active
    std::string message;
};

class DriverInstaller {
public:
    DriverInstaller() = default;

    void          listDrivers(const WiFiAdapter& adapter);
    InstallResult install(const WiFiAdapter& adapter, bool noConfirm);

private:
    std::vector<DriverPackage> buildPackageList(const WiFiAdapter& adapter);
    std::vector<DriverPackage> realtekPackages(const WiFiAdapter& adapter);

    bool isModuleLoaded(const std::string& module) const;
    bool isPackageInstalled(const std::string& pkg) const;
    // Whether pacman (official repos) or the detected AUR helper knows this
    // package name — tells you whether `--install` can actually fetch it,
    // without downloading anything.
    bool isPackageAvailable(const std::string& pkg, bool isAUR) const;

    bool runPacman(const std::vector<std::string>& pkgs, bool noConfirm) const;
    bool runAUR(const std::vector<std::string>& pkgs, bool noConfirm) const;
    std::string detectKernelHeadersPackage() const;
    std::string detectAurHelper() const;

    void printDriverRow(const DriverPackage& dp) const;

    // Every real pacman/AUR package name that could plausibly end up
    // installed for this vendor across ANY chip variant (not just the one
    // detected) — used to spot leftover/wrong-chip driver packages that
    // buildPackageList() for the CURRENT adapter would never mention.
    std::vector<std::string> vendorPackageUniverse(WiFiVendor vendor) const;

    // Installed pacman packages relevant to this adapter's vendor, plus any
    // that conflict with the currently recommended driver (a different
    // chip's DKMS package left over, or a mutually-exclusive alternative
    // driver installed alongside the primary one).
    void printInstalledAndConflicts(const WiFiAdapter& adapter,
                                    const std::vector<DriverPackage>& pkgs) const;
};

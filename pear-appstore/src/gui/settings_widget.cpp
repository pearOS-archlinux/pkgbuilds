#include "settings_widget.h"
#include "scroll_bar_fade.h"
#include "../utils/logger.h"
#include "../core/alpm_wrapper.h"
#include "../core/package_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QScrollArea>
#include <QWidget>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent) {
    
    setupUi();
    loadCurrentSettings();
    
    Logger::info("SettingsWidget created successfully");
}

void SettingsWidget::setupUi() {
    // Create main layout for the widget
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create scroll area
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget that will be scrollable
    auto* contentWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    auto* titleLabel = new QLabel("Settings", contentWidget);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Repository Settings
    createRepositorySettings();
    mainLayout->addWidget(m_repositoryGroup);
    
    // Chaotic-AUR Setup
    createChaoticAurSettings();
    mainLayout->addWidget(m_chaoticAurGroup);
    
    // Maintenance Settings
    createMaintenanceSettings();
    mainLayout->addWidget(m_maintenanceGroup);
    
    // Status label
    m_statusLabel = new QLabel(contentWidget);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->hide();
    mainLayout->addWidget(m_statusLabel);
    
    // Add stretch at the bottom
    mainLayout->addStretch();
    
    // Set the content widget to the scroll area
    scrollArea->setWidget(contentWidget);
    applyScrollBarFade(scrollArea);
    outerLayout->addWidget(scrollArea);
    setLayout(outerLayout);
}

void SettingsWidget::createRepositorySettings() {
    m_repositoryGroup = new QGroupBox("Package Repositories", this);
    auto* repoLayout = new QVBoxLayout(m_repositoryGroup);
    
    // Description
    auto* descLabel = new QLabel(
        "Select which package repositories to use for searching and installing packages.\n"
        "Core and Extra repositories are required and cannot be disabled.", 
        this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: #666; margin-bottom: 10px; }");
    repoLayout->addWidget(descLabel);
    
    // Core repository (always enabled, cannot be disabled)
    m_coreRepoCheckbox = new QCheckBox("Core - Essential system packages", this);
    m_coreRepoCheckbox->setChecked(true);
    m_coreRepoCheckbox->setEnabled(false);
    m_coreRepoCheckbox->setToolTip("Core repository is required and cannot be disabled");
    repoLayout->addWidget(m_coreRepoCheckbox);
    
    // Extra repository (always enabled, cannot be disabled)
    m_extraRepoCheckbox = new QCheckBox("Extra - Additional official packages", this);
    m_extraRepoCheckbox->setChecked(true);
    m_extraRepoCheckbox->setEnabled(false);
    m_extraRepoCheckbox->setToolTip("Extra repository is required and cannot be disabled");
    repoLayout->addWidget(m_extraRepoCheckbox);
    
    // Multilib repository (optional, can be enabled/disabled)
    m_multilibRepoCheckbox = new QCheckBox("Multilib - 32-bit packages on x86_64", this);
    m_multilibRepoCheckbox->setToolTip(
        "Enable multilib repository for 32-bit applications on 64-bit systems.\n"
        "This modifies /etc/pacman.conf and requires administrator privileges.");
    connect(m_multilibRepoCheckbox, &QCheckBox::checkStateChanged, 
            this, &SettingsWidget::onSettingsChanged);
    repoLayout->addWidget(m_multilibRepoCheckbox);
    
    // Chaotic-AUR repository (optional, can be enabled/disabled)
    m_chaoticAurCheckbox = new QCheckBox("Chaotic-AUR - Pre-built AUR packages", this);
    m_chaoticAurCheckbox->setToolTip(
        "Enable chaotic-aur repository for pre-built AUR packages.\n"
        "This modifies /etc/pacman.conf and requires administrator privileges.\n"
        "Note: chaotic-keyring and chaotic-mirrorlist must be installed first.");
    connect(m_chaoticAurCheckbox, &QCheckBox::checkStateChanged, 
            this, &SettingsWidget::onSettingsChanged);
    repoLayout->addWidget(m_chaoticAurCheckbox);
    
    // Info label
    auto* infoLabel = new QLabel(
        "Note: Changes to repositories require modifying system configuration files "
        "and may require administrator privileges.", 
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #888; font-style: italic; margin-top: 10px; }");
    repoLayout->addWidget(infoLabel);
    
    // Apply button
    auto* applyLayout = new QHBoxLayout();
    applyLayout->addStretch();
    
    m_applyButton = new QPushButton("Apply Changes", this);
    m_applyButton->setMinimumWidth(150);
    m_applyButton->setEnabled(false);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsWidget::onApplyClicked);
    applyLayout->addWidget(m_applyButton);
    
    repoLayout->addLayout(applyLayout);
    
    m_repositoryGroup->setLayout(repoLayout);
}

void SettingsWidget::createChaoticAurSettings() {
    m_chaoticAurGroup = new QGroupBox("Chaotic-AUR Setup", this);
    auto* chaoticLayout = new QVBoxLayout(m_chaoticAurGroup);
    
    // Description
    auto* descLabel = new QLabel(
        "Chaotic-AUR provides pre-built AUR packages, making installation faster and easier.\n"
        "Setup requires installing the keyring and mirrorlist packages.",
        this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: #666; margin-bottom: 10px; }");
    chaoticLayout->addWidget(descLabel);
    
    // Setup button section
    auto* setupLayout = new QHBoxLayout();
    
    auto* setupLabel = new QLabel("Install Chaotic-AUR:", this);
    setupLabel->setStyleSheet("QLabel { font-weight: bold; }");
    setupLayout->addWidget(setupLabel);
    
    setupLayout->addStretch();
    
    m_setupChaoticButton = new QPushButton("Setup Chaotic-AUR", this);
    m_setupChaoticButton->setMinimumWidth(150);
    m_setupChaoticButton->setToolTip(
        "Install chaotic-keyring and chaotic-mirrorlist packages.\n"
        "This will enable access to pre-built AUR packages.");
    connect(m_setupChaoticButton, &QPushButton::clicked, this, &SettingsWidget::onSetupChaoticClicked);
    setupLayout->addWidget(m_setupChaoticButton);
    
    chaoticLayout->addLayout(setupLayout);
    
    // Setup info
    auto* setupInfoLabel = new QLabel(
        "This will install chaotic-keyring and chaotic-mirrorlist from the official Chaotic-AUR repository.",
        this);
    setupInfoLabel->setWordWrap(true);
    setupInfoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 5px; margin-left: 10px; }");
    chaoticLayout->addWidget(setupInfoLabel);
    
    // Spacer
    chaoticLayout->addSpacing(10);
    
    // Remove button section
    auto* removeLayout = new QHBoxLayout();
    
    auto* removeLabel = new QLabel("Remove Chaotic-AUR:", this);
    removeLabel->setStyleSheet("QLabel { font-weight: bold; }");
    removeLayout->addWidget(removeLabel);
    
    removeLayout->addStretch();
    
    m_removeChaoticButton = new QPushButton("Remove Chaotic-AUR", this);
    m_removeChaoticButton->setMinimumWidth(150);
    m_removeChaoticButton->setToolTip(
        "Remove chaotic-keyring and chaotic-mirrorlist packages.\n"
        "You may need to manually remove the repository from /etc/pacman.conf");
    connect(m_removeChaoticButton, &QPushButton::clicked, this, &SettingsWidget::onRemoveChaoticClicked);
    removeLayout->addWidget(m_removeChaoticButton);
    
    chaoticLayout->addLayout(removeLayout);
    
    // Remove info
    auto* removeInfoLabel = new QLabel(
        "This will remove the Chaotic-AUR packages. You need to uncheck the Chaotic-AUR option above or manually edit /etc/pacman.conf to remove the repository configuration.",
        this);
    removeInfoLabel->setWordWrap(true);
    removeInfoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 5px; margin-left: 10px; }");
    chaoticLayout->addWidget(removeInfoLabel);
    
    m_chaoticAurGroup->setLayout(chaoticLayout);
}

void SettingsWidget::createMaintenanceSettings() {
    m_maintenanceGroup = new QGroupBox("Maintenance", this);
    auto* maintenanceLayout = new QVBoxLayout(m_maintenanceGroup);
    
    // Description
    auto* descLabel = new QLabel(
        "System maintenance and troubleshooting tools.",
        this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: #666; margin-bottom: 10px; }");
    maintenanceLayout->addWidget(descLabel);
    
    // Lock file section
    auto* lockFileLayout = new QHBoxLayout();
    
    auto* lockFileLabel = new QLabel(
        "Pacman Database Lock:",
        this);
    lockFileLabel->setStyleSheet("QLabel { font-weight: bold; }");
    lockFileLayout->addWidget(lockFileLabel);
    
    lockFileLayout->addStretch();
    
    m_removeLockButton = new QPushButton("Remove Lock File", this);
    m_removeLockButton->setMinimumWidth(150);
    m_removeLockButton->setToolTip(
        "Remove /var/lib/pacman/db.lck if pacman is stuck.\n"
        "Only use this if you're sure no other pacman process is running.");
    connect(m_removeLockButton, &QPushButton::clicked, this, &SettingsWidget::onRemoveLockClicked);
    lockFileLayout->addWidget(m_removeLockButton);
    
    maintenanceLayout->addLayout(lockFileLayout);
    
    // Lock file info
    auto* lockInfoLabel = new QLabel(
        "If pacman was interrupted, it may leave a lock file that prevents other operations.\n"
        "Remove it only if you're certain no package manager is currently running.",
        this);
    lockInfoLabel->setWordWrap(true);
    lockInfoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 5px; margin-left: 10px; }");
    maintenanceLayout->addWidget(lockInfoLabel);
    
    // Spacer
    maintenanceLayout->addSpacing(15);
    
    // Sync repositories section
    auto* syncReposLayout = new QHBoxLayout();
    
    auto* syncReposLabel = new QLabel(
        "Synchronize Repositories:",
        this);
    syncReposLabel->setStyleSheet("QLabel { font-weight: bold; }");
    syncReposLayout->addWidget(syncReposLabel);
    
    syncReposLayout->addStretch();
    
    m_syncReposButton = new QPushButton("Sync Repositories", this);
    m_syncReposButton->setMinimumWidth(150);
    m_syncReposButton->setToolTip(
        "Manually synchronize package databases (pacman -Sy).\n"
        "This updates the list of available packages from all enabled repositories.");
    connect(m_syncReposButton, &QPushButton::clicked, this, &SettingsWidget::onSyncReposClicked);
    syncReposLayout->addWidget(m_syncReposButton);
    
    maintenanceLayout->addLayout(syncReposLayout);
    
    // Sync info
    auto* syncInfoLabel = new QLabel(
        "Use this to manually update your package database. This is useful after enabling/disabling repositories\n"
        "or when you want to ensure you have the latest package information.",
        this);
    syncInfoLabel->setWordWrap(true);
    syncInfoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 5px; margin-left: 10px; }");
    maintenanceLayout->addWidget(syncInfoLabel);
    
    // Spacer
    maintenanceLayout->addSpacing(15);
    
    // Cancel running process section
    auto* cancelProcessLayout = new QHBoxLayout();
    
    auto* cancelProcessLabel = new QLabel(
        "Cancel Running Process:",
        this);
    cancelProcessLabel->setStyleSheet("QLabel { font-weight: bold; }");
    cancelProcessLayout->addWidget(cancelProcessLabel);
    
    cancelProcessLayout->addStretch();
    
    m_cancelProcessButton = new QPushButton("Cancel Process", this);
    m_cancelProcessButton->setMinimumWidth(150);
    m_cancelProcessButton->setToolTip(
        "Cancel any running package operation (install, uninstall, update).\n"
        "Use this if an operation is stuck or taking too long.\n"
        "This is different from removing the lock file - it actually stops the running process.");
    connect(m_cancelProcessButton, &QPushButton::clicked, this, &SettingsWidget::onCancelProcessClicked);
    cancelProcessLayout->addWidget(m_cancelProcessButton);
    
    maintenanceLayout->addLayout(cancelProcessLayout);
    
    // Cancel process info
    auto* cancelInfoLabel = new QLabel(
        "Use this to cancel a stuck installation, uninstallation, or update process.\n"
        "This is useful when you see 'Another operation is already in progress' and want to stop it.",
        this);
    cancelInfoLabel->setWordWrap(true);
    cancelInfoLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; margin-top: 5px; margin-left: 10px; }");
    maintenanceLayout->addWidget(cancelInfoLabel);
    
    m_maintenanceGroup->setLayout(maintenanceLayout);
}

void SettingsWidget::loadCurrentSettings() {
    // Check if multilib is currently enabled
    bool multilibEnabled = isMultilibEnabledInPacmanConf();
    m_multilibRepoCheckbox->setChecked(multilibEnabled);
    m_originalMultilibState = multilibEnabled;
    
    // Check if chaotic-aur is enabled
    bool chaoticAurEnabled = isChaoticAurEnabledInPacmanConf();
    m_chaoticAurCheckbox->setChecked(chaoticAurEnabled);
    m_originalChaoticAurState = chaoticAurEnabled;
    
    Logger::info(QString("Loaded settings: multilib=%1, chaotic-aur=%2")
                 .arg(multilibEnabled ? "enabled" : "disabled")
                 .arg(chaoticAurEnabled ? "enabled" : "disabled"));
}

bool SettingsWidget::isMultilibEnabledInPacmanConf() const {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QTextStream in(&file);
    bool inMultilibSection = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Check for [multilib] section header
        if (line == "[multilib]") {
            inMultilibSection = true;
            continue;
        }
        
        // If we found [multilib] section, check if it's not commented
        if (inMultilibSection && !line.isEmpty() && !line.startsWith("#")) {
            // If we find Include directive, multilib is enabled
            if (line.startsWith("Include")) {
                file.close();
                return true;
            }
        }
        
        // If we hit another section, stop
        if (inMultilibSection && line.startsWith("[") && line != "[multilib]") {
            break;
        }
    }
    
    file.close();
    return false;
}

bool SettingsWidget::isChaoticAurEnabledInPacmanConf() const {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QTextStream in(&file);
    bool inChaoticAurSection = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Check for [chaotic-aur] section header
        if (line == "[chaotic-aur]") {
            inChaoticAurSection = true;
            continue;
        }
        
        // If we found [chaotic-aur] section, check if it's not commented
        if (inChaoticAurSection && !line.isEmpty() && !line.startsWith("#")) {
            // If we find Include or Server directive, chaotic-aur is enabled
            if (line.startsWith("Include") || line.startsWith("Server")) {
                file.close();
                return true;
            }
        }
        
        // If we hit another section, stop
        if (inChaoticAurSection && line.startsWith("[") && line != "[chaotic-aur]") {
            break;
        }
    }
    
    file.close();
    return false;
}

bool SettingsWidget::enableMultilibInPacmanConf() {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    bool multilibSectionFound = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        
        // Check if this is a commented [multilib] section
        if (line.trimmed() == "#[multilib]") {
            lines.append("[multilib]");
            multilibSectionFound = true;
        } 
        // Check if the Include line in multilib section is commented
        else if (multilibSectionFound && line.trimmed().startsWith("#Include") && 
                 line.contains("mirrorlist")) {
            lines.append(line.mid(1)); // Remove the # comment character
            multilibSectionFound = false; // Reset flag after processing
        }
        else {
            lines.append(line);
        }
    }
    file.close();
    
    // Write back to file using pkexec for elevated privileges
    QString tempFile = "/tmp/pacman.conf.tmp";
    QFile temp(tempFile);
    if (!temp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error("Failed to create temporary file");
        return false;
    }
    
    QTextStream out(&temp);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    temp.close();
    
    // Use pkexec to copy the file with elevated privileges
    QProcess process;
    process.start("pkexec", QStringList() << "cp" << tempFile << "/etc/pacman.conf");
    process.waitForFinished(30000); // 30 second timeout
    
    if (process.exitCode() != 0) {
        Logger::error("Failed to update pacman.conf with elevated privileges");
        QFile::remove(tempFile);
        return false;
    }
    
    QFile::remove(tempFile);
    Logger::info("Successfully enabled multilib repository");
    return true;
}

bool SettingsWidget::disableMultilibInPacmanConf() {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    bool inMultilibSection = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();
        
        // Check if this is [multilib] section
        if (trimmedLine == "[multilib]") {
            lines.append("#[multilib]");
            inMultilibSection = true;
        }
        // Check if we're in multilib section and this is the Include line
        else if (inMultilibSection && trimmedLine.startsWith("Include") && 
                 trimmedLine.contains("mirrorlist")) {
            lines.append("#" + line);
            inMultilibSection = false;
        }
        // Check if we hit another section
        else if (trimmedLine.startsWith("[") && trimmedLine != "[multilib]") {
            lines.append(line);
            inMultilibSection = false;
        }
        else {
            lines.append(line);
        }
    }
    file.close();
    
    // Write back to file using pkexec for elevated privileges
    QString tempFile = "/tmp/pacman.conf.tmp";
    QFile temp(tempFile);
    if (!temp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error("Failed to create temporary file");
        return false;
    }
    
    QTextStream out(&temp);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    temp.close();
    
    // Use pkexec to copy the file with elevated privileges
    QProcess process;
    process.start("pkexec", QStringList() << "cp" << tempFile << "/etc/pacman.conf");
    process.waitForFinished(30000); // 30 second timeout
    
    if (process.exitCode() != 0) {
        Logger::error("Failed to update pacman.conf with elevated privileges");
        QFile::remove(tempFile);
        return false;
    }
    
    QFile::remove(tempFile);
    Logger::info("Successfully disabled multilib repository");
    return true;
}

bool SettingsWidget::enableChaoticAurInPacmanConf() {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    bool inCommentedChaoticAurSection = false;
    bool chaoticAurSectionExists = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();
        
        // Check if chaotic-aur section already exists (uncommented or commented)
        if (trimmedLine == "[chaotic-aur]" || trimmedLine == "#[chaotic-aur]") {
            chaoticAurSectionExists = true;
            
            // If it's commented, uncomment it
            if (trimmedLine == "#[chaotic-aur]") {
                lines.append("[chaotic-aur]");
                inCommentedChaoticAurSection = true;
            } else {
                // Already uncommented, keep as is
                lines.append(line);
            }
        } 
        // Check if the Include/Server line in chaotic-aur section is commented
        else if (inCommentedChaoticAurSection && trimmedLine.startsWith("#") && 
                 (trimmedLine.contains("Include") || trimmedLine.contains("Server"))) {
            // Remove the # comment character
            lines.append(line.mid(line.indexOf('#') + 1));
            inCommentedChaoticAurSection = false;
        }
        // Check if we hit another section, reset flag
        else if (trimmedLine.startsWith("[") && trimmedLine != "[chaotic-aur]" && trimmedLine != "#[chaotic-aur]") {
            lines.append(line);
            inCommentedChaoticAurSection = false;
        }
        else {
            lines.append(line);
        }
    }
    file.close();
    
    // If chaotic-aur section doesn't exist at all, add it
    if (!chaoticAurSectionExists) {
        lines.append("");
        lines.append("[chaotic-aur]");
        lines.append("Include = /etc/pacman.d/chaotic-mirrorlist");
    }
    
    // Write back to file using pkexec for elevated privileges
    QString tempFile = "/tmp/pacman.conf.tmp";
    QFile temp(tempFile);
    if (!temp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error("Failed to create temporary file");
        return false;
    }
    
    QTextStream out(&temp);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    temp.close();
    
    // Use pkexec to copy the file with elevated privileges
    QProcess process;
    process.start("pkexec", QStringList() << "cp" << tempFile << "/etc/pacman.conf");
    process.waitForFinished(30000); // 30 second timeout
    
    if (process.exitCode() != 0) {
        Logger::error("Failed to update pacman.conf with elevated privileges");
        QFile::remove(tempFile);
        return false;
    }
    
    QFile::remove(tempFile);
    Logger::info("Successfully enabled chaotic-aur repository");
    return true;
}

bool SettingsWidget::disableChaoticAurInPacmanConf() {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    bool inChaoticAurSection = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();
        
        // Check if this is [chaotic-aur] section (uncommented or already commented)
        if (trimmedLine == "[chaotic-aur]") {
            lines.append("#[chaotic-aur]");
            inChaoticAurSection = true;
        }
        else if (trimmedLine == "#[chaotic-aur]") {
            // Already commented, keep as is
            lines.append(line);
            inChaoticAurSection = false;
        }
        // Check if we're in chaotic-aur section and this is the Include/Server line (not already commented)
        else if (inChaoticAurSection && !trimmedLine.startsWith("#") &&
                 (trimmedLine.startsWith("Include") || trimmedLine.startsWith("Server"))) {
            lines.append("#" + line);
            inChaoticAurSection = false;
        }
        // Check if we hit another section
        else if (trimmedLine.startsWith("[") && trimmedLine != "[chaotic-aur]" && trimmedLine != "#[chaotic-aur]") {
            lines.append(line);
            inChaoticAurSection = false;
        }
        else {
            lines.append(line);
        }
    }
    file.close();
    
    // Write back to file using pkexec for elevated privileges
    QString tempFile = "/tmp/pacman.conf.tmp";
    QFile temp(tempFile);
    if (!temp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error("Failed to create temporary file");
        return false;
    }
    
    QTextStream out(&temp);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    temp.close();
    
    // Use pkexec to copy the file with elevated privileges
    QProcess process;
    process.start("pkexec", QStringList() << "cp" << tempFile << "/etc/pacman.conf");
    process.waitForFinished(30000); // 30 second timeout
    
    if (process.exitCode() != 0) {
        Logger::error("Failed to update pacman.conf with elevated privileges");
        QFile::remove(tempFile);
        return false;
    }
    
    QFile::remove(tempFile);
    Logger::info("Successfully disabled chaotic-aur repository");
    return true;
}

void SettingsWidget::onSettingsChanged() {
    // Enable apply button when settings change
    bool hasChanges = (m_multilibRepoCheckbox->isChecked() != m_originalMultilibState) ||
                      (m_chaoticAurCheckbox->isChecked() != m_originalChaoticAurState);
    m_applyButton->setEnabled(hasChanges);
    m_statusLabel->hide();
}

void SettingsWidget::onApplyClicked() {
    bool currentMultilibState = m_multilibRepoCheckbox->isChecked();
    bool currentChaoticAurState = m_chaoticAurCheckbox->isChecked();
    bool success = true;
    bool changesApplied = false;
    
    // Handle multilib changes
    if (currentMultilibState != m_originalMultilibState) {
        // Show confirmation dialog
        QString message;
        if (currentMultilibState) {
            message = "This will enable the multilib repository by modifying /etc/pacman.conf.\n"
                     "You will be prompted for administrator privileges.\n\n"
                     "After enabling, you should run 'sudo pacman -Sy' to sync the databases.\n\n"
                     "Do you want to continue?";
        } else {
            message = "This will disable the multilib repository by modifying /etc/pacman.conf.\n"
                     "You will be prompted for administrator privileges.\n\n"
                     "Do you want to continue?";
        }
        
        auto reply = QMessageBox::question(this, "Confirm Repository Change", 
                                          message,
                                          QMessageBox::Yes | QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            return;
        }
        
        // Apply the change
        if (currentMultilibState) {
            success = enableMultilibInPacmanConf();
        } else {
            success = disableMultilibInPacmanConf();
        }
        
        if (success) {
            m_originalMultilibState = currentMultilibState;
            changesApplied = true;
            
            // Emit signal to notify other widgets
            emit multilibStatusChanged(currentMultilibState);
        } else {
            success = false;
        }
    }
    
    // Handle chaotic-aur changes
    if (currentChaoticAurState != m_originalChaoticAurState) {
        // Show confirmation dialog
        QString message;
        if (currentChaoticAurState) {
            message = "This will enable the chaotic-aur repository by modifying /etc/pacman.conf.\n"
                     "You will be prompted for administrator privileges.\n\n"
                     "Note: Make sure chaotic-keyring and chaotic-mirrorlist are installed first.\n\n"
                     "After enabling, you should run 'sudo pacman -Sy' to sync the databases.\n\n"
                     "Do you want to continue?";
        } else {
            message = "This will disable the chaotic-aur repository by modifying /etc/pacman.conf.\n"
                     "You will be prompted for administrator privileges.\n\n"
                     "Do you want to continue?";
        }
        
        auto reply = QMessageBox::question(this, "Confirm Repository Change", 
                                          message,
                                          QMessageBox::Yes | QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            return;
        }
        
        // Apply the change
        bool chaoticSuccess = false;
        if (currentChaoticAurState) {
            chaoticSuccess = enableChaoticAurInPacmanConf();
        } else {
            chaoticSuccess = disableChaoticAurInPacmanConf();
        }
        
        if (chaoticSuccess) {
            m_originalChaoticAurState = currentChaoticAurState;
            changesApplied = true;
            
            // Emit signal to notify other widgets
            emit chaoticAurStatusChanged(currentChaoticAurState);
        } else {
            success = false;
        }
    }
    
    // Show results and offer database sync if changes were applied
    if (changesApplied && success) {
        m_statusLabel->setText("Settings applied successfully! Please sync package databases.");
        m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
        m_statusLabel->show();
        
        m_applyButton->setEnabled(false);
        
        // Suggest database sync
            auto reply = QMessageBox::question(this, "Sync Package Database",
                                              "Would you like to sync the package database now?\n"
                                              "(This will run 'pkexec pacman -Sy')",
                                              QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                QProcess process;
                m_statusLabel->setText("Syncing package databases...");
                process.start("pkexec", QStringList() << "pacman" << "-Sy");
                process.waitForFinished(60000); // 60 second timeout
                
                if (process.exitCode() == 0) {
                    m_statusLabel->setText("Package databases synced successfully!");
                    Logger::info("Package databases synced after repository change");
                    
                    // Refresh ALPM databases to pick up the new repository
                    AlpmWrapper::instance().refreshDatabases();
                } else {
                    m_statusLabel->setText("Failed to sync package databases. Please run 'sudo pacman -Sy' manually.");
                    m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
                }
            } else {
                // Even if they don't sync now, refresh ALPM to detect the new repo configuration
                AlpmWrapper::instance().refreshDatabases();
            }
    } else if (!success) {
        m_statusLabel->setText("Failed to apply settings. Please check permissions.");
        m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
        m_statusLabel->show();
    }
}

bool SettingsWidget::isMultilibEnabled() const {
    return m_multilibRepoCheckbox->isChecked() && 
           (m_multilibRepoCheckbox->isChecked() == m_originalMultilibState);
}

bool SettingsWidget::isChaoticAurEnabled() const {
    return m_chaoticAurCheckbox->isChecked() && 
           (m_chaoticAurCheckbox->isChecked() == m_originalChaoticAurState);
}

void SettingsWidget::applySettings() {
    onApplyClicked();
}

void SettingsWidget::onRemoveLockClicked() {
    QString lockFilePath = "/var/lib/pacman/db.lck";
    
    // Check if lock file exists
    QFile lockFile(lockFilePath);
    if (!lockFile.exists()) {
        QMessageBox::information(this, "Lock File Not Found",
            "The pacman lock file does not exist.\n"
            "No action needed.");
        return;
    }
    
    // Show warning dialog with checkbox
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Remove Pacman Lock File");
    msgBox.setText("Are you sure you want to remove the pacman lock file?");
    msgBox.setInformativeText(
        "This will remove: /var/lib/pacman/db.lck\n\n"
        "WARNING: Only do this if you are certain that no other package manager "
        "(pacman, yay, paru, etc.) is currently running.\n\n"
        "Removing the lock file while a package operation is in progress can "
        "corrupt your package database!");
    
    QCheckBox* confirmCheckbox = new QCheckBox("I understand the risks and confirm no package manager is running");
    msgBox.setCheckBox(confirmCheckbox);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    int ret = msgBox.exec();
    
    if (ret == QMessageBox::Yes && confirmCheckbox->isChecked()) {
        // Use pkexec to remove the lock file with elevated privileges
        QProcess process;
        process.start("pkexec", QStringList() << "rm" << "-f" << lockFilePath);
        process.waitForFinished(30000); // 30 second timeout
        
        if (process.exitCode() == 0) {
            m_statusLabel->setText("Lock file removed successfully!");
            m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
            m_statusLabel->show();
            Logger::info("Pacman lock file removed successfully");
            
            QMessageBox::information(this, "Success",
                "The pacman lock file has been removed successfully.\n"
                "You can now run package operations.");
        } else {
            m_statusLabel->setText("Failed to remove lock file. Check permissions.");
            m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
            m_statusLabel->show();
            Logger::error("Failed to remove pacman lock file");
            
            QMessageBox::critical(this, "Error",
                "Failed to remove the lock file.\n"
                "You may need to run: sudo rm /var/lib/pacman/db.lck");
        }
    } else if (ret == QMessageBox::Yes && !confirmCheckbox->isChecked()) {
        QMessageBox::warning(this, "Confirmation Required",
            "You must check the confirmation box to proceed.");
    }
}

void SettingsWidget::onSetupChaoticClicked() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Setup Chaotic-AUR");
    msgBox.setText("Install Chaotic-AUR repository?");
    msgBox.setInformativeText(
        "This will:\n"
        "1. Download chaotic-keyring and chaotic-mirrorlist packages\n"
        "2. Install them using pacman\n"
        "3. Add the repository to /etc/pacman.conf\n\n"
        "This requires internet connection and administrator privileges.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    
    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    m_statusLabel->setText("Setting up Chaotic-AUR repository...");
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->show();
    m_setupChaoticButton->setEnabled(false);
    
    // Use a shell script to download and install chaotic-aur packages
    // This follows the official installation guide from aur.chaotic.cx
    QString script = 
        "cd /tmp && "
        "rm -f chaotic-keyring.pkg.tar.zst chaotic-mirrorlist.pkg.tar.zst && "
        "curl -L -O https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-keyring.pkg.tar.zst && "
        "curl -L -O https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-mirrorlist.pkg.tar.zst && "
        "pacman -U --noconfirm chaotic-keyring.pkg.tar.zst chaotic-mirrorlist.pkg.tar.zst";
    
    QProcess* process = new QProcess(this);
    
    // Capture both stdout and stderr for debugging
    process->setProcessChannelMode(QProcess::MergedChannels);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        QString output = process->readAll();
        
        Logger::info(QString("Chaotic-AUR setup exit code: %1, status: %2")
                    .arg(exitCode)
                    .arg(exitStatus == QProcess::NormalExit ? "Normal" : "Crashed"));
        
        if (!output.isEmpty()) {
            Logger::debug(QString("Chaotic-AUR setup output:\n%1").arg(output));
        }
        
        process->deleteLater();
        m_setupChaoticButton->setEnabled(true);
        
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            m_statusLabel->setText("Chaotic-AUR packages installed successfully!");
            m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
            m_statusLabel->show();
            Logger::info("Chaotic-AUR packages installed successfully");
            
            // Refresh the chaotic-aur checkbox status
            loadCurrentSettings();
            
            QMessageBox::information(this, "Success",
                "Chaotic-AUR packages installed successfully!\n\n"
                "You can now enable the Chaotic-AUR repository using the checkbox above.\n"
                "After enabling, remember to sync the package databases.");
        } else {
            m_statusLabel->setText("Failed to install Chaotic-AUR packages.");
            m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
            m_statusLabel->show();
            Logger::error(QString("Failed to install Chaotic-AUR packages. Exit code: %1").arg(exitCode));
            
            // Show output in error message if available
            QString errorDetails = "Possible reasons:\n"
                                  "• No internet connection\n"
                                  "• Download failed\n"
                                  "• Installation cancelled\n"
                                  "• User denied authentication\n\n";
            
            if (!output.isEmpty() && output.length() < 500) {
                errorDetails += "Error output:\n" + output;
            }
            
            QMessageBox::critical(this, "Error",
                "Failed to install Chaotic-AUR packages.\n\n" + errorDetails);
        }
    });
    
    process->start("pkexec", QStringList() << "bash" << "-c" << script);
}

void SettingsWidget::onRemoveChaoticClicked() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Remove Chaotic-AUR");
    msgBox.setText("Remove Chaotic-AUR repository?");
    msgBox.setInformativeText(
        "This will remove:\n"
        "• chaotic-keyring\n"
        "• chaotic-mirrorlist\n\n"
        "Note: You may need to manually remove the [chaotic-aur] section "
        "from /etc/pacman.conf to fully disable the repository.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    m_statusLabel->setText("Removing Chaotic-AUR packages...");
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->show();
    m_removeChaoticButton->setEnabled(false);
    
    // Remove chaotic-keyring and chaotic-mirrorlist
    QProcess* process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        m_removeChaoticButton->setEnabled(true);
        
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            m_statusLabel->setText("Chaotic-AUR packages removed successfully!");
            m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
            m_statusLabel->show();
            Logger::info("Chaotic-AUR packages removed successfully");
            
            // Refresh the chaotic-aur checkbox status
            loadCurrentSettings();
            
            QMessageBox::information(this, "Success",
                "Chaotic-AUR packages removed successfully!\n\n"
                "To fully disable the repository, you may need to remove or comment out "
                "the [chaotic-aur] section in /etc/pacman.conf");
        } else {
            m_statusLabel->setText("Failed to remove Chaotic-AUR packages.");
            m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
            m_statusLabel->show();
            Logger::error("Failed to remove Chaotic-AUR packages");
            
            QMessageBox::critical(this, "Error",
                "Failed to remove Chaotic-AUR packages.\n"
                "Please check the logs for details.");
        }
    });
    
    process->start("pkexec", QStringList() << "pacman" << "-Rns" << "--noconfirm" 
                   << "chaotic-keyring" << "chaotic-mirrorlist");
}

void SettingsWidget::onSyncReposClicked() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Sync Repositories");
    msgBox.setText("Synchronize package databases?");
    msgBox.setInformativeText(
        "This will run: pacman -Sy\n\n"
        "This updates the list of available packages from all enabled repositories.\n"
        "This is useful after enabling/disabling repositories or when you want to "
        "ensure you have the latest package information.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    
    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    m_statusLabel->setText("Synchronizing repositories...");
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->show();
    m_syncReposButton->setEnabled(false);
    
    // Run pacman -Sy with pkexec
    QProcess* process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        m_syncReposButton->setEnabled(true);
        
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            m_statusLabel->setText("Repositories synchronized successfully!");
            m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
            m_statusLabel->show();
            Logger::info("Repositories synchronized successfully");
            
            // Refresh ALPM databases
            AlpmWrapper::instance().refreshDatabases();
            
            QMessageBox::information(this, "Success",
                "Package databases synchronized successfully!\n\n"
                "The package list has been updated with the latest available packages.");
        } else {
            m_statusLabel->setText("Failed to synchronize repositories.");
            m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
            m_statusLabel->show();
            Logger::error("Failed to synchronize repositories");
            
            QMessageBox::critical(this, "Error",
                "Failed to synchronize package databases.\n"
                "Please check your internet connection and try again.");
        }
    });
    
    process->start("pkexec", QStringList() << "pacman" << "-Sy");
}

void SettingsWidget::onCancelProcessClicked() {
    // Check if there's actually a process running
    if (!PackageManager::instance().isOperationRunning()) {
        QMessageBox::information(this, "No Process Running",
            "There is no package operation currently running.\n"
            "Nothing to cancel.");
        return;
    }
    
    // Show confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Cancel Running Process");
    msgBox.setText("Are you sure you want to cancel the running package operation?");
    msgBox.setInformativeText(
        "This will stop the current installation, uninstallation, or update process.\n\n"
        "WARNING: Cancelling a package operation may leave your system in an inconsistent state.\n"
        "You may need to run the operation again to complete it properly.\n\n"
        "It's recommended to only cancel if the process is truly stuck or unresponsive.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    m_statusLabel->setText("Cancelling running process...");
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->show();
    
    // Cancel the operation
    PackageManager::instance().cancelRunningOperation();
    
    m_statusLabel->setText("Process cancelled successfully!");
    m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
    m_statusLabel->show();
    
    Logger::info("User cancelled running package operation from settings");
    
    QMessageBox::information(this, "Process Cancelled",
        "The running package operation has been cancelled.\n\n"
        "If you were in the middle of installing or updating a package, "
        "you may need to run the operation again to complete it.");
}

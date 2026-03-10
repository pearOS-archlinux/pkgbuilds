#include "package_manager.h"
#include "../utils/logger.h"
#include <QStandardPaths>
#include <QFile>

PackageManager& PackageManager::instance() {
    static PackageManager instance;
    return instance;
}

PackageManager::PackageManager()
    : QObject(nullptr)
    , m_process(std::make_unique<QProcess>()) {
    
    detectHelper();
    
    connect(m_process.get(), &QProcess::finished, 
            this, &PackageManager::onProcessFinished);
    connect(m_process.get(), &QProcess::errorOccurred,
            this, &PackageManager::onProcessError);
    connect(m_process.get(), &QProcess::readyReadStandardOutput,
            this, &PackageManager::onProcessOutput);
    connect(m_process.get(), &QProcess::readyReadStandardError,
            this, &PackageManager::onProcessOutput);
}

PackageManager::~PackageManager() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        m_process->waitForFinished(3000);
    }
}

void PackageManager::detectHelper() {
    // Check for yay first
    QString yayPath = QStandardPaths::findExecutable("yay");
    if (!yayPath.isEmpty()) {
        m_helper = Helper::Yay;
        Logger::info("Using yay as package helper");
        return;
    }
    
    // Check for paru - deprecate because paru doesn't allow running with pkexec
    // QString paruPath = QStandardPaths::findExecutable("paru");
    // if (!paruPath.isEmpty()) {
    //     m_helper = Helper::Paru;
    //     Logger::info("Using paru as package helper");
    //     return;
    // }
    
    // Default to pacman
    m_helper = Helper::Pacman;
    Logger::info("Using pacman as package helper");
}

QString PackageManager::getHelperName() const {
    switch (m_helper) {
        case Helper::Yay: return "yay";
        case Helper::Pacman: return "pacman";
        default: return "pacman";
    }
}

void PackageManager::installPackage(const QString& packageName, const QString& repository) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Logger::info(QString("Installing package: %1 from %2").arg(packageName, repository.isEmpty() ? "default" : repository));
    emit operationStarted(QString("Installing %1...").arg(packageName));
    
    // Determine if this is an AUR package (not from official repos or chaotic-aur)
    QString repoLower = repository.toLower();
    bool isAUR = repoLower == "aur";
    QString helper = getHelperName();
    
    QString command;
    if (isAUR && (m_helper == Helper::Yay)) {
        // AUR packages - use pkexec to get userpassword before hand
        // Paru has a problem here, so default to yay
        command = QString("pkexec %1 -S %2 --noconfirm").arg(helper, packageName);
    } else {
        // Official repos and chaotic-aur need root access and use pacman
        command = QString("pkexec pacman -S %1 --noconfirm").arg(packageName);
    }
    
    executeCommand("sh", QStringList() << "-c" << command);
}

void PackageManager::uninstallPackage(const QString& packageName, const QString& repository) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Logger::info(QString("Uninstalling package: %1 from %2").arg(packageName, repository.isEmpty() ? "default" : repository));
    emit operationStarted(QString("Uninstalling %1...").arg(packageName));
    
    // Uninstall always needs root (even for AUR packages, they're in the system db once installed)
    QString command = QString("pkexec pacman -Rdd %1 --noconfirm").arg(packageName);
    
    executeCommand("sh", QStringList() << "-c" << command);
}

void PackageManager::updatePackage(const QString& packageName, const QString& repository) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Logger::info(QString("Updating package: %1 from %2").arg(packageName, repository.isEmpty() ? "default" : repository));
    emit operationStarted(QString("Updating %1...").arg(packageName));
    
    // Determine if this is an AUR package (not from official repos or chaotic-aur)
    QString repoLower = repository.toLower();
    bool isAUR = repoLower == "aur";
    QString helper = getHelperName();
    
    QString command;
    if (isAUR && (m_helper == Helper::Yay)) {
        // AUR packages - run helper as regular user (no pkexec)
        command = QString("%1 -S %2 --noconfirm").arg(helper, packageName);
    } else {
        // Official repos and chaotic-aur need root access and use pacman
        command = QString("pkexec pacman -S %1 --noconfirm").arg(packageName);
    }
    
    executeCommand("sh", QStringList() << "-c" << command);
}

void PackageManager::updateAllPackages() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Logger::info("Updating all packages");
    emit operationStarted("Updating all packages...");
    
    QString command = QString("pkexec %1 -Syu --noconfirm")
                        .arg(getHelperName());
    
    executeCommand("sh", QStringList() << "-c" << command);
}

void PackageManager::executeCommand(const QString& command, const QStringList& args) {
    if (m_process->state() != QProcess::NotRunning) {
        Logger::warning("Another operation is already running");
        emit operationError("Another operation is already in progress");
        return;
    }
    
    // Merge stdout and stderr so we capture all output
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    
    Logger::debug(QString("Executing: %1 %2").arg(command, args.join(" ")));
    
    // Emit the actual command being executed to the UI for visibility
    QString fullCommand = command + " " + args.join(" ");
    emit operationOutput(QString(">> Executing: %1\n").arg(fullCommand));
    
    m_process->start(command, args);
    
    // Check if process started successfully
    if (!m_process->waitForStarted(3000)) {
        QString error = QString("Failed to start process: %1").arg(m_process->errorString());
        Logger::error(error);
        emit operationError(error);
    }
}

void PackageManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QString output = m_process->readAllStandardOutput();
    QString error = m_process->readAllStandardError();
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        Logger::info("Operation completed successfully");
        emit operationCompleted(true, "Operation completed successfully");
    } else {
        Logger::error(QString("Operation failed with exit code %1").arg(exitCode));
        Logger::error(QString("Error output: %1").arg(error));
        emit operationCompleted(false, QString("Operation failed: %1").arg(error));
    }
}

void PackageManager::onProcessError(QProcess::ProcessError /*error*/) {
    QString errorString = m_process->errorString();
    Logger::error(QString("Process error: %1").arg(errorString));
    emit operationError(errorString);
}

void PackageManager::onProcessOutput() {
    // Since we merged channels, only read stdout (which includes stderr)
    QString output = m_process->readAll();
    if (!output.isEmpty()) {
        Logger::debug(QString("Process output: %1").arg(output.trimmed()));
        emit operationOutput(output);
    }
}

void PackageManager::cancelRunningOperation() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        Logger::warning("Cancelling running operation...");
        emit operationOutput("\n>>> Operation cancelled by user <<<\n");
        
        // When using pkexec, we need to kill the actual pacman/yay/paru process
        // not just the pkexec wrapper. Use pkill to terminate all package manager processes.
        QProcess killProcess;
        killProcess.start("pkexec", QStringList() << "bash" << "-c" 
                         << "pkill -TERM pacman; pkill -TERM yay; pkill -TERM paru");
        killProcess.waitForFinished(2000);
        
        // Also terminate the QProcess wrapper
        m_process->terminate();
        
        // Wait up to 3 seconds for graceful termination
        if (!m_process->waitForFinished(3000)) {
            // Force kill if still running
            Logger::warning("Process did not terminate gracefully, forcing kill...");
            killProcess.start("pkexec", QStringList() << "bash" << "-c" 
                             << "pkill -KILL pacman; pkill -KILL yay; pkill -KILL paru");
            killProcess.waitForFinished(2000);
            
            m_process->kill();
            m_process->waitForFinished(1000);
        }
        
        emit operationCompleted(false, "Operation cancelled by user");
        Logger::info("Operation cancelled successfully");
    } else {
        Logger::warning("No operation is currently running");
    }
}

bool PackageManager::isOperationRunning() const {
    return m_process && m_process->state() != QProcess::NotRunning;
}

#include "alpm_wrapper.h"
#include "../utils/logger.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <algorithm>

AlpmWrapper& AlpmWrapper::instance() {
    static AlpmWrapper instance;
    return instance;
}

AlpmWrapper::AlpmWrapper() {
    // Member initialization is done in header file
}

AlpmWrapper::~AlpmWrapper() {
    release();
}

bool AlpmWrapper::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    alpm_errno_t err;
    m_handle = alpm_initialize("/", "/var/lib/pacman", &err);
    
    if (!m_handle) {
        Logger::error(QString("Failed to initialize ALPM: %1")
                     .arg(alpm_strerror(err)));
        return false;
    }
    
    // Register sync databases - read from enabled repositories
    QStringList repos = getEnabledRepositories();
    for (const auto& repo : repos) {
        alpm_db_t* db = alpm_register_syncdb(m_handle, 
                                             repo.toStdString().c_str(),
                                             ALPM_SIG_USE_DEFAULT);
        if (!db) {
            Logger::warning(QString("Failed to register sync db: %1").arg(repo));
        } else {
            Logger::info(QString("Registered sync db: %1").arg(repo));
        }
    }
    
    m_syncDbs = alpm_get_syncdbs(m_handle);
    m_initialized = true;
    
    Logger::info("ALPM initialized successfully");
    return true;
}

void AlpmWrapper::release() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_handle) {
        alpm_release(m_handle);
        m_handle = nullptr;
        m_syncDbs = nullptr;
        m_initialized = false;
        Logger::info("ALPM released");
    }
}

QVector<PackageInfo> AlpmWrapper::searchPackages(const QString& query) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        Logger::error("ALPM not initialized");
        return {};
    }
    
    QVector<PackageInfo> results;
    
    // Search in sync databases
    for (alpm_list_t* i = m_syncDbs; i; i = i->next) {
        auto* db = static_cast<alpm_db_t*>(i->data);
        searchInDatabase(db, query, results);
    }
    
    return results;
}

void AlpmWrapper::searchInDatabase(alpm_db_t* db, const QString& query,
                                  QVector<PackageInfo>& results) {
    if (!db) return;
    
    alpm_list_t* pkgs = alpm_db_get_pkgcache(db);
    QString lowerQuery = query.toLower();
    
    for (alpm_list_t* i = pkgs; i; i = i->next) {
        auto* pkg = static_cast<alpm_pkg_t*>(i->data);
        QString pkgName = QString::fromUtf8(alpm_pkg_get_name(pkg));
        
        if (pkgName.toLower().contains(lowerQuery)) {
            PackageInfo info;
            info.name = pkgName;
            info.version = QString::fromUtf8(alpm_pkg_get_version(pkg));
            info.description = QString::fromUtf8(alpm_pkg_get_desc(pkg));
            info.repository = QString::fromUtf8(alpm_db_get_name(db));
            info.maintainer = QString::fromUtf8(alpm_pkg_get_packager(pkg));
            info.upstreamUrl = QString::fromUtf8(alpm_pkg_get_url(pkg));
            info.dependList = convertDependList(alpm_pkg_get_depends(pkg));
            
            alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
            info.lastUpdated = QDateTime::fromSecsSinceEpoch(buildDate);
            
            results.push_back(std::move(info));
        }
    }
}

QVector<PackageInfo> AlpmWrapper::getInstalledPackages() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        Logger::error("ALPM not initialized");
        return {};
    }
    
    QVector<PackageInfo> packages;
    alpm_db_t* localDb = alpm_get_localdb(m_handle);
    
    if (!localDb) {
        Logger::error("Failed to get local database");
        return {};
    }
    
    alpm_list_t* pkgs = alpm_db_get_pkgcache(localDb);
    
    for (alpm_list_t* i = pkgs; i; i = i->next) {
        auto* pkg = static_cast<alpm_pkg_t*>(i->data);
        
        PackageInfo info;
        info.name = QString::fromUtf8(alpm_pkg_get_name(pkg));
        info.version = QString::fromUtf8(alpm_pkg_get_version(pkg));
        info.description = QString::fromUtf8(alpm_pkg_get_desc(pkg));
        info.repository = QString::fromUtf8(alpm_db_get_name(alpm_pkg_get_db(pkg)));
        info.maintainer = QString::fromUtf8(alpm_pkg_get_packager(pkg));
        info.upstreamUrl = QString::fromUtf8(alpm_pkg_get_url(pkg));
        info.dependList = convertDependList(alpm_pkg_get_depends(pkg));
        
        alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
        info.lastUpdated = QDateTime::fromSecsSinceEpoch(buildDate);
        
        packages.push_back(std::move(info));
    }
    
    return packages;
}

bool AlpmWrapper::isPackageInstalled(const QString& packageName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return false;
    }
    
    alpm_db_t* localDb = alpm_get_localdb(m_handle);
    if (!localDb) {
        return false;
    }
    
    alpm_pkg_t* pkg = alpm_db_get_pkg(localDb, packageName.toStdString().c_str());
    return pkg != nullptr;
}

PackageInfo AlpmWrapper::getPackageInfo(const QString& packageName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    PackageInfo info;
    if (!m_initialized) {
        return info;
    }
    
    // First check local database
    alpm_db_t* localDb = alpm_get_localdb(m_handle);
    if (localDb) {
        alpm_pkg_t* pkg = alpm_db_get_pkg(localDb, packageName.toStdString().c_str());
        if (pkg) {
            info.name = QString::fromUtf8(alpm_pkg_get_name(pkg));
            info.version = QString::fromUtf8(alpm_pkg_get_version(pkg));
            info.description = QString::fromUtf8(alpm_pkg_get_desc(pkg));
            info.repository = QString::fromUtf8(alpm_db_get_name(alpm_pkg_get_db(pkg)));
            info.maintainer = QString::fromUtf8(alpm_pkg_get_packager(pkg));
            info.upstreamUrl = QString::fromUtf8(alpm_pkg_get_url(pkg));
            info.dependList = convertDependList(alpm_pkg_get_depends(pkg));
            
            alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
            info.lastUpdated = QDateTime::fromSecsSinceEpoch(buildDate);
            
            return info;
        }
    }
    
    // Check sync databases
    for (alpm_list_t* i = m_syncDbs; i; i = i->next) {
        auto* db = static_cast<alpm_db_t*>(i->data);
        alpm_pkg_t* pkg = alpm_db_get_pkg(db, packageName.toStdString().c_str());
        
        if (pkg) {
            info.name = QString::fromUtf8(alpm_pkg_get_name(pkg));
            info.version = QString::fromUtf8(alpm_pkg_get_version(pkg));
            info.description = QString::fromUtf8(alpm_pkg_get_desc(pkg));
            info.repository = QString::fromUtf8(alpm_db_get_name(db));
            info.maintainer = QString::fromUtf8(alpm_pkg_get_packager(pkg));
            info.upstreamUrl = QString::fromUtf8(alpm_pkg_get_url(pkg));
            info.dependList = convertDependList(alpm_pkg_get_depends(pkg));
            
            alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
            info.lastUpdated = QDateTime::fromSecsSinceEpoch(buildDate);
            
            return info;
        }
    }
    
    return info;
}

QStringList AlpmWrapper::getPackageFileList(const QString& packageName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QStringList out;
    if (!m_initialized) return out;

    alpm_pkg_t* pkg = nullptr;
    alpm_db_t* localDb = alpm_get_localdb(m_handle);
    if (localDb) {
        pkg = alpm_db_get_pkg(localDb, packageName.toStdString().c_str());
    }
    if (!pkg) {
        for (alpm_list_t* i = m_syncDbs; i; i = i->next) {
            auto* db = static_cast<alpm_db_t*>(i->data);
            pkg = alpm_db_get_pkg(db, packageName.toStdString().c_str());
            if (pkg) break;
        }
    }
    if (!pkg) return out;

    const alpm_filelist_t* filelist = alpm_pkg_get_files(pkg);
    if (!filelist) return out;
    for (size_t j = 0; j < filelist->count; ++j) {
        out.append(QString::fromUtf8(filelist->files[j].name));
    }
    return out;
}

QString AlpmWrapper::getPackageFilename(const QString& packageName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) return QString();
    for (alpm_list_t* i = m_syncDbs; i; i = i->next) {
        auto* db = static_cast<alpm_db_t*>(i->data);
        alpm_pkg_t* pkg = alpm_db_get_pkg(db, packageName.toStdString().c_str());
        if (pkg) {
            const char* fn = alpm_pkg_get_filename(pkg);
            return fn ? QString::fromUtf8(fn) : QString();
        }
    }
    return QString();
}

QVector<UpdateInfo> AlpmWrapper::getAvailableUpdates() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QVector<UpdateInfo> updates;
    
    if (!m_initialized) {
        Logger::error("ALPM not initialized");
        return updates;
    }
    
    alpm_db_t* localDb = alpm_get_localdb(m_handle);
    if (!localDb) {
        return updates;
    }
    
    alpm_list_t* pkgs = alpm_db_get_pkgcache(localDb);
    
    for (alpm_list_t* i = pkgs; i; i = i->next) {
        auto* localPkg = static_cast<alpm_pkg_t*>(i->data);
        const char* pkgName = alpm_pkg_get_name(localPkg);
        
        // Check each sync database for newer version
        for (alpm_list_t* j = m_syncDbs; j; j = j->next) {
            auto* syncDb = static_cast<alpm_db_t*>(j->data);
            alpm_pkg_t* syncPkg = alpm_db_get_pkg(syncDb, pkgName);
            
            if (syncPkg) {
                int cmp = alpm_pkg_vercmp(alpm_pkg_get_version(syncPkg),
                                         alpm_pkg_get_version(localPkg));
                
                if (cmp > 0) {
                    UpdateInfo update;
                    update.name = QString::fromUtf8(pkgName);
                    update.oldVersion = QString::fromUtf8(alpm_pkg_get_version(localPkg));
                    update.newVersion = QString::fromUtf8(alpm_pkg_get_version(syncPkg));
                    update.repository = QString::fromUtf8(alpm_db_get_name(syncDb));
                    update.downloadSize = alpm_pkg_get_size(syncPkg);
                    
                    updates.push_back(std::move(update));
                    break;
                }
            }
        }
    }
    
    Logger::info(QString("Found %1 available updates").arg(updates.size()));
    return updates;
}

QStringList AlpmWrapper::convertDependList(alpm_list_t* deps) {
    QStringList result;
    
    for (alpm_list_t* i = deps; i; i = i->next) {
        auto* dep = static_cast<alpm_depend_t*>(i->data);
        result.append(QString::fromUtf8(dep->name));
    }
    
    return result;
}

QStringList AlpmWrapper::getEnabledRepositories() const {
    QStringList repos;
    
    // Read /etc/pacman.conf to find enabled repositories
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf");
        // Return default repositories
        return {"core", "extra"};
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Check for repository sections (not commented out)
        if (line.startsWith("[") && line.endsWith("]") && !line.startsWith("#")) {
            QString repo = line.mid(1, line.length() - 2);
            
            // Filter out non-repository sections
            if (repo != "options" && repo != "testing" && repo != "core-testing" && 
                repo != "extra-testing" && repo != "multilib-testing") {
                repos.append(repo);
            }
        }
    }
    
    file.close();
    
    // Ensure core and extra are always present
    if (!repos.contains("core")) {
        repos.prepend("core");
    }
    if (!repos.contains("extra")) {
        repos.insert(1, "extra");
    }
    
    Logger::info(QString("Enabled repositories: %1").arg(repos.join(", ")));
    return repos;
}

void AlpmWrapper::refreshDatabases() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        Logger::error("ALPM not initialized");
        return;
    }
    
    // Release current handle
    if (m_handle) {
        alpm_release(m_handle);
        m_handle = nullptr;
        m_syncDbs = nullptr;
        m_initialized = false;
    }
    
    // Re-initialize to pick up new repositories
    m_mutex.unlock();
    initialize();
    m_mutex.lock();
    
    Logger::info("ALPM databases refreshed");
}
